#include "constants.hh"
#include <iostream>
#include "constantsymbol.hh"
#include "papi.hh"

//Variable para controlar la impresion
extern bool PRINTING_CONSTANTSLIST;
extern bool PRINTING_EFFECT;
extern bool PRINTING_COMPARATIONGOAL;
extern bool PRINTING_DURATIONS;
extern bool PRINTING_PRECONDITIONS;
extern bool PRINTING_CONDITIONS;
extern bool PRINTING_TASKSNETWORK;
extern bool PRINTING_FORALLPARAMETERS;
extern bool PRINTING_COMPARABLE;
extern bool PRINTING_PREDICATELIST;
extern bool PRINTING_HEADER_FUNCTION;
extern bool PRINTING_PARAMETERS;
extern bool PRINT_DEFINEDTYPES;
extern bool PRINT_CONSTANTTYPE;
extern bool PRINT_NUMBERTYPE;

extern string str2XML(string s);

void ConstantSymbol::print(ostream * os, int indent) const
{
    string s(indent,' ');
    *os << s << name;

    typecit b, e;
    bool PRINT = true;
    //*os << s << name << " [" << getId() << "]";

    //Solo se imprimen los tipos si no estamos imprimiendo un efecto
    /*if((!PRINTING_HEADER_FUNCTION && !PRINTING_EFFECT && !PRINTING_COMPARATIONGOAL && !PRINTING_DURATIONS && !PRINTING_EFFECT && !PRINTING_PRECONDITIONS && !PRINTING_TASKSNETWORK && !PRINTING_CONDITIONS && !PRINTING_COMPARABLE) || PRINTING_FORALLPARAMETERS || PRINTING_PARAMETERS) {
    */

    e=types.end();
    for(b=types.begin();b!=e;b++) {
	if(!PRINT_NUMBERTYPE && (strcmp("number",(*b)->getName())==0))
	    PRINT = false;
    }

    if(PRINT_DEFINEDTYPES && PRINT_CONSTANTTYPE && PRINT){
	if(types.size() > 0)
	    *os << " -";
	if(types.size() > 1)
	    *os << " (either";

	for_each(types.begin(), types.end(), Print2<Type>(os,1));

	if(types.size() > 1)
	    *os << ")";
    }

    if(PRINTING_CONSTANTSLIST)
	*os << s << endl;
}

void ConstantSymbol::toxml(XmlWriter * writer) const{
    writer->startTag("constant")
	->addAttrib("name",name)
	->addAttrib("id",getId());

    vector<Type *>::const_iterator i,e;
    e = types.end();
    for(i=types.begin();i!=e;i++)
	(*i)->toxml(writer);

    writer->endTag();
};


Term * ConstantSymbol::clone(void) const
{
    return new ConstantSymbol(this);
}

bool isTimePoint(const pkey & p){
    if (p.first == -1)
        return 0;
    const ConstantSymbol * c = parser_api->termtable->getConstant(p);
    const char * n = c->getName();
    return (n[0] == '#');
};

int extractTimePoint(const pkey & p){
    int res;
    const ConstantSymbol * c = parser_api->termtable->getConstant(p);
    const char * n = c->getName();
    if (n[0] == '#'){
	if(sscanf(n,"#%d",&res))
	    return res;
    }
    return -1;
};


