#ifndef UNDOCHANGEPRODUCER
#define UNDOCHANGEPRODUCER

#include "undoElement.hh"
#include "literal.hh"
#include "primitivetask.hh"

using namespace std;

/**
 * Cuando introducimos literales en el estado producto de la aplicaci�n
 * de alg�n efecto, puede darse el caso de que dos tareas proporcioenen
 * el mismo literal. En ese caso la �ltima tarea productora marcar� el
 * literal como producida por ella. Al volver por bactracking se debe de
 * recuperar el valor antiguo.
 * Esta clase se encarga de deshacer dichos cambios.
 */
class UndoChangeProducer: public UndoElement
{
    public:
	/**
	 * Constructor de la clase.
	 * @param l El literal al que afectan los cambios.
	 * @param op Productor que ten�a el literal antes de establecer uno nuevo.
	 */
	UndoChangeProducer(Literal * l, const PrimitiveTask * op) {lit=l; oldProducer = op;};

	/**
	 * Imprime en el flujo dado. Util para depurar.
	 * @param os el flujo por el que imprimiremos.
	 */
	virtual void print(ostream * os) const
	{
	    *os << lit->getName() << " was produced by: ";
	    if(oldProducer)
		*os << oldProducer->getName();
	    else
		*os << "initial state";
	};

	/**
	 * Restaura el producer del literal a su valor antiguo.
	 */
	virtual void undo(void)
	{
	    lit->setProducer(oldProducer);
	};

	/**
	 * Para poder hacer type castings.
	 */
	virtual bool isUndoChangeProducer(void) const {return true;};

	/** 
	 * Destructor de la clase
	 */
	virtual ~UndoChangeProducer(void) {};

	virtual UndoElement * clone(void) const {return new UndoChangeProducer(lit,oldProducer);};

	virtual void toxml(XmlWriter * writer) const{
	    writer->startTag("produced_by");
	    if(oldProducer)
		writer->addAttrib("name",oldProducer->getName());
	    else
		writer->addAttrib("name","init");
	    writer->endTag();
	};

    private:
	/** Literal al que afectan los cambios */
	Literal * lit;
	/** Su productor angituo */
	const PrimitiveTask * oldProducer;
};
#endif
