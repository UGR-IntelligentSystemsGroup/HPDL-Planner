#ifndef TIMESTAMPED_HH
#define TIMESTAMPED_HH   

#include "constants.hh"
#include "evaluable.hh"

/**
 * Esta clase será heredada por aquellos objetos que puedan llevar
 * alguna marca de tiempo (típicamente un efecto temporizado).
 */

class TimeStamped{
    public:
	/**
	 * Constructor por defecto.
	 */
	TimeStamped(void) {
	    time = 0;
	};

	/**
	 * Constructor de copia.
	 */
	TimeStamped(const TimeStamped * other) {
	    if(other->time) 
		time = other->time->cloneEvaluable();
	    else time=0;
	};

	/**
	 * El destructor de la clase.
	 */
	virtual ~TimeStamped() {
	    if(time) 
		delete time;
	};

	/**
	 * Establece el momento en el cual se consigue el objetivo.
	 * Ojo que la función hace uso del puntero directamente, no lo clona.
	 */
	inline void setTime(Evaluable * c) {
	    time = c;
	};

	/**
	 * Obtiene el momento en el cual se consigue el objetivo.
	 */
	inline const Evaluable * getTime(void) const {return time;};

	inline Evaluable * getModificableTime(void) const {return time;};

	/**
	 * Evalua el momento en el cual se hace cierto el efecto respecto a la
	 * tarea que lo consique.
	 */
	virtual float evalTime(void) const {
	    if(time)
		return time->eval(0,0).second;
	    else
		return ATEND;
	};

	virtual void setNewTime(const Evaluable * o) {
	    if(time) 
		delete time; 
	    time=0; 
	    if(o) 
		time = o->cloneEvaluable();
	};

	virtual void resetTime(void) {
	    if(time) 
		delete time; 
	    time = 0;
	};

	virtual bool isTimed(void) const {return (time != 0);};

    protected:
	/** Marca de tiempo en la cual se consigue el efecto */ 
	Evaluable * time;
};

#endif /* TIMESTAMPED_HH */
