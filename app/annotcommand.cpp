/**
 * @file annotcommand.cc
 *
 *  Created on: Aug 10, 2020
 *  Author: Peter N Robinson
 */

#include "annotcommand.h"
#include "../lib/jsonobo.h"
#include "../lib/termid.h"


#include <iostream>
#include <fstream>

using std::cout;
using std::cerr;
using std::make_unique;
using std::map;

using namespace phenotools;

string AnnotationCommand::DEFAULT_OUTFILE_NAME = "hpo_annot.txt";

AnnotationCommand::AnnotationCommand(const string &path, 
                const string &hp_json, 
                const string &date, 
                const string &enddate, 
                const string &termid,
                const string &outpath):
    PhenotoolsCommand(hp_json),
    phenotype_hpoa_path(path),
    termid_(termid),
    date_(date),
    enddate_(enddate),
    outpath_(outpath)
{
    if (! date.empty()) {
        this->start_date_ = make_unique<struct tm>(string_to_time(date));
    } else {
        // If the user does not supply a begin date, we will choose a date that is
        // very likely to be before any current bio-ontology
        this->start_date_ = make_unique<struct tm>(string_to_time("1000-01-01"));
    }
    if (enddate.empty()) {
        // this means the user did not pass an end date. We set the end date to 42 days after today to include everything
        // up to and including the present time
        // current date/time based on current system
        time_t now = time(0);
        tm *gmtm = gmtime(&now);
        this->end_date_ = make_unique<struct tm>(*gmtm);
    } else {
        this->end_date_ = make_unique<struct tm>(string_to_time(enddate));
    }
    if (termid.empty()) {
        do_by_toplevel_category_ = true;
    }
    cout << "[INFO] Parsing " << phenotype_hpoa_path << "\n";
    annotations_ = HpoAnnotation::parse_phenotype_hpoa(phenotype_hpoa_path);
    cout << "[INFO] Obtained " << annotations_.size() << " annotations.\n";
    output_annotation_stats(std::cout);
}

/**
 * Constructor used if user does not indicate a path -- output is written to std::out.
 */
AnnotationCommand::AnnotationCommand(const string &path, 
                const string &hp_json, 
                const string &date, 
                const string &enddate, 
                const string &termid):
                AnnotationCommand(path, hp_json,date,enddate,termid, "")
{
    
}




void
AnnotationCommand::output_descendants(std::ostream & ost)
{
    int total = 0;
    int total_newer = 0; // created after target date.
    TermId tid = TermId::from_string(termid_);
    cout <<"[INFO] HPO ontology with " << ontology_->current_term_count() << " terms\n" ;
    cout <<"[INFO] We will output descendants from term " << tid << "\n";
    std::optional<Term> termopt = ontology_->get_term(tid);
    string term_label = "n/a";
    if (termopt) {
       term_label = termopt->get_label();
    } else {
        cout << "[ERROR] Could not retrieve term object for " << tid << "\n";
        return;
    }
    ost << "#" << tid << " (" << term_label << ")\n";
    for (HpoAnnotation ann : annotations_) {
        if (! ann.is_omim()) {
            continue;
        }
        TermId hpoid = ann.get_hpo_id();
        if (! ontology_->exists_path(hpoid, tid)) {
            continue;
            // the term is not a descendant
        }
        total++;
        tm anndate = ann.get_curation_date();
        if (in_time_window(ann.get_curation_date())) {
            total_newer++;
            ost << ann.get_disease_id() 
                << "\t" 
                << ann.get_disease_name() 
                << "\t"
                << ann.get_hpo_id()
                << "\t"
                << ann.get_negated()
                << "\t"
                << ann.get_biocuration_string()
                << "\n";
        } 
    }
    ost << "#total annotations to terms descending from " << term_label
        << ":"
        << total
        << "\n";
    ost << "#total annotations newer than "
        << date_
        << ":"
        << total_newer
        << "\n";
}

 void 
 AnnotationCommand::process_by_top_level_categories() const
 {
    int total = 0;
    int total_in_window = 0;
    std::ofstream outfile;
    if (outpath_.empty()) {
        outfile.open(DEFAULT_OUTFILE_NAME);
    } else {
        outfile.open(outpath_);
    }
    if (! outfile.good()) {
         cerr << "[ERROR ("
            << __FILE__ << ":l." << __LINE__
            << ")] Could not open \"" << outpath_ << "\" for writing\n";
        return;
    }
    for (HpoAnnotation ann : annotations_) {
        if (! ann.is_omim()) {
            continue;
        }
        TermId hpoid = ann.get_hpo_id();
        std::optional<Term> term = ontology_->get_term(hpoid);
        if (! term) {
            cerr << "[ERROR] Could not retrieve term for id: " << hpoid << "\n";
            continue;
            // the term is not a descendant
        }
        string label = term->get_label();
        total++;
        if (! in_time_window(ann.get_curation_date())) {
            continue;
        }
        total_in_window++;
        std::optional<TermId> category = get_toplevel(hpoid);
        if (! category) {
            cerr << "[ERROR] Could not identify top-level id for " << hpoid << "\n";
            continue;
        }
        outfile << hpoid << "\t" << *category << "\n";
    }
    outfile.close();
    
 }



