
#include "ontology.h"
#include <iostream>


/// remove the following after development

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
void
printJJ(const rapidjson::Value &json)
{
	using namespace rapidjson;

	StringBuffer sb;
	PrettyWriter<StringBuffer> writer(sb);
	json.Accept(writer);
	auto str = sb.GetString();
    std::cout << "printing element...\n";
	std::cout << str << "\n";
}


TermId::TermId(const string &s,std::size_t pos):value_(s),separator_pos_(pos){}

TermId
TermId::of(const string &s){
    std::size_t found = s.find_first_of(':');
    if (found == string::npos) {
        string msg = "Malformed ontology term id: " +s;
        throw JsonParseException(msg);
    }
    return TermId{s,found}; // rely on RVO/move
}

TermId TermId::of(const rapidjson::Value &val){
    printJJ(val);
    if (! val.IsString() ) {
        throw JsonParseException("Attempt to construct TermId from non-string");
    }
    return TermId::of(val.GetString());
}


TermId::TermId(const TermId &tid): value_(tid.value_),separator_pos_(tid.separator_pos_) {
}

TermId::TermId(TermId &&tid): value_(tid.value_),separator_pos_(tid.separator_pos_) {
}

TermId &
TermId::operator=(const TermId &tid){
    value_ = tid.value_;
    separator_pos_ = tid.separator_pos_;
    return *this;
}


std::ostream& operator<<(std::ostream& ost, const TermId& tid){
 ost << tid.value_;
 return ost;
}



Xref::Xref(const Xref &txr)
:term_id_( txr.term_id_) {
}

Xref
Xref::of(const rapidjson::Value &val){
    if (val.IsString() ) {
        return Xref::fromCurieString(val);
    } else {
        throw JsonParseException("Could not construct Xref");
    }
}


Xref
Xref::fromCurieString(const rapidjson::Value &val){
    TermId tid = TermId::of(val);
    Xref xr{tid};
    return xr;
}



Xref &
Xref::operator=(const Xref &txr) {
 term_id_ = txr.term_id_;
 return *this;
}

std::ostream& operator<<(std::ostream& ost, const Xref& txref){
 ost << "[todo-termxref] " << txref.term_id_;
 return ost;
}

Term::Term(const string &id, const string &label):
  id_(id),label_(label) {}


void
Term::add_definition(const string &def) {
  definition_.assign(def);
}

void
Term::add_definition_xref(const Xref &txref){
  definition_xref_list_.push_back(txref);
}

std::ostream& operator<<(std::ostream& ost, const Term& term){
  ost << term.label_ << " [" << term.id_ << "]\n";
  ost << "def: " << (term.definition_.empty() ? "n/a":term.definition_) << "\n";
  if (! term.definition_xref_list_.empty()) {
    for (const auto& p : term.definition_xref_list_){
      ost << "\tdef-xref:" << p << "\n";
    }
  }
  if (! term.definition_xref_list_.empty()) {
     for (const auto& p : term.term_xref_list_){
      ost << "\tterm-xref:" << p << "\n";
    }
  }
  return ost;
}
