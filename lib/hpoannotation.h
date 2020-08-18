#ifndef HPO_ANNOTATION_H
#define HPO_ANNOTATION_H

#include <string>
#include <vector>
#include <memory>
#include <ctime>
#include <iostream>

#include "termid.h"

using std::string;
using std::vector;
using std::unique_ptr;

namespace phenotools {

    class Biocuration {
        private:
            string curator_;
            tm curation_date_;
        public:
            Biocuration(const string &curation);
            string get_curator() const { return curator_; }
            tm get_curation_date() const { return curation_date_; }
            string to_string() const;
    };

    class HpoAnnotation {
    private:
        unique_ptr<TermId> disease_id_;
        string disease_name_;
        bool negated_;
        unique_ptr<TermId> hpo_id_;
        vector<Biocuration> curations_;



    public:
         HpoAnnotation(const string &line);
         HpoAnnotation(const HpoAnnotation &annot);
         static vector<HpoAnnotation> parse_phenotype_hpoa(const string &path);
         bool is_omim() const;
         TermId get_disease_id() const;
         string get_disease_name() const;
         string get_negated() const;
         TermId get_hpo_id() const;
         string get_biocuration_string() const;
         tm get_curation_date() const;
        friend std::ostream& operator<<(std::ostream& ost, const HpoAnnotation& annot);
        static tm DEFAULT_CREATION_DATE;
    };
    std::ostream& operator<<(std::ostream& ost, const HpoAnnotation& annot);
};
#endif
