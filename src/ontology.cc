
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
	Prop prop = Prop::UNKNOWN;
	if (pred == "oboInOwl#created_by") {
		prop = Prop::CREATED_BY;
	} else if (pred == "oboInOwl#creation_date") {
	 	prop = Prop::CREATION_DATE;
	} else if (pred == "oboInOwl#hasOBONamespace") {
		prop = Prop::HAS_OBO_NAMESPACE;
	} else if (pred == "oboInOwl#hasAlternativeId") {
		prop = Prop::HAS_ALTERNATIVE_ID;
	} else if (pred == "rdf-schema#comment") {
		prop = Prop::RDF_SCHEMA_COMMENT;
	} else if (pred == "date") {
		prop = Prop::DATE;
	} else if (pred == "owl#deprecated") {
		prop = Prop::OWL_DEPRECATED;
	} else if (pred == "oboInOwl#is_anonymous") {
		prop = Prop::IS_ANONYMOUS;
	} else if (pred == "oboInOwl#consider") {
		prop = Prop::CONSIDER;
	} else if (pred == "hsapdv#editor_notes") {
		prop = Prop::EDITOR_NOTES;
	} else if (pred == "creator") {
		prop = Prop::CREATOR;
	} else if (pred == "description") {
		prop = Prop::DESCRIPTION;
	} else if (pred == "license") {
		prop = Prop::LICENSE;
	} else if (pred == "rights") {
		prop = Prop::RIGHTS;
	} else if (pred == "subject") {
		prop = Prop::SUBJECT;
	} else if (pred == "title") {
		prop = Prop::TITLE;
	} else if (pred == "oboInOwl#default-namespace") {
		prop = Prop::DEFAULT_NAMESPACE;
	} else if (pred == "oboInOwl#logical-definition-view-relation") {
		prop = Prop::LOGICAL_DEFINITION_VIEW_RELATION;
	} else if (pred == "oboInOwl#saved-by") {
		prop = Prop::	SAVED_BY;
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
		case Prop::CREATED_BY: ost << "created_by: "; break;
		case Prop::CREATION_DATE: ost << "creation_date: "; break;
		case Prop::HAS_OBO_NAMESPACE: ost << "has_obo_namespace: "; break;
		case Prop::DATE: ost << "date: "; break;
		case Prop::CREATOR: ost << "creator: "; break;
		case Prop::DESCRIPTION: ost << "description: "; break;
		case Prop::LICENSE: ost << "license: "; break;
		case Prop::RIGHTS: ost << "rights: "; break;
		case Prop::SUBJECT: ost << "subject: "; break;
		case Prop::TITLE: ost << "title: "; break;
		case Prop::DEFAULT_NAMESPACE: ost << "default-namespace: "; break;
		case Prop::LOGICAL_DEFINITION_VIEW_RELATION: ost << "logical-definition-view-relation: "; break;
		case Prop::SAVED_BY: ost << "saved-by: "; break;

		default: ost <<"other property value: "; break;
	}
	ost << pv.value_;
	return ost;
}

Property
Property::of(const rapidjson::Value &val){
	string id;
	string label;
	vector<PropertyValue> propvals;
	if (! val.IsObject()) {
		throw JsonParseException("Attempt to add malformed node (not JSON object)");
	}
	if (! val.HasMember("type")) {
		throw JsonParseException("Attempt to add malformed node (no type information).");
	} else if (strcmp ( val["type"].GetString(),"PROPERTY") ) {
		string tt =val["type"].GetString();
		throw JsonParseException("Attempt to add malformed property node (not a PROPERTY): " + tt);
	}
	if (! val.HasMember("id")) {
		throw JsonParseException("Attempt to add malformed node (no id).");
	} else {
		id = val["id"].GetString();
	}
	if (! val.HasMember("lbl")) {
		throw JsonParseException("Malformed node ("+id+"): no label.");
	} else {
		label = val["lbl"].GetString();
	}
	TermId tid = TermId::of(id);
	if (! val.HasMember("meta")) {
		throw JsonParseException("Malformed node ("+id+"): no Metainformation");
	} else {
		const rapidjson::Value &meta = val["meta"];
		if (! meta.IsObject()) {
			throw JsonParseException("Malformed node ("+id+"): meta is not JSON object.");
		}
		auto itr = meta.FindMember("basicPropertyValues");
		if (itr != meta.MemberEnd()) {
			const rapidjson::Value &propertyVals = itr->value;
			if (! propertyVals.IsArray()) {
				throw JsonParseException("Malformed node ("+id+"): Term property values not array");
			}
			for (auto elem = propertyVals.Begin(); elem != propertyVals.End(); elem++) {
				PropertyValue propval = PropertyValue::of(*elem);
				propvals.push_back(propval);
			}
		}
	}
	Property p{tid,label,propvals};
	return p;
}

