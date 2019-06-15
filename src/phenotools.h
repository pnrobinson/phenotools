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
static const string EMPTY="";// use for elements that are not present in the phenopacket input

class Validation{
    
private:
    string message_;
    ValidationType validation_type_;
    Validation(const string &msg,ValidationType vt):
        message_(msg),
        validation_type_(vt)
        {}
public:
    static Validation createError(const string &msg){
        return Validation(msg, ValidationType::ERROR);
    }
    static Validation createWarning(const string &msg) {
         return Validation(msg, ValidationType::WARNING);
    }
    
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
    ~OntologyClass(){}
    vector<Validation> validate();
    
};


class Age {
private:
    string age_;
    
};


class Subject {
private:
    // required
  string id_;
    // optional
  string dataset_id_;
    // optional (timestamp)
  string data_of_birth_;
    
  
  
  
public:
  Subject(org::phenopackets::schema::v1::core::Individual individual) {

  }

};




class Phenopacket {
private:
    vector<Validation> validation_list_;    
    unique_ptr<Subject> subject_;
  
  
			      
			      
			      
  void validate(org::phenopackets::schema::v1::Phenopacket &pp);
  
public:
  Phenopacket(org::phenopackets::schema::v1::Phenopacket &pp) {
    validate(pp);
    
  }
  




};








#endif
