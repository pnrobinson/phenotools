#ifndef ONTOLOGY_H
#define ONTOLOGY_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <rapidjson/document.h>
#include "termid.h"
#include "property.h"
#include "jsonparse_exception.h"

using std::string;
using std::vector;
using std::map;


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
  static Term of(const rapidjson::Value &val);
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
  DISEASE_HAS_FEATURE,
  DISEASE_HAS_BASIS_IN_DISRUPTION_OF,
  DISEASE_HAS_BASIS_IN_DYSFUNCTION_OF,
  DISEASE_HAS_BASIS_IN_FEATURE,
  DISEASE_HAS_INFLAMMATION_SITE,
  DISEASE_HAS_LOCATION,
  DISEASE_HAS_MAJOR_FEATURE,
  DISEASE_CAUSES_FEATURE,
  DISEASE_RESPONDS_TO,
  DISEASE_SHARES_FEATURES_OF,
  DISEASE_HAS_BASIS_IN_DEVELOPMENT_OF,
  HAS_MODIFIER,
  REALIZED_IN_RESPONSE_TO,
  REALIZED_IN_RESPONSE_TO_STIMULUS,
  TRANSMITTED_BY,
  DISEASES_CAUSES_DISRUPTION_OF,
  DISEASE_ARISES_FROM_STRUCTURE,
  PREDISPOSES_TOWARDS,
  REALIZED_IN,
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
  static map<string,EdgeType> edgetype_registry_;   
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
  vector<Property> property_list_;
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
  void add_property(const Property & prop);
  void add_all_terms(const vector<Term> &terms);
  void add_all_edges(const vector<Edge> &edges);
  int current_term_count() const { return current_term_ids_.size(); }
  int total_term_id_count() const { return term_map_.size(); }
  int edge_count() const { return edge_list_.size(); }
  int property_count() const { return property_list_.size(); }
  std::optional<Term> get_term(const TermId &tid) const;

  Ontology(vector<Term> terms,vector<Edge> edges,string id, vector<PropertyValue> properties);


  friend std::ostream& operator<<(std::ostream& ost, const Ontology& ontology);
};
std::ostream& operator<<(std::ostream& ost, const Ontology& ontology);

#endif
