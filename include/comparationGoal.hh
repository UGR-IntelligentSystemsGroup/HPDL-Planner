#ifndef COMPARATIONGOAL_HH
#define COMPARATIONGOAL_HH

#include "constants.hh"
#include "unifierTable.hh"
#include "termTable.hh"
#include "evaluable.hh"
#include "goal.hh"

using namespace std;

enum Comparator {CEQUAL,CGREATHER,CLESS,CLESS_EQUAL,CGREATHER_EQUAL,CDISTINCT};

struct Compare
{
    bool operator()(Comparator c, const pkey & a, const pkey & b)
    {
        switch(c)
        {
            case CEQUAL: return (a.first == b.first && a.second == b.second); break;
            case CDISTINCT: return (a.first == b.first && a.second != b.second); break;
            case CGREATHER: return (a.first == -1 && b.first == -1 && a.second > b.second); break;
            case CLESS: return (a.first == -1 && b.first == -1 && a.second < b.second); break;
            case CLESS_EQUAL: return (a.first == -1 && b.first == -1 && a.second <= b.second); break;
            case CGREATHER_EQUAL: return (a.first == -1 && b.first == -1 && a.second >= b.second); break;
            default: return false;
        };
    return false;
    };
};

using namespace std;

/**
 * Esta clase realiza la comparaci�n entre dos elementos
 * generalmente num�ricos.
 */
class ComparationGoal :public Goal
{
  public:
    ComparationGoal(void);

    virtual bool isComparationGoal(void) {return true;};

    virtual ~ComparationGoal(void);

    /**
     * @brief Crea una copia exacta del objeto.
     * @description Todos los herederos deben implementar este m�todo.
     * @author oscar
    */
    virtual Expression * clone(void) const;

    /**
     @brief Imprime el contenido del objeto por la salida estandard.
     @param indent el n�mero de espacios a dejar antes de la cadena.
    */
    virtual void print(ostream * os, int indent) const;

    virtual void toxml(XmlWriter * writer) const;

    /**
     @brief Dada una tarea primitiva, y una serie de unificaciones, se construyen todas las
     posibles tareas primitivas resultado de todas las posibles unificaciones.
    */
    virtual UnifierTable * getUnifiers(const State * state, const Unifier * context, bool inheritPolarity, pair<unsigned int,unsigned int> * protection) const;

    /**
     @brief establece el primer elemento para hacer la comparaci�n
    */
    virtual void setFirst(Evaluable * f) {assert(f); assert(!first); first = f;};

    /**
     @brief establece el segundo elemento para hacer la comparaci�n
    */
    virtual void setSecond(Evaluable * s) {assert(s); assert(!second); this->second = s;};

    /**
     @brief establece el primer elemento para hacer la comparaci�n
    */
    virtual const Evaluable *  getFirst(void) const {return first;};

    /**
     @brief establece el segundo elemento para hacer la comparaci�n
    */
    virtual const Evaluable * getSecond(void) const {return second;};

    virtual void setComparator(Comparator c) {comp = c;};

    virtual Comparator getComparator(void) const {return comp;};

    virtual pkey getTermId(const char *) const;

    virtual bool hasTerm(int id) const;

    virtual void renameVars(Unifier * u, VUndo * undo);

    virtual bool isReachable(ostream * err, bool inheritPolarity) const {return true;};

  protected:
    Evaluable * first;
    Evaluable * second;
    Comparator comp;

    const char * printComp(Comparator c) const;
};

#endif
