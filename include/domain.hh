#ifndef DOMAIN_H
#define DOMAIN_H
#include "method.hh"
#include <vector>
#include <tr1/unordered_map>
#include <assert.h>
#include "type.hh"
#include "literal.hh"
#include "axiom.hh"
#include "xmlwriter.hh"
#include "meta.hh"

typedef std::tr1::unordered_multimap<int, Task *> TaskTable;
typedef std::tr1::unordered_multimap<int, Literal *> LiteralTable;
typedef std::tr1::unordered_multimap<int, Axiom *> AxiomTable;
typedef std::pair<TaskTable::const_iterator, TaskTable::const_iterator> TaskTableRange ;
typedef std::pair<LiteralTable::const_iterator, LiteralTable::const_iterator> LiteralTableRange ;
typedef std::pair<AxiomTable::const_iterator, AxiomTable::const_iterator> AxiomTableRange ;
typedef std::vector<Type *> TypeTable;

typedef TypeTable::const_iterator typetablecit;
typedef TypeTable::iterator typetableit;
typedef TaskTable::const_iterator tasktablecit;
typedef TaskTable::iterator tasktableit;
typedef LiteralTable::const_iterator literaltablecit;
typedef LiteralTable::iterator literaltableit;
typedef AxiomTable::const_iterator axiomtablecit;
typedef AxiomTable::iterator axiomtableit;
typedef constantTable::const_iterator constablecit;

typedef std::tr1::unordered_map<std::string, int , icaseDJBHash, ieqstr> LDictionary;
typedef LDictionary::const_iterator ldictionarycit;
typedef LDictionary::iterator ldictionaryit;

ldictionaryit SearchDictionary(LDictionary * dict, const char * key);

typedef std::vector<int> TypeRelations;

using namespace std;

class Domain
{
    public:

	Domain(void);

	/**
	  @brief Constructor
	  @param name el nombre del dominio
	  */
	Domain(const char * n);

	virtual void print(ostream * out, int nindent=0) const;

	virtual void toxml(ostream * out) const;

	virtual void toxml(XmlWriter * writer) const;

	/**
	 * Escribe la jerarqu�a de tipos en formato xml.
	 **/
	virtual void typeHierarchyToxml(XmlWriter * writer) const;

	/**
	  @brief Destructor
	  */
	virtual ~Domain(void);

	/**
	 * @brief Se a�ade una tarea a la tabla de tareas.
	 * @param task la tarea a a�adir, �ojo! se realiza una copia del puntero,
	 * el destructor de esta clase se encarga de liberar la memoria ocupada.
	 * @author oscar
	 */
	void addTask(Task * task);
	/**
	 * @brief Cuenta el n�mero de tareas con el nombre dado.
	 * @param name, el identificador de las tareas a contar.
	 * @author oscar
	 */
	int countTaskElements(int id) const;
	/**
	 * @brief Devuelve los iteradores necesarios para recorrer las tareas con
	 * un determinado nombre (clave).
	 * @description Puede haber varias tareas con el mismo nombre pero distinto
	 * n�mero de argumentos. Esta funci�n sirve para recorrerlas.
	 * Ejemplo: TaskTableRange r = getTaskRange("pepito");
	 * for(taskHash::const_iterator it = r.first; it != r.second; ++i) {const Task * t = (*it).second;}
	 * @return el rango.
	 * @author oscar
	 */
	TaskTableRange getTaskRange(int id) const {return tasktable.equal_range(id);};
	/**
	 * @brief Devuelve un iterador con el contenido de la primera tarea.
	 * @param range El rango devuelto por la funci�n getTaskRange
	 * @see getTask
	 * @see getTaskRange
	 * @return un iterador
	 * @author oscar
	 */
	inline tasktablecit getBeginRangeTask(TaskTableRange range) const {return range.first;};

	inline tasktablecit getEndRangeTask(TaskTableRange range) const {return range.second;};

	/**
	 * @brief Devuelve un iterador con el contenido de la i-esima tarea.
	 * @param range El rango devuelto por la funci�n getTaskRange
	 * @param it, el iterador devuelto por una llamada anterior a getFirstTask o
	 * a getNextElement
	 * @see getTask
	 * @see getFirstTask
	 * @see getTaskRange
	 * @return un iterador
	 * @author oscar
	 */
	tasktablecit getNextTask(TaskTableRange range,tasktablecit it) const;
	/**
	 * @brief Borra el elemento apuntado por el iterador.
	 * @param it El iterador
	 * @author oscar
	 * @return true en caso de fallo false en otro caso
	 */
	bool deleteTask(tasktablecit it);