int
AnnotationCommand::execute()
{
    if (do_by_toplevel_category_) {
        init_toplevel_categories();
        process_by_top_level_categories();
        return EXIT_SUCCESS;
    }


    if (termid_.empty()) {
        cerr << "[ERROR] No term id passed\n";
        return 1;
    } else if (date_.empty()) {
        cerr << "[ERROR] No date string passed\n";
        return 1;
    }
    if (outpath_.empty()) {
        output_descendants(std::cout);
    } else {
        std::ofstream fout;
        fout.open(outpath_.c_str(), std::ios::out);
        if (!fout) {
            cerr << "error: open file for output failed!\n";
            abort();  // in <cstdlib> header
        }
        output_descendants(fout);
        fout.close();
    }
    output_annotation_stats(std::cout);
    return 0;
}


/**
 * This function checks whether a date is within the two trheshold dates
 * A typical use case is to ask whether a term was created between 2015 and 2018.
 */
 bool 
 AnnotationCommand::in_time_window(tm time) const
 {
    if (time.tm_year == start_date_->tm_year) {
        if (time.tm_mon < start_date_->tm_mon) {
            return false;
        } else if (time.tm_mon == start_date_->tm_mon && time.tm_mday < start_date_->tm_mday) {
            return false;
        }
        // if we get here, the years are the same and the month day are not earlier.
        // so far, ,so good
    } else if (time.tm_year < start_date_->tm_year) {
        return false;
    }
    // if we get here, that time is equal to or later than start_date_
    if (time.tm_year == end_date_->tm_year) {
        if (time.tm_mon > end_date_->tm_mon) {
            return false;
        } else if (time.tm_mon == end_date_->tm_mon && time.tm_mday > end_date_->tm_mday) {
            return false;
        }
    } else if (time.tm_year > end_date_->tm_year) {
        return false;
    }
    return true;
 }


void 
AnnotationCommand::output_annotation_stats_per_database(std::ostream & ost, const map<string, int> &annotmap, const string &dbasename) const
{
    double n_total = 0;
    auto it = annotmap.begin();
    while (it != annotmap.end()) {
        n_total += static_cast<double>(it->second);
        it++;
    }
       
    it = annotmap.begin();
    while (it != annotmap.end()) {
        int count = it->second;
        double perc = 100.0 * static_cast<double>(count) / n_total;
        cout << dbasename << "/" << it->first << ": " << count << " (" << perc << "%)\n";
        it++;
    } 
    cout << dbasename << " (total): " << static_cast<int>(n_total) << "\n";

}

/**
 * Output counts of sources of annotations according to database and evidence code
 */
void 
AnnotationCommand::output_annotation_stats(std::ostream & ost) const {
    std::map<string, int> decipher;
    std::map<string, int> omim;
    std::map<string, int> orpha;
    map<string, int> decipher_terms;
    map<string, int> orpha_terms;
    map<string, int> omim_terms;
    map<string, int> total_terms;

    for (HpoAnnotation annot : annotations_) {
        string dbase = annot.get_database();
        auto etype =  annot.get_evidence_type_string();
        string termidstring = annot.get_hpo_id().get_value();
        if (dbase == "OMIM") {
            omim[etype]++;
            omim_terms[termidstring]++;
            total_terms[termidstring]++;
        } else if (dbase == "ORPHA") {
            orpha[etype]++;
            orpha_terms[termidstring]++;
            total_terms[termidstring]++;
        } else if (dbase == "DECIPHER") {
            decipher[etype]++;
            decipher_terms[termidstring]++;
            total_terms[termidstring]++;
        } else {
            // should never happen
            std::cerr <<" [ERROR] malformed database prefix: " << dbase << "\n";
        }
    }
    output_annotation_stats_per_database(ost, decipher, "DECIPER");
    output_annotation_stats_per_database(ost, orpha, "ORPHANET");
   output_annotation_stats_per_database(ost, omim, "OMIM");
    cout << "Total annotations: "
        << annotations_.size() << "\n";
    cout << "HPO terms used for annotations:\n";
    cout << "DECIPHER: n=" << decipher_terms.size() << "\n";
    cout << "ORPPHANET n=" << orpha_terms.size() << "\n";
    cout << "OMIM n=" << omim_terms.size() << "\n";
    cout << "Total n=" << total_terms.size() << "\n";

}
