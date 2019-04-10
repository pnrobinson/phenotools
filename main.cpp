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

    //Phenopacket phenopacket;
    // Create a TypeResolver used to resolve protobuf message types
  ::google::protobuf::util::JsonParseOptions options;

  /*
  inline util::Status JsonStringToMessage(StringPiece input,
                                          Message* message) {
  return JsonStringToMessage(input, message, JsonParseOptions());
}
*/
  Phenopacket phenopacket;
  //options.always_print_primitive_fields = true;
  //std::unique_ptr<google::protobuf::util::TypeResolver> resolver(google::protobuf::util::NewTypeResolverForDescriptorPool(
    //"type.googleapis.com", google::protobuf::DescriptorPool::generated_pool()));
  //  phenopacket = google::protobuf::JsonToMessage("org/phenopackets/schema/v1/phenopackets.Phenopacket", JSONstring);
  ::google::protobuf::util::JsonStringToMessage(JSONstring,&phenopacket,options);
}
