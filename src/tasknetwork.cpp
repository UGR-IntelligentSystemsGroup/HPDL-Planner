#include "tasknetwork.hh"
#include <string>
#include <iostream>
#include <sstream>
#include "domain.hh"
#include "papi.hh"

//Variable para controlar la impresion
extern bool PRINTING_TASKSNETWORK;

bool LAST_TASK_PRINT_SEC = false;

TaskNetwork::TaskNetwork(void) {
    // crear los 0s s�mbolos de inicio y de fin
    succ.push_back(new vector<int> );
    pred.push_back(new vector<int> );
}

TaskNetwork::TaskNetwork(Task * t) {
    succ.push_back(new vector<int> );
    pred.push_back(new vector<int> );
    tasklist.push_back(t);
    invalid.push_back(false);
    inmediate.push_back(false);
    backtracking.push_back(false);
    succ.push_back(new vector<int> );
    pred.push_back(new vector<int> );
    // la sucesora de la 0 es la 1 la nueva tarea
    // insertada
    succ[0]->push_back(1);
    succ[1]->push_back(0);
    pred[1]->push_back(0);
    pred[0]->push_back(1);
}

TaskNetwork::TaskNetwork(vector<Task *> * vt, bool parallel) {
    if (parallel) {
        // se crean las estructuras para mantener el 0
        succ.push_back(new vector<int> );
        pred.push_back(new vector<int> );

        // se a�aden las tareas desde el vector.
        for_each(vt->begin(), vt->end(), AddV<Task> (&tasklist));

        // como son paralelas todas parten del 0
        // y llegan hasta el 0
        int s = tasklist.size();
        for (int i = 1; i <= s; i++) {
            succ[0]->push_back(i);
            pred[0]->push_back(i);
            // crear las estructuras de datos
            succ.push_back(new vector<int> );
            pred.push_back(new vector<int> );
            succ[i]->push_back(0);
            pred[i]->push_back(0);
        }
        invalid.insert(invalid.end(), s, false);
        inmediate.insert(inmediate.end(), s, false);
        backtracking.insert(backtracking.end(), s, false);
    } else {
        // en el caso de que sean secuenciales unas se colocan detr�s de
        // las otras
        for_each(vt->begin(), vt->end(), AddV<Task> (&tasklist));
        int s = tasklist.size();
        for (int i = 0; i <= s; i++) {
            succ.push_back(new vector<int> );
            pred.push_back(new vector<int> );
            if (i == s)
                succ[i]->push_back(0);
            else
                succ[i]->push_back(i + 1);
            if (!i)
                pred[i]->push_back(s);
            else
                pred[i]->push_back(i - 1);
        }
        invalid.insert(invalid.end(), s, false);
        inmediate.insert(inmediate.end(), s, false);
        backtracking.insert(backtracking.end(), s, false);
    }
    /*cerr << "Sucesores: " << endl;
    printTable(&succ);
    cerr << "Predecesores: " << endl;
    printTable(&pred);*/
}

struct CloneVINT {
    vector<vector<int> *> * v;
    CloneVINT(vector<vector<int> *> * par) {
        v = par;
    }
    ;
    void operator()(const vector<int> * aux) {
        if (aux)
            v->push_back(new vector<int> (*aux));
        else
            v->push_back(new vector<int> );
    }
    ;
};

TaskNetwork::TaskNetwork(const TaskNetwork * other) :
    inmediate(other->inmediate), invalid(other->invalid), backtracking(
            other->backtracking) {
    for_each(other->tasklist.begin(), other->tasklist.end(), CloneV<Task> (
            &tasklist));
    for_each(other->succ.begin(), other->succ.end(), CloneVINT(&succ));
    for_each(other->pred.begin(), other->pred.end(), CloneVINT(&pred));
    for_each(other->constraintref.begin(), other->constraintref.end(),
            CloneVINT(&constraintref));
    // ahora las restricciones
    for_each(other->tconstraints.begin(), other->tconstraints.end(), CloneVCT(
            &tconstraints));
}
;

struct UnaryAdd {
    int constant;
    UnaryAdd(int c) {
        constant = c;
    }
    ;

    int operator()(int & ref) {
        return ref + constant;
    }
    ;
};

void TaskNetwork::join(TaskNetwork * other) {
    /*cerr << "JOINING-------------------------" << endl;
    this->printDebug(&cerr);
    cerr << "--------------------------------" << endl;
    other->printDebug(&cerr);
    cerr << "--------------------------------" << endl;*/
    // unimos dos redes de tareas de forma secuencial
    // El punto de fin de this, se convierte en el punto de inicio de other.
    // El punto de fin de this ser� el nuevo punto de fin.

    // guardo el tama�o antes de hacer la uni�n.
    int s = tasklist.size();
    // copio la lista de tareas de other en this.
    for_each(other->tasklist.begin(), other->tasklist.end(), AddV<Task> (
            &tasklist));

    // Copiamos reajustando los �ndices los predecesores y los
    // sucesores de other en this. Exceptuando la marca de tarea inicial/final.
    intvit i, e;
    vector<vector<int> *>::iterator j, je;

    je = other->succ.end();
    for (j = other->succ.begin() + 1; j != je; j++) {
        succ.push_back(new vector<int> );
        e = (*j)->end();
        for (i = (*j)->begin(); i != e; i++)
            if ((*i) != 0)
                succ.back()->push_back((*i) + s);
            else
                succ.back()->push_back(0);
    }

    je = other->pred.end();
    for (j = other->pred.begin() + 1; j != je; j++) {
        pred.push_back(new vector<int> );
        e = (*j)->end();
        for (i = (*j)->begin(); i != e; i++)
            if ((*i) != 0)
                pred.back()->push_back((*i) + s);
            else
                pred.back()->push_back(0);
    }

    // Los predecesores en this de 0 (la marca final) van a tener ahora como
    // sucesores las tareas iniciales de other (succesores de 0 marca inicial)
    // (El 0 actua tanto como marca de inicio como de f�n interpretandose seg�n
    // hablemos de los predecesores o de los sucesores).
    //
    // Tambi�n ocurre a la inversa los predecesores de las tareas iniciales de
    // other ahora son las tareas finales de this.
    intvit k, oe;
    e = pred[0]->end();
    oe = other->succ[0]->end();
    for (k = other->succ[0]->begin(); k != oe; k++) {
        pred[(*k) + s]->clear();
    }

    for (i = pred[0]->begin(); i != e; i++) {
        // preparamos las e.d, borramos los enlaces anteriores
        succ[(*i)]->clear();

        // realizamos los nuevos enlaces
        for (k = other->succ[0]->begin(); k != oe; k++) {
            succ[(*i)]->push_back((*k) + s);
            pred[(*k) + s]->push_back((*i));
        }
    }

    // Los sucesores de la tarea inicial siguen siendo los mismos
    // pero no los predecesores de la tarea final
    pred[0]->clear();
    e = other->pred[0]->end();
    for (i = other->pred[0]->begin(); i != e; i++) {
        pred[0]->push_back((*i) + s);
    }

    // copiamos otras estructuras auxiliares, la lista de invalidos
    // y la lista de inmediates
    invalid.insert(invalid.end(), other->invalid.begin(), other->invalid.end());
    inmediate.insert(inmediate.end(), other->inmediate.begin(),
            other->inmediate.end());

    backtracking.insert(backtracking.end(), other->backtracking.begin(),
            other->backtracking.end());
    // limpiamos las estructuras de datos de other, para evitar que se borren
    // estructuras de datos que yo estoy utilizando
    other->tasklist.clear();
    for_each(other->succ.begin(), other->succ.end(), DeleteVector<int> ());
    other->succ.clear();
    for_each(other->pred.begin(), other->pred.end(), DeleteVector<int> ());
    other->pred.clear();

    int oldsize = other->constraintref.size();
    constraintref.insert(constraintref.end(), other->constraintref.begin(),
            other->constraintref.end());
    other->constraintref.clear();
    for (int i = 0; i < oldsize; i++)
        transform(other->constraintref[i]->begin(),
                other->constraintref[i]->end(),
                other->constraintref[i]->begin(), UnaryAdd(s));
    tconstraints.insert(tconstraints.end(), other->tconstraints.begin(),
            other->tconstraints.end());
    other->tconstraints.clear();

    //this->printDebug(&cerr);
}

