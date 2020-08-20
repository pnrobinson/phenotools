#include <iostream>
#include <iomanip> 
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


string stringify_date(tm *t) {
    stringstream ss;
    ss  << t->tm_year + 1900
        << "-"
        <<  std::right << std::setfill('0') << std::setw(2)
        << t->tm_mon + 1
        << "-"
        <<  std::right << std::setfill('0') << std::setw(2)
        << t->tm_mday;
    return ss.str();
}



HpoCommand::HpoCommand(const string &hp_json_path, 
                bool descriptive_stats, 
                bool quality_control,
                const string &date,
                const string &end_date,
                const string &termid,
                bool debug) : HpoCommand(hp_json_path, descriptive_stats,quality_control,date,end_date,termid,debug,EMPTY_STRING) 
                {
                }

HpoCommand::HpoCommand(const string &hp_json_path, 
                        bool descriptive_stats, 
                        bool quality_control,
                        const string &date,
                        const string &end_date,
                        const string &termid,
                        bool debug,
                        const string &outpath):
    show_descriptive_stats(descriptive_stats),
    show_quality_control(quality_control),
    threshold_date_str(date),
    end_date_str(end_date),
    debug_(debug),
    outpath_(outpath)
{
    JsonOboParser parser{hp_json_path};
    error_list_ = parser.get_errors();
    this->ontology = parser.get_ontology();
    if (date.empty()) {
        // the following is the birthday of the HPO
        this->start_date_ = make_unique<struct tm>(string_to_time("2008-11-01"));
    } else {
        this->start_date_ = make_unique<struct tm>(string_to_time(date));
    }
    if (end_date.empty()) {
        // this means the user did not pass an end date. We set the end date to 42 days after today to include everything
        // up to and including the present time
        // current date/time based on current system
        time_t now = time(0);
        tm *gmtm = gmtime(&now);
        this->end_date_ = make_unique<struct tm>(*gmtm);
    } else {
        this->end_date_ = make_unique<struct tm>(string_to_time(end_date));
    }
    if (! termid.empty()) {
        this->tid_ = make_unique<TermId>(TermId::from_string(termid));
    } 
}

HpoCommand::HpoCommand(const string &hp_json_path, const string &hpo_term_file, const string &outpath):
    hpo_termfile_(hpo_term_file),
    outpath_(outpath),
    do_term_annotation_(true)
    {
         JsonOboParser parser{hp_json_path};
         error_list_ = parser.get_errors();
         this->ontology = parser.get_ontology();
         if (! error_list_.empty()) {
             for (string s : error_list_) {
                 cerr << "[ERROR] " << s << "\n";
             }
         }
    }

void
HpoCommand::init_toplevel_categories() 
{
    TermId ABNORMAL_CELLULAR = TermId::from_string("HP:0025354"); 
    toplevelCatories_.push_back(ABNORMAL_CELLULAR);
    TermId BLOOD = TermId::from_string("HP:0001871");
    toplevelCatories_.push_back(BLOOD);
    TermId CONNECTIVE_TISSUE = TermId::from_string("HP:0003549");
    toplevelCatories_.push_back(CONNECTIVE_TISSUE);
    TermId HEAD_AND_NECK = TermId::from_string("HP:0000152");
    toplevelCatories_.push_back(HEAD_AND_NECK);
    TermId LIMBS = TermId::from_string("HP:0040064");
    toplevelCatories_.push_back(LIMBS);
    TermId METABOLISM = TermId::from_string("HP:0001939");
    toplevelCatories_.push_back(METABOLISM);
    TermId PRENATAL = TermId::from_string("HP:0001197");
    toplevelCatories_.push_back(PRENATAL);
    TermId BREAST = TermId::from_string("HP:0000769");
    toplevelCatories_.push_back(BREAST);
    TermId CARDIOVASCULAR = TermId::from_string("HP:0001626");
    toplevelCatories_.push_back(CARDIOVASCULAR);
    TermId DIGESTIVE = TermId::from_string("HP:0025031");
    toplevelCatories_.push_back(DIGESTIVE);
    TermId EAR = TermId::from_string("HP:0000598");
    toplevelCatories_.push_back(EAR);
    TermId ENDOCRINE = TermId::from_string("HP:0000818");
    toplevelCatories_.push_back(ENDOCRINE);
    TermId EYE = TermId::from_string("HP:0000478");
    toplevelCatories_.push_back(EYE);
    TermId GENITOURINARY = TermId::from_string("HP:0000119");
    toplevelCatories_.push_back(GENITOURINARY);
    TermId IMMUNOLOGY = TermId::from_string("HP:0002715");
    toplevelCatories_.push_back(IMMUNOLOGY);
    TermId INTEGUMENT = TermId::from_string("HP:0001574");
    toplevelCatories_.push_back(INTEGUMENT);
    TermId MUSCLE = TermId::from_string("HP:0003011");
    toplevelCatories_.push_back(MUSCLE);
    TermId NERVOUS_SYSTEM = TermId::from_string("HP:0000707");
    toplevelCatories_.push_back(NERVOUS_SYSTEM);
    TermId RESPIRATORY = TermId::from_string("HP:0002086");
    toplevelCatories_.push_back(RESPIRATORY);
    TermId SKELETAL = TermId::from_string("HP:0000924");
    toplevelCatories_.push_back(SKELETAL);
    TermId THORACIC_CAVITY = TermId::from_string("HP:0045027");
    toplevelCatories_.push_back(THORACIC_CAVITY);
    TermId VOICE = TermId::from_string("HP:0001608");
    toplevelCatories_.push_back(VOICE);
    TermId CONSTITUTIONAL = TermId::from_string("HP:0025142");
    toplevelCatories_.push_back(CONSTITUTIONAL);
    TermId GROWTH = TermId::from_string("HP:0001507");
    toplevelCatories_.push_back(GROWTH);
    TermId NEOPLASM = TermId::from_string("HP:0002664");
    toplevelCatories_.push_back(NEOPLASM);
    TermId CLINICAL_MODIFIER = TermId::from_string("HP:0012823");
    toplevelCatories_.push_back(CLINICAL_MODIFIER);
    TermId FREQUENCY = TermId::from_string("HP:0040279");
    toplevelCatories_.push_back(FREQUENCY);
    TermId CLINICAL_COURSE = TermId::from_string("HP:0031797");
    toplevelCatories_.push_back(CLINICAL_COURSE);
    TermId MODE_OF_INHERITANCE = TermId::from_string("HP:0000005");
    toplevelCatories_.push_back(MODE_OF_INHERITANCE);
    TermId PAST_MEDICAL_HX = TermId::from_string("HP:0032443");
    toplevelCatories_.push_back(PAST_MEDICAL_HX);
    TermId BLOOD_GROUP = TermId::from_string("HP:0032223");
    toplevelCatories_.push_back(BLOOD_GROUP);
}

