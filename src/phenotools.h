#ifndef PHENOTOOLS_H
#define PHENOTOOLS_H

#include <string>
#include <memory>
#include <vector>

#include "phenopackets.pb.h"
#include "base.pb.h"

using std::unique_ptr;
using std::string;
using std::vector;



enum class ValidationType { WARNING, ERROR };
enum class ValidationCause {
   ONTOLOGY_ID_EMPTY, // "OntologyClass id not allowed to be empty"
   INVALID_ONTOLOGY_ID, //OntologyClass id not a valid CURIE"
    ONTOLOGY_LABEL_EMPTY, // "OntologyClass label not allowed to be empty"
    AGE_ELEMENT_UNINITIALIZED,//"At least one of age and age_class must be present in Age element",
    INDIVIDUAL_LACKS_ID,//"id must be present in Individual element"
    INDIVIDUAL_LACKS_AGE,//"individual lacks age or age range information
    UNKNOWN_SEX, //"individual sex not known/provided"
    EXTERNAL_REFERENCE_LACKS_ID, //"evidence must have an id"
    EVIDENCE_LACKS_CODE, //"Evidence element must contain an ontology code";
    PHENOTYPIC_FEATURE_LACKS_ONTOLOGY_TERM, //"PhenotypicFeature element must contain an ontology term representing the phenotype";
    PHENOTYPIC_FEATURE_LACKS_EVIDENCE, //"PhenotypicFeature element must contain an evidence element";
} ;
static const string EMPTY="";// use for elements that are not present in the phenopacket input

class Validation{
    
private:
    enum ValidationCause cause_;
    ValidationType validation_type_;
    string message_;
    Validation(ValidationCause cause,ValidationType vt):
        cause_(cause),
        validation_type_(vt),
        message_(EMPTY)
        {}
    Validation(ValidationCause cause,ValidationType vt,string msg):
        cause_(cause),
        validation_type_(vt),
        message_(msg)
        {}
public:
    Validation(Validation &&v)=default;
    Validation(const Validation &v)=default;
    Validation& operator=(const Validation &v)=default;
    static Validation createError(ValidationCause msg){
        return Validation(msg, ValidationType::ERROR);
    }
    static Validation createError(ValidationCause cause, string msg){
        return Validation(cause, ValidationType::ERROR,msg);
    }
    static Validation createWarning(ValidationCause cause) {
         return Validation(cause, ValidationType::WARNING);
    }
    static Validation createWarning(ValidationCause cause, string msg) {
         return Validation(cause, ValidationType::WARNING,msg);
    }
  bool is_error() const { return validation_type_ == ValidationType::ERROR; }
  bool is_warning() const {  return validation_type_ == ValidationType::WARNING; }
  enum ValidationCause get_cause() const { return cause_; }
  const string message() const;
    
};

class ValidatorI {
public:
    virtual ~ValidatorI(){}
    virtual vector<Validation> validate()=0;
};

class OntologyClass : public ValidatorI {
private:
    string id_;
    string label_;
    
public:
    OntologyClass(const string &id, const string &label):
        id_(id),label_(label) {
        }
    OntologyClass(org::phenopackets::schema::v1::core::OntologyClass ontclz):
        id_(ontclz.id()),
        label_(ontclz.label()) {}
    OntologyClass(const OntologyClass & from):id_(from.id_),label_(from.label_){}
    ~OntologyClass(){}
    vector<Validation> validate();
};


class Age : public ValidatorI {
private:
  string age_;
  unique_ptr<OntologyClass> age_class_;

public:
  Age()=default;
  Age(const org::phenopackets::schema::v1::core::Age &a);
  ~Age(){}
   vector<Validation> validate();
};

class AgeRange : public ValidatorI {
private:
  Age start_;
  Age end_;
public:
  AgeRange(const org::phenopackets::schema::v1::core::AgeRange &ar):
      start_(ar.start()),
      end_(ar.end()) {}
  ~AgeRange(){}
   vector<Validation> validate();
};



enum class Sex {
  UNKNOWN_SEX,
  FEMALE ,
  MALE,
  OTHER_SEX
};

enum class KaryotypicSex {
    UNKNOWN_KARYOTYPE ,
    XX ,
    XY ,
    XO ,
    XXY ,
    XXX ,
    XXYY ,
    XXXY,
    XXXX ,
    XYY ,
    OTHER_KARYOTYPE
};


class Individual : public ValidatorI  {
private:
    // required
  string id_;
    // optional
  string dataset_id_;
    // optional (timestamp, converted to RFC 3339 date string)
  string date_of_birth_;
  // optional
  unique_ptr<Age> age_;
  // optional
  unique_ptr<AgeRange> age_range_;
  enum Sex sex_;
  enum KaryotypicSex karyotypic_sex_;
  unique_ptr<OntologyClass> taxonomy_ptr_;
    
  
  
public:
  Individual(org::phenopackets::schema::v1::core::Individual individual);
  ~Individual(){}
  vector<Validation> validate();
};


class ExternalReference : public ValidatorI {
private:
    string id_;
    string description_;
public:
    ExternalReference(org::phenopackets::schema::v1::core::ExternalReference er):
        id_(er.id()),
        description_(er.description()) {}
    ~ExternalReference(){}
    vector<Validation> validate();
};

class Evidence : public ValidatorI {
private:
    unique_ptr<OntologyClass> evidence_code_;
    unique_ptr<ExternalReference> reference_;
public:
    Evidence(org::phenopackets::schema::v1::core::Evidence evi);
    Evidence(const Evidence& from);
    Evidence(Evidence &&);
    Evidence & operator=(const Evidence &);
    ~Evidence(){}
    vector<Validation> validate();
};


class PhenotypicFeature : public ValidatorI {
private:
    string description_;
    unique_ptr<OntologyClass> type_;
    bool negated_;
    unique_ptr<OntologyClass> severity_;
    vector<OntologyClass> modifiers_;
    // onset can be one of the following three.
    unique_ptr<Age> age_of_onset_;
    unique_ptr<AgeRange> age_range_of_onset_;
    unique_ptr<OntologyClass> class_of_onset_;
    vector<Evidence> evidence_;
public:
    PhenotypicFeature(org::phenopackets::schema::v1::core::PhenotypicFeature pf);
    PhenotypicFeature(const PhenotypicFeature & pfeat) { std::cerr<<"TODO";  }
    PhenotypicFeature &operator=(const PhenotypicFeature & pfeat) { std::cerr<<"TODO"; return *this; }
    ~PhenotypicFeature(){}
    vector<Validation> validate();
    
};


class Phenopacket : public ValidatorI {
private:
    unique_ptr<Individual> subject_;
    vector<PhenotypicFeature> phenotypic_features_;
  

  
  
public:
  Phenopacket(const org::phenopackets::schema::v1::Phenopacket &pp) ;
  ~Phenopacket(){}
   vector<Validation> validate();




};








#endif