void TaskNetwork::merge(TaskNetwork * other) {
    /*cerr << "MERGING-------------------------" << endl;
    this->printDebug(&cerr);
    cerr << "--------------------------------" << endl;
    other->printDebug(&cerr);
    cerr << "--------------------------------" << endl;*/
    intvit i, e;
    int s = tasklist.size();
    for_each(other->tasklist.begin(), other->tasklist.end(), AddV<Task> (
            &tasklist));

    // Hay que unir dos redes de tareas de forma paralela.
    // Por lo tanto hay que unir sus puntos de inicio y de fin
    // marcados por la tarea con id 0
    //
    // Copiamos, reajustando los �ndices, los succesores de la tarea inicial
    // de la tarea con la que nos vamos a unir
    e = other->succ[0]->end();
    for (i = other->succ[0]->begin(); i != e; i++)
        succ[0]->push_back(s + *i);

    // y los predecesores de la tarea final
    e = other->pred[0]->end();
    for (i = other->pred[0]->begin(); i != e; i++)
        pred[0]->push_back(s + *i);

    // ahora copiamos el resto de enlaces reajustando
    // los �ndices
    vector<vector<int> *>::iterator j, je;

    je = other->succ.end();
    for (j = other->succ.begin() + 1; j != je; j++) {
        succ.push_back(new vector<int> );
        e = (*j)->end();
        for (i = (*j)->begin(); i != e; i++)
            if ((*i) != 0)
                succ.back()->push_back((*i) + s);
            else
                succ.back()->push_back(0);
    }

    je = other->pred.end();
    for (j = other->pred.begin() + 1; j != je; j++) {
        pred.push_back(new vector<int> );
        e = (*j)->end();
        for (i = (*j)->begin(); i != e; i++)
            if ((*i) != 0)
                pred.back()->push_back((*i) + s);
            else
                pred.back()->push_back(0);
    }

    // copiamos otras estructuras auxiliares, la lista de invalidos
    // y la lista de inmediates
    invalid.insert(invalid.end(), other->invalid.begin(), other->invalid.end());
    inmediate.insert(inmediate.end(), other->inmediate.begin(),
            other->inmediate.end());

    backtracking.insert(backtracking.end(), other->backtracking.begin(),
            other->backtracking.end());

    // limpiamos las estructuras de datos de other, para evitar que se borren
    // estructuras de datos que yo estoy utilizando
    other->tasklist.clear();
    for_each(other->succ.begin(), other->succ.end(), DeleteVector<int> ());
    other->succ.clear();
    for_each(other->pred.begin(), other->pred.end(), DeleteVector<int> ());
    other->pred.clear();

    int oldsize = other->constraintref.size();
    constraintref.insert(constraintref.end(), other->constraintref.begin(),
            other->constraintref.end());
    other->constraintref.clear();
    for (int i = 0; i < oldsize; i++)
        transform(other->constraintref[i]->begin(),
                other->constraintref[i]->end(),
                other->constraintref[i]->begin(), UnaryAdd(s));
    tconstraints.insert(tconstraints.end(), other->tconstraints.begin(),
            other->tconstraints.end());
    other->tconstraints.clear();

    //this->printDebug(&cerr);
}

TaskNetwork::~TaskNetwork(void) {
    for_each(tasklist.begin(), tasklist.end(), Delete<Task> ());
    for_each(succ.begin(), succ.end(), DeleteVector<int> ());
    for_each(pred.begin(), pred.end(), DeleteVector<int> ());
    for_each(constraintref.begin(), constraintref.end(), DeleteVector<int> ());
    for_each(tconstraints.begin(), tconstraints.end(), DeleteVCT());
}

int TaskNetwork::addTask(Task *t) {
    assert(t != 0);

    tasklist.push_back(t);

    invalid.push_back(false);
    inmediate.push_back(false);
    backtracking.push_back(false);
    succ.push_back(new vector<int> );
    pred.push_back(new vector<int> );
    return tasklist.size();
}

void TaskNetwork::fixTaskNetwork(void) {
    intvvit i, e;
    int c = 1;
    e = pred.end();
    for (i = pred.begin() + 1; i != e; i++, c++)
        if ((*i)->empty()) {
            succ[0]->push_back(c);
            (*i)->push_back(0);
        }

    c = 1;
    e = succ.end();
    for (i = succ.begin() + 1; i != e; i++, c++)
        if ((*i)->empty()) {
            pred[0]->push_back(c);
            (*i)->push_back(0);
        }
}