	/**
	  @brief Devuelve un iterador al primer elemento de la tabla de tareas
	  */
	inline tasktablecit getBeginTask(void) const {return tasktable.begin();};

	/**
	  @brief Devuelve un iterador uno despu�s del �ltimo elemento de la
	  tabla de tipos
	  */
	inline tasktablecit getEndTask(void) const {return tasktable.end();};

	/**
	  @brief Devuelve el tipo apuntado por el iterador
	  */
	inline const Task * getTask(tasktablecit i) const {return (*i).second;};

	/**
	  @brief Devuelve la tarea con metaId proporcionado
	  */
	const Task * getTaskMetaID(int id, int MetaID) const;

	// Operaciones sobre la tabla de literales.
	void addLiteral(Literal * l);

	int countLiteralElements(int id) const;

	LiteralTableRange getLiteralRange(int id) const {return literaltable.equal_range(id);};

	inline literaltablecit getBeginRangeLiteral(LiteralTableRange range) const {return range.first;};

	inline literaltablecit getEndRangeLiteral(LiteralTableRange range) const {return range.second;};

	literaltablecit getNextLiteral(LiteralTableRange range,literaltablecit it) const;

	bool deleteLiteral(literaltablecit it);

	inline literaltablecit getBeginLiteral(void) const {return literaltable.begin();};

	inline literaltablecit getEndLiteral(void) const {return literaltable.end();};

	inline const Literal * getLiteral(literaltablecit i) const {return (*i).second;};

	// Operaciones sobre la tabla de axiomas.
	void addAxiom(Axiom * a);

	int countAxiomElements(int id) const;

	AxiomTableRange getAxiomRange(int id) const {return axiomtable.equal_range(id);};

	inline axiomtablecit getBeginRangeAxiom(AxiomTableRange range) const {return range.first;};

	inline axiomtablecit getEndRangeAxiom(AxiomTableRange range) const {return range.second;};

	axiomtablecit getNextAxiom(AxiomTableRange range,axiomtablecit it) const;

	bool deleteAxiom(axiomtablecit it);

	inline axiomtablecit getBeginAxiom(void) const {return axiomtable.begin();};

	inline axiomtablecit getEndAxiom(void) const {return axiomtable.end();};

	inline const Axiom * getAxiom(axiomtablecit i) const {return (*i).second;};


	/**
	  @brief A�ade un nuevo tipo al arbol de tipos. Si ya existe devuelve
	  un puntero al tipo existente.
	  @param t el nombre del tipo
	  @return el tipo a�adido o encontrado
	  */
	const Type * addType(const char * t);

	const Type * addType(Type * type);

	/**
	  @brief Busca un tipo a partir de su nombre.
	  @param t el nombre del tipo
	  @return El tipo buscado. Null en el caso de que no se encuentre.
	  */
	const Type * getType(const char * name);

	/**
	  @brief Busca un tipo a partir de su nombre.
	  @param t el nombre del tipo
	  @return El tipo buscado. Null en el caso de que no se encuentre.
	  */
	Type * getModificableType(const char * name);

	inline Type * getModificableType(int id) {return typetable[id];};

	/**
	  @brief Comprueba si el tipo se encuentra definido
	  en la tabla de tipos
	  @param t el nombre del tipo
	  @return true en el caso de encontrarlo false en otro caso
	  */
	bool isDefined(const char * t);

	/**
	  @brief supertype se convierte en supertipo de base
	  @param base el tipo base
	  @param supertype el supertipo que deseamos asignar
	  */
	void setSuperType(const Type * base, Type * supertype);

	/**
	  @brief Devuelve un iterador al primer elemento de la tabla de tipos
	  */
	inline typetablecit getBeginType(void) const {return typetable.begin();};

	/**
	  @brief Devuelve un iterador uno despu�s del �ltimo elemento de la
	  tabla de tipos
	  */
	inline typetablecit getEndType(void) const {return typetable.end();};

	/**
	  @brief Devuelve el tipo apuntado por el iterador
	  */
	inline const Type * getType(typetablecit i) {return (*i);};

	inline const Type * getType(int id) {return typetable[id];};

