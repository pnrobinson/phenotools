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
  
  org::phenopackets::schema::v1::core::OntologyClass* c =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::OntologyClass>(&arena);
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
  org::phenopackets::schema::v1::core::Age* agepb1 =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::Age>(&arena);
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


TEST_CASE("Test HGVS allele","[hgvsallele]"){
  org::phenopackets::schema::v1::core::HgvsAllele hgvspb;
  // error -- no id or hgvs
  HgvsAllele allele1(hgvspb);
  vector<Validation> validation = allele1.validate();
  REQUIRE(validation.size()==2);
  Validation v1 = validation.at(0);
  REQUIRE(v1.is_warning()==true);
  REQUIRE(v1.get_cause() == ValidationCause::ALLELE_LACKS_ID);
  Validation v2 = validation.at(1);
  REQUIRE(v2.is_error()==true);
  REQUIRE(v2.get_cause() == ValidationCause::ALLELE_LACKS_HGVS);
  // add the HGVS -- there should be only a warning now
  hgvspb.set_hgvs("NM_000276.3:c.2581G>A");
  HgvsAllele allele2(hgvspb);
  validation = allele2.validate();
  REQUIRE(validation.size()==1);
  v1 = validation.at(0);
  REQUIRE(v1.is_warning()==true);
  REQUIRE(v1.get_cause() == ValidationCause::ALLELE_LACKS_ID);
  // add an ID -- there should now be no warning
  hgvspb.set_id("id:A");
  HgvsAllele allele3(hgvspb);
  validation = allele3.validate();
  REQUIRE(validation.empty()==true);
}


TEST_CASE("Test VcfAllele","[vcfallele]") {
  org::phenopackets::schema::v1::core::VcfAllele vcfpb;
  // error -- no data
  VcfAllele allele1(vcfpb);
  vector<Validation> validation = allele1.validate();
  REQUIRE(validation.size()==5);
  Validation v = validation.at(0);
  REQUIRE(v.is_warning()==true);
  REQUIRE(v.get_cause() == ValidationCause::ALLELE_LACKS_ID);
  v = validation.at(1);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == ValidationCause::LACKS_GENOME_ASSEMBLY);
  v = validation.at(2);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == ValidationCause::LACKS_CHROMOSOME);
  v = validation.at(3);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == ValidationCause::LACKS_REF);
  v = validation.at(4);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == ValidationCause::LACKS_ALT);
  // Now let's make a well formed VCF
  vcfpb.set_id("id:A");
  vcfpb.set_genome_assembly("GRCh37");
  vcfpb.set_chr("chr1");
  vcfpb.set_pos(42);
  vcfpb.set_ref("A");
  vcfpb.set_alt("C");
  VcfAllele allele2(vcfpb);
  validation = allele2.validate();
  REQUIRE(validation.empty()==true);
}


TEST_CASE("Test Variant","[variant]"){
  org::phenopackets::schema::v1::core::Variant variantpb;
  // error -- no data
  Variant var1(variantpb);
  vector<Validation> validation = var1.validate();
  REQUIRE(validation.size()==2);
  Validation v = validation.at(0);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == ValidationCause::LACKS_ALLELE);
  v = validation.at(1);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == ValidationCause::LACKS_ZYGOSITY);
  // now construct a correct variant
  org::phenopackets::schema::v1::core::VcfAllele* vc =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::VcfAllele>(&arena);
  vc->set_id("id:A");
  vc->set_genome_assembly("GRCh37");
  vc->set_chr("chr1");
  vc->set_pos(42);
  vc->set_ref("A");
  vc->set_alt("C");

  // Now make an Age that has an ontology class
  string id = "GENO:0000135";
  string label = "heterozygous";
  org::phenopackets::schema::v1::core::OntologyClass* c =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::OntologyClass>(&arena);
  c->set_id(id);
  c->set_label(label);

  variantpb.set_allocated_vcf_allele(vc);
  variantpb.set_allocated_zygosity(c);

  Variant var2(variantpb);
  validation = var2.validate();
  REQUIRE(validation.empty()==true); // we expect no errors/warnings here.
  
  variantpb.release_vcf_allele();
  variantpb.release_zygosity();
}


