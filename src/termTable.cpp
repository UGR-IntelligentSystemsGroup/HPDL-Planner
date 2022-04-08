#include "termTable.hh"
#include "variablesymbol.hh"
#include "constantsymbol.hh"
#include "unifier.hh"
#include "papi.hh"
#include "domain.hh"
#include "constants.hh"

//Variable para controlar la impresion
extern bool PRINTING_COMPARATIONGOAL;
extern bool PRINTING_TASKSNETWORK;
extern bool PRINTING_DURATIONS;
extern bool PRINTING_EFFECT;
extern bool PRINTING_PRECONDITIONS;
extern bool PRINTING_CONDITIONS;
extern bool PRINTING_FORALLPARAMETERS;
extern bool PRINTING_PREDICATELIST;
extern bool PRINTING_PARAMETERS;
extern bool PRINT_NUMBERTYPE;

int LIMIT_GARBAGE=100;

void TermTable::eraseAll(void)
{
    for_each(variables.begin(),variables.end(),Delete<Term>());
    variables.clear();
    for_each(constants.begin(),constants.end(),Delete<Term>());
    constants.clear();
}

void TermTable::performGarbageCollection(int index){
    if(index < 0 && index > (int) variables.size()){
	for_each(variables.begin() + index,variables.end(),Delete<Term>());
	variables.erase(variables.begin() + index, variables.end());
    }
};

void TermTable::print(ostream * out ) const
{   
    varTable::const_iterator v, ve;
    constantTable::const_iterator cs, ce;
    int c;

    ve = variables.end();
    c=-2;
    for(v = variables.begin(); v != ve; c--, v++)
    {
        *out << "[" << c << "]\t\t\t\t";
        (*v)->print(out);
        (*v)->printReferences(out);
        *out << endl;
    }

    ce = constants.end();
    c=0;
    for(cs = constants.begin(); cs != ce; c++, cs++)
    {
        *out << "[" << c << "]\t\t\t\t";
        (*cs)->print(out);
        *out << endl;
    }
}

void TermTable::print(const pkey &key, ostream * out, int nindent ) const
{   
    if(nindent > 0)
	*out << string(nindent, ' ');
    if(key.first == -1) {
	if(PRINT_NUMBERTYPE) 
	    *out << key.second << " - number ";
	else
	    *out << key.second << " ";
    }else if(key.first >= 0)
        constants[key.first]->print(out);
    else
        variables[-2 -key.first]->print(out);
}

pkey TermTable::addVariable(VariableSymbol * v) {
    pkey pk = make_pair(-2 -variables.size(),0); 
    v->setId(pk.first);
    variables.push_back(v); 
    return pk;
};

pkey TermTable::addConstant(const char * name) {
    pkey pk = make_pair(constants.size(),0); 
    constants.push_back(new ConstantSymbol(name,pk.first)); 
    (parser_api->domain->cdictionary).insert(make_pair(name,pk.first));
    //cerr << "ADDC:" << name << " " << pk.first << " " << pk.second << endl;
    return pk;
};

pkey TermTable::addConstant(ConstantSymbol * c) {
    pkey pk = make_pair(constants.size(),0); 
    c->setId(pk.first);
    (parser_api->domain->cdictionary).insert(make_pair(c->getName(),pk.first));
    constants.push_back(c);
    //cerr << "ADDC:" << c->getName() << " " << pk.first << " " << pk.second << endl;
    return pk;
};

ConstantSymbol * TermTable::getConstantFromName(const char * name)
{
    ldictionarycit f;
    if((f=(parser_api->domain->cdictionary).find(name)) != (parser_api->domain->cdictionary).end())
	return getConstant((*f).second);
    else
	return 0;
};

