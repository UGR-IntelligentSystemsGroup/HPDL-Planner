#ifndef FLUENTVAR_HH
#define FLUENTVAR_HH

#include "constants.hh"
#include <assert.h>
#include "evaluable.hh"
#include "undoChangeValue.hh"
#include "variablesymbol.hh"
#include "unifier.hh"
#include "constantsymbol.hh"

using namespace std;

/**
 * Esta clase representa a variables que son usadas dentro
 * de expresiones aritm�ticas o de comparaci�n.
 * Proporciona las funciones adecuadas para manejarlas y
 * evaluarlas.
 */
class FluentVar :public Evaluable, public ValueChangeable
{
    public:
    /**
     * Constructor
     */
        FluentVar(int id) {setId(id);};


        FluentVar(const pkey * id) {this->id.first=id->first; this->id.second=id->second;};

    /**
     * Constructor de copia.
     */
    FluentVar(const FluentVar * other) :Evaluable(other) {id = other->id;};

    virtual ~FluentVar(void) 
    {
        // Al eliminar el fluent, eliminar las referencias cruzadas que pueda tener la variable.
        if(id.first < -1)
    	parser_api->termtable->getVariable(id)->removeReference(this);
    };

        inline void setId(int v) {id.first = v; id.second = 0;};

        inline pkey getId(void) const {return id;};

        virtual Evaluable * cloneEvaluable(void) const {return new FluentVar(this);};

    /**
     * Imprime en texto la informaci�n sobre la clase.
     * @param os El flujo de salida.
     * @param indent N�mero de espacios que se utilizar�n al principio de cada l�nea
     */
    virtual void printEvaluable(ostream * os,int indent=0) const 
    {
        string s(indent,' ');
        if(id.first != -1){
    	 *os << s;
    	 parser_api->termtable->print(id,os);
        }
        else
    	*os << s << id.second;
    };

    /**
     * Imprime en texto la informaci�n sobre la clase.
     * @param os El flujo de salida.
     * @param indent N�mero de espacios que se utilizar�n al principio de cada l�nea
     */
        virtual void vcprint(ostream * os, int indent=0) const {printEvaluable(os,indent);};

    /**
     * Imprime en xml la informaci�n sobre la clase.
     */
    virtual void toxmlEvaluable(XmlWriter * writer) const {
        writer->startTag("fluent")
    	->addAttrib("value",parser_api->termtable->getTerm(id)->getName())
    	->endTag();
    };

    virtual void vctoxml(XmlWriter * w) const {toxmlEvaluable(w);};

        virtual pkey compGetTermId(const char * name) const {if(id.first < -1 && strcasecmp(name,parser_api->termtable->getTerm(id)->getName()) == 0) return id; else return make_pair(-1,0);};

        virtual bool compHasTerm(int i) const {if(id.first==i)return true; else return false;};

    virtual void setVar(int pos, pkey &newval)
    {
        id = newval;
    };

    /**
     * Esta funci�n sirve para que todas las variables hagan
     * referencia a la misma cosa. Cuando hay una unificaci�n
     * de dos variables, se hace que las dos tengan el mismo
     * id de forma que cuando hay una unificaci�n la sustituci�n
     * autom�ticamente afecta a todas las variables.
     */
        virtual void compRenameVars(Unifier * u, VUndo * undo) 
    {
        pkey p;

        if(u->getSubstitution(id.first,&p))
        {
    	if(undo)
    	    undo->push_back(new UndoChangeValue(this,0,id,p));
    	id = p;
    	if(p.first < -1)
    	    parser_api->termtable->getVariable(p)->addReference(new UndoChangeValue(this,0,p,p));
        }
        else
        {
    	//se crea una nueva entrada en la tabla de t�rminos
    	pkey nv = parser_api->termtable->addVariable((VariableSymbol *) parser_api->termtable->getVariable(id)->clone());
    	if(undo)
    	    undo->push_back(new UndoChangeValue(this,0,id,nv));
    	parser_api->termtable->getVariable(nv)->addReference(new UndoChangeValue(this,0,nv,nv));
    	u->addSubstitution(id.first,nv);
    	id = nv; 
        }
    };

