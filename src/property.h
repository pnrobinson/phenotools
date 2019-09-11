#ifndef PROPERTY_H
#define PROPERTY_H

#include <vector>
#include <string>
#include <map>
//#include <rapidjson/document.h>
#include "termid.h"
#include "jsonparse_exception.h"

using std::vector;
using std::map;
using std::string;

enum class Predicate {
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
  * A simple class that stores Predicates and their Values. These are used
  * to add information about a Term; each instance of this class represents
  * a predicate and its value (i.e., object).
  */
class PredicateValue {
private:
  Predicate predicate_;
  string value_;
  static map<string, Predicate> predicate_registry_;
public:
  PredicateValue(Predicate p, const string &v):predicate_(p),value_(v){}
  //static PropertyValue of(const rapidjson::Value &val);
  bool is_alternate_id() const { return predicate_ == Predicate::HAS_ALTERNATIVE_ID; }
  Predicate get_property() const { return predicate_; }
  string get_value() const { return value_; }
  static Predicate string_to_predicate(const string &s);
  friend std::ostream& operator<<(std::ostream& ost, const PredicateValue& pv);
};
std::ostream& operator<<(std::ostream& ost, const PredicateValue& pv);

/**
  * This class represents items such as "UK spelling" and "abbreviation" that
  * are used to indicate properties of other elements such as synonyms. Note
  * that these values are distinct from the Predicates used above.
  */
class Property {
private:
  TermId id_;
  string label_;
  vector<PredicateValue> property_values_;
public:
  Property(TermId id,string label,vector<PredicateValue> vals):
    id_(id),
    label_(label),
    property_values_(vals){}
  Property(const Property &p);
  Property(Property &&p);
  ~Property(){}
  Property &operator=(const Property &p);
  Property &operator=(Property &&p);
  friend std::ostream& operator<<(std::ostream& ost, const Property& prop);
};
std::ostream& operator<<(std::ostream& ost, const Property& prop);


#endif