	/**
	  @brief Devuelve una lista de tareas que unifiquen con el TaskHeader pasado como argumento.
	  @description Se busca una Task cuya cabecera o purpose unifique con TaskHeader.
	  Se clona la misma y se instancian los argumentos. Se a�ade a la lista devuleta.
	  @param th El task header
	  @return un puntero a un vector de tarea que deber� ser liberado por la funci�n llamadora.
	  (Tanto el vector como los punteros contenidos).
	  El vector estar� vac�o en el caso de que no se encuentre ninguna tarea que unifique.
	  */
	vector<Task *> * getUnifyTask(TaskHeader * th, vector<VUndo *> * undo) const;

	/**
	  @brief Add a PDDL 2.2 requirement to the requires list.
	  @param requirement the requirement to add (i.e. "strips")
	  @return */
	void addRequirement(const char * requirement);

	/**
	  @brief Look for the requirements of the domain.
	  @param requirement the requirement to search.
	  @return true if this is a requirement on the current domain, false otherwise*/
	bool hasRequirement(const char * requirement);

	/** flag vars for avoid to show repeat error msg */
	bool errtyping;
	bool errfluents;
	bool errdisjunctive;
	bool errexistential;
	bool erruniversal;
	bool errnegative;
	bool errconditionals;
	bool errmetatags ;
	bool errhtn;
	bool errder;
	bool errdurative;

	// flag para evitar definir el dominio dos veces
	bool loaded;

	// diccionarios para transformar nombres
	// en identificadores
	/** Diccionario para los identificadores de los literales */
	LDictionary ldictionary;
	/** Diccionario para los nombres de tareas */
	LDictionary tdictionary;
	/** Diccionario para los identificadores de las constantes */
	LDictionary cdictionary;
	// metainformacion de los elementos (variables, acciones, literales...
	// pueden aparecer tambi�n en ldictionary).
	MetaInfo metainfo;

	inline const char * getMetaName(int mid) {assert(mid >= 0 && mid < (int) metainfo.size()); return metainfo[mid]->name.c_str();};

	inline void setMetaName(int mid, const char * n) {assert(mid >= 0 && mid < (int) metainfo.size()); assert(n); metainfo[mid]->name = n;};

	inline const char * getName(void) const {return name.c_str();};

	inline void setDomainName(const char * n) {name = n;};

	/**
	 * Construye la tabla de relaciones de herencia.
	 * Para evitar recorrer continuamente el �rbol de tipos durante la intersecci�n de tipos
	 * en las unificaciones, lo cual puede
	 * ser costoso si el �rbol de tipos es profundo, o si una variable tiene m�ltiples tipos
	 * creamos la tabla de relaciones de herencia typerelations.
	 */
	void buildTypeRelations(void);

	/**
	 * Devuelve true si el tipo de �ndice i es subtipo del tipo de indice j.
	 * @param i el primer tipo
	 * @param j el segundo tipo
	 * @return un valor booleano
	 */
	inline bool isSubType(int i, int j) {return typerelations[i*ntypes+j];};

	/**
	 * Imprime la tabla de relaciones.
	 * Puede ser �til para depurar.
	 * @param os flujo por el que escribir.
	 */
	void printTypeRelations(ostream * os) const;

	/**
	 * Devuelve true, si el dominio es temporizado
	 * false en otro caso.
	 */
	inline bool isTimed(void) const {return istimed;};

	// Distintas banderas de configuraci�n.

	/** Unidad de tiempo seleccionada */
	TimeUnit FLAG_TIME_UNIT;

	/** Cuando comienza el plan */
	time_t FLAG_TIME_START;

	/** Que formato de tiempo se usa para la impresi�n */
	string TFORMAT;

	/** Control del time-horizon */
	int MAX_THORIZON;
	int REL_THORIZON;

    protected:

	TaskTable tasktable; /**< Tabla hash para mantener la definici�n de todas las tareas
			       primitivas o abstractas */
	TypeTable typetable; /** Tabla hash para mantener el �rbol de tipos */

	/** Esta matriz almacena las relaciones de herencia entre dos clases.
	 * Si i y j son dos identificadores de tipo i es subtipo de j si
	 * typerelations[i][j] es true
	 */
	TypeRelations typerelations;

	int ntypes;

	LiteralTable literaltable; /**< Tabla hash para mantener la definici�n de todas los
				     predicados usados en el dominio */
	AxiomTable axiomtable;

	string name;

	// los requerimientos sobre el planificador impuestos por el dominio
	vector<string> requirements;

	/** Determina si la resoluci�n del problema va a necesitar de las
	 * estructuras de datos temporales */
	bool istimed;

	/**
	 * Funci�n para hacer la inicializaci�n de la estructura de tipos
	 */
	void initTypes(void);
};


#endif
