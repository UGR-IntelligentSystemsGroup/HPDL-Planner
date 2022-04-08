#ifndef UNDOARLITERALSTATE
#define UNDOARLITERALSTATE

#include "undoElement.hh"
#include "literal.hh"


/**
 * Esta clase almacena los cambios producidos al a�adir o eliminar un literal
 * del estado.
 */

class UndoARLiteralState: public UndoElement
{
    public:
	UndoARLiteralState(Literal * l, bool v);

	virtual ~UndoARLiteralState(void);

	virtual void print(ostream * os) const;

	inline bool wasAdded(void) const {return added;};
	inline void setAdded(bool v = true) {added=v;};

	inline void setLiteral(Literal * l) {ref=l;};
	inline Literal * getLiteral(void) const {return ref;};

	virtual bool isUndoARLiteralState(void) const {return true;};
	virtual void undo(void);

	virtual UndoElement * clone(void) const;
	
	// Instante de tiempo en el cual conseguimos el efecto
	// con respecto a la acci�n que gener� el efecto.
	float time;

	virtual void toxml(XmlWriter * writer) const;

    protected:
	// flag para indicar si el literal fue a�adido o eliminado
	// del estado.
	bool added;
//	bool deleted;
	// literal con el que estamos tratando
	Literal * ref;
};

#endif
