/**
 * @file jsonobo.h
 * @brief Leverage rapidjson to parse JSON files of phenotype ontologies.
 * @author Peter N Robinson
 * @date  Created on: Sep 13, 2019
 */

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
	/** Path to the input JSON file. */
	string path_;
	/** The identifier of the ontology we are parsing. */
	string ontology_id_;
	/** List of all terms (classes) of the ontlogy we are parsing. */
	vector<Term> term_list_;
	/** List of all edges of the ontology we are parsing. */
	vector<Edge> edge_list_;
	/** List of all property values of the ontology we are parsing. */
	vector<PredicateValue> predicate_value_list_;

	vector<Property> property_list_;
	/** A list of errors, if any, encountered while parsing the input file.*/
	vector<string> error_list_;

	//Ontology ontology_;

	void process_metadata(const rapidjson::Value &val);
	/** Ingest vertices from the JSON file */
	void process_nodes(const rapidjson::Value& nodes);
	/** Ingest edges from the JSON file */
	void process_edges(const rapidjson::Value& edges);

	PredicateValue json_to_predicate_value(const rapidjson::Value &val);
	Property json_to_property(const rapidjson::Value &val);
	Term json_to_term(const rapidjson::Value &val);
	Xref json_to_xref(const rapidjson::Value &val);

public:
	JsonOboParser(const string path);
	~JsonOboParser(){}
	/** Transform the nodes and edges into an Ontology object
			with CLR graph. When this method is called, the CTOR
			has ingested data to the term_list and the edge_list.*/
	std::unique_ptr<Ontology> get_ontology();

	void dump_errors() const;
};


#endif
