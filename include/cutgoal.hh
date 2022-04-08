#ifndef CUTGOAL
#define CUTGOAL

#include "goal.hh"

class CutGoal: public Goal
{
    public:
	CutGoal(void) :Goal() {goal=0;};
	CutGoal(const CutGoal * cg) :Goal(cg) {if(cg->goal) goal = (Goal *)cg->goal->clone(); else goal=0;};
	virtual ~CutGoal(void) {delete goal;};

	virtual bool isCutGoal(void) const {return true;};
	inline void setGoal(Goal * g) {goal=g;};
	inline const Goal * getGoal(void) {return goal;};

	virtual UnifierTable * getUnifiers(const State * state, const Unifier * context, bool inheritPolarity,  pair<unsigned int,unsigned int> * protection) const
	{
	    bool pol = true;
	    if((!getPolarity() || !inheritPolarity) && (!(!getPolarity() && !inheritPolarity)))
		pol = false;
	    UnifierTable * ut = goal->getUnifiers(state,context,pol,protection);
	    if(ut)
		ut->cut();
	    return ut;
	};

	virtual bool isReachable(ostream * err, bool pol) const
	{
	    bool p = true;
	    if((!getPolarity() || !pol) && (!(!getPolarity() && !pol)))
		p = false;
	    return goal->isReachable(err,p);
	};

	virtual void renameVars(Unifier* u , VUndo * vr)
	{
	    goal->renameVars(u,vr);
	};

	virtual pkey getTermId(const char* n) const
	{
	    return goal->getTermId(n);
	};

	virtual bool hasTerm(int i) const
	{
	    return goal->hasTerm(i);
	};

	virtual void print(ostream* out, int ind) const{
	    string s(ind,' ');
	    *out << s << "(! ";
	    goal->print(out,ind);
	    *out << ")";
	};

	virtual void toxml(XmlWriter * writer) const{
	    writer->startTag("cut");
	    goal->toxml(writer);
	    writer->endTag();
	};

	virtual Expression* clone() const
	{
	    return (Expression *) new CutGoal(this);
	};
    protected:
	Goal * goal;
};
#endif
