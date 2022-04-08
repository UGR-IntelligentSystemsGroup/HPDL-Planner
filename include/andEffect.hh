#ifndef ANDEFFECT_HH
#define ANDEFFECT_HH

#include "constants.hh"
#include <vector>
#include "containerEffect.hh"
#include "unifierTable.hh"
#include "termTable.hh"

using namespace std;

class AndEffect : public ContainerEffect
{
    public:
	AndEffect(void) :ContainerEffect() {};

	AndEffect(const AndEffect * ae);

	inline effectcit getBegin(void) const {return effects.begin();}; 

	inline effectcit getEnd(void) const {return effects.end();}; 

	inline int size(void) const {return effects.size();};

	void clear(void);

	virtual void addEffect(const Effect * g) {assert(g!=0); effects.push_back((Effect *) g->clone());};

	virtual void addEffectByRef(Effect * g) {assert(g!=0); effects.push_back(g);};

	virtual bool isAndEffect(void) {return true;};

	virtual ~AndEffect(void);

	/**
	 * @brief Crea una copia exacta del objeto.
	 * @description Todos los herederos deben implementar este m�todo.
	 */
	virtual Expression * clone(void) const;

	/**
	  @brief Imprime el contenido del objeto por la salida estandard.
	  @param indent el n�mero de espacios a dejar antes de la cadena.
	  */
	virtual void print(ostream * os,int indent=0) const;

	virtual void toxml(XmlWriter * writer) const;

	virtual bool apply(State *sta, VUndo * undo, Unifier * uf);

	virtual pkey getTermId(const char * name) const;

	bool hasTerm(int id) const;

	virtual void renameVars(Unifier * u, VUndo * undo);

	virtual bool provides(const Literal *) const;

    private:
	vector<Effect *> effects;
};

#endif
