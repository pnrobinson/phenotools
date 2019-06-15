


#include "phenotools.h"

using std::make_unique;

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

Age::Age(org::phenopackets::schema::v1::core::Age a):age_(a.age()){
    if (a.has_age_class()) {
        age_class_ = make_unique<OntologyClass>(a.age_class());
    }
}

vector<Validation> Age::validate(){
    vector<Validation> vl;
    if (age_.empty() && ! age_class_ ){
        Validation e = Validation::createError("At least one of age and age_class must be present in Age element");
        vl.push_back(e);
    }
    if (age_class_) {
        vector<Validation> age_class_valid = age_class_->validate();
        if (age_class_valid.size()>0) {
            vl.insert( vl.end(), age_class_valid.begin(), age_class_valid.end() );
        }
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
