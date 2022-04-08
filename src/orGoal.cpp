#include "orGoal.hh"
#include <sstream>
#include <string>
#include "goal.hh"
#include "state.hh"

OrGoal::OrGoal(const OrGoal * ag)
    :ContainerGoal(ag)
{
    for_each(ag->goals.begin(),ag->goals.end(),CloneV<Goal>(&goals));
}

OrGoal::~OrGoal(void)
{
    for_each(goals.begin(),goals.end(),Delete<Goal>());
    goals.clear();
}

Expression * OrGoal::clone(void) const
{
    return new OrGoal(this);
}

void OrGoal::print(ostream * os, int indent) const
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
	*os << "(or" << endl;
	for_each(goals.begin(),goals.end(),Print<Goal>(os,indent + 3));
	*os << s;
	if(!getPolarity())
		*os << ")"; 
	if(isTimed())
	    *os <<  "))" << endl;
	else
	    *os <<  ")" << endl;
}

void OrGoal::toxml(XmlWriter * writer) const{
    writer->startTag("or");

    if(!polarity)
	writer->addAttrib("polarity","negated");
    else 
	writer->addAttrib("polarity","affirmed");

    if(isTimed())
	time->toxml(writer);

    for_each(goals.begin(),goals.end(),ToXML<Goal,XmlWriter>(writer));

    writer->endTag();
}

UnifierTable * OrGoal::getUnifiers(const State * state, const Unifier * context, bool inheritPolarity, Protection * protection) const
{
    UnifierTable * ret=0, * tmp=0;
    
    bool pol = true;
    // determinar si el predicado est� negado
    if((!getPolarity() || !inheritPolarity) && (!(!getPolarity() && !inheritPolarity)))
        pol = false;

    /*state->print(errflow);
    *errflow << "--------------" << endl;*/

    /**errflow << endl << "--------------" << endl;
    aux->print(errflow);
    *errflow << endl << "--------------" << endl; */

    Protection  p;
    if(time){
	p.first = time->evalStart(context,state);
	p.second = time->evalEnd(context,state);
	protection = &p;
    }

    if(pol) {
	ret = new UnifierTable();
	for(goalcit i = getBegin(); i != getEnd(); i++)
	{
	    //cerr << "..." << endl;
	    //context->print(&cerr,0);
	    //cerr << endl;
	    //(*i)->print(&cerr,0);
	    //cerr << endl;
	    tmp = (*i)->getUnifiers(state,context,true,protection);
	    if(tmp && !tmp->isEmpty()){
		//cerr << "aadding " << tmp->countUnifiers() << endl;
		//tmp->print(&cerr);
		ret->addUnifiers(tmp);
	    }
	    if(tmp){
		delete tmp;
		tmp =0;
	    }
	}
	if(!ret->isEmpty())
	    return ret;
	else{
	    //cerr << "EMPTY!!" << endl;
	    delete ret;
	    return 0;
	}
    }
    else {
	// asegurarse de que ninguno unifique
	for(goalcit i = getBegin(); i != getEnd(); i++)
	{
	    ret = (*i)->getUnifiers(state,context,true,protection);
	    if(ret && !ret->isEmpty()){
		delete ret;
		return 0;
	    }	
	}
	ret = new UnifierTable();
	if(context)
	    ret->addUnifier(context->clone());
	else
	    ret->addUnifier(new Unifier());
	return ret;
    }
}

pkey OrGoal::getTermId(const char * name) const
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

bool OrGoal::hasTerm(int id) const
{
    const_mem_fun1_t<bool,Unifiable,int > f(&Unifiable::hasTerm);
    if(find_if(goals.begin(),goals.end(),bind2nd(f,id)) != goals.end())
	return true;
    if(isTimed())
	return time->hasTerm(id);
    return false;
}

void OrGoal::renameVars(Unifier * u, VUndo * undo)
{
    goalcit i, e = goals.end();
    for(i=goals.begin();i!=e;i++)
	(*i)->renameVars(u,undo);
    if(isTimed())
	time->renameVars(u,undo);
}

bool OrGoal::isReachable(ostream * err, bool inheritPolarity) const
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

