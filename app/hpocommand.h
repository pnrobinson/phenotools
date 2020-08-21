#ifndef HPOCOMMAND_H
#define HPOCOMMAND_H

#include <string>
#include <memory>
#include <vector>
#include <ctime>

#include "../lib/phenotools.h"
#include "phenotoolscommand.h"
#include "../lib/ontology.h"
#include "../lib/termid.h"


using std::vector;
using std::string;

namespace phenotools {
/**
 * Interface for the command classes.
 */
class HpoCommand : public PhenotoolsCommand {

    public:
      HpoCommand(const string &hp_json_path, 
                bool descriptive_stats, 
                bool quality_control,
                const string &date,
                const string &end_date,
                const string &termid,
                bool debug,
                const string &outpath);
      HpoCommand(const string &hp_json_path, 
                bool descriptive_stats, 
                bool quality_control,
                const string &date,
                const string &end_date,
                const string &termid,
                bool debug);
      HpoCommand(const string &hp_json_path, const string &hpo_term_file, const string &outpath);
      virtual int execute();
      /** path: file with one HPO term per line. outpath:name of file to print the ther together with its top level category. */
      void print_category(const string &path, const string &outpath) const;

    private:
      bool show_descriptive_stats;
      bool show_quality_control;
      std::unique_ptr<struct tm> start_date_;
      std::unique_ptr<struct tm> end_date_;
      string threshold_date_str;
      string end_date_str;
      std::unique_ptr<TermId> tid_;
      bool debug_;
      string hpo_termfile_;
      string outpath_;
      bool do_term_annotation_ = false;
      

      void show_qc();
      void show_stats();
      void count_descendants();
      void output_descendants(std::ostream & ost);
      bool in_time_window(tm time) const;
      void annotate_termfile() const;
      void output_terms_by_category() const;
     

};

};


#endif