void TaskNetwork::replacePrimitive(int indexTask, PrimitiveTask * task,
        vector<pair<int, bool> > * agenda,
        vector<pair<bool, pair<int, int> > > * added,
        vector<pair<bool, pair<int, int> > > * deleted) {

    assert(added != 0);
    assert(deleted != 0);
    assert(indexTask < getNumOfNodes() && indexTask >= 0);

    //cerr << "Profundidad:" << pred.size() << endl;
    //cerr << "Primitiva AAAAAAAAAAAAAAAAANNNNNNNNNNNNNNNNNNNNNNNNNTTTTTTTTTTTTTTTTTTTTTTTTTTEEEEEEEEEEEEEEEESSSSSSSSSSSSSS" << endl;
    //printDebug(&cerr);
    //comprobarEstructura();
    //cerr << "Sustituyendo: " << indexTask +1 << " por primitiva."<< endl;
    //task->printHead(&cerr);

    // se inserta la nueva tarea y se abre espacio para la estructura de datos
    tasklist.push_back(task);

    invalid.push_back(false);
    inmediate.push_back(false);
    backtracking.push_back(false);
    succ.push_back(new vector<int> );
    pred.push_back(new vector<int> );
    //cerr << "Invalidando:" << indexTask << endl;
    invalid[indexTask] = true;

    indexTask++;
    int offset = succ.size() - 1;
    vector<int> * v = succ[offset];
    intvit i, j, ee;
    intvit e = succ[indexTask]->end();

    for (i = succ[indexTask]->begin(); i != e; i++) {
        // se copia en los sucesores de la nueva entrada
        // los succesores de indexTask
        v->push_back(*i);
        ee = pred[(*i)]->end();
        // sustituimos las aparaciones de indexTask en los
        // predecesores de los sucesores de indexTask por
        // la nueva entrada
        for (j = pred[(*i)]->begin(); j != ee; j++)
            if ((*j) == indexTask) {
                (*j) = offset;
                added->push_back(make_pair(true, make_pair((*i), offset)));
                //cerr << "Deleted:" << " true " << (*i) << " " <<  offset << endl;
                deleted->push_back(make_pair(true, make_pair((*i), indexTask)));
                //cerr << "Deleted:" << " true " << (*i) << " " << indexTask << endl;
                break;
            }
    }

    // el proceso sim�trico
    v = pred[offset];
    e = pred[indexTask]->end();
    for (i = pred[indexTask]->begin(); i != e; i++) {
        v->push_back(*i);
        ee = succ[(*i)]->end();
        for (j = succ[(*i)]->begin(); j != ee; j++)
            if ((*j) == indexTask) {
                (*j) = offset;
                added->push_back(make_pair(false, make_pair((*i), offset)));
                //cerr << "Added:" << " false " << (*i) << " " << offset << endl;
                deleted->push_back(make_pair(false, make_pair((*i), indexTask)));
                //cerr << "Deleted:" << " false " << (*i) << " " << indexTask << endl;
                break;
            }
    }

    // los sucesores de la tarea primitiva
    // pasan a ser nuevos objetivos a resolver. Se introducen en la agenda.

    intvrcit ai, ae;

    ae = getSuccREnd(offset);
    for (ai = getSuccRBegin(offset); ai != ae; ai++) {
        if ((*ai) != 0)
            agenda->push_back(make_pair((*ai) - 1, backtracking[(*ai) - 1]));
    }

    //cerr << "Profundidad:" << pred.size() << endl;
    //cerr << "Primitiva DEEEEEEEEEEEEEEEEEEEEESSSSSSSSSSSSSSSSSSSSSSSSPPPPPPPPPPPPPPPPPPPPUUUUUUUUUUESSSSSSSSSSSSS" << endl;
    //printDebug(&cerr);
    //comprobarEstructura();
}

