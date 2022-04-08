#ifndef PRINTGOAL_HH
#define PRINTGOAL_HH

#include <vector>
#include "goal.hh"
#include "unifierTable.hh"
#include "termTable.hh"

using namespace std;

class PrintGoal: public Goal, public ParameterContainer
{
    public:
	   PrintGoal(void) :Goal(),ParameterContainer() {goal=0;str="";};

	   PrintGoal(const PrintGoal * g) :Goal(g), ParameterContainer(g) {
	       goal=0;
	       str="";
	       if(g->goal)
		   goal = (Goal *)g->goal->clone();
	       str= g->str;
	   };

           virtual ~PrintGoal(void) {if(goal) delete goal;};
	
           virtual Expression * clone(void) const{
	       return (Expression *) new PrintGoal(this);
	   };

           virtual void print(ostream * os, int indent=0) const{
	       string s(indent,' ');

	       *os << s;
	       if(isTimed()){
		   *os << "(";
		   time->print(os,0);
		   *os << " ";
	       }
	       if(!polarity)
		   *os << "(not ";
	       *os << "(:print" << endl;
	       if(goal)
		   goal->print(os,indent+NINDENT);
	       else if(!parameters.empty()){
		   for_each(parameters.begin(),parameters.end(),PrintKey(os));
	       }
	       else
		   *os << s << "   \"" << str<< "\"" << endl;
	       *os << endl << s << ")" << endl;
	       if(!getPolarity())
		   *os << ")";
	       if(isTimed())
		   *os << ")";
	   };

	   virtual void vcprint(ostream * os, int indent=0) const {print(os,indent);};

           virtual void toxml(XmlWriter * writer) const {};

	   virtual void vctoxml(XmlWriter * w) const {toxml(w);};

           virtual UnifierTable * getUnifiers(const State * state, const Unifier * context, bool inheritPolarity, pair<unsigned int,unsigned int> * protection) const{
	       if(goal){
		   UnifierTable * ut = goal->getUnifiers(state,context,inheritPolarity,protection);
		   if(ut)
		       ut->print(errflow);
		   return ut;
	       }
	       else if(!parameters.empty()){
		   keylistcit i, e = parameters.end();
		   for(i=parameters.begin();i!=e;i++){
		       if(context && parser_api->termtable->isVariable((*i))){
			   pkey p;
			   if(context->getSubstitution((*i).first,&p))
			       parser_api->termtable->print(p,errflow);
			   else
			       parser_api->termtable->print(*i,errflow);
		       }
		       else
			   parser_api->termtable->print(*i,errflow);
		   }
	       }
	       else {
		   string aux(1,'\n');
		   string s = str;
		   int pos = s.find("\\n",0);
		   while(pos < (int)s.length() && pos >= 0){
		       s.replace(pos,2,aux);
		       pos = s.find("\\n",0);
		   }
		   *errflow << s;
	       }
	       UnifierTable * ut = new UnifierTable();
	       if(context)
		   ut->addUnifier(context->clone());
	       else
		   ut->addUnifier(new Unifier());
	       return ut;

	   };

           virtual pkey getTermId(const char * n) const{
	       keylistcit i = searchTermName(n);
	       if(i != parameters.end())
		   return (*i);
	       if(goal)
		   return goal->getTermId(n);
	       else 
		   return make_pair(-1,0);
	   };

           virtual bool hasTerm(int id) const{
	       if(searchTermId(id) != parameters.end())
		   return true;
	       if(goal)
		   return goal->hasTerm(id);
	       else
		   return false;
	   };

           virtual void renameVars(Unifier * u, VUndo * undo){
	       varRenaming(u,undo);
	       if(goal)
		   goal->renameVars(u,undo);
	   };

	   virtual bool isReachable(ostream * err, bool inheritPolarity) const{
	       if(goal)
		   return goal->isReachable(err,inheritPolarity);
	       return true;
	   };

	   inline void setGoal(Goal * g) {goal = g;};

	   inline void setStr(const char * s) {
	       str = s; 
	   };

	   inline void setStr(string s) {str = s;};

    protected:
        Goal * goal;
	string str;
};

#endif
