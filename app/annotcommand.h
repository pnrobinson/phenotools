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
        AnnotationCommand(const string &path, const string &hp_json, const string &date, const string &enddate, const string &termid, const string &outpath);
        AnnotationCommand(const string &path, const string &hp_json, const string &date, const string &enddate, const string &termid);
        virtual int execute();


        private:
            string phenotype_hpoa_path;
            string hp_json_path;
            string termid_;
            string date_;
            string enddate_;
            string outpath_;
            std::unique_ptr<struct tm> start_date_;
            std::unique_ptr<struct tm> end_date_;
            vector<HpoAnnotation> annotations;
            bool in_time_window(tm time) const;
            void output_descendants(std::ostream & ost);


    };

};


#endif