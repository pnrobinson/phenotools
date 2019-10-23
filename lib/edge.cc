/**
 * @file edge.cc
 *
 *  Created on: Sep 13, 2019
 *  Author: Peter N Robinson
 */

#include "edge.h"

#include <iostream> // remove after debug

/**
 * Static initialization of map of edge types. */
map<string,EdgeType>
Edge::edgetype_registry_ = {
			    {"is_a",EdgeType::IS_A},
			    {"RO_0004020",EdgeType::DISEASE_HAS_BASIS_IN_DYSFUNCTION_OF},
			    {"RO_0004021",EdgeType::DISEASE_HAS_BASIS_IN_DISRUPTION_OF},
			    {"RO_0004022",EdgeType::DISEASE_HAS_BASIS_IN_FEATURE},
			    {"RO_0004026",EdgeType::DISEASE_HAS_LOCATION},
			    {"RO_0009501",EdgeType::REALIZED_IN_RESPONSE_TO},
			    {"RO_0004027",EdgeType::DISEASE_HAS_INFLAMMATION_SITE},
			    {"RO_0004028",EdgeType::REALIZED_IN_RESPONSE_TO_STIMULUS},
			    {"RO_0004029",EdgeType::DISEASE_HAS_FEATURE},
			    {"RO_0002573",EdgeType::HAS_MODIFIER},
			    {"RO_0002451",EdgeType::TRANSMITTED_BY},
			    {"RO_0004024",EdgeType::DISEASES_CAUSES_DISRUPTION_OF},
			    {"RO_0004030",EdgeType::DISEASE_ARISES_FROM_STRUCTURE},
			    {"mondo#predisposes_towards",EdgeType::PREDISPOSES_TOWARDS},
			    {"mondo#disease_has_major_feature",EdgeType::DISEASE_HAS_MAJOR_FEATURE},
			    {"mondo#disease_causes_feature",EdgeType::DISEASE_CAUSES_FEATURE},
			    {"mondo#disease_responds_to",EdgeType::DISEASE_RESPONDS_TO},
			    {"mondo#disease_shares_features_of",EdgeType::DISEASE_SHARES_FEATURES_OF},
			    {"mondo#disease_has_basis_in_development_of",EdgeType::DISEASE_HAS_BASIS_IN_DEVELOPMENT_OF},
			    {"BFO_0000054",EdgeType::REALIZED_IN},
  };

EdgeType
Edge::string_to_edgetype(const string &s)
{
  auto p = Edge::edgetype_registry_.find(s);
  if (p == Edge::edgetype_registry_.end()) {
    throw "Unrecognized edgetype:" + s;
  }
  return p->second;
}

Edge
Edge::get_is_a_inverse() const
{
	//Edge(TermId s,EdgeType et,TermId o):
	Edge inv{dest_, EdgeType::IS_A_INVERSE,source_};
	return inv;
}

Edge
Edge::of(const rapidjson::Value &val){
  if (! val.IsObject()) {
    throw JsonParseException("Edge element not JSON object");
  }
  auto p = val.FindMember("sub");
  if (p == val.MemberEnd()) {
    throw JsonParseException("Edge did not contain \'sub\' element");
  }
  
  TermId subj = TermId::from_url(p->value.GetString());
  p = val.FindMember("obj");
  if (p == val.MemberEnd()) {
    throw JsonParseException("Edge did not contain \'obj\' element");
  }
  TermId obj = TermId::from_url(p->value.GetString());
  p = val.FindMember("pred");
  if (p == val.MemberEnd()) {
    throw JsonParseException("Edge did not contain \'pred\' element");
  }
  string etype = p->value.GetString();
  size_t pos = etype.find_last_of("/");
  if (pos != string::npos) {
    etype = etype.substr(pos+1);
  }
  EdgeType edgetype;
  auto it = edgetype_registry_.find(etype);
  if (it != edgetype_registry_.end()) {
    edgetype = it->second;
  } else {
    throw JsonParseException("TODO -- finish up.Could not find edge type "+etype);
  }
  Edge e{subj,edgetype,obj};
  return e;
}

bool
Edge::operator<(const Edge& rhs) const
{/**
  TermId source_;
  TermId dest_;
  EdgeType edge_type_;
  */
  if (source_ == rhs.source_) {
    // sort of dest if the source is the same
    // do not bother sorting on EdgeType because multiple edges between the
    // same to vertices are rarely found in our OBO ontologies
    return dest_ < rhs.dest_;
  } else {
    return source_ < rhs.source_;
  }
}
