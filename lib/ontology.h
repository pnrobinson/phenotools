/**
 * @file ontology.h
 * @brief An object to represent a phenotype or related ontology (HP, MP, GO, MONDO, ECTO).
 * @author Peter N Robinson
 * @date  Created on: Sep 13, 2019
 *
 * Ontology objects contain most of the information contained in the JSON file
 * and additionally provide some algorithms.
 */
#ifndef ONTOLOGY_H
#define ONTOLOGY_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <set>
#include <optional>
#include "termid.h"
#include "edge.h"
#include "property.h"

#include <iostream> // remove after debug


using std::string;
using std::vector;
using std::map;


class Xref {
private:
    TermId term_id_;
public:
    Xref(const TermId &tid): term_id_(tid){}
    Xref(const Xref &txr);
    Xref(Xref &&txr) = default;
    Xref &operator=(const Xref &txr);
    TermId get_termid() const { return term_id_; }
    friend std::ostream& operator<<(std::ostream& ost, const Xref& txref);
};
std::ostream& operator<<(std::ostream& ost, const Xref& txref);

/**
  * synonym of an ontology term
  */
class Synonym {
private:
  enum SynonymType { EXACT, BROAD, NARROW, RELATED };
  SynonymType stype_;
  string label_;

public:
  Synonym(const string &lbl, const string &typ);
  Synonym(const Synonym &) = default;
  Synonym(Synonym &&) = default;
  Synonym &operator=(const Synonym &) = default;
  Synonym &operator=(Synonym &&) = default;
  ~Synonym(){}
  string get_label() const { return label_; }
  bool is_exact() const { return stype_ == SynonymType::EXACT; }
  bool is_broad() const { return stype_ == SynonymType::BROAD; }
  bool is_narrow() const { return stype_ == SynonymType::NARROW; }
  bool is_related() const { return stype_ == SynonymType::RELATED; }
  friend std::ostream& operator<<(std::ostream& ost, const Synonym& synonym);
};
std::ostream& operator<<(std::ostream& ost, const Synonym& synonym);

class Term {
private:
  TermId id_;
  string label_;
  string definition_;
  vector<Xref> definition_xref_list_;
  vector<Xref> term_xref_list_;
  vector<TermId> alternative_id_list_;
  vector<PredicateValue> property_values_;
  vector<Synonym> synonym_list_;
  bool is_obsolete_ = false;

public:
  Term(const TermId &id, const string &label);
  void add_definition(const string &def);
  void add_definition_xref(const Xref &txref);
  void add_term_xref(const Xref &txref) { term_xref_list_.push_back(txref); }
  void add_predicate_value(const PredicateValue &pv);
  void add_synonym(const string &pred, const string &lbl);

  TermId get_term_id() const { return id_; }
  string get_label() const { return label_; }
  string get_definition() const { return definition_; }
  vector<Xref> get_definition_xref_list() const { return definition_xref_list_;}
  vector<Xref> get_term_xref_list() const {return term_xref_list_;}
  bool has_alternative_ids() const { return ! alternative_id_list_.empty(); }
  vector<TermId> get_alternative_ids() const { return alternative_id_list_; }
  vector<PredicateValue> get_property_values() const { return property_values_; }
  vector<TermId> get_isa_parents(const TermId &child) const;
  vector<Synonym> get_synonyms() const { return synonym_list_; }
  bool obsolete() const { return is_obsolete_; }
  friend std::ostream& operator<<(std::ostream& ost, const Term& term);
};
std::ostream& operator<<(std::ostream& ost, const Term& term);


class Ontology {
private:
  string id_;
  vector<PredicateValue> predicate_values_;
  vector<Property> property_list_;
  map<TermId, std::shared_ptr<Term> > term_map_;
  /** Current primary TermId's. */
  vector<TermId> current_term_ids_;
  /** obsoleted and alt ids. */
  vector<TermId> obsolete_term_ids_;
  /** Key: a TermId object. Value: Corresponding index in current_term_ids_. */
  map<TermId, int> termid_to_index_;
  /**  offset_e stores offsets into e_to that indicate where the adjacency lists begin.
  The list for an arbitrary vertex begins at e_to[offset_e[v]] and ends at
  e_to[offset_e[v+1]]-1. */
  vector<int> offset_to_edge_;
  /** The inverse of the above, to allow us to traverse the graph in reverse */
  vector<int> offset_from_edge_;
  /** CSR (Compressed Storage Format) Adjacency list. */
  vector<int> edge_to_;
  /** CSR (Compressed Storage Format) Adjacency list (reverse direction of edges) */
  vector<int> edge_from_;
  /** List of edge types, e.g., IS_A, PART_OF. Has same order as e_to_. */
  vector<EdgeType> edge_type_list_;



  int is_a_edge_count_ = 0;


public:
  Ontology() = delete;
  Ontology(const string &id,
          const vector<Term> &terms,
          vector<Edge> &edges,
          const vector<PredicateValue> &predicates,
          const vector<Property> &properties);
  Ontology(const Ontology &other);
  Ontology(Ontology &other);
  Ontology& operator=(const Ontology &other);
  Ontology& operator=(Ontology &&other);
  ~Ontology(){}
  void set_id(const string &id) { id_ = id; }
  void add_predicate_value(const PredicateValue &propval);
  void add_property(const Property & prop);
  void add_all_terms(const vector<Term> &terms);
  void add_all_edges(vector<Edge> &edges);
  int current_term_count() const { return current_term_ids_.size(); }
  int total_term_id_count() const { return term_map_.size(); }
  int edge_count() const { return edge_to_.size(); }
  int is_a_edge_count() const { return is_a_edge_count_;}
  int predicate_count() const { return predicate_values_.size(); }
  int property_count() const { return property_list_.size(); }
  std::optional<Term> get_term(const TermId &tid) const;
  vector<TermId> get_isa_parents(const TermId &child) const;
  /** @return true if there exists a path from source to dest */
  bool exists_path(const TermId &source, const TermId &dest) const;
  /** @return true if there exists a path of edges that have the indicated edgetype. */
  bool exists_path(const TermId &source, const TermId &dest, EdgeType etype) const;
  /** @return true if t1 and t2 have a common ancestor excluding root */
  bool have_common_ancestor(const TermId &t1, const TermId &t2, const TermId &root) const;
  Ontology(vector<Term> terms,vector<Edge> edges,string id, vector<PredicateValue> properties);
  vector<TermId> get_current_term_ids() const { return current_term_ids_; }
  void debug_print() const;
  friend std::ostream& operator<<(std::ostream& ost, const Ontology& ontology);
};
std::ostream& operator<<(std::ostream& ost, const Ontology& ontology);

#endif
