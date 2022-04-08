#include "comparationGoal.hh"
#include "function.hh"
#include "papi.hh"
#include "problem.hh"
#include "fluentLiteral.hh"
#include <sstream>
#include "constants.hh"
#include "plan.hh"

//Variable para controlar la impresion
extern bool PRINTING_COMPARATIONGOAL;
extern bool PRINTING_COMPARABLE;

extern bool PRINT_NUMBERTYPE;
extern bool PRINT_OBJECTTYPE;
extern bool PRINT_DEFINEDTYPES;


ComparationGoal::~ComparationGoal(void)
{
    delete first;
    delete second;
    first = 0;
    second = 0;
}

ComparationGoal::ComparationGoal(void)
    :Goal()
{
    first = 0;
    second = 0;
    comp = CEQUAL;
}

Expression * ComparationGoal::clone(void) const
{
    ComparationGoal * clon = new ComparationGoal();

    clon->setComparator(getComparator());

    assert(first);
    assert(second);
    clon->setFirst(first->cloneEvaluable());
    clon->setSecond(second->cloneEvaluable());
    clon->setPolarity(getPolarity());
    return clon;
}

void ComparationGoal::print(ostream * os, int indent) const
{
    bool PRINT_NUMBERTYPE2, PRINT_OBJECTTYPE2, PRINT_DEFINEDTYPES2;

    PRINTING_COMPARATIONGOAL = true;
    string s(indent,' ');
    *os << s;

    if(isTimed()){
        *os << "(";
        time->print(os,0);
        *os << " ";
    }
    if(!getPolarity())
        *os << "(not ";

    *os << "(" << printComp(getComparator());

    PRINTING_COMPARABLE = true;


        //Guardamos los valores antiguos
    PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
    PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
    PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;

    //Fijamos los nuevos valores
    PRINT_NUMBERTYPE = false;
    PRINT_OBJECTTYPE = false;
    PRINT_DEFINEDTYPES = false;

    //*os << endl;
    //first->printEvaluable(os,indent + NINDENT);
    //*os << endl;
    //second->printEvaluable(os,indent + NINDENT);
    //*os << endl;
    first->printEvaluable(os,1);
    second->printEvaluable(os,1);

    //Recuperamos los valores anteriores
    PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;
    PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
    PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;


    PRINTING_COMPARABLE = false;

    //*os << s << ")";
    *os << ")";
    if(!getPolarity())
        *os << ")";
    if(isTimed())
        *os <<  ")";
    PRINTING_COMPARATIONGOAL = false;
}

void ComparationGoal::toxml(XmlWriter * writer) const {
    writer->startTag("comparation");

    if(!polarity)
    writer->addAttrib("polarity","negated");
    else
    writer->addAttrib("polarity","affirmed");

    writer->addAttrib("operator",printComp(getComparator()));

    if(isTimed())
    time->toxml(writer);

    first->toxmlEvaluable(writer);
    second->toxmlEvaluable(writer);

    writer->endTag();
}

const char * ComparationGoal::printComp(Comparator c) const
{
    static string s;
    s = "";
    switch(c)
    {
        case CEQUAL:
            s = "=";
            break;
        case CDISTINCT:
            s = "!=";
            break;
        case CGREATHER:
            s = ">";
            break;
        case CLESS:
            s = "<";
            break;
        case CLESS_EQUAL:
            s = "<=";
            break;
        case CGREATHER_EQUAL:
            s = ">=";
            break;
    }
    return s.c_str();
};

pkey ComparationGoal::getTermId(const char * name) const
{
    pkey result = first->compGetTermId(name);
    if(result.first != -1)
    return result;
    result = second->compGetTermId(name);
    if(result.first != -1)
    return result;
    if(isTimed())
    result = time->getTermId(name);
    return result;
}

bool ComparationGoal::hasTerm(int id) const
{
    if(first->compHasTerm(id))
    return true;
    if(second->compHasTerm(id))
    return true;
    if(isTimed())
    return time->hasTerm(id);
    return false;
}

void ComparationGoal::renameVars(Unifier * u, VUndo * undo)
{
    first->compRenameVars(u,undo);
    second->compRenameVars(u,undo);
    if(isTimed())
    time->renameVars(u,undo);
}

UnifierTable * ComparationGoal::getUnifiers(const State * state, const Unifier * context, bool inheritPolarity, Protection * protection) const
{
    bool pol = true;
    bool result;
    Causal * c = 0;

    // determinar mi polaridad
    if((!getPolarity() || !inheritPolarity) && (!(!getPolarity() && !inheritPolarity)))
        pol = false;
    Compare cmp;
    pkey p1,p2;
    Function *f1=0, *f2=0;
    if(current_plan->getDomain()->isTimed()){
        if(first->isFluentLiteral())
            p1 = ((FluentLiteral *)first)->eval(state,context,&f1);
        else
            p1 = first->eval(state,context);
        if(second->isFluentLiteral())
            p2 = ((FluentLiteral *)second)->eval(state,context,&f2);
        else
            p2 = second->eval(state,context);
    }
    else {
        p1 =first->eval(state,context);
        p2 = second->eval(state,context);
    }
//    print(&cerr,0);
//    cerr << "c[" << p1.first << "," << p1.second << "]" << endl;
//    cerr << "c[" << p2.first << "," << p2.second << "]" << endl;
    if(p1.first == INT_MAX || p2.first == INT_MAX){
        return 0;
    }
    result = false;
    result = cmp(comp,p1,p2);
//    *errflow << "RETURN: " << result << endl;
    if((result && pol) || (!result && !pol)){
        Unifier * u;
        if(context)
            u = context->clone();
        else
            u = new Unifier();
        UnifierTable * ut = new UnifierTable();
        // Registrar vi�nculos
        u->createCLTable();
        if(f1 != 0){
            c = new Causal((Function *)f1,protection);
            u->addCL(c);
        }
        if(f2 != 0){
            c = new Causal((Function *)f2,protection);
            u->addCL(c);
        }

        if(current_plan->getDomain()->isTimed()){
            Protection  p;
            if(time){
                p.first = time->evalStart(context,state);
                p.second = time->evalEnd(context,state);
                protection = &p;
            }
        }

        ut->addUnifier(u);
        return ut;
    }
    else{
        return 0;
    }
};

