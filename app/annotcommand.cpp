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


using namespace phenotools;

AnnotationCommand::AnnotationCommand(const string &path, 
                const string &hp_json, 
                const string &date, 
                const string &termid,
                const string &outpath):
    phenotype_hpoa_path(path),
    hp_json_path(hp_json),
    termid_(termid),
    date_(date),
    outpath_(outpath)
{

}

AnnotationCommand::AnnotationCommand(const string &path, 
                const string &hp_json, 
                const string &date, 
                const string &termid):
    phenotype_hpoa_path(path),
    hp_json_path(hp_json),
    termid_(termid),
    date_(date),
    outpath_("")
{

}




void
AnnotationCommand::output_descendants(std::ostream & ost)
{
    tm threshold_date = {};
    int y,M,d,h,m;
    float s; 
    sscanf(date_.c_str(), "%d-%d-%dT%d:%d:%fZ", &y, &M, &d, &h, &m, &s);
    threshold_date.tm_year = y - 1900; // Year since 1900
    threshold_date.tm_mon = M - 1;     // 0-11
    threshold_date.tm_mday = d;  
    int total = 0;
    int total_newer = 0; // created after target date.
    TermId tid = TermId::from_string(termid_);
    cout << "[INFO] Parsing " << phenotype_hpoa_path << "\n";
    vector<HpoAnnotation> annots = HpoAnnotation::parse_phenotype_hpoa(phenotype_hpoa_path);
    cout << "[INFO] Obtained " << annots.size() << " annotations.\n";
    JsonOboParser parser{hp_json_path};
    auto ontology =  parser.get_ontology();
    std::optional<Term> termopt = ontology->get_term(tid);
    string term_label = "n/a";
    if (termopt) {
       term_label = termopt->get_label();
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
        if (ann.newer_than(threshold_date)) {
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