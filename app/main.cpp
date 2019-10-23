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
#include "../lib/phenopackets.pb.h"
#include "../lib/base.pb.h"
#include "../lib/phenotools.h"
#include "../lib/jsonobo.h"

using std::string;
using std::cout;
using std::cerr;

int main (int argc, char ** argv) {
  /** Path to HPO ontology file hp.json. */
  string hp_json_path;
  /** Path to mondo file file, mondo.json. */
  string mondo_json_path;
  string phenopacket_path;
  bool show_descriptive_stats = false;
  bool show_quality_control = false;

  CLI::App app ( "phenotools" );
  // phenopacket options
  CLI::App* phenopacket_command = app.add_subcommand ( "phenopacket", "work with GA4GH phenopackets" );
  CLI::Option* phenopacket_path_option = phenopacket_command->add_option ( "-p,--phenopacket",phenopacket_path,"path to input phenopacket" )->check ( CLI::ExistingFile );
  CLI::Option* phenopacket_hp_option = phenopacket_command->add_option ( "--hp",hp_json_path,"path to hp.json file" )->check ( CLI::ExistingFile );

  // mondo options
  auto mondo_app = app.add_subcommand("mondo", "work with MONDO");
  auto mondo_json_option = mondo_app->add_option("-j,--json",mondo_json_path,"path to mondo.json file");
  
  
  // HPO options
  CLI::App* hpo_command = app.add_subcommand ( "hpo", "Q/C of JSON HP ontology file" );
  CLI::Option* hp_json_path_option = hpo_command->add_option ( "--hp", hp_json_path,"path to  hp.json file" )->check ( CLI::ExistingFile );
  auto hp_stats = hpo_command->add_flag("-s,--stats",show_descriptive_stats,"show descriptive statistics");
  auto hp_qc = hpo_command->add_flag("-q,--qc", show_quality_control, "show quality assessment");

  // DEBUG OPTIONs
  CLI::App* debug_command = app.add_subcommand ( "debug", "print details of HPO parse" );
  CLI::Option* debug_ont_option = debug_command->add_option("--hp,--ontology",hp_json_path,"path to hp.json or other ontology")->check ( CLI::ExistingFile );




  CLI11_PARSE ( app, argc, argv );

  if ( hpo_command->parsed() ) {
    if (! *hp_json_path_option) {
      cerr << "[ERROR] --hp <path to hp.json> option required for hpo command.\n";
      exit(EXIT_FAILURE);
    }
    JsonOboParser parser{hp_json_path};
    std::unique_ptr<Ontology>  ontology = parser.get_ontology();
    if (show_descriptive_stats) {
      ontology->output_descriptive_statistics();
    }
    if (show_quality_control) {
      parser.output_quality_assessment();
    }
    if (show_descriptive_stats || show_quality_control ) {
      return EXIT_SUCCESS;
    } else {
      cout << "Consider running with -s or -q option to see descriptive stats and Q/C results.\n";
      return EXIT_SUCCESS;
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
    auto validation = ppacket.validate();
    if (*phenopacket_hp_option) {
      JsonOboParser parser {hp_json_path};
      std::unique_ptr<Ontology>  ontology = parser.get_ontology();
      auto semvalidation = ppacket.semantically_validate(ontology);
      validation.insert(validation.end(),semvalidation.begin(), semvalidation.end());
    } else {
      cout << "[INFO] Validation performed without ontology\n";
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
  } else if (mondo_app->parsed()) {
    // run the MONDO mode
    if (! *mondo_json_option) {
      cerr << "[ERROR] mondo command requires -j/--json <path to mondo.json> option.\n";
      exit(EXIT_FAILURE);
    }
    try {
    JsonOboParser parser{mondo_json_path};
    std::unique_ptr<Ontology>  ontology = parser.get_ontology();
    } catch (JsonParseException &e) {
      std::cerr << e.what();
    }
    
  } else {
    std::cerr << "[ERROR] No command passed. Run with -h option to see usage\n";
  }
}
