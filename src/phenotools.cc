


#include "phenotools.h"
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/util/time_util.h>
#include <time.h>


using std::make_unique;


static string EMPTY_STRING="";



const string 
Validation::message() const {
    if (! message_.empty()) return message_;
    switch(cause_) {
        case ValidationCause::ONTOLOGY_ID_EMPTY: return "OntologyClass id not allowed to be empty";
        case ValidationCause::INVALID_ONTOLOGY_ID: return "OntologyClass id not a valid CURIE";
        case ValidationCause::ONTOLOGY_LABEL_EMPTY: return "OntologyClass label not allowed to be empty";
        case ValidationCause::AGE_ELEMENT_UNINITIALIZED: return "At least one of age and age_class must be present in Age element";
        case ValidationCause::INDIVIDUAL_LACKS_ID: return "id must be present in Individual element";
        case ValidationCause::INDIVIDUAL_LACKS_AGE: return "individual lacks age or age range information";
        case ValidationCause::UNKNOWN_SEX: return "individual sex not known/provided";
        case ValidationCause::EXTERNAL_REFERENCE_LACKS_ID: return "external reference must have an id";
        case ValidationCause::EVIDENCE_LACKS_CODE: return "Evidence element must contain an ontology code";
    }
    // should never happen
    return "unknown error";
}


vector<Validation> 
OntologyClass::validate(){
    vector<Validation> vl;
    if (id_.empty()) {
        Validation e = Validation::createError(ValidationCause::ONTOLOGY_ID_EMPTY);
        vl.push_back(e);
    } else {
         std::size_t pos = id_.find(':');
         if (pos == std::string::npos) {
             Validation e = Validation::createWarning(ValidationCause::INVALID_ONTOLOGY_ID,
                                                      "OntologyClass id ("+id_+") not a valid CURIE");
             vl.push_back(e);
         }
    }
    if (label_.empty()) {
        Validation e = Validation::createError(ValidationCause::ONTOLOGY_LABEL_EMPTY);
        vl.push_back(e);
    }
    return vl;
}

Age::Age(const org::phenopackets::schema::v1::core::Age &a):age_(a.age()){
    if (a.has_age_class()) {
        age_class_ = make_unique<OntologyClass>(a.age_class());
    }
}