void TaskNetwork::replaceTN(int indexTask, TaskNetwork * newTN, vector<pair<
        int, bool> > * agenda, vector<pair<bool, pair<int, int> > > * added,
        vector<pair<bool, pair<int, int> > > * deleted) {
    assert(indexTask < getNumOfNodes() && indexTask >= 0);

    tasklistcit e = newTN->getEndTask();
    tasklistcit i;

    // Se a�aden las nuevas tareas, observar que no se clonan.
    // ESTA FUNCION ESTA PENSADA PARA APLICARSE A LA RED DE TAREAS QUE
    // REPRESENTA EL PLAN, NO PARA LAS REDES DE TAREAS CONTENIDAS EN
    // LOS METODOS.
    int oldsize = tasklist.size();

    //cerr << "Profundidad:" << pred.size() << endl;
    //cerr << "AAAAAAAAAAAAAAAAANNNNNNNNNNNNNNNNNNNNNNNNNTTTTTTTTTTTTTTTTTTTTTTTTTTEEEEEEEEEEEEEEEESSSSSSSSSSSSSS" << endl;
    //printDebug(&cerr);
    //comprobarEstructura();
    //cerr << "Sustituyendo: " << indexTask << " por red de tareas."<< endl;
    //newTN->printDebug(&cerr);

    //cerr << "Sustituyendo: " << indexTask << endl;
    //  cerr << "Sucesores NewTN: " << endl;
    //  printTable(&(newTN->succ));

    //cerr << "Sucesores: " << endl;
    //  printTable(&succ);


    for (i = newTN->getBeginTask(); i != e; i++) {
        tasklist.push_back(*i);
        invalid.push_back(false);
        succ.push_back(new vector<int> );
        pred.push_back(new vector<int> );
        //inmediate.push_back(false);
        //parallelizable.push_back(true);
    }

    inmediate.insert(inmediate.end(), newTN->inmediate.begin(),
            newTN->inmediate.end());
    int btsize = backtracking.size();
    backtracking.insert(backtracking.end(), newTN->backtracking.begin(),
            newTN->backtracking.end());
    if (backtracking[indexTask])
        fill(backtracking.begin() + btsize, backtracking.end(), true);

    // copiar las tablas de newTN, se supone que en newTN no hay inv�lidos
    // por lo que no los tengo en cuenta. No se copia la posici�n 0. Se
    // hace renombrado de �ndices.

    intvit ite, ee, ee2, ite2;
    vector<int> * v;
    int s = newTN->getNumOfNodes();
    int offset = oldsize;
    for (int c = 1; c <= s; c++) {
        ee = newTN->getPredEnd(c);
        v = pred[c + offset];
        for (ite = newTN->getPredBegin(c); ite != ee; ite++) {
            if ((*ite) != 0)
                v->push_back((*ite) + offset);
        }

        ee = newTN->getSuccEnd(c);
        v = succ[c + offset];
        for (ite = newTN->getSuccBegin(c); ite != ee; ite++) {
            if ((*ite) != 0)
                v->push_back((*ite) + offset);
        }
    }

    invalid[indexTask] = true;
    //cerr << "Invalidando:" << indexTask << endl;
    indexTask++;

    // los sucerosres de la tarea inicial (0) en NewTN
    // pasan a ser nuevos objetivos a resolver. Se introducen en la agenda.

    intvrcit ai, ae;

    ae = newTN->getSuccREnd(0);
    for (ai = newTN->getSuccRBegin(0); ai != ae; ai++) {
        agenda->push_back(make_pair((*ai) + offset - 1, backtracking[(*ai)
                + offset - 1]));
    }

    /*if(parallelizable[indexTask]){
    intvcit
    parallelizable[(*ai) + offset -1] = true;
    }*/
    /*cerr << "Sucesores: " << endl;
    printTable(&succ);*/

    // los predecesores de las primeras tareas de la red NewTN
    // son los predecesores de la tarea que vamos a sustituir
    ee2 = newTN->getSuccEnd(0);
    for (ite2 = newTN->getSuccBegin(0); ite2 != ee2; ite2++) {
        v = pred[(*ite2) + offset];
        ee = pred[indexTask]->end();
        for (ite = pred[indexTask]->begin(); ite != ee; ite++)
            v->push_back((*ite));
    }

    // el sim�trico
    ee2 = newTN->getPredEnd(0);
    for (ite2 = newTN->getPredBegin(0); ite2 != ee2; ite2++) {
        v = succ[(*ite2) + offset];
        ee = succ[indexTask]->end();
        for (ite = succ[indexTask]->begin(); ite != ee; ite++)
            v->push_back((*ite));
    }

    /*cerr << "Sucesores: " << endl;
    printTable(&succ);*/

    //Borrar indexTask en las listas donde aparezca y sustituirlos
    //por los succesores y predecesores de 0 de newTN donde corresponda
    //(acci�n de insertar la red e invalidar indexTask

    ee = succ[indexTask]->end();
    for (ite = succ[indexTask]->begin(); ite != ee; ite++) {
        v = pred[(*ite)];
        ee2 = v->end();
        for (ite2 = v->begin(); ite2 != ee2; ite2++) {
            if ((*ite2) == indexTask) {
                deleted->push_back(make_pair(true, make_pair((*ite), (*ite2))));
                //cerr << "Deleted:" << " true " << (*ite) << " " << (*ite2) << endl;
                v->erase(ite2);
                break;
            }
        }

        ee2 = newTN->getPredEnd(0);
        for (ite2 = newTN->getPredBegin(0); ite2 != ee2; ite2++) {
            added->push_back(make_pair(true,
                    make_pair((*ite), (*ite2) + offset)));
            //cerr << "Added:" << " true " << (*ite) << " " << (*ite2) + offset << endl;
            v->push_back((*ite2) + offset);
        }
    }

    // el sim�trico
    ee = pred[indexTask]->end();
    for (ite = pred[indexTask]->begin(); ite != ee; ite++) {
        v = succ[(*ite)];
        ee2 = v->end();
        for (ite2 = v->begin(); ite2 != ee2; ite2++) {
            if ((*ite2) == indexTask) {
                deleted->push_back(make_pair(false, make_pair((*ite), (*ite2))));
                //cerr << "Deleted:" << " false " << (*ite) << " " << (*ite2) << endl;
                v->erase(ite2);
                break;
            }
        }

        ee2 = newTN->getSuccEnd(0);
        for (ite2 = newTN->getSuccBegin(0); ite2 != ee2; ite2++) {
            added->push_back(make_pair(false, make_pair((*ite), (*ite2)
                    + offset)));
            //cerr << "Added:" << "false " << (*ite) << " " << (*ite2) + offset << endl;
            v->push_back((*ite2) + offset);
        }
    }
    /*cerr << "Sucesores: " << endl;
    printTable(&succ);*/

    //cerr << "Profundidad:" << pred.size() << endl;
    //cerr << "DEEEEEEEEEEEEEEEEEEEEESSSSSSSSSSSSSSSSSSSSSSSSPPPPPPPPPPPPPPPPPPPPUUUUUUUUUUESSSSSSSSSSSSS" << endl;
    //printDebug(&cerr);
    //comprobarEstructura();

    /*    for_each(other->constraintref.begin(),other->constraintref.end(),CloneVINT(&constraintref));
    // ahora las restricciones
    for_each(other->tconstraints.begin(),other->tconstraitns.end(),CloneVCT(&tconstraints));
    for(int i=0;i<oldsize;i++)
    for_each(constraintref[i]->begin(),constraintref[i]->end(),plus<int>(oldsize));*/
}

void TaskNetwork::restoreTo(int n) {
    tasklist.erase(tasklist.begin() + n, tasklist.end());
    invalid.erase(invalid.begin() + n, invalid.end());
    inmediate.erase(inmediate.begin() + n, inmediate.end());
    backtracking.erase(backtracking.begin() + n, backtracking.end());
    for_each(succ.begin() + (n + 1), succ.end(), DeleteVector<int> ());
    succ.erase(succ.begin() + (n + 1), succ.end());
    for_each(pred.begin() + (n + 1), pred.end(), DeleteVector<int> ());
    pred.erase(pred.begin() + (n + 1), pred.end());
}

