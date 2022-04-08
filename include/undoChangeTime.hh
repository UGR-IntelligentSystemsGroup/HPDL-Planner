#ifndef UNDOCHANGETIME_HH
#define UNDOCHANGETIME_HH

#include "constants.hh"
#include "undoElement.hh"
#include "timeStamped.hh"

/** 
 * Esta clase almacena la informaci�n necesaria para deshacer los cambios
 * provocados sobre el time stamp de un objeto 
 * una variable o un literal.
 */
class UndoChangeTime :public UndoElement 
{
    public:
	/**
	 * Constructor de la clase.
	 * @param o El objeto sobre el cual se aplican cambios de tiempo.
	 * @param time Es el tiempo antiguo que se pretende recuperar 
	 * tras hacer el undo.
	 */
	UndoChangeTime(TimeStamped * o, Evaluable * time, const Evaluable * new_time) {
	    obj=o; 
	    oldTime=time;
	    if(new_time)
		newTime = new_time->cloneEvaluable();
	    else
		newTime = 0;
	};

	/**
	 * Destructor.
	 */
	~UndoChangeTime() {
	    if (oldTime) delete oldTime;
	    if (newTime) delete newTime;
	}

	/**
	 * La funci�n que realiza el undo.
	 */
        virtual void undo(void){
	    // se borra el valor actual del time si hay alguno
	    obj->resetTime();
	    obj->setTime(oldTime);
	    oldTime = 0;
	};

        virtual void print(ostream * os) const {*os << "Time change to: "; if(oldTime) oldTime->printEvaluable(os); else *os << "null"; *os << endl;};

	virtual bool isUndoChangeTime(void) const {return true;};

	UndoElement* clone(void) const{return new UndoChangeTime(obj,oldTime->cloneEvaluable(),newTime);};

	virtual void toxml(XmlWriter * writer) const{
	    writer->startTag("changeTime");
	    writer->startTag("from");
	    if(oldTime)
		oldTime->toxmlEvaluable(writer);
	    writer->endTag();
	    writer->startTag("to");
	    if(newTime)
		newTime->toxmlEvaluable(writer);
	    writer->endTag();
	    writer->endTag();
	};

    protected:
	/** objeto sobre el que aplicar el undo. */
	TimeStamped * obj;
	/** El tiempo antiguo */
	Evaluable * oldTime;
	/** El nuevo tiempo */
	Evaluable * newTime;
};

#endif
