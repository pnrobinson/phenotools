#ifndef ANNOTATION_COMMAND_H
#define ANNOTATION_COMMAND_H

#include <map>
#include <memory>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::vector;

#include "phenotoolscommand.h"
#include "../lib/hpoannotation.h"
#include "../lib/ontology.h"


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
            bool do_by_toplevel_category_ = false;
            std::unique_ptr<struct tm> start_date_;
            std::unique_ptr<struct tm> end_date_;
            vector<HpoAnnotation> annotations_;
            bool in_time_window(tm time) const;
            void process_by_top_level_categories() const;
            void output_descendants(std::ostream & ost);
            void output_annotation_stats(std::ostream & ost) const;
            int output_annotation_stats_per_database(std::ostream & ost, const map<string, int> &annotmap, const string &dbasename) const;
            static string DEFAULT_OUTFILE_NAME;
    };

};


#endif