#include "unifier.hh"
#include "papi.hh"
#include "termTable.hh"
#include "variablesymbol.hh"
#include "literaleffect.hh"
#include "literal.hh"
#include "function.hh"

#define eqpair(a,b) ((a).first == (b).first && (a).second == (b).second)

Unifier::Unifier(void){
    cltable = 0;
    typeSubstitutions = 0;
};

Unifier::Unifier(const Unifier * other)
    :substitutions(other->substitutions)
{
    cltable = 0;
    typeSubstitutions = 0;
    if(other->cltable){
	cltable = new CLTable;
	//cltite i,e = other->cltable->end();
	//for(i=other->cltable->begin(); i != e; i++)
	//    cerr << this << " @@@ clone @@@ " << other << " " << (*i) << endl;
	for_each(other->cltable->begin(),other->cltable->end(),CloneV<Causal>(cltable));
	//cltable->insert(cltable->end(),other->cltable->begin(),other->cltable->end());
    }
    if(other->typeSubstitutions){
	typeSubstitutions = new vTSubstitutions;
	typesubite i, e = other->typeSubstitutions->end();
	for(i = other->typeSubstitutions->begin();i!=e;i++){
	    typeSubstitutions->push_back(make_pair((*i).first,new vector<Type *>(*((*i).second))));
	}
    }
};

Unifier::~Unifier(void){
    if(cltable){
	//cltite i,e = cltable->end();
	//for(i=cltable->begin(); i != e; i++)
	//    cerr << this << " @@@ borrando @@@ " << (*i) << endl;
	for_each(cltable->begin(),cltable->end(),Delete<Causal>());
	delete cltable;
    }
    if(typeSubstitutions){
	typesubite i, e = typeSubstitutions->end();
	for(i = typeSubstitutions->begin();i!=e;i++){
	    delete (*i).second;
	    (*i).second = 0;
	}
	delete typeSubstitutions;
    }
};

Unifier * Unifier::clone(void) const{
    return new Unifier(this);
};

struct HasKey{
    int key;
    HasKey(int k){key=k;};
    bool operator()(const pair<int,pair<int,float> > & element)
    { return element.first == key;};
};

bool Unifier::getSubstitution(int i, pkey * p) const{
   subscit j = find_if(substitutions.begin(),substitutions.end(),HasKey(i));
   if(j == substitutions.end())
       return false;
   else {
       *p =(*j).second;
       return true;
   }
}

struct HasStr{
    const char * s;
    HasStr(const char * v){s=v;};
    bool operator()(const pair<int,pair<int,float> > & element)
    { return strcasecmp(parser_api->termtable->getVariable(element.first)->getName(),s) == 0;};
};

bool Unifier::getSubstitution(const char * v, pkey * p) const{
   subscit j = find_if(substitutions.begin(),substitutions.end(),HasStr(v));
   if(j == substitutions.end())
       return false;
   else {
       *p =(*j).second;
       return true;
   }
}

void Unifier::addSubstitution(int i, pkey p){
    substitutions.push_front(make_pair(i,p));
}

void Unifier::addFSubstitution(int i, pkey p){
    substitutions.push_front(make_pair(i,p));
}

void Unifier::addCLTable(Unifier * u)
{
    if(u->cltable) {
	if(cltable){

	//    cltite i,e = u->cltable->end();
	//    for(i=u->cltable->begin(); i != e; i++)
	//	cerr << this << " @@@ CLTable @@@ " << u << " " << (*i) << endl;

	    cltable->insert(cltable->end(),u->cltable->begin(),u->cltable->end());
	    u->cltable->clear();
	    delete u->cltable;
	    u->cltable = 0;
	}
	else {
	    cltable = u->cltable;
	    u->cltable= 0;
	}
    }
};

struct PrintSub{
    string sindent;
    ostream * os;

    PrintSub(ostream *o, int in) :sindent(in,' ') {os=o;};

    void operator()(const pair<const int,pair<int,float> > & item) {
	*os << sindent;
	parser_api->termtable->getVariable(item.first)->print(os,0);
	*os << " <- ";
	parser_api->termtable->print(item.second,os);
	*os << endl;
    };
};

struct PrintTSub{
    string sindent;
    ostream * os;

    PrintTSub(ostream *o, int in) :sindent(in,' ') {os=o;};

