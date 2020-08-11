
#include "phenotoolscommand.h"
#include <iostream>



#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

#include "../lib/phenopackets.pb.h"
#include "../lib/base.pb.h"
#include "../lib/phenotools.h"
#include "../lib/jsonobo.h"

using namespace phenotools;
using std::cerr;
using std::cout;
using std::vector;

PhenotoolsCommand::PhenotoolsCommand()
{
    // no -op
}


struct tm 
PhenotoolsCommand::string_to_time(string iso8601date) const
{
    tm time;
    int y,M,d,h,m;
    float s;
    sscanf(iso8601date.c_str(), "%d-%d-%dT%d:%d:%fZ", &y, &M, &d, &h, &m, &s);
    time.tm_year = y - 1900; // Year since 1900
    time.tm_mon = M - 1;     // 0-11
    time.tm_mday = d;        // 1-31
    return time;
}



 PhenopacketCommand::PhenopacketCommand(const string &phenopacket, const string &hp_json):
    hp_json_path(hp_json),
    phenopacket_path(phenopacket)
 {

 }

 int
PhenopacketCommand::execute() 
{
    std::ifstream inFile;
    inFile.open ( phenopacket_path );
    if ( ! inFile.good() ) {
      cerr << "Could not open Phenopacket file at " << phenopacket_path <<"\n";
      return EXIT_FAILURE;
    }
    GOOGLE_PROTOBUF_VERIFY_VERSION;
   
    std::stringstream sstr;
    sstr << inFile.rdbuf();
    string JSONstring = sstr.str();
    ::google::protobuf::util::JsonParseOptions options;
    ::org::phenopackets::schema::v1::Phenopacket phenopacketpb;
    ::google::protobuf::util::JsonStringToMessage (JSONstring, &phenopacketpb, options);
    cout << "\n#### Phenopacket at: " << phenopacket_path << " ####\n\n";
    phenotools::Phenopacket ppacket(phenopacketpb);
    auto validation = ppacket.validate();
    if (hp_json_path.empty()) {
        cout << "[INFO] path to hp.json not passed, skipping ontology-based validation.\n";
        return 0;
    }
    JsonOboParser parser{hp_json_path};
    vector<string> error_list = parser.get_errors();
    if (! error_list.empty()) {
        cerr << "[ERROR] Unable to parse ontology:\n:";
        for (string e : error_list) {
            cerr << "\t" << e << "\n";
        }
        exit(1);
    }
    std::unique_ptr<Ontology>  ontology = parser.get_ontology();
    auto semvalidation = ppacket.semantically_validate(ontology);
    validation.insert(validation.end(),semvalidation.begin(), semvalidation.end());
    
    if ( validation.empty() ) {
      cout << "No Q/C issues identified!\n";
    } else {
      auto N = validation.size();
      cout << "#### We identified "
	        << N << " Q/C issue" << ( N>1?"s":"" )
	        << " ####\n";
      for ( auto v : validation ) {
	       cout << v << "\n";
      }

    }
    return 0;
}