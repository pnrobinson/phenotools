
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
	edge_list_(other.edge_list_)
	 {
		// no-op
	 }
Ontology::Ontology(Ontology &other): 	id_(other.id_){
	property_values_ = std::move(other.property_values_);
	term_map_ = std::move(other.term_map_);
	current_term_ids_ = std::move(other.current_term_ids_);
	obsolete_term_ids_ = std::move(other.obsolete_term_ids_);
	edge_list_ = std::move(other.edge_list_);
}
Ontology&
Ontology::operator=(const Ontology &other){
	if (this != &other) {
		id_ = other.id_;
		property_values_ = other.property_values_;
		term_map_ = other.term_map_;
		current_term_ids_ = other.current_term_ids_;
		obsolete_term_ids_ = other.obsolete_term_ids_;
		edge_list_ = other.edge_list_;
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
		edge_list_ = std::move(other.edge_list_);
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
	std::cout << "Added and size is now "<<property_list_.size() << "\n";
}

void
Ontology::add_all_terms(const vector<Term> &terms){
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
}

/**
	* Figure out how to be more efficient later.
	*/
void
Ontology::add_all_edges(const vector<Edge> &edges){
	edge_list_ = edges;
	int bad=0;
	for (const auto &e : edges) {
		auto src = term_map_.find(e.get_source());
		if (src == term_map_.end() ) {
			std::cerr <<++bad <<") Attempt to add edge with " + e.get_source().get_value() +" not in ontology\n";
		}
		auto dest = term_map_.find(e.get_destination());
		if (dest == term_map_.end() ) {
			std::cerr <<++bad <<") Attempt to add edge with " + e.get_destination().get_value() +" not in ontology\n";
		}
	}
	std::cout << " Could not find " << bad << "\n";
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

std::ostream& operator<<(std::ostream& ost, const Ontology& ontology){
	ost << "### Ontology ###\n"
		<< "id: " << ontology.id_ << "\n";
	for (const auto &pv : ontology.property_values_) {
		ost << pv << "\n";
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
