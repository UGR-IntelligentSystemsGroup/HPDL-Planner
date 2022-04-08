#include "andGoal.hh"
#include <sstream>
#include <string>
#include "goal.hh"
#include "state.hh"

AndGoal::AndGoal(const AndGoal * ag)
    :ContainerGoal(ag)
{
    for_each(ag->goals.begin(),ag->goals.end(),CloneV<Goal>(&goals));
}

AndGoal::~AndGoal(void)
{
    for_each(goals.begin(),goals.end(),Delete<Goal>());
    goals.clear();
}

Expression * AndGoal::clone(void) const
{
    return new AndGoal(this);
}

void AndGoal::print(ostream * os, int indent) const
{
	string s(indent,' ');

	*os << s;
	if(isTimed()){
	    *os << "(";
	    time->print(os,0);
	    *os << " ";
	}
	if(!getPolarity())
		*os << "(not ";
	*os << "(and" << endl;
	for_each(goals.begin(),goals.end(),Print<Goal>(os,indent + 3));
	*os << s;
	if(!getPolarity())
		*os << ")"; 
	if(isTimed())
	    *os <<  "))" << endl;
	else
	    *os <<  ")" << endl;
}

void AndGoal::toxml(XmlWriter * writer) const{
    writer->startTag("and");

    if(!polarity)
	writer->addAttrib("polarity","negated");
    else 
	writer->addAttrib("polarity","affirmed");

    if(isTimed())
	time->toxml(writer);

    for_each(goals.begin(),goals.end(),ToXML<Goal,XmlWriter>(writer));

    writer->endTag();
}

UnifierTable * AndGoal::getUnifiers(const State * state, const Unifier * context, bool inheritPolarity, pair<unsigned int, unsigned int> * protection) const
{
    assert(state != 0);
    UnifierTable * ret;
    UnifierTable * aux = new UnifierTable();
    UnifierTable * ret2;
    
    bool pol = true;
    // determinar si el predicado est� negado
    if((!getPolarity() || !inheritPolarity) && (!(!getPolarity() && !inheritPolarity)))
        pol = false;

    /*state->print(errflow);
    *errflow << "--------------" << endl;*/

    if(context)
        aux->addUnifier(context->clone());

    pair<unsigned int, unsigned int>  p;
    if(time) {
	p.first = time->evalStart(context,state);
	p.second = time->evalEnd(context,state);
	protection = &p;
    }

//    *errflow << endl << "--------------" << endl;
//    aux->print(errflow);
//    *errflow << endl << "--------------" << endl; 

    for(goalcit i = getBegin(); i != getEnd(); i++) {
        //(*i)->print(errflow,0);
        ret = new UnifierTable();
        for(unifiercit j = aux->getUnifierBegin(); j != aux->getUnifierEnd(); j++) {
            ret2 = (*i)->getUnifiers(state,*j,pol,protection);
            if(ret2 != 0){
                ret->addUnifiers(ret2);
                delete ret2;
            }
        }
        if(aux->isEmpty()) {
            if(i == getBegin()) {
                ret2 = (*i)->getUnifiers(state,0,pol,protection);
                if(ret2 != 0){
                    ret->addUnifiers(ret2);
                    delete ret2;
                }
            }
            else {
                delete aux;
                return ret;
            }
        }
        delete aux;
        aux = ret;
        ret = 0;
//	*errflow << endl << "--------------" << endl;
//	aux->print(errflow);
//	*errflow << endl << "--------------" << endl;
    }
    return aux;
}

pkey AndGoal::getTermId(const char * name) const
{
    pkey result(-1,-1);
    goalcit i,e;
    e = getEnd();
    for(i = getBegin(); i != e; i++)
    {
        result = (*i)->getTermId(name);
        if(result.first != -1)
            return result;
    }
    if(isTimed())
	result = time->getTermId(name);
    return result;
}

bool AndGoal::hasTerm(int id) const
{
    const_mem_fun1_t<bool,Unifiable,int > f(&Unifiable::hasTerm);
    if(find_if(goals.begin(),goals.end(),bind2nd(f,id)) != goals.end())
	return true;
    if(isTimed())
	return time->hasTerm(id);
    return false;
}

void AndGoal::renameVars(Unifier * u, VUndo * undo)
{
    goalcit i, e = goals.end();
    for(i=goals.begin();i!=e;i++)
	(*i)->renameVars(u,undo);
    if(isTimed())
	time->renameVars(u,undo);
}

bool AndGoal::isReachable(ostream * err, bool inheritPolarity) const
{
    bool pol = true,r=true;
    // determinar si el predicado est� negado
    if((!getPolarity() || !inheritPolarity) && (!(!getPolarity() && !inheritPolarity)))
        pol = false;

    goalcit i, e = goals.end();
    for(i=goals.begin();i!=e;i++){
	if(!(*i)->isReachable(err,pol))
	    r=false;
    }
    return r;
};

