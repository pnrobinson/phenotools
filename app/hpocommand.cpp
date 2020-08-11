#include <iostream>
#include <optional>
#include <sstream>
#include <fstream>

#include "hpocommand.h"
#include "../lib/jsonobo.h"
#include "../lib/property.h"

#define EMPTY_STRING ""

using namespace phenotools;
using std::string;
using std::cout;
using std::cerr;
using std::stringstream;
using std::make_unique;




HpoCommand::HpoCommand(string hp_json_path, 
                bool descriptive_stats, 
                bool quality_control,
                const string &date,
                const string &termid,
                bool debug) : HpoCommand(hp_json_path, descriptive_stats,quality_control,date,termid,debug,EMPTY_STRING) 
                {
                }

HpoCommand::HpoCommand(std::string hp_json_path, 
                        bool descriptive_stats, 
                        bool quality_control,
                        const string &date,
                        const string &termid,
                        bool debug,
                        const string &outpath):
    show_descriptive_stats(descriptive_stats),
    show_quality_control(quality_control),
    debug_(debug),
    outpath_(outpath),
    threshold_date_str(date)
{
    JsonOboParser parser{hp_json_path};
    error_list_ = parser.get_errors();
    this->ontology = parser.get_ontology();
    if (! date.empty()) {
        this->threshold_date_ = make_unique<struct tm>(string_to_time(date));
    }
    if (! termid.empty()) {
        this->tid_ = make_unique<TermId>(TermId::from_string(termid));
    } 
}

int
HpoCommand::execute()
{
    if (show_quality_control) {
      show_qc();
    }
    if (show_descriptive_stats) {
      show_stats();
    }
    if (debug_) {
        ontology->debug_print();
    }
    if (tid_) {
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
        //count_descendants();
    }
    return EXIT_SUCCESS;
}


void
HpoCommand::show_qc()
{
 if (error_list_.size() == 0) {
    cout <<"[INFO] No errors enounted in JSON parse\n";
    return;
  }
  cout << "[ERRORS]:\n";
  for (string e : error_list_) {
    cout << e << "\n";
  }
   cout << "\n";
}

void
HpoCommand::show_stats()
{
    ontology->output_descriptive_statistics();
}


void 
HpoCommand::output_descendants(std::ostream & ost)
{
    vector<TermId> descs = this->ontology->get_descendant_term_ids(*tid_);
    std::optional<Term> term = this->ontology->get_term(*tid_);
    if (! term) {
        cerr << "[ERROR] Could not find term for " << *tid_ << "\n";
        return;
    }
    string label = term->get_label();
    int N = descs.size(); 
    int total = 0;
    int total_newer = 0;
    ost << "#Subontology: "
        << *tid_
        << " ("
        << label
        << ")\n";
    // Now print the header
    ost << "#hpo.id\thpo.label\tcreation.date\tincluded\n";
    if (threshold_date_) {
        for (TermId tid : descs) {
            total++;
            std::optional<Term> termopt = this->ontology->get_term(tid);
            if (! termopt) {
                cerr << "[ERROR] Could not find term for " << tid << "\n";
                return;
            } 
            label = termopt->get_label();
            tm creation_date = termopt->get_creation_date();
            stringstream ss;
            ss << creation_date.tm_year + 1900
                << "-"
                << creation_date.tm_mon + 1
                << "-"
                << creation_date.tm_mday;
            bool passes_threshold = later_than(creation_date);
            ost << tid
                << "\t"
                << label
                << "\t"
                << ss.str()
                << "\t"
                << (passes_threshold ? "T" : "F")
                << "\n";
            if (passes_threshold) {
                total_newer++;
            }
        }
        ost << "#Created after " << threshold_date_str << ": " << total_newer << "\n";
        ost << "#Total: " << total << "\n";
    } else {
        cout << "#Total: " << total << "\n";
    }
}

void
HpoCommand::count_descendants() 
{
    int total = 0;
    int total_newer = 0;
    vector<TermId> descs = this->ontology->get_descendant_term_ids(*tid_);
    if (threshold_date_) {
        for (TermId tid : descs) {
            total++;
            std::optional<Term> termopt = this->ontology->get_term(tid);
            if (! termopt) {
                cerr << "[ERROR] Could not find term for " << tid << "\n";
                return;
            } else {
                cout << tid << ": " << termopt->get_label() << "\n";
            }
            tm creation_date = termopt->get_creation_date();
            if (later_than(creation_date)) {
                cout << tid << " was created before: " << (1900 + creation_date.tm_year) << "\n";
            } else {
                cout << tid << " was created after: " << (1900 + creation_date.tm_year) << "\n";
                total_newer++;
            }
        }
        cout << " We found " << total << " descendants of " << *tid_ << " including " << total_newer << " new terms\n";
    } else {
        int N = descs.size();
        cout << "Term " << *tid_ << " has " << N << " descendants.\n";
    }
}








 bool 
 HpoCommand::later_than(tm time) const
 {
    if (time.tm_year > threshold_date_->tm_year) {
        return true;
    } else if (time.tm_mon > threshold_date_->tm_mon) {
        return true;
    } else if (time.tm_mday > threshold_date_->tm_mday) {
        return true;
    } else {
        return false;
    }
 }