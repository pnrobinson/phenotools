#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

#include "CLI11.hpp"
#include "phenopackets.pb.h"
#include "base.pb.h"
#include "phenotools.h"
#include "jsonobo.h"



using std::string;
using std::cout;
using std::cerr;





int main(int argc, char ** argv) {


    string hp_json_path;
    string phenopacket_path;

    CLI::App app("phenotools");
    CLI::Option* hp_json_path_option = app.add_option("--hp",hp_json_path,"path to hp.json file")->check(CLI::ExistingFile);
    CLI::Option* phenopacket_path_option = app.add_option("-p,--phenopacket",phenopacket_path,"path to input phenopacket")->check(CLI::ExistingFile);

    CLI11_PARSE(app, argc, argv);

    if (*hp_json_path_option) {
        JsonOboParser parser{hp_json_path};
        cout << "[INFO] Done JSON demo.\n";
        Ontology ontology = parser.get_ontology();
        cout << ontology << "\n";

        


        return EXIT_SUCCESS;
    }
    // if we get here, then we must have the path to a phenopacket
    if (! *phenopacket_path_option) {
        cerr << "[FATAL] -p/--phenopacket option required!\n";
        return EXIT_FAILURE;
    }



  GOOGLE_PROTOBUF_VERIFY_VERSION;

  std::stringstream sstr;
  std::ifstream inFile;
  inFile.open(phenopacket_path);
  if (! inFile.good()) {
    cerr << "Could not open Phenopacket file at " << phenopacket_path <<"\n";
    return EXIT_FAILURE;
  }
  sstr << inFile.rdbuf();
  string JSONstring = sstr.str();
  //cout <<"[INFO] reading phenopacket\n" << JSONstring << "\n";

  ::google::protobuf::util::JsonParseOptions options;
  ::org::phenopackets::schema::v1::Phenopacket phenopacketpb;
  ::google::protobuf::util::JsonStringToMessage(JSONstring,&phenopacketpb,options);
  cout << "\n#### Phenopacket at: " << phenopacket_path << " ####\n\n";

  phenotools::Phenopacket ppacket(phenopacketpb);

  cout << ppacket << "\n";

  auto validation = ppacket.validate();
  if (validation.empty()) {
    cout << "No Q/C issues identified!\n";
  } else {
    auto N = validation.size();
    cout << "#### We identified "
	 << N << " Q/C issue" << (N>1?"s":"")
	 << " ####\n";
    for (auto v : validation) {
      cout << v << "\n";
    }

  }



}
