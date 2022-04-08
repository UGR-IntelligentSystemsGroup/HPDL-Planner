#include "replan.hh"
#include "problem.hh"

bool RePlan::undoUntilDepth(int depth) {

//    StackNode * next=0;
//
//    int cd = p->stack.size();
//    // mientras no se alcance la profundidad pasada como argumento
//    while(cd >= depth){
//	// si entra otra vez en el bucle y queda algo por borrar
//	if(next){
//	    delete next;
//	    next = 0;
//	}
//	// si no quedan elementos en la pila ocurrió un error
//	if( p->stack.empty())
//	{
//	    p->plan.clear();
//	    cerr << "pila vacia" << endl;
//	    return false;
//	}
//	next = p->stack.back();
//	p->stack.pop_back();
//
//	// guardamos la información de la decisión tomada con anterioridad
//	OldDecision * od = new OldDecision(next,p->tasknetwork);
//	oldDecisions.push_back(od);
//    
//	// Deshacemos los cambios provocados en la etapa de planificación
//	if(!p->isTimed())
//	    next->undo(p->tasknetwork,&( p->plan));
//	else
//	    next->undo(p->tasknetwork,&( p->plan), p->stp);
//	cd--;
//    }
//    // Restauramos la profundidad actual, de forma que todas las
//    // decisiones puedan aplicarse desde un principio.
//    if(next){
//	StackNode * sn = new StackNode(next);
//	sn->restoreAllDP();
//	p->stack.push_back(sn);
//	delete next;
//    }
//    else
//	return false;
//    for_each(oldDecisions.rbegin(), oldDecisions.rend(),Print<OldDecision>(&cerr));
    return true;
};

bool RePlan::repairUnification(int depth, int uni){
//    maxDepth = depth;
//    seluni = uni;
//    deleteOldDecisions();
//    if(!undoUntilDepth(depth)){
//	abort();
//    }
//    cerr << "Terminé el undo" << endl;
//
////    StackNode * context = p->stack.back();
////    cerr << p->stack.size() << endl;
////    cerr << "He encontrado las siguientes unificaciones restantes:" << endl;
////    int k = 0;
////    unifierit ui, ue;
////    cerr << context->utable->countUnifiers() << endl;
////    ue = context->utable->getUnifierEnd();
////    for(ui = context->utable->getUnifierBegin(),ui++; ui!=ue; ui++,k++)
////    {
////	*errflow << "Unification [" << k << "]:" << endl;
////	(*ui)->print(&*errflow);
////	*errflow << endl;
////    }
////    cerr << "Escoge el índice de alguna de ellas: ";
////    cin >> seluni;
//
//
//    posOD = oldDecisions.size() -1;
//
//    if(! p->mainLoop()){
//	p->plan.clear();
//	return false;
//    }
//
//    p->printPlan(&cerr);

    return true;
};

void RePlan::deleteOldDecisions(void){
    for_each(oldDecisions.begin(),oldDecisions.end(),Delete<OldDecision>());
};

//int RePlan::selectUnification(State * state, const StackNode * sn, const TaskNetwork * tn, const TPlan * plan){
//    if((int)p->stack.size() < maxDepth)
//	return -2;
//    if(seluni != -1){
//	int s = seluni;
//	seluni = -1;
//	return s;
//    }
//    else
//	return Selector::selectUnification(state,sn,tn,plan);
//};

int RePlan::selectFromAgenda(State * state, const StackNode * sn, const TaskNetwork * tn, const TPlan * plan){
    // Mirar si el nodo por el que vamos coincide con el nodo del plan antiguo
    // si es así tratar de tomar la misma decisión que se tomo con anterioridad.
    // Si no es así tratar de analizar el árbol de búsqueda antiguo tratando de encontrar
    // decisiones parecidas a la que tenemos que tomar ahora.

//    current = p->stack.back();
//    tope = oldDecisions[posOD];
//    posOD--;
//
//    // Escoger la misma decisión que se tomó anteriormente
//    const Task * t = tope->task_selected;
//    const Task * t2 = 0;
//    int s = current->agenda.size();
//
//    for(int i=0; i<s; i++){
//	t2 = p->tasknetwork->getTask(current->agenda[i].first);
//	if(match(t,t2))
//	    return current->agenda[i].first;
//	}
//    cerr << "Estoy abortando en selectFromAgenda" << endl;
//    abort();
    return 0;
};

int RePlan::selectTask(State * state, const StackNode * sn, const TaskNetwork * tn, const TPlan * plan){

//    const Task * t = p->tasknetwork->getTask(current->taskid);
//    const Task * th = tope->task_selected; 
//
//    if(match(t,th)){
//	int s = current->offspring->size(); 
//	for(int i =0; i<s; i++){
//	    if((*(current->offspring))[i]->getMetaId() == th->getMetaId())
//		return i;
//	}
//    }
//    cerr << "Estoy abortando en selectTask" << endl;
//    abort();
    return 0;
};

int RePlan::selectMethod(State * state, const StackNode * sn, const TaskNetwork * tn, const TPlan * plan){
//    const Task * t = p->tasknetwork->getTask(current->taskid);
//    const Task * th = tope->task_selected; 
//
//    if(match(t,th)){
//	int s = current->methods->size(); 
//	for(int i =0; i<s; i++){
//	    if((*(current->methods))[i]->getMetaId() == tope->method_id)
//		return i;
//	}
//    }
//    cerr << "Estoy abortando en selectMethod" << endl;
//    abort();
    return 0;
};

