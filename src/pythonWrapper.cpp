#include "constants.hh"
#include "pythonWrapper.hh"
bool FLAG_TRUSTED=true;
#ifdef PYTHON_FOUND

#include "termTable.hh"
#include "header.hh"
#include "unifier.hh"
#include "unifierTable.hh"
#include "domain.hh"
#include "problem.hh"
#include "papi.hh"
#include <pthread.h>
#include "pyAPI.cpp"

using namespace std;


PythonWrapper::PythonWrapper(void)
{
    Py_Initialize();
    Py_InitModule("siadex",SiadexMethods);

    // usamos como namespace el standard
    modules.push_back(PyImport_ImportModule("__main__"));
    modules.push_back(PyImport_ImportModule("siadex"));
    if(!FLAG_TRUSTED)
        // activamos el modo de ejecuci�n restringido
        modules.push_back(PyImport_ImportModule("rexec"));

    if(modules.front() == 0){
        *errflow << "Fatal Error: Python is unable to load module '__main__'" << endl;
        exit(EXIT_FAILURE);
    }

    if(modules[1] == 0){
        *errflow << "Fatal Error: Python is unable to load module 'siadex'" << endl;
        exit(EXIT_FAILURE);
    }

    SdxTerm_initModule(modules[1]);
    SdxUTable_initModule(modules[1]);
    SdxLiteral_initModule(modules[1]);

    // obtener el diccionario de nombres asociado al namespace
    pDict = PyModule_GetDict(modules.front());
    if(pDict == 0){
        *errflow << "Fatal Error: Python is unable to find dictionary." << endl;
        exit(EXIT_FAILURE);
    }
};

PythonWrapper::~PythonWrapper(void)
{
    vector<PyObject *>::iterator i, e;
    e = modules.end();
    for(i=modules.begin();i!=e;i++)
        Py_DECREF(*i);
    Py_DECREF(pDict);
    Py_Finalize();
};

void PythonWrapper::loadModule(const char * module)
{
    modules.push_back(PyImport_ImportModule((char *)module));
    if(modules.front() == 0){
        *errflow << "Fatal Error: Python is unable to load module '" << module << "'" << endl;
        exit(EXIT_FAILURE);
    }
    PyObject * dict = PyModule_GetDict(modules.back());
    if(!dict || PyDict_Merge(pDict,dict,0)!=0){
        *errflow << "Fatal Error: Python is unable to load dictionary of module '" << module << "'" << endl;
        exit(EXIT_FAILURE);
    }
    Py_DECREF(dict);
};

PyObject * PythonWrapper::parse(const Header * func, const char * code)
{
    string aux;
    ostringstream mycode;
    const char * ret;
    bool inCad1 = false, inCad2 = false;

    mycode << "def " << func->getName() << "(";
    keylistcit i,b = func->parametersBegin(), e = func->parametersEnd();
    for(i = b; i != e; i++){
        ret = termtable->getVariable((*i))->getName();
        if(i!=b)
            mycode << ", ";
        mycode << &(ret[1]);
    }
    mycode << "):" << endl;
    aux = code;
    string tmp = "";
    // quitar los '?' de las variables
    int j, k = aux.size();
    for(j=0; j != k; j++){
        if(aux[j] == '\n'){
            mycode << "\t" << tmp << endl;
            tmp ="";
        }
        else if(aux [j] == '\"'){
            if(inCad1)
                inCad1 = false;
            else
                inCad1 = true;
            tmp += aux[j];
        }
        else if(aux [j] == '\''){
            if(inCad2)
                inCad2 = false;
            else
                inCad2 = true;
            tmp += aux[j];
        }
        else if(aux[j] != '?' ) {
            tmp += aux[j];
        }
        else if(aux[j] == '?' && (inCad1 || inCad2)){
            tmp += aux[j];
        }
    }

    mycode << endl << endl <<  "RETURN = " << func->getName() << "(";
    int c, max = func->sizep();
    for(c=0; c < max; c++){
        if(c!=0)
            mycode << ", ";
        mycode << "v" << c;
    }
    mycode << ")" << endl;

    PyObject * obj =Py_CompileString(mycode.str().c_str(),func->getName(),Py_file_input);
    if(!obj){
        *errflow << "Error: PythonWrapper::parse(): Unable to parse python : " << endl << mycode.str() << endl;
    }

    return obj;
};

