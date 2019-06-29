#ifndef JSONPARSE_EXCEPTION_H
#define JSONPARSE_EXCEPTION_H


#include <exception>
#include <string>

/**
 * Use for Parsing errors from the JSON.
 * catch (const JsonParseException& e)
 */
class JsonParseException : public std::exception {
private:
    std::string what_message;

public:
    JsonParseException(char const* const message):what_message(message){}
     JsonParseException(const std::string & message):what_message(message){}
    const char* what() const throw();
};



#endif
