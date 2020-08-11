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



#include "CLI11.hpp"


// the commands
#include "phenotoolscommand.h"
#include "hpocommand.h"
#include "annotcommand.h"

using std::string;
using std::cout;
using std::cerr;
using std::make_unique;
using namespace phenotools;

// prototypes
void print_validation(const std::vector<phenotools::Validation> validation_items, std::string message = "Validation");

int main (int argc, char ** argv) {
  /** Path to HPO ontology file hp.json. */
  string hp_json_path;
  /** Path to phenotype.hpoa file */
  string phenotype_hpoa_path;
  /** Path to mondo file file, mondo.json. */
  string mondo_json_path;
  string phenopacket_path;
  /** A String representing a date, such as 2018-07-21. */
  string iso_date;
  /** A string representing the target TermId */
  string termid;
  bool show_descriptive_stats = false;
  bool show_quality_control = false;
  bool omim_analysis = false; 
  bool hpo_debug = false;

  CLI::App app ( "phenotools" );
  // phenopacket options
  CLI::App* phenopacket_command = app.add_subcommand ( "phenopacket", "work with GA4GH phenopackets" );
  CLI::Option* phenopacket_path_option = phenopacket_command->add_option ( "-p,--phenopacket",phenopacket_path,"path to input phenopacket" )->check ( CLI::ExistingFile );
  CLI::Option* phenopacket_hp_option = phenopacket_command->add_option ( "--hp",hp_json_path,"path to hp.json file" )->check ( CLI::ExistingFile );
  // annotation options
  auto annot_command = app.add_subcommand("annotation", "work with phenotype.hpoa");
  auto annot_annot_option = annot_command->add_option("-a,--annot",phenotype_hpoa_path,"path to mondo.json file");
  auto annot_date_option = annot_command->add_option("-d,--date", iso_date, "threshold_date (e.g., 2018-09-23)");
  auto annot_term_option = annot_command->add_option("-t,--term", termid, "TermId (target)");
  auto annot_hp_option = annot_command->add_option("--hp,--ontology",hp_json_path,"path to hp.json or other ontology")->check ( CLI::ExistingFile );
  // HPO options
  CLI::App* hpo_command = app.add_subcommand ( "hpo", "Q/C of JSON HP ontology file" );
  CLI::Option* hp_json_path_option = hpo_command->add_option ( "--hp", hp_json_path,"path to  hp.json file" )->check ( CLI::ExistingFile );
  auto hp_stats = hpo_command->add_flag("-s,--stats",show_descriptive_stats,"show descriptive statistics");
  auto hp_qc = hpo_command->add_flag("-q,--qc", show_quality_control, "show quality assessment");
  CLI::Option* date_option = hpo_command->add_option("-d,--date", iso_date, "threshold_date (e.g., 2018-09-23)");
  CLI::Option* term_option = hpo_command->add_option("-t,--term", termid, "TermId (target)");
  auto hpo_debug_option = hpo_command->add_flag("--debug", hpo_debug, "print details of HPO parse" );




  CLI11_PARSE ( app, argc, argv );

  if ( hpo_command->parsed() ) {
    if (! *hp_json_path_option) {
      cerr << "[ERROR] --hp <path to hp.json> option required for hpo command.\n";
      exit(EXIT_FAILURE);
    }
    std::unique_ptr<PhenotoolsCommand> hpo = make_unique<HpoCommand>(hp_json_path, 
          show_descriptive_stats, 
          show_quality_control,
          iso_date,
          termid,
          hpo_debug);
    hpo->execute();
  } else if ( annot_command->parsed() ) { 
    std::unique_ptr<PhenotoolsCommand> annot = make_unique<AnnotationCommand>(phenotype_hpoa_path,
                        hp_json_path, iso_date, termid);
    annot->execute();
  } else if ( phenopacket_command->parsed() ) {
    // if we get here, then we must have the path to a phenopacket
    if ( ! *phenopacket_path_option ) {
      cerr << "[FATAL] -p/--phenopacket option required!\n";
      return EXIT_FAILURE;
    }
   std::unique_ptr<PhenotoolsCommand> ppcommand = make_unique<PhenopacketCommand>(phenopacket_path, hp_json_path);
    ppcommand->execute();
    
  } else {
    std::cerr << "[ERROR] No command passed. Run with -h option to see usage\n";
  }
  /*else if (mondo_app->parsed()) {
    // run the MONDO mode
    if (! *mondo_json_option) {
      cerr << "[ERROR] mondo command requires -j/--json <path to mondo.json> option.\n";
      exit(EXIT_FAILURE);
    }
    JsonOboParser parser{mondo_json_path};
    parser.output_quality_assessment();
    std::unique_ptr<Ontology>  ontology = parser.get_ontology();
    Mondo mondo{std::move(ontology)};
    if (mondo_omim_flag) {
      mondo.omim_stats();
    }
    ontology->output_descriptive_statistics();
  } else {
    std::cerr << "[ERROR] No command passed. Run with -h option to see usage\n";
  }
  */
}



void print_validation(const std::vector<phenotools::Validation> validation_items, std::string message)
{
  std::cout << message << "\n";
  for (auto v : validation_items) {
    std::cout << v << "\n";
  }
}
