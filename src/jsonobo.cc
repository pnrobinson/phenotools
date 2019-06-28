
#include "jsonobo.h"


#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <iostream>
#include <fstream>

static const char* kTypeNames[] =
{ "Null", "False", "True", "Object", "Array", "String", "Number" };


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
JsonOboParser::add_node(const rapidjson::Value &val){
	//printJ(val);
	string id;
	string label;
	if (! val.IsObject()) {
		std::cerr << "[ERROR] Attempt to add malformed node (not JSON object). Skipping...\n";
		return;
	}
	if (! val.HasMember("type")) {
		std::cerr << "[ERROR] Attempt to add malformed node (no type information). Skipping...\n";
		return;
	} else if (strcmp ( val["type"].GetString(),"CLASS") ) {
		std::cerr << "[ERROR] Attempt to add malformed node (not a CLASS). Skipping...\n";
		return;
	}
	if (! val.HasMember("id")) {
		std::cerr << "[ERROR] Attempt to add malformed node (no id). Skipping...\n";
		return;
	} else {
		id = val["id"].GetString();
		size_t pos = id.find_last_of("/");
		if (pos != string::npos) {
			id = id.substr(pos+1);
		}
		pos = id.find_first_of("_");
		if (pos != string::npos) {
			id[pos]=':';
		}
	}
	if (! val.HasMember("lbl")) {
		std::cerr << "[ERROR] Attempt to add malformed node (no label). Skipping...\n";
		return;
	} else {
		label = val["lbl"].GetString();
	}
	Term term{id,label};
	if (! val.HasMember("meta")) {
		std::cerr << "[WARN] Term object lacks Meta\n";
	} else {
		const rapidjson::Value &meta = val["meta"];
		if (! meta.IsObject()) {
			std::cerr << "[ERROR] Attempt to add malformed node (meta is not JSON object). Skipping...\n";
			return;
		}
		rapidjson::Value::ConstMemberIterator itr = meta.FindMember("definition");
		if (itr != meta.MemberEnd()) {
			const rapidjson::Value &definition = meta["definition"];

			//printJ(definition);
			rapidjson::Value::ConstMemberIterator it = definition.FindMember("val");
			if (it != definition.MemberEnd()) {
				string definition_value = it->value.GetString();
			//	std::cout << "ading " << definition_value << "\n";
				term.add_definition(definition_value);
			}
			it = definition.FindMember("xrefs");
			if (it != definition.MemberEnd()) {
				const rapidjson::Value& xrefs = it->value;
				if (! xrefs.IsArray()) {
					std::cerr << "[FATAL] xref not array\n";
					std::exit(EXIT_FAILURE);
				}
				for (rapidjson::Value::ConstValueIterator xrefs_itr = xrefs.Begin(); xrefs_itr != xrefs.End(); ++xrefs_itr)
    			term.add_definition_xref(xrefs_itr->GetString());
			}
			itr = meta.FindMember("comments");
			if (itr != meta.MemberEnd()) {
				const rapidjson::Value &comments = itr->value;
				if (! comments.IsArray()) {
					std::cerr << "[ERROR] Comments not array\n";
				} else {
					for (auto iterator = comments.Begin();iterator != comments.End(); iterator++) {
						std::cout << "Comment" << iterator->GetString() << "\n"; std::exit(90);
					}
				}

			}
		}


std::cout << term <<"\n";
			//	std::cerr << "META";//itr->value.GetString() << "\n";



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
		std::cerr << "[FATAL] Ontology JSON did not contain graphs element array\n";
		std::exit(EXIT_FAILURE);
	}
	// the first item in the array is an object with a list of nodes
	if( a.Size() < 1){
		std::cerr << "[FATAL] Ontology JSON did not contain array of nodes\n";
		std::exit(EXIT_FAILURE);
	}
	const rapidjson::Value& mainObject = a[0];
	if ( ! mainObject.IsObject()) {
		std::cerr << "[FATAL] Main object was not object\n";
		std::exit(EXIT_FAILURE);
	}
	const rapidjson::Value& nodes = mainObject["nodes"];
	if (! nodes.IsArray()) {
		std::cerr << "[FATAL] Ontology nodes not array\n";
		std::exit(EXIT_FAILURE);
	}



	for (auto& v : nodes.GetArray()) {
		add_node(v);
	}



//	for (rapidjson::Value::ConstMemberIterator itr = nodes.MemberBegin();
//		 itr != nodes.MemberEnd(); ++itr)
//	{
//		std::cout << "Type of member " <<
//			   itr->name.GetString() << " is " << kTypeNames[itr->value.GetType()] << "\n";
//	}


	//rapidjson::StringBuffer buffer;
	//rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	//d.Accept(writer);

	// Output {"project":"rapidjson","stars":11}
	std::cout << "DONE:" <<  std::endl;
	exit(1);

}
