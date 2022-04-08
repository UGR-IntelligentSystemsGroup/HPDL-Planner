#include "fluentOperator.hh"
#include <math.h>
#include "constants.hh"

//Variable para controlar la impresion
extern bool PRINTING_COMPARABLE;

extern bool PRINT_NUMBERTYPE;
extern bool PRINT_OBJECTTYPE;
extern bool PRINT_DEFINEDTYPES;

FluentOperator::FluentOperator(const FluentOperator * other)
    :Evaluable(other) 
{
    op = other->op; 
    first = other->first->cloneEvaluable();
    second = 0;
    if(other->second)
	second = other->second->cloneEvaluable();
    tnumber = parser_api->domain->getModificableType(0);
}

Evaluable * FluentOperator::cloneEvaluable(void) const
{
    return new FluentOperator(this);
}

const char * FluentOperator::printOp(Operation c) const
{
    static string s;
    s = "";
    switch(c)
    {
        case OADD:
            s = "+";
            break;
        case OSUBSTRACT:
            s = "-";
            break;
        case OTIMES:
            s = "*";
            break;
        case ODIVIDE:
            s = "/";
            break;
	case UABS:
	    s = ":abs";
	    break;
	case USQRT:
	    s = ":sqrt";
	    break;
	case OPOW:
	    s = ":pow";
	    break;
    }
    return s.c_str();
};

void FluentOperator::printEvaluable(ostream * os, int indent) const
{
	bool PRINT_NUMBERTYPE2, PRINT_OBJECTTYPE2, PRINT_DEFINEDTYPES2;
	
	PRINTING_COMPARABLE = true;
	
	string s(indent,' ');

	*os << s;

	*os << "(" << printOp(getOperator());

	
	//Guardamos los valores antiguos
	PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
	PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
	PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;
	
	//Fijamos los nuevos valores
	PRINT_NUMBERTYPE = false;
	PRINT_OBJECTTYPE = false;
	PRINT_DEFINEDTYPES = false;	
	
	first->printEvaluable(os,1);
	if(second){
	    second->printEvaluable(os,1);
	}

	
	//Recuperamos los valores anteriores
	PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;
	PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
	PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;	
	
	*os << ")";
	
	PRINTING_COMPARABLE = false;
}

void FluentOperator::toxmlEvaluable(XmlWriter * writer) const {
    writer->startTag("operation")
	->addAttrib("operator", printOp(getOperator()));

    first->toxmlEvaluable(writer);
    if(second)
	second->toxmlEvaluable(writer);
    writer->endTag();
}

pkey FluentOperator::compGetTermId(const char * name) const
{
    pkey result = first->compGetTermId(name);
    if(result.first != -1)
	return result;
    if(second)
	return second->compGetTermId(name);
    return result;
}

bool FluentOperator::compHasTerm(int id) const
{
    if(first->compHasTerm(id))
	return true;

    if(second)
	return second->compHasTerm(id);
    return false;
}

void FluentOperator::compRenameVars(Unifier * u, VUndo * undo)
{
    first->compRenameVars(u,undo);
    if(second)
	second->compRenameVars(u,undo);
}

pkey FluentOperator::eval(const State * state, const Unifier * u) const
{

    pkey v1 = first->eval(state,u);
    pkey v2(-1,0);
    if(second)
	v2 = second->eval(state,u);

    static pkey result;
    result.first = INT_MAX; 
    result.second = 0; 

    //printEvaluable(&cerr,0);
    //cerr << "o[" << v1.first << "," << v1.second << "]" << endl;
    //cerr << "o[" << v2.first << "," << v2.second << "]" << endl;
    // solo se operar con n�meros
    if(v1.first != -1 || v2.first != -1){
	*errflow << "FluentOperator:: Error: Evaluating expression: ";
	if(v1.first != -1)
	    *errflow << "first operand.";
	else
	    *errflow << "second operand.";
	*errflow << " Not a number or not defined." << endl;
	printEvaluable(errflow,0);
	*errflow << endl;
	exit(-1);
    }

    result.first = -1;
    switch(op)
    {
        case OADD:
	    result.second = v1.second + v2.second;
            break;
        case OSUBSTRACT:
	    if(second)
		result.second = v1.second - v2.second;
	    else
		result.second =  -v1.second;
            break;
        case OTIMES:
	    result.second = v1.second*v2.second;
            break;
        case ODIVIDE:
	    result.second = v1.second/v2.second;
            break;
	case UABS:
	    result.second = fabs(v1.second);
	    break;
	case USQRT:
	    result.second = sqrt(v1.second);
	    break;
	case OPOW:
	    result.second = pow(v1.second,v2.second);
	    break;
    }

    //cerr << "o[" << result.first << "," << result.second << "]" << endl;
    return result;
}

pkey FluentOperator::evaltp(const State * state, const Unifier * u, pkey * tp, bool * p) const{
    pkey tp1(-1,0), tp2(-1,0);
    static pkey result(INT_MAX,0);
    bool b1=true, b2=true;

    // se eval�an los dos operandos
    pkey v1 = first->evaltp(state,u,&tp1,&b1);
    pkey v2(-1,0);
    if(second)
	v2 = second->evaltp(state,u,&tp2,&b2);

    // hay diferentes posibilidades, de combinaciones de operaciones
    // entre n�meros y timepoints

    // si hay dos time points definitivamente no se como operar con ellos
    if(tp1.first != -1 && tp2.first != -1){
	*errflow << "FluentOperator:: Error: expression with two or more time points." << endl;
	printEvaluable(errflow,0);
	*errflow << endl;
	exit(-1);
    }

    // si hay un time point y se utiliza un operador distinto de la suma
    // o la resta, tampoco se como operar con �l.
    if((tp1.first != -1 || tp2.first != -1) && (op != OADD && op != OSUBSTRACT)){
	*errflow << "FluentOperator:: Error: unable to perform requested operation over a time point operator." << endl;
	printEvaluable(errflow,0);
	*errflow << endl;
	exit(-1);
    }

    // en otro caso estamos listos para operar
    // primero operamos los n�meros entre s�

    //printEvaluable(&cerr,0);
    //cerr << "o[" << v1.first << "," << v1.second << "]" << endl;
    //cerr << "o[" << v2.first << "," << v2.second << "]" << endl;
    if(v1.first != -1 || v2.first != -1){
	*errflow << "FluentOperator:: Error: Evaluating expression: ";
	if(v1.first != -1)
	    *errflow << "first operand.";
	else
	    *errflow << "second operand.";
	*errflow << " Not a number or not defined." << endl;
	printEvaluable(errflow,0);
	*errflow << endl;
	exit(-1);
    }

    result.first = -1;
    switch(op)
    {
        case OADD:
	    result.second = v1.second + v2.second;
            break;
        case OSUBSTRACT:
	    if(second)
		result.second = v1.second - v2.second;
	    else
		result.second =  -v1.second;
            break;
        case OTIMES:
	    result.second = v1.second*v2.second;
            break;
        case ODIVIDE:
	    result.second = v1.second/v2.second;
            break;
	case UABS:
	    result.second = fabs(v1.second);
	    break;
	case USQRT:
	    result.second = sqrt(v1.second);
	    break;
	case OPOW:
	    result.second = pow(v1.second,v2.second);
	    break;
    }

    // en result tenemos el nuevo n�mero
    // mirar si tambi�n hay que devolver un time-point
    if(tp1.first != -1)
	*tp = tp1;

    if(tp2.first != -1)
	*tp = tp2;

    return result;
}

