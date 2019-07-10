#include "property.h"

#include <iostream>

using std::cerr;


#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
/**
 * generate a printable String from an arbitrary RAPIDJSON value object.
 * @param json A rapid json object
 * @return a String representation of the JSON.
 */
string get_json_string42(const rapidjson::Value &json)
{
  using namespace rapidjson;
  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  json.Accept(writer);
  return sb.GetString();
}

/**
 * map from String to value, static initialization.
 */
map<string,Prop>
property_registry_ = {
		   {"oboInOwl#created_by", Prop::CREATED_BY},
		   {"oboInOwl#creation_date", Prop::CREATION_DATE},
		   {"oboInOwl#hasOBONamespace",Prop::HAS_OBO_NAMESPACE},
		   {"oboInOwl#hasAlternativeId",Prop::HAS_ALTERNATIVE_ID},
		   {"oboInOwl#is_class_level",Prop::IS_CLASS_LEVEL},
		   {"oboInOwl#is_anonymous",Prop::IS_ANONYMOUS},
		   {"oboInOwl#consider",Prop::CONSIDER},
		   {"oboInOwl#default-namespace" ,Prop::DEFAULT_NAMESPACE},
		   {"oboInOwl#logical-definition-view-relation" ,Prop::LOGICAL_DEFINITION_VIEW_RELATION},
		   {"oboInOwl#saved-by",Prop::SAVED_BY},
		   {"oboInOwl#is_metadata_tag",Prop::IS_METADATA_TAG},
		   {"oboInOwl#shorthand",Prop::SHORT_HAND},
		   {"oboInOwl#hasOBOFormatVersion",Prop::HAS_OBO_FORMAT_VERSION},
		   {"core#closeMatch",Prop::CLOSE_MATCH},
		   {"core#exactMatch",Prop::EXACT_MATCH},
		   {"core#broadMatch",Prop::BROAD_MATCH},
		   {"core#narrowMatch" ,Prop::NARROW_MATCH},
		   {"rdf-schema#comment",Prop::RDF_SCHEMA_COMMENT},
		   {"rdf-schema#seeAlso",Prop::SEE_ALSO},
		   {"mondo#related",Prop::RELATED},
		   {"mondo#excluded_subClassOf",Prop::EXCLUDED_SUBCLASS_OF},
		   {"mondo#pathogenesis",Prop::PATHOGENESIS},
		   {"date",Prop::DATE},
		   {"owl#deprecated",Prop::OWL_DEPRECATED},
		   {"hsapdv#editor_notes",Prop::EDITOR_NOTES},
		   {"creator",Prop::CREATOR},
		   {"description",Prop::DESCRIPTION},
		   {"license",Prop::LICENSE},
		   {"rights",Prop::RIGHTS},
		   {"subject",Prop::SUBJECT},
		   {"title",Prop::TITLE},
		   {"IAO_0100001",Prop::TERM_REPLACED_BY},
		   {"RO_0002161",Prop::NEVER_IN_TAXON},
		   {"mondo#excluded_synonym",Prop::EXCLUDED_SYNONYM},
		   {"source",Prop::SOURCE},
		   {"homepage",Prop::HOMEPAGE},
};

/**
 * construct a PropertyValue from a JSON object
 */
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
 
  auto it = property_registry_.find(pred);
  if (it == property_registry_.end()) {
    std::cerr << get_json_string42(val);
    throw JsonParseException("PropertyValue unrecognized: \"" + pred + "\"");
  } 
  Prop prop = it->second;
   
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
  case Prop::SOURCE: ost << "source: "; break;
  default: ost <<"other property value(todo): "; break;
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

