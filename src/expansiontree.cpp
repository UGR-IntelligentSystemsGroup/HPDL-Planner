#include "expansiontree.hh"
#include "plan.hh"

ExpansionTree::ExpansionTree(const TPlan * plan, const TaskNetwork * tn){
    TPlan::const_iterator ip, ep = plan->end();
    const Task * t;

    tasklistcit i, e= tn->getEndTask();
    // para cada una de las tareas primitivas que hay en el plan
    for(ip=plan->begin(); ip!=ep; ip++){
	t = tn->getTask(*ip);
	// la insertamos en la tabla hash que mantiene las tareas
	// del árbol de expansión.
	tree.insert(make_pair((long)t,new NodeTaskTree(t)));

	bool encontrado = false;

	// recorremos hacia arriba la estrutura del árbol hasta llegar a la
	// raíz. Comprobamos que las tareas padre que hay por encima no 
	// hayan sido previamente escritas por otra tarea primitiva.
	while(t->getParent() && !encontrado){
	    t = t->getParent();
	    if(tree.find((long) t) == tree.end()){
		tree.insert(make_pair((long)t,new NodeTaskTree(t)));
	    }
	    else
		encontrado = true;
	}
    }

    // durante la segunda vuelta añadimos los hijos, y las tareas raíz
    ttdite id,jd,ed = tree.end();
    for(id=tree.begin(); id!=ed; id++){
	t = (*id).second->task->getParent();
	if(t == 0)
	    roots.push_back((*id).second->task);
	else{
	    jd = tree.find((long)t);
	    if(jd!=ed)
		(*jd).second->children.push_back((*id).second->task);
	}
    }
};

ExpansionTree::~ExpansionTree(void){
    ttdite i,e = tree.end();
    for(i=tree.begin(); i!=e; i++){
	delete (*i).second;
	(*i).second = 0;
    }
};

const NodeTaskTree * ExpansionTree::getNode(int k) const{
    if(current_plan && current_plan->hasPlan()){
	const Task * t = current_plan->getTaskNetwork()->getTask(k);
	ttdcite i = tree.find((long) t);
	if( i == tree.end())
	    return 0;
	else
	    return (*i).second;
    }
    return 0;
};

