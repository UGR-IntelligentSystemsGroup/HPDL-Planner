#ifndef CONSTANTS_HH
#define CONSTANTS_HH

#if PYTHON_FOUND
#include <python2.7/Python.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cctype>

using namespace std;

// n�mero de espacios a dejar cuando se necesite realizar una indentaci�n
// a la hora de imprimir la descripci�n de cualquier elemento.
#define NINDENT 3

// Tama�o del historial para la shell de depuraci�n
#define HISTORY_SIZE 20

// estructura para hacer la comparaci�n de dos elementos en una tabla hash
// se supone que las tablas hash son case-insensitive
struct icasecharcmp {
    template <class T1, class T2>
    bool operator() (const T1 & input, const T2 & test) const {
        return toupper(input) == toupper(test);
    }
};

struct ieqstr {
    bool operator() (const std::string & input, const std::string & test) const {
        if (input.size() != test.size()) return false;
        return std::equal(input.begin(), input.end(), test.begin(), icasecharcmp());
    }
};

inline unsigned int _strlen(const std::string & str) {
    return str.size();
};

inline unsigned int _strlen(const char * str) {
    return strlen(str);
};

inline std::string::const_iterator _begin(const std::string & str) {
    return str.begin();
};

inline const char * _begin(const char * str) {
    return str;
};

inline std::string::const_iterator _end(const std::string & str) {
    return str.end();
};

inline const char * _end(const char * str) {
    return str + strlen(str);
};

template <class T1, class T2>
inline bool icasestrcmp(const T1 & input, const T2 & test) {
        if (_strlen(input) != _strlen(test)) return false;
        return true;
        return std::equal(_begin(input), _end(input), _begin(test), icasecharcmp());
};

struct icaseDJBHash
        : std::unary_function<std::string, std::size_t>
    {
        icaseDJBHash() {}

        template <typename String>
        std::size_t operator()(String const& x) const {
            std::size_t hash = 5381;

            for(typename String::const_iterator it = x.begin(); it != x.end(); ++it) {
                hash = ((hash << 5) + hash) + toupper(*it);
            }

            return hash;
        };
};

template<class T1,class T2>
struct EQPAIR{
    pair<T1,T2> element;
    EQPAIR(pair<T1,T2> e) :element(e) {};
    bool operator()(const pair<T1,T2> & o){
        return (o.first == element.first && o.second == element.second);
    }
};

template<class T1,class T2>
struct EQPAIRFIRST{
    T1 element;
    EQPAIRFIRST(T1 e) {element = e;};
    bool operator()(const pair<T1,T2> & o){
        return (o.first == element);
    }
};

template<class T1,class T2>
struct EQPAIRSECOND{
    T2 element;
    EQPAIRSECOND(T2 e) {element = e;};
    bool operator()(const pair<T1,T2> & o){
        return (o.second == element);
    }
};

/**
 * Definici�n de banderas para el planificador
 */

// Guardar todo el espacio de b�squeda. (Mantener una lista de nodos cerrados)
extern bool FLAG_DEBUG;
extern int FLAG_VERBOSE;
class Debugger;
extern Debugger * debugger;
extern int idCounter;
// en que flujo escribir los mensjes de error
extern ostream * errflow;

class Controlrules;
extern Controlrules * controlrules;

template<class T>
struct Delete
{
    void operator()(T * c) const {
        if(c)
            delete c;
        c=0;
    };

    void operator()(pair<const int,T * > * c) const {
        if(c){
            if(c->second)
                delete c->second;
        }
        c->second=0;
    };

    void operator()(pair<const int,T * > & c) const {
        if(c.second)
            delete c.second;
        c.second=0;
    };
};

template<class T>
struct DeleteVector {
    void operator()(vector<T> * v) const {
        if(v)
            delete v;
        v=0;
    }
};

template<class T>
struct CloneV
{
    CloneV(vector<T *> * v) {this->v = v;};
    void operator()(const T * e) {
        if(e)
            v->push_back((T *) e->clone());
        else
            v->push_back((T *)0);
    };
    vector<T *> * v;
};

