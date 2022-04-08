#include "forallgoal.hh"
#include <sstream>
#include "variablesymbol.hh"
#include "header.hh"
#include "constants.hh"

//Variables para controlar la impresion
extern bool PRINTING_FORALLPARAMETERS;
extern bool PRINT_NUMBERTYPE;
extern bool PRINT_OBJECTTYPE;
extern bool PRINT_DEFINEDTYPES;


ForallGoal::ForallGoal(Goal *g, bool pol)
    :Goal(), ParameterContainer()
{
  goal = g;
  polarity = pol;
}

ForallGoal::ForallGoal(Goal *g, bool pol, const KeyList * p)
    :Goal(), ParameterContainer(p)
{
  goal = g;
  polarity = pol;
}

ForallGoal::ForallGoal(void)
    :Goal(), ParameterContainer()
{
    goal = 0;
}

ForallGoal::ForallGoal(const ForallGoal * cp)
    :Goal(cp), ParameterContainer(cp)
{
    if(cp->goal)
    goal = (Goal *) cp->goal->clone();
    else
    goal =0;
}

Expression * ForallGoal::clone(void) const
{
 return new ForallGoal(this);
}

void ForallGoal::print(ostream * os, int indent) const
{
    string s(indent,' ');
    bool PRINT_NUMBERTYPE2, PRINT_OBJECTTYPE2, PRINT_DEFINEDTYPES2;

    *os << s;
    if(isTimed()){
        *os << "(";
        time->print(os,0);
        *os << " ";
    }

    PRINTING_FORALLPARAMETERS = true;
    //Guardamos los valores antiguos
    PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
    PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
    PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;

    //Fijamos los nuevos valores
    PRINT_NUMBERTYPE = true;
    PRINT_OBJECTTYPE = true;
    PRINT_DEFINEDTYPES = true;

    *os << "(forall (";
    for_each(parameters.begin(),parameters.end(),PrintKey(os));
    *os << ") " << endl;

    //Recuperamos los valores anteriores
    PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
    PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;
    PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;
    PRINTING_FORALLPARAMETERS = false;

    goal->print(os,indent + NINDENT);
    *os << s << ")";
    if(isTimed())
        *os << ")";
}

void ForallGoal::toxml(XmlWriter * writer) const{
    writer->startTag("forall");

    if(isTimed())
    time->toxml(writer);
    for_each(parameters.begin(),parameters.end(),ToXMLKey(writer));
    goal->toxml(writer);

    writer->endTag();
}

pkey ForallGoal::getTermId(const char * name) const
{
    keylistcit pit;
    pkey k;
    pit = find_if(parameters.begin(),parameters.end(),HasName(name));
    if(pit != parameters.end())
        return (*pit);

    k =goal->getTermId(name);
    if(k.first != -1)
    return k;
    if(isTimed())
    k = time->getTermId(name);
    return k;
}

bool ForallGoal::hasTerm(int id) const
{
    if(find_if(parameters.begin(),parameters.end(),HasId(id)) != parameters.end())
        return true;

    if(goal->hasTerm(id))
    return true;
    if(isTimed())
    return time->hasTerm(id);
    return false;
}

void ForallGoal::renameVars(Unifier * u, VUndo * undo)
{
    varRenaming(u,undo);
    goal->renameVars(u,undo);
    if(isTimed())
    time->renameVars(u,undo);
}

UnifierTable *ForallGoal::getUnifiers(const State * sta, const Unifier *context, bool inheritPolarity, Protection * protection) const
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

    bool result = test(sta,aux,inheritPolarity,protection,0);

    bool pol = true;
    // determinar mi polaridad
    if((!getPolarity() || !inheritPolarity) && (!(!getPolarity() && !inheritPolarity)))
        pol = false;

    delete aux;
    aux = 0;

    if((result && pol) || (!result && !pol)){
        Unifier * u;
        if(context)
            u = context->clone();
        else
            u = new Unifier();
        UnifierTable * ut = new UnifierTable();
        ut->addUnifier(u);
        return ut;
    }

     return 0;
};


bool ForallGoal::test(const State * sta, Unifier * context, bool inheritPolarity, Protection * p, int pos) const
{
    // Condici�n de parada de la recursividad
    if(pos >= (int) parameters.size())
    {
    //cerr << "probando" << endl;
    //goal->print(&cerr,0);
    //cerr << endl;
    //context->print(&cerr,0);
    //cerr << endl << "###############" << endl;
        UnifierTable * ret = goal->getUnifiers(sta,context,inheritPolarity,p);
    if(!ret || ret->isEmpty()){
        //cerr << "FALSE!!" << endl;
        //exit(EXIT_FAILURE);
        if(ret)
        delete ret;
        return false;
    }
        else
        {
        //cerr << "TRUE!!" << endl;
        //exit(EXIT_FAILURE);
            delete ret;
            return true;
        }
    }

    if(!parser_api->termtable->isVariable(parameters[pos]))
    test(sta,context,inheritPolarity,p,pos+1);
    else{
    // para la variable cuantificada que toque en la llamada recursiva
    VariableSymbol * v = (VariableSymbol *) parser_api->termtable->getVariable(parameters[pos]);

    // obtener todas las constantes que son del tipo de la variable
    if(v->isObjectType())
    {
        if(parser_api->termtable->constants.empty())
        return true;
        constantTable::const_iterator j, je;
        je = parser_api->termtable->constants.end();
        for(j = parser_api->termtable->constants.begin(); j != je; j++)
        {
        Unifier * tmp = 0;
        tmp = context->clone();
        tmp->addFSubstitution(v->getId(),make_pair((*j)->getId(),0));
        if(!test(sta,tmp,inheritPolarity,p,pos+1)){
            delete tmp;
            return false;
        }
        delete tmp;
        }
    }
    else
    {
        typecit i,e;
        vconstants::const_iterator j, je;

        e = v->getEndType();
        for(i = v->getBeginType();i != e; i++)
        {
        if((*i)->emptyReferencedBy())
            return true;

        je = (*i)->getReferencedEnd();
        for(j = (*i)->getReferencedBegin(); j != je; j++)
        {
            Unifier * tmp = 0;
            tmp = context->clone();
            tmp->addFSubstitution(v->getId(),make_pair((*j)->getId(),0));
            if(!test(sta,tmp,inheritPolarity,p,pos+1)){
            delete tmp;
            return false;
            }
            delete tmp;
        }
        }
    }
    }

    return true;
}

