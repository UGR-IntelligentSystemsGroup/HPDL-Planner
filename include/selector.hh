#ifndef SELECTOR_HH
#define SELECTOR_HH

#include "constants.hh"
#include "stacknode.hh"
#include "state.hh"
#include "tasknetwork.hh"

using namespace std;

class Plan;

/**
 * Esta clase implementa el comportamiento básico del planificador realizando
 * una búsqueda primero en profuncidad. Hay 4 puntos de backtracking, cada
 * uno de ellos tiene una función de selección asociada en esta clase. Si se quiere
 * modificar el comportamiento por defecto, es necesario heredar de esta
 * clase y reimplementar las funciones de manera adecuada.
 */
class Selector{
    public:
	virtual ~Selector(void){};
	/**
	 * Dada la lista de tareas que se encuentran en la agenda por expandir selecciona
	 * una de ellas.
	 * @param plan El plan sobre el que operamos.
	 * @return El índice de la lista de tareas unordered pendientes en la agenda, que se
	 * quiere explorar.
	 * Devuelve -1 para forzar backtrack.
	 * Devuelve -2 para detener el proceso de planificación.
	 */
	virtual int selectFromAgenda(Plan * plan);

	/**
	 * De las distintas tareas que pueden unificar
	 * seleccionar una de ellas
	 * @param plan El plan sobre el que operamos.
	 * @return El índice de los hijos offspring del contexto que se quiere expandir. 
	 * Devuelve -1 para forzar backtrack.
	 * Devuelve -2 para detener el proceso de planificación.
	 */
	virtual int selectTask(Plan * plan);

	/**
	 * Si la tarea en expansión es abstracta entonces puede tener distintos métodos que se pueden 
	 * ejecutar. Esta función permite seleccionar el orden en el que se evalúan los métodos.
	 * @param plan El plan sobre el que operamos.
	 * @return El objeto contexto de tipo StackNode mantiene la lista de métodos methods que se puede
	 * intentar aplicar.
	 * Devuelve -1 para forzar backtrack.
	 * Devuelve -2 para detener el proceso de planificación.
	 */
	virtual int selectMethod(Plan * plan);

	/**
	 * Selecciona alguna de las posibles unificaciones encontradas por el algoritmo.
	 * @param plan El plan sobre el que operamos.
	 * @return Un entero indicando el índice en la lista de unificaciones encontradas 
	 * que se desea aplicar y que se encuentra en el objeto de contexto sn->unif
	 * Devuelve -1 para forzar backtrack.
	 * Devuelve -2 para detener el proceso de planificación.
	 */
	virtual int selectUnification(Plan * plan);
};

#endif /* SELECTOR_HH */