bool PythonWrapper::exec(PyObject * pCode, const Header * func, const Unifier * context, UnifierTable * u, double * res) const
{
    *res = 0;
    char var[5];

    PyObject * pResult;

    keylistcit i, e = func->parametersEnd();
    int c=0;
    pkey aux;

    if(current_plan->FLAG_VERBOSE){
        *errflow << "(***) Python call: ";
        *errflow << "(" << func->getName();
        for(i=func->parametersBegin();i!=e;i++){
            aux = (*i);
            if(context)
                context->getSubstitution((*i).first,&aux);
            *errflow << " ";
            termtable->print(aux,errflow);
        }
        *errflow << ")" << endl;
    }

    // poner los argumentos en el diccionario de python
    for(i=func->parametersBegin(); i!= e; i++,c++){
        snprintf(var,4,"v%d",c);
        aux = (*i);
        if(context)
            context->getSubstitution((*i).first,&aux);
        if(termtable->isNumber(aux)){
            PyObject * n = PyFloat_FromDouble(aux.second);
            PyDict_SetItemString(pDict,var,n);
            Py_DECREF(n);
        }
        else if(termtable->isConstant(aux)){
            PyObject * s = PyString_FromString(termtable->getConstant(aux)->getName());
            PyDict_SetItemString(pDict,var,s);
            Py_DECREF(s);
        }
        else if(u != 0){
            PyObject * l = PyList_New(0);
            PyDict_SetItemString(pDict,var,l);
            Py_DECREF(l);
        }
        else {
            *errflow << "Error: PythonWrapper::exec(): Passing an unbound variable to a python function: " << aux.first << endl << endl;
            exit(EXIT_FAILURE);
        }
    }

    PyEval_EvalCode((PyCodeObject *)pCode, pDict, pDict);

    pResult = PyDict_GetItemString(pDict,"RETURN");
    if(!pResult){
        if(current_plan->FLAG_VERBOSE)
            *errflow << "(***) Python call fail!" << endl;
        return false;
    }
    else{
        PyObject * type = PyObject_Type(pResult);
        if(current_plan->FLAG_VERBOSE){
            *errflow << "(***) Python call result: ";
            PyObject_Print(pResult,stderr,Py_PRINT_RAW);
            *errflow << " ";
            PyObject_Print(type,stderr,Py_PRINT_RAW);
            *errflow << endl;
        }
        if(PyFloat_Check(pResult)){
            *res = PyFloat_AsDouble(pResult);
        }
        else if(PyInt_Check(pResult)){
            *res = (double) PyInt_AsLong(pResult);
        }
        else if(PyLong_Check(pResult)){
            *res = (double) PyLong_AsLong(pResult);
        }
        else if(PyBool_Check(pResult)){
            *res = (double) PyInt_AsLong(pResult);
        }
        else{
            *errflow << "Warning: PythonWrapper::exec(): Unexpected return type" << endl;
        }
        Py_DECREF(type);
        if(u){
            // comprobar si entraron variables libres
            c=0;
            for(i=func->parametersBegin(); i!= e; i++,c++){
                aux = (*i);
                snprintf(var,4,"v%d",c);
                if(context)
                    context->getSubstitution((*i).first,&aux);
                if(termtable->isVariable(aux)){
                    // es una variable libre.
                    // Buscamos si se le a asignado alg�n valor
                    PyObject * pList = PyDict_GetItemString(pDict,var);
                    int us = u->countUnifiers();
                    if(pList){
                        if(PyList_Check(pList)){
                            int s = PyList_Size(pList);
                            s--;
                            for(int j=0; j<=s; j++){
                                PyObject * item = PyList_GetItem(pList,j);
                                if(!addUnifier(item,u,aux.first,us,j==s))
                                {
                                    u->eraseAll();
                                    return false;
                                }
                            }
                        }
                        else if(!addUnifier(pList,u,aux.first,us,true)){
                            u->eraseAll();
                            return false;
                        }
                    }
                    else {
                        u->eraseAll();
                        return false;
                    }
                }
            }
        }
    }
    return res;
};

