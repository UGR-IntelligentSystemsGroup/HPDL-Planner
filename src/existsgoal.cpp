#include "existsgoal.hh"
#include <sstream>
#include "variablesymbol.hh"
#include "header.hh"

ExistsGoal::ExistsGoal(Goal *g, bool pol)
    :Goal(), ParameterContainer()
{
  goal = g;
  polarity = pol;
}

ExistsGoal::ExistsGoal(Goal *g, bool pol, const KeyList * p)
    :Goal(), ParameterContainer(p)
{
  goal = g;
  polarity = pol;
}

ExistsGoal::ExistsGoal(void)
    :Goal(), ParameterContainer()
{
    goal = 0;
}

ExistsGoal::ExistsGoal(const ExistsGoal * cp)
    :Goal(cp), ParameterContainer(cp)
{
    if(cp->goal)
	goal = (Goal *) cp->goal->clone();
    else
	goal =0;
}

Expression * ExistsGoal::clone(void) const
{
 return new ExistsGoal(this);
}

void ExistsGoal::print(ostream * os, int indent) const
{
	string s(indent,' ');

	*os << s;
	if(isTimed()){
	    *os << "(";
	    time->print(os,0);
	    *os << " ";
	}
	if(!polarity)
		*os << "(not ";
	*os << "(exists (";
	for_each(parameters.begin(),parameters.end(),PrintKey(os));
	*os << ") " << endl;
	goal->print(os,indent + NINDENT);
	*os << s << ")";
	if(!getPolarity())
		*os << ")";
	if(isTimed())
	    *os << ")";
}

void ExistsGoal::toxml(XmlWriter * writer) const{
    writer->startTag("exists");

    if(!polarity)
	writer->addAttrib("polarity","negated");
    else 
	writer->addAttrib("polarity","affirmed");

    if(isTimed())
	time->toxml(writer);

    for_each(parameters.begin(),parameters.end(),ToXMLKey(writer));
    goal->toxml(writer);
    writer->endTag();
}

pkey ExistsGoal::getTermId(const char * name) const
{
    keylistcit pit;
    pkey k;
    pit = find_if(parameters.begin(),parameters.end(),HasName(name));
    if(pit != parameters.end())
        return (*pit);

    k = goal->getTermId(name);
    if(k.first != -1)
	return k;
    if(isTimed())
	k = time->getTermId(name);
    return k;
}

bool ExistsGoal::hasTerm(int id) const
{
    if(find_if(parameters.begin(),parameters.end(),HasId(id)) != parameters.end())
        return true;

    if(goal->hasTerm(id))
	return true;
    if(isTimed())
	return time->hasTerm(id);
    return false;
}

void ExistsGoal::renameVars(Unifier * u, VUndo * undo)
{
    varRenaming(u,undo);
    goal->renameVars(u,undo);
    if(isTimed())
	time->renameVars(u,undo);
}

UnifierTable *ExistsGoal::getUnifiers(const State * sta, const Unifier *context, bool inheritPolarity, Protection * protection) const
{
    Unifier * aux;

    if(context){
	aux = context->clone();
    }
    else{
	aux = new Unifier();
    }

    Protection  p;
    if(time){
	p.first = time->evalStart(context,sta);
	p.second = time->evalEnd(context,sta);
	protection = &p;
    }

    UnifierTable * ut = new UnifierTable();
    test(sta,aux,ut,inheritPolarity,protection,0);

    bool pol = true;
    // determinar mi polaridad
    if((!getPolarity() || !inheritPolarity) && (!(!getPolarity() && !inheritPolarity)))
        pol = false;

    delete aux;
    aux = 0;

    bool result = !ut->isEmpty();
    if((result && pol) || (!result && !pol)){
        return ut;
    }
    else
	delete ut;

    return 0; 
};


void ExistsGoal::test(const State * sta, Unifier * context, UnifierTable * ut, bool inheritPolarity, Protection *  p, int pos) const{
    Unifier * nc;
    // Condici�n de parada de la recursividad
    if(pos >= (int) parameters.size())
    {
	//cerr << "probando" << endl;
	//goal->print(&cerr,0);
	//cerr << endl;
	//cerr << "---" << endl;
	//context->print(&cerr,0);
	//cerr << endl << "###############" << endl;
        UnifierTable * ret = goal->getUnifiers(sta,context,inheritPolarity,p);
	if(!ret || ret->isEmpty()){
	    //cerr << "FALSE!!" << endl;
	    //exit(EXIT_FAILURE);
	    if(ret)
		delete ret;
	    return;
	}
        else
        {
	    //cerr << "TRUE!!" << endl;
	    //exit(EXIT_FAILURE);
	    ut->addUnifiers(ret);
            delete ret;
            return;
        }
    }

    if(!parser_api->termtable->isVariable(parameters[pos]))
	test(sta,context,ut,inheritPolarity,p,pos+1);
    else{
	// para la variable cuantificada que toque en la llamada recursiva
	VariableSymbol * v = (VariableSymbol *) parser_api->termtable->getVariable(parameters[pos]);

	// obtener todas las constantes que son del tipo de la variable
	if(v->isObjectType())
	{
	    if(parser_api->termtable->constants.empty())
		return;
	    constantTable::const_iterator j, je;
	    je = parser_api->termtable->constants.end();
	    for(j = parser_api->termtable->constants.begin(); j != je; j++){
		nc = context->clone();
		nc->addFSubstitution(v->getId(),make_pair((*j)->getId(),0));
		test(sta,nc,ut,inheritPolarity,p,pos+1);
		delete nc;
	    }
	}
	else
	{
	    typecit i,e;
	    vconstants::const_iterator j, je;

	    e = v->getEndType();
	    for(i = v->getBeginType();i != e; i++){
		if(!(*i)->emptyReferencedBy()){
		    je = (*i)->getReferencedEnd();
		    for(j = (*i)->getReferencedBegin(); j != je; j++){
			nc = context->clone();
			nc->addFSubstitution(v->getId(),make_pair((*j)->getId(),0));
			test(sta,nc,ut,inheritPolarity,p,pos+1);
			delete nc;
		    }
		}
	    }
	}
    }
}