void TaskNetwork::undoReplace(int oldsize,
        vector<pair<bool, pair<int, int> > > * lAdded, vector<pair<bool, pair<
                int, int> > > * lDeleted) {
    //cerr << "Profundidad:" << pred.size() << endl;
    //cerr << "arriba" << endl;
    //printDebug(&cerr);
    //comprobarEstructura();

    int actualsize = getNumOfNodes();
    restoreTo(oldsize);
    /*    while(actualsize > oldsize)
    {
    tasklist.pop_back();
    delete (succ.back());
    succ.pop_back();
    delete (pred.back());
    pred.pop_back();
    inmediate.pop_back();
    parallelizable.pop_back();
    invalid.pop_back();
    actualsize--;
    }*/

    actualsize = getNumOfNodes();

    // los a�adidos anterirmente son ahora eliminados.
    vector<pair<bool, pair<int, int> > >::const_iterator i, e;
    vector<int>::iterator j, k;

    e = lAdded->end();
    for (i = lAdded->begin(); i != e; i++) {
        //cerr << "add:[" << (*i).first << ",[" << (*i).second.first << "," << (*i).second.second << "]]" << endl;
        if ((*i).first) {
            if (((*i).second.first) <= actualsize) {
                k = pred[(*i).second.first]->end();
                for (j = pred[(*i).second.first]->begin(); j != k; j++)
                    if ((*j) == (*i).second.second) {
                        pred[(*i).second.first]->erase(j);
                        break;
                    }
            }
        } else {
            if (((*i).second.first) <= actualsize) {
                k = succ[(*i).second.first]->end();
                for (j = succ[(*i).second.first]->begin(); j != k; j++)
                    if ((*j) == (*i).second.second) {
                        succ[(*i).second.first]->erase(j);
                        break;
                    }
            }
        }

    }

    // los eliminados anteriormente ahora son a�adidos
    e = lDeleted->end();
    for (i = lDeleted->begin(); i != e; i++) {
        //cerr << "del:[" << (*i).first << ",[" << (*i).second.first << "," << (*i).second.second << "]]" << endl;
        if ((*i).first)
            pred[(*i).second.first]->push_back((*i).second.second);
        else
            succ[(*i).second.first]->push_back((*i).second.second);
    }

    assert(lDeleted->size() > 0);
    e = lDeleted->begin();
    invalid[(*e).second.second - 1] = false;
    //cerr << "Revalidando:" << (*e).second.second -1 << endl;

    //cerr << "Profundidad:" << pred.size() << endl;
    //cerr << "abajo" << endl;
    //printDebug(&cerr);
    //comprobarEstructura();
}
;

void TaskNetwork::print(ostream * out, int nindent) {

    int i;
    bool PRINT_NUMBERTYPE2, PRINT_OBJECTTYPE2, PRINT_DEFINEDTYPES2;

    PRINTING_TASKSNETWORK = true;

    //Inicializamos las variables globales necesarias para la impresion
    writed = new bool[succ.size()];
    for (i = 0; i < (int) succ.size(); i++)
        writed[i] = false;

    rightKey = new bool[succ.size() + 1];
    for (i = 0; i < (int) succ.size() + 1; i++)
        rightKey[i] = false;

    leftKey = new bool[succ.size() + 1];
    for (i = 0; i < (int) succ.size() + 1; i++)
        leftKey[i] = false;

    // cerr << "Sucesores: " << endl;
    // printTable(&(succ));

    // cerr << "Predecesores: " << endl;
    // printTable(&pred);

    string stabs(nindent, ' ');

    //Guardamos los valores antiguos
    PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
    PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
    PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;

    //Fijamos los nuevos valores
    PRINT_NUMBERTYPE = false;
    //PRINT_OBJECTTYPE = false;
    //PRINT_DEFINEDTYPES = false;

    *out << stabs << ":tasks (" << endl;
    //print(out,0,nindent+NINDENT);
    print(out, 0, nindent + NINDENT);
    *out << stabs << ")" << endl;

    //Recuperamos los valores anteriores
    PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;
    PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
    PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;

    PRINTING_TASKSNETWORK = false;
    delete writed;
}

void TaskNetwork::printDebug(ostream * out) const {
    *out << "Sucesores: " << endl;
    printTable(out, &(succ));

    *out << "Predecesores: " << endl;
    printTable(out, &pred);

    *out << "Lista de tareas: " << endl;
    for (int j = 0; j != (int) tasklist.size(); j++) {
        *out << "[" << j << "] ";
        if (invalid[j])
            *out << "<i> ";
        tasklist[j]->printHead(out);
    }

    *out << "Invalidados: " << endl;
    for (int j = 0; j < (int) invalid.size(); j++)
        *out << invalid[j] << " ";
    *out << endl;

    *out << "Vector backtracking: " << endl;
    for (int j = 0; j < (int) backtracking.size(); j++)
        *out << backtracking[j] << " ";
    *out << endl;

    *out << "Restricciones: " << endl;
    vector<TCTR>::const_iterator b;
    int j = 0;
    struct PrintCT pct(out, 0);
    for (b = tconstraints.begin(); b != tconstraints.end(); b++) {
        *out << "+++ " << j++ << endl;
        pct(*b);
    }

    for (j = 0; j < (int) constraintref.size(); j++) {
        *out << "---> " << j << endl;
        for_each(constraintref[j]->begin(), constraintref[j]->end(),
                Print<int> (out));
    }

}

vector<int> TaskNetwork::getConvergenceNodes(int id_root, int id_limit) const {

    int i;
    int new_branchs;
    vector<int> convergence_nodes;

    new_branchs = 0;
    // Tenemos que empezar por el nodo 1 y terminar por el 0
    // ya que el nodo 0 a lo mejor es el ultimo utilizado para hacer un final
    for (i = 1; i < (int) succ.size(); i++) {
        //Si es un nodo convergente
        if (pred[i]->size() > 1) {
            if (hasThisSuccesor(id_root, i) || i == 0) { //A�adimos i=0 para el caso que terminen en el nodo ficticio 0
                if (new_branchs == 0) {
                    convergence_nodes.push_back(i);
                } else {
                    new_branchs--;
                }
            }
        }
        if (hasThisSuccesor(id_root, i) && succ[i]->size() > 1)
            new_branchs++;
    }
    //Como se comentaba antes ahora comprobamos si el nodo 0
    //es convergente para id_root, ya que si la red de tareas termina
    //con varios caminos todos ellos confluyen ficticiamente en el nodo 0
    //Si es un nodo convergente
    if (pred[0]->size() > 1) {
        //if(hasThisSuccesor(id_root, 0)) { //A�adimos i=0 para el caso que terminen en el nodo ficticio 0
        if (new_branchs == 0) {
            convergence_nodes.push_back(i);
        }
        //}
    }

    return convergence_nodes;
}

