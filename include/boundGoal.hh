#ifndef BOUNDGOAL
#define BOUNDGOAL

#include "fluentVar.hh"
#include "goal.hh"

using namespace std;

/**
 * Esta construcci�n no es parte del PDDL standard. Sirve para en las preconduciones
 * forzar la unificaci�n de una variable con una expresi�n.
 */
class BoundGoal: public Goal
{
    public:
	BoundGoal(void) :Goal() {};
	BoundGoal(FluentVar * v, Evaluable * e) :Goal() {var=v; exp=e;};
	BoundGoal(const BoundGoal * o); 
	virtual ~BoundGoal(void) {delete var; delete exp;};

	virtual bool isBoundGoal(void) const {return true;};
	virtual Expression * clone(void) const;
	virtual UnifierTable * getUnifiers(const State * state, const Unifier * context, bool inheritPolarity,  pair<unsigned int,unsigned int> * protection) const;
	virtual bool isReachable(ostream * err, bool pol) const;
	virtual void renameVars(Unifier*, VUndo*);
	virtual pkey getTermId(const char*) const;
	virtual bool hasTerm(int) const;
	virtual void print(ostream*, int) const;
	virtual void toxml(XmlWriter * writer) const;

    protected:
	FluentVar * var;
	Evaluable * exp;
};

#endif
