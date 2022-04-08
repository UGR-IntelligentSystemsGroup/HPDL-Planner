#ifndef UNDOELEMENT
#define UNDOELEMENT

#include "constants.hh"
#include <iostream>
#include <vector>
#include "xmlwriter.hh"
using namespace std;

/**
 * Esta clase trata de servir de base para todas aquellas estructuras
 * de datos que mantienen informaci�n para durante el proceso de backtracking
 * deshacer los cambios que se hicieron sobre las ED al ir hacia adelante.
 * La informaci�n que hay que mantener es demasiado variopinta para que esta
 * clase base implemente mucha funcionalidad, lo que tratamos es intentar
 * disponer de una clase ra�z para tratar de unificar toda la informaci�n.
 */

class UndoElement
{
    public:
	// Es interesante poder mostrar por pantalla el contenido de las
	// ED de undo. De esta forma se facilita la depuraci�n
	virtual void print(ostream * os) const = 0;
	// Se supone que cada elemento almacena informaci�n suficiente para
	// poder hacer el undo por s� mismo sin necesidad de m�s par�metros.
	virtual void undo(void) = 0;

	virtual bool isUndoChangeValue(void) const {return false;};
	virtual bool isUndoTypeValue(void) const {return false;};
	virtual bool isUndoARLiteralState(void) const {return false;};
	virtual bool isUndoChangeProducer(void) const {return false;};
	virtual bool isUndoCLinks(void) const {return false;};
	virtual bool isUndoChangeTime(void) const {return false;};
	virtual ~UndoElement(void) {};
	virtual UndoElement * clone(void) const = 0;

	/**
	 * Devuelve una descripci�n como documento xml del cambio realizado.
	 **/
	virtual void toxml(ostream * os) const{
	    XmlWriter w(os);
	    toxml(&w);
	    w.flush();
	};

	virtual void toxml(XmlWriter * writer) const = 0;
};

typedef vector<UndoElement *> VUndo;
typedef VUndo::iterator undoit;
typedef VUndo::const_iterator undocit;

#endif
