/**
 * @file main.cpp
 *
 *  Created on: May 30, 2019
 *  @author: Peter N Robinson
 */

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

int main (int argc, char ** argv) {
  /** Path to HPO ontology file hp.json. */
  string hp_json_path;
  /** Path to mondo file file, mondo.json. */
  string mondo_json_path;
  string phenopacket_path;

  CLI::App app ( "phenotools" );
  // phenopacket options
  CLI::App* phenopacket_command = app.add_subcommand ( "phenopacket", "work with GA4GH phenopackets" );
  CLI::Option* phenopacket_path_option = phenopacket_command->add_option ( "-p,--phenopacket",phenopacket_path,"path to input phenopacket" )->check ( CLI::ExistingFile );
  //CLI::Option* phenopacket_hp_option = phenopacket_command->add_option ( "-h,--hp",phenopacket_path,"path to hp.json file" )->check ( CLI::ExistingFile );

  // validate options
  CLI::App* validate_command = app.add_subcommand ( "validate", "perform Q/C of JSON ontology file (HP,MP,MONDO,GO)" );
  CLI::Option* hp_json_path_option = app.add_option ( "--hp", hp_json_path,"path to  hp.json file" )->check ( CLI::ExistingFile );
  CLI::Option* mondo_json_path_option = validate_command->add_option ( "--mondo", mondo_json_path,"path to  mondo.json file" )->check ( CLI::ExistingFile );

  // DEBUG OPTIONs
  CLI::App* debug_command = app.add_subcommand ( "debug", "print details of HPO parse" );
  CLI::Option* debug_ont_option = debug_command->add_option("-o,--ontology",hp_json_path,"path to hp.json or other ontology")->check ( CLI::ExistingFile );


  CLI11_PARSE ( app, argc, argv );

  if ( validate_command->parsed() ) {
    if (*hp_json_path_option) {
      JsonOboParser parser{hp_json_path};
      std::unique_ptr<Ontology>  ontology = parser.get_ontology();
      cout << *ontology << "\n";
      return EXIT_SUCCESS;
    } else if (*mondo_json_path_option) {
      JsonOboParser parser{mondo_json_path};
      std::unique_ptr<Ontology> ontology = parser.get_ontology();
      cout << *ontology << "\n";
      return EXIT_SUCCESS;
    } else {
      std::cerr << "[ERROR] --hp or --mondo option required for validate command!\n";
    }
  } else if (debug_command->parsed() ) {
    if (*debug_ont_option) {
      //TODO REMOVE AFTER DEV
      JsonOboParser parser {hp_json_path};
      std::unique_ptr<Ontology>  ontology = parser.get_ontology();
      cout << *ontology << "\n";
      ontology->debug_print();
    } else {
      cerr << "debug command required -o option.\n";
    }
  } else if ( phenopacket_command->parsed() ) {
    // if we get here, then we must have the path to a phenopacket
    if ( ! *phenopacket_path_option ) {
      cerr << "[FATAL] -p/--phenopacket option required!\n";
      return EXIT_FAILURE;
    }
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    std::ifstream inFile;
    inFile.open ( phenopacket_path );
    if ( ! inFile.good() ) {
      cerr << "Could not open Phenopacket file at " << phenopacket_path <<"\n";
      return EXIT_FAILURE;
    }
    std::stringstream sstr;
    sstr << inFile.rdbuf();
    string JSONstring = sstr.str();
    ::google::protobuf::util::JsonParseOptions options;
    ::org::phenopackets::schema::v1::Phenopacket phenopacketpb;
    ::google::protobuf::util::JsonStringToMessage (JSONstring, &phenopacketpb, options);
    cout << "\n#### Phenopacket at: " << phenopacket_path << " ####\n\n";

    phenotools::Phenopacket ppacket(phenopacketpb);

    cout << ppacket << "\n";

    auto validation = ppacket.validate();
    if ( *hp_json_path_option ) {
      JsonOboParser parser {hp_json_path};
      std::unique_ptr<Ontology>  ontology = parser.get_ontology();
      auto semantic_validation = ppacket.semantically_validate(ontology);
    }
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
  } else {
    std::cerr << "[ERROR] No command passed. Run with -h option to see usage\n";
  }
}
