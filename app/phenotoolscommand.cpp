
#include "phenotoolscommand.h"
#include <iostream>



#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

#include "../lib/phenopackets.pb.h"
#include "../lib/base.pb.h"
#include "../lib/phenotools.h"
#include "../lib/jsonobo.h"

using namespace phenotools;
using std::cerr;
using std::cout;
using std::vector;

PhenotoolsCommand::PhenotoolsCommand()
{
    // no -op
}

PhenotoolsCommand::PhenotoolsCommand(const string & hp_json):
    hp_json_path_(hp_json) 
{
    cout <<"hp json path " << hp_json << "\n";
    JsonOboParser parser{hp_json_path_};
    error_list_ = parser.get_errors();
    this->ontology_ = parser.get_ontology();
    if (! error_list_.empty()) {
        for (auto error : error_list_) {
            cout << "[ERROR] " << error << "\n";
        }
    }
}


struct tm 
PhenotoolsCommand::string_to_time(string iso8601date) const
{
    tm time;
    int y,M,d,h,m;
    float s;
    sscanf(iso8601date.c_str(), "%d-%d-%dT%d:%d:%fZ", &y, &M, &d, &h, &m, &s);
    time.tm_year = y - 1900; // Year since 1900
    time.tm_mon = M - 1;     // 0-11
    time.tm_mday = d;        // 1-31
    return time;
}


void
PhenotoolsCommand::init_toplevel_categories() 
{
    TermId ABNORMAL_CELLULAR = TermId::from_string("HP:0025354"); 
    toplevelCatories_.push_back(ABNORMAL_CELLULAR);
    TermId BLOOD = TermId::from_string("HP:0001871");
    toplevelCatories_.push_back(BLOOD);
    TermId CONNECTIVE_TISSUE = TermId::from_string("HP:0003549");
    toplevelCatories_.push_back(CONNECTIVE_TISSUE);
    TermId HEAD_AND_NECK = TermId::from_string("HP:0000152");
    toplevelCatories_.push_back(HEAD_AND_NECK);
    TermId LIMBS = TermId::from_string("HP:0040064");
    toplevelCatories_.push_back(LIMBS);
    TermId METABOLISM = TermId::from_string("HP:0001939");
    toplevelCatories_.push_back(METABOLISM);
    TermId PRENATAL = TermId::from_string("HP:0001197");
    toplevelCatories_.push_back(PRENATAL);
    TermId BREAST = TermId::from_string("HP:0000769");
    toplevelCatories_.push_back(BREAST);
    TermId CARDIOVASCULAR = TermId::from_string("HP:0001626");
    toplevelCatories_.push_back(CARDIOVASCULAR);
    TermId DIGESTIVE = TermId::from_string("HP:0025031");
    toplevelCatories_.push_back(DIGESTIVE);
    TermId EAR = TermId::from_string("HP:0000598");
    toplevelCatories_.push_back(EAR);
    TermId ENDOCRINE = TermId::from_string("HP:0000818");
    toplevelCatories_.push_back(ENDOCRINE);
    TermId EYE = TermId::from_string("HP:0000478");
    toplevelCatories_.push_back(EYE);
    TermId GENITOURINARY = TermId::from_string("HP:0000119");
    toplevelCatories_.push_back(GENITOURINARY);
    TermId IMMUNOLOGY = TermId::from_string("HP:0002715");
    toplevelCatories_.push_back(IMMUNOLOGY);
    TermId INTEGUMENT = TermId::from_string("HP:0001574");
    toplevelCatories_.push_back(INTEGUMENT);
    TermId MUSCLE = TermId::from_string("HP:0003011");
    toplevelCatories_.push_back(MUSCLE);
    TermId NERVOUS_SYSTEM = TermId::from_string("HP:0000707");
    toplevelCatories_.push_back(NERVOUS_SYSTEM);
    TermId RESPIRATORY = TermId::from_string("HP:0002086");
    toplevelCatories_.push_back(RESPIRATORY);
    TermId SKELETAL = TermId::from_string("HP:0000924");
    toplevelCatories_.push_back(SKELETAL);
    TermId THORACIC_CAVITY = TermId::from_string("HP:0045027");
    toplevelCatories_.push_back(THORACIC_CAVITY);
    TermId VOICE = TermId::from_string("HP:0001608");
    toplevelCatories_.push_back(VOICE);
    TermId CONSTITUTIONAL = TermId::from_string("HP:0025142");
    toplevelCatories_.push_back(CONSTITUTIONAL);
    TermId GROWTH = TermId::from_string("HP:0001507");
    toplevelCatories_.push_back(GROWTH);
    TermId NEOPLASM = TermId::from_string("HP:0002664");
    toplevelCatories_.push_back(NEOPLASM);
    TermId CLINICAL_MODIFIER = TermId::from_string("HP:0012823");
    toplevelCatories_.push_back(CLINICAL_MODIFIER);
    TermId FREQUENCY = TermId::from_string("HP:0040279");
    toplevelCatories_.push_back(FREQUENCY);
    TermId CLINICAL_COURSE = TermId::from_string("HP:0031797");
    toplevelCatories_.push_back(CLINICAL_COURSE);
    TermId MODE_OF_INHERITANCE = TermId::from_string("HP:0000005");
    toplevelCatories_.push_back(MODE_OF_INHERITANCE);
    TermId PAST_MEDICAL_HX = TermId::from_string("HP:0032443");
    toplevelCatories_.push_back(PAST_MEDICAL_HX);
    TermId BLOOD_GROUP = TermId::from_string("HP:0032223");
    toplevelCatories_.push_back(BLOOD_GROUP);
}

