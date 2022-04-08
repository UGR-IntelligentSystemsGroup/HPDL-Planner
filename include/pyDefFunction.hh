#ifndef PY_DEF_FUNCTION_HH
#define PY_DEF_FUNCTION_HH

using namespace std;

#include "constants.hh"
#include <limits.h>
#include "literal.hh"
#include "pythonWrapper.hh"

/**
 * Esta clase almacena las definiciones de funciones PDDL y adem�s simult�neamente 
 * las definiciones de las funciones python.
 * Ambas son equivalentes, se pueden usar en los mismos lugares de la 
 * descripci�n de dominio y problema. 
 * Se diferencian a la horad e evaluarlas (llamada a python), o buscar el
 * objeto Function almacenado en el estado. 
 * @see Function
 */

class PyDefFunction: public Literal 
{
  public:
      /**
       * Constructor de clase.
       * @param id el identificador de la instancia
       * @param mid el metaidentificador
       */
      PyDefFunction(int id, int mid);

      /**
       * Constructor de clase.
       * @param id el identificador de la instancia
       * @param mid el metaidentificador
       * @param parameters Una lista de par�metros 
       */
      PyDefFunction(int id, int mid, const KeyList * parameters);

      /**
       * Destructor de la clase.
       */
      virtual ~PyDefFunction(void);

      /**
       * Devuelve true siempre.
       */
      virtual bool isFunction(void) const {return true;};

      /**
       * Distingue entre definiciones normales y definiciones
       * de funciones python.
       */
      inline bool isPython(void) const {return code != 0;};

      /**
       * Establece el c�digo de una funci�n python.
       */
      inline void setCode(PyObject * c) {code = c;};

      /**
       * Devuelve el c�digo asociado a la funci�n.
       */
      inline PyObject * getCode(void) const {return code;};

      /**
       * Establece el c�digo python sin compilar
       */
      virtual bool setCode(const char * c);

      /**
       * Devuelve una descripci�n como una cadena de la definici�n
       * de funci�n.
       * @return un puntero a cadena con la descripci�n del objeto.
       */
      virtual const char * toString(void) const {static string s; ostringstream os; printL(&os,0); s = os.str(); return s.c_str();}; 

      /**
       * Imprime en un flujo de salida una descripci�n del objeto.
       */
      virtual void printL(ostream * os, int indent=0) const;

      virtual void vcprint(ostream * os, int indent=0) const {printL(os,indent);};

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

      virtual void vctoxml(XmlWriter * w) const {toxml(w);};

      /**
       * A�ade un tipo a la lista soportada por la funci�n
       */
      virtual void compAddType(Type * t) {assert(t != 0); types.push_back(t);};

      /**
       * A�ade la lista de tipos soportada por la funci�n.
       */
      virtual void addTypes(const vector<Type *> * vt) {types.insert(types.end(),vt->begin(),vt->end());};

      /**
	@brief Devuelve el primer tipo del t�rmino. 
	@return el primer tipo o getEndType() si el t�rmino es de tipo object.
	*/
      virtual typecit getBeginType(void) const {return types.begin();};

      /**
	@brief Devuelve un iterador uno despu�s del �ltimo elemento. 
	*/
      virtual typecit getEndType(void) const {return types.end();};

      /**
	@brief Devuelve el tipo apuntado por el iterador. 
	*/
      virtual const Type * getType(typecit i) const {return (*i);};

      virtual vctype * getTypes(void) {return &types;};

      /**
	@brief Algo es de tipo object si no tiene tipos asociados.
	*/
      virtual bool isObjectType(void) const {return types.empty();};

      /**
	@brief Devuelve true si el t�rmino es del tipo indicado, false en otro caso
	*/
      virtual bool isType(const Type *t) const {for(typecit i=types.begin(); i!= types.end(); i++) if((*i)->isSubTypeOf(t)) return true; return false;};

      virtual bool hasType(const Type *t) const {for(typecit i=types.begin(); i!= types.end(); i++) if((*i)->equal(t)) return true; return false;};

      virtual bool hasTypeId(int id) const {for(typecit i=types.begin(); i!= types.end(); i++) if((*i)->getId()== id) return true; return false;};

      virtual void setPol(bool b) {};

      virtual bool getPol(void) const {return false;}

      virtual Literal * cloneL(void) const {return 0;};
	
  protected:

      virtual void clearTypes(void) {types.clear();};
      
      // Objeto de la librer�a python que tiene el c�digo compilado de la
      // funci�n.
      PyObject * code;

      // el tipo del elemento que estamos comparando
      // de momento tiene que ser compatible con lo
      // definido en pddl
      vctype types;
};
#endif
