#include "boundGoal.hh"
#include "constants.hh"

//Variable para controlar la impresion
extern bool PRINTING_BINDS;
extern bool PRINT_NUMBERTYPE;
extern bool PRINT_OBJECTTYPE;
extern bool PRINT_DEFINEDTYPES;

BoundGoal::BoundGoal(const BoundGoal * o) 
    :Goal(o) 
{
    var = (FluentVar *) o->var->cloneEvaluable(); 
    exp = (Evaluable *) o->exp->cloneEvaluable();
};

Expression * BoundGoal::clone(void) const
{
    return new BoundGoal(this);
};

UnifierTable * BoundGoal::getUnifiers(const State * state, const Unifier * context, bool inheritPolarity, Protection * p) const
{
    bool pol = true;
    // determinar si el predicado est� negado
    if((!getPolarity() || !inheritPolarity) && (!(!getPolarity() && !inheritPolarity)))
        pol = false;

    if(pol)
    {
	pkey res = exp->eval(state,context);
	pkey val = var->eval(state,context);
	if(res.first != INT_MAX)
	{
	    // si la variable ya ha sido ligada por
	    // una unificaci�n anterior
	    // comprobar que la unificaci�n coincida
	    // con la cosa a la que queremos unificar.
	    if(val.first != INT_MAX) {
		if(!(res.first == val.first && res.second == val.second))
		    return 0;
		else {
		    Unifier * u = 0;
		    if(context)
			 u = context->clone();
		    else
			u = new Unifier();
		    UnifierTable * ut = new UnifierTable();
		    ut->addUnifier(u);
		    return ut;
		} 
	    }
	    else {
		// en otro caso realizar la sustituci�n
		Unifier * u;
		if(context)
		    u = context->clone();
		else
		    u = new Unifier();
		u->addSubstitution(var->getId().first,res);
		UnifierTable * ut = new UnifierTable();
		ut->addUnifier(u);
		return ut;
	    }
	}
	else
	    return 0;
    }
    UnifierTable * ut = new UnifierTable();
    ut->addUnifier(context->clone());
    return ut;
};

bool BoundGoal::isReachable(ostream * err, bool pol) const
{
    return true;
};

void BoundGoal::renameVars(Unifier* u, VUndo* undo)
{
    var->compRenameVars(u,undo);
    exp->compRenameVars(u,undo);
};

pkey BoundGoal::getTermId(const char* n) const
{
    pkey ret(-1,0);
    ret = var->compGetTermId(n);
    if(ret.first != -1)
	return ret;
    return exp->compGetTermId(n);
};

bool BoundGoal::hasTerm(int id) const
{
    if(var->compHasTerm(id))
	return true;
    else
	return exp->compHasTerm(id);
};

void BoundGoal::print(ostream* os, int nindent) const
{
    bool PRINT_NUMBERTYPE2, PRINT_OBJECTTYPE2, PRINT_DEFINEDTYPES2;
	
	string s(nindent,' ');


	PRINTING_BINDS = true;
	*os << s << "(bind ";
    
	//Guardamos los valores antiguos
	PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
	PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
	PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;
	
	//Fijamos los nuevos valores
	PRINT_NUMBERTYPE = false;
	PRINT_OBJECTTYPE = false;
	PRINT_DEFINEDTYPES = false;
	
	var->printEvaluable(os,0);
	
	    
	exp->printEvaluable(os,nindent + NINDENT);
    
	//Recuperamos los valores anteriores
	PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
	PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;
	PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;
		
	*os << ")" << endl;
	PRINTING_BINDS = false;
};

void BoundGoal::toxml(XmlWriter * writer) const {
    writer->startTag("bind");
    var->toxmlEvaluable(writer);
    exp->toxmlEvaluable(writer);
    writer->endTag();
};

