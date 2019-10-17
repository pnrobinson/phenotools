/**
 * @file ontology.cc
 *
 *  Created on: Sep 13, 2019
 *  @author: Peter N Robinson
 */

#include "ontology.h"
#include <iostream>
#include <utility> // make_pair
#include <algorithm> // sort
#include <stack>


using std::cerr;
using std::cout;



Xref::Xref(const Xref &txr):
  term_id_( txr.term_id_)
{
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

Synonym::Synonym(const string &typ, const string &lbl):
  label_(lbl)
{
  if (typ=="hasExactSynonym") {
    stype_ = SynonymType::EXACT;
  } else if (typ == "hasBroadSynonym") {
    stype_ = SynonymType::BROAD;
  } else if (typ == "hasNarrowSynonym") {
    stype_ = SynonymType::NARROW;
  } else if (typ == "hasRelatedSynonym") {
    stype_ = SynonymType::RELATED;
  } else {
    cerr <<"[WARNING] could not determine synonym type for " << typ << "\n";
  }
}

std::ostream&
operator<<(std::ostream& ost, const Synonym& synonym)
{
  switch(synonym.stype_) {
    case Synonym::SynonymType::EXACT:
      ost << "exact: ";break;
    case Synonym::SynonymType::BROAD:
      ost << "braod: ";break;
    case Synonym::SynonymType::NARROW:
      ost << "narrow: ";break;
    case Synonym::SynonymType::RELATED:
      ost << "related: ";break;
  }
  ost << synonym.label_;
  return ost;
}


Term::Term(const TermId &id, const string &label):
  id_(id),label_(label) {}



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
Term::add_predicate_value(const PredicateValue &pv){
  if (pv.is_alternate_id()){
    TermId alt_id = TermId::from_string(pv.get_value());
    alternative_id_list_.push_back(alt_id);
  } else {
    property_values_.push_back(pv);
  }
}

void
Term::add_synonym(const string &pred, const string &lbl)
{
  synonym_list_.emplace_back(Synonym(pred,lbl));
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
  if (! term.synonym_list_.empty()) {
    ost <<"\tsynonyms:\n";
    for (const auto &s : term.synonym_list_) {
      ost << s << "\n";
    }
  }
  return ost;
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
	predicate_values_(other.predicate_values_),
  property_list_(other.property_list_),
	term_map_(other.term_map_),
	current_term_ids_(other.current_term_ids_),
	obsolete_term_ids_(other.obsolete_term_ids_),
  termid_to_index_(other.termid_to_index_),
  offset_to_edge_(other.offset_to_edge_),
  offset_from_edge_ (other.offset_from_edge_),
	edge_to_(other.edge_to_),
  edge_from_(other.edge_from_)
	 {
		// no-op
	 }
Ontology::Ontology(Ontology &other): 	id_(other.id_){
	predicate_values_ = std::move(other.predicate_values_);
  property_list_ = std::move(other.property_list_);
	term_map_ = std::move(other.term_map_);
	current_term_ids_ = std::move(other.current_term_ids_);
  termid_to_index_ = std::move(other.termid_to_index_);
	obsolete_term_ids_ = std::move(other.obsolete_term_ids_);
	edge_to_ = std::move(other.edge_to_);
  edge_from_ = std::move(other.edge_from_);
  offset_to_edge_ = std::move(other.offset_to_edge_);
  offset_from_edge_ = std::move(other.offset_from_edge_);
}
Ontology&
Ontology::operator=(const Ontology &other){
	if (this != &other) {
		id_ = other.id_;
		predicate_values_ = other.predicate_values_;
    property_list_ = other.property_list_;
		term_map_ = other.term_map_;
    termid_to_index_ = other.termid_to_index_;
		current_term_ids_ = other.current_term_ids_;
		obsolete_term_ids_ = other.obsolete_term_ids_;
		edge_to_ = other.edge_to_;
    edge_from_ = other.edge_from_;
    offset_to_edge_ = other.offset_to_edge_;
    offset_from_edge_ = other.offset_from_edge_;
	}
	return *this;
}
Ontology&
Ontology::operator=(Ontology &&other){
	if (this != &other) {
		id_ = std::move(other.id_);
		predicate_values_ = std::move(other.predicate_values_);
    property_list_ = std::move(other.property_list_);
		term_map_ = std::move(other.term_map_);
		current_term_ids_ = std::move(other.current_term_ids_);
		obsolete_term_ids_ = std::move(other.obsolete_term_ids_);
    termid_to_index_ = std::move(other.termid_to_index_);
		edge_to_ = std::move(other.edge_to_);
    edge_from_ = std::move(other.edge_from_);
    offset_to_edge_ = std::move(other.offset_to_edge_);
    offset_from_edge_ = std::move(other.offset_from_edge_);
	}
	return *this;
}


/**
  * Add a BasicPropetyValue of the ontology
  * These elements are stored in the section "basicPropertyValues"
  * "basicPropertyValues" : [ {
  *  "pred" : "http://purl.org/dc/elements/1.1/creator",
  *  "val" : "Human Phenotype Ontology Consortium"
  *},
  * Note this is distinct from Property elements, that do not describe the
  * ontology but instead are designed to be used to describe other elements
  * (for instance, 'UKL spelling' is used to describe some synonyms).
  */
void
Ontology::add_predicate_value(const PredicateValue &propval){
  predicate_values_.push_back(propval);
}

void
Ontology::add_property(const Property & prop){
  property_list_.push_back(prop);
}

Ontology::Ontology(const string &id,
		   const vector<Term> &terms,
		   vector<Edge> &edges,
		   const vector<PredicateValue> &predicates,
		   const vector<Property> &properties):
  id_(id),
  predicate_values_(predicates),
  property_list_(properties)
{
  add_all_terms(terms);
  add_all_edges(edges);
}

void
Ontology::add_all_terms(const vector<Term> &terms){
  auto N = terms.size();
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
  for (auto i=0u; i<current_term_ids_.size(); ++i) {
    termid_to_index_[current_term_ids_[i]] = i;
  }
}

/**
 * Figure out how to be more efficient later.
 */
void
Ontology::add_all_edges(vector<Edge> &edges){
  // First we add inverse edges for all IS_A edges
  vector<Edge> inverse_edges;
  for (Edge e: edges) {
    if (e.is_is_a()) {
      inverse_edges.emplace_back(e.get_is_a_inverse());
    }
  }
  // now merge
  edges.insert(edges.end(),inverse_edges.begin(), inverse_edges.end());
  // First sort the edges on their source element
  // this will mean that edges has the same oder of source
  // TermIds as the current_term_ids_ list. If the source is the
  // same, sort on the dest
  std::sort(edges.begin(),edges.end(),[](const Edge &a, const Edge &b) {
					return a.get_source() == b.get_source() ?
					  a.get_destination() < b.get_destination() :
					  a.get_source() < b.get_source();
				      });
  int n_vertices = current_term_ids_.size();
  int number_isa_edges = std::count_if(edges.begin(), edges.end(), [](Edge e){return e.is_is_a();});
  int total_edge_count = edges.size() + number_isa_edges; // add this for the inverse edges
  edge_to_.reserve(total_edge_count);
  edge_from_.reserve(edges.size());
  edge_type_list_.reserve(edges.size());
  offset_to_edge_.reserve(n_vertices+1);
  offset_from_edge_.reserve(n_vertices+1);
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
  // second pass -- set the offset_to_edge_ according to the number of edges
  // emanating from each source ids.
  offset_to_edge_.push_back(0); // offset of zeroth source TermId is zero
  int offset = 0;
  for (auto i=0u; i < current_term_ids_.size(); ++i) {
    // these i's are the indices of all of the TermIds
    auto p = index2edge_count.find(i);
    if (p != index2edge_count.end()) {
      int n_edges = p->second;
      offset += n_edges;
    }
    // note if we cannot find anything for i, then the i'th TermId
    // has no outgoing edges
    offset_to_edge_.push_back(offset);
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
    //edge_to_[source_index + offset] = destination_index;
    edge_to_.push_back(destination_index);
    edge_type_list_.push_back(e.get_edge_type());
    if (e.is_is_a()) {
      is_a_edge_count_++;
    }
  }
  // When we get here, we are done! Print a message
  cout << "[INFO] Done parsing edges: n=" << edge_to_.size() << __FILE__ << ":"<<__LINE__
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
  for (int i = offset_to_edge_[idx]; i < offset_to_edge_[1+idx]; i++) {
    if (edge_type_list_[i] != EdgeType::IS_A) { 
      continue;
    }
    int next_node = edge_to_[i];
    TermId par = current_term_ids_.at(next_node);
    parents.push_back(par);
  }
  return parents;
}

/**
 * This function checks whether there is a path of IS_A links that starts
 * at source and ends at dest.
 * */
bool
Ontology::exists_path(const TermId &source, const TermId &dest) const
{
  auto p = termid_to_index_.find(source);
  if (p == termid_to_index_.end()) {
    // not found
    // should never happen, todo return exception
    return false;
  }
  std::stack<int> st;
  int index = p->second;
  p = termid_to_index_.find(dest);
  if (p == termid_to_index_.end()) {
    // not found
    // should never happen, todo return exception
    return false;
  }
  int dest_idx = p->second;
  st.push(index);
  while (! st.empty()) {
    index = st.top();
    st.pop();
    for (int i = offset_to_edge_[index]; i < offset_to_edge_[1+index]; i++) {
      int next_node = edge_to_[i];
      if (edge_type_list_[i] != EdgeType::IS_A) {
        continue;
      }
      if (next_node == dest_idx) {
        return true;
      }
      st.push(next_node);
    }
  }
  // if we get here, there was not path from source to dest
  return false;
}


bool
Ontology::exists_path(const TermId &source, const TermId &dest, EdgeType etype) const
{
  auto p = termid_to_index_.find(source);
  if (p == termid_to_index_.end()) {
    // not found
    // should never happen, todo return exception
    return false;
  }
  std::stack<int> st;
  int index = p->second;
  p = termid_to_index_.find(dest);
  if (p == termid_to_index_.end()) {
    // not found
    // should never happen, todo return exception
    return false;
  }
  int dest_idx = p->second;
  st.push(index);
  while (! st.empty()) {
    index = st.top();
    st.pop();
    for (int i = offset_to_edge_[index]; i < offset_to_edge_[1+index]; i++) {
      int next_node = edge_to_[i];
      if (etype != edge_type_list_[i]){
        continue; // only follow path of indicated edge type
      }
      if (next_node == dest_idx) {
        return true;
      }
      st.push(next_node);
    }
  }
  // if we get here, there was not path from source to dest
  return false;
}




bool
Ontology::have_common_ancestor(const TermId &t1, const TermId &t2, const TermId &root) const
{
  auto p = termid_to_index_.find(t1);
  if (p == termid_to_index_.end()) {
    // not found, should never happen
    throw PhenopacketException("Unrecognized TermId: " + t1.get_value());
  }
  int t1_index = p->second;
  p = termid_to_index_.find(t2);
  if (p == termid_to_index_.end()) {
    // not found, should never happen
    throw PhenopacketException("Unrecognized TermId: " + t1.get_value());
  }
  int t2_index = p->second;
  p = termid_to_index_.find(root);
  if (p == termid_to_index_.end()) {
    // not found, should never happen
    throw PhenopacketException("Unrecognized TermId: " + t1.get_value());
  }
  int root_index = p->second;
  std::stack<int> st;
  std::set<int> t1_ancestors;
  st.push(t1_index);
  while (! st.empty()) {
    int index = st.top();
    if (index == root_index) {
      break;
    }
    t1_ancestors.insert(index);
    st.pop();
    for (int i = offset_to_edge_[index]; i < offset_to_edge_[1+index]; i++) {
      st.push(i);
    }
  }
  // when we get here, t1_ancestors has the indices of t1 and all its ancestors
  std::stack<int> st2;
  st2.push(t2_index);
  while (! st2.empty()) {
    int index = st.top();
    if (index == root_index) {
      break;
    }
    if (t1_ancestors.find(index) != t1_ancestors.end()) {
      return true;
    }
    st2.pop();
    for (int i = offset_to_edge_[index]; i < offset_to_edge_[1+index]; i++) {
      st2.push(i);
    }
  }
  // if we get here, there was no common ancestor
  return false;
}

std::ostream& operator<<(std::ostream& ost, const Ontology& ontology){
	ost << "### Ontology ###\n"
		<< "id: " << ontology.id_ << "\n";
	for (const auto &pv : ontology.predicate_values_) {
		ost << "\t" << pv << "\n";
	}
	ost << "### Terms ###\n"
			<< "total current terms: " << ontology.current_term_count() << "\n"
			<< "total term ids (including obsolete/alternative term ids): " <<
				ontology.total_term_id_count() << "\n";
  int is_a_count = ontology.is_a_edge_count();
  int other_edge_count = ontology.edge_count() - 2 * is_a_count;
	ost << "### Edges ###\n"
			<< "is_a edges: " << is_a_count << "\n";
  if (other_edge_count) {
    ost << "other edges n=" << other_edge_count << "\n";
  }
	ost << "### Properties ###\n"
			<< "Property count: " << ontology.property_count() << "\n";
	for (auto p : ontology.property_list_) {
			ost << p << "\n";
	}
		return ost;
}

/**
  * Print out lots of details for helping with debugging and testing.
  */
void
Ontology::debug_print() const
{
  cout << "List of all edges";
  cout << "current_term_ids_ size="<<current_term_ids_.size()<<"\n";
  cout << "edge_type_list_ size= " << edge_type_list_.size()<<"\n";
  for (auto i=0u; i<offset_to_edge_.size()-1;i++) {
    for (int j=offset_to_edge_[i]; j<offset_to_edge_[i+1]; j++) {
      cout << "i="<<i <<", j="<<j<<", offset_to_edge_.size()="<<offset_to_edge_.size()<<"\n";
      int dest_idx = edge_to_.at(j);
      TermId source_id = current_term_ids_.at(i);
      TermId dest_id = current_term_ids_.at(dest_idx);
      // if we get here, the following "has to" work
      auto source = term_map_.at(source_id);
      auto dest = term_map_.at(dest_id);
      EdgeType etype = edge_type_list_[j];
      cout <<"i="<<i <<": " << source->get_label() << " [" << source_id << "]";
      if (etype == EdgeType::IS_A) {
        cout << " is_a ";
      } else {
        cout << " todo-edge ";
      }
        cout << dest->get_label() << " [" << dest_id << "]\n";
    }
  }
  cout <<"begins at e_to[offset_e[v]] and ends at e_to[offset_e[v+1]]-1.\n";
  for (auto i=0u; i<current_term_ids_.size(); i++) {
    cout <<"current_term_ids_["<<i<<"]= " << current_term_ids_[i]<<"\n";
  }
  for (auto i=0u; i<offset_to_edge_.size();i++) {
    cout << "offset_to_edge_["<<i <<"]= " << offset_to_edge_[i] <<"\n";
  }
  //
  for (auto i=0u; i< edge_type_list_.size();i++) {
    EdgeType et = edge_type_list_.at(i);
    cout << i<< ") edgetype= " << (et == EdgeType::IS_A?"isa":"not isa") <<"\n";
  }
  cout << "offset_e stores offsets into e_to that indicate where the adjacency lists begin. "
    << "The list for an arbitrary vertex begins at e_to[offset_e[v]] and ends at "
  << "  e_to[offset_e[v+1]]-1.\n";
  cout << "offset_to_edge_\n";
  for (auto i=0u; i < offset_to_edge_.size(); i++) {
    cout << "\t" << offset_to_edge_[i] << "\n";
  }
  cout << "edge_to_\n";
  for (auto i=0u; i < edge_to_.size(); i++) {
    cout << "\t" << edge_to_[i] << "\n";
  }

}
