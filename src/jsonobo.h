#ifndef JSONOBO_H
#define JSONOBO_H

#include <string>
#include <vector>
#include <rapidjson/document.h>

using std::string;

class JsonOboParser {
private:
	string path_;


	void add_node(const rapidjson::Value &val);

public:
	JsonOboParser(const string path);



};


#endif
