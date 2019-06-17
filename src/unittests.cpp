#define CATCH_CONFIG_MAIN

#include <memory>

#include <iostream>

#include "catch.hpp"
#include "base.pb.h"
#include "phenopackets.pb.h"
#include "interpretation.pb.h"
#include "phenotools.h"

// The arena object is used to allocate certain protobuf objects
google::protobuf::Arena arena;



TEST_CASE("test OntologyClass basics","[ontologyclass]") {
  org::phenopackets::schema::v1::core::OntologyClass ontclz;
  string hpo_id = "HP:0001166";
  string hpo_label = "Arachnodactyly";
  ontclz.set_id(hpo_id);
  ontclz.set_label(hpo_label );
  REQUIRE( ontclz.IsInitialized());
  REQUIRE( ! ontclz.id().empty() );
  REQUIRE(ontclz.id() == hpo_id);
  REQUIRE(ontclz.label() == hpo_label);
}

TEST_CASE("test OntologyClass malformed","[ontologyclass]") {
  org::phenopackets::schema::v1::core::OntologyClass ontclz;
  string hpo_id = "HP0001166";
  string hpo_label = "";
  ontclz.set_id(hpo_id);
  ontclz.set_label(hpo_label);
  OntologyClass myoc(ontclz);
  vector<Validation> validation = myoc.validate();
  // we assume here that the id is tested first (v1) and then the label (v2)
  REQUIRE( validation.size() == 2 );
  Validation v1 = validation.at(0);
  REQUIRE( v1.is_warning() == true );
  REQUIRE( v1.message() == "OntologyClass id (HP0001166) not a valid CURIE" );
  Validation v2 = validation.at(1);
  REQUIRE( v2.is_warning() == false );
  REQUIRE( v2.is_error() == true );
  REQUIRE( v2.message() == "OntologyClass label not allowed to be empty" );
}

TEST_CASE("test Age with string and empty","[age]") {
  org::phenopackets::schema::v1::core::Age agepb;
  string age40 = "P40Y"; // 40 years old
  agepb.set_age(age40);
  Age age(agepb);
  vector<Validation> validation = age.validate();
  REQUIRE( validation.empty() == true );
  agepb.clear_age(); // now the Age protobuf class has neither age nor age_class
  // this is an error
  Age age2(agepb);
  validation = age2.validate();
  REQUIRE( validation.size() == 1 );
  Validation v1 = validation.at(0); 
  REQUIRE( v1.is_error() == true );
  REQUIRE( v1.message() == "At least one of age and age_class must be present in Age element" );
  // Now make an Age that has an ontology class
  string id = "HsapDv:0000236";
  string label = "second decade human stage";
  
  org::phenopackets::schema::v1::core::OntologyClass* c =   google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::OntologyClass>(&arena);
  c->set_id(id);
  c->set_label(label);
  // now we are OK again -- the Age object as an age-class OntologyClass object.
  agepb.set_allocated_age_class(c);
  Age age3(agepb);
  validation = age3.validate();
  REQUIRE( validation.empty() == true );
  agepb.release_age_class();
}

TEST_CASE("AgeRange","[agerange]") {
  org::phenopackets::schema::v1::core::Age* agepb1 =   google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::Age>(&arena);
  string age40 = "P40Y"; // 40 years old
  agepb1->set_age(age40);
  
  org::phenopackets::schema::v1::core::Age* agepb2 =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::Age>(&arena);
  string age50 = "P50Y"; // 40 years old
  agepb2->set_age(age50);
  
  org::phenopackets::schema::v1::core::AgeRange agerangepb;
  agerangepb.set_allocated_start(agepb1); 
  agerangepb.set_allocated_end(agepb2);
  
  AgeRange ar(agerangepb);
  
  vector<Validation> validation = ar.validate();
  REQUIRE(validation.size()==0);
  
  // Now make one of the elements invalid
  agepb1->set_age("");
  agerangepb.release_start();
  agerangepb.set_allocated_start(agepb1);
  AgeRange ar2(agerangepb);
  validation = ar2.validate();
  REQUIRE(validation.size()==1);
  agerangepb.release_start();
  agerangepb.release_end();
}