template<class T>
struct AddV {
    AddV(vector<T *> * v) {this->v = v;};
    void operator()(T * e)
    {
        v->push_back((T *) e);
    };
    vector<T *> * v;
};

template<class T>
struct AddV2 {
    AddV2(vector<T> * v) {this->v = v;};
    void operator()(T e)
    {
        v->push_back((T) e);
    };
    vector<T> * v;
};

template<class T>
struct Print {
    Print(ostream * os) {this->os = os; indent=0;};
    Print(ostream * os, int nindent) {this->os = os;indent=nindent;};
    void operator()(const T * e) const {
        e->print(os,indent);
        *os << endl;
    };

    void operator()(const pair<int,T *> &e) const {
        e.second->print(os,indent);
        *os << endl;
    };

    void operator()(T e) const {
        string s(indent,' ');
        *os << s;
        *os << e;
        *os << endl;
    };

    ostream * os;
    int indent;
};

template<class T>
struct PrintL {
    PrintL(ostream * os) {this->os = os; indent=0;};
    PrintL(ostream * os, int nindent) {this->os = os;indent=nindent;};
    void operator()(const T * e) const {
        e->printL(os,indent);
        *os << endl;
    };

    void operator()(const pair<int,T *> &e) const {
        e.second->printL(os,indent);
        *os << endl;
    };

    void operator()(T e) const {
        string s(indent,' ');
        *os << s;
        *os << e;
        *os << endl;
    };

    ostream * os;
    int indent;
};


template<class T, class FLOW>
struct ToXML {
    ToXML(FLOW * os) {this->os = os;};

    void operator()(const T * e) const {
        e->toxml(os);
    };

    void operator()(const pair<int,T *> &e) const{
        e.second->toxml(os);
    }

    FLOW * os;
};

template<class T, class FLOW>
struct ToXMLL {
    ToXMLL(FLOW * os) {this->os = os;};

    void operator()(const T * e) const {
        e->toxmlL(os);
    };

    void operator()(const pair<int,T *> &e) const {
        e.second->toxmlL(os);
    }

    FLOW * os;
};

template<class T, class TN>
struct PrintHead {
    PrintHead(ostream * os) :s(NINDENT,' ') {this->os = os;};
    PrintHead(ostream * os, int nindent) :s(nindent,' ') {this->os = os;};
    PrintHead(ostream * os, int nindent, const TN * tn) :s(nindent,' ') {this->os = os; this->tn = tn;};
    void operator()(const T * e) const {
        *os << s;
        e->printHead(os);
        *os << endl;
    };

    void operator()(const pair<int,T *> & e) const {
        *os << s;
        e.second->printHead(os);
    };

    void operator()(long t) const {
        *os << s;
        const T * e = tn->getTask(t);
        e->printHead(os);
        *os << endl;
    };

    ostream * os;
    const TN * tn;
    string s;
};

template<class T, class TN>
struct ToXMLHead {
    ToXMLHead(ostream * os) :s(NINDENT,' ') {this->os = os;};
    ToXMLHead(ostream * os, int nindent) :s(nindent,' ') {this->os = os;};
    ToXMLHead(ostream * os, int nindent, const TN * tn) :s(nindent,' ') {this->os = os; this->tn = tn;};
    void operator()(const T * e) const {
        *os << s;
        e->toxmlHead(os);
        *os << endl;
    };

    void operator()(long t) const {
        *os << s;
        const T * e = tn->getTask(t);
        e->toxmlHead(os);
        *os << endl;
    };

    ostream * os;
    const TN * tn;
    string s;
};

template<class T>
struct Print2 {
    Print2(ostream * os) {this->os = os; s = "";};
    Print2(ostream * os, int nindent) :s(nindent,' ') {this->os = os;};
    void operator()(const T * e) const {
        *os << s;
        e->print(os);
    };

    void operator()(const T t) const {
        *os << s << t;
    };

    string s;
    ostream * os;
};

typedef vector<string *> vstring;
typedef pair<int,float> pkey;

#endif
