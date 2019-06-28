
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
JsonOboParser::add_node(const rapidjson::Value &val){
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
				const rapidjson::Value& defxrefs = it->value;
				if (! defxrefs.IsArray()) {
					throw JsonParseException("xref not array");
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
					throw JsonParseException("Term Xrefs not array");
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
					throw JsonParseException("Term property values not array");
				}
				for (auto elem = propertyVals.Begin(); elem != propertyVals.End(); elem++) {
					PropertyValue propval = PropertyValue::of(*elem);
					term.add_property_value(propval);
				}
			}

			}
		}

    term_list_.push_back(term);

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
    try {
        for (auto& v : nodes.GetArray()) {
            add_node(v);
        }
				std::cout << " Size of nodes array is " << nodes.Size() << "\n";
    } catch (const JsonParseException& e) {
        std::cerr << e.what() << "\n";
				myError = e.what();
    }

		int c=0;
    for (const auto & p : term_list_) {
        std::cout << ++c << ") " << p << "\n";
    }

		std::cout << myError << "\n";
	std::cout << "DONE:" <<  std::endl;

}
