#ifndef PHENOTOOLS_COMMAND_H
#define PHENOTOOLS_COMMAND_H


#include <ctime>
#include <string>
using std::string;

namespace phenotools {

    class PhenotoolsCommand{

        public:
            virtual int execute() = 0;
            PhenotoolsCommand();

        protected:
                // "2014-11-12T19:12:14.505Z"
            struct tm string_to_time(string iso8601date) const;
        

    };

};











#endif