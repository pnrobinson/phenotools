#ifndef PROPERTY_H
#define PROPERTY_H

#include <vector>
#include <string>
#include <map>
#include <rapidjson/document.h>
#include "termid.h"
#include "jsonparse_exception.h"

using std::vector;
using std::map;
using std::string;

enum class Prop {
  UNKNOWN,
  CREATED_BY, //created_by
  CREATION_DATE, //creation_date
  HAS_OBO_NAMESPACE, //hasOBONamespace
  HAS_ALTERNATIVE_ID, //hasAlternativeId
  RDF_SCHEMA_COMMENT,//rdf-schema#comment
  DATE,//"date" -- probably an error
  OWL_DEPRECATED,//owl#deprecated
  IS_ANONYMOUS,//oboInOwl#is_anonymous
  CONSIDER,//oboInOwl#consider
  EDITOR_NOTES,//hsapdv#editor_notes
  CREATOR, //creator
  DESCRIPTION, //description
  LICENSE, //license
  RIGHTS,//rights
  SUBJECT,//subject
  TITLE,//title
  DEFAULT_NAMESPACE,//oboInOwl#default-namespace
  LOGICAL_DEFINITION_VIEW_RELATION,//oboInOwl#logical-definition-view-relation
  SAVED_BY,//oboInOwl#saved-by
  CLOSE_MATCH, // core#closeMatch
  EXACT_MATCH, // core#exactMatch
  BROAD_MATCH,//core#broadMatch
  NARROW_MATCH, //core#narrowMatch
  EXCLUDED_SUBCLASS_OF, // mondo#excluded_subClassOf
  SEE_ALSO, // rdf-schema#seeAlso
  IS_METADATA_TAG, //oboInOwl#is_metadata_tag
  SHORT_HAND, //oboInOwl#shorthand
  TERM_REPLACED_BY, // IAO_0100001
  RELATED, //mondo#related
  EXCLUDED_SYNONYM, // mondo#excluded_synonym
  IS_CLASS_LEVEL, //oboInOwl#is_class_level
  PATHOGENESIS, //mondo#pathogenesis
  NEVER_IN_TAXON, // RO_0002161
  SOURCE, //http://purl.org/dc/terms/source
  HAS_OBO_FORMAT_VERSION,
  HOMEPAGE,
};
/**
  * A simple class that stores the basicPropertyValues elements about
  * a Term.
  */
class PropertyValue {
private:
  Prop property_;
  string value_;
  PropertyValue(Prop p, const string &v):property_(p),value_(v){}
public:
  static PropertyValue of(const rapidjson::Value &val);
  friend std::ostream& operator<<(std::ostream& ost, const PropertyValue& pv);
  bool is_alternate_id() const { return property_ == Prop::HAS_ALTERNATIVE_ID; }
  string get_value() const { return value_; }
};
std::ostream& operator<<(std::ostream& ost, const PropertyValue& pv);


class Property {
private:
  TermId id_;
  string label_;
  vector<PropertyValue> property_values_;
  Property(TermId id,string label,vector<PropertyValue> vals):
    id_(id),
    label_(label),
    property_values_(vals){}
  static map<string,Prop> property_registry_;
public:
  static Property of(const rapidjson::Value &val);
  Property(const Property &p);
  Property(Property &&p);
  ~Property(){}
  Property &operator=(const Property &p);
  Property &operator=(Property &&p);
  friend std::ostream& operator<<(std::ostream& ost, const Property& prop);
};
std::ostream& operator<<(std::ostream& ost, const Property& prop);


#endif
