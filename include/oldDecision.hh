#ifndef OLDDECISION_HH
#define OLDDECISION_HH   

#include "constants.hh"
#include "stacknode.hh"

using namespace std;

typedef vector<Task *> VTask;
typedef vector<pair<int,int> > VODAgenda;

/**
 * Esta clase sirve para mantener la cuenta de las decisiones que se han llevado a
 * cabo con anterioridad y que pueden ser útiles para el proceso de replanificación.
 */
class OldDecision{
    public:
	/**
	 * @brief La información necesaria para mantener las decisiones que se
	 * realizaron con anterioridad se extrae del contexto que se utilizó
	 * durante la etapa de planificación.
	 */
	OldDecision(const StackNode * st, const TaskNetwork * tn);

	/**
	 * Destructor
	 */
	~OldDecision(void);

	void print(ostream * os, int tab=0) const;

	/** control de agenda primer punto de backtracking*/
	Task * task_selected;

	/** flag que dice si la tarea es o no primitiva */
	bool primitive;

	/** Si la tarea es compuesta el id del método que se expandió */
	int method_id;

	/** El unificador que se seleccionó */
	Unifier * u;
};

#endif /* OLDDECISION_HH */
