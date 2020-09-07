/**
 * @file main.cpp
 *
 *  Created on: May 30, 2019
 *  @author: Peter N Robinson
 */

#include <iostream>
#include <string>
#include <memory>

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

int main (int argc, char ** argv) {
  /** Path to HPO ontology file hp.json. */
  string hp_json_path;
  /** Path to phenotype.hpoa file */
  string phenotype_hpoa_path;
  /** Path to mondo file file, mondo.json. */
  string mondo_json_path;
  string phenopacket_path;
  /** Path/name for file with one HPO term per line that we will annotate with top level terms */
  string hpo_termfile;
  /** Path/name for any outputt file. */
  string outpath = "phenotools.out";
  /** A String representing a date, such as 2018-07-21. */
  string iso_date;
   /** A String representing a date, such as 2018-07-21. */
  string iso_date_end;
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
  auto annot_annot_option = annot_command->add_option("-a,--annot",phenotype_hpoa_path,"path to phenotype.hpoa file")->check ( CLI::ExistingFile )->required();
  auto annot_date_option = annot_command->add_option("-d,--date", iso_date, "threshold_date (e.g., 2018-09-23)");
  auto annot_enddate_option = annot_command->add_option("-e,--enddate", iso_date_end, "date for end of period (e.g., 2020-07-23)");
  auto annot_term_option = annot_command->add_option("-t,--term", termid, "TermId (target)");
  auto annot_hp_option = annot_command->add_option("--hp,--ontology",hp_json_path,"path to hp.json or other ontology")->check ( CLI::ExistingFile )->required();
  auto annot_outpath_option = annot_command->add_option("-o,--out", outpath, "name/path for output file" );


  // HPO options
  CLI::App* hpo_command = app.add_subcommand ( "hpo", "Q/C of JSON HP ontology file" );
  CLI::Option* hp_json_path_option = hpo_command->add_option ( "--hp", hp_json_path,"path to  hp.json file" )->check ( CLI::ExistingFile )->required();
  auto hp_stats = hpo_command->add_flag("-s,--stats",show_descriptive_stats,"show descriptive statistics");
  auto hp_qc = hpo_command->add_flag("-q,--qc", show_quality_control, "show quality assessment");
  CLI::Option* date_option = hpo_command->add_option("-d,--date", iso_date, "threshold_date (e.g., 2018-09-23)");
  auto hpo_enddate_option  = hpo_command->add_option("-e,--enddate", iso_date_end, "date for end of period (e.g., 2020-07-23)");
  CLI::Option* term_option = hpo_command->add_option("-t,--term", termid, "TermId (target)");
  auto hpo_debug_option = hpo_command->add_flag("--debug", hpo_debug, "print details of HPO parse" );
  auto hpo_outpath_option = hpo_command->add_option("-o,--out", outpath, "name/path for output file" );
  

  CLI::App* toplevel_command = app.add_subcommand("toplevel", "annotate top level terms from an input file");
  auto toplevel_json_path_option = toplevel_command->add_option ( "--hp", hp_json_path,"path to  hp.json file" )->check ( CLI::ExistingFile );
  auto toplevel_infile_option = toplevel_command->add_option("-i", hpo_termfile, "input file (one HPO term per line)");
  auto topvel_outpath_option = toplevel_command->add_option("-o,--out", outpath, "name/path for output file" );



  CLI11_PARSE ( app, argc, argv );
  std::unique_ptr<PhenotoolsCommand> ptcommand; 

  if ( hpo_command->parsed() ) {
    if (! *hp_json_path_option) {
      cerr << "[ERROR] --hp <path to hp.json> option required for hpo command.\n";
      exit(EXIT_FAILURE);
    }
    if (* hpo_outpath_option) {
      ptcommand = make_unique<HpoCommand>(hp_json_path, 
          show_descriptive_stats, 
          show_quality_control,
          iso_date,
          iso_date_end,
          termid,
          hpo_debug, 
          outpath);
    } else {
      ptcommand = make_unique<HpoCommand>(hp_json_path, 
          show_descriptive_stats, 
          show_quality_control,
          iso_date,
          iso_date_end,
          termid,
          hpo_debug);
    }
  } else if (toplevel_command->parsed()) {
      ptcommand = make_unique<HpoCommand>(hp_json_path, hpo_termfile, outpath);
  }  else if ( annot_command->parsed() ) { 
    if (*annot_outpath_option) {
        ptcommand = make_unique<AnnotationCommand>(phenotype_hpoa_path,
                        hp_json_path, iso_date,  iso_date_end, termid, outpath);
    } else {
       ptcommand = make_unique<AnnotationCommand>(phenotype_hpoa_path,
                        hp_json_path, iso_date,  iso_date_end, termid);
    }
  } else if ( phenopacket_command->parsed() ) {
    // if we get here, then we must have the path to a phenopacket
    if ( ! *phenopacket_path_option ) {
      cerr << "[FATAL] -p/--phenopacket option required!\n";
      return EXIT_FAILURE;
    }
   ptcommand = make_unique<PhenopacketCommand>(phenopacket_path, hp_json_path);
  } else {
    std::cerr << "[ERROR] No command passed. Run with -h option to see usage\n";
    return 1;
  }
   ptcommand->execute();
   return 0;
}