vector<Validation> Age::validate(){
    vector<Validation> vl;
    if (age_.empty() && ! age_class_ ){
        Validation e = Validation::createError(ValidationCause::AGE_ELEMENT_UNINITIALIZED);
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


vector<Validation> 
AgeRange::validate(){
   vector<Validation> vl;
   vector<Validation> age1 = start_.validate();
   if (age1.size()>0) {
       vl.insert(vl.end(),age1.begin(),age1.end() );
   }
   vector<Validation> age2 = end_.validate();
   if (age2.size()>0) {
       vl.insert(vl.end(),age2.begin(),age2.end() );
   }
   return vl;
}



Individual::Individual(org::phenopackets::schema::v1::core::Individual individual):
    id_(individual.id()),
    dataset_id_(individual.dataset_id())
{
   google::protobuf::Timestamp tstamp = individual.date_of_birth();
   if (tstamp.IsInitialized()) {
    date_of_birth_=google::protobuf::util::TimeUtil::ToString(tstamp);
   } else {
    date_of_birth_= EMPTY_STRING;
   }
   
   if (individual.has_age_at_collection()){
       age_ = make_unique<Age>(individual.age_at_collection());
   } else if (individual.has_age_range_at_collection()) {
       age_range_ = make_unique<AgeRange>(individual.age_range_at_collection());
   }
   
    switch(individual.sex()) {
        case org::phenopackets::schema::v1::core::Sex::MALE:
            sex_=Sex::MALE;
            break;
        case org::phenopackets::schema::v1::core::Sex::FEMALE:
            sex_=Sex::FEMALE;
            break;
        case org::phenopackets::schema::v1::core::Sex::OTHER_SEX:
            sex_=Sex::OTHER_SEX;
            break;
        case org::phenopackets::schema::v1::core::Sex::UNKNOWN_SEX:
        default:
            sex_=Sex::UNKNOWN_SEX;
            break;
    }
    switch(individual.karyotypic_sex()){
        case org::phenopackets::schema::v1::core::KaryotypicSex::XX:
            karyotypic_sex_=KaryotypicSex::XX;
            break;
         case org::phenopackets::schema::v1::core::KaryotypicSex::XY:
            karyotypic_sex_=KaryotypicSex::XY;
            break;
        case org::phenopackets::schema::v1::core::KaryotypicSex::XO:
            karyotypic_sex_=KaryotypicSex::XO;
            break;   
          case org::phenopackets::schema::v1::core::KaryotypicSex::XXY:
            karyotypic_sex_=KaryotypicSex::XXY;
            break;   
           case org::phenopackets::schema::v1::core::KaryotypicSex::XXX:
            karyotypic_sex_=KaryotypicSex::XXX;
            break;
         case org::phenopackets::schema::v1::core::KaryotypicSex::XXYY:
            karyotypic_sex_=KaryotypicSex::XXYY;
            break;    
        case org::phenopackets::schema::v1::core::KaryotypicSex::XXXY:
            karyotypic_sex_=KaryotypicSex::XXXY;
            break;    
         case org::phenopackets::schema::v1::core::KaryotypicSex::XXXX:
            karyotypic_sex_=KaryotypicSex::XXXX;
            break;     
         case org::phenopackets::schema::v1::core::KaryotypicSex::XYY:
            karyotypic_sex_=KaryotypicSex::XYY;
            break; 
        case org::phenopackets::schema::v1::core::KaryotypicSex::OTHER_KARYOTYPE:
            karyotypic_sex_=KaryotypicSex::OTHER_KARYOTYPE;
            break; 
        case org::phenopackets::schema::v1::core::KaryotypicSex::UNKNOWN_KARYOTYPE:
        default:
            karyotypic_sex_=KaryotypicSex::UNKNOWN_KARYOTYPE;
    }
    if (individual.has_taxonomy()){
        taxonomy_ptr_ = make_unique<OntologyClass>(individual.taxonomy());
    } 
}


vector<Validation> 
Individual::validate(){
     vector<Validation> vl;
    if (id_.empty()) {
        Validation e = Validation::createError(ValidationCause::INDIVIDUAL_LACKS_ID);
        vl.push_back(e);
    }
    // dataset_id and date_of_birth are optional so we do not validate them
    if (age_) {
        vector<Validation> age_validation = age_->validate();
        if (!age_validation.empty()) {
         vl.insert(vl.end(),age_validation.begin(),age_validation.end() );   
        }
    } else if (age_range_) {
         vector<Validation> age_range_validation = age_range_->validate();
        if (!age_range_validation.empty()) {
         vl.insert(vl.end(),age_range_validation.begin(),age_range_validation.end() );   
        }
    } else { 
        // i.e., neither age nor age_range provided. 
        // It is recommended to have at least one of them so this generates a Warning 
        Validation e = Validation::createWarning(ValidationCause::INDIVIDUAL_LACKS_AGE);
        vl.push_back(e);
    }
    if (sex_ == Sex::UNKNOWN_SEX) {
        Validation e = Validation::createWarning(ValidationCause::UNKNOWN_SEX);
        vl.push_back(e);
    }
     // karyotype and taxonomy are option so we do not validate them
     
     return vl;
}

vector<Validation>
ExternalReference::validate(){
    vector<Validation> vl;
    if (id_.empty()) {
        Validation e = Validation::createError(ValidationCause::EXTERNAL_REFERENCE_LACKS_ID);
        vl.push_back(e);
    }
    // the description is optional so we do not validate it.
    return vl;
}

Evidence::Evidence(org::phenopackets::schema::v1::core::Evidence evi){
    if (evi.has_evidence_code()) {
     evidence_code_ = make_unique<OntologyClass>(evi.evidence_code());   
    } 
    if (evi.has_reference()) {
        reference_ = make_unique<ExternalReference>(evi.reference());
    }
}


vector<Validation> 
Evidence::validate(){
    vector<Validation> vl;
    if (! evidence_code_ ) {
      Validation e = Validation::createError(ValidationCause::EVIDENCE_LACKS_CODE);
      vl.push_back(e);
    } else {
        vector<Validation> v2 = evidence_code_->validate();
        if (v2.size()>0) {
            vl.insert(vl.end(),v2.begin(),v2.end() );  
        }
    }
    // external reference is optional, if it is absent there is no ERROR/WARNING
    if (reference_) {
         vector<Validation> v2 = reference_->validate();
        if (v2.size()>0) {
            vl.insert(vl.end(),v2.begin(),v2.end() );  
        }
    }
    return vl;
}



void
Phenopacket::validate(org::phenopackets::schema::v1::Phenopacket &pp){
    // check the subject
    org::phenopackets::schema::v1::core::Individual subject = pp.subject();
    if (subject.id().empty() ){
        Validation e = Validation::createError(ValidationCause::INDIVIDUAL_LACKS_ID);
        validation_list_.push_back(e);
    }
}
