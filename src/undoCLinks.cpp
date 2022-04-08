#include "undoCLinks.hh"
#include "causalTable.hh"
#include "task.hh"

void UndoCLinks::print(ostream * os) const {
    *os <<  "UndoCLinks:: key = " << key << endl;
};

void UndoCLinks::undo(void) {
    causalTable.eraseCausalLinks(key);
};

void UndoCLinks::toxml(XmlWriter * writer) const{
};

