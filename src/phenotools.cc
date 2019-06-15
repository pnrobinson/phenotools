


#include "phenotools.h"





void
Phenopacket::validate(org::phenopackets::schema::v1::Phenopacket &pp){
    // check the subject
    org::phenopackets::schema::v1::core::Individual subject = pp.subject();
    if (subject.id().empty() ){
        Validation e = Validation::createError("Subject id not allowed to be empty");
        validation_list_.push_back(e);
        
    }
}
