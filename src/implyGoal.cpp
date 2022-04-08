#include "implyGoal.hh"
#include "problem.hh"
#include "papi.hh"

ImplyGoal::ImplyGoal(const ImplyGoal * o) 
    :Goal(o) 
{
    if(o->condition)
	condition = (Goal *) o->condition->clone(); 
    else
	condition =0;
    if(o->goal)
	goal = (Goal *) o->goal->clone();
    else
	goal = 0;
};

void ImplyGoal::renameVars(Unifier* u, VUndo* undo)
{
    if(condition)
	condition->renameVars(u,undo);
    if(goal)
	goal->renameVars(u,undo);
    if(isTimed())
	time->renameVars(u,undo);
};

pkey ImplyGoal::getTermId(const char* n) const
{
    pkey ret(-1,0);

    if(condition)
	ret = condition->getTermId(n);
    if(ret.first != -1)
	return ret;
    else if(goal)
	ret = goal->getTermId(n);
    if(ret.first != -1)
	return ret;
    else if(isTimed())
	ret = time->getTermId(n);
	
    return ret;
};

bool ImplyGoal::hasTerm(int id) const
{
    if(condition && condition->hasTerm(id))
	return true;
    else if(goal && goal->hasTerm(id))
	return true;
    else if(isTimed() && time->hasTerm(id))
	return true;
    return false; 
};

void ImplyGoal::print(ostream* out, int nindent) const
{
    string s(nindent,' ');
    *out << s;
    if(isTimed()){
	*out << "(";
	time->print(out,0);
	*out << " ";
    }
    *out << "(imply" << endl;
    if(condition)
	condition->print(out,nindent + NINDENT);
    else
	*out << s << "()";
    *out  << endl;

    if(goal)
	goal->print(out,nindent + NINDENT);
    else
	*out << s << "()" ;
    *out << endl;

    if(isTimed())
	*out << s << "))" << endl;
    else
	*out << s << ")" << endl;
};

void ImplyGoal::toxml(XmlWriter * writer) const{
    writer->startTag("imply");

    if(!polarity)
	writer->addAttrib("polarity","negated");
    else 
	writer->addAttrib("polarity","affirmed");

    if(isTimed())
	time->toxml(writer);
    if(condition)
	condition->toxml(writer);
    if(goal)
	goal->toxml(writer);
    writer->endTag();
};

Expression* ImplyGoal::clone(void) const
{
    return new ImplyGoal(this);
};

UnifierTable* ImplyGoal::getUnifiers(const State* state, const Unifier* u, bool polarity, Protection * protection) const
{
    bool pol = true;
    // determinar si el predicado est� negado
    if((!getPolarity() || !polarity) && (!(!getPolarity() && !polarity)))
        pol = false;

    Protection  p;
    if(time){
	p.first = time->evalStart(u,state);
	p.second = time->evalEnd(u,state);
	protection = &p;
    }

    if(pol)
    {
	// La �nica condici�n con la cual el imply falla
	// es cuando se da a y no b. Es lo que compruebo.
	Unifier * uf;
	if(u)
	    uf= u->clone();
	else
	    uf = new Unifier();

	// comprobar si el antecedente es cierto
	UnifierTable * ret=0;
	if(condition)
	    ret = condition->getUnifiers(state,u,true,protection);

	// Cuando el antecedente sea cierto el consecuente 
	// tambi�n lo debe ser.
	if(ret && !ret->isEmpty()){
	    if(goal){
		UnifierTable * aux;
		unifiercit j, e = ret->getUnifierEnd();
		for(j = ret->getUnifierBegin(); j != e; j++)
		{
		    aux = goal->getUnifiers(state,*j,true,protection);
		    if(aux == 0 || aux->isEmpty()){
			// esto es un fallo, tenemos un consecuente
			// falso con un antecedente cierto.
			// a y no b
			delete aux;
			delete ret;
			delete uf;
			return 0;
		    }
		    uf->addCLTable((*j));
		    unifiercit k , ke = aux->getUnifierEnd();
		    for(k=aux->getUnifierBegin();k!=ke;k++)
			uf->addCLTable((*k));
		    delete aux;
		}
		delete ret;
	    }
	}
	ret = new UnifierTable();
	ret->addUnifier(uf);
	return ret; 
    }
    else {
	// La unica situaci�n en que el imply falla cuando este est�
	// negado es que se d� b y no a 
	Unifier * uf;
	if(u)
	    uf= u->clone();
	else
	    uf = new Unifier();

	// comprobar cuando el consecuente es cierto
	// el antecedente tambi�n lo debe ser 
	UnifierTable * ret=0;
	if(goal)
	    ret = goal->getUnifiers(state,u,true,protection);

	if(ret && !ret->isEmpty()){
	    if(condition){
		UnifierTable * aux;
		unifiercit j, e = ret->getUnifierEnd();
		for(j = ret->getUnifierBegin(); j != e; j++)
		{
		    aux = condition->getUnifiers(state,*j,true,protection);
		    if(aux == 0 || aux->isEmpty()){
			// esta es la situaci�n de fallo
			delete aux;
			delete ret;
			delete uf;
			return 0;
		    }
		    uf->addCLTable((*j));
		    unifiercit k , ke = aux->getUnifierEnd();
		    for(k=aux->getUnifierBegin();k!=ke;k++)
			uf->addCLTable((*k));
		    delete aux;
		}
		delete ret;
	    }
	}
	ret = new UnifierTable();
	ret->addUnifier(uf);
	return ret; 
    }
    return 0;
};

bool ImplyGoal::isReachable(ostream* err, bool p) const
{
    bool pol = true,r=true;
    // determinar si el predicado est� negado
    if((!getPolarity() || !p) && (!(!getPolarity() && !p)))
        pol = false;

    if(condition)
	r = (r && condition->isReachable(err,pol));
    if(goal)
	r = (r && goal->isReachable(err,pol));

    return r;
};

