#include "property.h"

#include <iostream>

using std::cerr;


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
	  prop = Prop::SAVED_BY;
	} else if (pred == "core#closeMatch"){
	  prop = Prop::CLOSE_MATCH;
	} else if (pred == "core#exactMatch"){
	  prop = Prop::EXACT_MATCH;
	} else if (pred == "core#broadMatch"){
	  prop = Prop::BROAD_MATCH;
	} else if (pred == "core#narrowMatch"){
	  prop = Prop::NARROW_MATCH;
	} else if (pred == "mondo#excluded_subClassOf") {
	  prop = Prop::EXCLUDED_SUBCLASS_OF;
	} else if (pred == "rdf-schema#seeAlso") {
	  prop = Prop::SEE_ALSO;
	} else if (pred == "oboInOwl#is_metadata_tag") {
	  prop = Prop::IS_METADATA_TAG;
	} else if (pred == "oboInOwl#shorthand") {
	  prop = Prop::SHORT_HAND;
	} else if (pred == "IAO_0100001") {
	  prop = Prop::TERM_REPLACED_BY;
	} else if (pred == "mondo#related") {
	  prop = Prop::RELATED;
	} else if (pred == "mondo#excluded_synonym") {
	  prop = Prop::EXCLUDED_SYNONYM;
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
    //throw JsonParseException("Malformed node ("+id+"): no Metainformation");
    cerr << "[WARNING] Property has no ("+id+"): no Metainformation\n";
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