void 
HpoCommand::annotate_termfile() const
{
    std::ifstream file(hpo_termfile_);
    if (! file.good()) {
        cerr << "[Could not open " << hpo_termfile_ << " for reading\n";
        return;
    }
    std::ofstream outfile;
    outfile.open(outpath_);
    if (! outfile.good()) {
         cerr << "[Could not open " << outpath_ << " for writing\n";
        return;
    }
    string line; 
    while (std::getline(file, line))
    {
        TermId tid = TermId::from_string(line);
        if (tid.get_value() == "0000118") {
            // root term, skip
            continue;
        }
        std::optional<Term> term = ontology->get_term(tid);
        if (! term) {
            cerr << "[WARNING] Could not retrieve term for " << tid << "\n";
        } else if (term->obsolete()){
            continue;
        }
        if (term->is_alternative_id(tid)) {
            continue;
        }
        std::set<TermId> ancestors = ontology->get_ancestors(tid);
        bool found = false;
        for (TermId t : ancestors) {
            auto p = std::find (toplevelCatories_.begin(), toplevelCatories_.end(), t);
            if (p != toplevelCatories_.end()) {
                outfile << tid << "\t" << *p << "\n";
                found = true;
                break;
            }
        }
        if (! found) {
            cout << "[WARN] Not placed in category: "    
                <<  term->get_label() 
                <<": "
                << tid 
                << "\n";
        }
    }
}

void
HpoCommand::output_terms_by_category() const {
    vector<TermId> termids = ontology->get_current_term_ids();
    std::ofstream outfile;
    outfile.open(outpath_);
    if (! outfile.good()) {
         cerr << "[Could not open " << outpath_ << " for writing\n";
        return;
    }
    for (TermId tid : termids) {
        std::optional<Term> term = ontology->get_term(tid);
        if (! term) {
            cerr << "[WARNING] Could not retrieve term for " << tid << "\n";
        } else if (term->obsolete()){
            continue;
        }
        std::set<TermId> ancestors = ontology->get_ancestors(tid);
        bool found = false;
        for (TermId t : ancestors) {
            auto p = std::find (toplevelCatories_.begin(), toplevelCatories_.end(), t);
            if (p != toplevelCatories_.end()) {
                outfile << tid << "\t" << *p << "\n";
                found = true;
                break;
            }
        }
        if (! found) {
            cout << "[WARN] Not placed in category: "    
                <<  term->get_label() 
                <<": "
                << tid 
                << "\n";
        }
    }

}



int
HpoCommand::execute()
{
    if (do_term_annotation_) {
        init_toplevel_categories();
        if (hpo_termfile_.empty()) {
            output_terms_by_category();
        } else {
            annotate_termfile();
        }
        return EXIT_SUCCESS;
    }


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
   
    for (TermId tid : descs) {
        total++;
        std::optional<Term> termopt = this->ontology->get_term(tid);
        if (! termopt) {
            cerr << "[ERROR] Could not find term for " << tid << "\n";
            continue;
        } 
        label = termopt->get_label();
        tm creation_date = termopt->get_creation_date();
        stringstream ss;
        ss << creation_date.tm_year + 1900
            << "-"
            << creation_date.tm_mon + 1
            << "-"
            << creation_date.tm_mday;
        bool passes_threshold = in_time_window(creation_date);
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
        ost << "#Created after " << threshold_date_str << ": " << total_newer << "\n";
        ost << "#Total: " << total << "\n";
    } 
}

void
HpoCommand::count_descendants() 
{
    int total = 0;
    int total_newer = 0;
    vector<TermId> descs = this->ontology->get_descendant_term_ids(*tid_);
    if (start_date_) {
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
            if (in_time_window(creation_date)) {
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



/**
 * This function checks whether a date is within the two trheshold dates
 * A typical use case is to ask whether a term was created between 2015 and 2018.
 */
 bool 
 HpoCommand::in_time_window(tm time) const
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
 HpoCommand::print_category(const string &path, const string &outpath) const
 {
    std::ifstream file(path);
    std::string line; 
    while (std::getline(file, line))
    {
        cout << line;
    }
     
 }

