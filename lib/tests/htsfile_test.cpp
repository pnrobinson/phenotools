/**
 * Tests related to the HtsFile class
 * */

#include "catch.hpp"
#include "../base.pb.h"
#include "../phenopackets.pb.h"
#include "../interpretation.pb.h"
#include "../phenotools.h"

TEST_CASE("Test HtsFile lacks data","[htsfile_lacksdata]") {
    org::phenopackets::schema::v1::core::HtsFile htsfilepb;
    // error -- no data
    phenotools::HtsFile f1(htsfilepb);
    vector<phenotools::Validation> validation = f1.validate();
    // THe errors are that we did not specifiy the file type, we did not
    // set the genome assembly, and we did not set the sample map.
    REQUIRE(validation.size()==4);
    phenotools::Validation v = validation.at(0);
    REQUIRE(v.is_error()==true);
    REQUIRE(v.get_cause() == phenotools::ValidationCause::HTSFILE_LACKS_URI);
    v = validation.at(1);
    REQUIRE(v.is_error()==true);
    REQUIRE(v.get_cause() == phenotools::ValidationCause::UNIDENTIFIED_HTS_FILETYPE);
    v = validation.at(2);
    REQUIRE(v.is_error()==true);
    REQUIRE(v.get_cause() == phenotools::ValidationCause::LACKS_GENOME_ASSEMBLY);
    v = validation.at(3);
    REQUIRE(v.is_warning()==true);
    REQUIRE(v.get_cause() == phenotools::ValidationCause::LACKS_SAMPLE_MAP);
    // set the HTS format to VCF. Then we should only have 3 QC issues
}


TEST_CASE("Test HtsFile","[htsfile]") {
  org::phenopackets::schema::v1::core::HtsFile htsfilepb;
  // error -- no data
  phenotools::HtsFile f1(htsfilepb);
  vector<phenotools::Validation> validation = f1.validate();
  // THe errors are that we did not specifiy the file type, we did not
  // set the genome assembly, and we did not set the sample map.
  
  // set the HTS format to VCF. Then we should only have 3 QC issues
  htsfilepb.set_hts_format(org::phenopackets::schema::v1::core::HtsFile_HtsFormat_BAM);
  // set the genome assembly. Then we should only have 2 Q/C issues
  htsfilepb.set_genome_assembly("GRCh38");
  // add an entry to the sample id map. Then we should only have one error
  (*(htsfilepb.mutable_individual_to_sample_identifiers()))["sample 1"]="file 1";
  htsfilepb.set_uri("http://www.example.org");
  phenotools::HtsFile f4(htsfilepb);
  validation = f4.validate();
  for (phenotools::Validation v : validation) {
      std::cout << v << "\n";
  }
  REQUIRE(validation.size()==0);
  REQUIRE(validation.empty());
}
