#ifndef LITERAL_HH
#define LITERAL_HH

#include "constants.hh"
#include <assert.h>
#include "unifierTable.hh"
#include "header.hh"
#include "xmlwriter.hh"
#include <tr1/unordered_map>

using namespace std;

class PrimitiveTask;

class Literal: public Header
{
    public:

	/**
	  @brief Constructor.
	  @param n nombre del literal.
	  @param p La polaridad, por defecto positiva.
	  */
	Literal(int id, int mid);

	Literal(int id, int mid, const KeyList * param);

	Literal(const Literal * other);

	virtual ~Literal(void) {};

	virtual Literal * cloneL(void) const { return NULL; };

	/**
	  @brief Imprime el contenido del objeto.
	  @param os Un flujo de salida.
	  */
	virtual void printL(ostream * os, int indent=0) const;

	/**
	 * Devuelve en un documento xml la descripci�n del literal.
	 * @param os El flujo en el que escribir.
	 **/
	virtual void toxmlL(ostream * os) const{
	    XmlWriter * writer = new XmlWriter(os);
	    toxmlL(writer);
	    writer->flush();
	    delete writer;
	};

	/**
	 * Devuelve en un documento xml la descripci�n del literal.
	 * @param writer el objeto donde escribir.
	 **/
	virtual void toxmlL(XmlWriter * writer) const;

	virtual pkey getTermIdL(const char * name) const;

	virtual bool hasTermL(int id) const;

	virtual void renameVarsL(Unifier * u, VUndo * undo);

	virtual bool isFunction(void) const {return false;};

	virtual bool isLEffect(void) const {return false;};

	inline void setProducer(const PrimitiveTask * p) {producer=p;};

	/**
	 * La tarea productora puede no ser v�lida en el caso de los fluents.
	 * En el caso de los fluents se almacena la �ltima de las tareas que modifico
	 * el fluent como productora
	 */
	inline const PrimitiveTask * getProducer(void) const {return producer;};

	virtual bool getPol(void) const { return false; };

	virtual void setPol(bool t) {};

	/**
	 * Comprueba si el literal se encuentra temporizado.
	 */

	virtual bool isTimeLine(void) const {return false;};

    protected:
	/** Tarea primitiva que a generado el efecto, si el puntero es null
	 * se considerará que el efecto estaba definido en el estado inicial
	 */
	const PrimitiveTask * producer;
};

typedef std::tr1::unordered_multimap<int, Literal *, std::tr1::hash<int>,equal_to<int> > ISTable;
typedef ISTable::const_iterator iscit;
typedef ISTable::iterator isit;

#endif
