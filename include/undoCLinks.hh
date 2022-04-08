#ifndef UNDOCLINKS
#define UNDOCLINKS

#include "constants.hh"
#include "undoElement.hh"

class Task;

using namespace std;

/**
 * Clase para deshacer tras un backtracking los cambios
 * provocados sobre la estructura causal.
 */
class UndoCLinks: public UndoElement
{
    public:
	virtual void print(ostream * os) const ;
	virtual void undo(void);

	virtual bool isUndoCLinks(void) const {return true;};
	virtual ~UndoCLinks(void) {};

	// clave de la tarea a eliminar
	const Task * key;

	virtual UndoElement * clone(void) const {
	    UndoCLinks * ul = new UndoCLinks();
	    ul->key = key;
	    return ul;
	};

	virtual void toxml(XmlWriter * writer) const;
};

#endif