    /**
     * Esta funci�n es una de las m�s importantes de la clase.
     * Dado un estado y un contexto de unificaci�n, la funci�n
     * busca en el contexto de unificaci�n si hay alguna sustituci�n
     * de la variable. Si la encuentra devuelve dicha sustituci�n
     * en otro caso, devuelve un error.
     * @param El estado actual de planificaci�n.
     * @param El contexto de unificaci�n.
     * @return la pkey que representa el contenido de la variable.
     */
        virtual pkey eval(const State * state, const Unifier * context) const 
    {
        static pkey result;

        if(id.first >= -1)
    	return id;

        if(context){
    	if(context->getSubstitution(id.first,&result))
    	    return result;
        }

        return make_pair(INT32_MAX,0);
    };

    /**
     * Esta funci�n es una de las m�s importantes de la clase.
     * Dado un estado y un contexto de unificaci�n, la funci�n
     * busca en el contexto de unificaci�n si hay alguna sustituci�n
     * de la variable. Si la encuentra devuelve dicha sustituci�n
     * en otro caso, devuelve un error.
     * @param state El estado actual de planificaci�n.
     * @param contex El contexto de planificaci�n.
     * @param tp Es un par�metro de salida. Contendr� un time point si
     * durante la evaluaci�n se encuentra una referencia a dicho elemento.
     * @param pol Es un par�metro de salida. Si es true el tp no va afectado
     * por un signo negativo, false en otro caso.
     * @return un pkey con el resultado de la evaluaci�n.
     */
        virtual pkey evaltp(const State * state, const Unifier * context, pkey * tp, bool * pol) const 
    {
        static pkey result = make_pair(INT32_MAX,0);

        if(id.first >= -1){
    	// la variable ha sido substituida por un s�mbolo
    	// o por un n�mero.
    	if(id.first == -1)
    	    return id;
    	else if(isTimePoint(id)){
    	    if(tp->first != -1){
    		*errflow << "FluentVar:: Error: Two or more time points in the expression are not allowed." << endl;
    		exit(-1);
    	    }
    	    *tp = id;
    	    return result;
    	}
    	else
    	    return id;
        }

        if(context){
    	if(context->getSubstitution(id.first,&result)) {
    	    if(isTimePoint(result)){
    		if(tp->first != -1){
    		    *errflow << "FluentVar:: Error: Two or more time points in the expression are not allowed." << endl;
    		    exit(-1);
    		}
    		*tp = result;
    		return make_pair(INT32_MAX,0);
    	    }
    	    else {
    		return result;
    	    }
    	}
        }

        return result;
    };

    virtual void compAddType(Type * t) {assert(t != 0);getVarTypes()->push_back(t);};

    virtual void addTypes(const vector<Type *> * vt) {getVarTypes()->insert(getVarTypes()->end(),vt->begin(),vt->end());};

    /**
      @brief Devuelve el primer tipo del t�rmino. 
      @return el primer tipo o getEndType() si el t�rmino es de tipo object.
      */
    virtual typecit getBeginType(void) const {return getVarTypes()->begin();};

    /**
      @brief Devuelve un iterador uno despu�s del �ltimo elemento. 
      */
    virtual typecit getEndType(void) const {return getVarTypes()->end();};

    /**
      @brief Devuelve el tipo apuntado por el iterador. 
      */
    virtual const Type * getType(typecit i) const {return (*i);};

    virtual vctype * getTypes(void) {if(id.first != -1) return parser_api->termtable->getTerm(id)->getTypes(); else return 0;};

    /**
      @brief Algo es de tipo object si no tiene tipos asociados.
      */
    virtual bool isObjectType(void) const {return getVarTypes()->empty();};

    /**
      @brief Devuelve true si el t�rmino es del tipo indicado, false en otro caso
      */
    virtual bool isType(const Type *t) const {for(typecit i=getVarTypes()->begin(); i!=getVarTypes()->end(); i++) if((*i)->isSubTypeOf(t)) return true; return false;};

    virtual bool hasType(const Type *t) const {for(typecit i=getVarTypes()->begin(); i!=getVarTypes()->end(); i++) if((*i)->equal(t)) return true; return false;};

    virtual bool hasTypeId(int id) const {for(typecit i=getVarTypes()->begin(); i!=getVarTypes()->end(); i++) if((*i)->getId()== id) return true; return false;};

    /**
      @brief Elimina los tipos asociados al t�rmino
      */
    virtual void clearTypes(void) {getVarTypes()->clear();};

    protected:
    vctype * getVarTypes(void) const {return parser_api->termtable->getTerm(id)->getTypes();};

        pkey id;
};

#endif

