#ifndef EDGE_H
#define EDGE_H

#include <map>
#include <rapidjson/document.h>
#include "termid.h"
#include "jsonparse_exception.h"

using std::map;

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
  bool operator<(const Edge& rhs) const;
  friend std::ostream& operator<<(std::ostream& ost, const Edge& edge);
};
std::ostream& operator<<(std::ostream& ost, const Edge& edge);


#endif