std::optional<TermId> 
PhenotoolsCommand::get_toplevel(const TermId &tid) const
{
    std::set<TermId> ancestors = ontology_->get_ancestors(tid);
    bool found = false;
    for (TermId t : ancestors) {
        auto p = std::find (toplevelCatories_.begin(), toplevelCatories_.end(), t);
        if (p != toplevelCatories_.end()) {
            return std::optional<TermId>(*p);
        }
    }
    return std::nullopt;
}




 PhenopacketCommand::PhenopacketCommand(const string &phenopacket, const string &hp_json):
    hp_json_path(hp_json),
    phenopacket_path(phenopacket)
 {

 }

int
PhenopacketCommand::execute() 
{
    std::ifstream inFile;
    inFile.open ( phenopacket_path );
    if ( ! inFile.good() ) {
      cerr << "Could not open Phenopacket file at " << phenopacket_path <<"\n";
      return EXIT_FAILURE;
    }
    GOOGLE_PROTOBUF_VERIFY_VERSION;
   
    std::stringstream sstr;
    sstr << inFile.rdbuf();
    string JSONstring = sstr.str();
    ::google::protobuf::util::JsonParseOptions options;
    ::org::phenopackets::schema::v1::Phenopacket phenopacketpb;
    ::google::protobuf::util::JsonStringToMessage (JSONstring, &phenopacketpb, options);
    cout << "\n#### Phenopacket at: " << phenopacket_path << " ####\n\n";
    phenotools::Phenopacket ppacket(phenopacketpb);
    auto validation = ppacket.validate();
    if (hp_json_path.empty()) {
        cout << "[INFO] path to hp.json not passed, skipping ontology-based validation.\n";
        return 0;
    }
    JsonOboParser parser{hp_json_path};
    vector<string> error_list = parser.get_errors();
    if (! error_list.empty()) {
        cerr << "[ERROR] Unable to parse ontology:\n:";
        for (string e : error_list) {
            cerr << "\t" << e << "\n";
        }
        exit(1);
    }
    std::unique_ptr<Ontology>  ontology = parser.get_ontology();
    auto semvalidation = ppacket.semantically_validate(ontology);
    validation.insert(validation.end(),semvalidation.begin(), semvalidation.end());
    
    if ( validation.empty() ) {
      cout << "No Q/C issues identified!\n";
    } else {
      auto N = validation.size();
      cout << "#### We identified "
	        << N << " Q/C issue" << ( N>1?"s":"" )
	        << " ####\n";
      for ( auto v : validation ) {
	       cout << v << "\n";
      }

    }
    return 0;
}