int RePlan::selectUnification(State * state, const StackNode * sn, const TaskNetwork * tn, const TPlan * plan){

//    const Task * t = p->tasknetwork->getTask(current->taskid);
//    const Task * th = tope->task_selected; 
//    unifierit i, e;
//    int j;
//
//    if(match(t,th)){
//	if(tope->u){
//	    //cerr << current->utable->countUnifiers() << endl;
//	    //tope->u->print(&cerr);
//	    //cerr << "--------------------------------" << endl;
//	    e = current->utable->getUnifierEnd();
//	    for(i= current->utable->getUnifierBegin(),j=0; i != e; i++,j++){
//		//(*i)->print(&cerr);
//		if((*i)->equal(tope->u))
//		    return j;
//	    }
//	}
//    }
//    cerr << "Estoy abortando en selectUnification" << endl;
//    abort();
    return 0;
};

double RePlan::compareAgenda(StackNode * a, StackNode * b){
//    double valoracion = 0;
//    bool encontrado;
//
//    int i,j,ej, ei = a->explored.size();
//
//    // si las listas tienen el mismo tamaño le damos +2
//    if(a->explored.size() == b->explored.size() && a->agenda.size() == b->agenda.size())
//	valoracion += 2;
//
//    // si no son iguales pero su suma si le damos +1
//    else if(a->explored.size() + a->agenda.size() == b->explored.size() + b->agenda.size())
//	valoracion += 1;
//
//
//    bool first = true;
//    int lastIndex = -1;
//    // recorrer la lista de explorados de a
//    for(i=0;i<ei;i++){
//	// recorrer la lista de explorados de b
//	ej = b->explored.size();
//	encontrado = false;
//	for(j=0;j<ej && !encontrado;j++){
//	    if(eqpair(a->explored[i],b->explored[j])){
//		if(first || lastIndex +1 != j){
//		    // +1 por ser iguales +1 por estar en la misma lista
//		    valoracion += 2;
//		    first = false;
//		}
//		else
//		    // +1 por ser iguales +1 por estar en la misma lista +1 por ser un acierto
//		    // consecutivo
//		    valoracion +=3;
//		lastIndex = j;
//		encontrado = true;
//	    }
//	}
//	// recorrer la lista de agenda de b
//	ej = b->agenda.size();
//	for(j=0;j<ej && !encontrado;j++){
//	    if(eqpair(a->explored[i],b->agenda[j])){
//		if(first || lastIndex +1 != j){
//		    // +1 por ser iguales 
//		    valoracion++;
//		    first = false;
//		}
//		else
//		    // +1 por ser iguales +1 por ser un acierto consecutivo
//		    valoracion +=2;
//		lastIndex = j;
//		encontrado = true;
//	    }
//	}
//	if(!encontrado){
//	    lastIndex = -1;
//	    first = true;
//	}
//    }
//
//    first = true;
//    lastIndex = -1;
//    ei = a->agenda.size();
//    // recorrer la lista de agenda de a
//    for(i=0;i<ei;i++){
//	// recorrer la lista de agenda de b
//	ej = b->agenda.size();
//	encontrado = false;
//	for(j=0;j<ej && !encontrado;j++){
//	    if(eqpair(a->agenda[i],b->agenda[j])){
//		if(first || lastIndex +1 != j){
//		    // +1 por ser iguales +1 por estar en la misma lista
//		    valoracion += 2;
//		    first = false;
//		}
//		else
//		    // +1 por ser iguales +1 por estar en la misma lista +1 por ser un acierto
//		    // consecutivo
//		    valoracion +=3;
//		lastIndex = j;
//		encontrado = true;
//	    }
//	}
//	// recorrer la lista de explorados de b
//	ej = b->explored.size();
//	int last = ej-1;
//	for(j=0;j<ej && !encontrado;j++){
//	    if(eqpair(a->agenda[i],b->explored[j])){
//		if(first || lastIndex +1 != j){
//		    // +1 por ser iguales 
//		    valoracion++;
//		    first = false;
//		}
//		else
//		    // +1 por ser iguales +1 por ser un acierto consecutivo
//		    valoracion +=2;
//		lastIndex = j;
//		encontrado = true;
//		if(j == last){ 
//		    // si el último que se exploró en b está sin explorar en la agenda de pendientes
//		    // de a damos +5 porque es el siguiente que trataremos de explorar
//		    valoracion += 5;
//		}
//	    }
//	}
//	if(!encontrado){
//	    lastIndex = -1;
//	    first = true;
//	}
//    }
//
//    return valoracion/(7.0 + (a->agenda.size() + a->explored.size())*3.0);
    return 0;
};

//gislist * RePlan::searchNode(StackNode * n, vector<StackNode *> * l, int from){
//    gislist * candidates = new gislist;
//    
//    // sacar la tarea de la red de tareas
//    const Task * t = p->tasknetwork->getTask(n->taskid);
//    const TaskHeader * th; 
//
//    int i, s = l->size();
//    for(i=from;i<s;i++){
//	th = (*l)[i]->task_selected;
//	if(match(t,th))
//	    candidates->push_front(i);
//    }
//    return candidates;
//};

bool RePlan::match(const Task * t , const Task * th){
//    if(t->getId() == th->getId()){
//	// si unifican ya tenemos un candidato
//	if(unify(t->getParameters(),th->getParameters())){
//	    return true;
//	}
//    }
    return false;
};
