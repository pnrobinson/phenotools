/**
 * Tests related to the MetaData element in phenotools.h
 * */

#include "catch.hpp"


#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>

#include "catch.hpp"
#include "../base.pb.h"
#include "../phenopackets.pb.h"
#include "../phenotools.h"
#include "../ontology.h"
#include "../jsonobo.h"
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

using std::cout;
using std::cerr;



TEST_CASE("MetaData","[metadata]") {
  org::phenopackets::schema::v1::core::MetaData mdpb;
  // error -- no data
  // Note that protobuf creates a timestamp automatically and so this is not
  // lacking in our test
  phenotools::MetaData md1(mdpb);
  vector<phenotools::Validation> validation = md1.validate();
  REQUIRE(validation.size()==2);
  phenotools::Validation v = validation.at(0);
  REQUIRE(v.is_error());

  REQUIRE(v.get_cause() == phenotools::ValidationCause::METADATA_LACKS_CREATED_BY);
  v = validation.at(1);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == phenotools::ValidationCause::METADATA_LACKS_RESOURCES);
}

/*
We should identify the following errors. 
[ERROR] [ERROR] Could not find HP:9999999 in the ontology
[ERROR] Metadata did not contain ontology used in phenopacket: NCBITaxon
[WARNING] Metadata contains ontolog not used in Phenopacket: ECO
[WARNING] Metadata contains ontolog not used in Phenopacket: NCBITAXON
[WARNING] Metadata contains ontolog not used in Phenopacket: PATO
*/


TEST_CASE("Check Metadata", "[metadata_has_redundant]") {
  string hp_json_path = "../testdata/hp.small.json";
  JsonOboParser parser {hp_json_path};
  std::unique_ptr<Ontology>  ontology = parser.get_ontology();
  string phenopacket_path = "../testdata/small-phenopacket-1.json";
  std::ifstream inFile;
  
  inFile.open ( phenopacket_path );
  if ( ! inFile.good() ) {
    cerr << "Could not open Phenopacket file at " << phenopacket_path <<"\n";
     exit(1);
  }
  std::stringstream sstr;
  sstr << inFile.rdbuf();
  string JSONstring = sstr.str();
  ::google::protobuf::util::JsonParseOptions options;
  ::org::phenopackets::schema::v1::Phenopacket phenopacketpb;
  ::google::protobuf::util::JsonStringToMessage (JSONstring, &phenopacketpb, options);
  phenotools::Phenopacket ppacket(phenopacketpb);
  // now test if the annotations in the phenpacket are redundant.
  vector<phenotools::Validation> validation = ppacket.semantically_validate(ontology);
  REQUIRE(5 == validation.size());
  phenotools::Validation val = validation.at(0);
  REQUIRE(val.get_cause() == phenotools::ValidationCause::REDUNDANT_ANNOTATION);
  string msg = "[ERROR] Redundant terms: HP:0000003(Fake term 3) is a subclass of HP:0000002(Fake term 2)";
  REQUIRE(msg == val.message());
  REQUIRE(val.is_error());
  val = validation.at(1);
  REQUIRE(val.get_cause() == phenotools::ValidationCause::METADATA_DOES_NOT_CONTAIN_ONTOLOGY);
  REQUIRE(val.is_error());
  val = validation.at(2);
  REQUIRE(val.get_cause() == phenotools::ValidationCause::METADATA_HAS_SUPERFLUOUS_ONTOLOGY);
  REQUIRE(val.is_warning());
  val = validation.at(3);
  REQUIRE(val.get_cause() == phenotools::ValidationCause::METADATA_HAS_SUPERFLUOUS_ONTOLOGY);
  REQUIRE(val.is_warning());
  val = validation.at(4);
  REQUIRE(val.get_cause() == phenotools::ValidationCause::METADATA_HAS_SUPERFLUOUS_ONTOLOGY);
  REQUIRE(val.is_warning());

}