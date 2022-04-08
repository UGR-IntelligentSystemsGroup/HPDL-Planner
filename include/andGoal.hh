#ifndef ANDGOAL_HH
#define ANDGOAL_HH

#include "constants.hh"
#include <vector>
#include "containerGoal.hh"
#include "unifierTable.hh"
#include "termTable.hh"

using namespace std;

class AndGoal :public ContainerGoal
{
    public:
	   AndGoal(void) :ContainerGoal() {};

	   AndGoal(const AndGoal * ag);

           virtual ~AndGoal(void);
	
           inline goalcit getBegin(void) const {return goals.begin();}; 

           inline goalcit getEnd(void) const {return goals.end();}; 

           inline int size(void) const {return goals.size();};

           virtual void addGoal(const Goal * g) {assert(g!=0); goals.push_back((Goal *) g->clone());};

           virtual void addGoalByRef(Goal * g) {assert(g!=0); goals.push_back(g);};

           virtual bool isAndGoal(void) {return true;};

           /**
            * @brief Crea una copia exacta del objeto.
            * @description Todos los herederos deben implementar este m�todo.
            * @author oscar
            */
           virtual Expression * clone(void) const;

           /**
             @brief Imprime el contenido del objeto por la salida estandard.
             @param indent el n�mero de espacios a dejar antes de la cadena.
            */
           virtual void print(ostream * os, int indent=0) const;

           virtual void toxml(XmlWriter * writer) const;

           virtual UnifierTable * getUnifiers(const State * state, const Unifier * context, bool inheritPolarity, pair<unsigned int,unsigned int> * protection) const;

           virtual pkey getTermId(const char *) const;

           virtual bool hasTerm(int id) const;

           virtual void renameVars(Unifier * u, VUndo * undo);

	   virtual bool isReachable(ostream * err, bool inheritPolarity) const;

    protected:
        vector<Goal *> goals;
};

#endif
