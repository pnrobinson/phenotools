
#include "ontology.h"
#include <iostream>
#include <utility> // make_pair


using std::cerr;
using std::cout;


/// remove the following after development

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
void
printJJ(const rapidjson::Value &json)
{
  using namespace rapidjson;

  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  json.Accept(writer);
  auto str = sb.GetString();
  std::cout << "printing element...\n";
  std::cout << str << "\n";
}


Xref::Xref(const Xref &txr)
:term_id_( txr.term_id_) {
}

Xref
Xref::of(const rapidjson::Value &val){
    if (val.IsString() ) {
        return Xref::fromCurieString(val);
    } else {
        throw JsonParseException("Could not construct Xref");
    }
}


Xref
Xref::fromCurieString(const rapidjson::Value &val){
    TermId tid = TermId::of(val);
    Xref xr{tid};
    return xr;
}



Xref &
Xref::operator=(const Xref &txr) {
 term_id_ = txr.term_id_;
 return *this;
}

std::ostream& operator<<(std::ostream& ost, const Xref& txref){
 ost << "[todo-termxref] " << txref.term_id_;
 return ost;
}


Term::Term(const TermId &id, const string &label):
  id_(id),label_(label) {}


Term Term::of(const rapidjson::Value &val){
  string id;
  string label;
  if (! val.IsObject()) {
    throw JsonParseException("Attempt to add malformed node (not JSON object)");
  }
  if (! val.HasMember("type")) {
    throw JsonParseException("Attempt to add malformed node (no type information).");
  } else if (strcmp ( val["type"].GetString(),"CLASS") ) {
    string tt =val["type"].GetString();
    throw JsonParseException("Attempt to add malformed node (not a CLASS): " + tt);
  }
  if (! val.HasMember("id")) {
    throw JsonParseException("Attempt to add malformed node (no id).");
  } else {
    id = val["id"].GetString();
  }
  if (! val.HasMember("lbl")) {
    //throw JsonParseException("Malformed node ("+id+"): no label.");
    cerr << "[WARNING] node  ("+id+"): no label.\n";
  } else {
    label = val["lbl"].GetString();
  }
  TermId tid = TermId::of(id);
  Term term{tid,label};
  if (! val.HasMember("meta")) {
    //throw JsonParseException("Malformed node ("+id+"): no Metainformation");
    cerr << "[WARNING] node (" << id << ") has no Metainformation\n";
  } else {
    const rapidjson::Value &meta = val["meta"];
    if (! meta.IsObject()) {
      throw JsonParseException("Malformed node ("+id+"): meta is not JSON object.");
    }
    rapidjson::Value::ConstMemberIterator itr = meta.FindMember("definition");
    if (itr != meta.MemberEnd()) {
      const rapidjson::Value &definition = meta["definition"];
      rapidjson::Value::ConstMemberIterator it = definition.FindMember("val");
      if (it != definition.MemberEnd()) {
	string definition_value = it->value.GetString();
	term.add_definition(definition_value);
      }
      it = definition.FindMember("xrefs");
      if (it != definition.MemberEnd()) {
	const rapidjson::Value& defxrefs = it->value;
	if (! defxrefs.IsArray()) {
	  throw JsonParseException("Malformed node ("+id+"): xref not array");
	}
	for (auto xrefs_itr = defxrefs.Begin();
	     xrefs_itr != defxrefs.End(); ++xrefs_itr) {
	  Xref xr = Xref::fromCurieString(*xrefs_itr); // xrefs in definitions are simply CURIEs.
	  term.add_definition_xref(xr);
        }
      } // done with definition
      itr = meta.FindMember("xrefs");
      if (itr != meta.MemberEnd()) {
	const rapidjson::Value &xrefs = itr->value;
	if (! xrefs.IsArray()) {
	  throw JsonParseException("Malformed node ("+id+"): Term Xrefs not array");
	} else {
	  for (auto elem = xrefs.Begin(); elem != xrefs.End(); elem++) {
	    auto elem_iter = elem->FindMember("val");
	    if (elem_iter != elem->MemberEnd()) {
	      Xref txr = Xref::of(elem_iter->value);
	      term.add_term_xref(txr);
	    }
          }
	}
	itr = meta.FindMember("basicPropertyValues");
	if (itr != meta.MemberEnd()) {
	  const rapidjson::Value &propertyVals = itr->value;
	  if (! propertyVals.IsArray()) {
	    throw JsonParseException("Malformed node ("+id+"): Term property values not array");
	  }
	  for (auto elem = propertyVals.Begin(); elem != propertyVals.End(); elem++) {
	    PropertyValue propval = PropertyValue::of(*elem);
	    term.add_property_value(propval);
	  }
	}
      }
    }
  }
  return term;
}


