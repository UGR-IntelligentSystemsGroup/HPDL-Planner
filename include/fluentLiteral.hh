#ifndef FLUENT_LITERAL_HH
#define FLUENT_LITERAL_HH

#include "constants.hh"
#include "literal.hh"
#include "evaluable.hh"
#include "state.hh"
#include "literaleffect.hh"
#include "problem.hh"

using namespace std;

class Function;

/**
 * Esta función representa a los llamados fluents en PDDL.
 * Un fluent es un literal de la forma (cabeza par1, ... , parn)
 * que tiene un comportamiento similar al de una variable.
 * Un ejemplo de fluent podría ser: (distancia granada motril)
 * que contendría el valor 50.
 * Los fluent literals pueden ser resueltos al evaluarse de tres formas:
 * - Mediante un axioma.
 * - Mediante un fluent almacenado en el estado.
 * - Usando una definición Python.
 */
class FluentLiteral: public Evaluable, public Literal 
{
    public:
	FluentLiteral(const FluentLiteral * o) :Evaluable(o), Literal(o) {};
	FluentLiteral(int id, int mid) :Evaluable(), Literal(id,mid) {};
	virtual ~FluentLiteral(void) {};
	virtual Evaluable * cloneEvaluable(void) const;
	virtual Literal * cloneL(void) const;
	virtual void printEvaluable(ostream * os, int indent=0) const;
	virtual void vcprint(ostream * os, int indent=0) const {printEvaluable(os,indent);};
	virtual void toxmlEvaluable(XmlWriter * writer) const;
	virtual pkey compGetTermId(const char * name) const;
	virtual bool compHasTerm(int id) const;
	virtual void compRenameVars(Unifier * u, VUndo * undo);
	virtual pkey eval(const State* sta, const Unifier* u) const {return eval(sta,u,0);};
	virtual pkey evaltp(const State* sta, const Unifier* u, pkey * tp, bool * pol) const;
	virtual pkey eval(const State* sta, const Unifier* u, Function ** f) const;
	virtual bool isType(const Type * t) const;
	virtual const char * toString(void) const {static string s; ostringstream os; printEvaluable(&os,0); s = os.str(); return s.c_str();}; 
	virtual void printL(ostream * os, int indent=0) const {printEvaluable(os,indent);};
	virtual void toxmlL(XmlWriter * writer) const {toxmlEvaluable(writer);};
	virtual bool getPol(void) const {return true;};
	virtual void setPol(bool t){};
	virtual void vctoxml(XmlWriter * w) const {toxmlL(w);};
	virtual bool isFluentLiteral(void) const {return true;}; 
};

#endif
