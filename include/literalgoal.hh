#ifndef LITERALGOAL_HH
#define LITERALGOAL_HH

#include "constants.hh"
#include <assert.h>
#include "goal.hh"
#include "unifierTable.hh"
#include "header.hh"
#include <tr1/unordered_map>

using namespace std;

class State;

class LiteralGoal : public Goal, public Literal 
{
    public:

        /**
          @brief Constructor.
          @param n nombre del literal.
          @param p La polaridad, por defecto positiva.
         */
        LiteralGoal(int id, int mid,  bool p=true);

        LiteralGoal(int id, int mid, const KeyList * param, bool p=true);

	LiteralGoal(const LiteralGoal * other);

        virtual ~LiteralGoal(void) {};

        virtual bool isLiteral(void) const {return true;};

        /**
        * @brief realiza una copia exacta a this.
        */
        virtual Expression * clone(void) const;

        virtual Literal * cloneL(void) const {return (Literal *) clone();};

        /**
        @brief Imprime el contenido del objeto.
        @param os Un flujo de salida.
        */
        virtual void print(ostream * os, int indent=0) const;

        virtual void vcprint(ostream * os, int indent=0) const {print(os,indent);};

        virtual void toxmlL(XmlWriter * writer) const {toxml(writer);};;

        virtual void toxml(XmlWriter * writer) const;

	virtual void vctoxml(XmlWriter * w) const {toxml(w);};

        /**
          @brief Dado un estado, se calculan todas las posibles unificaciones, de este literal con el estado.
          @param context, son sustituciones hechas con anterioridad, se puede dejar a null. Por ejemplo en el caso
          de (and (l1 ?x ?y) (l2 ?x ?w)), la posible sustituci�n de ?x hecha durante la unificaci�n de l1 debe de tenerse
          en cuenta para la unificaci�n de l2.
          @return El resultado se devuelve en una tabla de unificadores.
         */
        virtual UnifierTable * getUnifiers(const State * state, const Unifier * context, bool inheritPolarity, pair<unsigned int,unsigned int> * protection) const;

        virtual pkey getTermId(const char * name) const;

        virtual bool hasTerm(int id) const;

        virtual void renameVars(Unifier * u, VUndo * undo);

	virtual bool isReachable(ostream * err, bool pol) const;

        virtual void printL(ostream * os, int indent=0) const {print(os,indent);};

	virtual bool getPol(void) const {return getPolarity();};

	virtual void setPol(bool t) {setPolarity(t);};

};

typedef std::tr1::unordered_multimap<int, Literal *> ISTable;
typedef ISTable::const_iterator iscit;
typedef ISTable::iterator isit;

#endif
