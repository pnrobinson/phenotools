

#include "myexception.h"



JsonParseException::JsonParseException(char const* const message)
{
    what_message = message;
}

JsonParseException::JsonParseException(const std::string & message)
{
     what_message = message;
}


const char*
PhenopacketException::what() const throw() {
    return what_message.c_str();
}

const char*
JsonParseException::what() const throw() {
    return what_message.c_str();
}