Property::Property(const Property &p):
	id_(p.id_),
	label_(p.label_),
	property_values_(p.property_values_)
	{ }

Property::Property(Property &&p):
	id_ (std::move(p.id_)),
	label_( std::move(p.label_) ),
	property_values_ (std::move(p.property_values_) ) {}


Property &
Property::operator=(const Property &p){
	id_ = p.id_;
	label_ = p.label_;
	property_values_ = p.property_values_;
	return *this;
}
Property &
Property::operator=(Property &&p){
	id_ = std::move(p.id_);
	label_ = std::move(p.label_);
	property_values_ = std::move(p.property_values_);
	return *this;
}

std::ostream& operator<<(std::ostream& ost, const Property& prop){
	ost << prop.label_ << "[" << prop.id_ << "] ";
	for (auto pv : prop.property_values_ ) {
		ost << pv <<"; ";
	}
	return ost;
}



Term::Term(const TermId &id, const string &label):
  id_(id),label_(label) {}


Term Term::of(const rapidjson::Value &val){
	string id;
	string label;
	if (! val.IsObject()) {
		throw JsonParseException("Attempt to add malformed node (not JSON object)");
	}
	if (! val.HasMember("type")) {
		throw JsonParseException("Attempt to add malformed node (no type information).");
	} else if (strcmp ( val["type"].GetString(),"CLASS") ) {
		string tt =val["type"].GetString();
		throw JsonParseException("Attempt to add malformed node (not a CLASS): " + tt);
	}
	if (! val.HasMember("id")) {
		throw JsonParseException("Attempt to add malformed node (no id).");
	} else {
		id = val["id"].GetString();
	}
	if (! val.HasMember("lbl")) {
		throw JsonParseException("Malformed node ("+id+"): no label.");
	} else {
		label = val["lbl"].GetString();
	}
	TermId tid = TermId::of(id);
	Term term{tid,label};
	if (! val.HasMember("meta")) {
		throw JsonParseException("Malformed node ("+id+"): no Metainformation");
	} else {
		const rapidjson::Value &meta = val["meta"];
		if (! meta.IsObject()) {
			throw JsonParseException("Malformed node ("+id+"): meta is not JSON object.");
		}
		rapidjson::Value::ConstMemberIterator itr = meta.FindMember("definition");
		if (itr != meta.MemberEnd()) {
			const rapidjson::Value &definition = meta["definition"];
			rapidjson::Value::ConstMemberIterator it = definition.FindMember("val");
			if (it != definition.MemberEnd()) {
				string definition_value = it->value.GetString();
				term.add_definition(definition_value);
			}
			it = definition.FindMember("xrefs");
			if (it != definition.MemberEnd()) {
				const rapidjson::Value& defxrefs = it->value;
				if (! defxrefs.IsArray()) {
					throw JsonParseException("Malformed node ("+id+"): xref not array");
				}
				for (auto xrefs_itr = defxrefs.Begin();
									xrefs_itr != defxrefs.End(); ++xrefs_itr) {
              Xref xr = Xref::fromCurieString(*xrefs_itr); // xrefs in definitions are simply CURIEs.
              term.add_definition_xref(xr);
        }
			} // done with definition
			itr = meta.FindMember("xrefs");
			if (itr != meta.MemberEnd()) {
				const rapidjson::Value &xrefs = itr->value;
				if (! xrefs.IsArray()) {
					throw JsonParseException("Malformed node ("+id+"): Term Xrefs not array");
				} else {
					for (auto elem = xrefs.Begin(); elem != xrefs.End(); elem++) {
              auto elem_iter = elem->FindMember("val");
              if (elem_iter != elem->MemberEnd()) {
	                Xref txr = Xref::of(elem_iter->value);
                  term.add_term_xref(txr);
              }
          }
				}
			itr = meta.FindMember("basicPropertyValues");
			if (itr != meta.MemberEnd()) {
				const rapidjson::Value &propertyVals = itr->value;
				if (! propertyVals.IsArray()) {
					throw JsonParseException("Malformed node ("+id+"): Term property values not array");
				}
				for (auto elem = propertyVals.Begin(); elem != propertyVals.End(); elem++) {
					PropertyValue propval = PropertyValue::of(*elem);
					term.add_property_value(propval);
				}
			}
			}
		}
	}
	return term;
}


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
Ontology::add_property(const Property & prop){
	property_list_.push_back(prop);
	std::cout << "Added and size is now "<<property_list_.size() << "\n";
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
	ost << "### Properties ###\n"
			<< "total properties: " << ontology.property_count() << "\n";
		for (auto p : ontology.property_list_) {
			ost << p << "\n";
		}
		return ost;

}