    void operator()(pair<int,vector<Type *> * > & item){
	*os << sindent;
	parser_api->termtable->getVariable(item.first)->print(os,0);
	*os << " <- ";
	vector<Type *> * v = item.second;
	for_each(v->begin(),v->end(),Print<Type>(os,1));
	*os << endl;
    };
};

void Unifier::print(ostream * os, int indent) const
{
    for_each(substitutions.begin(),substitutions.end(),PrintSub(os,indent));
    *os << substitutions.size() << " variable subtition(s)." << endl;
    if(typeSubstitutions){
	for_each(typeSubstitutions->begin(),typeSubstitutions->end(),PrintTSub(os,indent));
	*os << typeSubstitutions->size() << " type subtition(s)." << endl;
    }
};

struct ApplyTypeSubstitution{
    VUndo * undo;
    ApplyTypeSubstitution(VUndo *u) {undo = u;};
    
    void operator()(pair<int,vector<Type *> * > & item){
	VariableSymbol * var = parser_api->termtable->getVariable(item.first);
	if(undo)
	    undo->push_back(new UndoChangeType(var,var->getTypes()));

	var->clearTypes();
	for_each(item.second->begin(),item.second->end(),AddV<Type>(var->getTypes()));
    }
};

bool Unifier::applyTypeSubstitutions(VUndo * undoApply){
    if(typeSubstitutions)
	for_each(typeSubstitutions->begin(),typeSubstitutions->end(),ApplyTypeSubstitution(undoApply));
    return true;
}

ApplyVarSubstitution::ApplyVarSubstitution(VUndo *u) {undo = u;};
    
void ApplyVarSubstitution::operator()(const pair<const int,pair<int,float> > & item) {
	VariableSymbol * v = parser_api->termtable->getVariable(item.first);
	referencesit i, e = v->references.end();
	for(i=v->references.begin();i!=e;i++) {
	    (*i)->applySubstitution(item.second);
	    if(undo){
		undo->push_back((*i));
	    }
	    else {
		delete (*i);
		(*i) = 0;
	    }
	}
	v->references.clear();
};


bool Unifier::applyVarSubstitutions(VUndo * undoApply){
    for_each(substitutions.begin(),substitutions.end(),ApplyVarSubstitution(undoApply));
    return true;
}

void Unifier::addTSubstitution(int i, vector<Type *> * p){
    if(!typeSubstitutions)
	typeSubstitutions = new vTSubstitutions;
    typeSubstitutions->push_back(make_pair(i,p));
}

void Unifier::addTSubstitution2(int i, const vector<Type *> * p){
    if(!typeSubstitutions)
	typeSubstitutions = new vTSubstitutions;
    typeSubstitutions->push_back(make_pair(i,new vector<Type*>(*p)));
}

bool Unifier::apply(VUndo * undoApply){
    applyVarSubstitutions(undoApply);
    if(typeSubstitutions)
	applyTypeSubstitutions(undoApply);
    return true;
};

void Unifier::merge(Unifier * u){
    substitutions.merge(u->substitutions);
    if(u->typeSubstitutions){
	if(!typeSubstitutions)
	    typeSubstitutions = new vTSubstitutions;
	typeSubstitutions->insert(typeSubstitutions->end(),u->typeSubstitutions->begin(),u->typeSubstitutions->end());
    }
};

void Unifier::erase(int n){
    for(int i=0;i<n;i++)
	substitutions.pop_front();
};

struct SetConsumer{
    const Task * consumer;

    SetConsumer(const Task * c){
	consumer = c;
    };

    void operator()(Causal * c){
	c->setConsumer(consumer);
    };

};

void Unifier::setCLConsumer(const Task * consumer){
    if(cltable)
	for_each(cltable->begin(),cltable->end(),SetConsumer(consumer));
};

bool Unifier::equal(const Unifier * other) const{
    subscit i, e;
    subscit j, je;
    const char * n1, * n2;
    e = other->substitutions.end();
    for(i=other->substitutions.begin();i!=e;i++){
	je = substitutions.end();
	n2 = parser_api->termtable->getTerm((*i).first)->getName();
	for(j=substitutions.begin();j!=je;j++){
	    n1 = parser_api->termtable->getTerm((*j).first)->getName();
	    if(strcasecmp(n1,n2) == 0){
		if(!eqpair((*i).second,(*j).second)){
		    return false;
		}
		else
		    break;
	    }
	}
	if(j == substitutions.end()){
	    return false;
	}
    }

    return true;
};

