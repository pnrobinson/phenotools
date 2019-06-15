#define CATCH_CONFIG_MAIN

#include <memory>

#include "catch.hpp"
#include "base.pb.h"
#include "phenopackets.pb.h"
#include "interpretation.pb.h"
#include "phenotools.h"




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
    ontclz.set_label(hpo_label );
    OntologyClass myoc(ontclz);
    vector<Validation> validation = myoc.validate();
   
    REQUIRE( validation.size() == 2 );
    
}
