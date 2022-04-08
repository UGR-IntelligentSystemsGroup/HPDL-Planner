#include "foralleffect.hh"
#include <sstream>
#include "variablesymbol.hh"
#include "header.hh"
#include "constants.hh"

//Variables para controlar la impresion
extern bool PRINTING_FORALLPARAMETERS;
extern bool PRINT_NUMBERTYPE;
extern bool PRINT_OBJECTTYPE;
extern bool PRINT_DEFINEDTYPES;

ForallEffect::ForallEffect(Effect * e)
    :Effect(), ParameterContainer()
{
  effect = e;
}

ForallEffect::ForallEffect(Effect *e, const KeyList * p)
    :Effect(), ParameterContainer(p)
{
  effect = e;
}

ForallEffect::ForallEffect(void)
    :Effect(), ParameterContainer()
{
    effect = 0;
}

ForallEffect::ForallEffect(const ForallEffect * cp)
    :Effect(cp), ParameterContainer(cp)
{
    if(cp->effect)
	effect = (Effect *) cp->effect->clone();
    else
	effect =0;
}

Expression * ForallEffect::clone(void) const
{
 return new ForallEffect(this);
}

void ForallEffect::print(ostream * os, int indent) const
{
	string s(indent,' ');
	bool PRINT_NUMBERTYPE2, PRINT_OBJECTTYPE2, PRINT_DEFINEDTYPES2;
	
	PRINTING_FORALLPARAMETERS = true;
	
	
	PRINTING_FORALLPARAMETERS = true;
	//Guardamos los valores antiguos
	PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
	PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
	PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;
	
	//Fijamos los nuevos valores
	PRINT_NUMBERTYPE = true;
	PRINT_OBJECTTYPE = true;
	PRINT_DEFINEDTYPES = true;
		
	*os << s << "(forall (";
	for_each(parameters.begin(),parameters.end(),PrintKey(os));
	*os << ") " << endl;
	
	//Recuperamos los valores anteriores
	PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
	PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;
	PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;	
	PRINTING_FORALLPARAMETERS = false;	
	PRINTING_FORALLPARAMETERS = false;
	
	effect->print(os,indent + NINDENT);
	*os << s << ")";
}

void ForallEffect::toxml(XmlWriter * writer) const{
    writer->startTag("forall");
    for_each(parameters.begin(),parameters.end(),ToXMLKey(writer));
    effect->toxml(writer);
    writer->endTag();
}

pkey ForallEffect::getTermId(const char * name) const
{
    keylistcit pit;
    pit = find_if(parameters.begin(),parameters.end(),HasName(name));
    if(pit != parameters.end())
        return (*pit);

  return effect->getTermId(name);
}

bool ForallEffect::hasTerm(int id) const
{
    if(find_if(parameters.begin(),parameters.end(),HasId(id)) != parameters.end())
        return true;

    return effect->hasTerm(id);
}

void ForallEffect::renameVars(Unifier * u, VUndo * undo)
{
    varRenaming(u,undo);
    effect->renameVars(u,undo);
}

bool ForallEffect::apply(State *sta, VUndo * undo, Unifier * uf)
{
    Unifier context;
    return assertl(sta,undo,&context,0,uf);
}

bool ForallEffect::assertl(State * sta, VUndo * undo, Unifier * context, int pos, Unifier * uf)
{
    bool ret=false;
    // Condici�n de parada de la recursividad
    if(pos >= (int) parameters.size())
    {
	VUndo undoUni;
	renameVars(context,&undoUni);
	ret=effect->apply(sta,undo,uf);
	undoit beg, end;
	end = undoUni.end();
	for(beg = undoUni.begin(); beg != end; beg++){
	    (*beg)->undo();
	}
	return ret;
    }

    if(!parser_api->termtable->isVariable(parameters[pos])){
	ret = assertl(sta,undo,context,pos+1,uf);
	return ret;
    }
    else{
	// para la variable cuantificada que toque en la llamada recursiva
	VariableSymbol * v = (VariableSymbol *) parser_api->termtable->getVariable(parameters[pos]);

	// obtener todas las constantes que son del tipo de la variable
	if(v->isObjectType())
	{
	    if(parser_api->termtable->constants.empty()){
		return true;
	    }
	    constantTable::const_iterator j, je;
	    je = parser_api->termtable->constants.end();
	    for(j = parser_api->termtable->constants.begin(); j != je; j++)
	    {
		context->addFSubstitution(v->getId(),make_pair((*j)->getId(),0));
		ret= assertl(sta,undo,context,pos+1,uf);
		if(!ret){
		    return false;
		}
	    }
	    return true;
	}
	else
	{
	    typecit i,e;
	    vconstants::const_iterator j, je;

	    e = v->getEndType();
	    for(i = v->getBeginType();i != e; i++)
	    {
		if((*i)->emptyReferencedBy()){
		    return true;
		}

		je = (*i)->getReferencedEnd();
		for(j = (*i)->getReferencedBegin(); j != je; j++)
		{
		    context->addFSubstitution(v->getId(),make_pair((*j)->getId(),0));
		    ret = assertl(sta,undo,context,pos+1,uf);
		    if(!ret){
			return false;
		    }
		}
	    }
	    return true;
	}
    }
    return false;
}

bool ForallEffect::provides(const Literal * l) const
{
    return effect->provides(l);
};
