#ifndef FLUENT_OPERATOR_HH
#define FLUENT_OPERATOR_HH

#include "constants.hh"
#include "domain.hh"
#include "evaluable.hh"

using namespace std;

enum Operation {OADD,OSUBSTRACT,OTIMES,ODIVIDE,UABS,USQRT,OPOW};

/**
 * Esta clase representa las operaciones aritm�ticas binarias que
 * se llevan a cabo entre dos expresiones de tipo num�rico.
 */
class FluentOperator: public Evaluable
{
    public:
	FluentOperator(Operation o) :Evaluable() {first=0;second=0;op=o;tnumber = parser_api->domain->getModificableType(0);};

	FluentOperator(const FluentOperator * other);

	/**
	  @brief establece el primer elemento para hacer la operaci�n
	  */
	virtual void setFirst(Evaluable* first) {this->first = first;};

	/**
	  @brief establece el segundo elemento para hacer la operaci�n
	  */
	virtual void setSecond(Evaluable* second) {this->second = second;};

	/**
	  @brief establece el primer elemento para hacer la operaci�n
	  */
	virtual const Evaluable*  getFirst(void) const {return first;};

	/**
	  @brief establece el segundo elemento para hacer la operaci�n
	  */
	virtual const Evaluable* getSecond(void) const {return second;};

	virtual void setOperator(Operation o) {op = o;};

	virtual Operation getOperator(void) const {return op;};

	const char * printOp(Operation c) const;

	void printEvaluable(ostream * os, int indent) const;

	void toxmlEvaluable(XmlWriter * writer) const;

	void compRenameVars(Unifier * u, VUndo * undo);

	bool compHasTerm(int id) const;

	pkey compGetTermId(const char * name) const;

	Evaluable * cloneEvaluable(void) const;

	virtual pkey eval(const State * state, const Unifier * u) const;

	virtual pkey evaltp(const State * state, const Unifier * u, pkey * tp, bool * p) const;

	virtual bool isType(const Type * t) const {return tnumber->isSubTypeOf(t);};

    protected:
	/** El primer operando */
	Evaluable * first;
	/** El segundo operando */
	Evaluable * second;
	/** El operador */
	Operation op;
	/** El tipo resultante de la operacion (numerico) */
	const Type * tnumber;
};
#endif
