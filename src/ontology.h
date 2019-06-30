#ifndef ONTOLOGY_H
#define ONTOLOGY_H

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <rapidjson/document.h>
#include "jsonparse_exception.h"

using std::string;
using std::vector;
using std::map;




class TermId {
private:
    string value_;
    std::size_t separator_pos_;
    TermId(const string &s,std::size_t pos);

public:

    TermId(const TermId  &tid);
    TermId(TermId &&tid);
    TermId &operator=(const TermId &tid);
    bool operator<(const TermId& rhs) const;
    ~TermId(){}
    static TermId of(const string &s);
    static TermId from_url(const string &s);
    static TermId of(const rapidjson::Value &val);
    string get_value() const { return value_; }
    string get_prefix() const { return value_.substr(0,separator_pos_); }
    string get_id() const { return value_.substr(separator_pos_+1); }

    friend std::ostream& operator<<(std::ostream& ost, const TermId& tid);
};
std::ostream& operator<<(std::ostream& ost, const TermId& tid);

class Xref {
private:
    TermId term_id_;
    Xref(const TermId &tid): term_id_(tid){}

public:
    Xref(const Xref &txr);
    Xref(Xref &&txr) = default;
    Xref &operator=(const Xref &txr);
    static Xref of(const rapidjson::Value &val);
    static Xref fromCurieString(const rapidjson::Value &val);
    friend std::ostream& operator<<(std::ostream& ost, const Xref& txref);
};
std::ostream& operator<<(std::ostream& ost, const Xref& txref);

enum class Property {
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
};
/**
  * A simple class that stores the basicPropertyValues elements about
  * a Term.
  */
class PropertyValue {
private:
  Property property_;
  string value_;
  PropertyValue(Property p, const string &v):property_(p),value_(v){}
public:
  static PropertyValue of(const rapidjson::Value &val);
  friend std::ostream& operator<<(std::ostream& ost, const PropertyValue& pv);
  bool is_alternate_id() const { return property_ == Property::HAS_ALTERNATIVE_ID; }
  string get_value() const { return value_; }
};
std::ostream& operator<<(std::ostream& ost, const PropertyValue& pv);


class Term {
private:
  TermId id_;
  string label_;
  string definition_;
  vector<Xref> definition_xref_list_;
  vector<Xref> term_xref_list_;
  vector<TermId> alternative_id_list_;
  vector<PropertyValue> property_values_;
  bool is_obsolete_ = false;

public:
  Term(const TermId &id, const string &label);
  void add_definition(const string &def);
  void add_definition_xref(const Xref &txref);
  void add_term_xref(const Xref &txref) { term_xref_list_.push_back(txref); }
  void add_property_value(const PropertyValue &pv);

  TermId get_term_id() const { return id_; }
  bool has_alternative_ids() const { return ! alternative_id_list_.empty(); }
  vector<TermId> get_alternative_ids() const { return alternative_id_list_; }
  bool obsolete() const { return is_obsolete_; }
  friend std::ostream& operator<<(std::ostream& ost, const Term& term);
};
std::ostream& operator<<(std::ostream& ost, const Term& term);

enum class EdgeType {
  IS_A,
};

class Edge {
private:
  TermId source_;
  TermId dest_;
  EdgeType edge_type_;
  Edge(TermId s,EdgeType et,TermId o):
    source_(s),
    dest_(o),
    edge_type_(et) {}
public:
  static Edge of(const rapidjson::Value &val);
  TermId get_source() const { return source_; }
  TermId get_destination() const { return dest_; }
  friend std::ostream& operator<<(std::ostream& ost, const Edge& edge);
};
std::ostream& operator<<(std::ostream& ost, const Edge& edge);


class Ontology {
private:
  string id_;
  vector<PropertyValue> property_values_;
  map<TermId, std::shared_ptr<Term> > term_map_;
  /** Current primary TermId's. */
  vector<TermId> current_term_ids_;
  /** obsoleted and alt ids. */
  vector<TermId> obsolete_term_ids_;

  vector<Edge> edge_list_;


public:
  Ontology() = default;
  Ontology(const Ontology &other);
  Ontology(Ontology &other);
  Ontology& operator=(const Ontology &other);
  Ontology& operator=(Ontology &&other);
  ~Ontology(){}
  void set_id(const string &id) { id_ = id; }
  void add_property_value(const PropertyValue &propval);
  void add_all_terms(const vector<Term> &terms);
  void add_all_edges(const vector<Edge> &edges);
  int current_term_count() const { return current_term_ids_.size(); }
  int total_term_id_count() const { return term_map_.size(); }
  int edge_count() const { return edge_list_.size(); }
  std::optional<Term> get_term(const TermId &tid) const;

  Ontology(vector<Term> terms,vector<Edge> edges,string id, vector<PropertyValue> properties);


  friend std::ostream& operator<<(std::ostream& ost, const Ontology& ontology);
};
std::ostream& operator<<(std::ostream& ost, const Ontology& ontology);

#endif
