#ifndef TYPE_H
#define TYPE_H
#include "constants.hh"
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include "xmlwriter.hh"

using namespace std;

class Type;
class ConstantSymbol;

typedef vector<Type *> vctype;
typedef vector<Type *>::iterator typeit;
typedef vector<Type *>::const_iterator typecit;
typedef vector<const ConstantSymbol *> vconstants;

/**
 * Clase que representa los tipos definidos por el usuario
 * en el dominio.
 */
class Type {
    public:

    Type(const char * n, int i) :name(n) {id=i;lineNumber=0;fileId=-1;};

    Type(const char * n) :name(n) {id=-1;lineNumber=0;fileId=-1;};

    Type(void) {name="";id=-1;lineNumber=0;fileId=-1;};

    virtual ~Type(void) {};

    void addSuperType(Type * t);

    void addSuperTypes(const vector<Type *> * v);

    /**
      @brief devuelve true si this es subtype o es igual a t
      */
    bool isSubTypeOf(const Type * t) const;

    /**
      @brief Comprueba si dos tipos son iguales 
      */
    virtual bool equal(const Type * t) const;

    /**
      @brief Imprime el contenido del objeto por la salida estandard.
      @param indent el n�mero de espacios a dejar antes de la cadena.
      */
    virtual void print(ostream * os, int indent=0) const { string s(indent,' '); *os << s << name;};

    virtual void printSuperTypes(ostream * os) const;

    /**
     * Devuelve en un documento xml la descripci�n de la tarea.
     * @param os El flujo en el que escribir.
     **/
    virtual void toxml(ostream * os, bool super = false, bool sub= false) const{
        XmlWriter * writer = new XmlWriter(os);
        toxml(writer,super,sub);
        writer->flush();
        delete writer;
    };

    /**
     * Devuelve en un documento xml la descripci�n de la tarea.
     * @param writer El objeto en donde escribir.
     **/
    virtual void toxml(XmlWriter * writer, bool super=false,bool sub = false) const;

    inline vector<Type *>::const_iterator getParentsBegin(void) const {return parents.begin();};

    inline vector<Type *>::const_iterator getParentsEnd(void) const {return parents.end();};

    inline const Type * getType(vector<Type *>::const_iterator i) const {return (*i);};

    inline bool isRoot(void) const {return parents.empty();};

    void addReferencedBy(const ConstantSymbol * cs);

    void addRefsBy(const vector<ConstantSymbol *> * cs) {for_each(cs->begin(),cs->end(),bind1st(mem_fun1_t<void,Type,const ConstantSymbol *>(&Type::addReferencedBy),this));};

    inline vconstants::const_iterator getReferencedBegin(void) const  {return referencedBy.begin();};

    inline vconstants::const_iterator getReferencedEnd(void) const  {return referencedBy.end();};

    inline bool emptyReferencedBy(void) const {return referencedBy.empty();};

    inline int getNumberOfParents(void) const {return parents.size();};

    inline const char * getName(void) const {return name.c_str();};

    inline void setName(const char * n) {name = n;};

    inline int getId(void) const {return id;};

    inline void setId(int i) {id = i;};

    inline int getLineNumber(void) const {return lineNumber;};

    inline void setLineNumber(int i) {lineNumber = i;};

    inline int getFileId(void) const {return fileId;};

    inline void setFileId(int i) {fileId = i;};

    protected:
    vector<Type *> parents;
    vector<Type *> children;
    // este vector sirve para mantener referencias inversas de objetos (constantes) que
    // tienen asociado este tipo. De esta forma si tengo por ejemplo (forall (?x - tipo))
    // puedo encontrar r�pidamente las constantes que son de tipo "tipo"
    vconstants referencedBy;
    string name;
    int id;
    int lineNumber;
    int fileId;

    /**
     * A�ade un subtipo a un tipo determinado.
     */
    void addSubType(Type * t);
};

struct PrintTree
{
    PrintTree(ostream * os) {this->os = os; indent=0;};
    PrintTree(ostream * os, int nindent) {this->os = os; indent=nindent;};
    void operator()(const Type * t) const
    {
    if(strcmp("number", (char *) t->getName())!=0) {
        t->print(os,indent);
        t->printSuperTypes(os);
        *os << endl;
    }
    };

    ostream * os;
    int indent;
};


#endif
