#ifndef PYTHONWRAPPER_HH
#define PYTHONWRAPPER_HH
#ifdef PYTHON_FOUND

using namespace std;
#include "constants.hh"
#include <python2.7/compile.h>
#include <python2.7/eval.h>
#include <sstream>
#include <vector>

class Header;
class Unifier;
class UnifierTable;

class PythonWrapper
{
    public:
	PythonWrapper(void);

	~PythonWrapper(void);

	/* return null on error */
	PyObject * parse(const Header * func, const char * code);

	bool exec(PyObject * pCode, const Header * func, const Unifier * context, UnifierTable * u, double * res) const;

	/**
	 * Carga los m�dulos necesarios para la ejecuci�n de ciertos
	 * algoritmos python
	 */
	void loadModule(const char * module);

	/**
	 * Carga el siguiente fichero python en el interprete y lo ejecuta.
	 */
	void loadFile(const char * name);

	/**
	 * Compila el c�digo python pasado como argumento
	 * y lo encola para su ejecuci�n en el interprete.
	 * Durante el proceso de parseo hay elementos de SIADEX
	 * que solo est�n disponibles en tiempo de ejecuci�n.
	 * Por ello se implementa una cola donde se va almacenando
	 * el c�digo, hasta el inicio de la fase de planificaci�n
	 * donde se ejecuta.
	 */
	int loadStr(const char * code);

	/**
	 * Ejecuta el c�digo que est� pendiente de ejecuci�n
	 * en la cola.
	 */
	int execQueue(void);

    private:
	// El diccionario donde python guarda las referencias a los objetos
	PyObject * pDict;
	// Vector que mantiene las referencias a los m�dulos cargados
	vector<PyObject *> modules;
	// Pila de c�digo pendiente para su ejecuci�n.
	vector<string> queue;

	/**
	 * Esta funci�n es de uso interno.
	 * Dado un objeto python lo a�ade a la tabla de unificadores.
	 * /param pObj el objeto python
	 * /param ut la tabla de unificadores
	 * /param El identificador de la variable para hacer la substituci�n
	 * /param El tama�o de la tabla de unificadores antes de modificarla.
	 * /param Flag para mejorar la eficiencia del m�todo, indica si el objeto
	 * python que se va a a�adir es o no el �ltimo
	 */
	bool addUnifier(PyObject * pObj, UnifierTable * ut, int v, int size, bool last) const;

};

#else
#define PyObject int
class Header;
class Unifier;
class UnifierTable;

class PythonWrapper
{
    public:
	PyObject * parse(const Header * func, const char * code)
	{
	    return 0;
	};

    /**
     * Carga los m�dulos necesarios para la ejecuci�n de ciertos
     * algoritmos python
     */
    void loadModule(const char * module) {};
    void loadFile(const char * name) {};
    void loadStr(const char * code) {};
    bool exec(PyObject * pCode, const Header * func, const Unifier * context, UnifierTable * u, double * res) const{return true;};
    int execQueue(void);
};
#endif
#endif
