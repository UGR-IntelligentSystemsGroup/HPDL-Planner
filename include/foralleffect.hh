#ifndef FORALLEFFECT_H
#define FORALLEFFECT_H

#include "constants.hh"
#include <assert.h>
#include "effect.hh"
#include "state.hh"
#include "header.hh"
#include "unifierTable.hh"

using namespace std;

class State;

class ForallEffect : public Effect, public ParameterContainer
{
    public:

        /**
          @brief Constructor.
         */
        ForallEffect(Effect *e);

        ForallEffect(Effect *e, const KeyList * p);

	ForallEffect(const ForallEffect * g);

	ForallEffect(void);

        /**
          @brief Devuelve el objetivo del forall.
         */
        inline Effect * getEffect() const {return effect;};

        /**
          @brief Edita el objetivo del forall.
          @param g el nuevo objetivo.
         */
        inline void setEffect(Effect * e) {effect = e;};

        /**
          @brief Destructor
         */
        virtual ~ForallEffect() {if(effect) delete effect;};

        virtual bool isForallEffect(void) const {return true;};

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

	virtual bool apply(State *sta, VUndo * undo, Unifier * uf);

	virtual bool provides(const Literal *) const;

    protected:

        Effect * effect;         /**< @brief Objetivo que deben cumplir en un estado todos los objetos
                              que tengan los mismos tipos que los par�metros del forall */

	bool assertl(State * sta, VUndo * u, Unifier * context, int pos, Unifier * uf);

};

#endif