void
Term::add_definition(const string &def) {
  definition_.assign(def);
}

void
Term::add_definition_xref(const Xref &txref){
  definition_xref_list_.push_back(txref);
}

/**
	* Add a PropertyValue to this term. Some property values
	* encode alt_ids, and we put them into a separate list.
	*/
void
Term::add_property_value(const PropertyValue &pv){
  if (pv.is_alternate_id()){
    TermId alt_id = TermId::of(pv.get_value());
    alternative_id_list_.push_back(alt_id);
  } else {
    property_values_.push_back(pv);
  }
}

std::ostream& operator<<(std::ostream& ost, const Term& term){
  ost << term.label_ << " [" << term.id_ << "]\n";
  ost << "def: " << (term.definition_.empty() ? "n/a":term.definition_) << "\n";
  if (! term.definition_xref_list_.empty()) {
    for (const auto& p : term.definition_xref_list_){
      ost << "\tdef-xref:" << p << "\n";
    }
  }
  if (! term.definition_xref_list_.empty()) {
     for (const auto& p : term.term_xref_list_){
      ost << "\tterm-xref:" << p << "\n";
    }
  }
	if (! term.property_values_.empty()) {
		for (const auto& p : term.property_values_){
		 ost << "\tproperty val:" << p << "\n";
	 }
	}
  return ost;
}

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

std::ostream& operator<<(std::ostream& ost, const Edge& edge){
	ost << edge.source_;
	switch (edge.edge_type_){
		case EdgeType::IS_A: ost << " is_a ";break;
		default: ost << " unknown_edge_type ";
	}
	ost << edge.dest_;
	return ost;
}

Ontology::Ontology(const Ontology &other):
	id_(other.id_),
	property_values_(other.property_values_),
	term_map_(other.term_map_),
	current_term_ids_(other.current_term_ids_),
	obsolete_term_ids_(other.obsolete_term_ids_),
  termid_to_index_(other.termid_to_index_),
  offset_e_(other.offset_e_),
	e_to_(other.e_to_)
	 {
		// no-op
	 }
Ontology::Ontology(Ontology &other): 	id_(other.id_){
	property_values_ = std::move(other.property_values_);
	term_map_ = std::move(other.term_map_);
	current_term_ids_ = std::move(other.current_term_ids_);
  termid_to_index_ = std::move(other.termid_to_index_);
	obsolete_term_ids_ = std::move(other.obsolete_term_ids_);
	e_to_ = std::move(other.e_to_);
  offset_e_ = std::move(other.offset_e_);
}
Ontology&
Ontology::operator=(const Ontology &other){
	if (this != &other) {
		id_ = other.id_;
		property_values_ = other.property_values_;
		term_map_ = other.term_map_;
    termid_to_index_ = other.termid_to_index_;
		current_term_ids_ = other.current_term_ids_;
		obsolete_term_ids_ = other.obsolete_term_ids_;
		e_to_ = other.e_to_;
    offset_e_ = other.offset_e_;
	}
	return *this;
}
Ontology&
Ontology::operator=(Ontology &&other){
	if (this != &other) {
		id_ = std::move(other.id_);
		property_values_ = std::move(other.property_values_);
		term_map_ = std::move(other.term_map_);
		current_term_ids_ = std::move(other.current_term_ids_);
		obsolete_term_ids_ = std::move(other.obsolete_term_ids_);
    termid_to_index_ = std::move(other.termid_to_index_);
		e_to_ = std::move(other.e_to_);
    offset_e_ = std::move(other.offset_e_);
	}
	return *this;
}



void
Ontology::add_property_value(const PropertyValue &propval){
		property_values_.push_back(propval);
}

void
Ontology::add_property(const Property & prop){
	property_list_.push_back(prop);
}

void
Ontology::add_all_terms(const vector<Term> &terms){
  int N = terms.size();
	for (auto t : terms) {
		std::shared_ptr<Term> sptr = std::make_shared<Term>(t);
		TermId tid = t.get_term_id();
		string id = tid.get_id();
		term_map_.insert(std::make_pair(tid,sptr));
		if (t.obsolete()){
			obsolete_term_ids_.push_back(tid);
		} else {
			current_term_ids_.push_back(tid);
		}
		if (t.has_alternative_ids()) {
			vector<TermId> alt_ids = t.get_alternative_ids();
			for (auto atid : alt_ids) {
					term_map_.insert(std::make_pair(atid,sptr));
			}
		}
	}
  std::sort(current_term_ids_.begin(), current_term_ids_.end());
  if (current_term_ids_.size() != N) {
    // sanity check
    // should never ever happen. TODO add exception
    cerr << "[FATAL] Number of term ids not equal to number of terms";
    std::exit(1);
  }
  for (int i=0; i<current_term_ids_.size(); ++i) {
    termid_to_index_[current_term_ids_[i]] = i;
  }
}

