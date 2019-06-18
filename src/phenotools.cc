// $Id$
/**
 * @file phenotools.cc
 * Classes for ingestion, validation, creation, and output of phenopackets
 *
 * @brief Main implementation file for phenotools
 *
 * @author Peter N Robinson
 *
 * @version 0.02
 */


#include "phenotools.h"
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/util/time_util.h>
#include <time.h>


using std::make_unique;


static string EMPTY_STRING="";
static string NOT_AVAILABLE="n/a";
static string NOT_IMPLEMENTED="NOT IMPLEMENTED"; // TODO -- IMPLEMENT ALL OF THIS



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
  case ValidationCause::PHENOTYPIC_FEATURE_LACKS_ONTOLOGY_TERM: return "PhenotypicFeature element must contain an ontology term representing the phenotype";
  case ValidationCause::PHENOTYPIC_FEATURE_LACKS_EVIDENCE: return "PhenotypicFeature element must contain an evidence element";
  case ValidationCause::GENE_LACKS_ID: return "Gene must have id element";
  case ValidationCause::GENE_LACKS_SYMBOL: return "Gene must have symbol";
  case ValidationCause::ALLELE_LACKS_ID: return "Variant should have an id";
  case ValidationCause::ALLELE_LACKS_HGVS: return "HgvsAllele lack HGVS string";
  case ValidationCause::LACKS_GENOME_ASSEMBLY: return "Genome assembly missing";
  case ValidationCause::LACKS_CHROMOSOME: return "Chromosome missing";
  case ValidationCause::LACKS_REF: return "ref missing";
  case ValidationCause::LACKS_ALT: return "alt missing";
  case ValidationCause::LACKS_ZYGOSITY: return "zygosity missing";
  case ValidationCause::LACKS_ALLELE: return "allele missing";
  case ValidationCause::DISEASE_LACKS_TERM: return "disease lacks term";
  case ValidationCause::FILE_LACKS_SPECIFICATION: return "File must has path or uri";
    
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

std::ostream& operator<<(std::ostream& ost, const OntologyClass& oc){
  ost <<oc.label_ << " ["<<oc.id_ << "]";
  return ost;
}

Age::Age(const org::phenopackets::schema::v1::core::Age &a):age_(a.age()){
  if (a.has_age_class()) {
    age_class_ = make_unique<OntologyClass>(a.age_class());
  }
}

