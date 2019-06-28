#ifndef ONTOLOGY_H
#define ONTOLOGY_H

#import <string>
#import <vector>

using std::string;
using std::vector;



class Term {
private:
  string id_;
  string label_;
  string definition_;
  vector<string> definition_xref_list_;

public:
  Term(const string &id, const string &label);
  void add_definition(const string &def);
  void add_definition_xref(const string &xr);

  friend std::ostream& operator<<(std::ostream& ost, const Term& term);
};
std::ostream& operator<<(std::ostream& ost, const Term& term);

#endif
