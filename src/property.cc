#include "property.h"

#include <iostream>
using std::cerr;


/**
 * map from String to value, static initialization.
 */
map<string, Predicate>
PredicateValue::predicate_registry_ = {
		   {"oboInOwl#created_by", Predicate::CREATED_BY},
		   {"oboInOwl#creation_date", Predicate::CREATION_DATE},
		   {"oboInOwl#hasOBONamespace", Predicate::HAS_OBO_NAMESPACE},
		   {"oboInOwl#hasAlternativeId", Predicate::HAS_ALTERNATIVE_ID},
		   {"oboInOwl#is_class_level", Predicate::IS_CLASS_LEVEL},
		   {"oboInOwl#is_anonymous", Predicate::IS_ANONYMOUS},
		   {"oboInOwl#consider", Predicate::CONSIDER},
		   {"oboInOwl#default-namespace", Predicate::DEFAULT_NAMESPACE},
		   {"oboInOwl#logical-definition-view-relation", Predicate::LOGICAL_DEFINITION_VIEW_RELATION},
		   {"oboInOwl#saved-by", Predicate::SAVED_BY},
		   {"oboInOwl#is_metadata_tag", Predicate::IS_METADATA_TAG},
		   {"oboInOwl#shorthand", Predicate::SHORT_HAND},
		   {"oboInOwl#hasOBOFormatVersion", Predicate::HAS_OBO_FORMAT_VERSION},
		   {"core#closeMatch", Predicate::CLOSE_MATCH},
		   {"core#exactMatch", Predicate::EXACT_MATCH},
		   {"core#broadMatch", Predicate::BROAD_MATCH},
		   {"core#narrowMatch", Predicate::NARROW_MATCH},
		   {"rdf-schema#comment", Predicate::RDF_SCHEMA_COMMENT},
		   {"rdf-schema#seeAlso", Predicate::SEE_ALSO},
		   {"mondo#related", Predicate::RELATED},
		   {"mondo#excluded_subClassOf", Predicate::EXCLUDED_SUBCLASS_OF},
		   {"mondo#pathogenesis", Predicate::PATHOGENESIS},
		   {"date", Predicate::DATE},
		   {"owl#deprecated", Predicate::OWL_DEPRECATED},
		   {"hsapdv#editor_notes", Predicate::EDITOR_NOTES},
		   {"creator", Predicate::CREATOR},
		   {"description", Predicate::DESCRIPTION},
		   {"license",Predicate::LICENSE},
		   {"rights",Predicate::RIGHTS},
		   {"subject",Predicate::SUBJECT},
		   {"title",Predicate::TITLE},
		   {"IAO_0100001",Predicate::TERM_REPLACED_BY},
		   {"RO_0002161",Predicate::NEVER_IN_TAXON},
		   {"mondo#excluded_synonym",Predicate::EXCLUDED_SYNONYM},
		   {"source",Predicate::SOURCE},
		   {"homepage",Predicate::HOMEPAGE},
};

Predicate
PredicateValue::string_to_predicate(const string &s)
{
  auto p = PredicateValue::predicate_registry_.find(s);
  if (p == PredicateValue::predicate_registry_.end()) {
    cerr<< "[WARNING] Unrecognized property: " << s << "\n";
    return Predicate::UNKNOWN;
  }
  return p->second;
}


std::ostream& operator<<(std::ostream& ost, const PredicateValue& pv) {
  switch (pv.predicate_) {
  case Predicate::CREATED_BY: ost << "created_by: "; break;
  case Predicate::CREATION_DATE: ost << "creation_date: "; break;
  case Predicate::HAS_OBO_NAMESPACE: ost << "has_obo_namespace: "; break;
  case Predicate::DATE: ost << "date: "; break;
  case Predicate::CREATOR: ost << "creator: "; break;
  case Predicate::DESCRIPTION: ost << "description: "; break;
  case Predicate::LICENSE: ost << "license: "; break;
  case Predicate::RIGHTS: ost << "rights: "; break;
  case Predicate::SUBJECT: ost << "subject: "; break;
  case Predicate::TITLE: ost << "title: "; break;
  case Predicate::DEFAULT_NAMESPACE: ost << "default-namespace: "; break;
  case Predicate::LOGICAL_DEFINITION_VIEW_RELATION: ost << "logical-definition-view-relation: "; break;
  case Predicate::SAVED_BY: ost << "saved-by: "; break;
  case Predicate::SOURCE: ost << "source: "; break;
  default: ost <<"[WARNING] other Predicate value(todo, property.cc): \n"; break;
  }
  ost << pv.value_;
  return ost;
}

/**
 * map from String to value, static initialization.
 */
map<string, AllowedPropertyValue>
Property::property_registry_ = {
  {"UK spelling", AllowedPropertyValue::UK_SPELLING},
  {"abbreviation", AllowedPropertyValue::ABBREVIATION},
  {"plural form", AllowedPropertyValue::PLURAL_FORM},
  {"layperson term", AllowedPropertyValue::LAYPERSON_TERM},
  {"Consequence of a disorder in another organ system.", AllowedPropertyValue::CONSEQUENCE_OF_A_DISORDER_IN_ANOTHER_ORGAN_SYSTEM},
  {"display label", AllowedPropertyValue::DISPLAY_LABEL},
	{"unknown", AllowedPropertyValue::UNKNOWN},
};

AllowedPropertyValue
Property::string_to_property(const string &s)
{
  auto p = Property::property_registry_.find(s);
  if (p == Property ::property_registry_.end()) {
    cerr<< "[WARNING] Unrecognized property: " << s << "\n";
    exit(1);
  }
  return p->second;
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
  ost << prop.label_ << " [" << prop.id_ << "] ";
  for (auto pv : prop.property_values_ ) {
    ost << pv <<"; ";
  }
  return ost;
}
