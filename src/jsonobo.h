#ifndef JSONOBO_H
#define JSONOBO_H

#include <string>
#include <vector>
#include <memory>
#include <rapidjson/document.h>
#include "ontology.h"
#include "property.h"

using std::string;
using std::vector;
using std::unique_ptr;

class JsonOboParser {
private:
	string path_;
	/** The identifier of the ontology we are parsing. */
	string ontology_id_;
	/** List of all terms (classes) of the ontlogy we are parsing. */
	vector<Term> term_list_;
	/** List of all edges of the ontology we are parsing. */
	vector<Edge> edge_list_;
	/** List of all property values of the ontology we are parsing. */
	vector<PredicateValue> property_value_list_;
	/** A list of errors, if any, encountered while parsing the input file.*/
	vector<string> error_list_;

	Ontology ontology_;

	void process_metadata(const rapidjson::Value &val);
	/** Ingest vertices from the JSON file */
	void process_nodes(const rapidjson::Value& nodes);
	/** Ingest edges from the JSON file */
	void process_edges(const rapidjson::Value& edges);

	void transform_input_data_to_ontology();
	PredicateValue json_to_property_value(const rapidjson::Value &val);
	Property json_to_property(const rapidjson::Value &val);
	Term json_to_term(const rapidjson::Value &val);
	Xref json_to_xref(const rapidjson::Value &val);

public:
	JsonOboParser(const string path);
	~JsonOboParser(){}
	/** Transform the nodes and edges into an Ontology object
			with CLR graph. When this method is called, the CTOR
			has ingested data to the term_list and the edge_list.*/
	Ontology get_ontology();

	void dump_errors() const;
};


#endif
