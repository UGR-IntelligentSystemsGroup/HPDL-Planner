#include "stacknode.hh"
#include "function.hh"
#include "papi.hh"
#include "problem.hh"
#include "causalTable.hh"
#include "plan.hh"

#ifdef USE_AC3
#include "tcnm-ac3.hh"
#endif

#ifdef USE_PC2
#include "tcnm-pc2.hh"
#endif

#ifdef USE_PC2_CL
#include "tcnm-pc2-cl.hh"
#endif

struct PrintUndoElementln2
{
    ostream * os;
    PrintUndoElementln2(ostream * o) {os=o;};

    void operator()(const UndoElement * u) const {
	*os << "(uuu) ";
	if(u->isUndoChangeValue()) {
	    Function * f = (Function *)(((UndoChangeValue *)u)->target);
	    *os << "Changing value: ";
	    f->printL(os,0);
	}
	else
	    u->print(os);
	*os << endl;
    };
};

StackNode::StackNode(void) {
    offspring = 0; 
    task = -1; utable = 0; methods = 0; 
    mpos = -1; done = false; oldsize=0; tcnStackSize=-1;
    intervals = 0; tcnpos = 0; tl_tcnStackSize=-1; taskid=-1;
    backtrack = false; 
};

StackNode::StackNode(const VAgenda * age)
    :agenda(*age)
{offspring = 0; task = -1; utable = 0; methods = 0; 
    mpos = -1; done = false; oldsize=0; tl_tcnStackSize=-1;
    intervals = 0; tcnpos = 0; tcnStackSize=-1;taskid=-1;
    backtrack = false; 
};

// Constructor de copia
StackNode::StackNode(const StackNode * o)
    :lAdded(o->lAdded), lDeleted(o->lDeleted), agenda(o->agenda), explored(o->explored)
{
        oldsize = o->oldsize;

	intervals = 0;
	if(o->intervals)
	    intervals = new VIntervals(*(o->intervals));

	tcnpos = o->tcnpos;
	tl_tcnStackSize = o->tl_tcnStackSize;
	tcnStackSize = o->tcnStackSize;

	backtrack = o->backtrack;
        taskid = o->taskid;
        inmediate = o->inmediate;

	offspring = 0;
	if(o->offspring){
	    offspring = new vector<Task *>;
	    for_each(o->offspring->begin(),o->offspring->end(),CloneV<Task>(offspring));
	}

        task = o->task;
        unif = o->unif;

	utable = 0;
	if(o->utable)
	    utable =  new UnifierTable(o->utable);

        /*! Lista de m�todos no expandidos si task es una tarea compuesta */
	methods = 0;
	if(o->methods)
	    methods = new VMethods(*(o->methods));

        mpos = o->mpos;
        done = o->done;
	ttsize = o->ttsize;
};

void StackNode::undoEffects(void) {
    if(current_plan->FLAG_VERBOSE == 3){
        for_each(stateChanges.begin(),stateChanges.end(),PrintUndoElementln2(errflow));
    }
    for_each(stateChanges.begin(),stateChanges.end(),mem_fun(&UndoElement::undo));
};

void StackNode::undoApplyUnification(void)
{
    if(current_plan->FLAG_VERBOSE == 3) {
        *errflow << "(uuu) Undoing unification." << endl;
    }
    // antes de deshacer la unificaci�n me aseguro que la
    // tabla de v�nculos causales no quede inconsistente

//    const Unifier * uf = 0;
//    if(this->utable)
//	this->utable->getUnifierAt(this->unif);
//    const CLTable * tb =0;
//    if (uf)
//	tb = uf->getCLTable();
//    if(tb && !tb->empty()){
//	const Task * t  = tb->front().getConsumer();
//	causalTable.eraseCausalLinks(t);
//    }

    for_each(undoApply.begin(),undoApply.end(),mem_fun(&UndoElement::undo));
    undoApply.clear();
};

void StackNode::clearUndoApply(void) {
    for_each(undoApply.begin(),undoApply.end(),Delete<UndoElement>()); 
    undoApply.clear();
};

void StackNode::undoCLchanges(void) {
    if(current_plan->FLAG_VERBOSE == 3){
        *errflow << "(uuu) Undoing causal links." << endl;
    }

    for_each(undoCL.begin(),undoCL.end(),mem_fun(&UndoElement::undo));
    undoCL.clear();
};

void StackNode::clearUndoCL(void) 
{
    for_each(undoCL.begin(),undoCL.end(),Delete<UndoElement>()); 
    undoCL.clear();
};

void StackNode::clearUndoEffects(void) 
{
    for_each(stateChanges.begin(),stateChanges.end(),Delete<UndoElement>()); 
    stateChanges.clear();
};