int TaskNetwork::getLastConvergenceNode(int id_root, int id_limit) const {

    //Nodos buscados
    vector<int> convergence_nodes;

    //Nodo buscado (c)
    int c = -1, ct;

    // Contador de los nodos comunes encontrados
    int n_encontrados = 0;
    // Indicador si se ha encontrado el nodo a lo largo del proceso de busqueda
    bool encontrado = false;

    //Iteradores
    int i, j;

    // Numero de ramas para el id_root
    int n, nreal;

    n = succ[id_root]->size();

    intvit er, jr;
    intvit *b = new intvit[n];
    intvit *it = new intvit[n];
    intvit *e = new intvit[n];
    intvit g;

    //Guardamos los iteradores al primer y al ultimo elemento de los sucesores
    nreal = 0;
    er = succ[id_root]->end();
    for (jr = succ[id_root]->begin(); jr != er; jr++) {
        if ((*jr) != 0) {
            cout << "Hijo " << nreal << endl;
            b[nreal] = succ[*jr]->begin();
            e[nreal] = succ[*jr]->end();
            nreal++;
        }

    }

    cout << "Encontrados " << nreal << " hijos de " << n << endl;

    //Recorremos toda la secuencia del primer sucesor hasta:
    // - Llegar al final (todos las ramas convergen en el final de la red de tareas)
    // - Llegar a una tarea comun c (todas las ramas convergen en la tarea comun c)
    encontrado = false;

    n_encontrados = 0;
    //Recorremos todos los valores de la primera rama (rama referencia)
    for (j = 0; j < nreal; j++) {
        g = b[j];
        while ((*g) < id_limit && (*g) != 0) {

            if ((*g) != 0) {
                // Vamos a buscar el nodo ct
                ct = *g;
                //Recorremos todas las ramas
                n_encontrados = 0;
                for (i = 0; i < nreal; i++) {
                    //cout << "Analizando el hijo " << i << endl;

                    // Recorremos la rama 'i' buscando el valor 'ct'
                    it[i] = b[i];
                    while ((*it[i]) < id_limit && (*it[i]) != 0) {
                        if (*(it[i]) != 0) {
                            //cout << "Posicion " << *(it[i]) << endl;
                            if (*(it[i]) == ct) {
                                n_encontrados++;
                                cout << "Valor " << n_encontrados << "/"
                                        << nreal << " encontrado de " << ct
                                        << endl;
                            }
                        }
                        //Pasamos al siguiente valor de la rama 'i'
                        it[i] = succ[*(it[i])]->begin();
                    }

                    //cout << "Fin del hijo " << i << endl;
                }
                if (n_encontrados == nreal) {
                    encontrado = true;
                    c = ct;
                }
            }

            //Pasamos al siguiente valor de la rama de referencia
            g = succ[*g]->begin();
        }
    }
    if (!encontrado)
        c = -1;

    //cout << "Devolviendo " << c << " y succ.size() es " << succ.size() << endl;

    return c;

}

bool TaskNetwork::hasThisSuccesor(int id_root, int s) const {
    intvit j, e;
    bool r = false;

    j = succ[id_root]->begin();
    while ((*j) != 0) {
        if (*j == s)
            r = true;
        j = succ[*j]->begin();
    }

    return r;
}

bool TaskNetwork::isFirstSuccesor(int i, vector<int> convergence_nodes) const {

    bool r = true;
    int j;
    intvit b, e;

    //Recorremos todos los nodos convergentes en busca del valor 'i'
    for (j = 0; j < (int) convergence_nodes.size(); j++) {

        //Recorremos los sucesores de los nodos convergentes
        b = succ[convergence_nodes[j]]->begin();
        while ((*b) != 0) {
            //Si el valor es el buscado y no es la posicion del valor buscado es porque hemos encontrado
            //el valor buscado desde una rama que no es la que sale de el
            if (*b == convergence_nodes[i] && i != 0)
                r = false;
            b = succ[*b]->begin();
        }
    }

    return r;

}

void TaskNetwork::print(ostream * out, int id_root, int id_limit, int nindent) const {

}

bool TaskNetwork::allFathersPrinted(int tid) const {
    bool r;
    intvit e, i;

    r = true;
    e = pred[tid]->end();
    for (i = pred[tid]->begin(); i != e; i++) {
        if (!writed[*i])
            r = false;
    }

    return r;
}

void TaskNetwork::printArc(ostream *out, int origin, int destiny, int nindent) {

    int i;
    vector<int> convergence_nodes;
    string stabs(nindent, ' ');

    // ///// PARENTESIS DERECHOS (Para cerrar ramas paralelas)///////
    if (pred[destiny]->size() > 1)
        *out << ")";

    // //////////////////////// CORCHETE DERECHO //////////////////////////////////////
    //Si la tarea destino tiene mas de un padre  cerramos la rama paralela
    // con un parentesis
    if (pred[destiny]->size() > 1) {
        //Si ya han sido escritos todos los padres cerramos las ramas
        //paralelas con un corchete
        if (allFathersPrinted(destiny)) {
            *out << endl << stabs << "]" << endl << stabs;
            rightKey[destiny] = true;
        }
    }

    // ///// PARENTESIS IZQUIERDOS (Para abrir ramas paralelas)///////
    int numParalel = 0;
    int numOpenedParalel = 0;
    if (succ[origin]->size() > 1) {
        // Buscamos los nodos de convergencia
        convergence_nodes = getConvergenceNodes(destiny, succ.size());
        for (i = 0; i < (int) convergence_nodes.size(); i++) {
            //Si es el primer hijo de la rama paralela se imprime un corchete
            numParalel++;
            if (!leftKey[convergence_nodes[i]]) {
                numOpenedParalel++;
            }
        }
    }
    if (numOpenedParalel < numParalel)
        *out << endl << stabs << "(";

    // ///////////////////// CORCHETE IZQUIERDO ///////////////////////////////////////
    //Si la tarea contiene sucesores convergentes abrimos el parentesis
    //para todos esos sucesores convergentes a los que no se le haya
    //abierto corchete.
    if (succ[origin]->size() > 1) {
        // Buscamos los nodos de convergencia
        for (i = 0; i < (int) convergence_nodes.size(); i++) {
            //Si es el primer hijo de la rama paralela se imprime un corchete
            if (!leftKey[convergence_nodes[i]]) {
                *out << endl << stabs << "[ (";
                leftKey[convergence_nodes[i]] = true;
            }
        }
    }

    //     if(isParalel && !isFirst)
    //         *out << endl << stabs << "(";

    // ///// PARENTESIS IZQUIERDOS (Para abrir ramas paralelas)///////
    //Si la tarea contiene sucesores convergentes abrimos el parentesis
    //para todos esos sucesores convergentes a los que no se le haya
    //abierto el parentesis.
    //     if(succ[origin]->size() > 1) {
    //         if(convergence_nodes.size()>0)
    //             *out << "( ";
    //         }


}

