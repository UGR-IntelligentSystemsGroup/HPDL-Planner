#ifndef EXPRESSION_HH
#define EXPRESSION_HH
using namespace std;

#include "unifier.hh"
#include "termTable.hh"
#include "unifiable.hh"
#include "xmlwriter.hh"

class Expression : public Unifiable
{
    public:
	virtual ~Expression(void) {};

        virtual bool isGoal(void) const {return false;};
        virtual bool isAndGoal(void) const {return false;};
        virtual bool isEffect(void) const {return false;};
        virtual bool isAndEffect(void) const {return false;};
        virtual bool isLiteral(void) const {return false;};
        virtual bool isLiteralEffect(void) const {return false;};
        virtual bool isForallGoal(void) const {return false;};
        virtual bool isForallEffect(void) const {return false;};
        virtual bool isFluent(void) const {return false;};

	/**
	 * Devuelve una descripci�n textual del objeto.
	 * @param out El flujo donde escribir.
	 * @param indent, el n�mero de espacios a dejar como tabulado antes de la impresi�n.
	 **/
	virtual void print(ostream * out, int indent=0) const {};

	/**
	 * Devuelve una descripci�n como cadena del objeto.
	 **/
        virtual const char * toString(void) const {
	    static string s; ostringstream os;
	    print(&os,0); s = os.str();
	    return s.c_str();
	};

	/**
	 * Realiza una copia exacta del objeto.
	 **/
	virtual Expression * clone(void) const { return NULL; };

	/**
	 * Devuelve en un documento xml la descripci�n de la tarea.
	 * @param os El flujo en el que escribir.
	 **/
	virtual void toxml(ostream * os) const{
	    XmlWriter * writer = new XmlWriter(os);
	    toxml(writer);
	    writer->flush();
	    delete writer;
	};

	/**
	 * Devuelve en un documento xml la descripci�n de la tarea.
	 * @param writer El objeto en donde escribir.
	 **/
	virtual void toxml(XmlWriter * writer) const {};
};

#endif