void StackNode::undoTaskNetwork(TaskNetwork * tasknetwork, TPlan * plan)
{
    if(!lAdded.empty() && !lDeleted.empty()) {
	// Si lo �ltimo que a�adimos a la red de tareas fue una tarea primitiva
	// es necesario eliminarla del plan
//	VLinks::iterator i;
//	for(i=lAdded.begin();i!=lAdded.end();i++) {
//	    cerr << "[" << (*i).first << "," << "[" << (*i).second.first << "," << (*i).second.second << "]]" << endl;
//	}
//	for(i=lDeleted.begin();i!=lDeleted.end();i++) {
//	    cerr << "[" << (*i).first << "," << "[" << (*i).second.first << "," << (*i).second.second << "]]" << endl;
//	}
	//cerr << "UUUUUUUUUUUUUUUUUUUUUNNNNNNNNNNNNNNNNNNNNNNNNNDDDDDDDDDDDDDDDDDDDDDDDDDDDDOOOOOOOOOOOOOOOOOOO" << endl;
	//tasknetwork->printDebug(&cerr);

	tasknetwork->undoReplace(oldsize, &lAdded, &lDeleted);
	//cerr << "============================================================================================" << endl;
//	tasknetwork->printDebug(&cerr);
//	cerr << "UUUUUUUUUUUUUUUUUUUUUNNNNNNNNNNNNNNNNNNNNNNNNNDDDDDDDDDDDDDDDDDDDDDDDDDDDDOOOOOOOOOOOOOOOOOOO" << endl;
    }
    //else{
    //    cerr << "EMPTY??" << endl;
    //}
    undoPlan(plan,tasknetwork);
};

void StackNode::undoPlan(TPlan * plan, TaskNetwork * tasknetwork){
    while(!plan->empty() && (plan->back() >= tasknetwork->getNumOfNodes())) {
        plan->pop_back();
    }
};

void StackNode::clearUndoTaskNetwork(void)
{
    lAdded.clear();
    lDeleted.clear();
};

void StackNode::undo(TaskNetwork * tasknetwork, TPlan * plan) {
    undoCLchanges();
    clearUndoCL();
    undoEffects();
    clearUndoEffects();
    undoApplyUnification();
    clearUndoApply();
    undoTaskNetwork(tasknetwork,plan);
    clearUndoTaskNetwork();
};

void StackNode::undo(TaskNetwork * tasknetwork, TPlan * plan, STP * stp) {
    undo(tasknetwork,plan);
    undoSTP(stp);

};

void StackNode::undoSTP(STP * stp, int l)
{
    if(l == -1)
	l = tcnStackSize;

    if(l != -1){
	if(current_plan->FLAG_VERBOSE){
	    *errflow << "(uuu) Simple Temporal Network: ";
	};
	//cerr << "Restaurando: " << l << endl;
	//cerr << stp->getUndoLevels() << endl;
	while((int) l < (int) stp->getUndoLevels()){
	    //cerr << "Quitando nivel" << endl;
	    stp->UnSetLevel();
	    //cerr << "Nuevo tope: " << stp->getUndoLevels() << endl;
	}
	if(current_plan->FLAG_VERBOSE){
	    *errflow << (int)stp->getUndoLevels() << endl;
	};
    }
};

StackNode::~StackNode(void)
{
    if(offspring){
	vector<Task *>::iterator i, e = offspring->end();
	for(i=offspring->begin(); i!= e; i++){
	    if((*i)->isPrimitiveTask()){
		if(!((PrimitiveTask *)(*i))->getInline()){
		    delete (*i);
		    (*i) = 0;
		}
	    }
	    else {
		delete (*i);
		(*i) = 0;
	    }
	}
	delete offspring;
	offspring=0;
    } 
    //for_each(sAdded.begin(),sAdded.end(),Delete<Literal>());
    // for_each(sDeleted.begin(),sDeleted.end(),Delete<Literal>());
    if(utable)
	delete utable;
    if(methods){
	methods->clear();
	delete methods;
    }
    clearUndoApply(); 
    clearUndoTask();
    clearUndoTaskNetwork();
    clearUndoEffects();
};

void StackNode::clearUndoTask(void) 
{
    vector<VUndo *>::iterator i, e = undoTask.end();
    for(i=undoTask.begin(); i!= e; i++){
	if(*i){
	    for_each((*i)->begin(),(*i)->end(),Delete<UndoElement>());
	    delete (*i);
	    (*i)=0;
	}
    }
    undoTask.clear();
};

void StackNode::undoTaskSelection(int task) {
    if(!undoTask.empty() && undoTask[task]){
        for_each(undoTask[task]->begin(),undoTask[task]->end(),mem_fun(&UndoElement::undo));
    }
};


void StackNode::removeTask(int index) {
    vector<Task *>::iterator ite;
    ite = offspring->begin() + index;
    Task * tmp = (*ite);
    offspring->erase(ite);
    if(tmp->isPrimitiveTask()) {
        if(((PrimitiveTask *)tmp)->getInline()) {
            return;
        }
    }
    delete tmp;
}

void StackNode::restoreAllDP(void){
    if(!explored.empty())
	agenda.insert(agenda.end(),explored.begin(),explored.end());
    explored.clear();
    backtrack = false;
    if(offspring){
	vector<Task *>::iterator i, e = offspring->end();
	for(i=offspring->begin(); i!= e; i++){
	    if((*i)->isPrimitiveTask()){
		if(!((PrimitiveTask *)(*i))->getInline()){
		    delete (*i);
		    (*i) = 0;
		}
	    }
	    else {
		delete (*i);
		(*i) = 0;
	    }
	}
	delete offspring;
	offspring=0;
    } 
    //for_each(sAdded.begin(),sAdded.end(),Delete<Literal>());
    // for_each(sDeleted.begin(),sDeleted.end(),Delete<Literal>());
    if(utable)
	delete utable;
    utable = 0;
    if(methods){
	methods->clear();
	delete methods;
    }
    methods = 0;

    task = -1;
    mpos = -1;

    done = false; 
    tcnStackSize=-1;
    if(intervals)
	delete intervals;
    intervals = 0; 
    taskid=-1;
};


