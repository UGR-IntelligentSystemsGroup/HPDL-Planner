/**
 * Este módulo implementa una api para siadex, con funcionalidad que puede ser
 * utilizada, dentro del python incrustrado dentro de la definición de dominio
 * del planificador.
 */

#include "constants.hh"
#include "pythonWrapper.hh"
#include "papi.hh"
#include "literaleffect.hh"
#include "domain.hh"
#include "problem.hh"
#include "literalgoal.hh"
#include <python2.7/structmember.h>
#include "pyLiteral.cpp"
using namespace std;

#ifdef PYTHON_FOUND

/**
 * Devuelve información con la versión del planificador SIADEX.
 * No toma argumentos.
 */
static PyObject* siadex_ver(PyObject *self, PyObject *args) {
    if(!PyArg_ParseTuple(args, ":numargs"))
        return NULL;
    return Py_BuildValue("s", parser_api->ver());
};

/**
 * Construye un literal a partir de una lista de elementos tomados
 * de python. La lista comienza con una cadena que se corresponde
 * con el nombre del literal. El resto de elmentos son términos, que
 * pueden ser números o cadenas (que serán tratadas como símbolos). El
 * elemento None es tratado como una variable libre (sólo los goals pueden
 * tener variables libres).
 * El flag effect controla que se construya un LiteralEffect o un LiteralGoal
 * y es de uso interno.
 */
static Literal * buildLiteral(PyObject * args, bool effect){
    PyObject * o, * e;
    const char * name;
    Literal * lit=0;
    Meta * mt = 0;

    if(!PyArg_ParseTuple(args, "O",&o))
	return 0;

    if(PyTuple_Check(o)){
	int l = PyTuple_Size(o);
	if(l <= 0)
	    return 0;

	// Cogemos el item 0 que debe ser el nombre del literal
	e = PyTuple_GetItem(o,0);
	if(e == NULL)
	    return 0;

	if(!PyString_Check(e)){
	    Py_DECREF(e);
	    return 0;
	}

	name = PyString_AsString(e);

	// buscamos si el literal ya está definido en el diccionario de
	// nombres de literales (debería estarlo)
	ldictionaryit posit = SearchDictionary(&(domain->ldictionary),name);
	if(posit != (domain->ldictionary).end()) {
	    if(effect)
		lit = new LiteralEffect(posit->second,domain->metainfo.size());
	    else
		lit = new LiteralGoal(posit->second,domain->metainfo.size());
	    mt = new Meta(name,0,-1);
	    domain->metainfo.push_back(mt);
	}
	else
	{
	    if(effect)
		lit = new LiteralEffect(idCounter++,domain->metainfo.size());
	    else
		lit = new LiteralGoal(posit->second,domain->metainfo.size());
	    mt = new Meta(name,0,-1);
	    domain->metainfo.push_back(mt);
	    (domain->ldictionary).insert(make_pair(lit->getName(), lit->getId()));
	}
	Py_DECREF(e);

	// Recorremos el resto de posibles argumentos construyendo
	// las diferentes constantes.
	for(int i=1;i<l;i++){
	    e = PyTuple_GetItem(o,i);
	    if(e == NULL){
		return 0;
	    }

	    // chequear si es un flotante
	    if(PyFloat_Check(e)){
		lit->addParameter(make_pair(-1,(float)PyFloat_AsDouble(e)));
	    }
	    // chequear si es un entero
	    else if(PyInt_Check(e)){
		lit->addParameter(make_pair(-1,(float)PyInt_AsLong(e)));
	    }

	    // por último chequear si es una cadena, en cuyo
	    // caso habrá que extraer el símbolo correspondiente
	    else if(PyString_Check(e)){
		name = PyString_AsString(e);

		// la constante debería haberse definido con anterioridad, en otro caso
		// se trata de un error
		ldictionaryit posit = (domain->cdictionary).find(name);
		if(posit != (domain->ldictionary).end()) {
		    // devolver el pkey de la constante
		    lit->addParameter(make_pair((*posit).second, 0));
		}
		else {
                    *errflow << "Python warning: Undefined constant `" << name;
                    *errflow << "'. While creating literal: " << lit->getName() << endl;
                    ConstantSymbol * n = new ConstantSymbol(name,-1);
                    n->setLineNumber(0);
                    n->setFileId(-1);
                    lit->addParameter(termtable->addConstant(n));
		}
	    }
	    else if(e == Py_None && !effect){
		int n = domain->metainfo.size();
		ostringstream s;
		s << "?v" << n;
		VariableSymbol * v = new VariableSymbol(-1,n);
		Meta * mt = new Meta(s.str().c_str(),0,-1);
		pkey id = termtable->addVariable(v);
		domain->metainfo.push_back(mt);
		lit->addParameter(id);
	    }

	    else {
		//*errflow << "Python fatal error: Invalid term. ";
		//*errflow << "While creating literal: " << lit->getName() << endl;
		Py_DECREF(e);
		return 0;
	    }
	    Py_DECREF(e);
	}
    }
    else
	return 0;

    return lit;
};

/**
 * Construye un literal, se pasa como argumentos una tupla
 * en la que el primer elemento es el identificador del literal
 * y el resto son constantes. El literal es añadido al estado
 * como si estuviese definido en el estado inicial.
 * Devuelve 0 si hay éxito. Otro número en otro caso.
 */
static PyObject * add_literal(PyObject * self, PyObject * args){
    LiteralEffect * lit = (LiteralEffect *) buildLiteral(args,true);
    if(lit == 0)
	return Py_BuildValue("i", -1);
    current_plan->addToState(lit);
    return Py_BuildValue("i", 0);
}

/**
 * Recibe como entrada una lista de elementos donde el primero es el
 * nombre del literal y el resto son los argumentos. Si alguno de los
 * argumentos es None, se considera como una variable libre con la
 * que se puede unificar.
 * Devuelve una lista de listas, con el resultado de la unificación,
 * o lista vacía si no hubo unificación o hubo algún problema.
 */
static PyObject * get_lunifiers(PyObject * self, PyObject * args){

    pair<unsigned int, unsigned int> p(0,0);
    UnifierTable * ut;
    LiteralGoal * lit = (LiteralGoal *) buildLiteral(args,false);
    if(lit == 0)
	return Py_BuildValue("()");
    ut = lit->getUnifiers(current_plan->getState(),0,true,&p);
    ut->print(&cerr);
    delete ut;
    return Py_BuildValue("()");
}

static PyMethodDef SiadexMethods[] = {
    {"ver", siadex_ver, METH_VARARGS,
     "Return a string with version information about siadex."},
    {"add_literal", add_literal, METH_VARARGS,
     "Add a tuple containing a literal to the current state. Returns 0 on success."},
    {"get_lunifiers", get_lunifiers, METH_VARARGS,
     "Given an unifier in the form (literal [, arg1, ... , argN]) where argx is a number or string or None. Returns a list of literals that unify in te current state."},
    {NULL, NULL, 0, NULL}
};

#endif
