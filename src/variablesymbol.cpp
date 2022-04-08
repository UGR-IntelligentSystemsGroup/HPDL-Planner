#include "variablesymbol.hh"
#include <iostream>
#include <sstream>
#include "unifier.hh"
#include "papi.hh"
#include "domain.hh"
#include "constants.hh"

int VAR_COUNTER=0;

//Variable para controlar la impresion
extern bool PRINTING_BINDS;
extern bool PRINTING_COMPARATIONGOAL;
extern bool PRINTING_TASKSNETWORK;
extern bool PRINTING_SORTGOAL;
extern bool PRINTING_DURATIONS;
extern bool PRINTING_PRECONDITIONS;
extern bool PRINTING_CONDITIONS;
extern bool PRINTING_FORALLPARAMETERS;
extern bool PRINTING_PREDICATELIST;
extern bool PRINTING_HEADER_FUNCTION;
extern bool PRINTING_PARAMETERS;
extern bool PRINTING_TYPELIST;
extern bool PRINT_OBJECTTYPE;
extern bool PRINT_DEFINEDTYPES;
extern bool PRINT_NUMBERTYPE;

extern string str2XML(string s);


Term * VariableSymbol::clone(void) const
{
    return new VariableSymbol(this);
};

void VariableSymbol::print(ostream * os, int indent) const
{
    typecit b, e;
    string s(indent,' ');
    bool PRINT = true;

    //*os << s << getName() << "[" << id << "]";
    *os << s << getName();


    int siz = types.size();

    e=types.end();
    for(b=types.begin();b!=e;b++) {
	if(!PRINT_NUMBERTYPE && (strcmp("number",(*b)->getName())==0))
	    PRINT = false;
    }

    if(siz == 1){
	if(PRINT_DEFINEDTYPES && PRINT){
	    *os << " -";
	    for_each(types.begin(), types.end(), Print2<Type>(os,1));
	}
    }
    else if(siz > 1){
	if(PRINT_DEFINEDTYPES && PRINT){
	    *os << " - (either";
	    for_each(types.begin(), types.end(), Print2<Type>(os,1));
	    *os << ")";
	}
    }
    else
	if(PRINT_OBJECTTYPE)
	    *os << " - object";

};

void VariableSymbol::toxml(XmlWriter * writer) const{
    writer->startTag("variable")
	->addAttrib("name",getName())
	->addAttrib("id",getId());

    vector<Type *>::const_iterator i,e;
    e = types.end();
    for(i=types.begin();i!=e;i++)
	(*i)->toxml(writer);

    writer->endTag();
};

const char * VariableSymbol::getName(void) const 
{return parser_api->domain->getMetaName(metaid);};

void VariableSymbol::setName(const char * n) 
{parser_api->domain->setMetaName(metaid,n);};

