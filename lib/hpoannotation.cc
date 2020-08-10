/**
 * @file hpoannotation.cc
 *
 *  Created on: Aug 10, 2020
 *  Author: Peter N Robinson
 */

#include "hpoannotation.h"
#include "../lib/termid.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace phenotools;
using std::cout;
using std::cerr;
using std::make_unique;


#define EXPECTED_NUMBER_OF_FIELDS 12


/**
 * parse from a string line HPO:skoehler[YYYY-MM-DD]
 */
Biocuration::Biocuration(const string &curation)
{
    size_t i = curation.find_first_of('[');
    if (i<0) {
        i = 0; // should never happen, but safe is safe
        curator_ = "n/a";
    } else {
        curator_ = curation.substr(0,i);
    }
    size_t j = curation.find_last_of(']');
    if (j<0) j = curation.size();
    size_t len = j-i;
    string payload = curation.substr(i+1,len);
    tm date = {};
    int y,M,d,h,m;
    float s;
    sscanf(payload.c_str(), "%d-%d-%dT%d:%d:%fZ", &y, &M, &d, &h, &m, &s);
    date.tm_year = y - 1900; // Year since 1900
    date.tm_mon = M - 1;     // 0-11
    date.tm_mday = d;  
    curation_date_ = date;
}


string 
Biocuration::to_string() const
{
    std::stringstream ss(curator_);
    ss << "["
        << (curation_date_.tm_year + 1900)
        << "-"
        << (curation_date_.tm_mon + 1)
        << "-"
        << (curation_date_.tm_mday)
        << "]";
    return ss.str();
}


/**
 * Split a string by tab
 */
vector<string> split(const string &str, char delim)
{
    std::stringstream ss(str);
    std::string item;
    std::vector<string> fields;
    while (std::getline(ss, item, delim))
    {
        fields.push_back(item);
    }
    return fields;
}

/**
 * static function that parses the phenotype.hpoa file and returns a list of annotation lines
 * 
 */
vector<HpoAnnotation> 
HpoAnnotation::parse_phenotype_hpoa(const string &path){
    vector<HpoAnnotation> annotations;
    std::ifstream ifs(path);
    if (! ifs.good()) {
        std::cerr << "[ERROR] Could not open JSON ontology file \"" << path << "\"\n";
        exit(EXIT_FAILURE);
    }
    string line;
    while(getline(ifs, line)){ //read data from file object and put it into string.
        if (line.rfind("#", 0) == 0) {
            continue;
        }
        //cout << line << "\n"; 
        HpoAnnotation annot{line};
        annotations.push_back(annot);
      }
    return annotations;
}


HpoAnnotation::HpoAnnotation(const string &line)
{
    vector<string> fields = split(line,'\t');
    if (fields.size() != EXPECTED_NUMBER_OF_FIELDS) {
        cerr << "[ERROR] Malformed line with "
            << fields.size() << " fields (we expected: "
            << EXPECTED_NUMBER_OF_FIELDS << ")\n";
        exit(1);
    }
    disease_id_ = make_unique<TermId>(TermId::from_string(fields[0]));
    disease_name_ = fields[1];
    negated_ = fields[2].rfind("NOT", 0) == 0;
    hpo_id_ = make_unique<TermId>(TermId::from_string(fields[3]));
    string biocuration_string = fields[11];
    vector<string> curats = split(biocuration_string, ';');
    for (string s : curats) {
        curations_.push_back(Biocuration{s});
    }
}


HpoAnnotation::HpoAnnotation(const HpoAnnotation &annot):
    disease_id_ (make_unique<TermId>(*(annot.disease_id_))),
    disease_name_(annot.disease_name_),
    negated_(annot.negated_),
    hpo_id_(make_unique<TermId>(*(annot.hpo_id_))),
    curations_(annot.curations_)
{

}


 bool 
 HpoAnnotation::is_omim() const
 {
     return disease_id_->get_prefix() == "OMIM";
 }


TermId 
 HpoAnnotation::get_disease_id() const 
 {
     TermId tid(*disease_id_);
     return tid;
 }

TermId
HpoAnnotation::get_hpo_id() const
{
    TermId tid(*hpo_id_);
     return tid;
}


bool 
HpoAnnotation::newer_than(const tm &threshold_date) const
{
   
    for (Biocuration bc : curations_) {
        tm curation_date = bc.get_curation_date();
        if (threshold_date.tm_year > curation_date.tm_year) {
            return false;
        } else if (threshold_date.tm_mon > curation_date.tm_mon) {
            return false;
        } else if (threshold_date.tm_mday > curation_date.tm_mday) {
            return false;
        }
    }
    return true;
}

string 
HpoAnnotation::get_disease_name() const
{
    return disease_name_;
}
         
string 
HpoAnnotation::get_negated() const
{
    return negated_ ? "NOT" : "";
}

         
string 
HpoAnnotation::get_biocuration_string() const
{
    std::stringstream ss;
    for (auto p = curations_.begin(); p != curations_.end(); ++p) {
      ss << p->to_string();
      if (p != curations_.end() - 1)
        ss << ";";
   }
   return ss.str();
}



std::ostream& operator<<(std::ostream& ost, const HpoAnnotation& annot){
  ost << annot.get_disease_id() 
        << "\t" 
        << annot.get_disease_name() 
        << "\t"
        << annot.get_hpo_id()
        << "\t"
        << annot.get_negated()
        << "\t"
        << annot.get_biocuration_string();
    return ost;
}