/**
	* Figure out how to be more efficient later.
	*/
void
Ontology::add_all_edges(vector<Edge> &edges){
  // First sort the edges on their source element
  // this will mean that edges has the same oder of source
  // TermIds as the current_term_ids_ list.
  std::sort(edges.begin(),edges.end());
  int n_vertices = current_term_ids_.size();
  e_to_.reserve(edges.size());
  offset_e_.reserve(n_vertices+1);
  // We perform two passes
  // In the first pass, we count how many edges emanate from each
  // source
  map<int,int> index2edge_count;
  for (const auto &e : edges) {
    TermId source = e.get_source();
    auto it = termid_to_index_.find(source);
    if (it == termid_to_index_.end()) {
      // sanity check, should never happen unless input file is corrupted
      // todo -- write Exception
      cerr << "[FATAL] could not find TermId for source node:" << source << "\n";
      std::exit(1);
    }
    int idx = it->second;
    auto p = index2edge_count.find(idx);
    if (p == index2edge_count.end()) {
      index2edge_count[idx] = 1; // first edge from this source index
    } else {
      index2edge_count[idx] =  1 + p->second; // increment
    }
  }
  // second pass -- set the offset_e_ according to the number of edges
  // emanating from each source ids.
  offset_e_.push_back(0); // offset of zeroth source TermId is zero
  int offset = 0;
  for (int i=0; i < current_term_ids_.size(); ++i) {
    // these i's are the indices of all of the TermIds
    auto p = index2edge_count.find(i);
    if (p != index2edge_count.end()) {
      int n_edges = p->second;
      offset += n_edges;
    }
    // note if we cannot find anything for i, then the i'th TermId
    // has no outgoing edges
    offset_e_.push_back(offset);
  }
  // third pass -- add the actual edges
  // use the offset variable to keep track of how many edges we have already
  // entered for a given source index
  int current_source_index = -1;
  offset = 0;
  for (const auto &e : edges) {
    TermId source = e.get_source();
    TermId destination = e.get_destination();
    // note we have already checked all of the source id's above
    auto it = termid_to_index_.find(source);
    int source_index = it->second;
    auto p = termid_to_index_.find(destination);
    if (p == termid_to_index_.end()) {
      std::cerr <<"[ERROR] Could not find index of destination TermId " << destination << "\n";
      continue;
    }
    int destination_index = p->second;
    if (source_index != current_source_index) {
      current_source_index = source_index;
      offset = 0; // start a new block
    } else {
      offset++; // go to next index (for a new destination of the previous source)
    }
    //e_to_[source_index + offset] = destination_index;
    e_to_.push_back(destination_index);
  }
  // When we get here, we are done! Print a message
  cout << "[INFO] edges: n=" << e_to_.size()
        << " terms: n=" << n_vertices << "\n";
}

std::optional<Term>
Ontology::get_term(const TermId &tid) const{
	auto p = term_map_.find(tid);
	if (p != term_map_.end()) {
		return *(p->second);
	} else {
		return std::nullopt;
	}
}

vector<TermId>
Ontology::get_isa_parents(const TermId &child) const
{
  vector<TermId> parents;
  auto p = termid_to_index_.find(child);
  if (p == termid_to_index_.end()) {
    // not found, return empty vector
    return parents;
  }
  int idx = p->second;
  for (int i = offset_e_[idx]; i < offset_e_[1+idx]; i++) {
    TermId par = current_term_ids_.at(i);
    parents.push_back(par);
  }
  return parents;
}

std::ostream& operator<<(std::ostream& ost, const Ontology& ontology){
	ost << "### Ontology ###\n"
		<< "id: " << ontology.id_ << "\n";
	for (const auto &pv : ontology.property_values_) {
		ost << "property value: " << pv << "\n";
	}
	ost << "### Terms ###\n"
			<< "total current terms: " << ontology.current_term_count() << "\n"
			<< "total term ids (including obsolete/alternative term ids): " <<
				ontology.total_term_id_count() << "\n";
	ost << "### Edges ###\n"
			<< "total edges: " << ontology.edge_count() << "\n";
	ost << "### Properties ###\n"
			<< "total properties: " << ontology.property_count() << "\n";
		for (auto p : ontology.property_list_) {
			ost << p << "\n";
		}
		return ost;

}
