// --------------------------------------------------------------------------------------------
// Definición del tipo SdxTerm que equivale al Term para python.
// --------------------------------------------------------------------------------------------

#include "constants.hh"
#include "pythonWrapper.hh"
#include "papi.hh"
#include "domain.hh"
#include "type.hh"
#include <python2.7/structmember.h>
#include "constantsymbol.hh"
#include "variablesymbol.hh"
using namespace std;

#ifdef PYTHON_FOUND

/**
 * Estructura que define el tipo
 */
typedef struct {
    PyObject_HEAD
    pkey t;
} SdxTerm;

/**
 * Liberar la memoria ocupada por el tipo
 */
static void SdxTerm_dealloc(SdxTerm * self)
{
    self->ob_type->tp_free((PyObject*)self);
}


/**
 * Inicialización del tipo.
 */
static PyObject * SdxTerm_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    SdxTerm *self;
    self = (SdxTerm *)type->tp_alloc(type, 0);
    self->t = make_pair(-1,0);
    return (PyObject *)self;
}

/**
 * Inicialización de la estructura
 */
static int SdxTerm_init(SdxTerm *self, PyObject *args, PyObject *kwds)
{
    PyObject *n;
    const char * name;

    // Capturamos la inicialización dada por el usuario
    if (! PyArg_ParseTuple(args, "O", &n)){
        return -1;
    }

    if(PyString_Check(n)){
	// Coger el nombre
	name = PyString_AsString(n);
	// Averiguar si es una variable o un símbolo.
	if(name[0] == '?'){
	    // Es una variable
	    int n = parser_api->domain->metainfo.size();
	    VariableSymbol * v = new VariableSymbol(-1,n);
	    Meta * mt = new Meta(name,0,-1);
	    self->t = parser_api->termtable->addVariable(v);
	    parser_api->domain->metainfo.push_back(mt);
	}
	else{
	    // Es un símbolo
	    // la constante debería haberse definido con anterioridad, en otro caso
	    // se trata de un error
	    ldictionaryit posit = (parser_api->domain->cdictionary).find(name);
	    if(posit != (parser_api->domain->ldictionary).end()) {
		// devolver el pkey de la constante
		self->t = make_pair((*posit).second, 0);
	    }
	    else {
                *errflow << "Python warning: Undefined constant `" << name;
                *errflow << "'. During term initialization. " << endl;
                ConstantSymbol * n = new ConstantSymbol(name,-1);
                n->setLineNumber(0);
                n->setFileId(-1);
                self->t = parser_api->termtable->addConstant(n);
	    }
	}
    }
    // chequear si es un flotante
    else if(PyFloat_Check(n)){
	self->t = make_pair(-1,(float)PyFloat_AsDouble(n));
    }
    // chequear si es un entero
    else if(PyInt_Check(n)){
	self->t = make_pair(-1,(float)PyInt_AsLong(n));
    }
    else{
	return -1;
    }

    return 0;
}

/**
 * Inicialización de la estructura
 */
static PyObject * SdxTerm_set_type(SdxTerm *self, PyObject *args, PyObject *kwds)
{
    PyObject *n;
    const char * name;

    // Mirar si soy una variable, si no lo soy no se me puede añadir un tipo
    if(!parser_api->termtable->isVariable(self->t)){
        return PyInt_FromLong(-1);
    };

    // Capturamos la inicialización dada por el usuario
    if (! PyArg_ParseTuple(args, "O", &n)){
        return PyInt_FromLong(-1);
    }

    if(PyString_Check(n)){
	// Coger el nombre
	name = PyString_AsString(n);
	// Buscar ahora si hay un tipo con ese nombre
	Type * t = parser_api->domain->getModificableType(name);
	if(!t)
	    return PyInt_FromLong(-1);
	VariableSymbol * v = parser_api->termtable->getVariable(self->t);
	v->addType(t);
    }
    else{
        return PyInt_FromLong(-1);
    }
    return PyInt_FromLong(0);
};

/**
 * wrapper función de impresión.
 */
static PyObject * SdxTerm_str(SdxTerm * self){
    PyObject * result;
    ostringstream s;

    PrintKey pk = PrintKey(&s);
    pk(self->t);
    result = PyString_FromString(s.str().c_str());
    return result;
};

static int SdxTerm_print(SdxTerm * self, FILE * fp, int flags){
    ostringstream s;

    PrintKey pk = PrintKey(&s);
    pk(self->t);
    fprintf(fp,"%s",s.str().c_str());
    return 0;
};


static PyMemberDef SdxTerm_members[] = {
    {NULL}  /* Sentinel */
};


static PyMethodDef SdxTerm_methods[] = {
    {"set_type", (PyCFunction)SdxTerm_set_type, METH_VARARGS,
	"Stablish a new type for a variable.",
    },
    {"str", (PyCFunction)SdxTerm_str, METH_VARARGS,
	"Returns a textual representation of the object.",
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject SdxTermType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "siadex.SdxTerm",             /*tp_name*/
    sizeof(SdxTerm),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor) SdxTerm_dealloc, /*tp_dealloc*/
    (printfunc) SdxTerm_print,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    (reprfunc) SdxTerm_str,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Siadex Type",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    SdxTerm_methods,             /* tp_methods */
    SdxTerm_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)SdxTerm_init,      /* tp_init */
    0,                         /* tp_alloc */
    SdxTerm_new,                 /* tp_new */
};

/**
 * Inicialización del módulo
 */
static void SdxTerm_initModule(PyObject * m){
    // Definir los nuevos tipos para siadex
    PyTypeObject * sptr = &SdxTermType;
    if (PyType_Ready(sptr) < 0)
	return;

    Py_INCREF(sptr);
    PyModule_AddObject(m, "SdxTerm", (PyObject *) sptr);
}

#endif
