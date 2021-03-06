#ifndef PHENOTOOLS_H
#define PHENOTOOLS_H

#include <string>
#include <memory>
#include <vector>
#include <set>
#include <map>
#include <locale>         // std::locale, std::toupper

#include "phenopackets.pb.h"
#include "base.pb.h"
#include "ontology.h"

using std::unique_ptr;
using std::shared_ptr;
using std::make_shared;
using std::string;
using std::vector;
using std::map;
using std::ostream;

namespace phenotools {

  /*
  static std::set<string> prefix_set;
  void register_ontology_prefix(string prefix) {
    prefix_set.insert(prefix);
  }*/

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
      UNIDENTIFIED_HTS_FILETYPE,
      HTSFILE_LACKS_URI,
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
      METADATA_DOES_NOT_CONTAIN_ONTOLOGY,
      METADATA_HAS_SUPERFLUOUS_ONTOLOGY,
      PROCEDURE_LACKS_CODE,
      PHENOPACKET_LACKS_ID,
      PHENOPACKET_LACKS_SUBJECT,
      PHENOPACKET_LACKS_PHENOTYPIC_FEATURES,
      PHENOPACKET_LACKS_METADATA,
      BIOSAMPLE_LACKS_ID,
      BIOSAMPLE_LACKS_SAMPLED_TISSUE,
      BIOSAMPLE_LACKS_PHENOTYPES,
      BIOSAMPLE_LACKS_AGE,
      BIOSAMPLE_LACKS_HISTOLOGY,
      BIOSAMPLE_LACKS_TUMOR_PROGRESSION,
      BIOSAMPLE_LACKS_TUMOR_GRADE,
      BIOSAMPLE_LACKS_TUMOR_STAGE,
      BIOSAMPLE_LACKS_DIAGNOSTIC_MARKERS,
      ONTOLOGY_NOT_IN_METADATA,
      REDUNDANT_ANNOTATION,
      UNRECOGNIZED_TERMID,
      };
  static const string EMPTY="";// use for elements that are not present in the phenopacket input

  class Validation {
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
    friend ostream& operator<<(ostream& ost, const Validation& v);
  };
  ostream& operator<<(ostream& ost, const Validation& v);

  class ValidatorI {
  public:
    virtual ~ValidatorI(){}
    virtual vector<Validation> validate() const = 0;
    void validate(vector<Validation> &v) const;
  };

  class OntologyClass : public ValidatorI {
  private:
    string id_;
    string label_;
  public:
    OntologyClass(const string &id, const string &label):
      id_(id),label_(label){}
    OntologyClass(org::phenopackets::schema::v1::core::OntologyClass ontclz):
      id_(ontclz.id()), label_(ontclz.label()) {}
    OntologyClass(const OntologyClass & from):id_(from.id_),label_(from.label_){}
    ~OntologyClass(){}
    vector<Validation> validate() const override;
    void validate(vector<Validation> &v) const;
    const string & get_id() const { return id_; }
    const string & get_label() const { return label_; }
    string get_prefix() const;
    friend std::ostream& operator<<(std::ostream& ost, const OntologyClass& oc);
  };
  ostream& operator<<(ostream& ost, const OntologyClass& oc);


  class Age : public ValidatorI {
  private:
    /** An ISO8601 string such as P40Y6M (40 years and 6 months old).*/
    string age_;

  public:
    Age()=default;
    Age(const org::phenopackets::schema::v1::core::Age &a);
    Age(const Age& age);
    ~Age(){}
    vector<Validation> validate() const override;
    void validate(vector<Validation> &v) const;
    friend ostream& operator<<(ostream& ost, const Age& age);
  };
  ostream& operator<<(ostream& ost, const Age& age);

  class AgeRange : public ValidatorI {
  private:
    Age start_;
    Age end_;
  public:
  AgeRange(const org::phenopackets::schema::v1::core::AgeRange &ar):
    start_(ar.start()),
      end_(ar.end()) {}
    ~AgeRange(){}
    vector<Validation> validate() const override;
    void validate(vector<Validation> &v) const {}
    friend ostream& operator<<(std::ostream& ost, const AgeRange& agerange);
  };
  ostream& operator<<(std::ostream& ost, const AgeRange& agerange);


  enum class Sex {
    UNKNOWN_SEX,
      FEMALE ,
      MALE,
      OTHER_SEX
      };
  ostream& operator<<(ostream& ost, const enum Sex& sex);

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


  class Individual {
  private:
    /** Required, arbitrary (application-dependent) identifier. */
    string id_;
    // optional
    vector<string> alternate_ids_;
    /** optional (timestamp, converted to RFC 3339 date string). */
    string date_of_birth_;
    // optional
    unique_ptr<Age> age_;
    // optional
    unique_ptr<AgeRange> age_range_;
    enum Sex sex_;
    enum KaryotypicSex karyotypic_sex_;
    shared_ptr<OntologyClass> taxonomy_ptr_;

  public:
    Individual(org::phenopackets::schema::v1::core::Individual individual);
    ~Individual(){}
    vector<Validation> validate() const;
    void validate(vector<Validation> &v) const;
    const string & get_id() const { return id_; }
    shared_ptr<OntologyClass> get_taxon() const { return taxonomy_ptr_; }
    friend ostream& operator<<(ostream& ost, const Individual& ind);
  };

  ostream& operator<<(ostream& ost, const Individual& ind);


  class ExternalReference {
  private:
    string id_;
    string description_;
  public:
  ExternalReference(org::phenopackets::schema::v1::core::ExternalReference er):
    id_(er.id()),
      description_(er.description()) {}
    ~ExternalReference(){}
    vector<Validation> validate() const;
    void validate(vector<Validation> &v) const;
  };

  class Evidence {
  private:
    unique_ptr<OntologyClass> evidence_code_;
    unique_ptr<ExternalReference> reference_;
  public:
    Evidence(org::phenopackets::schema::v1::core::Evidence evi);
    Evidence(const Evidence& from);
    Evidence(Evidence &&);
    Evidence & operator=(const Evidence &);
    ~Evidence(){}
    vector<Validation> validate() const;
    void validate(vector<Validation> &v) const;
  };


  class PhenotypicFeature {
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
    vector<Validation> validate() const;
    void validate(vector<Validation> &v) const;
    const string &get_id() const { return type_->get_id(); }
    const string &get_label() const { return type_->get_label(); }
    bool is_negated() const { return negated_; }
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
    vector<Validation> validate() const;
    void validate(vector<Validation> &v) const {}
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
    vector<Validation> validate() const;
    void validate(vector<Validation> &v) const {}
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
    vector<Validation> validate() const;
    void validate(vector<Validation> &v) const {}
    friend std::ostream &operator<<(std::ostream& ost, const VcfAllele& vcf);
  };
  std::ostream &operator<<(std::ostream& ost, const VcfAllele& vcf);



  class Variant : public ValidatorI {
  private:
    shared_ptr<HgvsAllele> hgvs_allele_;
    shared_ptr<VcfAllele> vcf_allele_;
    // todo spdi, iscn
    shared_ptr<OntologyClass> zygosity_;

  public:
    Variant(const org::phenopackets::schema::v1::core::Variant & var);
    Variant(const Variant & var);
    vector<Validation> validate() const;
    void validate(vector<Validation> &v) const {}
    bool has_zygosity() const { return nullptr != zygosity_; }
    shared_ptr<OntologyClass> get_zygosity() const { return zygosity_; }
    friend std::ostream &operator<<(std::ostream& ost, const Variant& var);
  };
  std::ostream &operator<<(std::ostream& ost, const Variant& var);


  class Disease : public ValidatorI {
  private:
    shared_ptr<OntologyClass> term_;
    shared_ptr<Age> age_of_onset_;
    shared_ptr<AgeRange> age_range_of_onset_;
    shared_ptr<OntologyClass> class_of_onset_;

  public:
    Disease(const org::phenopackets::schema::v1::core::Disease & dis);
    Disease(const Disease &dis);
    vector<Validation> validate() const;
    void validate(vector<Validation> &v) const {}
    shared_ptr<OntologyClass> get_term() const { return term_; }
    bool has_age_of_onset() const { return nullptr != age_of_onset_; }
    shared_ptr<Age> get_age_of_onset() const { return age_of_onset_; }
    bool has_age_range_of_onset() const { return nullptr != age_range_of_onset_; }
    shared_ptr<AgeRange> get_age_range_of_onset() const { return age_range_of_onset_; }
    bool has_class_of_onset() { return nullptr != class_of_onset_; }
    shared_ptr<OntologyClass> get_class_of_onset() const { return class_of_onset_; }
    friend std::ostream &operator<<(std::ostream& ost, const Disease& dis);
  };
  std::ostream &operator<<(std::ostream& ost, const Disease& dis);

  enum class HtsFormat { UNKNOWN, SAM, BAM, CRAM, VCF, BCF, GVCF };
  std::ostream &operator<<(std::ostream& ost, const HtsFormat htsf);

  class HtsFile {
  private:
    string uri_;
    string description_;
    HtsFormat hts_format_;
    string genome_assembly_;
    map<string,string> individual_to_sample_identifiers_;

  public:
    HtsFile(const org::phenopackets::schema::v1::core::HtsFile &htsfile);
    HtsFile(const HtsFile &htsfile);
    vector<Validation> validate() const;
    void validate(vector<Validation> &v) const;
    string get_uri() const { return uri_; }
    string get_description() const { return description_; }
    HtsFormat get_htsformat() const { return hts_format_; }
    friend std::ostream &operator<<(std::ostream& ost, const HtsFile& htsfile);
  };
  std::ostream &operator<<(std::ostream& ost, const HtsFile& htsfile);



  class Resource {
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
    vector<Validation> validate() const;
    void validate(vector<Validation> &v) const;
    /** @return the identifier (prefix) of this ontology, e.g., HP. */
    string get_id() const { return id_; }
    string get_namespace_prefix() const { return namespace_prefix_; }
    friend std::ostream &operator<<(std::ostream& ost, const Resource& resource);
  };
  std::ostream &operator<<(std::ostream& ost, const Resource& resource);



  class MetaData : public ValidatorI {
  private:
    //  (timestamp, converted to RFC 3339 date string)
    string created_;
    /** Name/id of the person who created this Phenopacket. */
    string created_by_;
    /** Name/id of the person who submitted this Phenopacket. */
    string submitted_by_;
    vector<Resource> resources_;
    string phenopacket_schema_version_;
    vector<ExternalReference> external_references_;
  public:
    MetaData(const org::phenopackets::schema::v1::core::MetaData &md);
    MetaData(const MetaData & md);
    vector<Validation> validate() const;
    void validate(vector<Validation> &val) const;
    void validate_ontology_prefixes(const std::set<string> prefixes, vector<Validation> &val) const;
    std::set<string> get_prefixes() const;
    friend std::ostream &operator<<(std::ostream& ost, const MetaData& md);
  };
  std::ostream &operator<<(std::ostream& ost, const MetaData& md);


  class Procedure  {
  private:
    unique_ptr<OntologyClass> code_;
    unique_ptr<OntologyClass> body_site_;

  public:
    Procedure(const org::phenopackets::schema::v1::core::Procedure & procedure);
    Procedure(const Procedure & procedure);
    ~Procedure(){}
    vector<Validation> validate() const;
    friend std::ostream &operator<<(std::ostream& ost, const Procedure& procedure);
  };
  std::ostream &operator<<(std::ostream& ost, const Procedure& procedure);

  class Biosample  : public ValidatorI {
   private:
    string id_;
    string dataset_id_;
    string individual_id_;
    string description_;
    unique_ptr<OntologyClass> sampled_tissue_;
    vector<PhenotypicFeature> phenotypic_features_;
    unique_ptr<OntologyClass> taxonomy_;
    // one of the following two age elements is required
    unique_ptr<Age> age_of_individual_at_collection_;
    unique_ptr<AgeRange> age_range_of_individual_at_collection_;
    unique_ptr<OntologyClass> histological_diagnosis_;
    unique_ptr<OntologyClass> tumor_progression_;
    unique_ptr<OntologyClass> tumor_grade_;
    vector<OntologyClass> tumor_stage_;
    vector<OntologyClass> diagnostic_markers_;
    unique_ptr<Procedure> procedure_;
    vector<HtsFile> hts_files_;
    vector<Variant> variants_;
    bool is_control_sample_;

  public:
    Biosample(const org::phenopackets::schema::v1::core::Biosample &bsample);
    ~Biosample(){}
    vector<Validation> validate() const;
    void validate(vector<Validation> &val) const;
    friend std::ostream& operator<<(std::ostream& ost, const Biosample& bsample);
  };

  std::ostream& operator<<(std::ostream& ost, const Biosample& bsample);

  class Phenopacket {
  private:
    string id_;
    /** The subject (e.g., index patient) of this Phenopacket. */
    unique_ptr<Individual> subject_;
    /** A list of phenotypic abnormalities observed or explicitly excluded in the subject.*/
    vector<PhenotypicFeature> phenotypic_features_;
    vector<Biosample> biosamples_;
    vector<Gene> genes_;
    vector<Variant> variants_;
    vector<Disease> diseases_;
    vector<HtsFile> htsFiles_;
    shared_ptr<MetaData> metadata_;
    /** Check the semantics and consistency of the phenopacket. */
    void validate_metadata_prefixes(vector<Validation> &v) const;

  public:
    Phenopacket(const org::phenopackets::schema::v1::Phenopacket &pp) ;
    ~Phenopacket(){}
    vector<Validation> validate() const;
    vector<Validation> semantically_validate(const std::unique_ptr<Ontology> &ptr) const;
    void validate(vector<Validation> &v) const {}
    friend std::ostream& operator<<(std::ostream& ost, const Phenopacket& ppacket);
  };

  std::ostream& operator<<(std::ostream& ost, const Phenopacket& ppacket);





}


#endif
