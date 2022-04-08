#ifndef UNIFIER_H
#define UNIFIER_H

#include "constants.hh"
#include <iostream>
#include <assert.h>
#include <ext/slist>
#include "undoElement.hh"
#include "constants.hh"
#include "causal.hh"

using namespace std;

class Type;

typedef pair<int,vector<Type *> *> TypeSubstitution;
typedef vector<TypeSubstitution> vTSubstitutions;
typedef vector<TypeSubstitution>::iterator typesubite;

typedef __gnu_cxx::slist<pair<int,pair<int,float> > > vSubstitutions;
typedef vSubstitutions::const_iterator subscit;
typedef vSubstitutions::iterator subsit;

struct ApplyVarSubstitution{
    VUndo * undo;

    ApplyVarSubstitution(VUndo *u);
    void operator()(const pair<const int,pair<int,float> > & item);
};

class Unifier 
{
    public:
	/**
	 * Constructor por defecto.
	 */
	Unifier(void);

	/** 
	 * Constructor de copia.
	 * @param other El elemento del cual haremos una copia exacta.
	 */
	Unifier(const Unifier * other);

	/**
	 * El destructor de la clase
	 */
	~Unifier(void);

	/**
	 * Crea un clon de este unificador.
	 * @return el clon
	 */ 
	Unifier * clone(void) const;

	/**
	 * Devuelve la substituci�n de una variable.
	 * @param i el �ndice de la variable
	 * @param p una estructura donde almacenar el valor devuelto.
	 * @return true en caso de que exista sustituci�n, falso en otro caso.
	 */
	bool getSubstitution(int i, pkey * p) const;

	/**
	 * Devuelve la substituci�n de una variable.
	 * @param v el nombre de la variable 
	 * @param p una estructura donde almacenar el valor devuelto.
	 * @return true en caso de que exista sustituci�n, falso en otro caso.
	 */
	bool getSubstitution(const char * v, pkey * p) const;

	/**
	 * A�ade una sustituci�n a la tabla de sustituciones.
	 * ��Comprobar que previamente no exista!!,
	 * En otro caso petar�.
	 * @param i El �ndice de la variable.
	 * @param p La pkey por la que sustituimos.
	 */
	void addSubstitution(int i, pkey p);

	/**
	 * A�ade una sustituci�n a la tabla de sustituciones.
	 * Identica a la anterior, pero machaca si hay una
	 * sustituci�n previa.
	 * @param i El �ndice de la variable.
	 * @param p La pkey por la que sustituimos.
	 */
	void addFSubstitution(int i, pkey p);

	/**
	 * Aplica la substituciones de tipos en el caso de que sea
	 * necesario realizar alguna.
	 */
	void applyTypeSubstitutions(VUndo * undoApply) const;

	/**
	 * A�ade una type sustituci�n a la tabla de sustituciones de tipos.
	 * @param i El �ndice de la variable.
	 * @param p El vector de tipos que deseamos asignar.
	 */
	void addTSubstitution(int i, vector<Type *> * p);

	/**
	 * A�ade una type sustituci�n a la tabla de sustituciones de tipos.
	 * @param i El �ndice de la variable.
	 * @param p El vector de tipos que deseamos asignar.
	 */
	void addTSubstitution2(int i, const vector<Type *> * p);

	/**
	 * Esta funci�n a�ade a this los unificadores contenidos en u.
	 * Puede alterar el contenido de u.
	 * @param u El unificador del que queremos extraer la informaci�n.
	 */
	void merge(Unifier * u);

	/** 
	 * Reserva mermoria para la tabla de v�nculos causales. Si la
	 * memoria ya se encuentra reservada entonces no hace nada
	 */
	inline void createCLTable(void) {if(!cltable) cltable = new CLTable;};

	/**
	 * Registra un v�nculo causal en la tabla correspondiente.
	 * @param l El literal al cual queremos enlazar.
	 */
	inline void addCL(Causal * c) {cltable->push_back(c);};

	/**
	 * Registra una acci�n como consumidora de los v�nculos generados
	 * durante la unificaci�n.
	 * @param consumer la tarea, normalmente una primitiva aunque tambi�n
	 * podr�a ser una tarea compuesta.
	 */
	void setCLConsumer(const Task * consumer);

	/** 
	 * A�ade los elementos almacenados en la tabla de v�nculos de other
	 * a this.
	 * @param other unificador del que se desean extraer los v�nculos
	 * causales.
	 */
	void addCLTable(Unifier * other);

	/**
	 * Devuelve la tabla de v�nculos causales, asociados a esta
	 * unificaci�n.
	 */
	inline const CLTable * getCLTable(void) const {return cltable;};

	/**
	 * Devuelve la tabla de v�nculos causales, asociados a esta
	 * unificaci�n.
	 */
	inline CLTable * getModifiableCLTable(void) const {return cltable;};

	/**
	 * Devuelve true si hay alguna sustitucion de tipos pendiente
	 * en el unificador
	 * @return un booleano
	 */
	inline bool hasTypeSubstitutions(void) {return (typeSubstitutions != 0);};

	/** 
	 * Imprime el contenido del unificador.
	 * @param os el flujo donde escribiremos
	 * @param indent la indentaci�n a usar
	 */
	void print(ostream * os, int indent=0) const;

	/**
	 * Aplica todas las sustituciones pendientes.
	 * Los cambios se almacenan en el vector de undo.
	 * @param undoApply estructora para almacenar los cambios
	 * provocados por la unificaci�n
	 * @return true on success
	 */
	bool apply(VUndo * undoApply=0);

	/**
	 * Aplica las sustituciones que afectan a los tipos de la variable..
	 * Los cambios se almacenan en el vector de undo.
	 * @param undoApply estructora para almacenar los cambios
	 * provocados por la unificaci�n
	 * @return true on success
	 */
	bool applyTypeSubstitutions(VUndo * undoApply=0);

	/**
	 * Aplica todas las sustituciones variable t�rmino pendientes.
	 * Los cambios se almacenan en el vector de undo.
	 * @param undoApply estructora para almacenar los cambios
	 * provocados por la unificaci�n
	 * @return true on success
	 */
	bool applyVarSubstitutions(VUndo * undoApply=0);

	/**
	 * Devuelve el n�mero de sustituciones de variables
	 */
	inline int size(void) const {return substitutions.size();};

	/**
	 * Borra n elementos por el principio
	 */
	void erase(int n);

	/**
	 * Devuelve un iterador a la primera sustituci�n. 
	 */
	inline subsit begin(void) {return substitutions.begin();};

	/**
	 * Devuelve un iterador a la �ltima sustituci�n. 
	 */
	inline subsit end(void)  {return substitutions.end();};

	/**
	 * Comprueba si el unificador es igual a otro dado.
	 */
	bool equal(const Unifier * other) const;

    protected:
	/** Estructura para almacenar las substituciones de variable por t�rmino */
	vSubstitutions substitutions;
	/** Vector para almacenar en el caso de que se produzcan, sustituciones de tipos */
	vTSubstitutions * typeSubstitutions;
	/** Tabla de v�nculos causales consumidos durante la unificaci�n */
	CLTable * cltable;
};
#endif
