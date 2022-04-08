#ifndef FORALLGOAL_H
#define FORALLGOAL_H

#include "constants.hh"
#include <assert.h>
#include "goal.hh"
#include "state.hh"
#include "header.hh"
#include "unifierTable.hh"

using namespace std;

class State;

class ForallGoal : public Goal, public ParameterContainer
{
    public:

        /**
          @brief Constructor.
         */
        ForallGoal(Goal *g, bool polarity=true);

        ForallGoal(Goal *g, bool polarity, const KeyList * p);

	ForallGoal(const ForallGoal * g);

	ForallGoal(void);

        /**
          @brief Devuelve el objetivo del forall.
         */
        inline Goal * getGoal() const {return goal;};

        /**
          @brief Edita el objetivo del forall.
          @param g el nuevo objetivo.
         */
        inline void setGoal(Goal * g) {goal = g;};

        /**
          @brief Destructor
         */
        virtual ~ForallGoal() {if(goal) delete goal;};

        virtual bool isForallGoal(void) const {return true;};

        /**
          @brief realiza una copia exacta a this.
         */
        virtual Expression * clone(void) const;

        /**
          @brief Imprime el contenido del objeto.
          @param indent el n�mero de espacios a dejar antes de la cadena.
          @param os Un flujo de salida por defecto la salida estandard.
         */
        virtual void print(ostream * os, int indent=0) const;

        virtual void vcprint(ostream * os, int indent=0) const {print(os,indent);};

        virtual void toxml(XmlWriter * writer) const;

	virtual void vctoxml(XmlWriter * w) const {toxml(w);};

        virtual pkey getTermId(const char * name) const;

        bool hasTerm(int id) const;

        void renameVars(Unifier * u, VUndo * undo);

        virtual UnifierTable * getUnifiers(const State * state, const Unifier * context, bool inheritPolarity, Protection * protection) const;

	virtual bool isReachable(ostream * err, bool inheritPolarity) const {return goal->isReachable(err,true);};


    protected:

        /**< @brief Estructura del forall: (forall <parameters> <goal>) */

        Goal *goal;         /**< @brief Objetivo que deben cumplir en un estado todos los objetos
                              que tengan los mismos tipos que los par�metros del forall */

        /**
          @brief Devuelve todas las unificaciones posibles de las variables del forall con el
          estado sta (se mira el tipo de los par�metros)
         */
        bool test(const State * sta, Unifier * context, bool inheritPolarity, Protection * p, int pos) const;

};

#endif
