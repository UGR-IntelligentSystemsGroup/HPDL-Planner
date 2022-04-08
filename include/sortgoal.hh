#ifndef SORTGOAL
#define SORTGOAL

#include "goal.hh"
#include "parameterContainer.hh"
#include "unifier.hh"

//Variable para controlar la impresion
extern bool PRINTING_SORTGOAL;
extern bool PRINT_NUMBERTYPE;
extern bool PRINT_OBJECTTYPE;
extern bool PRINT_DEFINEDTYPES;

enum SortType {SASC,SDESC};
#define DEFAULT_CRITERIA SDESC
typedef vector<SortType> VCriteria;
typedef VCriteria::const_iterator criteriacit;
typedef VCriteria::iterator criteriait;

// funci�n que realiza la ordenaci�n entre dos unificadores,
// teniendo en cuenta una variable
struct CriteriaF
{
    const VCriteria * f;
    const KeyList * par;

    CriteriaF(const KeyList * par, const VCriteria * f) 
    {
	this->f = f;
	this->par = par;
    };

    // debe devolver true si a va antes que b
    // false en otro caso
    bool operator()(Unifier * a, Unifier * b) const
    {
	criteriacit i = f->begin(), e = f->end();
	int s=0, r;
	pkey v,e1,e2;
	if(par){
	    s = par->size();
	}
	//cerr << "Comparando: ";
	//a->print(&cerr);
	//b->print(&cerr);
	for(int c=0;c<s;i++,c++)
	{
	    // escogemos el par�metro que nos toque y que estemos
	    // usando para ordenar.
	    //cerr << s << " " << c;
	    try{
		v = (*par)[c];
		// buscamos sus sustituciones
		if(!a->getSubstitution(v.first,&e1) || !b->getSubstitution(v.first,&e2)) {
		    *errflow << "Runtime error (1): (sortby) Triying to compare an unbound var. " << endl;
		    //cerr << "true" << endl;
		    return true;
		}	
		//cerr << "Comparando: [" << e1.first << "," << e1.second << "] [" << e2.first << "," << e2.second << "]" << endl;
		if(e1.first < -1 || e2.first < -1) {
		    *errflow << "Runtime error (2): (sortby) Triying to compare an unbound var." << endl;
		    //cerr << "true" << endl;
		    return true;
		}
	    }catch(...){
		*errflow << "Runtime error (2): (sortby) Triying to compare an unbound var." << endl;
		return true;
	    }

	    // Usar criterio por defecto (Ascendente)
	    // es un n�mero
	    if(e1.first == -1) {
		if(e2.first == -1) {
		    if(e1.second != e2.second){
			if((*i) == SDESC) {
			    //cerr << "desc " << (e1.second > e2.second) << endl;
			    return (e1.second > e2.second);
			}
			else{
			    //cerr << "asc " << (e1.second < e2.second) << endl;
			    return (e1.second < e2.second);
			}
		    }
		}
		else {
		    *errflow << "Runtime error (1): (sortby) Triying to compare number with term." << endl;
		    return true;
		}
	    }
	    else {
		// es un t�rmino, ordenaci�n lexicogr�fica por defecto.
		if(e2.first == -1) {
		    *errflow << "Runtime error (2): (sortby) Triying to compare number with term." << endl;
		    //cerr << "false" << endl;
		    return false;
		}
		else{
		    r =strcasecmp(parser_api->termtable->getConstant(e1.first)->getName(),parser_api->termtable->getConstant(e2.first)->getName());
		    if(r != 0){
			if((*i) == SDESC) {
			    //cerr << (r > 0) << endl;
			    return r > 0;
			}
			else
			    //cerr << (r < 0) << endl;
			    return r < 0;
		    }
		}
	    }
	}
	// cerr << "true" << endl;
	return true;
    };

};

class SortGoal: public Goal, public ParameterContainer
{
    public:
	SortGoal(void) :Goal(), ParameterContainer() {goal=0;};
	SortGoal(const SortGoal* cg) :Goal(cg), ParameterContainer(cg), criteria(cg->criteria)
	{
	    if(cg->goal) 
		goal = (Goal *)cg->goal->clone(); 
	    else goal=0;
	};

	virtual ~SortGoal(void) 
	{
	    delete goal;
	};