void TaskNetwork::print(ostream * out, int tid, int nindent) {

    intvit e, i;

    const vector<TCTR> * v;

    if (tid < (int) succ.size()) { // Si no hemos llegado al final

        //Imprimimos el nodo si todos sus padres han sido impresos o si es el nodo raiz
        if ((!writed[tid] && allFathersPrinted(tid)) || tid == 0) {
            /*            if(succ[tid]->size()==1)
            *out << "(";*/
            v = getTConstraints(tid - 1);
            if (!v->empty())
                *out << "(";
            if (!v->empty())
                for_each(v->begin(), v->end(), PrintCT(out, 1));

            //Caso base imprimimos la tarea
            if (tid > 0) {
                if (inmediate[tid - 1])
                    *out << "!";
                if (backtracking[tid - 1])
                    *out << "(bt) " << endl;
            }
            if (tid != 0) {
                tasklist[tid - 1]->print(out, nindent); // Restamos uno ya que el nodo 0 que nosotros recibimos es una raiz ficticia
            }
            if (!v->empty())
                *out << ")";

            //             if(succ[*(pred[tid]->begin())]->size()==1)
            //                 *out << ")";

            writed[tid] = true; // Marcamos como impreso el nodo que acabamos de imprimir


            //Imprimimos los arcos y los hijos
            e = succ[tid]->end();
            for (i = succ[tid]->begin(); i != e; i++) {
                printArc(out, tid, *i, nindent);
                if ((*i) != 0) {
                    //Imprimimos el arco que va desde el padre al hijo (de 'tid' a 'i')
                    //Imprimimos el hijo 'i'
                    print(out, *i, nindent);
                }
            }

        }
    }

}

void TaskNetwork::toxml(ostream * os, bool primitives, bool compound,
        bool inlines) const {
    XmlWriter * writer = new XmlWriter(os);

    toxml(writer, compound, inlines);

    writer->flush();
    delete writer;
}
;

void TaskNetwork::toxml(XmlWriter * writer, bool primitives, bool compound,
        bool inlines) const {
    // a�adimos ahora las tareas
    writer->startTag("tasks");

    for (int j = 0; j != (int) tasklist.size(); j++)
        if (!invalid[j]) {
            if (tasklist[j]->isCompoundTask() && compound) {
                tasklist[j]->toxml(writer);
            } else if (tasklist[j]->isPrimitiveTask()
                    && !((PrimitiveTask *) tasklist[j])->isInline()
                    && primitives) {
                tasklist[j]->toxml(writer);
            } else if (tasklist[j]->isPrimitiveTask()
                    && ((PrimitiveTask *) tasklist[j])->isInline() && inlines) {
                tasklist[j]->toxml(writer);
            }
        }

    // cierre del tag tasks
    writer->endTag();
}

void TaskNetwork::printAgenda(ostream * out, int nindent, const vector<pair<
        int, bool> > * agenda) const {
    string stabs(nindent, ' ');
    vector<pair<int, bool> >::const_iterator i, e;
    bool printed;

    *out << "Succesors:" << endl;
    printTable(out, &(succ));

    *out << "Task list: " << endl;
    for (int j = 0; j != (int) tasklist.size(); j++) {
        *out << "[" << j+1 << "] ";
        printed = false;
        if (invalid[j]) {
            *out << "(closed)     ";
            printed = true;
        } else {
            e = agenda->end();
            for (i = agenda->begin(); i != e && !printed; i++) {
                if ((*i).first == j) {
                    printed = true;
                    *out << "(agenda)";
                    if ((*i).second)
                        *out << "(bt) ";
                    else
                        *out << "     ";
                }
            }
        }

        if (!printed) {
            *out << "(pending)    ";
        }

        tasklist[j]->printHead(out);
    }

}

void TaskNetwork::printUnifyTask(const char * name,
        const vector<Term *> * args, ostream * out) const {
    for (tasklistcit i = tasklist.begin(); i != tasklist.end(); i++) {
        if (strcasecmp((*i)->getName(), name) == 0)
            if (unify(args, (*i)->getParameters())) {
                (*i)->print(out);
                *out << endl;
            }
    }
}

int TaskNetwork::initializeAgenda(vector<pair<int, bool> > * agenda, int node) const {
    /*cerr << "Agenda: ";
    for(vector<int>::const_iterator i = agenda->begin(); i != agenda->end(); i++)
    cerr << (*i) << " ";
    cerr << endl << "nodo: " << node << " pos: " << pos << endl; */
    // inicializacion

    node++;
    intvrit e = succ[node]->rend();
    for (intvrit i = succ[node]->rbegin(); i != e; i++)
        agenda->push_back(make_pair(*i - 1, backtracking[*i - 1]));

    if (inmediate[node] > 0)
        return inmediate[node];
    else
        return 0;
}

TaskNetwork * TaskNetwork::clone(void) const {
    return new TaskNetwork(this);
}

pkey TaskNetwork::getTermId(const char * name) const {
    pkey result(-1, -1);
    tasklistcit k, e;
    e = tasklist.end();
    for (k = tasklist.begin(); k != e; k++) {
        result = (*k)->getTermId(name);
        if (result.first != -1)
            return result;
    }
    return result;
}

bool TaskNetwork::hasTerm(int id) const {
    for (tasklistcit k = tasklist.begin(); k != tasklist.end(); k++) {
        if ((*k)->hasTerm(id))
            return true;
    }
    return false;
}

void TaskNetwork::renameVars(Unifier * u, VUndo * undo) {
    tasklistcit k, e = tasklist.end();
    for (k = tasklist.begin(); k != e; k++)
        (*k)->renameVars(u, undo);
    for_each(tconstraints.begin(), tconstraints.end(), RVCT(u, undo));
}

void TaskNetwork::printTable(ostream * out, const vector<vector<int> *> * v) {
    int c = 0;
    for (intvvcit i = v->begin(); i != v->end(); i++, c++) {
        *out << "[" << c << "] ";
        for (intvcit j = (*i)->begin(); j != (*i)->end(); j++) {
            *out << (*j) << " ";
        }
        *out << endl;
    }
}

struct NotWellDefined {
    ostream * err;
    bool * changes;

    NotWellDefined(ostream * os, bool * c) {
        err = os;
        changes = c;
    }
    ;

