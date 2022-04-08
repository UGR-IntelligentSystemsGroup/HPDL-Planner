#ifndef GOAL_HH
#define GOAL_HH
using namespace std;

#include "constants.hh"
#include "unifier.hh"
#include "variablesymbol.hh"
#include "termTable.hh"
#include "expression.hh"
#include "unifierTable.hh"
#include "timeInterval.hh"


/**
 * @author  Oscar
 * @description Esta clase representa la base para todos los posibles objetivos del planificador.
 */

class State;

class Goal : public Expression
{
    public:

	/**
	 * Constructor por defecto
	 */
	Goal(void) {polarity = true;time=0;};

	/**
	 * Constructor de copia.
	 * Observar que esta clase es abstracta.
	 * /param other La precondici�n que deseamos copiar.
	 */
	Goal(const Goal * other)
	{
	    polarity = other->polarity;
	    if(other->time)
		time = other->time->clone();
	    else
		time =0;
	};

	virtual ~Goal(void) {};

	virtual bool isLiteral(void) const {return false;};
	virtual bool isAndGoal(void) const {return false;};
	virtual bool isOrGoal(void) const {return false;};
	virtual bool isImplyGoal(void) const {return false;};
	virtual bool isForallGoal(void) const {return false;};
	virtual bool isExistsGoal(void) const {return false;};
	virtual bool isGoal(void) const {return true;};
	virtual bool isComparationGoal(void) const {return false;};
	virtual bool isFunction(void) const {return false;};
	virtual bool isCutGoal(void) const {return false;};
	virtual bool isSortGoal(void) const {return false;};
	virtual bool isBoundGoal(void) const {return false;};

	/**
	 * Comprueba si la precondic�n se encuentra temporizada
	 */
	inline bool isTimed(void) const {return time != 0;};

	/**
	  @brief Asignamos un nuevo signo al literal.
	  @param s: Nuevo signo.
	  */
	inline void setPolarity(bool s) {polarity=s;};

	/**
	  @brief Leemos el signo actual del literal.
	  @return El signo.
	  */
	inline bool getPolarity(void) const {return polarity;};

	/**
	  @brief Dado un estado, y una precondicion se calculan todas las posibles unificaciones.
	  @param context, son sustituciones hechas con anterioridad, se puede dejar a null. Por ejemplo en el caso
	  de (and (l1 ?x ?y) (l2 ?x ?w)), la posible sustituci�n de ?x hecha durante la unificaci�n de l1 debe de tenerse
	  en cuenta para la unificaci�n de l2.
	  @param state El estado actual en que se encuentra el mundo.
	  @param inheritPolarity Se debe inicializar a true, va llevando de forma recursiva si la polaridad es positiva
	  o negativa (el literal est� o no negado)
	  @return El resultado se devuelve en una tabla de unificadores.
	  */
	virtual UnifierTable * getUnifiers(const State * state, const Unifier * context, bool inheritPolarity, pair<unsigned int,unsigned int> * protection) const { return NULL; };

	/**
	 * Comprueba de una forma muy sencilla si la precondici�n es alcanzable.
	 * Esta funci�n es usada por el parser para hace un an�lisis de alcanzabilidad
	 * muy sencillo.
	 * /param err Flujo por el que se mostrar�n los mensajes de error en caso de
	 * producirse.
	 * /param pol Polaridad heredada de los goal que me contienen
	 */
	virtual bool isReachable(ostream * err, bool pol) const { return false; };
  //virtual bool isReachable(ostream * err, bool pol){return true;}

	/**
	 * Establece la temporizaci�n para la expresi�n.
	 * Usar con cuidado, es una funci�n de uso interno.
	 * La temporizaci�n tiene que mantener el mismo
	 * �mbito que las variables usadas en el efecto.
	 * /param ts La nueva temporizaci�n
	 */
	inline void setTime(TimeInterval * ts) {time = ts;};

	/**
	 * Obtiene la temporizaci�n de la expresi�n.
	 */
	inline const TimeInterval * getTime(void) const {return time;};

    protected:
	/** Estructura que mantiene la temporizaci�n */
	TimeInterval * time;

	bool polarity; /**< @brief El signo del literal */
};

/**
 * Definici�n de un iterador constante para un vector de Goal *
 */
typedef vector<Goal *>::const_iterator goalcit;

/**
 * Definici�n de un iterador para un vector de Goal *
 */
typedef vector<Goal *>::iterator goalit;
#endif
