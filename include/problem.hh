#ifndef PROBLEM_H
#define PROBLEM_H
#include "constants.hh"
#include <list>
#include "tasknetwork.hh"
#include "state.hh"
#include "papi.hh"
#include "meta.hh"

using namespace std;

class Problem
{
    public:

	/**
	  @brief Constructor
	  */
	Problem(const Domain * domain);

	/**
	  @brief Destructor
	  */
	virtual ~Problem(void);

	inline void setInitialGoal(TaskNetwork * g) {this->tasknetwork= g;};

	inline isit addToInitialState(Literal * l) {return state->addLiteral(l);};

	inline const char * getName(void) const {return name.c_str();};

	inline void setProblemName(const char * n) {name = n;};

	/**
	 * @brief Devuelve un puntero al estado inicial del problema.
	 */
	inline const State * getInitialState(void) const {return state;};

	/**
	 * Devuelve la red de tareas objetivo para resolver.
	 */
	inline const TaskNetwork * getInitialGoal(void) const {return tasknetwork;};

	/**
	 * Metainformaci�n asociada al problema
	 **/
	Meta meta;

	// Distintas banderas de configuraci�n.

	/** Unidad de tiempo seleccionada */
	TimeUnit FLAG_TIME_UNIT;

	/** Cuando comienza el plan */
	time_t FLAG_TIME_START;

	/** Que formato de tiempo se usa para la impresi�n */
	string TFORMAT;

	/** Control del time-horizon */
	int MAX_THORIZON;
	int REL_THORIZON;

    protected:
	/** Estructura para el estado inicial */
	State * state;

	/** Estructura que mantiene informaci�n sobre la red de tareas inicial */
	TaskNetwork * tasknetwork;

	/** Nombre del problema */
	string name;
};

#endif
