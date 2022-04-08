#ifndef EFFECT_HH
#define EFFECT_HH
using namespace std;

#include "constants.hh"
#include "termTable.hh"
#include "state.hh"
#include "expression.hh"
#include "timeInterval.hh"

/**
* @description Esta clase representa la base para todos los posibles efectos del planificador.
*/

class Effect :public Expression
{
    public:
	Effect(void) :Expression() {};
	Effect(const Effect * o) {};
	virtual ~Effect(void) {};
	virtual bool isAndEffect(void) const {return false;};
	virtual bool isForallEffect(void) const {return false;};
	virtual bool isLiteralEffect(void) const {return false;};
	virtual bool isEffect(void) const {return true;};
	virtual bool isFluentEffect(void) const {return false;};
	virtual bool isWhenEffect(void) const {return false;};

	/**
	  @brief Aplica el efecto sobre el estado sta. Todos los herederos deben
	  implementar este m�todo
	  @param sta El estado actual.
	  @param u Vector para almacenar la informaci�n necesaria para deshacer los
	  cambios.
	  @param uf Almacena un unificador, que puede ser null, en el caso de que
	  no lo sea ser� utilizado para almacenar dependencias de v�nculos causales.
	  @return true en caso de �xito, false en otro caso.
	  */
	virtual bool apply(State *sta, VUndo * u, Unifier * uf) { return NULL; };

	/**
	 * Analiza si el efecto es capaz de generar un literal
	 * necesario. Esta funci�n sirve para implementar un an�lisis de
	 * alcanzabilidad muy simple usado por el parser.
	 * /param l El literal a comprobar
	 */
	virtual bool provides(const Literal * l) const { return NULL; };

};

typedef vector<Effect *>::const_iterator effectcit;
typedef vector<Effect *>::iterator effectit;
#endif
