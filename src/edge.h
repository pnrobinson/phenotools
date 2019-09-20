/**
 * @file edge.h
 * @brief An edge represents a Subject/Predicate/Object from an OBO ontology.
 * @author Peter N Robinson
 * @date  Created on: Sep 13, 2019
 *
 * An Edge is created by parsing the JSON ontology file with rapid json. Edge
 * objects are not stored by the final Ontology object. Instead, a Compressed
 * Storage Format is used that represents the edges using integers to denote
 * source and destinate and has a separate array of EdgeTypes.
 */
#ifndef EDGE_H
#define EDGE_H

#include <map>
#include <rapidjson/document.h>
#include "termid.h"
#include "jsonparse_exception.h"

using std::map;

enum class EdgeType {
  IS_A,
  IS_A_INVERSE,
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
  Edge(TermId s, EdgeType et, TermId o):
    source_(s),
    dest_(o),
    edge_type_(et) {}
    /** Map from string representation of predicates from OBO-JSON file to EdgeType.*/
  static map<string, EdgeType> edgetype_registry_;
public:
  static Edge of(const rapidjson::Value &val);
  /** Construct an EdgeType from a string using edgetype_registry_ .*/
  static EdgeType string_to_edgetype(const string &s);
  TermId get_source() const { return source_; }
  TermId get_destination() const { return dest_; }
  EdgeType get_edge_type() const { return edge_type_; }
  /** Sort edges on source_ first and then dest_. */
  bool operator<(const Edge& rhs) const;
  bool is_is_a() const { return edge_type_ == EdgeType::IS_A; }
  /** @return an Edge object that is the opposite direction as this Edge.
  * The unchecked assumption is that this function will only be used for IS_A edges. */
  Edge get_is_a_inverse() const;
  friend std::ostream& operator<<(std::ostream& ost, const Edge& edge);
};
std::ostream& operator<<(std::ostream& ost, const Edge& edge);


#endif
