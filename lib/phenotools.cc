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
#include <set>
#include <sstream>


using std::make_unique;
using std::make_shared;
using std::cout;



namespace phenotools {

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
    case ValidationCause::PHENOTYPIC_FEATURE_LACKS_ONTOLOGY_TERM:
      return "PhenotypicFeature element must contain an ontology term representing the phenotype";
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
    case ValidationCause::UNIDENTIFIED_HTS_FILETYPE: return "Unidentified HTS file type";
    case ValidationCause::HTSFILE_LACKS_URI: return "HtsFile lacks a URI";
    case ValidationCause::LACKS_SAMPLE_MAP: return "no sample map for HTS file";
    case ValidationCause::LACKS_HTS_FILE: return "no HTS file found";
    case ValidationCause::RESOURCE_LACKS_ID: return "resource id missing";
    case ValidationCause::RESOURCE_LACKS_NAME: return "resource name missing";
    case ValidationCause::RESOURCE_LACKS_NAMESPACE_PREFIX: return "resource namespace prefix missing";
    case ValidationCause::RESOURCE_LACKS_URL: return "resource URL missing";
    case ValidationCause::RESOURCE_LACKS_VERSION: return "resource version missing";
    case ValidationCause::RESOURCE_LACKS_IRI_PREFIX: return "resource IRI prefix missing";
    case ValidationCause::METADATA_LACKS_CREATED_TIMESTAMP: return "metadata timestamp missing";
    case ValidationCause::METADATA_LACKS_CREATED_BY: return "metadata created-by missing";
    case ValidationCause::METADATA_LACKS_RESOURCES: return "metadata lacks resources";
    case ValidationCause::METADATA_DOES_NOT_CONTAIN_ONTOLOGY: return "metadata lacks ontology used in phenopacket";
    case ValidationCause::METADATA_HAS_SUPERFLUOUS_ONTOLOGY: return "metadata refers to ontology not used in phenopacket";
    case ValidationCause::PROCEDURE_LACKS_CODE: return "procedrure code missing";
    case ValidationCause::PHENOPACKET_LACKS_ID: return "phenopacket id missing";
    case ValidationCause::PHENOPACKET_LACKS_SUBJECT: return "phenopacket subject missing";
    case ValidationCause::PHENOPACKET_LACKS_PHENOTYPIC_FEATURES: return "phenopacket has no phenotypic features";
    case ValidationCause::PHENOPACKET_LACKS_METADATA: return "phenopacket has no metadata";
    case ValidationCause::BIOSAMPLE_LACKS_ID: return "biosample lacks id";
    case ValidationCause::BIOSAMPLE_LACKS_SAMPLED_TISSUE: return "biosample lacks a sampled tissue";
    case ValidationCause::BIOSAMPLE_LACKS_PHENOTYPES: return "biosample lacks phenotypic features";
    case ValidationCause::BIOSAMPLE_LACKS_AGE: return "biosample lacks age";
    case ValidationCause::BIOSAMPLE_LACKS_HISTOLOGY: return "biosample lacks histological diagnosis";
    case ValidationCause::BIOSAMPLE_LACKS_TUMOR_PROGRESSION: return "biosample lacks tumor progression data";
    case ValidationCause::BIOSAMPLE_LACKS_TUMOR_GRADE: return "biosample lacks tumor grade";
    case ValidationCause::BIOSAMPLE_LACKS_TUMOR_STAGE: return "biosample lacks tumor stage";
    case ValidationCause::BIOSAMPLE_LACKS_DIAGNOSTIC_MARKERS: return "biosample lacks diagnostic markers";
    case ValidationCause::ONTOLOGY_NOT_IN_METADATA: return "ontology used but not in metadata";
    case ValidationCause::REDUNDANT_ANNOTATION: return "redundant terms used in annotation";
    case ValidationCause::UNRECOGNIZED_TERMID: return "unrecognized TermId";
    }
    // should never happen
    return "unknown error";
  }

  void
  ValidatorI::validate(vector<Validation> &v) const {
      vector<Validation> val = validate();
      if (val.empty()) return;
      v.insert(v.end(),val.begin(),val.end());
  }


  std::ostream& operator<<(std::ostream& ost, const Validation& v){
    switch (v.validation_type_) {
    case ValidationType::WARNING:
      ost << "[WARNING] ";
      break;
    case ValidationType::ERROR:
      ost << "[ERROR] ";
      break;
    }
    ost << v.message();
    return ost;
  }

  /**
   * Return the prefix, i.e., "HP" from an id such as HP:0001234.
   */
  string 
  OntologyClass::get_prefix() const 
  {
    int i = id_.find_first_of(":");
    // when we get here we have already tested that
    // the prefixes are ok but guard against index errors
    if (i<0) return "N/A"; 
    return id_.substr(0,i);
  }


  vector<Validation>
  OntologyClass::validate() const {
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

  /**
   * Convenience function that fills an external vector with Validation objects if there
   * are errors.
   * @param validation_vector A vector that typically is derived from the class that contains this object
   */
  void
  OntologyClass::validate(vector<Validation> &validation_vector) const {
       vector<Validation> val = validate();
       if (val.empty()) return; // all is fine!
       validation_vector.insert(validation_vector.end(),val.begin(),val.end());
  }

  std::ostream& operator<<(std::ostream& ost, const OntologyClass& oc){
    ost <<oc.label_ << " ["<<oc.id_ << "]";
    return ost;
  }

  Age::Age(const org::phenopackets::schema::v1::core::Age &a):age_(a.age())
  {
  }

  Age::Age(const Age& age):age_(age.age_) 
  {
  }

  vector<Validation> Age::validate() const {
    vector<Validation> vl;
    if (age_.empty()){
      Validation e = Validation::createError(ValidationCause::AGE_ELEMENT_UNINITIALIZED);
      vl.push_back(e);
    }
    return vl;
  }

  void
  Age::validate(vector<Validation> &v) const
  {
    if (age_.empty()){
      Validation e = Validation::createError(ValidationCause::AGE_ELEMENT_UNINITIALIZED);
      v.push_back(e);
    }
  }


  std::ostream& operator<<(std::ostream& ost, const Age& age){
    ost << age.age_;
    return ost;
  }


  vector<Validation>
  AgeRange::validate() const {
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
    id_(individual.id())
  {
    google::protobuf::Timestamp tstamp = individual.date_of_birth();
    if (tstamp.IsInitialized()) {
      date_of_birth_=google::protobuf::util::TimeUtil::ToString(tstamp);
    } else {
      date_of_birth_= EMPTY_STRING;
    }
    if (! individual.alternate_ids().empty()) {
      alternate_ids_.insert(alternate_ids_.end(), individual.alternate_ids().begin(),individual.alternate_ids().end());
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
      taxonomy_ptr_ = std::make_shared<OntologyClass>(individual.taxonomy());
    }
  }


  vector<Validation>
  Individual::validate() const {
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

  void
  Individual::validate(vector<Validation> &val) const
  {
    vector<Validation> val2 = validate();
    if (val2.empty()) return;
    val.insert(val.end(),val2.begin(),val2.end());
  }



  vector<Validation>
  ExternalReference::validate() const {
    vector<Validation> vl;
    if (id_.empty()) {
      Validation e = Validation::createError(ValidationCause::EXTERNAL_REFERENCE_LACKS_ID);
      vl.push_back(e);
    }
    // the description is optional so we do not validate it.
    return vl;
  }

  void
  ExternalReference::validate(vector<Validation> &val) const
  {
    vector<Validation> v2 = validate();
    if (v2.empty()) return;
    val.insert(val.end(),v2.begin(),v2.end());
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
  Evidence::validate() const {
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
  Evidence::validate(vector<Validation> &val) const
  {
    vector<Validation> v2 = validate();
    if (v2.empty()) return;
    val.insert(val.end(), v2.begin(), v2.end());
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
  PhenotypicFeature::validate() const {
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
    // The only other field that is not optional is the evidence (recommended).
    if (evidence_.empty() ){
      Validation v = Validation::createWarning(ValidationCause::PHENOTYPIC_FEATURE_LACKS_EVIDENCE);
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

  void
  PhenotypicFeature::validate(vector<Validation> &val) const
  {
    vector<Validation> v2 = validate();
    if (v2.empty()) return;
    val.insert(val.end(), v2.begin(), v2.end());
  }

  vector<Validation>
  Gene::validate() const {
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
  HgvsAllele::validate() const {
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
  VcfAllele::validate() const {
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
      hgvs_allele_ = make_shared<HgvsAllele>(var.hgvs_allele());
    } else if (var.has_vcf_allele()) {
      vcf_allele_ = make_shared<VcfAllele>(var.vcf_allele());
    }

    // TODO
    if (var.has_zygosity()) {
      zygosity_ = make_shared<OntologyClass>(var.zygosity());
    }
  }

  Variant::Variant(const Variant & var) {
    if (var.hgvs_allele_) {
      hgvs_allele_ = make_shared<HgvsAllele>(*(var.hgvs_allele_.get()));
    } else if (var.vcf_allele_) {
      vcf_allele_ = make_shared<VcfAllele>(*(var.vcf_allele_.get()));
    }
    // todo
    if (var.zygosity_) {
      zygosity_ = make_shared<OntologyClass>(*(var.zygosity_));
    }


  }

  vector<Validation>
  Variant::validate() const {
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

  vector<Validation> Disease::validate() const {
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

  HtsFile::HtsFile(const org::phenopackets::schema::v1::core::HtsFile &htsfile):
    uri_(htsfile.uri())
  {
    switch(htsfile.hts_format()){
    case org::phenopackets::schema::v1::core::HtsFile_HtsFormat_BAM:
      hts_format_=HtsFormat::BAM; break;
    case org::phenopackets::schema::v1::core::HtsFile_HtsFormat_SAM:
      hts_format_=HtsFormat::SAM; break;
    case org::phenopackets::schema::v1::core::HtsFile_HtsFormat_CRAM:
      hts_format_=HtsFormat::CRAM; break;
    case org::phenopackets::schema::v1::core::HtsFile_HtsFormat_VCF:
      hts_format_=HtsFormat::VCF; break;
    case org::phenopackets::schema::v1::core::HtsFile_HtsFormat_BCF:
      hts_format_=HtsFormat::BCF; break;
    case org::phenopackets::schema::v1::core::HtsFile_HtsFormat_GVCF:
      hts_format_=HtsFormat::GVCF; break;
    default:
      hts_format_ = HtsFormat::UNKNOWN;
    }
    genome_assembly_ = htsfile.genome_assembly();
    individual_to_sample_identifiers_ .insert(htsfile.individual_to_sample_identifiers().begin(),
					      htsfile.individual_to_sample_identifiers().end());
  }

  HtsFile::HtsFile(const HtsFile & hts):
    hts_format_(hts.hts_format_),
    genome_assembly_(hts.genome_assembly_){
  }


  vector<Validation> HtsFile::validate() const {
    vector<Validation> vl;
    if (uri_.empty()) {
      vl.emplace_back(Validation::createError(ValidationCause::HTSFILE_LACKS_URI));
    }
    if (hts_format_ == HtsFormat::UNKNOWN) {
      Validation v = Validation::createError(ValidationCause::UNIDENTIFIED_HTS_FILETYPE);
      vl.push_back(v);
    }
    if (genome_assembly_.empty()) {
      Validation v = Validation::createError(ValidationCause::LACKS_GENOME_ASSEMBLY);
      vl.push_back(v);
    }
    if (individual_to_sample_identifiers_.empty()) {
      Validation v = Validation::createWarning(ValidationCause::LACKS_SAMPLE_MAP);
      vl.push_back(v);
    }
    return vl;
  }

  void
  HtsFile::validate(vector<Validation> &vals) const
  {
    vector<Validation> v2 = validate();
    if (v2.empty()) return;
    vals.insert(vals.end(), v2.begin(), v2.end());
  }


std::ostream &operator<<(std::ostream& ost, const HtsFormat htsf)
{
  switch (htsf) {
    case HtsFormat::BAM: ost << "BAM"; break;
    case HtsFormat::BCF: ost << "BST"; break;
    case HtsFormat::CRAM: ost << "CRAM"; break;
    case HtsFormat::GVCF: ost << "GVCF"; break;
    case HtsFormat::SAM: ost << "SAM"; break;
    case HtsFormat::VCF: ost << "VCF"; break;
    default: ost << "UNKNOWN";
  }
  return ost;
}

  std::ostream &operator<<(std::ostream& ost, const HtsFile& htsfile){
    ost << htsfile.get_uri() << ", ";
    ost << htsfile.get_htsformat();
    ost << ", "<<htsfile.genome_assembly_ << " ";
    
    return ost;
  }



  vector<Validation>
  Resource::validate() const {
    vector<Validation> vl;
    if (id_.empty()) {
      Validation v = Validation::createError(ValidationCause::RESOURCE_LACKS_ID);
      vl.push_back(v);
    }
    if (name_.empty()) {
      Validation v = Validation::createError(ValidationCause::RESOURCE_LACKS_NAME);
      vl.push_back(v);
    }
    if (namespace_prefix_.empty()) {
      Validation v = Validation::createError(ValidationCause::RESOURCE_LACKS_NAMESPACE_PREFIX);
      vl.push_back(v);
    }
    if (url_.empty()) {
      Validation v = Validation::createError(ValidationCause::RESOURCE_LACKS_URL);
      vl.push_back(v);
    }
    if (version_.empty()) {
      Validation v = Validation::createError(ValidationCause::RESOURCE_LACKS_VERSION);
      vl.push_back(v);
    }
    if (iri_prefix_.empty()) {
      Validation v = Validation::createError(ValidationCause::RESOURCE_LACKS_IRI_PREFIX);
      vl.push_back(v);
    }
    return vl;
  }


  std::ostream &operator<<(std::ostream& ost, const Resource& resource) {
    ost << resource.name_ << ": " << resource.id_
	      << "(" << resource.namespace_prefix_ << ";" << resource.url_
	      << ";" << resource.version_  << ";" << resource.iri_prefix_ <<")";
    return ost;
  }



  MetaData::MetaData(const org::phenopackets::schema::v1::core::MetaData &md){
    google::protobuf::Timestamp tstamp = md.created();
    if (tstamp.IsInitialized()) {
      created_=google::protobuf::util::TimeUtil::ToString(tstamp);
    } else {
      created_= EMPTY_STRING;
    }
    created_by_ = md.created_by();
    submitted_by_ = md.submitted_by();
    phenopacket_schema_version_ = md.phenopacket_schema_version();
    if (md.resources_size()>0) {
      for (auto r : md.resources()) {
	      Resource res(r);
	      resources_.push_back(res);
      }
    }
    if (md.external_references_size()>0) {
      for (auto er : md.external_references()) {
	      ExternalReference extref(er);
	      external_references_.push_back(extref);
      }
    }
  }

  MetaData::MetaData(const MetaData & md):
    created_(md.created_),
    created_by_(md.created_by_),
    submitted_by_(md.submitted_by_),
    phenopacket_schema_version_(md.phenopacket_schema_version_){
    for (Resource r : md.resources_) {
      resources_.push_back(r); // creates copy
    }
    for (ExternalReference extref : md.external_references_) {
      external_references_.push_back(extref);
    }
  }


  /**
   * Extract a set of prefixes of the ontologies used in the MetaData, e.g., HP, GENO, ...
   */
  std::set<string>
  MetaData::get_prefixes() const
  {
    std::set<string> prefixes;
    for (Resource r : resources_) {
      string id = r.get_id();
      transform(id.begin(), id.end(), id.begin(), ::toupper); 
      prefixes.insert(id);
    }
    return prefixes;
  }


  vector<Validation>
  MetaData::validate() const {
    vector<Validation> vl;
    if (created_.empty()) {
      Validation v = Validation::createError(ValidationCause::METADATA_LACKS_CREATED_TIMESTAMP);
      vl.push_back(v);
    }
    if (created_by_.empty()) {
      Validation v = Validation::createError(ValidationCause::METADATA_LACKS_CREATED_BY);
      vl.push_back(v);
    }
    if (resources_.empty()) {
      Validation v = Validation::createError(ValidationCause::METADATA_LACKS_RESOURCES);
      vl.push_back(v);
    }
    // other elements are optional and so we do not validate them
    return vl;
  }

  void
  MetaData::validate(vector<Validation> &val) const
  {
    vector<Validation> v2 = validate();
    if (v2.empty()) return;
    val.insert(val.end(), v2.begin(), v2.end());
  }

  void
  MetaData::validate_ontology_prefixes(const std::set<string> prefixes, vector<Validation> &val) const
  {
    std::set<string> ontology_prefixes;
    for (Resource r : resources_) {
      string id = r.get_id();
      transform(id.begin(), id.end(), id.begin(), ::toupper); 
      ontology_prefixes.insert(id);
    }
    for (string pr : prefixes) {
      auto p = ontology_prefixes.find(pr);
      if (p == ontology_prefixes.end()) {
        Validation v = Validation::createError(ValidationCause::ONTOLOGY_NOT_IN_METADATA);
        val.push_back(v);
      }
    }
    for (string pr : ontology_prefixes) {
      auto p = prefixes.find(pr);
      if (p == prefixes.end()) {
        std::stringstream sstr;
        sstr << "MetaData includes superfluous entry for " << pr;
        Validation v = Validation::createWarning(ValidationCause::METADATA_HAS_SUPERFLUOUS_ONTOLOGY,sstr.str());
        val.push_back(v);
        cout << sstr.str();
      } else {
        cout << " ALl is OK for " << pr << "\n";
      }
    }
  }

  std::ostream &operator<<(std::ostream& ost, const MetaData& md){
    ost << md.created_by_ << "(" << md.created_ << ")\n";
    for (Resource r : md.resources_) {
      ost << r << "\n";
    }
    return ost;
  }




  Procedure::Procedure(const org::phenopackets::schema::v1::core::Procedure & procedure){
    if (procedure.has_code()) {
      code_ = make_unique<OntologyClass>(procedure.code());
    }
    if (procedure.has_body_site()) {
      code_ = make_unique<OntologyClass>(procedure.body_site());
    }
  }

  Procedure::Procedure(const Procedure & procedure){
    if (procedure.code_) {
      code_ = make_unique<OntologyClass>(*(procedure.code_));
    }
    if (procedure.body_site_) {
      code_ = make_unique<OntologyClass>(*(procedure.body_site_));
    }
  }


  vector<Validation>
  Procedure:: validate() const {
    vector<Validation> vl;
    if (! code_) {
      Validation v = Validation::createError(ValidationCause::PROCEDURE_LACKS_CODE);
      vl.push_back(v);
    } else {
      vector<Validation> v2 = code_->validate();
      if (! v2.empty()) {
      	vl.insert(vl.end(),v2.begin(),v2.end() );
      }
    }
    return vl;
  }

  std::ostream &operator<<(std::ostream& ost, const Procedure& procedure) {
    if (procedure.code_) ost << *(procedure.code_.get());
    if (procedure.body_site_) ost <<"; body site: "<< *(procedure.body_site_.get());
    return ost;
  }


  Phenopacket::Phenopacket(const org::phenopackets::schema::v1::Phenopacket &pp):
    id_(pp.id()) {
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
    if (pp.hts_files_size()>0) {
      for (auto h : pp.hts_files()) {
	       HtsFile hts(h);
	        htsFiles_.push_back(hts);
      }
    }
    if (pp.has_meta_data()) {
      metadata_ = make_shared<MetaData>(pp.meta_data());
    }
  }



  vector<Validation>
  Phenopacket::validate() const {
    vector<Validation> vl;
    if (id_.empty()) {
      Validation v = Validation::createError(ValidationCause::PHENOPACKET_LACKS_ID);
      vl.push_back(v);
    }
    // check the subject
    if (! subject_) {
      Validation v = Validation::createWarning(ValidationCause::PHENOPACKET_LACKS_SUBJECT);
      vl.push_back(v);
    } else {
      vector<Validation> v2 = subject_->validate();
      if (! v2.empty() ){
	       vl.insert(vl.end(),v2.begin(),v2.end());
      }
    }
    // phenotypic_features: recommended
    if (phenotypic_features_.empty()) {
      vl.push_back(Validation::createWarning(ValidationCause::PHENOPACKET_LACKS_PHENOTYPIC_FEATURES));
    } else {
        for (auto pf : phenotypic_features_) {
            pf.validate(vl);
        }
    }
    if (!biosamples_.empty()) {
        for (const auto &p : biosamples_) {
			       p.validate(vl);
        }
    }
    if (!genes_.empty()) {
         for (const auto &p : genes_) {
            p.validate(vl);
        }
    }
    if (!variants_.empty()) {
         for (const auto &p : variants_) {
            p.validate(vl);
        }
    }
    if (!diseases_.empty()) {
         for (const auto &p : diseases_) {
            p.validate(vl);
        }
    }
    if (!htsFiles_.empty()) {
         for (const auto &p : htsFiles_) {
            p.validate(vl);
        }
    }
    if (! metadata_) {
     vl.push_back( Validation::createError(ValidationCause::PHENOPACKET_LACKS_METADATA) );
    } else {
		   metadata_->validate(vl);
    }
    return vl;
  }

  /**
   *  Check that all ontology terms use ontologies that are
    * listed inthe Metadata
    *assumption -- the prefix can be used to match uniquely
    */
  void
  Phenopacket::validate_metadata_prefixes(vector<Validation> &valids) const
  {
    std::set<string> prefixes;
    // taxon prefix
    if (subject_->get_taxon() != nullptr) {
      auto oc = subject_->get_taxon().get();
      prefixes.insert(oc->get_prefix());
    }
    if (diseases_.size()>0) {
      for (Disease d : diseases_) {
        auto oc = d.get_term();
        prefixes.insert(oc->get_prefix());
        if (d.has_class_of_onset()){
          oc = d.get_class_of_onset();
          prefixes.insert(oc->get_prefix());
        }
      }
    }
    // HPO prefix (or other phenotype ontology)
    for (PhenotypicFeature pf : phenotypic_features_) {
      string termid = pf.get_id();
      std::size_t i = termid.find_last_of(':');
      string prefix = termid.substr(0,i);
      prefixes.insert(prefix);
    }
    if (variants_.size()>0) {
      for (Variant v : variants_) {
        if (v.has_zygosity()) {}
          auto z = v.get_zygosity();
          prefixes.insert(z->get_prefix());
      }
    }
    std::set<string> metadata_prefixes = metadata_->get_prefixes();
    // are the ontology prefixes used in phenopacket that are not
    // mentioned in the MetaData?
    for (string s : prefixes) {
      auto p = metadata_prefixes.find(s);
      if (p == metadata_prefixes.end()) {
        std::stringstream sstr;
        sstr<< "Metadata did not contain ontology used in phenopacket: " << s;
        Validation v = Validation::createError(ValidationCause::METADATA_DOES_NOT_CONTAIN_ONTOLOGY,sstr.str());
        valids.push_back(v);
      } 
    }
    // Are there ontologies in the MetaData that are not used in the phenopacket?
    for (string s : metadata_prefixes) {
      auto p = prefixes.find(s);
      if (p == prefixes.end()) {
        std::stringstream sstr;
        sstr << "Metadata contains ontolog not used in Phenopacket: " << s;
        Validation v = Validation::createWarning(ValidationCause::METADATA_HAS_SUPERFLUOUS_ONTOLOGY,sstr.str());
        valids.push_back(v);
      } 
    }
  }


  std::ostream& operator<<(std::ostream& ost, const Individual& ind){
    ost << "ID: "<<ind.id_ << "\n";
    if (ind.age_ != nullptr) {
      ost << "Age: "<<*(ind.age_.get()) <<"\n";
    } else {
      ost << "Age: unknown\n";
    }
    
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
    if (ppacket.metadata_) {
      ost<<"Metadata:\n";
      ost << *(ppacket.metadata_.get()) << "\n";
    }
    return ost;
  }

  /**
   * Check whether all HPO terms used are present in the ontology. Check that no two terms
   * have ancestor descendant relation to each other.
   */
  vector<Validation>
  Phenopacket::semantically_validate(const std::unique_ptr<Ontology> &ontology_p) const
  {
    vector<Validation> validation;
    // collect the observed and excluded HP terms as TermId lists
    vector<TermId> observed;
    vector<TermId> excluded;
    for (auto pf : phenotypic_features_) {
      string id = pf.get_id();
      TermId tid = TermId::from_string(id);
      if (pf.is_negated()) {
        excluded.push_back(tid);
      } else {
        observed.push_back(tid);
      }
      // check whether the term is represented in the Ontology
      std::optional<Term> term_opt = ontology_p->get_term(tid);
      if (! term_opt) {
        std::stringstream sstr;
        sstr << "[ERROR] Could not find " << tid.get_value()
            << " in the ontology";
        Validation v = Validation::createError(ValidationCause::UNRECOGNIZED_TERMID, sstr.str());
        validation.push_back(v);
      }
    }
    for (auto i =0u; i < observed.size(); i++) {
      for (auto j = i+1; j < observed.size(); j++) {
        if (ontology_p->exists_path(observed.at(i), observed.at(j), EdgeType::IS_A)) {
          // if we get here, then the phenopacket includes
          // two terms which are ancestor-descendent to each other
          std::stringstream sstr;
          sstr << "[ERROR] Redundant terms: ";
          string label_i = ontology_p->get_term(observed.at(i))->get_label();
          string label_j = ontology_p->get_term(observed.at(j))->get_label();
          sstr << observed.at(i) << "(" << label_i << ")";
          sstr << " is a subclass of " << observed.at(j) << "(" << label_j << ")";
          Validation v = Validation::createError(ValidationCause::REDUNDANT_ANNOTATION, sstr.str());
          validation.push_back(v);
        }
        if (ontology_p->exists_path(observed.at(j), observed.at(i), EdgeType::IS_A)) {
          std::stringstream sstr;
          sstr << "[ERROR] Redundant terms: ";
          string label_i = ontology_p->get_term(observed.at(i))->get_label();
          string label_j = ontology_p->get_term(observed.at(j))->get_label();
          sstr << observed.at(j) << "(" << label_j << ")";
          sstr << " is a subclass of " << observed.at(i) << "(" << label_i << ")";
          Validation v = Validation::createError(ValidationCause::REDUNDANT_ANNOTATION, sstr.str());
          validation.push_back(v);
        }
      }
    }
    
    validate_metadata_prefixes(validation);
    if (validation.empty()) {
      std::cout << "[INFO] Semantic validation identified no errors.\n";
    } else {
      std::cout << "[INFO] Semantic validation identified " << validation.size() <<" issues\n";
    }
    return validation;

  }


  vector<Validation>
  Biosample::validate() const {
    vector<Validation> vl;
    if (id_.empty()) {
      vl.push_back(Validation::createError(ValidationCause::BIOSAMPLE_LACKS_ID));
    }
    //dataset_id_, individual_id_, description_ are optional and thus are not validated
    if (! sampled_tissue_) {
      vl.push_back(Validation::createError(ValidationCause::BIOSAMPLE_LACKS_SAMPLED_TISSUE));
    } else {
      vector<Validation> v2 = sampled_tissue_->validate();
      if (! v2.empty() ){
        vl.insert(vl.end(),v2.begin(),v2.end());
      }
    }
    if (! phenotypic_features_.empty() ){
      vl.push_back(Validation::createWarning(ValidationCause::BIOSAMPLE_LACKS_PHENOTYPES));
    } else {
      for (auto pf : phenotypic_features_){
        vector<Validation> v2 = pf.validate();
        if (! v2.empty() ){
          vl.insert(vl.end(),v2.begin(),v2.end());
        }
      }
    }
    //taxonomy_ is optional
     if (! age_of_individual_at_collection_ && ! age_range_of_individual_at_collection_) {
        vl.push_back(Validation::createWarning(ValidationCause::BIOSAMPLE_LACKS_AGE));
    } else if (age_of_individual_at_collection_) {
         vector<Validation> v2 = age_of_individual_at_collection_->validate();
         if (! v2.empty() ){
          vl.insert(vl.end(),v2.begin(),v2.end());
        }
    } else {
        vector<Validation> v2 = age_range_of_individual_at_collection_->validate();
         if (! v2.empty() ){
          vl.insert(vl.end(),v2.begin(),v2.end());
        }
    }
    if (! histological_diagnosis_ ) {
        vl.push_back(Validation::createWarning(ValidationCause::BIOSAMPLE_LACKS_HISTOLOGY));
    } else {
         histological_diagnosis_->validate(vl);
    }
    if (! tumor_progression_) {
        vl.push_back(Validation::createWarning(ValidationCause::BIOSAMPLE_LACKS_TUMOR_PROGRESSION));
    } else {
        tumor_progression_->validate(vl);
    }
    if (! tumor_grade_) {
        vl.push_back(Validation::createWarning(ValidationCause::BIOSAMPLE_LACKS_TUMOR_GRADE));
    } else {
        tumor_grade_->validate(vl);
    }
    if (tumor_stage_.empty()) {
        vl.push_back(Validation::createWarning(ValidationCause::BIOSAMPLE_LACKS_TUMOR_STAGE));
    } else {
        for (const auto & p : tumor_stage_) {
            p.validate(vl);
        }
    }
    if (diagnostic_markers_.empty()) {
        vl.push_back(Validation::createWarning(ValidationCause::BIOSAMPLE_LACKS_DIAGNOSTIC_MARKERS));
    } else {
        for (const auto &p : diagnostic_markers_) {
                p.validate(vl);
        }
    }
    // hts_files_ and variants_ are optional
    // is_control_sample is a bool with default false but it cannot be Q/C'd

    return vl;
  }

  void
  Biosample::validate(vector<Validation> &val) const
  {
    vector<Validation> v2 = validate();
    if (v2.empty()) return;
    val.insert(val.end(), v2.begin(), v2.end());
  }


} // namespace phenotools
