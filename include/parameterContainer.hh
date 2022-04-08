#ifndef PARAMETERCONTAINER_HH
#define PARAMETERCONTAINER_HH

#include "constants.hh"
#include <vector>
#include <string>
#include "variablesymbol.hh"
#include "constantsymbol.hh"
#include "papi.hh"
#include "undoChangeValue.hh"
#include "xmlwriter.hh"

typedef std::pair<int,float> pkey;
typedef std::vector<pkey> KeyList;
typedef KeyList::const_iterator keylistcit;
typedef KeyList::iterator keylistit;

//Variable para controlar la impresion
extern bool PRINTING_COMPARATIONGOAL;
extern bool PRINTING_TASKSNETWORK;
extern bool PRINTING_EFFECT;
extern bool PRINTING_DURATIONS;
extern bool PRINTING_COMPARABLE;
extern bool PRINTING_HEADER_FUNCTION;
extern bool PRINTING_PARAMETERS;

extern bool PRINT_NUMBERTYPE;

// Definici�n de funciones gen�ricas.
// De esta forma tengo los algoritmos centralizados y accesibles desde
// cualquier parte del c�digo. La idea es evitar tener el mismo trozo
// de c�digo replicado en muchos lugares. De esta forma la correcci�n
// de un error se hace sobre todos los objetos simult�neamente. 
struct PrintKey {
    PrintKey(ostream * os) {this->os=os;};
    void operator()(pkey k) const
    {
        *os << " ";
        if(k.first == -1) {
            if(PRINT_NUMBERTYPE) 
                *os << k.second << " - number ";
            else
                *os << k.second << " ";
        }else if(k.first >= 0)
            parser_api->termtable->constants[k.first]->print(os);
        else
            parser_api->termtable->variables[-2 -k.first]->print(os);
    };
    ostream * os;
};

struct ToXMLKey
{
    ToXMLKey(XmlWriter * w) {writer = w;};
    void operator()(pkey k)
    {
        if(k.first == -1){
	    writer->startTag("constant")
		->addAttrib("value",k.second)
		->startTag("type")
		->addAttrib("name","number")
		->endTag()
		->endTag();
	}
        else if(k.first >= 0)
            parser_api->termtable->constants[k.first]->toxml(writer);
        else
            parser_api->termtable->variables[-2 -k.first]->toxml(writer);
    };
    XmlWriter * writer;
};

struct HasName
{
    HasName(const char * name) {this->name=name;};
    bool operator()(pkey k) const
    {
        if(k.first == -1)
        { 
            return false;
        }
        else if(k.first >= 0){
            if(!strcasecmp(parser_api->termtable->constants[k.first]->getName(),name)) return true;
        }
        else{
            if(!strcasecmp(parser_api->termtable->variables[-2 -k.first]->getName(),name)) return true;
        }

        return false;
    };
    const char * name;
};

struct HasId {
    HasId(int id) {this->id=id;};
    bool operator()(pkey k) const {
        return k.first == id;
    };
    int id;
};

struct IsVar {
    bool operator()(pkey k) const {
        return k.first < -1;
    };
};

struct IsNotVar {
    bool operator()(pkey k) const {
        return k.first < -1;
    };
};

class ParameterContainer: public ValueChangeable 
{
    public:
        ParameterContainer(void) {};
	ParameterContainer(const ParameterContainer * pc) ;
	ParameterContainer(const KeyList * v);

	virtual ~ParameterContainer(void);

        /**
        @brief Devuelve un iterador constante al primer elemento de los argumentos
        para recorrer los mismos
        */
        inline keylistcit beginp(void) const {return parameters.begin();};

        /**
        @brief Devuelve un iterador constante un elemento despu�s del �ltimo de los argumentos
        para recorrer los mismos
        */
        inline keylistcit endp(void) const {return parameters.end();};

        /**
          @brief Leemos el numero de parametros del literal.
          @return Numero de parametros.
         */
        inline int sizep(void) const {return parameters.size();};

        /**
          @brief Anade un nuevo par�metro a la cabecera.
          @param name el nombre del par�metro. 
         */
        inline void addParameter(pkey k) {parameters.push_back(k); };

        inline void setParameter(int pos, pkey &val) {parameters[pos] = val;};

	virtual void setVar(int pos, pkey &newval); 

        inline keylistcit searchTermName(const char * name) const
        {
            return find_if(parameters.begin(),parameters.end(),HasName(name));
        };

        inline keylistcit searchTermId(int id) const
        {
            return find_if(parameters.begin(),parameters.end(),HasId(id));
        };

        void varRenaming(Unifier *u, VUndo * undo);

        inline keylistcit parametersBegin(void) const {return parameters.begin();}

        inline keylistcit parametersEnd(void) const {return parameters.end();}

	inline const KeyList * getParameters(void) const {return &parameters;};

	inline KeyList * getModificableParameters(void) {return &parameters;};

	inline pkey getParameter(int i) const {return parameters[i];};

	inline bool hasVariables(void) const
	{
	    return (parameters.end() != find_if(parameters.begin(),parameters.end(),IsVar()));
	};

	inline bool hasConstants(void) const
	{
	    return (parameters.end() != find_if(parameters.begin(),parameters.end(),IsNotVar()));
	};

    protected:
        KeyList parameters;
};

#endif