    bool operator()(const Task * th) {
        if (th->isTaskHeader()) {
            //th->print(&cerr);
            vector<Task *> cand;
            Unifier * u;

            MetaTH * mt =
                    (MetaTH *) parser_api->domain->metainfo[th->getMetaId()];
            TaskTableRange range;
            tasktablecit i, e;
            range = parser_api->domain->getTaskRange(th->getId());
            mt->candidates.clear();

            e = range.second;
            for (i = range.first; i != e; i++) {
                Task * t = (*i).second;
                cand.push_back(t);
                u = new Unifier();
                if (unify3(th->getParameters(), t->getParameters(), u))
                    mt->candidates.push_back(t);
                delete u;
            }

            if (mt->candidates.size() == 1) {
                // si solo hay un posible candidato
                // y la tarea es primitiva puedo adelantar
                // ya la unificaci�n
                u = new Unifier();
                //cerr << "==============================" << endl;
                unify3(th->getParameters(), mt->candidates[0]->getParameters(),
                        u);
                //    u->print(&cerr,0);
                //      cerr << endl;
                //      th->print(&cerr,0);
                //      cerr << endl;
                //      mt->candidates[0]->printHead(&cerr);
                //      cerr << endl << "---------------------------" << endl;
                if (u->hasTypeSubstitutions()) {
                    u->applyTypeSubstitutions(0);
                    //  cerr << "CAMBIOS" << endl;
                    *changes = true;
                }
                delete u;
                //cerr << "----" << endl;
            }
            if (mt->candidates.empty()) {
                *err << "There is no action to acomplish: ";
                th->print(err, 0);
                if (!cand.empty()) {
                    *err << "Possible candidates are:" << endl;
                    vector<Task *>::iterator cb, ce = cand.end();
                    for (cb = cand.begin(); cb != ce; cb++) {
                        (*cb)->printHead(err);
                        *err << " defined near or in:";
                        *err << "["
                                << parser_api->files[parser_api->domain->metainfo[(*cb)->getMetaId()]->fileid]
                                << "]";
                        *err << ":"
                                << parser_api->domain->metainfo[(*cb)->getMetaId()]->linenumber
                                << endl;
                    }
                }
                return true;
            }
        }
        //cerr << "FIN" << endl;
        return false;
    }
    ;
};

bool TaskNetwork::isWellDefined(ostream * err, bool * changes) const {
    tasklistcit i, e = tasklist.end();
    i = tasklist.begin();
    bool wd = true;
    while (e != (i = find_if(i, e, NotWellDefined(err, changes)))) {
        wd = false;
        i++;
    }
    return wd;
}

/*void TaskNetwork::addMaintain(Goal * g)
 {
 if(!maintain)
 {
 maintain = new vector< vector<Goal *> >;
 }

 int size = tasklist.size();

 for(int i=0; i<size; i++) {
 if(!maintain[i]){
 maintain[i] = new vector<Goal *>;
 }
 maintain[i] = g;
 }
 };*/

bool TaskNetwork::before(int a, int b) const {
    // cerr << "(" << a << "," << b << ")" << endl;
    // si son iguales se da como true
    if (a == b)
        return true;
    // si b es menor o igual que 0 nunca puede ir antes que a
    else if (b <= 0)
        return false;
    else if (a <= 0)
        return false;
    // si el �ndice de a es mayor que b, a nunca va antes de b
    else if (a > b)
        return false;

    // b tiene que ser alg�n sucesor de a para que a vaya antes que b
    intvit i, e = succ[a]->end();

    for (i = succ[a]->begin(); i != e; i++) {
        if (before(*i, b))
            return true;
    }

    //    if(b < (int) pred.size()){
    //    intvit i,e = pred[b]->end();
    //
    //    for(i=pred[b]->begin();i!=e;i++){
    //        if(before(a,*i))
    //        return true;
    //    }
    //    }
    return false;
}

int TaskNetwork::getIndexOf(const Task * t) const {
    tasklistcit i;
    if ((i = find(tasklist.begin(), tasklist.end(), t)) == tasklist.end())
        return -1;
    else
        return distance(tasklist.begin(), i);
}
;

bool TaskNetwork::iAfter(int taskA, int taskB) const {
    intvcit e = succ[taskB]->end();
    for (intvcit i = succ[taskB]->begin(); i != e; i++)
        if ((*i) == taskA)
            return true;
        else if (tasklist[(*i) - 1]->isPrimitiveTask()) {
            if (((PrimitiveTask *) tasklist[(*i) - 1])->getInline()) {
                if (iAfter(taskA, (*i)))
                    return true;
            }
        }
    return false;
}
;

bool TaskNetwork::iBefore(int taskA, int taskB) const {
    intvcit e = pred[taskB]->end();
    for (intvcit i = pred[taskB]->begin(); i != e; i++)
        if ((*i) == taskA)
            return true;
        else if (tasklist[(*i) - 1]->isPrimitiveTask()) {
            if (((PrimitiveTask *) tasklist[(*i) - 1])->getInline()) {
                if (iBefore(taskA, (*i)))
                    return true;
            }
        }
    return false;
}
;

const vector<TCTR> * TaskNetwork::getTConstraints(int index) const {
    static vector<TCTR> ret;
    ret.clear();
    vector<vector<int> *>::const_iterator i, e = constraintref.end(), b =
            constraintref.begin();
    for (i = b; i != e; i++) {
        if (find((*i)->begin(), (*i)->end(), index) != (*i)->end()) {
            ret.push_back(tconstraints[distance(b, i)]);
        }
    }
    return &ret;
}
;

void TaskNetwork::addTConstraint(TCTR & v) {
    tconstraints.push_back(v);
    constraintref.push_back(new vector<int> );
    for (int i = 0; i < (int) tasklist.size(); i++)
        constraintref.back()->push_back(i);
}
;

void TaskNetwork::comprobarEstructura(void) {
    int s, s2, i, j;
    int current_size, index;

    current_size = succ.size();

    s = succ.size();
    for (i = 1; i < s; i++) {
        s2 = succ[i]->size();
        if (!invalid[i - 1])
            for (j = 0; j < s2; j++) {
                index = (*(succ[i]))[j];
                if (index > 0 && index < current_size) {
                    if (invalid[index - 1]) {
                        cerr << "Hay un inv�lido: " << index
                                << " en sucesores de: " << i << endl;
                        exit(0);
                    }
                } else if (index != 0) {
                    cerr << "Indice fuera de rango en sucesores[" << i << "]: "
                            << index << " tama�o: " << current_size << endl;
                    exit(0);
                }
            }
    }

    s = pred.size();
    for (i = 1; i < s; i++) {
        s2 = pred[i]->size();
        if (!invalid[i - 1])
            for (j = 0; j < s2; j++) {
                index = (*(pred[i]))[j];
                if (index > 0 && index < current_size) {
                    if (invalid[index - 1]) {
                        cerr << "Hay un inv�lido: " << index
                                << " en predecesores de: " << i << endl;
                        exit(0);
                    }
                } else if (index != 0) {
                    cerr << "Indice fuera de rango en predecesores[" << i
                            << "]: " << index << " tama�o: " << current_size
                            << endl;
                    exit(0);
                }
            }
    }
}
;

