#ifndef TIME_INTERVAL_HH
#define TIME_INTERVAL_HH

#include <iostream>
#include <limits.h>

#define ATSTART 0
#define ATEND INT_MAX

class Unifier;
class Evaluable;
class State;

/**
 * Esta clase define un intervalo temporal.
 * Los intervalos se calculan usando dos expresiones temporales representadas por
 * fluents, el inicio y el fin.
 * Por coherencia se supone que inicio <= fin, aunque esto a veces no se puede comprobar
 * hasta el momento de planificaci�n.
 * Si no se establecieron tiempos de inicio y de fin, estos se evaluar�n como 0.
 * Si se establece el tiempo de inicio pero no el de fin, el intervalo se evaluar� como
 * un punto �nico, es decir los tiempos de inicio y de fin ser�n los mismos.
 * Si se establece el tiempo de fin pero no el de inicio, el intervalo se evaluar� como
 * entre start = 0 y el tiempo establecido en el fin
 */
class TimeInterval{
    public:
	/**
	 * Constructor por defecto.
	 */
	TimeInterval(void);

	/**
	 * Constructor de copia.
	 * /param other El objeto a copiar.
	 */
	TimeInterval(const TimeInterval * other);

	/**
	 * Destructor
	 */
	virtual ~TimeInterval(void);

	/**
	 * Establece el limite inferior del intervalo
	 * /param s El l�mite inferior
	 */
	void setStart(Evaluable * s);

	/**
	 * Establece el limite superior del intervalo
	 * /param e El l�mite inferior
	 */
	void setEnd(Evaluable * e);

	/**
	 * Establece el limite inferior del intervalo
	 * /param s El l�mite inferior
	 */
	void setStart(double s);

	/**
	 * Establece el limite superior del intervalo
	 * /param e El l�mite inferior
	 */
	void setEnd(double e);

	/**
	 * Imprime el intervalo con notaci�n pddl.
	 * /param os El flujo donde se escribir� la informaci�n.
	 * /indent El n�mero de espacios que se dejar�n de indentaci�n.
	 */
	virtual void print(ostream * os, int indent) const;

	/**
	 * Devuelve en un documento xml la descripci�n del objeto.
	 * @param os El flujo en el que escribir.
	 **/
	virtual void toxml(ostream * os) const{
	    XmlWriter * writer = new XmlWriter(os);
	    toxml(writer);
	    writer->flush();
	    delete writer;
	};

	/**
	 * Devuelve en un documento xml la descripci�n del objeto.
	 * @param writer el objeto donde escribir.
	 **/
	virtual void toxml(XmlWriter * writer) const;

	/**
	 * Hacer una copia exacta de este objeto
	 */
	TimeInterval * clone(void) const;

	/**
	 * Realiza un renombrado de las variables, al mismo tiempo va aplicando
	 * las sustituciones de variables que hay en u.
	 * @param u Estructura para ir almacenando las sustituciones que vamos realizando
	 * @param undo Estructura para ser capaces de deshacer los cambios que se van
	 * haciendo durante el renombrado. Puede ser null.
	 */
	virtual void renameVars(Unifier * u,VUndo * undo);

	/** Dado el nombre de una variable o una constante, devuelve su identificador asociado
	 * ��No busca t�rminos de tipo number!!. Si se devuelve pair<-1,...> no debe interpretarse
	 * como un n�mero sino como que el elemento no ha sido encontrado.
	 * @param name el nombre del t�rmino a buscar
	 * @return la pkey del t�rmino
	*/
	virtual pair<int,float> getTermId(const char * name) const;

	/** devuelve true si el objeto contiene el identificador pasado como argumento, en
	 * alguna de sus variables o constantes.
	 * @param id El identificador del t�rmino buscado.
	 * @return true si contenemos el identificador de t�rmino buscado.
	 */
	virtual bool hasTerm(int id) const;

	/**
	 * Evalua cuando debe estar disponble la precondici�n
	 */
	unsigned int evalStart(const Unifier * context, const State * sta);

	/**
	 * Evalua hasta cuando debe estar disponble la precondici�n
	 */
	unsigned int evalEnd(const Unifier * context, const State * sta);

    private:
	/** Tras la evaluaci�n devuelve el l�mite inferior del intervalo */
	Evaluable * start;
	/** Tras la evaluaci�n devuelve el l�mite superior del intervalo */
	Evaluable * end;
};

#endif
