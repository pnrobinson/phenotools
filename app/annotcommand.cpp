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

using std::cout;
using std::cerr;


using namespace phenotools;

AnnotationCommand::AnnotationCommand(const string &path, 
                const string &hp_json, 
                const string &date, 
                const string &termid):
    phenotype_hpoa_path(path),
    hp_json_path(hp_json),
    termid_(termid),
    date_(date)
{

}





int
AnnotationCommand::execute()
{
    cout << "[INFO] Parsing " << phenotype_hpoa_path << "\n";
    vector<HpoAnnotation> annots = HpoAnnotation::parse_phenotype_hpoa(phenotype_hpoa_path);
    cout << "[INFO] Obtained " << annots.size() << " annotations.\n";
    if (hp_json_path.empty()) {
        cerr << "[WARNING] No path to hp.json passed\n";
        return 0;
    }
    JsonOboParser parser{hp_json_path};
    auto ontology =  parser.get_ontology();
    if (termid_.empty()) {
        cerr << "[ERROR] No term id passed\n";
        return 1;
    } else if (date_.empty()) {
        cerr << "[ERROR] No date string passed\n";
        return 1;
    }
    TermId tid = TermId::from_string(termid_);
    tm threshold_date = {};
    int y,M,d,h,m;
    float s; 
    sscanf(date_.c_str(), "%d-%d-%dT%d:%d:%fZ", &y, &M, &d, &h, &m, &s);
    threshold_date.tm_year = y - 1900; // Year since 1900
    threshold_date.tm_mon = M - 1;     // 0-11
    threshold_date.tm_mday = d;  
    int total = 0;
    int total_newer = 0; // created after target date.
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
        }
    }
    cout << total
        << " total annotations to term " << tid 
        <<", of which " 
        << total_newer
        << " are newer than "
        << date_
        << "\n";

    return 0;
}