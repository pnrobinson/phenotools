

#include "jsonparse_exception.h"


const char*
JsonParseException::what() const throw() {
    return what_message.c_str();
}
