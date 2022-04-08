#ifndef UNDOCHANGETYPE_HH
#define UNDOCHANGETYPE_HH
#include <vector>
#include "undoElement.hh"
#include "type.hh"
#include "constants.hh"

using namespace std;

class TypeChangeable
{
    public:
	virtual ~TypeChangeable() {};
	virtual vctype * getTypeRef(void)=0;
};

/**
 * Esta clase trata de almacenar la informaci�n necesaria para
 * poder deshacer los cambios realizados sobre los tipos de un
 * determinado objeto.
 */
class UndoChangeType: public UndoElement 
{
    public:
        vctype types;
        TypeChangeable * target;

        virtual void undo(void) 
        {
            vctype * ref = target->getTypeRef();
	    ref->clear();
            for_each(types.begin(),types.end(),AddV<Type>(ref));
        };

        UndoChangeType(TypeChangeable * target) {this->target= target;};

        UndoChangeType(TypeChangeable * target, const vctype * v) :types(*v) {this->target= target;};

        virtual ~UndoChangeType(void) {types.clear();};

        virtual void setTarget(void * t) {target = (TypeChangeable *)t;};

        virtual void * getTarget(void) const {return target;};

        virtual void print(ostream * os) const
        {
	    typecit i, e;
	    *os << "[" << target << " ";
	    e = types.end();
	    for(i = types.begin(); i!= e; i++) 
	    {
		*os << (*i)->getName() << " ";
	    }
	    *os << "]" ;
        }

	virtual bool isUndoTypeValue(void) const {return true;};

	virtual UndoElement * clone(void) const {return new UndoChangeType(target,&types);};

	virtual void toxml(XmlWriter * writer) const{
	    writer->startTag("changeType")
		->endTag();
	};
};

#endif
