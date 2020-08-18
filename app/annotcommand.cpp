/**
 * @file annotcommand.cc
 *
 *  Created on: Aug 10, 2020
 *  Author: Peter N Robinson
 */

#include "annotcommand.h"
#include "../lib/jsonobo.h"
#include "../lib/ontology.h"
#include "../lib/termid.h"


#include <iostream>
#include <fstream>

using std::cout;
using std::cerr;
using std::make_unique;


using namespace phenotools;

AnnotationCommand::AnnotationCommand(const string &path, 
                const string &hp_json, 
                const string &date, 
                const string &enddate, 
                const string &termid,
                const string &outpath):
    phenotype_hpoa_path(path),
    hp_json_path(hp_json),
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
    if (! enddate.empty()) {
        // this means the user did not pass an end date. We set the end date to 42 days after today to include everything
        // up to and including the present time
        // current date/time based on current system
        time_t now = time(0);
        tm *gmtm = gmtime(&now);
        this->end_date_ = make_unique<struct tm>(*gmtm);
    } else {
        this->end_date_ = make_unique<struct tm>(string_to_time(enddate));
    }
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
    cout << "[INFO] Parsing " << phenotype_hpoa_path << "\n";
    vector<HpoAnnotation> annots = HpoAnnotation::parse_phenotype_hpoa(phenotype_hpoa_path);
    cout << "[INFO] Obtained " << annots.size() << " annotations.\n";
    JsonOboParser parser{hp_json_path};
    auto ontology =  parser.get_ontology();
    cout <<"[INFO] HPO ontology with " << ontology->current_term_count() << " terms\n" ;
    cout <<"[INFO] We will output descendants from term " << tid << "\n";
    std::optional<Term> termopt = ontology->get_term(tid);
    string term_label = "n/a";
    if (termopt) {
       term_label = termopt->get_label();
    } else {
        cout << "[ERROR] Could not retrieve term object for " << tid << "\n";
        return;
    }
    ost << "#" << tid << " (" << term_label << ")\n";
    for (HpoAnnotation ann : annots) {
        if (! ann.is_omim()) {
            continue;
        }
        TermId hpoid = ann.get_hpo_id();
        if (! ontology->exists_path(hpoid, tid)) {
            continue;
            // the term is not a descendant
        }
        total++;
        tm anndate = ann.get_curation_date();
        if (in_time_window(ann.get_curation_date())) {
            total_newer++;
            //cout << ann << "\n";
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



int
AnnotationCommand::execute()
{
    if (hp_json_path.empty()) {
        cerr << "[WARNING] No path to hp.json passed\n";
        return 0;
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
    return 0;
}


/**
 * This function checks whether a date is within the two trheshold dates
 * A typical use case is to ask whether a term was created between 2015 and 2018.
 */
 bool 
 AnnotationCommand::in_time_window(tm time) const
 {
     if (! start_date_) {
         cout << "NOT START\n";
         exit(1);
     }
    if (time.tm_year > start_date_->tm_year &&  time.tm_year < end_date_->tm_year) {
        return true;
    } else if (time.tm_mon > start_date_->tm_mon &&  time.tm_mon < end_date_->tm_mon) {
        return true;
    } else if (time.tm_mday >= start_date_->tm_mday &&  time.tm_mday <= end_date_->tm_mday) {
        return true;
    } else {
        return false;
    }
 }