TEST_CASE("test Individual","[individual]") {
  // All we need to get a valid Individual element is an id
  // if no Sex or Age information is provided, then two
  // warnings are generated.
  org::phenopackets::schema::v1::core::Individual individualpb;
  individualpb.set_id("42");
  
  Individual ind(individualpb);
  vector<Validation> validation = ind.validate();
  REQUIRE(validation.size()==2);
  Validation v1 = validation.at(0);
  REQUIRE(v1.get_cause() == ValidationCause::INDIVIDUAL_LACKS_AGE);
  REQUIRE(v1.is_warning() == true );
  Validation v2 = validation.at(1);
  REQUIRE(v2.get_cause() == ValidationCause::UNKNOWN_SEX);
  REQUIRE(v2.is_warning() == true );
  // Now let's generate an error by trying to create an individual
  // with an id
  individualpb.clear_id();
  Individual ind2(individualpb);
  validation = ind2.validate();
  REQUIRE(validation.size()==3);
  v1 = validation.at(0);
  REQUIRE(v1.get_cause() == ValidationCause::INDIVIDUAL_LACKS_ID);
  REQUIRE(v1.is_error() == true);
}


TEST_CASE("test ExternalReference element","[externalreference]") {
  org::phenopackets::schema::v1::core::ExternalReference extrefpb;
  
  ExternalReference er(extrefpb);
  // no id, this is an error
  vector<Validation> validation = er.validate();
  REQUIRE(validation.size()==1);
  Validation v1 = validation.at(0);
  REQUIRE(v1.is_error()==true);
  REQUIRE(v1.get_cause() == ValidationCause::EXTERNAL_REFERENCE_LACKS_ID);
  // set the id. Then the element is OK.
  extrefpb.set_id("PMID:1234");
  ExternalReference er2(extrefpb);
  validation = er2.validate();
  REQUIRE(validation.size()==0);
}


TEST_CASE("test Evidence","[evidence]") {
  
  org::phenopackets::schema::v1::core::Evidence evipb;
  // error -- Evidence needs to have an Ontology class evidence release_evidence_code
  
  Evidence evi1(evipb);
  vector<Validation> validation = evi1.validate();
  REQUIRE(validation.size()==1);
  Validation v1 = validation.at(0);
  REQUIRE(v1.is_error()==true);
  REQUIRE(v1.get_cause() == ValidationCause::EVIDENCE_LACKS_CODE);
  
  string eco_id = "ECO:0006017";
  string eco_label = "author statement from published clinical study used in manual assertion";
  
  org::phenopackets::schema::v1::core::OntologyClass* eco =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::OntologyClass>(&arena); 
  eco->set_id(eco_id);
  eco->set_label(eco_label);
  // now add an ontology term -- all should be OK
  evipb.set_allocated_evidence_code(eco);
  Evidence evi2(evipb);
  validation = evi2.validate();
  REQUIRE(validation.size()==0);
  evipb.release_evidence_code();  
}

TEST_CASE("test Gene","[gene]") {
  org::phenopackets::schema::v1::core::Gene genepb;
  // error -- no id or symbol
  Gene g1(genepb);
  vector<Validation> validation = g1.validate();
  REQUIRE(validation.size()==2);
  Validation v1 = validation.at(0);
  REQUIRE(v1.is_error()==true);
  REQUIRE(v1.get_cause() == ValidationCause::GENE_LACKS_ID);
  Validation v2 = validation.at(1);
  REQUIRE(v2.is_error()==true);
  REQUIRE(v2.get_cause() == ValidationCause::GENE_LACKS_SYMBOL);
  // error -- have id but no symbol
  genepb.set_id("ENTREZ:7547");
  Gene g2(genepb);
  validation = g2.validate();
  REQUIRE(validation.size()==1);
  v1 = validation.at(0);
  REQUIRE(v1.is_error()==true);
  REQUIRE(v1.get_cause() == ValidationCause::GENE_LACKS_SYMBOL);
  // fix everything
  genepb.set_symbol("ZIC3");
  Gene g3{genepb};
  validation = g3.validate();
  REQUIRE(validation.empty()==true);
  

}
