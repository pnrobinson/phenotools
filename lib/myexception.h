

#ifndef MY_EXCEPTION_H
#define MY_EXCEPTION_H


#include <exception>
#include <string>

/**
 * Use for Parsing errors from the JSON.
 * catch (const JsonParseException& e)
 */
class PhenopacketException : public std::exception {
protected:
  std::string what_message;

public:
  PhenopacketException() = default;
  PhenopacketException(char const* const message):what_message(message){}
  PhenopacketException(const std::string & message):what_message(message){}
  const char* what() const throw();
};

/**
 * Use for Parsing errors from the JSON.
 * catch (const JsonParseException& e)
 */
class JsonParseException : public PhenopacketException {


public:
  JsonParseException(char const* const message) { }
  JsonParseException(const std::string & message) { }
  const char* what() const throw();
};



#endif
