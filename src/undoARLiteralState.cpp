#include "papi.hh"
#include "state.hh"
#include "problem.hh"
#include "undoARLiteralState.hh"
#include "plan.hh"
extern Plan * current_plan;

UndoARLiteralState::UndoARLiteralState(Literal * l, bool v) 
{
    added=v;
    ref=l;
    time = -1;
    //deleted = false;
    //cerr << l->getPol() << " " << v << endl;
    //l->printL(&cerr);
    //cerr << "---------------------" << endl;
    //assert(l->getPol() == v);
};

void UndoARLiteralState::print(ostream * os) const
{
    if(wasAdded())
	*os << "Added to state: ";
    else
	*os << "Deleted from state: ";
    ref->printL(os,0);
};

void UndoARLiteralState::toxml(XmlWriter * writer) const{
    if(wasAdded()){
	writer->startTag("added");
	if(time== -1 || time >= ATEND)
	    writer->addAttrib("at","end");
	else
	    writer->addAttrib("at",time);
	ref->toxmlL(writer);
	writer->endTag();
    }
    else{
	writer->startTag("deleted");
	if(time== -1 || time >= ATEND)
	    writer->addAttrib("at","end");
	else
	    writer->addAttrib("at",time);
	ref->toxmlL(writer);
	writer->endTag();
    }
};

void UndoARLiteralState::undo(void)
{
    // si fue a�adido eliminar del estado
    if(wasAdded()){
	current_plan->deleteFromState(ref);
	//deleted=true;
    }
    else 
	current_plan->addToState(ref);
};

// Si yo lo a�ado yo lo quito.
// Si yo lo quito que lo quite el que lo a�adi� ;)
UndoARLiteralState::~UndoARLiteralState(void) 
{
    if(wasAdded()) {
	if(ref){
	    //if(!deleted){
	    //    cerr << "Warning: Destroying a literal not deleted from the state";
	    //    ref->printL(&cerr);
	    //}
	    //cerr << "("<< ref << ")" << " ---@@@@@ ----------------" << endl;
	    //ref->printL(&cerr);
	    //cerr << endl;
	    current_plan->deleteFromState(ref);
	    delete ref;
	}
	ref =0;
    }
};

UndoElement * UndoARLiteralState::clone(void) const
{
    return new UndoARLiteralState(ref->cloneL(),added);
};