bool PythonWrapper::addUnifier(PyObject * pObj, UnifierTable * ut, int v, int size, bool last) const
{
    // esto es un metodo de uso interno.

    // Comprobar el tipo de objeto que deseamos a�adir a la tabla de unificaciones
    pair<int,double> key(-1,0);

    if(PyFloat_Check(pObj)){
        key.second = PyFloat_AsDouble(pObj);
    }
    else if(PyInt_Check(pObj)){
        key.second = (double) PyInt_AsLong(pObj);
    }
    else if(PyLong_Check(pObj)){
        key.second = (double) PyLong_AsLong(pObj);
    }
    else if(PyBool_Check(pObj)){
        key.second = (double) PyInt_AsLong(pObj);
    }
    else if(PyString_Check(pObj)){
        string s = PyString_AsString(pObj);
        // la constante deber�a haberse definido con anterioridad, en otro caso
        // se trata de un error
        ldictionaryit posit = (domain->cdictionary).find(s.c_str());
        if(posit != (domain->ldictionary).end()) {
            // devolver el pkey de la constante
            key.first = (*posit).second;
        }
        else {
            //*errflow << "Warning: Python has defined a new constant: `" << s << "'" << endl;
            ConstantSymbol * n = new ConstantSymbol(s.c_str(),-1);
            key = termtable->addConstant(n);
        }
    }
    else {
        *errflow << "Warning: PythonWrapper: Unexpected type found!.";
        PyObject_Print(pObj,stderr,Py_PRINT_RAW);
        *errflow << endl;
        return false;
    }

    // una vez que tenemos el objeto, lo a�adimos
    // a las unificaciones existentes
    if(!size) {
        // no hay unificaciones previas
        Unifier * u = new Unifier();
        u->addSubstitution(v,key);
        ut->addUnifier(u);
    }
    unifierit b = ut->getUnifierBegin(), e;
    e = b + size;
    for(b = ut->getUnifierBegin(); b!=e; b++){
        if(last) {
            // esto es por motivos de eficiencia. Si soy el �ltimo que se
            // va a a�adir a la tabla de unificadores, entonces no es necesario
            // clonar puedo reutilizar la unificaci�n previa a la que
            // me a�ado
            (*b)->addSubstitution(v,key);
        }
        else {
            // en otro caso debo clonar la estructura antes
            // de a�adirme
            Unifier * u = (*b)->clone();
            u->addSubstitution(v,key);
            ut->addUnifier(u);
        }
    }
    return true;
};

void PythonWrapper::loadFile(const char * name){
    FILE * fp = fopen(name,"r");
    if(fp == 0){
        *errflow << "Errof: PythonWrapper:loadFile. File not found:" << name;
    }
    PyRun_SimpleFile(fp,name);
};

int PythonWrapper::loadStr(const char * code){
    if(code){
        PyObject * src = Py_CompileString(code,"<embedded>",Py_single_input);
        if(src == NULL){
            *errflow << "Error: PythonWrapper::parse(): Unable to parse python : " << endl << code << endl;
            return -1;
        }
        Py_XDECREF (src);
        queue.push_back(code);
    }
    return 0;
};

int PythonWrapper::execQueue(void){
    int s = queue.size();
    for(int i=0; i<s;i++){
        if(PyRun_SimpleString(queue[i].c_str())){
            *errflow << "Error: PythonWrapper::exec(): Unable to execute python : " << endl << queue[i] << endl;
            return -1;
        }
    }
    queue.clear();
    return 0;
};

#endif
