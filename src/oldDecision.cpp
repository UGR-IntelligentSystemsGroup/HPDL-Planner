#include "oldDecision.hh"

OldDecision::OldDecision(const StackNode * st, const TaskNetwork * tn){
    // Obtener cual es la tarea que se decidió expandir en este nodo
    const Task * t;

    primitive = true;
    method_id = -1;
    if(st->taskid != -1 && st->taskid < tn->getNumOfNodes()){
	t = (*(st->offspring))[st->task];	
	task_selected = new TaskHeader(t->getId(),t->getMetaId(),t->getParameters());
	if(!t->isPrimitiveTask()){
	    primitive = false;
	    const Method * m = (*(st->methods))[st->mpos];
	    method_id = m->getMetaId();
	}
	const Unifier * uf = 0;
	if (st->utable)
	    uf = st->utable->getUnifierAt(st->unif);
	if(uf)
	    u = uf->clone();
	else
	    u = 0;
    }
    else
	task_selected = 0;

};

OldDecision::~OldDecision(void){
    if(task_selected)
	delete task_selected;
    if(u)
	delete u;
};

void OldDecision::print(ostream * os, int tab) const {
    string t(' ',tab);

    *os << t << "Expanding task:" << endl;
    if(task_selected){
	if(primitive){
	    *os << "Primitive: ";
	    task_selected->print(os);
	}
	else{
	    *os << "Compound: ";
	    task_selected->print(os);
	    *os << "Method: " << method_id << endl;
	}
	if(u)
	    u->print(os);
    }
    else 
	*os << "none" << endl;
};

