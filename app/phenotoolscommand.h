#ifndef PHENOTOOLS_COMMAND_H
#define PHENOTOOLS_COMMAND_H


#include <ctime>
#include <string>
#include <vector>
#include <memory>
#include <optional>

using std::string;
using std::vector;
#include "../lib/termid.h"
#include "../lib/ontology.h"

namespace phenotools {

    class PhenotoolsCommand{
        private:
            string hp_json_path_;
        public:
            virtual int execute() = 0;
            PhenotoolsCommand();
        protected:
            PhenotoolsCommand(const string & hp_json);
            std::unique_ptr<Ontology> ontology_;
            // "2014-11-12T19:12:14.505Z"
            struct tm string_to_time(string iso8601date) const;
            vector<TermId> toplevelCatories_;
            /** A list of errors, if any, encountered while parsing the input file.*/
	        vector<string> error_list_;

            void init_toplevel_categories();
            std::optional<TermId> get_toplevel(const TermId &tid) const;
    };


    class PhenopacketCommand : public PhenotoolsCommand {
        private:
        string hp_json_path;
        string phenopacket_path;
        
        public:
        PhenopacketCommand(const string &phenopacket, const string &hp_json);
          virtual int execute();
    };

};











#endif