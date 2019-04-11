#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

#include "phenopackets.pb.h"


using namespace std;
using namespace org::phenopackets::schema::v1;


int main(int argc, char ** argv) {
  string fileName="Gebbia-1997-ZIC3.json";
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  stringstream sstr;
  ifstream inFile;
  inFile.open(fileName);
  if (! inFile.good()) {
     cerr << "Could not open Phenopacket file at " << fileName <<"\n";
     return EXIT_FAILURE;
   }
   sstr << inFile.rdbuf();
   string JSONstring = sstr.str();
   cout <<"reading phenopacket\n" << JSONstring << "\n";

  ::google::protobuf::util::JsonParseOptions options;
  Phenopacket phenopacket;
  ::google::protobuf::util::JsonStringToMessage(JSONstring,&phenopacket,options);
  cout << "Phenopacket at: " << fileName << "\n";
  cout << "\tsubject.id: "<<phenopacket.subject().id() << "\n";
  // print age if available
  if (phenopacket.subject().has_age_at_collection()) {
    Age age = phenopacket.subject().age_at_collection();
    if (! age.age().empty()) {
      cout <<"\tsubject.age: " << age.age() << "\n";
    }
      cout <<"\tsubject.sex: " ;
      Sex sex = phenopacket.subject().sex();
      switch (sex) {
        case UNKNOWN_SEX : cout << " unknown"; break;
        case FEMALE : cout <<"female"; break;
        case MALE: cout <<"male"; break;
        case OTHER_SEX:
        default:
          cout <<"other"; break;
      }
      cout << "\n";
  }
  for (auto i = 0; i < phenopacket.phenotypes_size(); i++) {
   const Phenotype& phenotype = phenopacket.phenotypes(i);
   const OntologyClass type = phenotype.type();
   cout << "\tid: " << type.id() << ": " << type.label() << "\n";
 }


}
