#ifndef AXIOM_H
#define AXIOM_H

#include "constants.hh"
#include "goal.hh"
#include "header.hh"
#include <vector>

using namespace std;

class Axiom: public Header
{
    public:

    /**
      @brief Constructor
      */
    Axiom(void) :Header() {goal=0;code=0;};

    Axiom(int id, int mid) :Header(id,mid) {goal=0;code=0;};

    Axiom(int id, int mid, const KeyList * v) :Header(id,mid,v) {goal=0;code=0;};

    Axiom(int id, int mid, const KeyList * v, Goal * g) :Header(id,mid,v) {goal=g;code=0;};

    /**
     * El constructor de copia.
     * /param other. El objeto a clonar
     */
    Axiom(const Axiom * other);

    /**
      @brief Destructor
      */
    virtual ~Axiom(void) {
        if(goal) {delete goal;} 
#ifdef PYTHON_FOUND
        if(code) {Py_DECREF(code);}
#endif
    };

    /**
      @brief Devuelve el antecedente del axioma
      */
    inline Goal * getGoal() const {return goal;};

    /**
      @brief Edita el antecedente al axioma.
      @param g: Objetivo que vamos a editar.
      */
    inline void setGoal(Goal * g) {goal=g;};

    /**
     * @brief realiza una copia exacta a this.
     */
    virtual Axiom * clone(void) const {return new Axiom(this);};

    /**
      @brief Imprime el contenido del objeto.
      @param os Un flujo de salida.
      */
    virtual void print(ostream * os, int indent=0) const;

        virtual void vcprint(ostream * os, int indent=0) const {print(os,indent);};

    virtual void toxml(ostream * os) const;

    virtual void toxml(XmlWriter * w) const;

    virtual void vctoxml(XmlWriter * w) const {toxml(w);};

        virtual const char * toString(void) const {static string s; ostringstream os; print(&os,0); s = os.str(); return s.c_str();}; 

    /**
     * Dado un estado y contexto de unificaciones, verifica si el
     * axioma es valido.
     * @param state El estado contra el que se verificar� el axioma.
     * @param context El contexto de unificaci�n.
     * @param pol La polaridad si es negativa se supondr� un (not axioma)
     * @return Una tabla de unificaciones no vac�a en caso de �xito al evaluar el
     * axioma, null o una tabla vac�a en otro caso.
     */
    virtual UnifierTable * test(const State * state, const Unifier * context, bool pol, Protection * p);

    /**
     * Devuelve true si el axioma est� codificado con python
     */
    inline bool isPython(void) const {return (code != 0);};

    /**
     * Establece el c�digo python necesario para ejecutar el
     * axioma.
     * @param c el c�digo a compilar.
     */
    bool setCode(const char * c);

    protected:
    Goal *goal;     /**< @brief Antecedente: Objetivo que tendr� que cumplirse para que el axioma pueda aplicarse */
    /** Objeto python que almacena el c�digo a ajecutar */
    PyObject * code;
};

#endif
