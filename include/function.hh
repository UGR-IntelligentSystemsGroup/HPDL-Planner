#ifndef FUNCTION_HH
#define FUNCTION_HH

using namespace std;

#include "constants.hh"
#include <limits.h>
#include "literaleffect.hh"

struct EqualPair
{
    const Unifier * u;

    EqualPair(const Unifier * u) {this->u =u;};
    EqualPair(void) {u =0;};

    bool operator()(const pair<int ,float> &pa,const pair<int ,float> &pb) const
    {
    pkey a = pa;
    pkey b = pb;
    if(u)
    {
        if(a.first < -1)
        u->getSubstitution(a.first,&a);
        if(b.first < -1)
        u->getSubstitution(a.first,&b);
    }
    // cerr << endl << a.first << "," << a.second << " == "<< b.first << "," << b.second<< endl;
    if(a.first == b.first) {
        if(a.first > 0){
        return true;
        }
        else if(a.second == b.second){
        return true;
        }
    }
    return false;
    };
};


/**
 * Esta clase define las funciones fluent que se almacenan en el estado de
 * planificaci�n.
 */
class Function: public LiteralEffect
{
  public:
      /**
       * Constructor de clase.
       * @param id el identificador de la instancia
       * @param mid el metaidentificador
       * @param p La polaridad
       * @param val El valor almacenado. 
       */
      Function(int id, int mid, bool p, double val);

      /**
       * Constructor de clase.
       * @param id el identificador de la instancia
       * @param mid el metaidentificador
       * @param p La polaridad
       * @param val El valor almacenado. 
       * @param parameters La lista de par�metros del fluent.
       */
      Function(int id, int mid, bool p, double val,const KeyList * parameters);

      /**
       * El constructor de copia
       * @param c La funci�n a clonar
       */
      Function(const Function *c);

      /**
       * Saber si el objeto es una funci�n
       * @return siempre true.
       */
      virtual bool isFunction(void) const {return true;};

      /**
       * Establecer un nuevo valor contenido por la funci�n.
       * @param v El valor.
       */
      inline void setValue(double v) {value.first=-1; value.second = v;};

      /**
       * Establecer un nuevo valor contenido por la funci�n.
       * @param v El valor.
       */
      inline void setValue(pkey &nv) {value = nv;};

      /**
       * Devuelve el valor almacenado en el fluent.
       * @return pkey con el valor.
       */
      inline pkey getValue(void) const {return value;};

      /**
       * Esta funci�n es usada por los undo
       */
      virtual void setVar(int pos, pkey &newval) {if(pos == -1) value = newval; else setParameter(pos,newval);}

      /**
       * Obtener una descripci�n de la funci�n como una cadena.
       * @return un puntero a cadena
       */
      virtual const char * toString(void) const {static string s; ostringstream os; printL(&os,0); s = os.str(); return s.c_str();}; 

      /**
       * Funci�n heredada de los literales, imprime una descripci�n de la
       * funci�n en un flujo de salida que se pasa como argumento.
       * @param os El flujo donde escribiremos
       * @param indent un n�mero de espacios que se dejar�n antes de imprimir
       */
      virtual void printL(ostream * os, int indent=0) const;

      /**
       * Realiza un clon de esta clase.
       */
      virtual Literal * cloneL(void) const;

      /**
       * Devuelve en un documento xml la descripci�n de la tarea.
       * @param writer el objeto donde escribir.
       **/
      virtual void toxml(XmlWriter * writer) const;

      /**
       * Devuelve en un documento xml la descripci�n de la tarea.
       * @param writer el objeto donde escribir.
       **/
      virtual void toxmlL(XmlWriter * writer) const {toxml(writer);};

      /**
       * Obtiene la polaridad de la funci�n.
       */
      virtual bool getPol(void) const {return true;};

      /**
       * Establece la polaridad de la funci�n.
       */
      virtual void setPol(bool t) {};

      virtual void vcprint(ostream * os, int indent=0) const {printL(os,indent);};

  protected:
      // la funci�n si no es python deber�a tener asociado un valor
      pkey value;
};
#endif
