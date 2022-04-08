#include "literaleffect.hh"
#include <sstream>
#include "state.hh"
#include "undoARLiteralState.hh"
#include "problem.hh"
#include "papi.hh"
#include "fluentNumber.hh"
#include "domain.hh"
#include "constants.hh"
#include "plan.hh"

//Variable para controlar la impresion
extern bool PRINTING_FUNCTIONS;

LiteralEffect::LiteralEffect(int id, int mid, bool p)
    :Effect(), Literal(id,mid), TimeStamped()
{
    polarity = p;
    maintain = false;
}

LiteralEffect::LiteralEffect(int id, int mid, const KeyList * param, bool p)
    :Effect(), Literal(id,mid,param), TimeStamped()
{
    polarity = p;
    maintain = false;
}

LiteralEffect::LiteralEffect(const LiteralEffect * le) 
    :Effect(le), Literal(le), TimeStamped(le)
{
    polarity = le->polarity; 
    maintain = false;
};

Expression * LiteralEffect::clone(void) const
{
    return new LiteralEffect(this);
};

Literal * LiteralEffect::cloneL(void) const
{
    return new LiteralEffect(this);
};

void LiteralEffect::print(ostream * os, int indent) const
{
    if(!PRINTING_FUNCTIONS) {
	string s(indent,' ');
	bool printed = false;

	*os << s;
	if(isTimed()){
	    *os << "(";
	    if(time->isFluentNumber()) {
		if(((FluentNumber *) time)->getValue().second == ATSTART){
		    *os << "at start ";
		    printed = true;
		}
		else if(((FluentNumber *) time)->getValue().second == ATEND){
		    *os <<  "at end ";
		    printed = true;
		}
	    }
	    if(!printed){
		*os << "at ";
		time->printEvaluable(os,0);
		*os << " ";
	    }
	}
	if(!polarity)
	    *os << "(not ";
	headerPrint(os);
	if(!getPolarity())
	    *os << ")";
	if(isTimed())
	    *os << ")";
    }
}

void LiteralEffect::toxml(XmlWriter * writer) const{
    writer->startTag("predicate")
	->addAttrib("name",getName());

    if(!polarity)
	writer->addAttrib("polarity","negated");
    else 
	writer->addAttrib("polarity","affirmed");

    if(isTimed()){
	time->toxmlEvaluable(writer);
	if(time->isFluentNumber()) {
	    if(((FluentNumber *) time)->getValue().second == ATSTART){
		writer->startTag("atstart")
		    ->endTag()
		    ->endTag();
		return;
	    }
	    else if(((FluentNumber *) time)->getValue().second == ATEND){
		writer->startTag("atend")
		    ->endTag()
		    ->endTag();
		return;
	    }
	}
	writer->startTag("at");
	time->toxmlEvaluable(writer);
	writer->endTag();
    }
    for_each(parameters.begin(),parameters.end(),ToXMLKey(writer));
    writer->endTag();
}


bool LiteralEffect::apply(State *sta, VUndo * undo, Unifier * uf){
    UndoARLiteralState * u;

    if (getPolarity()) 
    {
	// Es un literal de la lista de adici�n. Se a�ade al estado.
	// solo insertamos el literal si no est� ya insertado
	// comprobar que no se cuelen variables libres.
	if(hasVariables())
	{
	    // insultar al personal y abortar.
	    *errflow << "Runtime error: Trying to assert a literal in the current state with unbound vars." << endl;
	    print(errflow,NINDENT);
	    exit(EXIT_FAILURE);
	}
	LiteralEffect * l = (LiteralEffect *)this->clone();
	u = new UndoARLiteralState(l,true);
	u->time = evalTime();
	undo->push_back(u);
    }
    else 
    {
	// Es un literal de la lista de supresi�n. Se borran del estado todos los literales que unifiquen con �l.
	// Obtenemos todos los literales del estado que tienen el mismo nombre que el literal de la lista de supresi�n
	//cerr << "------------------------------------" << endl;
	//problem->getState()->print(&cerr);
	//cerr << "------------------------------------" << endl;
	ISTable_mrange range = sta->getModificableRange(getId());
	isit aux;
	isit i = range.first;
	isit e = range.second;
	while(i != e){
	    if(unify(&parameters,(*i).second->getParameters())){
		// si tratamos de eliminar un literal protegido se produce un
		// fallo
		if((*i).second->isLEffect()) {
		    if(((LiteralEffect *)(*i).second)->isMaintain()) {
			if(current_plan->FLAG_VERBOSE)
			    cerr << "(***) Triying to remove a protected effect. " << endl;
			return false;
		    }
		}
		aux = i;
		i++;
		u = new UndoARLiteralState((*aux).second,false);
		u->time = evalTime();
		undo->push_back(u);
		sta->getLiteralTable()->erase(aux);
	    } 
	    else
		i++;
	}
    }
    return true;
}

pkey LiteralEffect::getTermId(const char * name) const
{
    pkey ret;
    ret = getTermIdL(name);
    if(ret.first == -1 && isTimed())
	return time->compGetTermId(name);
    return ret;
}

bool LiteralEffect::hasTerm(int id) const
{
   if(searchTermId(id) != parameters.end())
       return true;
   if(isTimed() && time->compHasTerm(id))
       return true;
   return false;
}

void LiteralEffect::renameVars(Unifier * u, VUndo * undo)
{
    varRenaming(u,undo);
    if(isTimed())
	time->compRenameVars(u,undo);
}

bool LiteralEffect::provides(const Literal * l) const
{
    if(l->getId() == this->getId())
	if(getPol() == l->getPol())
	    if(unify(this->getParameters(),l->getParameters()))
		return true;
    return false;
};

