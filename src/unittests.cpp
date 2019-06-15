#define CATCH_CONFIG_MAIN

#include <memory>

#include "catch.hpp"
#include "base.pb.h"
#include "phenopackets.pb.h"
#include "interpretation.pb.h"
#include "phenotools.h"

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
