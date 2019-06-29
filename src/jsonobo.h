#ifndef JSONOBO_H
#define JSONOBO_H

#include <string>
#include <vector>
#include <rapidjson/document.h>
#include "ontology.h"

using std::string;
using std::vector;

class JsonOboParser {
private:
	string path_;
	vector<Term> term_list_;
	vector<Edge> edge_list_;


	void add_node(const rapidjson::Value &val);
	void add_edge(const rapidjson::Value &val);

public:
	JsonOboParser(const string path);



};


#endif
