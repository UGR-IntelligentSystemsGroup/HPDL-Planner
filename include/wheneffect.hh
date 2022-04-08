#ifndef WHENEFFECT_H
#define WHENEFFECT_H

#include "constants.hh"
#include <assert.h>
#include "effect.hh"
#include "state.hh"
#include "goal.hh"
#include "effect.hh"
#include "unifierTable.hh"

using namespace std;

class State;

class WhenEffect : public Effect
{
    public:

	/**
	  @brief Constructor.
	  */
	WhenEffect(const WhenEffect *eff);
	WhenEffect(Goal *g, Effect *eff);

	/**
	  @brief Devuelve el objetivo del when.
	  */
	inline Goal * getGoal(void) const {return goal;};

	/**
	  @brief Devuelve el efecto del when.
	  */
	inline Effect * getEffect(void) const {return effect;};

	/**
	  @brief Edita el objetivo del when.
	  @param g el nuevo objetivo.
	  */
	inline void setGoal(Goal * g) {goal = g;};

	/**
	  @brief Edita el efecto del when.
	  @param e el nuevo efecto.
	  */
	inline void setEffect(Effect * e) {effect = e;};

	/**
	  @brief Destructor
	  */
	virtual ~WhenEffect(void);

	virtual bool isWhenEffect(void) const {return true;};

	/**
	  @brief realiza una copia exacta a this.
	  */
	virtual Expression * clone(void) const;

	/**
	  @brief Imprime el contenido del objeto.
	  @param indent el n�mero de espacios a dejar antes de la cadena.
	  @param os Un flujo de salida por defecto la salida estandard.
	  */
	virtual void print(ostream * os, int nindent=0) const;

	virtual void toxml(XmlWriter * writer) const;

	virtual pkey getTermId(const char * name) const;

	virtual bool hasTerm(int id) const;

	virtual void renameVars(Unifier * u, VUndo * undo);

	virtual bool apply(State *sta, VUndo * u, Unifier * uf);

	virtual bool provides(const Literal*) const;

    protected:

	/**< @brief Estructura del when: (when <goal> <effect>) */

	Goal *goal;         /**< @brief Objetivo que debe cumplirse para aplicar el efecto */
	Effect *effect;     /**< @brief Efecto a aplicar si se cumple el objetivo */
};

#endif
