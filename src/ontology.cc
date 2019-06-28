
#include "ontology.h"
#include <iostream>




Term::Term(const string &id, const string &label):
  id_(id),label_(label) {}

void
Term::add_definition(const string &def) {
  definition_.assign(def);
}

void
Term::add_definition_xref(const string &xr){
  definition_xref_list_.push_back(xr);
}

std::ostream& operator<<(std::ostream& ost, const Term& term){
  ost << term.label_ << " [" << term.id_ << "]\n";
  ost << "def: " << term.definition_ << "\n";
  if (! term.definition_xref_list_.empty()) {
    for (const auto& p : term.definition_xref_list_){
      ost << "\t" << p << "\n";
    }
  }
  return ost;
}