TEST_CASE("Test Disease","[disease]"){
  org::phenopackets::schema::v1::core::Disease diseasepb;
  // error -- no data
  Disease d1(diseasepb);
  vector<Validation> validation = d1.validate();
  REQUIRE(validation.size()==1);
  Validation v = validation.at(0);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == ValidationCause::DISEASE_LACKS_TERM);
  
  // add a term and an onset with one error
  string id = "OMIM:101600";
  string label = "Pfeiffer syndrome";
  org::phenopackets::schema::v1::core::OntologyClass* diseaseTerm =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::OntologyClass>(&arena);
  diseaseTerm->set_id(id);
  diseaseTerm->set_label(label);

  org::phenopackets::schema::v1::core::OntologyClass* onsetTerm =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::OntologyClass>(&arena);
  onsetTerm->set_id("HP:0011463");
  // Forgot to set label  Childhood onset HP:0011463

  diseasepb.set_allocated_term(diseaseTerm);
  diseasepb.set_allocated_class_of_onset(onsetTerm);
  Disease d2(diseasepb);
  validation = d2.validate();
  REQUIRE(validation.size()==1);
  v = validation.at(0);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == ValidationCause::ONTOLOGY_LABEL_EMPTY);

  diseasepb.release_term();
  diseasepb.release_class_of_onset();
}

TEST_CASE("Test File","[file]"){
  org::phenopackets::schema::v1::core::File filepb;
  // error -- no data
  File f1(filepb);
  vector<Validation> validation = f1.validate();
  REQUIRE(validation.size()==1);
  Validation v = validation.at(0);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == ValidationCause::FILE_LACKS_SPECIFICATION);
  // add file data
  filepb.set_uri("http://www.example.org");
  File f2(filepb);
  validation = f2.validate();
  REQUIRE(validation.empty()==true);
}


TEST_CASE("Test HtsFile","[htsfile]") {
     org::phenopackets::schema::v1::core::HtsFile htsfilepb;
  // error -- no data
  HtsFile f1(htsfilepb);
   vector<Validation> validation = f1.validate();
  REQUIRE(validation.size()==4);
  Validation v = validation.at(0);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == ValidationCause::UNIDENTIFIED_HTS_FILETYPE);
  v = validation.at(1);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == ValidationCause::LACKS_GENOME_ASSEMBLY);
  v = validation.at(2);
  REQUIRE(v.is_warning()==true);
  REQUIRE(v.get_cause() == ValidationCause::LACKS_SAMPLE_MAP);
  v = validation.at(3);
  REQUIRE(v.is_error()==true);
  REQUIRE(v.get_cause() == ValidationCause::LACKS_HTS_FILE);
  // set the HTS format to VCF. Then we should only have 3 QC issues
  htsfilepb.set_hts_format(org::phenopackets::schema::v1::core::HtsFile_HtsFormat_BAM);
    HtsFile f2(htsfilepb);
   validation = f2.validate();
  REQUIRE(validation.size()==3);
  // set the genome assembly. Then we should only have 2 Q/C issues
  htsfilepb.set_genome_assembly("GRCh38");
    HtsFile f3(htsfilepb);
   validation = f3.validate();
  REQUIRE(validation.size()==2);
  // add an entry to the sample id map. Then we should only have one error
   (*(htsfilepb.mutable_individual_to_sample_identifiers()))["sample 1"]="file 1";
  HtsFile f4(htsfilepb);
   validation = f4.validate();
  REQUIRE(validation.size()==1);
  // And a File -- then we should be 100%
  org::phenopackets::schema::v1::core::File* file =
    google::protobuf::Arena::Create<org::phenopackets::schema::v1::core::File>(&arena);
  file->set_uri("http://www.example.org");
  htsfilepb.set_allocated_file(file);
  HtsFile f5(htsfilepb);
    validation = f5.validate();
  REQUIRE(validation.empty());
  htsfilepb.release_file();
}


TEST_CASE("Test Resource","[resource]") {
     org::phenopackets::schema::v1::core::Resource resourcepb;
  // error -- no data
  Resource r1(resourcepb);
   vector<Validation> validation = r1.validate();
  REQUIRE(validation.size()==6);
  Validation v = validation.at(0);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == ValidationCause::RESOURCE_LACKS_ID);
  v = validation.at(1);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == ValidationCause::RESOURCE_LACKS_NAME);
  v = validation.at(2);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == ValidationCause::RESOURCE_LACKS_NAMESPACE_PREFIX);
  v = validation.at(3);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == ValidationCause::RESOURCE_LACKS_URL);
   v = validation.at(4);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == ValidationCause::RESOURCE_LACKS_VERSION);
   v = validation.at(5);
  REQUIRE(v.is_error());
  REQUIRE(v.get_cause() == ValidationCause::RESOURCE_LACKS_IRI_PREFIX);
  
  resourcepb.set_id("hp");
  resourcepb.set_name("human phenotype ontology");
  resourcepb.set_namespace_prefix("HP");
  resourcepb.set_url("http://purl.obolibrary.org/obo/hp.owl");
  resourcepb.set_version("2018-03-08");
  resourcepb.set_iri_prefix("http://purl.obolibrary.org/obo/HP_");
   Resource r2(resourcepb);
   validation = r2.validate();
  REQUIRE(validation.empty());
 
    
    
}

