
#include "jsonobo.h"


#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <iostream>
#include <fstream>

void
printJ(const rapidjson::Value &json)
{
	using namespace rapidjson;

	StringBuffer sb;
	PrettyWriter<StringBuffer> writer(sb);
	json.Accept(writer);
	auto str = sb.GetString();
	std::cout << str << "\n";
}

void
JsonOboParser::add_edge(const rapidjson::Value &val){
	Edge e = Edge::of(val);
	edge_list_.push_back(e);
}

bool is_class(const rapidjson::Value &val){
	if (! val.IsObject()) {
		return false;
	}
	if (! val.HasMember("type")) {
		return false;
	} else if (! strcmp ( val["type"].GetString(),"CLASS") ) {
		return true;
	}
	return false;
}

bool is_property(const rapidjson::Value &val){
	if (! val.IsObject()) {
		return false;
	}
	if (! val.HasMember("type")) {
		return false;
	} else if (! strcmp ( val["type"].GetString(),"PROPERTY") ) {
		std::cout << "TURURUEUEURUEREURERERE";
		return true;
	}
	return false;
}

void
JsonOboParser::add_meta(const rapidjson::Value &val){
	if (! val.IsObject()) {
		throw JsonParseException("Attempt to add malformed meta (not JSON object).");
	}
	auto itr = val.FindMember("basicPropertyValues");
	if (itr != val.MemberEnd()) {
		const rapidjson::Value &propertyVals = itr->value;
		if (! propertyVals.IsArray()) {
			throw JsonParseException("Ontology property values not array");
		}
		for (auto elem = propertyVals.Begin(); elem != propertyVals.End(); elem++) {
			PropertyValue propval = PropertyValue::of(*elem);
			ontology_.add_property_value(propval);
		}
	}
}


JsonOboParser::JsonOboParser(const string path):
path_(path) {
	rapidjson::Document d;
	std::cout << " Parsing " << path_ << "\n";
	std::ifstream ifs(path_);
	rapidjson::IStreamWrapper isw(ifs);

	d.ParseStream(isw);
	const rapidjson::Value& a = d["graphs"];
	if (! a.IsArray()) {
		throw JsonParseException("Ontology JSON did not contain graphs element array.");
	}
	// the first item in the array is an object with a list of nodes
	if( a.Size() < 1){
		throw JsonParseException("Ontology JSON did not contain array of nodes.");
	}
	const rapidjson::Value& mainObject = a[0];
	if ( ! mainObject.IsObject()) {
		throw JsonParseException("Main object was not object.");
	}
	const rapidjson::Value& nodes = mainObject["nodes"];
	if (! nodes.IsArray()) {
		throw JsonParseException("Ontology nodes not array");
	}

	string myError;
	for (auto& v : nodes.GetArray()) {
		if (is_class(v)) {
			try {
				Term term = Term::of(v);
				term_list_.push_back(term);
    	} catch (const JsonParseException& e) {
    		std::cerr << e.what() << "\n";
				myError += e.what();
    	}
		}
		else if (is_property(v)){
			try{
				Property prop = Property::of(v);
				ontology_.add_property(prop);
			} catch (const JsonParseException& e) {
    		std::cerr << e.what() << "\n";
				myError += e.what();
    	}
		}
	}
	rapidjson::Value::ConstMemberIterator itr = mainObject.FindMember("edges");
	if (itr == mainObject.MemberEnd()){
		throw JsonParseException("Did not find edges element");
	}
	const rapidjson::Value& edges = mainObject["edges"];
	for (auto& v : edges.GetArray()) {
		try {
			add_edge(v);
		} catch (const JsonParseException& e) {
				std::cerr << e.what() << "\n";
				myError += e.what() ;
				myError += "\n";
		}
	}

		itr = mainObject.FindMember("id");
		if (itr == mainObject.MemberEnd()){
			throw JsonParseException("Did not find id element");
		} else {
			string id = itr->value.GetString();
			ontology_.set_id(id);
		}
		itr = mainObject.FindMember("meta");
		if (itr == mainObject.MemberEnd()){
			throw JsonParseException("Did not find meta element");
		} else {
			const rapidjson::Value& meta = mainObject["meta"];
			add_meta(meta);
		}


		ontology_.add_all_terms(term_list_);
		ontology_.add_all_edges(edge_list_);

		std::cout << myError << "\n";
	std::cout << "DONE:" <<  std::endl;

}