	virtual bool isSortGoal(void) const {return true;};
	inline void setGoal(Goal * g) {goal=g;};
	inline const Goal * getGoal(void) {return goal;};
	inline void addCriteria(SortType c) {criteria.push_back(c);};
	inline const VCriteria * getCriteria(void) {return &criteria;};
	inline criteriacit getCriteriaBegin(void) {return criteria.begin();};
	inline criteriacit getCriteriaEnd(void) {return criteria.end();};

	virtual UnifierTable * getUnifiers(const State * state, const Unifier * context, bool inheritPolarity,  pair<unsigned int,unsigned int> * protection) const
	{
	    bool pol = true;
	    if((!getPolarity() || !inheritPolarity) && (!(!getPolarity() && !inheritPolarity)))
		pol = false;
	    UnifierTable * ut = goal->getUnifiers(state,context,pol,protection);

	    // ordenar los unificadores usando el criterio dado por el codificador
	    // del dominio
	    /*unifiercit i, e = ut->getUnifierEnd();
	      for(i=ut->getUnifierBegin();i!=e;i++)
	      cerr << (*i) << endl;*/
	    if(ut)
		stable_sort(ut->getUnifierBegin(),ut->getUnifierEnd(),CriteriaF(&parameters,&criteria));
	    return ut;
	};

	virtual bool isReachable(ostream * err, bool pol) const
	{
	    bool p = true;
	    if((!getPolarity() || !pol) && (!(!getPolarity() && !pol)))
		p = false;
	    return goal->isReachable(err,p);
	};

	virtual void renameVars(Unifier* u , VUndo * vr)
	{
	    varRenaming(u,vr);
	    goal->renameVars(u,vr);
	};

	virtual pkey getTermId(const char* n) const
	{
	    keylistcit i = searchTermName(n);
	    if(i != parameters.end())
		return (*i);
	    return goal->getTermId(n);
	};

	virtual bool hasTerm(int i) const
	{
	    if(searchTermId(i) != parameters.end())
		return true;
	    if(goal->hasTerm(i))
		return true;
	    return false;
	};

	virtual void setVar(int pos, pkey &newval)
	{
	    setParameter(pos,newval);
	};

	virtual void print(ostream* out, int ind) const
	{
	    bool PRINT_NUMBERTYPE2, PRINT_OBJECTTYPE2, PRINT_DEFINEDTYPES2;

	    PRINTING_SORTGOAL = true;
	    string s(ind,' ');
	    *out << s << "(:sortby" << endl;

	    //Guardamos los valores antiguos
	    PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
	    PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
	    PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;

	    //Fijamos los nuevos valores
	    PRINT_NUMBERTYPE = false;
	    PRINT_OBJECTTYPE = false;
	    PRINT_DEFINEDTYPES = false;

	    int c = 0;
	    assert(criteria.size() == parameters.size());
	    criteriacit i, e = criteria.end();
	    for(i=criteria.begin();i!=e;i++,c++){
		parser_api->termtable->print(parameters[c],out,ind+NINDENT);

		if((*i) == SASC)
		    *out << " :asc" << endl;
		else 
		    *out << " :desc" << endl;
	    }

	    //Recuperamos los valores anteriores
	    PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;
	    PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
	    PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;

	    goal->print(out,ind+NINDENT);
	    *out << endl << s <<  ")";
	    PRINTING_SORTGOAL = false;
	};

	virtual void vcprint(ostream * os, int indent=0) const {print(os,indent);};

	virtual void toxml(XmlWriter * writer) const
	{
	    writer->startTag("sort");
	    criteriacit i, e = criteria.end();
	    int c=0;
	    for(i=criteria.begin();i!=e;i++,c++){
		writer->startTag("by")
		    ->addAttrib("variable",parser_api->termtable->getVariable(parameters[c])->getName());
		if((*i) == SASC)
		    writer->addAttrib("order","asc");
		else
		    writer->addAttrib("order","desc");
		writer->endTag();
	    }
	    goal->toxml(writer);
	    writer->endTag();
	};

	virtual void vctoxml(XmlWriter * w) const {toxml(w);};

	virtual Expression* clone() const
	{
	    return (Expression *) new SortGoal(this);
	};

    protected:
	Goal * goal;
	VCriteria  criteria;
};
#endif
