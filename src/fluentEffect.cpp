#include "fluentEffect.hh"
#include "undoARLiteralState.hh"
#include "undoChangeTime.hh"
#include "literaleffect.hh"
#include "fluentNumber.hh"
#include "constants.hh"

//Variable para controlar la impresion
extern bool PRINTING_COMPARABLE;
extern bool PRINT_NUMBERTYPE;
extern bool PRINT_OBJECTTYPE;
extern bool PRINT_DEFINEDTYPES;

FluentEffect::FluentEffect(FOperation o, FluentLiteral * fhead, Evaluable * fexp) 
    :Effect(), TimeStamped() 
{
    op=o;f=fhead; 
    exp=fexp;
};

FluentEffect::FluentEffect(const FluentEffect * fo) 
    :Effect(fo),TimeStamped(fo)
{
    op = fo->op; 
    f = (FluentLiteral *) fo->f->cloneEvaluable(); 
    exp = fo->exp->cloneEvaluable();
};

bool FluentEffect::apply(State *sta, VUndo * undo, Unifier * uf)
{
    // evaluar la expresi�n
    Unifier context;
    pkey ret = exp->eval(sta,&context);

    UndoARLiteralState * u;
    UndoChangeValue * uc;

    Function * ref;
    // buscar la funci�n en el estado
    ISTable_mrange range = sta->getModificableRange(f->getId());
    isit i;
    if((i=find_if(range.first,range.second,EqualLit(f->getParameters()))) == range.second){
	// no se encontr� una funci�n que unifique, en el estado...
	// la a�adimos
	ref =  new Function(f->getId(),f->getMetaId(),false,0,f->getParameters());
	// comprobar que no se cuelen variables libres.
	if(f->hasVariables())
	{
	    // insultar al personal y abortar.
	    *errflow << "Runtime error: Trying to assert a fluent-function in the current state with unbound vars." << endl;
	    f->printEvaluable(errflow,NINDENT);
	    exit(EXIT_FAILURE);
	}
	sta->addLiteral(ref);
	u = new UndoARLiteralState(ref,true);
	u->time = evalTime();
	undo->push_back(u);
    }
    else {
	ref = (Function *) (*i).second;
    }
    Evaluable * new_time = 0, * old_time = 0;
    if(time){
	new_time = time->cloneEvaluable();
	old_time = ref->getModificableTime();
	ref->setTime(new_time);
	undo->push_back(new UndoChangeTime(ref,old_time,new_time));
    }
    pkey curr;
    switch(op)
    {
	case FASSIGN:
	    uc = new UndoChangeValue(ref,-1,ref->getValue(),ret);
	    undo->push_back(uc);
	    uc->time = evalTime();
	    ref->setValue(ret);
	    break;
	case FSCALEUP:
	    curr = ref->getValue();
	    if(ret.first == -1 && curr.first == -1){
		ret.second = ret.second*curr.second;
		uc = new UndoChangeValue(ref,-1,curr,ret);
		undo->push_back(uc);
		uc->time = evalTime();
		ref->setValue(ret);
	    }
	    break;
	case FSCALEDOWN:
	    curr = ref->getValue();
	    if(ret.first == -1 && curr.first == -1){
		if(curr.second != 0) {
		    ret.second = curr.second/ret.second;
		    uc = new UndoChangeValue(ref,-1,curr,ret);
		    undo->push_back(uc);
		    uc->time = evalTime();
		    ref->setValue(ret);
		}
	    }
	    break;
	case FINCREASE:
	    curr = ref->getValue();
	    if(ret.first == -1 && curr.first == -1){
		ret.second = ret.second + curr.second;
		uc = new UndoChangeValue(ref,-1,curr,ret);
		undo->push_back(uc);
		uc->time = evalTime();
		ref->setValue(ret);
	    }
	    break;
	case FDECREASE:
	    curr = ref->getValue();
	    if(ret.first == -1 && curr.first == -1){
		ret.second = curr.second - ret.second;
		uc = new UndoChangeValue(ref,-1,curr,ret);
		undo->push_back(uc);
		uc->time = evalTime();
		ref->setValue(ret);
	    }
	    break;
    };
    return true;
};

