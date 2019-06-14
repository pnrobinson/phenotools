#ifndef PHENOTOOLS_H
#define PHENOTOOLS_H

#include <string>
#include "phenopackets.pb.h"
#include "base.pb.h"


class Subject {
  private:
   std::string id_;

  public Subject(org::phenopackets::schema::v1::core::Individual individual) {

  }

};




class Phenopacket {
  private:
     Subject subject_;



     private void validate(org::phenopackets::schema::v1::Phenopacket pp);

   public:
     Phenopacket(org::phenopackets::schema::v1::Phenopacket pp) {
      validate(pp);

     }





};








#endif
