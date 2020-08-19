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
      virtual int execute();

    private:
      std::unique_ptr<Ontology> ontology;
      /** A list of errors, if any, encountered while parsing the input file.*/
	  vector<string> error_list_;
      bool show_descriptive_stats;
      bool show_quality_control;
      std::unique_ptr<struct tm> start_date_;
      std::unique_ptr<struct tm> end_date_;
      string threshold_date_str;
      string end_date_str;
      std::unique_ptr<TermId> tid_;
      bool debug_;
      string outpath_;

      void show_qc();
      void show_stats();
      void count_descendants();
      void output_descendants(std::ostream & ost);
      bool in_time_window(tm time) const;
};

};


#endif