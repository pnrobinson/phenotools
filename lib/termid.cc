#include "termid.h"
#include "myexception.h"

#include <iostream>
using std::cout;


TermId::TermId(const string &s,std::size_t pos):
  value_(s),
  separator_pos_(pos)
{
}

TermId
TermId::from_string(const string &s)
{
  string cp = s;
  std::size_t i = cp.find_last_of('/');
  if (i != string::npos) {
    cp = cp.substr(i+1);
  }
  i = cp.find(':');
  if (string::npos == i ) {
    if (string::npos != s.find("hgnc")) {
      string hgnc = "HGNC:" + cp;
      return TermId{hgnc,4};
    } else if (string::npos != s.find("ICD10")) {
        string icd = "ICD10:" + cp;
      return TermId{icd,5};
    }
    // if we get here, we could not find one of the two
    // above cases
  } else {
    return TermId{cp,i}; // rely on RVO/move
  }
  i = cp.find('_'); // some terms are with _
  if (i != string::npos) {
    cp[i] = ':';
    TermId t{cp,i};
    return TermId{cp,i}; // rely on RVO/move
  }
  // orcid.org/0000-0001-5208-3432
  i = s.find("orcid.org/");
  if (i != string::npos) {
    string orcid= "ORCID:" + cp;
    return TermId{orcid,5};
  }
  throw PhenopacketException("Could not create TermId from string: Malformed ontology term id: " +s);
}

TermId
TermId::from_url(const string &s){
  std::size_t i = s.find("hgnc/");
  if (i != string::npos) {
    string hgnc = "HGNC:" + s.substr(i+5);
    return TermId{hgnc,4};
  }
  i = s.find_last_of('/');
  if (i == string::npos){
    throw PhenopacketException("Could not create TermId from URL: Malformed TermId URL: " + s);
  }
  string st = s.substr(i+1);
  TermId t = TermId::from_string(st);
  return t;
}



TermId::TermId(const TermId &tid):
  value_(tid.value_),
  separator_pos_(tid.separator_pos_) {
}

TermId::TermId(TermId &&tid):
  value_(std::move(tid.value_)),
  separator_pos_(tid.separator_pos_)
{
}

TermId &
TermId::operator=(const TermId &tid){
    value_ = tid.value_;
    separator_pos_ = tid.separator_pos_;
    return *this;
}
TermId &
TermId::operator=(TermId &&tid){
  value_ = std::move(tid.value_);
  separator_pos_ = tid.separator_pos_;
  return *this;
}


std::ostream& operator<<(std::ostream& ost, const TermId& tid){
 ost << tid.value_;
 return ost;
}

bool
operator==(const TermId& lhs, const TermId& rhs)
{
  return lhs.value_ == rhs.value_;
}


bool 
operator!=(const TermId& lhs, const TermId& rhs)
{
  return lhs.value_ != rhs.value_;
}
/** Comparison operator is used when we use TermId as a key for std::map */
bool
TermId::operator<(const TermId& rhs) const {
		return value_ < rhs.value_;
}


const TermId EMPTY_TERMID = TermId::from_string("Empty:Empty");