#ifndef PHENOTOOLS_H
#define PHENOTOOLS_H

#include <string>
#include <memory>
#include <vector>
#include <map>

#include "phenopackets.pb.h"
#include "base.pb.h"

using std::unique_ptr;
using std::string;
using std::vector;
using std::map;

namespace phenotools {

  enum class ValidationType { WARNING, ERROR };
  enum class ValidationCause {
    ONTOLOGY_ID_EMPTY, 
      INVALID_ONTOLOGY_ID,
      ONTOLOGY_LABEL_EMPTY, 
      AGE_ELEMENT_UNINITIALIZED,
      INDIVIDUAL_LACKS_ID,
      INDIVIDUAL_LACKS_AGE,
      UNKNOWN_SEX, 
      EXTERNAL_REFERENCE_LACKS_ID,
      EVIDENCE_LACKS_CODE, 
      PHENOTYPIC_FEATURE_LACKS_ONTOLOGY_TERM, 
      PHENOTYPIC_FEATURE_LACKS_EVIDENCE, 
      GENE_LACKS_ID, 
      GENE_LACKS_SYMBOL,
      ALLELE_LACKS_ID, 
      ALLELE_LACKS_HGVS, 
      LACKS_GENOME_ASSEMBLY,
      LACKS_CHROMOSOME, 
      LACKS_REF, 
      LACKS_ALT,
      LACKS_ZYGOSITY, 
      LACKS_ALLELE,
      DISEASE_LACKS_TERM,
      FILE_LACKS_SPECIFICATION,
      UNIDENTIFIED_HTS_FILETYPE,
      LACKS_SAMPLE_MAP, 
      LACKS_HTS_FILE, 
      RESOURCE_LACKS_ID,
      RESOURCE_LACKS_NAME,
      RESOURCE_LACKS_NAMESPACE_PREFIX, 
      RESOURCE_LACKS_URL,
      RESOURCE_LACKS_VERSION,
      RESOURCE_LACKS_IRI_PREFIX, 
      METADATA_LACKS_CREATED_TIMESTAMP,
      METADATA_LACKS_CREATED_BY, 
      METADATA_LACKS_RESOURCES,
      PROCEDURE_LACKS_CODE, 
      PHENOPACKET_LACKS_ID,
      PHENOPACKET_LACKS_SUBJECT,
      PHENOPACKET_LACKS_PHENOTYPIC_FEATURES, 
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
    friend std::ostream& operator<<(std::ostream& ost, const Validation& v); 
  };
  std::ostream& operator<<(std::ostream& ost, const Validation& v); 
  
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
    const string & get_id() const { return id_; }
    const string & get_label() const { return label_; }
    friend std::ostream& operator<<(std::ostream& ost, const OntologyClass& oc);  
  };
  std::ostream& operator<<(std::ostream& ost, const OntologyClass& oc); 
  
  
  class Age : public ValidatorI {
  private:
    /** An ISO8601 string such as P40Y6M (40 years and 6 months old).*/ 
    string age_;
    unique_ptr<OntologyClass> age_class_;
    
  public:
    Age()=default;
    Age(const org::phenopackets::schema::v1::core::Age &a);
    Age(const Age& age);
    ~Age(){}
    vector<Validation> validate();
    friend std::ostream& operator<<(std::ostream& ost, const Age& age);   
  };
  std::ostream& operator<<(std::ostream& ost, const Age& age);   
  
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
    friend std::ostream& operator<<(std::ostream& ost, const AgeRange& agerange);  
  };
  std::ostream& operator<<(std::ostream& ost, const AgeRange& agerange);  
  
  
  enum class Sex {
    UNKNOWN_SEX,
      FEMALE ,
      MALE,
      OTHER_SEX
      };
  std::ostream& operator<<(std::ostream& ost, const enum Sex& sex); 
  
  enum class KaryotypicSex {
    UNKNOWN_KARYOTYPE,
      XX,
      XY,
      XO,
      XXY,
      XXX,
      XXYY,
      XXXY,
      XXXX,
      XYY,
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
    const string & get_id() const { return id_; }
    friend std::ostream& operator<<(std::ostream& ost, const Individual& ind);
  };
  
  std::ostream& operator<<(std::ostream& ost, const Individual& ind);
  
  
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
    PhenotypicFeature(const org::phenopackets::schema::v1::core::PhenotypicFeature &pfeat);
    PhenotypicFeature(const PhenotypicFeature & pfeat);
    PhenotypicFeature &operator=(const PhenotypicFeature & pfeat) { std::cerr<<"TODO"; return *this; }
    ~PhenotypicFeature(){}
    vector<Validation> validate();
    const string &get_id() const { return type_->get_id(); }
    const string &get_label() const { return type_->get_label();}
  };
  
  
  class Gene : public ValidatorI {
  private:
    string id_;
    string symbol_;
  public:
  Gene(const org::phenopackets::schema::v1::core::Gene & gene):
    id_(gene.id()),
      symbol_(gene.symbol())
	{}
  Gene(const Gene &gene):id_(gene.id_),symbol_(gene.symbol_){}
    vector<Validation> validate();
    friend std::ostream &operator<<(std::ostream& ost, const Gene& gene);   
  };
  std::ostream &operator<<(std::ostream& ost, const Gene& gene);
  
  
  class HgvsAllele : public ValidatorI {
  private:
    string id_;
    string hgvs_;
  public:
  HgvsAllele(const org::phenopackets::schema::v1::core::HgvsAllele &ha):id_(ha.id()), hgvs_(ha.hgvs()) {}
  HgvsAllele(const HgvsAllele &ha):id_(ha.id_),hgvs_(ha.hgvs_){}
    HgvsAllele &operator=(const HgvsAllele &ha);
    vector<Validation> validate();
    friend std::ostream &operator<<(std::ostream& ost, const HgvsAllele& hgvs);
  };
  std::ostream &operator<<(std::ostream& ost, const HgvsAllele& hgvs);
  
  class VcfAllele : public ValidatorI {
  private:
    string genome_assembly_;
    string id_;
    string chr_;
    int pos_;
    string ref_;
    string alt_;
    string info_;
  public:
    VcfAllele(const org::phenopackets::schema::v1::core::VcfAllele &vcf);
    VcfAllele(const VcfAllele &vcf);
    vector<Validation> validate();
    friend std::ostream &operator<<(std::ostream& ost, const VcfAllele& vcf);
  };
  std::ostream &operator<<(std::ostream& ost, const VcfAllele& vcf);
  
  
  
  class Variant : public ValidatorI {
  private:
    unique_ptr<HgvsAllele> hgvs_allele_;
    unique_ptr<VcfAllele> vcf_allele_;
    // todo spdi, iscn,murine
    unique_ptr<OntologyClass> zygosity_;
    
  public:
    Variant(const org::phenopackets::schema::v1::core::Variant & var);
    Variant(const Variant & var);
    vector<Validation> validate();
    friend std::ostream &operator<<(std::ostream& ost, const Variant& var);
  };
  std::ostream &operator<<(std::ostream& ost, const Variant& var);

  
  class Disease : public ValidatorI {
  private:
    unique_ptr<OntologyClass> term_;
    unique_ptr<Age> age_of_onset_;
    unique_ptr<AgeRange> age_range_of_onset_;
    unique_ptr<OntologyClass> class_of_onset_;
    
  public:
    Disease(const org::phenopackets::schema::v1::core::Disease & dis);
    Disease(const Disease &dis);
    vector<Validation> validate();
    friend std::ostream &operator<<(std::ostream& ost, const Disease& dis);
  };
  std::ostream &operator<<(std::ostream& ost, const Disease& dis);
  
  
  class File : public ValidatorI {
  private:
    string path_;
    string uri_;
    string description_;
  public:
    File(const org::phenopackets::schema::v1::core::File &file);
    File(const File &file);
    vector<Validation> validate();
    friend std::ostream &operator<<(std::ostream& ost, const File& file);
  };
  std::ostream &operator<<(std::ostream& ost, const File& file);
  
  
  enum class HtsFormat { UNKNOWN, SAM, BAM, CRAM, VCF, BCF, GVCF };
  
  class HtsFile : public ValidatorI {
  private:
    HtsFormat hts_format_;
    string genome_assembly_;
    map<string,string> individual_to_sample_identifiers_;
    unique_ptr<File> file_;
    
  public:
    HtsFile(const org::phenopackets::schema::v1::core::HtsFile &htsfile);
    HtsFile(const HtsFile &htsfile);
    vector<Validation> validate();
    friend std::ostream &operator<<(std::ostream& ost, const HtsFile& htsfile);
  };
  std::ostream &operator<<(std::ostream& ost, const HtsFile& htsfile);
  
  
  
  class Resource : public ValidatorI {
  private:
    string id_;
    string name_;
    string namespace_prefix_;
    string url_;
    string version_;
    string iri_prefix_;
  public:
  Resource(const org::phenopackets::schema::v1::core::Resource & r):
    id_(r.id()),
      name_(r.name()),
      namespace_prefix_(r.namespace_prefix()),
      url_(r.url()),
      version_(r.version()),
      iri_prefix_(r.iri_prefix()){}
  Resource(const Resource & r):
    id_(r.id_),
      name_(r.name_),
      namespace_prefix_(r.namespace_prefix_),
      url_(r.url_),
      version_(r.version_),
      iri_prefix_(r.iri_prefix_){}
    vector<Validation> validate();
    friend std::ostream &operator<<(std::ostream& ost, const Resource& resource);
  };
  std::ostream &operator<<(std::ostream& ost, const Resource& resource);
  
  
  
  class MetaData : public ValidatorI {
  private:
    //  (timestamp, converted to RFC 3339 date string)
    string created_;
    string created_by_;
    string submitted_by_;
    vector<Resource> resources_;
    vector<string> updated_;
    string phenopacket_schema_version_;
    vector<ExternalReference> external_references_;
  public:
    MetaData(const org::phenopackets::schema::v1::core::MetaData &md);
    MetaData(const MetaData & md);
    vector<Validation> validate();
    friend std::ostream &operator<<(std::ostream& ost, const MetaData& md);
  };
  std::ostream &operator<<(std::ostream& ost, const MetaData& md);


  class Procedure : public ValidatorI  {
  private:  
    unique_ptr<OntologyClass> code_;
    unique_ptr<OntologyClass> body_site_;
    
  public:
    Procedure(const org::phenopackets::schema::v1::core::Procedure & procedure);
    Procedure(const Procedure & procedure);
    ~Procedure(){}
    vector<Validation> validate();
    friend std::ostream &operator<<(std::ostream& ost, const Procedure& procedure);
  };
  std::ostream &operator<<(std::ostream& ost, const Procedure& procedure);
  
  class Phenopacket : public ValidatorI {
  private:
    string id_;
    unique_ptr<Individual> subject_;
    vector<PhenotypicFeature> phenotypic_features_;
    vector<Gene> genes_;
    vector<Variant> variants_;
    vector<Disease> diseases_;
    vector<HtsFile> htsFiles_;
    unique_ptr<MetaData> metadata_;
    
  public:
    Phenopacket(const org::phenopackets::schema::v1::Phenopacket &pp) ;
    ~Phenopacket(){}
    vector<Validation> validate();
    friend std::ostream& operator<<(std::ostream& ost, const Phenopacket& ppacket);   
  };
  
  std::ostream& operator<<(std::ostream& ost, const Phenopacket& ppacket);   




}


#endif
