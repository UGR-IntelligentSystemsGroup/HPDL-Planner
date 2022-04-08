#ifndef LITERALEFFECT_H
#define LITERALEFFECT_H

#include "constants.hh"
#include <assert.h>
#include "effect.hh"
#include "unifierTable.hh"
#include "header.hh"
#include "timeStamped.hh"

using namespace std;

class State;

struct EqualKey
{
    bool operator()(pkey k1, pkey k2) const
    {
	if(k1.first == k2.first){
	    if(k1.first != -1){
		return true;
	    }
	    else if(k1.second == k2.second) {
		return true;
	    }
	}

        return false;
    };

};

struct EqualLit 
{
    const KeyList * param;
    EqualLit(const KeyList * param) {this->param = param;};

    bool operator()(const pair<int ,Literal *> &l) const
    {
        return equal(param->begin(),param->end(),l.second->getParameters()->begin(),EqualKey());
    } 
};


class LiteralEffect : public Effect, public Literal, public TimeStamped
{
    public:

        /**
          @brief Constructor.
          @param n nombre del literal.
          @param p La polaridad, por defecto positiva.
         */
        LiteralEffect(int id, int mid, bool p=true);

        LiteralEffect(int id, int mid, const KeyList * param, bool p);

	LiteralEffect(const LiteralEffect * le);

        /**
          @brief Destructor
         */
        virtual ~LiteralEffect() {};

        virtual bool isLiteralEffect(void) const {return true;};

	virtual bool isLEffect(void) const {return true;};

        /**
        * @brief realiza una copia exacta a this.
        */
        virtual Expression * clone(void) const;

        virtual Literal * cloneL(void) const; 

        /**
        @brief Imprime el contenido del objeto.
        @param indent el n�mero de espacios a dejar antes de la cadena.
        @param os Un flujo de salida por defecto la salida estandard.
        */
        virtual void print(ostream * os, int indent=0) const;

        virtual void vcprint(ostream * os, int indent=0) const {print(os,indent);};

        virtual void printL(ostream * os, int indent=0) const {this->print(os,indent);};

        virtual void toxmlL(XmlWriter * writer) const {toxml(writer);};

        virtual void toxml(XmlWriter * writer) const;

	virtual void vctoxml(XmlWriter * w) const {toxml(w);};

        virtual bool apply(State *sta, VUndo * undo, Unifier * uf);

        virtual pkey getTermId(const char * name) const;

        virtual bool hasTerm(int id) const;

        virtual void renameVars(Unifier * u, VUndo * undo);

	virtual bool provides(const Literal *) const;

	/**
	  @brief Asignamos un nuevo signo al literal.
	  @param s: Nuevo signo.
	  */
	inline void setPolarity(bool s) {polarity=s;};

	/**
	  @brief Leemos el signo actual del literal.
	  @return El signo.
	  */
	inline bool getPolarity(void) const {return polarity;};

	inline void setMaintain(bool v=true) {maintain=v;};

	inline bool isMaintain(void) {return maintain;};

	virtual bool getPol(void) const {return getPolarity();};

	virtual void setPol(bool t) {setPolarity(t);};

	virtual bool invertPolarity(void) {if(polarity) polarity=false; else polarity=true; return polarity;};
    protected:
	bool polarity; /**< @brief El signo del literal */

	/** El literal debe ser mantenido. */
	bool maintain; 

};

#endif
