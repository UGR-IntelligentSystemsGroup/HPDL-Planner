#include "header.hh"
#include "papi.hh"
#include "domain.hh"

const char * Header::getName(void) const 
{return parser_api->domain->getMetaName(metaid);};

void Header::setName(const char * n) 
{parser_api->domain->setMetaName(metaid,n);};

