#ifndef IMPLYGOAL
#define IMPLYGOAL

#include "constants.hh"
#include "goal.hh"

using namespace std;

class ImplyGoal: public Goal
{
    public:
	ImplyGoal(void) :Goal() {condition = 0; goal=0;};
	ImplyGoal(Goal * p, Goal * g) :Goal() {condition=p; goal = g;};
	ImplyGoal(const ImplyGoal * o);
	~ImplyGoal(void) {if(condition) delete condition; if(goal) delete goal;};

	virtual void renameVars(Unifier* u, VUndo* undo);
	virtual pkey getTermId(const char* n) const;
	virtual bool hasTerm(int id) const;
	virtual void print(ostream* out, int nindent) const;
	virtual void toxml(XmlWriter * writer) const;
	virtual Expression* clone(void) const;
	/**
	 * Calcula los unificadores para un imply.
	 * Realmente un imply no generar� unificaciones. Cuando falle devolver� null, cuando el
	 * imply sea cierto, se devolver� una tabla de unificadores con un solo unificador que estar�
	 * vac�o en el caso de que u sea null o un clon de u. 
	 * En el caso de que se necesiten almacenar los v�nculos causales, en el unificador
	 * devuelto se almacenar� la estructura que registra los v�nculos causales que fueron necesarios
	 * para llervar a t�rmino el imply.
	 * @param state Es el estado actual del mundo.
	 * @param u Es el contexto actual de unificaci�n.
	 * @param polarity Es la polaridad heredada, sirve para llevar la cuenta de los not.
	 * @return Lee arriba
	 */
	virtual UnifierTable* getUnifiers(const State* state, const Unifier* u, bool polarity,  pair<unsigned int,unsigned int> * protection) const;
	virtual bool isReachable(ostream* err, bool p) const;

    protected:
	Goal * condition;
	Goal * goal;
};

#endif
