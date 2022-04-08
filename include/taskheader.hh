#ifndef TASK_HEADER_H
#define TASK_HEADER_H

#include "constants.hh"
#include "constantsymbol.hh"
#include <assert.h>
#include "term.hh"
#include "variablesymbol.hh"
#include "task.hh"

using namespace std;

class TaskHeader: public Task {
public:
    /**
    @brief Constructor.
    @param name El nombre de la tarea.
    */
    TaskHeader(int id, int mid);

    TaskHeader(int id, int mid, const KeyList * v);

    TaskHeader(const TaskHeader * t);

    virtual ~TaskHeader(void) {};

    /**
    * @brief La funci�n es una descripci�n de acci�n
    */
    inline virtual bool isTaskHeader(void) const {return true;};

    /**
    @brief realiza una copia exacta a this.
    */
    virtual Expression * clone(void) const;

    /**
    @brief Imprime el contenido del objeto.
    @param os Un flujo de salida por defecto la salida estandard.
    */
    virtual void print(ostream * os, int indent = 0) const;

    virtual void vcprint(ostream * os, int indent = 0) const {
        print(os, indent);
    }
    ;

    virtual void toxml(XmlWriter * writer) const {
        toxml(writer, true);
    }
    ;

    virtual void toxml(XmlWriter * writer, bool complete) const;

    virtual void vctoxml(XmlWriter * w) const {
        toxml(w);
    }
    ;

    virtual void printHead(ostream * os) const {
        *os << ":unexpanded ";
        print(os);
    }
    ;

    virtual pkey getTermId(const char * name) const;

    virtual bool hasTerm(int id) const;

    virtual void renameVars(Unifier * u, VUndo * undo);

    virtual bool isReachable(ostream * err) const {
        return true;
    }
    ;

    virtual bool provides(const Literal * l) const {
        return false;
    }
    ;

};

#endif
