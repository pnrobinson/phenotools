
#include "ontology.h"
#include <iostream>
#include <utility> // make_pair


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
		string cp = s;
		std::size_t i = cp.find_last_of('/');
		if (i != string::npos) {
			cp = cp.substr(i+1);
		}
    i = cp.find_first_of(':');
		if (i != string::npos) {
			return TermId{cp,i}; // rely on RVO/move
		}
		i = cp.find_first_of('_'); // some terms are with _
		if (i != string::npos) {
			cp[i] = ':';
			return TermId{cp,i}; // rely on RVO/move
		}
		// orcid.org/0000-0001-5208-3432
		i = s.find_first_of("orcid.org/");
		if (i != string::npos) {
			string orcid= "ORCID:" + cp;
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
/** Comparison operator is used when we use TermId as a key for std::map */
bool
TermId::operator<(const TermId& rhs) const {
		return value_ < rhs.value_;
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
	} else if (pred == "creator") {
		prop = Property::CREATOR;
	} else if (pred == "description") {
		prop = Property::DESCRIPTION;
	} else if (pred == "license") {
		prop = Property::LICENSE;
	} else if (pred == "rights") {
		prop = Property::RIGHTS;
	} else if (pred == "subject") {
		prop = Property::SUBJECT;
	} else if (pred == "title") {
		prop = Property::TITLE;
	} else if (pred == "oboInOwl#default-namespace") {
		prop = Property::DEFAULT_NAMESPACE;
	} else if (pred == "oboInOwl#logical-definition-view-relation") {
		prop = Property::LOGICAL_DEFINITION_VIEW_RELATION;
	} else if (pred == "oboInOwl#saved-by") {
		prop = Property::	SAVED_BY;
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
		case Property::CREATOR: ost << "creator: "; break;
		case Property::DESCRIPTION: ost << "description: "; break;
		case Property::LICENSE: ost << "license: "; break;
		case Property::RIGHTS: ost << "rights: "; break;
		case Property::SUBJECT: ost << "subject: "; break;
		case Property::TITLE: ost << "title: "; break;
		case Property::DEFAULT_NAMESPACE: ost << "default-namespace: "; break;
		case Property::LOGICAL_DEFINITION_VIEW_RELATION: ost << "logical-definition-view-relation: "; break;
		case Property::SAVED_BY: ost << "saved-by: "; break;

		default: ost <<"other property value: "; break;
	}
	ost << pv.value_;
	return ost;
}

Term::Term(const TermId &id, const string &label):
  id_(id),label_(label) {}


void
Term::add_definition(const string &def) {
  definition_.assign(def);
}

void
Term::add_definition_xref(const Xref &txref){
  definition_xref_list_.push_back(txref);
}

/**
	* Add a PropertyValue to this term. Some property values
	* encode alt_ids, and we put them into a separate list.
	*/
void
Term::add_property_value(const PropertyValue &pv){
	if (pv.is_alternate_id()){
		TermId alt_id = TermId::of(pv.get_value());
		alternative_id_list_.push_back(alt_id);
	} else {
		property_values_.push_back(pv);
	}
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

Ontology::Ontology(const Ontology &other):
	id_(other.id_),
	property_values_(other.property_values_),
	term_map_(other.term_map_),
	current_term_ids_(other.current_term_ids_),
	obsolete_term_ids_(other.obsolete_term_ids_),
	edge_list_(other.edge_list_)
	 {
		// no-op
	 }
Ontology::Ontology(Ontology &other): 	id_(other.id_){
	property_values_ = std::move(other.property_values_);
	term_map_ = std::move(other.term_map_);
	current_term_ids_ = std::move(other.current_term_ids_);
	obsolete_term_ids_ = std::move(other.obsolete_term_ids_);
	edge_list_ = std::move(other.edge_list_);
}
Ontology&
Ontology::operator=(const Ontology &other){
	if (this != &other) {
		id_ = other.id_;
		property_values_ = other.property_values_;
		term_map_ = other.term_map_;
		current_term_ids_ = other.current_term_ids_;
		obsolete_term_ids_ = other.obsolete_term_ids_;
		edge_list_ = other.edge_list_;
	}
	return *this;
}
Ontology&
Ontology::operator=(Ontology &&other){
	if (this != &other) {
		id_ = std::move(other.id_);
		property_values_ = std::move(other.property_values_);
		term_map_ = std::move(other.term_map_);
		current_term_ids_ = std::move(other.current_term_ids_);
		obsolete_term_ids_ = std::move(other.obsolete_term_ids_);
		edge_list_ = std::move(other.edge_list_);
	}
	return *this;
}



void
Ontology::add_property_value(const PropertyValue &propval){
		property_values_.push_back(propval);

}

void
Ontology::add_all_terms(const vector<Term> &terms){
	for (auto t : terms) {
		std::shared_ptr<Term> sptr = std::make_shared<Term>(t);
		TermId tid = t.get_term_id();
		string id = tid.get_id();
		term_map_.insert(std::make_pair(tid,sptr));
		if (t.obsolete()){
			obsolete_term_ids_.push_back(tid);
		} else {
			current_term_ids_.push_back(tid);
		}
		if (t.has_alternative_ids()) {
			vector<TermId> alt_ids = t.get_alternative_ids();
			for (auto atid : alt_ids) {
					term_map_.insert(std::make_pair(atid,sptr));
			}
		}
	}
}

/**
	* Figure out how to be more efficient later.
	*/
void
Ontology::add_all_edges(const vector<Edge> &edges){
	edge_list_ = edges;
	int bad=0;
	for (const auto &e : edges) {
		auto src = term_map_.find(e.get_source());
		if (src == term_map_.end() ) {
			std::cerr <<++bad <<") Attempt to add edge with " + e.get_source().get_value() +" not in ontology\n";
		}
		auto dest = term_map_.find(e.get_destination());
		if (dest == term_map_.end() ) {
			std::cerr <<++bad <<") Attempt to add edge with " + e.get_destination().get_value() +" not in ontology\n";
		}
	}
	std::cout << " Could not find " << bad << "\n";
}

std::optional<Term>
Ontology::get_term(const TermId &tid) const{
	auto p = term_map_.find(tid);
	if (p != term_map_.end()) {
		return *(p->second);
	} else {
		return std::nullopt;
	}
}

std::ostream& operator<<(std::ostream& ost, const Ontology& ontology){
	ost << "### Ontology ###\n"
		<< "id: " << ontology.id_ << "\n";
	for (const auto &pv : ontology.property_values_) {
		ost << pv << "\n";
	}
	ost << "### Terms ###\n"
			<< "total current terms: " << ontology.current_term_count() << "\n"
			<< "total term ids (including obsolete/alternative term ids): " <<
				ontology.total_term_id_count() << "\n";
	ost << "### Edges ###\n"
			<< "total edges: " << ontology.edge_count() << "\n";
		return ost;

}
