
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
    std::size_t i = s.find_first_of(':');
		if (i != string::npos) {
			return TermId{s,i}; // rely on RVO/move
		}
		i = s.find_first_of('_'); // some terms are with _
		if (i != string::npos) {
			return TermId{s,i}; // rely on RVO/move
		}
		// orcid.org/0000-0001-5208-3432
		i = s.find_first_of("orcid.org/");
		if (i != string::npos) {
			string orcid= "ORCID:" + s.substr(10);
			return TermId{orcid,5};
		}
    throw JsonParseException("Malformed ontology term id: " +s);
}

TermId
TermId::from_url(const string &s){
	  std::size_t i = s.find_last_of('/');
		if (i == string::npos){
			throw JsonParseException("Malformed TermId URL");
		}
		string st = s.substr(i+1);
		TermId t = TermId::of(st);
		return t;
}

TermId TermId::of(const rapidjson::Value &val){
    //printJJ(val);
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


PropertyValue
PropertyValue::of(const rapidjson::Value &val) {
	if (! val.IsObject()) {
		throw JsonParseException("PropertyValue factory expects object");
	}
	auto p = val.FindMember("pred");
	if (p == val.MemberEnd()) {
		throw JsonParseException("PropertyValue did not contain \'pred\' element");
	}
	string pred = val["pred"].GetString();
	size_t pos = pred.find_last_of('/');
	if (pos != string::npos) {
		pred = pred.substr(pos+1);
	}
	Property prop = Property::UNKNOWN;
	if (pred == "oboInOwl#created_by") {
		prop = Property::CREATED_BY;
	} else if (pred == "oboInOwl#creation_date") {
	 	prop = Property::CREATION_DATE;
	} else if (pred == "oboInOwl#hasOBONamespace") {
		prop = Property::HAS_OBO_NAMESPACE;
	} else if (pred == "oboInOwl#hasAlternativeId") {
		prop = Property::HAS_ALTERNATIVE_ID;
	} else if (pred == "rdf-schema#comment") {
		prop = Property::RDF_SCHEMA_COMMENT;
	} else if (pred == "date") {
		prop = Property::DATE;
	} else if (pred == "owl#deprecated") {
		prop = Property::OWL_DEPRECATED;
	} else if (pred == "oboInOwl#is_anonymous") {
		prop = Property::IS_ANONYMOUS;
	} else if (pred == "oboInOwl#consider") {
		prop = Property::CONSIDER;
	} else if (pred == "hsapdv#editor_notes") {
		prop = Property::EDITOR_NOTES;
	} else {
		throw JsonParseException("PropertyValue unrecognized: \"" + pred + "\"");
	}
	p = val.FindMember("val");
	if (p == val.MemberEnd()) {
		throw JsonParseException("PropertyValue did not contain \'val\' element");
	}
	string valu = val["val"].GetString();
	PropertyValue pv{prop,valu};
	return pv;
}
std::ostream& operator<<(std::ostream& ost, const PropertyValue& pv) {
	switch (pv.property_) {
		case Property::CREATED_BY: ost << "created_by: "; break;
		case Property::CREATION_DATE: ost << "creation_date: "; break;
		case Property::HAS_OBO_NAMESPACE: ost << "has_obo_namespace: "; break;
		case Property::DATE: ost << "date: "; break;
	}
	ost << pv.value_;
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
	if (! term.property_values_.empty()) {
		for (const auto& p : term.property_values_){
		 ost << "\tproperty val:" << p << "\n";
	 }
	}
  return ost;
}

Edge
Edge::of(const rapidjson::Value &val){
	if (! val.IsObject()) {
		throw JsonParseException("Edge element not JSON object");
	}
	auto p = val.FindMember("sub");
	if (p == val.MemberEnd()) {
		throw JsonParseException("Edge did not contain \'sub\' element");
	}
	TermId subj = TermId::from_url(p->value.GetString());
	p = val.FindMember("obj");
	if (p == val.MemberEnd()) {
		throw JsonParseException("Edge did not contain \'obj\' element");
	}
	TermId obj = TermId::from_url(p->value.GetString());
	p = val.FindMember("pred");
	if (p == val.MemberEnd()) {
		throw JsonParseException("Edge did not contain \'pred\' element");
	}
	string etype = p->value.GetString();
	EdgeType edgetype = EdgeType::IS_A;
	if (etype == "is_a") {
		edgetype = EdgeType::IS_A;
	} else {
		throw JsonParseException("TODO -- finish up.Could not find edge type "+etype);
	}
	// todo etc.
	Edge e{subj,edgetype,obj};
	return e;

}

std::ostream& operator<<(std::ostream& ost, const Edge& edge){
	ost << edge.source_;
	switch (edge.edge_type_){
		case EdgeType::IS_A: ost << " is_a ";break;
		default: ost << " unknown_edge_type ";
	}
	ost << edge.dest_;
	return ost;
}