Age::Age(const Age& age):age_(age.age_)
{
  if (age.age_class_){
    age_class_ = make_unique<OntologyClass>(*(age.age_class_.get()));
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


std::ostream& operator<<(std::ostream& ost, const Age& age){
  if (! age.age_.empty()) {
    ost << age.age_;
  } else if (age.age_class_) {
    ost <<  age.age_class_->get_label() << "[" << age.age_class_->get_id() << "]";
  } else {
    ost << NOT_AVAILABLE;
  }
  return ost;
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

std::ostream& operator<<(std::ostream& ost, const AgeRange& agerange){
  ost << agerange.start_ << "-" << agerange.end_;
  return ost;
}

std::ostream& operator<<(std::ostream& ost, const enum Sex& sex){
  switch (sex) {
 
  case Sex::FEMALE:
    ost<<"female";break;
  case Sex::MALE:
    ost<<"male";break;
  case Sex::OTHER_SEX:
    ost<<"other sex";break;
  case Sex::UNKNOWN_SEX:
  default:
    ost<<"unknown sex"; break;
  }
  return ost;
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


Evidence::Evidence(const Evidence &from){
  if (from.evidence_code_) {
    evidence_code_.reset(new OntologyClass(*(from.evidence_code_)));   
  }
  if (from.reference_) {
    reference_.reset(new ExternalReference(*(from.reference_)));   
  }
}

Evidence::Evidence(Evidence && from){
  if (from.evidence_code_) {
    evidence_code_ = std::move(from.evidence_code_);   
  }
  if (from.reference_) {
    reference_= std::move(from.reference_);   
  }
}

Evidence & Evidence::operator=(const Evidence & from){
  if (from.evidence_code_) {
    evidence_code_.reset(new OntologyClass(*(from.evidence_code_)));   
  }
  if (from.reference_) {
    reference_.reset(new ExternalReference(*(from.reference_)));   
  }
  return *this;
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

PhenotypicFeature::PhenotypicFeature(const org::phenopackets::schema::v1::core::PhenotypicFeature &pf):
  description_(pf.description()),
  negated_(pf.negated())
{
  if (pf.has_type()) {
    type_ = make_unique<OntologyClass>(pf.type());
  }
  if (pf.has_severity()){
    severity_ = make_unique<OntologyClass>(pf.severity());
  }
  if (pf.modifiers_size()>0) {
    for (auto m : pf.modifiers()) {
      OntologyClass ocm(m);
      modifiers_.push_back(ocm);
    }
  }
  // onset can be one of the following three.
  if(pf.has_age_of_onset()) {
    age_of_onset_ = make_unique<Age>(pf.age_of_onset());
  } else if (pf.has_age_range_of_onset()) {
    age_range_of_onset_ = make_unique<AgeRange>(pf.age_range_of_onset());
  } else if (pf.has_class_of_onset()) {
    class_of_onset_ = make_unique<OntologyClass>(pf.class_of_onset());
  }
  if (pf.evidence_size()>0) {
    for (auto e : pf.evidence()) {
      Evidence evi(e);
      evidence_.push_back(evi);
    }
  }
}

PhenotypicFeature::PhenotypicFeature(const PhenotypicFeature & pfeat):
  description_(pfeat.description_),
  negated_(pfeat.negated_)
{
  if (pfeat.type_) {
    type_=make_unique<OntologyClass>((*(pfeat.type_.get())));
  }
  if (pfeat.severity_) {
    severity_ = make_unique<OntologyClass>((*(pfeat.severity_.get())));
  }
  if (pfeat.modifiers_.size()>0) {
    for (auto m : pfeat.modifiers_) {
      modifiers_.push_back(m);
    }
  }
  // onset can be one of the following three.
  if(pfeat.age_of_onset_) {
    age_of_onset_ = make_unique<Age>(*(pfeat.age_of_onset_));
  } else if (pfeat.age_range_of_onset_) {
    age_range_of_onset_ = make_unique<AgeRange>(*(pfeat.age_range_of_onset_));
  } else if (pfeat.class_of_onset_ ) {
    class_of_onset_ = make_unique<OntologyClass>(*(pfeat.class_of_onset_));
  }
  if (pfeat.evidence_.size()>0) {
    for (auto e : pfeat.evidence_) {
      evidence_.push_back(e);
    }
  }


}

vector<Validation> 
PhenotypicFeature::validate(){
  vector<Validation> vl;
  // description is optional so we will not check it
  if (! type_ ){
    Validation v = Validation::createError(ValidationCause::PHENOTYPIC_FEATURE_LACKS_ONTOLOGY_TERM);
    vl.push_back(v);
  } else {
    vector<Validation>  v2 =  type_->validate();
    if (v2.size()>0) {
      vl.insert(vl.end(),v2.begin(),v2.end() );  
    }
  }
  // we do not need to check the negated_ field -- it is a primitive bool
  // The only other field that is not optional is the evidence.
  if (evidence_.empty() ){
    Validation v = Validation::createError(ValidationCause::PHENOTYPIC_FEATURE_LACKS_EVIDENCE);
  } else  {
    for (Evidence e : evidence_) {
      vector<Validation>  v2 =  e.validate();
      if (v2.size()>0) {
	vl.insert(vl.end(),v2.begin(),v2.end() );  
      }  
    }
  } 
  return vl;
}

vector<Validation>
Gene::validate(){
  vector<Validation> vl;
  if (id_.empty()) {
    Validation v = Validation::createError(ValidationCause::GENE_LACKS_ID);
    vl.push_back(v);
  }
  if (symbol_.empty()) {
    Validation v = Validation::createError(ValidationCause::GENE_LACKS_SYMBOL);
    vl.push_back(v);
  }
  return vl;
}

std::ostream& operator<<(std::ostream& ost, const Gene& gene){
  ost <<gene.symbol_<<"["<<gene.id_<<"]";
  return ost;
}

vector<Validation>
HgvsAllele::validate(){
  vector<Validation> vl;
  if (id_.empty()) {
    Validation v = Validation::createWarning(ValidationCause::ALLELE_LACKS_ID);
    vl.push_back(v);
  }
  if (hgvs_.empty()) {
    Validation v = Validation::createError(ValidationCause::ALLELE_LACKS_HGVS);
    vl.push_back(v);
  }
  return vl;
}

std::ostream &operator<<(std::ostream& ost, const HgvsAllele& hgvs){
  if (! hgvs.id_.empty()) {
    ost << hgvs.id_ <<": ";
  }
  ost << hgvs.hgvs_;
  return ost;
}


VcfAllele::VcfAllele(const org::phenopackets::schema::v1::core::VcfAllele &vcf):
  genome_assembly_(vcf.genome_assembly()),
  id_(vcf.id()),
  chr_(vcf.chr()),
  pos_(vcf.pos()),
  ref_(vcf.ref()),
  alt_(vcf.alt()),
  info_(vcf.info()) {}

VcfAllele::VcfAllele(const VcfAllele &vcf):
  genome_assembly_(vcf.genome_assembly_),
  id_(vcf.id_),
  chr_(vcf.chr_),
  pos_(vcf.pos_),
  ref_(vcf.ref_),
  alt_(vcf.alt_),
  info_(vcf.info_) {}

vector<Validation>
VcfAllele::validate(){
 vector<Validation> vl;
  if (id_.empty()) {
    Validation v = Validation::createWarning(ValidationCause::ALLELE_LACKS_ID);
    vl.push_back(v);
  }
  if ( genome_assembly_.empty()) {
    Validation v = Validation::createError(ValidationCause::LACKS_GENOME_ASSEMBLY);
     vl.push_back(v);
  }
  if (chr_.empty()) {
      Validation v = Validation::createError(ValidationCause::LACKS_CHROMOSOME);
       vl.push_back(v);
  }
  // way of checking pos?
  if (ref_.empty()) {
    Validation v = Validation::createError(ValidationCause::LACKS_REF);
     vl.push_back(v);
  }
  if (alt_.empty()) {
    Validation v = Validation::createError(ValidationCause::LACKS_ALT);
    vl.push_back(v);
  }
  // info is optional, we do not check it
  return vl;
}

std::ostream &operator<<(std::ostream& ost, const VcfAllele& vcf){
  if (! vcf.id_.empty() ) {
    ost << vcf.id_ <<": ";
  }
  ost << vcf.chr_ << ":" << vcf.pos_ << vcf.ref_ << ">"<<vcf.alt_
      << "[" << vcf.genome_assembly_;
  if (! vcf.info_.empty() ) {
    ost << ", "<< vcf.info_;
  }
  ost << "]";
  return ost;
}



Variant::Variant(const org::phenopackets::schema::v1::core::Variant & var){
  if (var.has_hgvs_allele()) {
    hgvs_allele_ = make_unique<HgvsAllele>(var.hgvs_allele());
  } else if (var.has_vcf_allele()) {
    vcf_allele_ = make_unique<VcfAllele>(var.vcf_allele());
  }

  // TODO
  if (var.has_zygosity()) {
    zygosity_ = make_unique<OntologyClass>(var.zygosity());
  }

}

Variant::Variant(const Variant & var) {
  if (var.hgvs_allele_) {
    hgvs_allele_ = make_unique<HgvsAllele>(*(var.hgvs_allele_.get()));
  } else if (var.vcf_allele_) {
    vcf_allele_ = make_unique<VcfAllele>(*(var.vcf_allele_.get()));
  }
  // todo
  if (var.zygosity_) {
    zygosity_ = make_unique<OntologyClass>(*(var.zygosity_));
  }
 

}

vector<Validation>
Variant::validate() {
  vector<Validation> vl;
  bool has_allele=false;
  if (hgvs_allele_) {
    has_allele=true;
     vector<Validation>  v2 =  hgvs_allele_->validate();
     if (v2.size()>0) {
       vl.insert(vl.end(),v2.begin(),v2.end() );  
     }
  } else if (vcf_allele_) {
    has_allele=true;
    vector<Validation>  v2 =  vcf_allele_->validate();
    if (v2.size()>0) {
      vl.insert(vl.end(),v2.begin(),v2.end() );  
    }
  } else {
    std::cout <<"WARNING SOME ALLELE SUBTYPES NOT IMPLEMENTED YET";
  }
  if (! has_allele) {
    Validation v = Validation::createError(ValidationCause::LACKS_ALLELE);
    vl.push_back(v);
  }
  if (! zygosity_) {
    Validation v = Validation::createError(ValidationCause::LACKS_ZYGOSITY);
    vl.push_back(v);
  } else {
    vector<Validation>  v2 =  zygosity_->validate();
    if (v2.size()>0) {
      vl.insert(vl.end(),v2.begin(),v2.end() );  
    }
  }
  return vl;
}


 
Disease::Disease(const org::phenopackets::schema::v1::core::Disease & dis){
  if (dis.has_term()) {
    term_=make_unique<OntologyClass>(dis.term());
  }
  if (dis.has_age_of_onset()) {
    age_of_onset_ = make_unique<Age>(dis.age_of_onset());
  } else if (dis.has_age_range_of_onset()) {
    age_range_of_onset_ = make_unique<AgeRange>(dis.age_range_of_onset());
  } else if (dis.has_class_of_onset()) {
    class_of_onset_=make_unique<OntologyClass>(dis.class_of_onset());
  }
}

Disease::Disease(const Disease &dis){
  if (dis.term_) {
    term_=make_unique<OntologyClass>(*(dis.term_.get()));
  }
  if (dis.age_of_onset_) {
    age_of_onset_ = make_unique<Age>(*(dis.age_of_onset_.get()));
  } else if (dis.age_range_of_onset_) {
    age_range_of_onset_ = make_unique<AgeRange>(*(dis.age_range_of_onset_.get()));
  } else if (dis.class_of_onset_) {
    term_=make_unique<OntologyClass>(*(dis.class_of_onset_.get()));
  }

}
vector<Validation> Disease::validate(){
  vector<Validation> vl;
  if (! term_ ){
    Validation v = Validation::createError(ValidationCause::DISEASE_LACKS_TERM);
    vl.push_back(v);
  }
  // onset is optional but if present we check the validity
  if (age_of_onset_) {
    vector<Validation>  v2 =  age_of_onset_->validate();
    if (v2.size()>0) {
      vl.insert(vl.end(),v2.begin(),v2.end() );  
    }
  } else if (age_range_of_onset_) {
     vector<Validation>  v2 =  age_range_of_onset_->validate();
    if (v2.size()>0) {
      vl.insert(vl.end(),v2.begin(),v2.end() );  
    }
  } else if (class_of_onset_) {
    vector<Validation>  v2 =  class_of_onset_->validate();
    if (v2.size()>0) {
      vl.insert(vl.end(),v2.begin(),v2.end() );  
    }
  }
  return vl;
}
  
std::ostream &operator<<(std::ostream& ost, const Disease& dis){
  if (dis.term_) {
    ost << *(dis.term_);
  }
  if (dis.age_of_onset_) {
    ost << *(dis.age_of_onset_);
  } else if (dis.age_range_of_onset_) {
    ost << *(dis.age_range_of_onset_);
  } else if (dis.class_of_onset_) {
    ost << *(dis.class_of_onset_);
  }
  return ost;
}


std::ostream &operator<<(std::ostream& ost, const Variant& var){
  if (var.hgvs_allele_) {
    ost << *(var.hgvs_allele_.get());
  } else if (var.vcf_allele_) {
    ost << *(var.vcf_allele_.get());
  }
      return ost;
}



File::File(const org::phenopackets::schema::v1::core::File &file):
  path_(file.path()),
  uri_(file.uri()),
  description_(file.description())
{}

  
File::File(const File &file):
  path_(file.path_),
  uri_(file.uri_),
  description_(file.description_)
{}

vector<Validation>
File::validate(){
   vector<Validation> vl;
  if ( path_.empty() && uri_.empty()){
    Validation v = Validation::createError(ValidationCause::FILE_LACKS_SPECIFICATION);
    vl.push_back(v);
  }
  return vl;
}


std::ostream &operator<<(std::ostream& ost, const File& file){
 
  if (! file.path_.empty()) {
    ost << file.path_;
  } else if (file.uri_.empty()) {
    ost << file.uri_;
  }
  if (file.description_.empty()){
    ost << " [" << file.description_ << "]";
  }
  return ost;
}

/*
enum class HtsFormat {  SAM, BAM, CRAM, VCF, BCF, GVCF };

class HtsFile : public ValidatorI {
 private:
  HtsFormat hts_format_;
  string genome_assembly_;
  map<string,string> individual_to_sample_identifiers_;
  unique_ptr<File> file_;

 public:
  
  HtsFile(const HtsFile &htsfile);
  vector<Validation> validate();
  friend std::ostream &operator<<(std::ostream& ost, const HtsFile& htsfile);
};
std::ostream &operator<<(std::ostream& ost, const HtsFile& htsfile);
*/
HtsFile::HtsFile(const org::phenopackets::schema::v1::core::HtsFile &htsfile){
  



}





Phenopacket::Phenopacket(const org::phenopackets::schema::v1::Phenopacket &pp){
  if (pp.has_subject()){
    subject_ = make_unique<Individual> (pp.subject());
  }
  if (pp.phenotypic_features_size()>0) {
    for (auto m : pp.phenotypic_features()) {
      PhenotypicFeature phenofeature(m);
      phenotypic_features_.push_back(phenofeature);
    }
  }
  if (pp.biosamples_size()>0) {
    std::cerr<<"[WARNING] Biosamples not implemented yet\n";   
  }
  if (pp.genes_size()>0) {
    for (auto g:pp.genes()) {
      Gene gene(g);
      genes_.push_back(gene);
    }
  }
  if (pp.variants_size()>0) {
    for (auto v:pp.variants()) {
      Variant var(v);
      variants_.push_back(var);
    }
  }
  if (pp.diseases_size()>0) {
    for (auto d:pp.diseases()) {
      Disease dis(d);
      diseases_.push_back(dis);
    }
  }
}





vector<Validation> 
Phenopacket::validate(){
  // check the subject
  vector<Validation> vl;
  return vl;
}


std::ostream& operator<<(std::ostream& ost, const Individual& ind){
  ost << "ID: "<<ind.id_ << "\n";
  ost << "Age: "<<*(ind.age_.get()) <<"\n";
  ost << "Sex: "<<ind.sex_<<"\n";
  return ost;
  
}

std::ostream& operator<<(std::ostream& ost, const Phenopacket& ppacket)
{
  ost<<"Phenopacket:\n";
  if (ppacket.subject_) {
    ost<<*(ppacket.subject_.get());
  } else {
    ost <<"[ERROR] subject element not initialized\n";
  }
  for (PhenotypicFeature feature: ppacket.phenotypic_features_) {
    ost << feature.get_label() <<" ["<<feature.get_id()<<"]\n";   
  }
  if (ppacket.genes_.empty()) {
    ost << "genes: n/a\n";
  } else {
    for (Gene g : ppacket.genes_) {
      ost << "Gene: "<<g<<"\n";
    }
  }
  if (ppacket.variants_.empty()) {
    ost << "Variants: n/a\n";
  } else {
    for (Variant v : ppacket.variants_) {
      ost << "\t" << v << "\n";
    }
  }
  if (ppacket.diseases_.empty()) {
    ost << "Diseases: n/a\n";
  } else {
    for (Disease d : ppacket.diseases_) {
      ost << "Disease: "<< d <<"\n";
    }

  }
  
  //ost << *(ppacket.get_subject()) << "\n";
  return ost;
}
