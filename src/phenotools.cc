


#include "phenotools.h"



vector<Validation> 
OntologyClass::validate(){
    vector<Validation> vl;
    if (id_.empty()) {
        Validation e = Validation::createError("OntologyClass id not allowed to be empty");
        vl.push_back(e);
    } else {
         std::size_t pos = id_.find(':');
         if (pos == std::string::npos) {
             Validation e = Validation::createWarning("OntologyClass id ("+id_+") not a valid CURIE");
             vl.push_back(e);
         }
    }
    if (label_.empty()) {
        Validation e = Validation::createError("OntologyClass label not allowed to be empty");
        vl.push_back(e);
    }
    return vl;
}


void
Phenopacket::validate(org::phenopackets::schema::v1::Phenopacket &pp){
    // check the subject
    org::phenopackets::schema::v1::core::Individual subject = pp.subject();
    if (subject.id().empty() ){
        Validation e = Validation::createError("Subject id not allowed to be empty");
        validation_list_.push_back(e);
    }
}
