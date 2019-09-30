

#include "myexception.h"

const char*
PhenopacketException::what() const throw() {
    return what_message.c_str();
}

const char*
JsonParseException::what() const throw() {
    return what_message.c_str();
}
