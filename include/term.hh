#ifndef TERM_HH
#define TERM_HH

#include "constants.hh"
#include <string>
#include "type.hh"
#include <assert.h>
#include <sstream>
#include "xmlwriter.hh"

using namespace std;

class Unifier;

/**
 * Esta clase abstracta implementa la funcionalidad b�sica de los elementos
 * de tipo t�rmino. Constantes y Variables.
 */
class Term
{
    public:

	/**
	  @brief Constructor de t�rminos
	  */
	Term(int id) {this->id=id;};

	Term(int id, const vctype * v) :types(*v) {this->id=id;};

	Term(void) {id=-1;};

	Term(const Term * other) :types(other->types) {this->id=id;};

	virtual ~Term(void) {};

	/**
	 * @brief Comprueba si el t�rmino es una variable
	 */
	virtual bool isVariable(void) const {return false;};

	/**
	 * @brief Comprueba si el t�rmino es una constante
	 */
	virtual bool isConstant(void) const {return false;};

	/**
	  @brief Crea un duplicado del t�rmino
	  */
	virtual Term * clone(void) const=0;

	/**
	  @brief Especifica un tipo compatible con el t�rmino
	  */
	inline void addType(Type * t) {assert(t != 0); types.push_back(t);};

	/**
	 * Realiza la uni�n de los tipos del t�rmino con los de el vector pasado como argumento.
	 * @param vt El vector de tipos a a�adir.
	 * @see addTypes2
	 */
	inline void addTypes(const vector<Type *> * vt) {types.insert(types.end(),vt->begin(),vt->end());};

	/**
	 * Realiza la uni�n de los tipos del t�rmino con los de el vector pasado como argumento.
	 * El m�todo comprueba que al a�adir los nuevos tipos no se generen duplicados.
	 * @param vt El vector de tipos a a�adir.
	 * @see addTypes
	 */
	inline void addTypes2(vector<Type *> * vt)
	{
	    vector<Type *>::iterator i, e = vt->end();
	    for(i=vt->begin();i!=e;i++)
	    {
		if(!hasType(*i))
		    types.push_back(*i);
	    }
	};

	/**
	 * Realiza la especializaci�n de los tipos del t�rmino.
	 * Los tipos contenidos en la estructura vt, deben ser subtipos del
	 * tipo actual del t�rmino.
	 * @param vt El vector de tipos a los que queremos especializar.
	 * @see addTypes
	 * @return Se devolver� true en caso de �xito.
	 */
	bool specializeTypes(vector<Type *> * vt)
	{
	    if(types.empty()){
		addTypes(vt);
		return true;
	    }

	    vector<Type *> aux;
	    vector<Type *>::iterator i, e = vt->end(),j, k = types.end();
	    for(i=vt->begin();i!=e;i++)
		for(j=types.begin();j!=k;j++)
		    if((*i)->isSubTypeOf(*j)){
			aux.push_back(*i);
			break;
		    }

	    if(aux.empty())
		return false;
	    else {
		types.clear();
		addTypes(&aux);
	    }
	    return true;
	};

	/**
	 * Realiza la especializaci�n de los tipos del t�rmino.
	 * El tipo t debe ser subtipo de alguno de los tipos actuales del t�rmino.
	 * @param t El tipo al que queremos especializar.
	 * @see addTypes
	 * @return Se devolver� true en caso de �xito.
	 */
	bool specializeTypes(Type * t)
	{
	    if(types.empty()){
		addType(t);
		return true;
	    }

	    vector<Type *>::iterator j, k = types.end();
	    for(j=types.begin();j!=k;j++)
		if(t->isSubTypeOf(*j)){
		    types.clear();
		    addType(t);
		    return true;
		}

	    return false;
	};

	/**
	  @brief Devuelve el primer tipo del t�rmino.
	  @return el primer tipo o getEndType() si el t�rmino es de tipo object.
	  */
	inline typecit getBeginType(void) const {return types.begin();};

	/**
	  @brief Devuelve un iterador uno despu�s del �ltimo elemento.
	  */
	inline typecit getEndType(void) const {return types.end();};

	/**
	  @brief Devuelve el tipo apuntado por el iterador.
	  */
	inline const Type * getType(typecit i) const {return (*i);};

	inline vctype * getTypes(void) {return &types;};

	/**
	  @brief Algo es de tipo object si no tiene tipos asociados.
	  */
	inline bool isObjectType(void) const {return types.empty();};

	/**
	  @brief Devuelve true si el t�rmino es del tipo indicado, false en otro caso
	  */
	virtual bool isType(const Type *t) const {for(typecit i=types.begin(); i!= types.end(); i++) if((*i)->isSubTypeOf(t)) return true; return false;};

	virtual bool hasType(const Type *t) const {for(typecit i=types.begin(); i!= types.end(); i++) if((*i)->equal(t)) return true; return false;};

	virtual bool hasTypeId(int id) const {for(typecit i=types.begin(); i!= types.end(); i++) if((*i)->getId()== id) return true; return false;};

	/**
	  @brief Elimina los tipos asociados al t�rmino
	  */
	inline void clearTypes(void) {types.clear();};

	virtual const char * getName(void) const = 0;

	virtual void setName(const char * n) = 0;

	inline int getId(void) const {return id;};

	inline void setId(int i) {id = i;};

	virtual void print(ostream * out, int indent=0) const = 0;

	virtual const char * toString(void) const {static string s; ostringstream os; print(&os,0); s = os.str(); return s.c_str();};

	/**
	 * Devuelve en un documento xml la descripci�n de la tarea.
	 * @param os El flujo en el que escribir.
	 **/
	virtual void toxml(ostream * os) const{
	    XmlWriter * writer = new XmlWriter(os);
	    toxml(writer);
	    writer->flush();
	    delete writer;
	};

	/**
	 * Devuelve en un documento xml la descripci�n de la tarea.
	 * @param writer El objeto en donde escribir.
	 **/
	virtual void toxml(XmlWriter * writer) const = 0;

    protected:
	vctype types;
	int id;
};

struct PrintTerm
{
    PrintTerm(ostream * os) {this->os=os;};

    void operator()(const Term * t) const
    {
	t->print(os);
    };

    ostream * os;
};


#endif
