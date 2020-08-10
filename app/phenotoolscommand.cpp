
#include "phenotoolscommand.h"
#include <iostream>

using namespace phenotools;

PhenotoolsCommand::PhenotoolsCommand()
{
    // no -op
}


struct tm 
PhenotoolsCommand::string_to_time(string iso8601date) const
{
    tm time;
    int y,M,d,h,m;
    float s;
    sscanf(iso8601date.c_str(), "%d-%d-%dT%d:%d:%fZ", &y, &M, &d, &h, &m, &s);
    time.tm_year = y - 1900; // Year since 1900
    time.tm_mon = M - 1;     // 0-11
    time.tm_mday = d;        // 1-31
    return time;
}