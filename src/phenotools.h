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


class Subject {
private:
  std::string id_;
  
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