bool FluentEffect::provides(const Literal * l) const
{
    if(l->getId() == f->getId())
	if(unify(f->getParameters(),l->getParameters()))
	    return true;
    return false;
};

void FluentEffect::print(ostream * out, int indent) const
{
    string s(indent,' ');
    bool printed = false;
	bool PRINT_NUMBERTYPE2, PRINT_OBJECTTYPE2, PRINT_DEFINEDTYPES2;

    *out << s;
    if(isTimed()){
	*out << "(";
	if(time->isFluentNumber()) {
	    if(((FluentNumber *) time)->getValue().second == ATSTART){
		*out << "at start ";
		printed = true;
	    }
	    else if(((FluentNumber *) time)->getValue().second == ATEND){
		*out << "at end ";
		printed = true;
	    }
	}
	if(!printed){
	    PRINTING_COMPARABLE = true;	    
	    *out << "at ";
	    time->printEvaluable(out,0);
	    *out << " ";
		PRINTING_COMPARABLE = true;
	}
    }
    
	PRINTING_COMPARABLE = true;
	*out  << "(" << printOperation();
    
	//Guardamos los valores antiguos
	PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
	PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
	PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;
	
	//Fijamos los nuevos valores
	PRINT_NUMBERTYPE = false;
	PRINT_OBJECTTYPE = false;
	PRINT_DEFINEDTYPES = false;
		
	f->printEvaluable(out,indent + NINDENT);
    exp->printEvaluable(out,indent + NINDENT);
    *out << s << ")";
	
	//Recuperamos los valores anteriores
	PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
	PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;
	PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;
	
	PRINTING_COMPARABLE = true;
	
	if(isTimed()){
		*out << ")";
	}
};

const char * FluentEffect::printOperation(void) const
{
    static string s;

    switch(op) {
	case FASSIGN: 
	    s = "assign";
	    break;
	case FSCALEUP:
	    s = "scale-up";
	    break;
	case FSCALEDOWN:
	    s = "scale-down";
	    break;
	case FINCREASE:
	    s = "increase";
	    break;
	case FDECREASE:
	    s = "decrease";
	    break;
    }

    return s.c_str();
};

void FluentEffect::toxml(XmlWriter * writer) const {
    writer->startTag(printOperation());
    if(isTimed()){
	time->toxmlEvaluable(writer);
	if(time->isFluentNumber()) {
	    if(((FluentNumber *) time)->getValue().second == ATSTART){
		writer->startTag("atstart")->endTag();
	    }
	    else if(((FluentNumber *) time)->getValue().second == ATEND){
		writer->startTag("endstart")->endTag();
	    }
	}
	else{
	    writer->startTag("at");
	    time->toxmlEvaluable(writer);
	    writer->endTag();
	}
    }
    f->toxmlEvaluable(writer);
    exp->toxmlEvaluable(writer);
    writer->endTag();
};

void FluentEffect::renameVars(Unifier * u,VUndo * undo)
{
    f->compRenameVars(u,undo);
    exp->compRenameVars(u,undo);
};

pkey FluentEffect::getTermId(const char * name) const
{
    pkey k(-1,-1);
    k = f->compGetTermId(name);
    if(k.first != -1)
	return k;
    k = exp->compGetTermId(name);
    if(k.first == -1 && isTimed())
	return time->compGetTermId(name);
    return k;
};

bool FluentEffect::hasTerm(int id) const
{
    if(f->compHasTerm(id))
	return true;
    if(exp->compHasTerm(id))
	return true;
   if(isTimed() && time->compHasTerm(id))
       return true;
    return false;
};

