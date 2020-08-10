#ifndef ANNOTATION_COMMAND_H
#define ANNOTATION_COMMAND_H

#include <string>
#include <vector>

using std::string;
using std::vector;

#include "phenotoolscommand.h"
#include "../lib/hpoannotation.h"


using std::string;

namespace phenotools {

    class AnnotationCommand : public PhenotoolsCommand {

        public:
        AnnotationCommand(const string &path, const string &hp_json, const string &date, const string &termid);
        virtual int execute();


        private:
            string phenotype_hpoa_path;
            string hp_json_path;
            string termid_;
            string date_;
            vector<HpoAnnotation> annotations;


    };

};


#endif