#include "plan.hh"
#include <string>
#include <iostream>
#include <math.h>
#include "problem.hh"
#include "domain.hh"
#include "causalTable.hh"
#include "function.hh"
#include "debugger.hh"
#include "timeLineLitEffect.hh"
#include "undoARLiteralState.hh"
#include "xmlwriter.hh"

#ifdef USE_AC3
#include "tcnm-ac3.hh"
#endif

#ifdef USE_PC2
#include "tcnm-pc2.hh"
#endif

#ifdef USE_PC2_CL
#include "tcnm-pc2-cl.hh"
#endif

using namespace std;

extern int TU_Transform[6];

Plan * current_plan;

// Excepci�n lanzada al incurrir en error en la red de restricciones temporales
class Temporal_Exception: public exception {
    virtual const char* what() const throw() {
        return "STN Exception";
    }
} TException;

Plan::Plan(const Domain * domain, const Problem * problem) {
    // Inicializar las estructuras de datos.
    assert(domain != 0);
    assert(problem != 0);
    this->domain = domain;
    this->problem = problem;

    state = 0;
    tasknetwork = 0;
    stp = 0;
    select = 0;
    schedule = 0;
    etree = 0;
    XML_TFORMAT = problem->TFORMAT;
    FLAG_VERBOSE = 0;
    FLAG_EXPANSIONS_LIMIT = 0;
    FLAG_DEPTH_LIMIT = 0;
    FLAG_TLIMIT = 0;
    hasplan = false;
}
;

Plan::~Plan(void) {
    reset();
}
;

void Plan::reset(void) {
    for_each(stack.begin(), stack.end(), Delete<StackNode> ());
    if (tasknetwork)
        delete tasknetwork;
    if (state)
        delete state;
    if (stp)
        delete stp;
    if (state)
        delete state;
    if (schedule)
        delete schedule;
    if (etree)
        delete etree;
    etree = 0;
    schedule = 0;
    plan.clear();
    state = 0;
    tasknetwork = 0;
    stp = 0;
    nodeCounter = 0;
    mCounter = 0;
    eCounter = 0;
    cortados = 0;
    CL = 0;
    directos = 0;
    hasplan = false;
}
;

void Plan::init(void) {
    assert(domain != 0);
    assert(problem != 0);
    reset();
    // Digo que soy el plan actualmente en ejecuci�n
    current_plan = this;
    // Crear el estado inicial
    state = problem->getInitialState()->clone();
    // Inicializar la pila de contextos
    stack.push_back(new StackNode());
    // Copiar la red de tareas objetivo
    tasknetwork = problem->getInitialGoal()->clone();
    // inicializar la agenda
    tasknetwork->initializeAgenda(&(stack.front()->agenda), -1);
}
;

void Plan::printStatistics(ostream * flow) const {
//     *flow
//         << "-----------------------------------------------------------------"
//         << endl;
//     *flow << "Number of actions: " << getNumberOfActions() << " ("
//         << plan.size() << ")" << endl;
//     *flow << "Expansions: " << eCounter << endl;
//     *flow << "Generated nodes: " << nodeCounter << endl;
//     *flow << "Inferences: " << mCounter << endl;
//     *flow << "Time in seconds: " << (float) ((TEND - TSTART) * 1.0)
//         / (CLOCKS_PER_SEC * 1.0) << endl;
//     *flow << c1.Description() << ": " << c1.Time() << endl;
//     *flow << c2.Description() << ": " << c2.Time() << endl;
//     *flow << c3.Description() << ": " << c3.Time() << endl;
// #ifdef __tcnm_pc2_cl
//     cerr << "Cortados: " << cortados << endl;
//     cerr << "Directos: " << directos << endl;
//     cerr << "CL: " << CL << endl;
// #endif
//     *flow
//         << "-----------------------------------------------------------------"
//         << endl;

    // // MODIFICACION ------------------------------------------------------------
    // // Red de tareas
    // *flow
    //     << "Red de tareas" << endl
    //     << "-----------------------------------------------------------------"
    //     << endl;
    // tasknetwork->printDebug(flow);

    // *flow
    //     << "-----------------------------------------------------------------"
    //     << endl;

    // // Red de tareas
    // *flow
    //     << "Red de tareas - 2" << endl
    //     << "-----------------------------------------------------------------"
    //     << endl;

    // int counter=0;
    // for(tasklistcit i = getTaskNetwork()->getBeginTask(); i!= getTaskNetwork()->getEndTask(); i++)
    // {
    //     if((*i)->isTaskHeader())
    //     {
    //         *flow << "n" << counter << "[label=\"" << (*i)->getName() << "\",color=red]" << endl;
    //     }
    //     else if((*i)->isPrimitiveTask())
    //     {
    //         *flow << "n" << counter << "[label=\"" << (*i)->getName() << "\",color=green,shape=box]" << endl;
    //     }
    //     counter++;
    // }

    // int size = getTaskNetwork()->getNumOfNodes();
    // for(int k =0; k<size; k++)
    //     for(int j =0; j<size; j++)
    //     {
    //         if( getTaskNetwork()->inmediatelyAfter(k,j))
    //             *flow << "n" << k << "->" << "n" << j << endl;
    //     }

    // for(int k =0; k<size; k++)
    //     for(int j =0; j<size; j++)
    //     {
    //         if( getTaskNetwork()->inmediatelyBefore(k,j))
    //             *flow << "n" << k << "<-" << "n" << j << endl;
    //     }

    // *flow << "}";

    // *flow
    //     << "-----------------------------------------------------------------"
    //     << endl;

    // // Tabla de vínculos causuales
    // *flow
    //     << "Tabla de vínculos causales" << endl
    //     << "-----------------------------------------------------------------"
    //     << endl;

    // causalTable.plot(flow);

    // *flow
    //     << "-----------------------------------------------------------------"
    //     << endl;

    // // Nodos hoja del arbol de expansión
    // *flow
    //     << "Número de nodos hoja del árbol de expansión" << endl
    //     << "-----------------------------------------------------------------"
    //     << endl;

    // ExpansionTree * etree = new ExpansionTree(&plan, tasknetwork);
    // *flow << etree->getNumberOfRoots() << endl;

    // *flow
    //     << "-----------------------------------------------------------------"
    //     << endl;

    // // Matriz de adyacencia
    // *flow
    //     << "Matriz de adyacencia" << endl
    //     << "-----------------------------------------------------------------"
    //     << endl;

    // TaskNetwork * task = getTaskNetwork()->clone();
    // task->print(flow);

    // *flow
    //     << "-----------------------------------------------------------------"
    //     << endl;
}

int Plan::deleteFromState(int id, const KeyList * params) {
    ISTable_mrange range = state->getModificableRange(id);
    isit j;
    while ((j = find_if(range.first, range.second, EqualLit(params)))
            != range.second) {
        if ((*j).second->isLEffect()) {
            state->deleteLiteral(j);
        }
    }
    return 0;
}

int Plan::deleteFromState(const Literal * l) {
    ISTable_mrange r = state->getModificableRange(l->getId());
    isit i, e = r.second;
    for (i = r.first; i != e; i++)
        if (unify(l->getParameters(), (*i).second->getParameters())) {
            if (l->getPol() == (*i).second->getPol()) {
                state->deleteLiteral(i);
                return 0;
            }
        }
    return -1;
}
;

int Plan::addToState(Literal * l) {
    state->addLiteral(l);
    return 0;
}
;

struct PrintUndoElementln {
    ostream * os;
    PrintUndoElementln(ostream * o) {
        os = o;
    }
    ;

    void operator()(const UndoElement * u) const {
        *os << "(ccc) ";
        if (u->isUndoChangeValue()) {
            Function * f = (Function *) (((UndoChangeValue *) u)->target);
            *os << "Changing value: ";
            f->printL(os, 0);
        } else
            u->print(os);
        *os << endl;
    }
    ;
};

struct MarkParent {
    /**
     * OJO!! t deber�a ser una tarea compuesta.
     */
    MarkParent(Task * t, Method * m) {
        task = (CompoundTask *) t;
        met = m;
    }
    ;

    void operator()(Task * t) const {
        t->setParent(task);
        t->setParentMethod(met);
    }
    ;

    CompoundTask * task;
    Method * met;
};

bool Plan::solve(void) {
    init();

    c1.Start();
    c2.Start();
    c3.Start();
    TSTART = clock();

    if (FLAG_VERBOSE) {
        *errflow << "(*** " << stack.size()
            << " ***) Initializating planning process..." << endl;
    }

#ifdef PYTHON_FOUND
    if(FLAG_VERBOSE)
    {
        *errflow << "(*** " << stack.size() << " ***) Initializating python..." << endl;
    }
    parser_api->wpython.execQueue();
#endif

    // Si la agenda est� vac�a tenemos que realizar la red de
    // tareas vac�a. Hemos terminado
    if (stack.back()->agenda.empty()) {
        hasplan = true;
        return true;
    }

    // si es necesario inicializar la red temporal
    if (domain->isTimed()) {
        stp = new STP();
#ifdef __tcnm
        stp->setPropagationMethod(PC2);
        if(FLAG_VERBOSE)
            *errflow << "(***) Using PC2" << endl;
#endif
#ifdef __tcnm_pc2_cl
        stp->setPropagationMethod(PC2);
        stp->useContextInfo();
        if(FLAG_VERBOSE)
            *errflow << "(***) Using modified PC2" << endl;
#endif
#ifdef __tcnm3
        if(FLAG_VERBOSE)
            *errflow << "(***) Using AC3" << endl;
#endif
#ifdef __tcnm_pc2_cl
        // start point
        stp->InsertTPoint(0);
        // end point
        stp->InsertTPoint(0);
#else
        // start point
        stp->InsertTPoint();
        // end point
        stp->InsertTPoint();
#endif
        // hacer que el fin del plan siempre vaya despu�s del inicio
        stp->AddTConstraint2(0, 1, 0, POS_INF);

        //generateTPoints(tasknetwork);
        //cerr << "insertando tpoints para inicializacion " << stp->Size() << endl;
    }

    if (!mainLoop()) {
        c1.Stop();
        c2.Stop();
        c3.Stop();
        TEND = clock();
        plan.clear();
        return false;
    } else {
        // For the Decomposition Tree
        for (int i=0; i<stack_copy.size(); i++) {
            int mpos = stack_copy.at(i)->mpos;
            int explored_size = stack_copy.at(i)->explored.size();

            // Sigue habiendo tareas-basura al final, se puede comprobar con explored            
            if (explored_size > 0) {
                if (FLAG_TREE) {
                    *errflow << "\n===" << endl;

                    *errflow 
                        << "Tarea:"
                        << stack_copy.at(i)->explored.at(explored_size-1).first
                        << endl;
                }
                    
                // Si no es una primitiva
                if (FLAG_TREE) {
                    if (mpos != -1) {    
                        *errflow << endl;
                        stack_copy.at(i)->methods->at(mpos)->print(errflow);
                    }
                }
            }
        }        

        if (FLAG_TREE) {
            *errflow << "\n===" << endl;
            for (int i=0; i<stack_copy.size(); i++) {
                int mpos = stack_copy.at(i)->mpos;
                int explored_size = stack_copy.at(i)->explored.size();

                // Sigue habiendo tareas-basura al final, se puede comprobar con explored            
                if (explored_size > 0) {
                    // Si existen unificaciones (>=)
                    if (stack_copy.at(i)->unif >= 0) {
                        Unifier * uf = stack_copy.at(i)->utable->getUnifierAt(stack_copy.at(i)->unif);
                        vSubstitutions substitutions = uf->substitutions;
                        
                        if (substitutions.size() > 0) {
                            uf->printUnifications(&*errflow);
                        }
                    }
                }
            }
            *errflow << "===" << endl;
        
            tasknetwork->printDebug(errflow);
            *errflow << "###" << endl;
        }

        c1.Stop();
        c2.Stop();
        c3.Stop();
        TEND = clock();
        hasplan = true;
        return true;
    }
}

bool Plan::mainLoop(void) {
    StackNode * next;
    // tasknetwork->print(&cerr);
    // tasknetwork->printDebug(&cerr);
    // exit(EXIT_FAILURE);

    // IDs de tareas root
    if (FLAG_TREE) {
        *errflow << "Root:";
        for (int j = 0; j != (int) tasknetwork->tasklist.size(); j++) {
            *errflow << j << "-";
        }
    }

    // MODIFICACION
    stack_copy = stack;    

    while (!stack.empty()) {
        if (FLAG_EXPANSIONS_LIMIT > 0 && nodeCounter >= FLAG_EXPANSIONS_LIMIT) {
            *errflow << "Limit number of expansions reached: "
                << FLAG_EXPANSIONS_LIMIT << "/" << nodeCounter << endl;
            break;
        }
        if (FLAG_TLIMIT > 0 && FLAG_TLIMIT < ((clock() - TSTART) * 1.0)
                / (CLOCKS_PER_SEC * 1.0)) {
            *errflow << "Timeout: Maximun amount of time consumed: "
                << FLAG_TLIMIT << endl;
            break;
        }
        if (!(next = solve(stack.back()))) {
            // si se devuelve null, no hay nada mas que hacer en este
            // contexto. Se deshacen los cambios, y se vuelve al contexto
            // anterior.
            next = stack.back();
            // For the decomposition tree
            stack_copy.back();
            
            stack.pop_back();
            // For the decomposition tree
            stack_copy.pop_back();

            if (stack.empty()) {
                //delete next;
                *errflow << "\n[Error]: Empty stack\n";
                plan.clear();
                return false;
            }
            // *errflow << "Deshaciendo (principal)...." << endl;
            if ((int) (parser_api->termtable->variables.size() - next->ttsize)
                    > LIMIT_GARBAGE)
                parser_api->termtable->performGarbageCollection(next->ttsize);
            if (!domain->isTimed()) {
                next->undo(tasknetwork, &plan);
            }
            else {
                next->undo(tasknetwork, &plan, stp);
            }
            delete next;
        } else {
            // En otro caso se ha generado un nuevo contexto
            stack.push_back(next);
            // For the decomposition tree
            stack_copy.push_back(next);
            // Si en el siguiente contexto no hay tareas pendientes
            // significa que hemos alcanzado la condicion de fin
            if (next->agenda.empty()) {
                return true;
            }

            // Eliminar posibles duplicados en la agenda
            int inmediate = -1;
            for (unsigned int i = 0; i < next->agenda.size(); i++) {
                // buscar si alguna de las tareas que se encuentran en la agenda
                // son inmediate.
                if (tasknetwork->isInmediate(next->agenda[i].first)) {
                    if (inmediate == -1) {
                        inmediate = i;
                    }
                    else if (next->agenda[inmediate].first > next->agenda[i].first) {
                        inmediate = i;
                    }
                }
                for (unsigned int j = 0; j != next->agenda.size();) {
                    if (i != j && next->agenda[i].first
                            == next->agenda[j].first) {
                        //*errflow << "BORRANDO VOY" << endl;
                        next->agenda.erase(next->agenda.begin() + j);
                    } else
                        j++;
                }
            }

            // Hay alguna inmediate
            if (inmediate != -1) {
                // Realizar una reestructuracion de la agenda en el contexto
                // actual. Esta reestructuracion garantiza que no se violen
                // las condiciones impuestas por las clausulas inmediate
                unsigned int i = 0;
                while (i < next->agenda.size()) {
                    // si la tarea e no es predecesora en la red de tareas
                    // a la inmediate es descartada para ser expandida
                    // en esta ronda.
                    if (!tasknetwork->before(next->agenda[i].first + 1,
                                next->agenda[inmediate].first + 1)) {
                        next->explored.push_back(next->agenda[i]);
                        next->agenda.erase(next->agenda.begin() + i);
                    } else
                        i++;
                }
                // si queda alguna tarea (ademas de la inmediate)
                // significa que esta debe ejecutarse antes que la inmediate
                // por lo tanto la inmediate es descartada en esta ronda.
                if (next->agenda.size() > 1) {
                    next->explored.push_back(next->agenda[inmediate]);
                    next->agenda.erase(next->agenda.begin() + inmediate);
                }
            } else {
                // garantizar que los puntos join se realizan correctamente
                // no permitiendo que se ejecute ninguna tarea antes de la
                // cuenta.
                // tasknetwork->printDebug(&*errflow);
                unsigned int i = 0;
                int a, b;
                bool borrada;
                while (i < next->agenda.size()) {
                    borrada = false;
                    for (unsigned int j = 0; j < next->agenda.size()
                            && !borrada; j++) {
                        a = next->agenda[i].first;
                        b = next->agenda[j].first;
                        if (a != b)
                            // si la tarea j se ejecuta antes que la i, la i es
                            // descartada en esta ronda
                            if (tasknetwork->before(b + 1, a + 1)) {
                                next->explored.push_back(next->agenda[i]);
                                //*errflow << "Borrado: " << *i << endl;
                                next->agenda.erase(next->agenda.begin() + i);
                                borrada = true;
                            }
                    }
                    if (!borrada)
                        i++;
                }
            }
        }
    }
    return false;
}
;

// Esta funcion va devolviendo tareas por expandir, devolviendo una de las posibles
// cada vez que se la llama. Si no hay mas posibilidades devuelve 0.
//
// Internamente usa las estructuras de datos context->taskid y context->offspring.
// La funcion se encarga de (reservar/liberar) memoria segun sea conveniente.
// Se supone que en la ultima llamada (cuando devuelve 0) se a liberado toda la memoria
// ocupada por sus estructuras.
//
bool Plan::selectTask(StackNode * context) {
    // *errflow << "Current Agenda (ini):" << endl;
    // for_each(context->agenda.begin(),context->agenda.end(),Print<int>(&*errflow));

    if (domain->isTimed())
        context->undoSTP(stp);

    // Quedan metodos por explorar
    // dejamos que se generen sus unificaciones
    if (context->methods)
        return true;

    if (context->offspring) {
        // hay cosas en la estructura de datos, eliminamos la ultima tarea
        // que se uso, deshacemos sus posibles modificaciones

        // deshacer las unificaciones y sustituciones provocadas por el
        // uso de la tarea
        context->undoTaskSelection(context->task);
        context->undoCLchanges();

        // cargarse la ultima tarea usada
        context->removeTask(context->task);
        context->task = -1;

        if (context->offspring->empty()) {
            delete context->offspring;
            context->offspring = 0;

            if (FLAG_VERBOSE)
                *errflow << "(### " << stack.size()
                    << " ###) FAIL!!: No more candidate tasks." << endl;
        }
    }

    // si no hay tareas generadas las generamos si quedan cosas pendientes en la agenda
    while (!context->offspring) {
        if (FLAG_VERBOSE)
            *errflow << "(*** " << stack.size()
                << " ***) Selecting task to expand from agenda." << endl;

        // Sacamos de la agenda una de las posibles alternativas que podemos expandir
        if (context->agenda.empty()) {
            if (FLAG_VERBOSE)
                *errflow << "(### " << stack.size()
                    << " ###) FAIL!!: Agenda is empty." << endl;
            return false;
        }

        if (context->backtrack) {
            // Comprobar que la tarea que se escogio con anterioridad era permutable
            VAgenda::iterator f;
            f = find_if(context->explored.begin(), context->explored.end(),
                    EQPAIRFIRST<int, bool> (context->taskid));
            if ((*f).second) {
                // se ha llegado hasta aqui por un backtracking
                // eliminar de la agenda aquellas tareas que no sean
                // permutables,(tengan el bit de backtracking a 1)
                //context->agenda.erase(context->agenda.begin(),remove_if(context->agenda.begin(),context->agenda.end(),EQPAIRSECOND<int,bool>(false)));
                if (context->agenda.empty()) {
                    if (FLAG_VERBOSE)
                        *errflow << "(### " << stack.size()
                            << " ###) FAIL!!: No more permutable tasks."
                            << endl;
                    return false;
                }
            } else {
                if (FLAG_VERBOSE)
                    *errflow << "(### " << stack.size()
                        << " ###) FAIL!!: No more permutable tasks."
                        << endl;
                return false;
            }
        }

        // @!@ Punto de Bactracking 1. Seleccion de la agenda de la siguiente tarea "unordered"
        // a expandir.
        stage = SelectAgenda;
        context->taskid = select->selectFromAgenda(this);

        Task * father = tasknetwork->getModificableTask(context->taskid);

        // borrar de la agenda la tarea seleccionada
        VAgenda::iterator bit = context->agenda.begin(), finded;
        finded = find_if(bit, context->agenda.end(), EQPAIRFIRST<int, bool> (
                    context->taskid));
        context->explored.push_back(context->agenda[distance(bit, finded)]);
        context->agenda.erase(bit + distance(bit, finded));
        if (FLAG_VERBOSE == 3) {
            *errflow << "(ccc) Removing a task from agenda:" << endl;
            *errflow << "(ccc) New agenda:" << endl;
            for (VAgenda::iterator n = context->agenda.begin(); n
                    != context->agenda.end(); n++)
                *errflow << " [" << (*n).first << "," << (*n).second << "]"
                    << endl;
            *errflow << endl;
        }
        context->backtrack = true;

        if (father->isTaskHeader()) {
            TaskHeader * thfather = (TaskHeader *) father;
            if (FLAG_VERBOSE) {
                *errflow << "(*** " << stack.size() << " ***) Expanding: ["
                    << context->taskid << "] ";
                if (FLAG_VERBOSE > 1)
                    thfather->print(&*errflow);
                else
                    thfather->printHead(errflow);
                *errflow << endl;
            }

            // obtenemos las tareas del dominio que unifican con la definicion
            // de tarea en la red de tareas.
            context->offspring = domain->getUnifyTask(thfather,
                    &(context->undoTask));

            if (context->offspring->empty()) {
                delete context->offspring;
                context->offspring = 0;

                if (FLAG_VERBOSE)
                    *errflow << "(### " << stack.size()
                        << " ###) FAIL!!: No candidate tasks." << endl;
            }
        } else if (father->isPrimitiveTask()
                && ((PrimitiveTask *) father)->getInline()) {
            context->offspring = new vector<Task *> ;
            context->offspring->push_back(father);
        } else {
            *errflow << "Runtime error: Triying to expand an expanded task!."
                << endl;
            exit(EXIT_FAILURE);
        }
    } // del while

    // Se supone que tras los ifs anteriores hay "algo" en la estructura context->offspring
    if (FLAG_VERBOSE) {
        *errflow << "(*** " << stack.size()
            << " ***) Selecting a candidate task." << endl;
    }

    vector<Task *> * v = context->offspring;
    vector<string> * vs = 0;

    if (FLAG_VERBOSE || FLAG_DEBUG) {
        if (FLAG_DEBUG)
            vs = new vector<string> ;

        if (FLAG_VERBOSE)
            *errflow << "(*** " << stack.size() << " ***) Found: "
                << context->offspring->size() << " candidates (left)."
                << endl;
        for (int c = 0; c < (int) v->size(); c++) {
            if (FLAG_VERBOSE > 1) {
                *errflow << "      [" << c << "] ";
                (*v)[c]->printHead(errflow);
            }
            if (FLAG_DEBUG) {
                ostringstream os;
                (*v)[c]->print(&os);
                vs->push_back(os.str());
            }
        }

        if (FLAG_DEBUG)
            debugger->setOptions(vs);
    }

    // @!@ Punto de Bactracking 2. De entre las distintas posibilidades de
    // expansion para la tarea seleccionamos una
    stage = SelectTask;
    context->task = select->selectTask(this);

    if (vs) {
        delete vs;
        vs = 0;
    }

    return true;
}

bool Plan::selectUnification(StackNode * context) {
    if (context->intervals && !context->intervals->empty()) {
        // volver a ir hacia adelante cuando quedan cosas en el tl
        if ((context->tcnpos = searchTimeLine(context, *(context->intervals),
                        context->tcnpos)) != -1) {
            if (FLAG_VERBOSE)
                *errflow << "(*** " << stack.size()
                    << " ***) New time-line slot: " << context->tcnpos
                    << " from " << context->intervals->size() << "."
                    << endl;
            return true;
        }
        if (FLAG_VERBOSE)
            *errflow << "(### " << stack.size()
                << " ###) FAIL!!: No more slots in time-line." << endl;
        delete context->intervals;
        context->intervals = 0;
    }

    if (domain->isTimed())
        context->undoSTP(stp);

    if (context->offspring == 0 || context->task == -1)
        return false;

    // si no quedan unificaciones pendientes, es necesario seleccionar otra tarea
    // de las disponibles
    Task * t = (*(context->offspring))[context->task];

    if (context->utable) {
        context->undoCLchanges();
        context->undoApplyUnification();

        context->utable->erase(context->unif);
        context->unif = -1;
        if (context->utable->isEmpty()) {
            delete context->utable;
            context->utable = 0;
            if (FLAG_VERBOSE)
                *errflow << "(### " << stack.size()
                    << " ###) FAIL!!: No more unifications." << endl;
            return false;
        }
    }

    else if (!context->utable) {
        if (t->isPrimitiveTask()) {
            // generamos las unificaciones
            if (FLAG_VERBOSE) {
                *errflow << "(*** " << stack.size() << " ***) Solving action:"
                    << endl;
                if (FLAG_VERBOSE > 1)
                    t->print(&*errflow, 0);
                else
                    t->printHead(&*errflow);
            }
            PrimitiveTask * pt = (PrimitiveTask *) t;
            if (pt->getPrecondition()) {
                if (pt->isInline()) {
                    if (context->done) {
                        if (FLAG_VERBOSE)
                            *errflow << "(### " << stack.size()
                                << " ###) FAIL!!: No unifications." << endl;
                        context->utable = 0;
                        context->done = false;
                        return false;
                    } else {
                        context->utable = pt->getPrecondition()->getUnifiers(
                                state, 0, true, 0);

                        if (!context->utable || context->utable->isEmpty()) {
                            context->done = true;
                            if (FLAG_VERBOSE)
                                *errflow << "(### " << stack.size()
                                    << " ###) FAIL!!: No unifications."
                                    << endl;
                            if (context->utable)
                                delete context->utable;
                            context->utable = 0;
                            return false;
                        }
                    }
                } else {
                    context->utable = pt->getPrecondition()->getUnifiers(state,
                            0, true, 0);

                    if (!context->utable || context->utable->isEmpty()) {
                        if (FLAG_VERBOSE)
                            *errflow << "(### " << stack.size()
                                << " ###) FAIL!!: No unifications." << endl;
                        if (context->utable)
                            delete context->utable;
                        context->utable = 0;
                        return false;
                    }
                }
            } else {
                if (context->done) {
                    if (FLAG_VERBOSE)
                        *errflow << "(### " << stack.size()
                            << " ###) FAIL!!: No unifications." << endl;
                    context->utable = 0;
                    context->done = false;
                    return false;
                } else {
                    context->done = true;
                    context->utable = 0;
                }
            }
        } else if (t->isCompoundTask()) {
            // si es una tarea compuesta
            CompoundTask * ct = (CompoundTask *) t;

            if (FLAG_VERBOSE) {
                *errflow << "(*** " << stack.size()
                    << " ***) Selecting a method to expand from compound task."
                    << endl;
                *errflow << "      ";
                ct->printHead(&*errflow);
            }

            if (!context->methods) {
                // es la primera vez que se llega hasta aqui
                // se genera la lista de metodos...
                context->methods = new VMethods;
                methodcit em, j;
                em = ct->getEndMethod();
                for (j = ct->getBeginMethod(); j != em; j++) {
                    context->methods->push_back((*j));
                }
                if (ct->isRandom()) {
                    // reordenar los metodos de forma aleatoria
                    random_shuffle(context->methods->begin(),context->methods->end());
                }
            } else
                context->methods->erase(context->methods->begin()
                        + context->mpos);

            if (context->methods->empty()) {
                delete context->methods;
                context->methods = 0;
                context->mpos = -1;

                if (FLAG_VERBOSE)
                    *errflow << "(### " << stack.size()
                        << " ###) FAIL!!: No more methods to expand."
                        << endl;
                return false;
            }

            vector<Method *> * vm = context->methods;

            vector<string> * vs = 0;
            if (FLAG_VERBOSE || FLAG_DEBUG) {
                if (FLAG_DEBUG)
                    vs = new vector<string> ;

                if (FLAG_VERBOSE)
                    *errflow << "(*** " << stack.size() << " ***) Found: "
                        << vm->size() << " methods to expand (left)."
                        << endl;
                for (int c = 0; c < (int) vm->size(); c++) {
                    if (FLAG_VERBOSE > 1) {
                        *errflow << "      [" << c << "] " << endl;
                        (*vm)[c]->print(&*errflow, 6);
                        *errflow << endl;
                    } else {
                        *errflow << "      [" << c << "] "
                            << (*vm)[c]->getName() << endl;
                    }
                    if (FLAG_DEBUG) {
                        ostringstream os;
                        (*vm)[c]->print(&os);
                        vs->push_back(os.str());
                    }
                }

                if (FLAG_DEBUG)
                    debugger->setOptions(vs);
            }

            // @!@ Punto de Bactracking 3. De entre los distintos metodos
            // seleccionamos uno
            stage = SelectMethod;
            context->mpos = select->selectMethod(this);

            if (vs) {
                delete vs;
                vs = 0;
            }

            Method * m = (*vm)[context->mpos];

            if (FLAG_VERBOSE) {
                *errflow << "(*** " << stack.size()
                    << " ***) Expanding method: " << m->getName() << endl;
            }

            // Mirar si hay un deshacer pendiente
            context->undoTaskSelection(context->task);
            for_each(context->undoTask[context->task]->begin(),
                    context->undoTask[context->task]->end(),
                    Delete<UndoElement> ());
            context->undoTask[context->task]->clear();

            // Mirar si hay matching de tipos
            TaskHeader * father =
                (TaskHeader *) tasknetwork->getModificableTask(
                        context->taskid);
            Unifier * u = new Unifier();
            if (unify2(m->getParameters(), father->getParameters(), u)) {
                m->renameVars(u, 0);
                u->applyTypeSubstitutions(context->undoTask[context->task]);
            } else {
                if (FLAG_VERBOSE) {
                    *errflow << "(### " << stack.size()
                        << " ###) FAIL!!: No matching types." << endl;
                    *errflow << "Internal method parameters:" << endl;
                    for_each(m->getParameters()->begin(),
                            m->getParameters()->end(), PrintKey(errflow));
                    *errflow << endl;
                    *errflow << "Task parameters:" << endl;
                    for_each(father->getParameters()->begin(),
                            father->getParameters()->end(), PrintKey(errflow));
                    *errflow << endl;
                }
                delete u;
                return false;
            }
            delete u;

            if (m->getPrecondition()) {
                context->utable =
                    m->getPrecondition()->getUnifiers(state, 0, true, 0);

                if (!context->utable || context->utable->isEmpty()) {
                    if (context->utable) {
                        delete context->utable;
                    }
                    context->utable = 0;
                    if (FLAG_VERBOSE) {
                        *errflow << "(### " << stack.size()
                            << " ###) FAIL!!: No unifications." << endl;
                    }
                    return false;
                }
            } else {
               /*
                if (context->done) {
                    if (FLAG_VERBOSE) {
                        *errflow << "(### " << stack.size()
                            << " ###) FAIL!!: No unifications." << endl;
                    }
                    context->done = false;
                    context->utable = 0;
                    return false;
                } else {
                */
                    context->done = true;
                    context->utable = 0;
                //}
            }
        }
    }

    vector<string> * vus = 0;
    if (FLAG_VERBOSE || FLAG_DEBUG) {

        ostringstream os;

        if (FLAG_VERBOSE) {
            if (t->isPrimitiveTask()) {
                PrimitiveTask * pt = (PrimitiveTask *) t;
                *errflow << "(*** " << stack.size()
                    << " ***) working in action:" << endl;
                if (FLAG_VERBOSE > 1)
                    pt->print(&*errflow);
                else
                    pt->printHead(&*errflow);
                *errflow << endl;
            } else {
                CompoundTask * ct = (CompoundTask *) t;
                *errflow << "(*** " << stack.size() << " ***) Working in task:"
                    << endl;
                if (FLAG_VERBOSE > 1)
                    ct->print(&*errflow);
                else
                    ct->printHead(&*errflow);
                *errflow << endl;
                Method * met = (*(context->methods))[context->mpos];
                *errflow << "(*** " << stack.size() << " ***) Using method: "
                    << met->getName() << endl;
            }
        }

        if (context->utable) {
            if (FLAG_DEBUG)
                vus = new vector<string> ;

            if (FLAG_VERBOSE)
                *errflow << "(*** " << stack.size() << " ***) Found: "
                    << context->utable->countUnifiers()
                    << " unification(s) (left). " << endl;

            int k = 0;
            unifierit ui, ue;
            ue = context->utable->getUnifierEnd();
            for (ui = context->utable->getUnifierBegin(); ui != ue; ui++, k++) {
                if (FLAG_VERBOSE > 1) {
                    *errflow << "Unification [" << k << "]:" << endl;
                    (*ui)->print(&*errflow);
                    *errflow << endl;
                }
                if (FLAG_DEBUG) {
                    os << "Unification: " << endl;
                    (*ui)->print(&os);
                    os << endl;
                    vus->push_back(os.str());
                }
            }
        } else if (FLAG_VERBOSE)
            *errflow << "(*** " << stack.size() << " ***) No preconditions."
                << endl;

        if (FLAG_DEBUG) {
            *errflow << "Selecting unification: " << endl;
            debugger->setOptions(vus);
        }

    }

    // @!@ Punto de Bactracking 4. Escoger de entre las posibles unificaciones
    if (context->utable) {
        stage = SelectUnification;
        context->unif = select->selectUnification(this);
    } else
        context->unif = -1;

    if (vus) {
        delete vus;
        vus = 0;
    }
    return true;
}

void Plan::setTimeVars(const Unifier * uf, const TPoints * tp,
        const Unifiable * t) {
    // Parche para permitir las consultas del ?start y del ?end
    //cerr << tp->first << " " << tp->second << endl;
    ldictionarycit f;
    pkey aux;

    if (tp->first > 0) {

        pkey s = t->getTermId("?start");
        // hay una variable con el nombre de ?start
        if (s.first < -1) {
            // A�adir a la termtable el nuevo s�mbolo
            // si no estaba ya previamente insertado
            std::stringstream ststr;
            ststr << "#" << tp->first;
            string st = ststr.str();
            if ((f = (domain->cdictionary).find(st.c_str()))
                    == (domain->cdictionary).end())
                // Si a�adimos el valor del tp start a la termtable.
                aux = parser_api->termtable->addConstant(st.c_str());
            else
                aux = make_pair((*f).second, 0);

            // Miramos si hay substituci�n en la tabla
            // de unificaciones.
            if (!uf || !uf->getSubstitution(s.first, &aux)) {
                // aplicar la substitucion
                ApplyVarSubstitution(0)(make_pair(s.first, aux));
                if (FLAG_VERBOSE >= 2) {
                    *errflow << "(ccc) ?start[" << s.first << "] is " << st
                        << endl;
                }
            }
        }
    }
    // lo mismo con el ?end
    if (tp->second > 0) {
        pkey e = t->getTermId("?end");
        if (e.first < -1) {
            // A�adir a la termtable el nuevo s�mbolo
            // si no estaba ya previamente insertado
            std::stringstream ststr;
            ststr << "#" << tp->second;
            string end = ststr.str();
            if ((f = (domain->cdictionary).find(end.c_str()))
                    == (domain->cdictionary).end())
                // Si a�adimos el valor del tp start a la termtable.
                aux = parser_api->termtable->addConstant(end.c_str());
            else
                aux = make_pair((*f).second, 0);
            if (!uf || !uf->getSubstitution(e.first, &aux)) {
                // aplicar la substitucion
                ApplyVarSubstitution(0)(make_pair(e.first, aux));
                if (FLAG_VERBOSE >= 2) {
                    *errflow << "(ccc) ?end[" << e.first << "] is " << end
                        << endl;
                }
            }
        }
    }
}

    StackNode * Plan::solve(StackNode * context) {
        if (FLAG_VERBOSE)
            *errflow << "(*** " << stack.size() << " ***) Depth: " << stack.size()
                << endl;
        if (FLAG_VERBOSE == 3)
            *errflow << "(*** " << stack.size() << " ***) Plan size: "
                << plan.size() << endl;
        if (FLAG_DEPTH_LIMIT > 0 && (int) stack.size() >= FLAG_DEPTH_LIMIT) {
            if (FLAG_VERBOSE)
                *errflow << "(### " << stack.size()
                    << " ###) FAIL!!: Maximun depth reached: "
                    << FLAG_DEPTH_LIMIT << endl;
            return 0;
        }

        // if(tasknetwork->getNumOfNodes() > 611){
        //     PrimitiveTask * t = (PrimitiveTask *) tasknetwork->getTask(611);
        //     TaskHeader * th = (TaskHeader *) t->ctparent;
        //     TPoints tps = th->getTPoints();
        //     int st = -(stp->Query(tps.first,0));
        //     cerr << t->getId() << " --- " << st << " " << endl;
        //     t->printHead(&cerr);
        // }

        // *errflow << "GGGGGGGGGGGGGGGGGGGGGGGGGOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO" << endl;
        // tasknetwork->printDebug(&*errflow);
        // *errflow << "GGGGGGGGGGGGGGGGGGGGGGGGGOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO" << endl;

        //parser_api->termtable->print(0,&*errflow);

        while (true) {
            while (!selectUnification(context)) {
                if (!selectTask(context))
                    return 0;
            }
            try {
                // llegados a este punto seguro que tengo algo que hacer ;)
                Task * t = (*(context->offspring))[context->task];
                if (t->isPrimitiveTask()) {
                    PrimitiveTask * pt = (PrimitiveTask *) t;
                    if (context->unif != -1) {
                        Unifier * uf = context->utable->getUnifierAt(context->unif);
                        if (FLAG_VERBOSE) {
                            *errflow << "(ccc) Performing unification: " << endl;
                            uf->print(&*errflow);
                            *errflow << endl;
                        }
                        // Generar un nuevo contexto
                        StackNode * next = new StackNode((&context->agenda));
                        // Copiar los nodos que han sido descartados, para volver a considerarlos,
                        if (!context->explored.empty())
                            next->agenda.insert(next->agenda.end(),
                                    context->explored.begin(),
                                    context->explored.end() - 1);

                        // registrar de donde provengo
                        if (!pt->isInline()) {
                            TaskHeader * th = ((TaskHeader *) tasknetwork->getTask(context->taskid));
                            pt->setParent(th->getParent());
                            pt->setParentMethod(th->getParentMethod());
                            pt->setParentHeader(th);
                            assert(t != th);
                        }

                        // insertar en la red de tareas la tarea primitiva
                        next->oldsize = tasknetwork->getNumOfNodes();
                        tasknetwork->replacePrimitive(context->taskid, pt,
                                &(next->agenda), &(next->lAdded), &(next->lDeleted));
                        // aplicar unificacion.
                        uf->apply(&(context->undoApply));

                        // Aniadir los vinculos causales generados tras la unificacion
                        // y establecer quien es la accion consumidora.
                        uf->setCLConsumer(pt);
                        CLTable * clt = uf->getModifiableCLTable();
                        context->undoCL.push_back(causalTable.addCausalLinks(pt,clt));

                        // pt->print(&*errflow,0);
                        // aniadir la tarea primitiva al plan
                        plan.push_back(tasknetwork->getNumOfNodes() - 1);
                        //evaluar la duracion antes de aplicar los efectos
                        /***                    if(domain->isTimed()){
                          if(pt->isDurative()){
                        // evaluamos cual es su duracion.
                        pt->dur = (unsigned int) rint(pt->evalDuration(state,0));
                        }
                        else
                        pt->dur = 0;

                        if(FLAG_VERBOSE)
                         *errflow << "(*** " << stack.size() << " ***) Duration: " << pt->dur << endl;
                         } ***/

                        if (domain->isTimed() && (!context->intervals
                                    || context->intervals->empty())) {
                            if (!addPTConstraints(pt, context, uf)) {
                                if (FLAG_VERBOSE)
                                    *errflow << "(### " << stack.size()
                                        << " ###) FAIL!!: (I) during temporal constraint application. "
                                        << endl;
                                if (!domain->isTimed())
                                    next->undo(tasknetwork, &plan);
                                else
                                    next->undo(tasknetwork, &plan, stp);
                                delete next;
                                throw TException;
                            }
                            TPoints tps = pt->getTPoints();
                            setTimeVars(uf, &tps, (Unifiable *) pt);
                        }

                        // aplicar efectos
                        bool eret;
                        if (domain->isTimed()) {
                            eret = pt->applyEffects(state, &(next->stateChanges),
                                    uf);
                            if (!CLThreatAnalisys(pt, next)) {
                                if (FLAG_VERBOSE)
                                    *errflow << "(### " << stack.size()
                                        << " ###) FAIL!!: (II) during temporal constraint application. "
                                        << endl;
                                next->undo(tasknetwork, &plan, stp);
                                delete next;
                                throw TException;
                            }
                        }
                        else {
                            eret = pt->applyEffects(state, &(next->stateChanges), 0);
                        }
                        if (!eret) {
                            if (FLAG_VERBOSE) {
                                *errflow << "(### " << stack.size()
                                    << " ###) FAIL!!: (III) during effects aplication. "
                                    << endl;
                            }
                            // deshacer
                            if (!domain->isTimed())
                                next->undo(tasknetwork, &plan);
                            else
                                next->undo(tasknetwork, &plan, stp);
                            delete next;
                            throw TException;
                        } else {
                            if (FLAG_VERBOSE) {
                                for_each(next->stateChanges.begin(),
                                        next->stateChanges.end(),
                                        PrintUndoElementln(&*errflow));
                                if (FLAG_VERBOSE == 3) {
                                    *errflow << "(ccc) Creating new agenda:"
                                        << endl;
                                    for (VAgenda::iterator n = next->agenda.begin(); n
                                            != next->agenda.end(); n++)
                                        *errflow << " [" << (*n).first << ","
                                            << (*n).second << "]" << endl;
                                    *errflow << endl;
                                }
                            }
                            nodeCounter++;
                            // registrar el tamanio de la termtable
                            next->ttsize = (int) parser_api->termtable->variables.size();
                            // registrar que unificador utilice
                            pt->setUnifier(uf);
                            return next;
                        }
                    } else {
                        // Generar un nuevo contexto
                        StackNode * next = new StackNode(&(context->agenda));
                        // Copiar los nodos que han sido descartados, para volver a considerarlos,
                        if (!context->explored.empty())
                            next->agenda.insert(next->agenda.end(),
                                    context->explored.begin(),
                                    context->explored.end() - 1);

                        // registrar de donde provengo
                        if (!pt->isInline()) {
                            TaskHeader * th = ((TaskHeader *) tasknetwork->getTask(
                                        context->taskid));
                            pt->setParent(th->getParent());
                            pt->setParentMethod(th->getParentMethod());
                            pt->setParentHeader(th);
                            assert(t != th);
                        }

                        // insertar en la red de tareas la tarea primitiva
                        next->oldsize = tasknetwork->getNumOfNodes();
                        tasknetwork->replacePrimitive(context->taskid, pt,
                                &(next->agenda), &(next->lAdded), &(next->lDeleted));
                        // Aniadir los vinculos causales generados tras la unificacion
                        context->undoCL.push_back(causalTable.addCausalLinks(pt,0));
                        // aniadir la tarea primitiva al plan
                        plan.push_back(tasknetwork->getNumOfNodes() - 1);
                        //evaluar la duracion antes de aplicar los efectos
                        /***                    if(domain->isTimed()){
                          if(pt->isDurative()){
                        // evaluamos cual es su duracion.
                        pt->dur = (unsigned int) rint(pt->evalDuration(state,0));
                        }
                        else
                        pt->dur = 0;

                        if(FLAG_VERBOSE)
                         *errflow << "(*** " << stack.size() << " ***) Duration: " << pt->dur << endl;
                         } ***/

                        if (domain->isTimed() && (!context->intervals
                                    || context->intervals->empty())) {
                            if (!addPTConstraints(pt, context, 0)) {
                                if (FLAG_VERBOSE)
                                    *errflow << "(### " << stack.size()
                                        << " ###) FAIL!!: (IV) during temporal constraint application. "
                                        << endl;
                                if (!domain->isTimed())
                                    next->undo(tasknetwork, &plan);
                                else
                                    next->undo(tasknetwork, &plan, stp);
                                delete next;
                                throw TException;
                            }
                            TPoints tps = pt->getTPoints();
                            setTimeVars(0, &tps, (Unifiable *) pt);
                        }

                        // aplicar efectos
                        if (!pt->getPrecondition()) {
                            bool eret;
                            //Unifier * uf = 0;
                            if (domain->isTimed()) {
                                eret = pt->applyEffects(state, &(next->stateChanges), 0);
                                if (!CLThreatAnalisys(pt, next)) {
                                    if (FLAG_VERBOSE)
                                        *errflow << "(### " << stack.size()
                                            << " ###) FAIL!!: (V) during temporal constraint application. "
                                            << endl;
                                    next->undo(tasknetwork, &plan, stp);
                                    delete next;
                                    throw TException;
                                }
                            }
                            else {
                                eret = pt->applyEffects(state, &(next->stateChanges), 0);
                            }
                            if (!eret) {
                                if (FLAG_VERBOSE) {
                                    *errflow << "(### " << stack.size()
                                        << " ###) FAIL!!: during effects aplication. "
                                        << endl;
                                }
                                // deshacer
                                if (!domain->isTimed())
                                    next->undo(tasknetwork, &plan);
                                else
                                    next->undo(tasknetwork, &plan, stp);
                                delete next;
                                throw TException;
                            }
                            else {
                                if (FLAG_VERBOSE) {
                                    for_each(next->stateChanges.begin(),
                                            next->stateChanges.end(),
                                            PrintUndoElementln(&*errflow));
                                    if (FLAG_VERBOSE == 3) {
                                        *errflow << "(ccc) Creating new agenda:"
                                            << endl;
                                        for (VAgenda::iterator n =
                                                next->agenda.begin(); n
                                                != next->agenda.end(); n++)
                                            *errflow << " [" << (*n).first << ","
                                                << (*n).second << "]" << endl;
                                        *errflow << endl;
                                    }
                                }
                                nodeCounter++;
                                // registrar el tamanio de la termtable
                                next->ttsize
                                    = (int) parser_api->termtable->variables.size();
                                return next;
                            }
                        }
                    }
                }
                else {
                    Method * met = (*(context->methods))[context->mpos];

                    if (context->unif != -1) {
                        Unifier * uf = context->utable->getUnifierAt(context->unif);
                        if (FLAG_VERBOSE) {
                            *errflow << "(ccc) Performing unification: " << endl;
                            uf->print(&*errflow);
                            *errflow << endl;
                        }

                        // Generar un nuevo contexto
                        StackNode * next = new StackNode(&(context->agenda));
                        // Copiar los nodos que han sido descartados, para volver a considerarlos,
                        if (!context->explored.empty())
                            next->agenda.insert(next->agenda.end(),
                                    context->explored.begin(),
                                    context->explored.end() - 1);

                        // Reemplazamos la tarea (abstracta) por la red de tareas correspondiente
                        // al m�todo.
                        // Actualizo la agenda con las nuevas tareas a�adidas
                        next->oldsize = tasknetwork->getNumOfNodes();
                        tasknetwork->replaceTN(context->taskid,
                                met->getTaskNetwork(), &(next->agenda),
                                &(next->lAdded), &(next->lDeleted));
                        // aplicamos la unificaci�n
                        /**errflow << "Antes-------------------------" << endl;
                          met->print(&*errflow);*/
                        uf->apply(&(context->undoApply));

                        // A�adir los v�nculos causales generados tras la unificaci�n
                        // y establecer quien es la acci�n consumidora.
                        //uf->setCLConsumer(t);
                        //const CLTable * clt = uf->getCLTable();
                        //context->undoApply.push_back(causalTable.addCausalLinks(t,clt));

                        /**errflow << "Despues-------------------------" << endl;
                          met->print(&*errflow);
                         *errflow << "-------------------------" << endl;*/
                        nodeCounter++;
                        eCounter++;
                        // si la tarea compuesta tiene un corte, descartamos el resto
                        // de metodos que queden por aplicar
                        if (((CompoundTask *) t)->isFirst()) {
                            context->methods->clear();
                            context->methods->push_back(met);
                            context->mpos = 0;
                        }
                        // registrar de donde provengo
                        TaskHeader * th = ((TaskHeader *) tasknetwork->getTask(
                                    context->taskid));
                        t->setParent(th->getParent());
                        t->setParentMethod(th->getParentMethod());
                        t->setParentHeader(th);
                        assert(t != th);

                        // para cada uno de mis hijos, digo que yo soy el padre.
                        for_each(met->getTaskNetwork()->getBeginTask(),
                                met->getTaskNetwork()->getEndTask(), MarkParent(t,
                                    met));
                        if (FLAG_VERBOSE == 3) {
                            *errflow << "(ccc) Creating new agenda:" << endl;
                            for (VAgenda::iterator n = next->agenda.begin(); n
                                    != next->agenda.end(); n++)
                                *errflow << " [" << (*n).first << ","
                                    << (*n).second << "]" << endl;
                            *errflow << endl;
                        }
                        // registrar el tamanio de la termtable
                        next->ttsize = (int) parser_api->termtable->variables.size();
                        if (domain->isTimed()) {
                            if (!addCTConstraints((CompoundTask *) t, met, context)) {
                                if (FLAG_VERBOSE) {
                                    *errflow << "(### " << stack.size()
                                        << " ###) FAIL!!: (VI) during temporal constraint application. "
                                        << endl;
                                }
                                if (!domain->isTimed()) {
                                    next->undo(tasknetwork, &plan);
                                }
                                else {
                                    next->undo(tasknetwork, &plan, stp);
                                }
                                delete next;
                                throw TException;
                            }
                            TPoints tps = ((CompoundTask *) t)->getTPoints();
                            setTimeVars(uf, &tps, (Unifiable *) met);
                        }
                        // registrar que unificador utilizo
                        t->setUnifier(uf);
                        return next;
                    } else {
                        // Generar un nuevo contexto
                        StackNode * next = new StackNode(&(context->agenda));
                        // Copiar los nodos que han sido descartados, para volver a considerarlos,
                        if (!context->explored.empty())
                            next->agenda.insert(next->agenda.end(),
                                    context->explored.begin(),
                                    context->explored.end() - 1);

                        next->oldsize = tasknetwork->getNumOfNodes();
                        tasknetwork->replaceTN(context->taskid,
                                met->getTaskNetwork(), &(next->agenda),
                                &(next->lAdded), &(next->lDeleted));

                        nodeCounter++;
                        eCounter++;
                        // si la tarea compuesta tiene un corte, descartamos el resto
                        // de metodos que queden por aplicar
                        if (((CompoundTask *) t)->isFirst()) {
                            context->methods->clear();
                            context->methods->push_back(met);
                            context->mpos = 0;
                        }
                        // registrar de donde provengo
                        TaskHeader * th = ((TaskHeader *) tasknetwork->getTask(
                                    context->taskid));
                        t->setParent(th->getParent());
                        t->setParentMethod(th->getParentMethod());
                        t->setParentHeader(th);
                        assert(t != th);

                        // para cada uno de mis hijos, digo que yo soy el padre.
                        for_each(met->getTaskNetwork()->getBeginTask(),
                                met->getTaskNetwork()->getEndTask(), MarkParent(t,
                                    met));
                        if (FLAG_VERBOSE == 3) {
                            *errflow << "(ccc) Creating new agenda:" << endl;
                            for (VAgenda::iterator n = next->agenda.begin(); n
                                    != next->agenda.end(); n++)
                                *errflow << " [" << (*n).first << ","
                                    << (*n).second << "]" << endl;
                            *errflow << endl;
                        }
                        // registrar el tamanio de la termtable
                        next->ttsize
                            = (int) parser_api->termtable->variables.size();
                        if (domain->isTimed()) {
                            if (!addCTConstraints((CompoundTask *) t, met, context)) {
                                if (FLAG_VERBOSE)
                                    *errflow << "(### " << stack.size()
                                        << " ###) FAIL!!: (VII) during temporal constraint application. "
                                        << endl;
                                if (!domain->isTimed())
                                    next->undo(tasknetwork, &plan);
                                else
                                    next->undo(tasknetwork, &plan, stp);
                                delete next;
                                throw TException;
                            }
                            TPoints tps = ((CompoundTask *) t)->getTPoints();
                            setTimeVars(0, &tps, (Unifiable *) met);
                        }
                        return next;
                    }
                }
            } catch (Temporal_Exception& e) {
            };
        } // del while
        return 0;
    }

bool Plan::addInmediateTConstraints(int miindex, TPoints & metp) {
    // en la red de tareas el índice en el vector de tareas no
    // corresponde con el de predecesoras y sucesoras
    // por eso sumo uno
    intvit b = tasknetwork->getPredBegin(miindex +1);
    intvit e = tasknetwork->getPredEnd(miindex +1);
    //cerr << "Aniadiento TPoints para inmediata " << miindex << endl;
    // tasknetwork->getTask(miindex)->printHead(&cerr);
    //tasknetwork->printDebug(&cerr);
    const Task * ref;
    TPoints otps;
    while (b != e) {
        // llegados a este punto todas mis predecesoras deben
        // ser tareas primitivas, las tareas abstractas ya se
        // habrán expandido ¿? el inmediate los obliga

        // para corregir la asimetria entre los vectores
        // de tareas con los de pred y succ le resto uno al indice
        if(*b > 0){
            ref = (const Task *) tasknetwork->getTask(*b -1);
            //cerr << "-->";
            //ref->printHead(&cerr);
            if (ref->isPrimitiveTask()) {
                PrimitiveTask * pt = (PrimitiveTask *) ref;

                if (pt->isInline()){
                    // si es una inline los enlaces tienen que hacerse con las predecesoras de
                    // la inline ya que la inline no genera TPs
                    addInmediateTConstraints(*b -1, metp);
                }
                else {
                    otps = ((PrimitiveTask *) ref)->getTPoints();
                    if (!stp->AddTConstraint2(otps.second, metp.first, 0, 0)) {
                        if (FLAG_VERBOSE)
                            *errflow << "(### " << stack.size() << " ###) Error: Making inmediate temporal constraints." << endl;
                        return false;
                    }
                }
            }
            else
                assert(false);
        }
        else {
            // se generan los enlaces con el inicio del plan
            if (!stp->AddTConstraint2(0, metp.first, 0, 0)) {
                if (FLAG_VERBOSE)
                    *errflow << "(### " << stack.size() << " ###) Error: Making inmediate temporal constraints." << endl;
                return false;
            }
        }
        b++;
    }
    return true;
};

bool Plan::addPTConstraints(PrimitiveTask * pt, StackNode * context,
        const Unifier * uf) {
    if (!pt->getInline()) {
        //cerr << "Procesando primitiva: ";
        //pt->printHead(&cerr);
        context->tcnStackSize = stp->getUndoLevels();
        stp->SetLevel();
        if (FLAG_VERBOSE) {
            *errflow << "(*** " << stack.size()
                << " ***) Saving Simple Temporal Network: "
                << stp->getUndoLevels() << endl;
        }
        if (!generateTPoints(pt))
            return false;
        TPoints metp = pt->getTPoints();
        // aniadir la restriccion de duracion
        if (pt->isDurative()) {
            // evaluamos las expresiones de duracion
            EvaluatedTCs * dexp = 0;
            EvaluatedTCs::iterator i, e;
            dexp = pt->evalDurationExp(state, uf);
            if (!dexp) {
                if (FLAG_VERBOSE)
                    *errflow << "(### " << stack.size()
                        << " ###) Error evaluating duration expressions."
                        << endl;
                return false;
            }
            if (FLAG_VERBOSE) {
                *errflow << "(*** " << stack.size() << " ***) Duration:";
                for_each(dexp->begin(), dexp->end(), PrintCTE(errflow, 1));
                *errflow << endl;
            }
            //cerr << "aplicando" << endl;
            if (!applyDConstraints(dexp, metp)) {
                delete dexp;
                if (FLAG_VERBOSE)
                    *errflow << "(### " << stack.size()
                        << " ###) Error applying duration temporal constraints."
                        << endl;
                return false;
            }
            delete dexp;
        }

        //cerr << "pase" << endl;
        // hacer mi ts y mi te relativos a la tc de la que provengo
        // si es necesario
        CompoundTask * t = pt->getParent();
        Method * m = pt->getParentMethod();
        if (t) {
            TPoints ptpoint = t->getTPoints();
            // Si mi padre genero time points, me hago depdender de ellos.
            if (ptpoint.first > 0) {
                //cerr << "Aniadiendo vinculos" << endl;
                //pt->printHead(&cerr);
                //cerr << endl;
                //th->print(&cerr);
                //cerr << endl;
                //vinculos causales con la tarea que me expandio
                //cerr << ptpoint.first << " " << metp.first << endl;
                if (!stp->AddTConstraint2(ptpoint.first, metp.first, 0, POS_INF)) {
                    //assert(false);
                    if (FLAG_VERBOSE)
                        *errflow << "(### " << stack.size()
                            << " ###) Error: Restrincting my duration to my parent duration."
                            << endl;
                    return false;
                }
                //cerr << metp.second  << " " << ptpoint.second << endl;
                if (!stp->AddTConstraint2(metp.second, ptpoint.second, 0,
                            POS_INF)) {
                    //assert(false);
                    if (FLAG_VERBOSE)
                        *errflow << "(### " << stack.size()
                            << " ###) Error: Restrincting my duration to my parent duration."
                            << endl;
                    return false;
                }
                //cerr << "done!" << endl;
            }
        }

        // Obtener las tareas que me preceden y
        // que me suceden
        // hacer vinculos de orden con ellas, cuando
        // sea posible.
        //cerr << "Fabricando enlaces" << endl;
        if (!makeOrderLinks(context->taskid, metp)) {
            //assert(false);
            if (FLAG_VERBOSE)
                *errflow << "(### " << stack.size()
                    << " ###) Error: Stablishing causal link time constraints"
                    << endl;
            return false;
        }
        //cerr << "done!" << endl;

        //cerr << "Analisis de restricciones" << endl;
        // Analisis de restriciones

        // si soy una tarea inmediata
        // mi start se une a los end de mis tareas predecesoras
        // accedo a la red de tareas global, no a la red de tareas
        // local de mi método.
        // mi padre context->taskid del cual me expando, es el que
        // tiene la marca de inmediato si la hay
        if (tasknetwork->isInmediate(context->taskid)) {
            int miindex = tasknetwork->getIndexOf(pt);
            if (!addInmediateTConstraints(miindex,metp)) {
                return false;
            }
        }

        const vector<TCTR> * v = 0;
        if (m) {
            int index = m->getTaskNetwork()->getIndexOf(pt->getParentHeader());
            v = m->getTaskNetwork()->getTConstraints(index);
        } else
            v = tasknetwork->getTConstraints(context->taskid);

        if (!applyNetConstraints(v, metp)) {
            if (FLAG_VERBOSE)
                *errflow << "(### " << stack.size()
                    << " ###) Error: Appliying user defined temporal constraints"
                    << endl;
            return false;
        }

        if (!causalLinkAnalisys(pt, context)) {
            if (FLAG_VERBOSE)
                *errflow << "(### " << stack.size()
                    << " ###) Error: In causal link analysis" << endl;
            return false;;
        }
    }

    return true;
}
;

bool Plan::causalLinkAnalisys(PrimitiveTask * pt, StackNode * context) {

    // sacar los time-points correspondientes a la tarea.

    TPoints metp = pt->getTPoints();

    // Obtenemos los v�nculos causales que son consumidos por esta
    // tarea de la tabla de v.c. Los recorremos y vamos a�adiendo
    // las restricciones pertinentes.
    const CLTable * clt = causalTable.getConsumedLinks(pt);
    cltcite bc, ec;
    if (clt) {
        ec = clt->end();
        //Recorremos los v�nculos causales que no forman parte del timeline
        for (bc = clt->begin(); bc != ec; bc++) {
            if (((*bc)->isNormalLink()) || (*bc)->isFluentLink()) {
                // La tarea que produjo el link
                const PrimitiveTask * pt2 = (*bc)->getProducer();
                // El instante de tiempo en el cual se consigue el efecto
                int res = (int) rint((*bc)->getTime());

                if (pt2) {

                    //pt2->printHead(&cerr);
                    //(*bc)->getLiteral()->print(&cerr);
                    //cerr << endl;

                    if (res < 0)
                        // por defecto los efectos se consiguien Atend
                        res = ATEND;
                    if (res >= ATEND) {
                        //cerr << pt2->getTPoints().first << " " << metp.first << " " << pt2->dur << " oo" << endl;
                        if (!stp->AddTConstraint2(pt2->getTPoints().second,
                                    metp.first, 0, POS_INF)) {
                            return false;
                        }
                    } else {
                        //cerr << pt2->getTPoints().first << " " << metp.first << " " << res << " oo" << endl;
                        if (!stp->AddTConstraint2(pt2->getTPoints().first,
                                    metp.first, res, POS_INF)) {
                            return false;
                        }
                    }
                } else {
                    // Anclamos conta el estado inicial

                    //cerr << "init" << endl;
                    //(*bc)->getLiteral()->print(&cerr);
                    //cerr << endl;

                    if (res < 0 || res == ATEND)
                        res = 0;
                    //cerr << "0 " << metp.first << " " << res << " oo" << endl;
                    if (!stp->AddTConstraint2(0, metp.first, res, POS_INF)) {
                        //assert(false);
                        return false;
                    }
                }
            }
        }

        //cerr << "timeline" << endl;
        if (!stp->Closure()) {
            //assert(false);
            return false;
        }

        // El ts de la acci�n ya est� fijado
        // recorremos ahora los v�nculos causales que forman parte del timeline
        // jugamos con ellos para encontrar un nuevo ts de la acci�n en el cual
        // todos los timeline sean ciertos simult�neamente.
        context->intervals = new VIntervals();
        int max, min;
        if (problem->MAX_THORIZON)
            max = problem->MAX_THORIZON;
        else
            // este no es un buen l�mite, hacer depender del tiempo de ejecuci�n de la acci�n
            max = abs(stp->Query(metp.second, 0)) + problem->REL_THORIZON;

        min = abs(stp->Query(metp.first, 0));

        for (bc = clt->begin(); bc != ec; bc++) {
            if ((*bc)->isTimeLineLink()) {
                //stp->PrintValues(cerr);
                //cerr << pt->tpoints.first << endl;
                //(*bc).getLiteral()->print(&cerr);
                //cerr << min << " **************************++ " << max << endl;
                ((TimeLineLiteralEffect *) (*bc)->getModificableLiteral())->merge(
                *(context->intervals), min, max);
                if (context->intervals->empty()) {
                    //cerr << "Fallo el merge" << endl;
                    //assert(false);
                    return false;
                }
            }
        }

        // guardar el tama�o de la red de tareas antes de a�adir las restricciones
        // provocadas por el timeline
        if (!context->intervals->empty()) {
            if ((context->tcnpos = searchTimeLine(context,
                            *(context->intervals), 0)) == -1) {
                //cerr << "Fallo en el searchTimeLine" << endl;
                //assert(false);
                if (FLAG_VERBOSE)
                    *errflow << "(### " << stack.size()
                        << " ###) FAIL!!: No more slots in time-line."
                        << endl;
                delete context->intervals;
                context->intervals = 0;
                return false;
            }
            if (FLAG_VERBOSE)
                *errflow << "(*** " << stack.size()
                    << " ***) New time-line slot: " << context->tcnpos
                    << " from " << context->intervals->size() << "."
                    << endl;
        }
    }

    return true;
}

bool Plan::CLThreatAnalisys(PrimitiveTask * pt, StackNode * context) {
    // Miramos ahora si la accion suprime literales del estado.
    // En el caso de que lo haga, es necesario aniadir restricciones para
    // que la accion no se ejecute antes que otras acciones en el plan que necesitan
    // de ese literal. Esto evita que el proceso de paralelizacion coloque esta accion
    // antes que las otras en el tiempo.
    undoit i, e = context->stateChanges.end();
    const Literal * l;
    const LiteralEffect * le;
    // Tabla de vinculos causales
    const CLTable * t;
    cltcite j, te;
    int res;
    TPoints ta, tb;

    ta = pt->getTPoints();
    //pt->print(&cerr);
    //cerr << ta.first << " " << ta.second << endl;
    //cerr << endl;

    for (i = context->stateChanges.begin(); i != e; i++) {
        if ((*i)->isUndoARLiteralState()) {
            if (!((UndoARLiteralState *) *i)->wasAdded()) {
                l = ((UndoARLiteralState *) *i)->getLiteral();
                // Buscar en la tabla de v.c, las tareas que son consumidoras de
                // ese literal.
                //l->printL(&cerr);
                //cerr << "  (l)  " << endl;
                t = causalTable.getLinks((LiteralEffect *) l);
                if (t) {
                    te = t->end();
                    for (j = t->begin(); j != te; j++) {
                        le = (*j)->getLiteral();
                        //le->printL(&cerr);
                        //cerr << "  (le)  " << endl;
                        if (le->getPol()) {
                            if (unify(l->getParameters(), le->getParameters())) {
                                tb
                                    = ((PrimitiveTask *) (*j)->getConsumer())->getTPoints();
                                if (!((PrimitiveTask *) (*j)->getConsumer())->isInline()) {
                                    res = (int) rint(
                                            ((UndoARLiteralState *) *i)->time);
                                    // mirar si el efecto es atstart o atend
                                    if (res < 0 || res >= ATEND) {
                                        // El efecto es ATEND
                                        // Ahora mirar si la precondici�n de la
                                        // tarea consumidora del v�nculo es una
                                        // overall
                                        if ((*j)->getProtectionFirst()
                                                == (*j)->getProtectionSecond()) {
                                            // Es una ATSTART
                                            if (pt->isInline()) {
                                                *errflow
                                                    << "Fatal error: Deleting a literal:"
                                                    << endl;
                                                le->printL(errflow);
                                                *errflow
                                                    << "inside a :inline action that is used by the action:"
                                                    << endl;
                                                ((PrimitiveTask *) (*j)->getConsumer())->printHead(
                                                errflow);
                                                *errflow
                                                    << "This causes a theat in the causal link structure that can not be solved.";
                                                *errflow
                                                    << " :inline actions should be used for control flow only."
                                                    << endl;
                                                exit(EXIT_FAILURE);
                                            } else if (!stp->AddTConstraint2(
                                                        tb.first, ta.second, 0,
                                                        POS_INF)) {
                                                return false;
                                            }
                                        } else {
                                            // Es un overall o algo parecido
                                            if (pt->isInline()) {
                                                *errflow
                                                    << "Fatal error: Deleting a literal:"
                                                    << endl;
                                                le->printL(errflow);
                                                *errflow
                                                    << "inside a :inline action that is used by the action:"
                                                    << endl;
                                                ((PrimitiveTask *) (*j)->getConsumer())->printHead(
                                                errflow);
                                                *errflow
                                                    << "This causes a theat in the causal link structure that can not be solved.";
                                                *errflow
                                                    << " :inline actions should be used for control flow only."
                                                    << endl;
                                                exit(EXIT_FAILURE);
                                            } else if (!stp->AddTConstraint2(
                                                        tb.second, ta.second, 0,
                                                        POS_INF)) {
                                                return false;
                                            }
                                        }
                                    } else {
                                        // El efecto es res unidades despu�s
                                        // del START
                                        // Ahora mirar si la precondici�n de la
                                        // tarea consumidora del v�nculo es una
                                        // overall
                                        if ((*j)->getProtectionFirst()
                                                == (*j)->getProtectionSecond()) {
                                            // Es una ATSTART
                                            if (pt->isInline()) {
                                                *errflow
                                                    << "Fatal error: Deleting a literal:"
                                                    << endl;
                                                le->printL(errflow);
                                                *errflow
                                                    << "inside a :inline action that is used by the action:"
                                                    << endl;
                                                ((PrimitiveTask *) (*j)->getConsumer())->printHead(
                                                errflow);
                                                *errflow
                                                    << "This causes a theat in the causal link structure that can not be solved.";
                                                *errflow
                                                    << " :inline actions should be used for control flow only."
                                                    << endl;
                                                exit(EXIT_FAILURE);
                                            } else if (!stp->AddTConstraint2(
                                                        tb.first, ta.first, res,
                                                        POS_INF)) {
                                                return false;
                                            }
                                        } else {
                                            // Es un overall o algo parecido
                                            if (pt->isInline()) {
                                                *errflow
                                                    << "Fatal error: Deleting a literal:"
                                                    << endl;
                                                le->printL(errflow);
                                                *errflow
                                                    << "inside a :inline action that is used by the action:"
                                                    << endl;
                                                ((PrimitiveTask *) (*j)->getConsumer())->printHead(
                                                errflow);
                                                *errflow
                                                    << "This causes a theat in the causal link structure that can not be solved.";
                                                *errflow
                                                    << " :inline actions should be used for control flow only."
                                                    << endl;
                                                exit(EXIT_FAILURE);
                                            } else if (!stp->AddTConstraint2(
                                                        tb.second, ta.first, res,
                                                        POS_INF)) {
                                                return false;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                // El efecto es a�adido al estado. Hay que mirar si hay
                // alguna precondici�n que tenga el literal negado y
                // a�adir las restricciones de orden correspondientes.
                l = ((UndoARLiteralState *) *i)->getLiteral();
                // Buscar en la tabla de v.c, las tareas que son consumidoras de
                // ese literal.
                //l->printL(&cerr);
                //cerr << "  (l)  " << endl;
                //causalTable.print(&cerr);
                t = causalTable.getLinks((LiteralEffect *) l);
                if (t) {
                    te = t->end();
                    for (j = t->begin(); j != te; j++) {
                        le = (*j)->getLiteral();
                        //le->printL(&cerr);
                        //cerr << "  (le)  " << endl;
                        if (!le->getPol()) {
                            if (unify(l->getParameters(), le->getParameters())) {
                                tb
                                    = ((PrimitiveTask *) (*j)->getConsumer())->getTPoints();
                                if (!((PrimitiveTask *) (*j)->getConsumer())->isInline()) {
                                    res = (int) rint(
                                            ((UndoARLiteralState *) *i)->time);
                                    // mirar si el efecto es atstart o atend
                                    if (res < 0 || res >= ATEND) {
                                        // El efecto es ATEND
                                        // Ahora mirar si la precondici�n de la
                                        // tarea consumidora del v�nculo es una
                                        // overall
                                        if ((*j)->getProtectionFirst()
                                                == (*j)->getProtectionSecond()) {
                                            // Es una ATSTART
                                            if (pt->isInline()) {
                                                *errflow
                                                    << "Fatal error: Adding a literal:"
                                                    << endl;
                                                le->printL(errflow);
                                                *errflow
                                                    << "inside a :inline action that is deleted in the action:"
                                                    << endl;
                                                ((PrimitiveTask *) (*j)->getConsumer())->printHead(
                                                errflow);
                                                *errflow
                                                    << "This causes a theat in the causal link structure that can not be solved.";
                                                *errflow
                                                    << " :inline actions should be used for control flow only."
                                                    << endl;
                                                exit(EXIT_FAILURE);
                                            } else if (!stp->AddTConstraint2(
                                                        tb.first, ta.second, 0,
                                                        POS_INF)) {
                                                return false;
                                            }
                                        } else {
                                            // Es un overall o algo parecido
                                            if (pt->isInline()) {
                                                *errflow
                                                    << "Fatal error: Adding a literal:"
                                                    << endl;
                                                le->printL(errflow);
                                                *errflow
                                                    << "inside a :inline action that is deleted in the action:"
                                                    << endl;
                                                ((PrimitiveTask *) (*j)->getConsumer())->printHead(
                                                errflow);
                                                *errflow
                                                    << "This causes a theat in the causal link structure that can not be solved.";
                                                *errflow
                                                    << " :inline actions should be used for control flow only."
                                                    << endl;
                                                exit(EXIT_FAILURE);
                                            } else if (!stp->AddTConstraint2(
                                                        tb.second, ta.second, 0,
                                                        POS_INF)) {
                                                return false;
                                            }
                                        }
                                    } else {
                                        // El efecto es res unidades despu�s
                                        // del START
                                        // Ahora mirar si la precondici�n de la
                                        // tarea consumidora del v�nculo es una
                                        // overall
                                        if ((*j)->getProtectionFirst()
                                                == (*j)->getProtectionSecond()) {
                                            // Es una ATSTART
                                            if (pt->isInline()) {
                                                *errflow
                                                    << "Fatal error: Adding a literal:"
                                                    << endl;
                                                le->printL(errflow);
                                                *errflow
                                                    << "inside a :inline action that is deleted in the action:"
                                                    << endl;
                                                ((PrimitiveTask *) (*j)->getConsumer())->printHead(
                                                errflow);
                                                *errflow
                                                    << "This causes a theat in the causal link structure that can not be solved.";
                                                *errflow
                                                    << " :inline actions should be used for control flow only."
                                                    << endl;
                                                exit(EXIT_FAILURE);
                                            } else if (!stp->AddTConstraint2(
                                                        tb.first, ta.first, res,
                                                        POS_INF)) {
                                                return false;
                                            }
                                        } else {
                                            // Es un overall o algo parecido
                                            if (pt->isInline()) {
                                                *errflow
                                                    << "Fatal error: Adding a literal:"
                                                    << endl;
                                                le->printL(errflow);
                                                *errflow
                                                    << "inside a :inline action that is deleted in the action:"
                                                    << endl;
                                                ((PrimitiveTask *) (*j)->getConsumer())->printHead(
                                                errflow);
                                                *errflow
                                                    << "This causes a theat in the causal link structure that can not be solved.";
                                                *errflow
                                                    << " :inline actions should be used for control flow only."
                                                    << endl;
                                                exit(EXIT_FAILURE);
                                            } else if (!stp->AddTConstraint2(
                                                        tb.second, ta.first, res,
                                                        POS_INF)) {
                                                return false;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool Plan::generateTPoints(Task * t) {
    unsigned int s = stp->Size();

    TPoints p;
    bool generados = false;

    if (t->isPrimitiveTask()) {
        // obtener los timepoints antiguos y comprobar
        // que efectivamente no han sido ya generados
        p = ((PrimitiveTask *) t)->getTPoints();
        if (p.first == 0 || p.first >= s) {
            ((PrimitiveTask *) t)->setTPoints(s, s + 1);
            p = ((PrimitiveTask *) t)->getTPoints();
            generados = true;
        }
    } else if (t->isCompoundTask()) {
        p = ((CompoundTask *) t)->getTPoints();
        if (p.first == 0 || p.first >= s) {
            ((CompoundTask *) t)->setTPoints(s, s + 1);
            p = ((CompoundTask *) t)->getTPoints();
            generados = true;
        }
    } else {
        *errflow
            << "Plan::getnerateTPoints Unable to assign timepoints to this type of task!"
            << endl;
        exit(-1);
    }
    if (generados) {
        s += 2;
#ifdef __tcnm_pc2_cl
        stp->InsertTPoint(t);
        stp->InsertTPoint(t);
#else
        stp->InsertTPoint();
        stp->InsertTPoint();
#endif
        //cerr << "Generando time-points para: ";
        //th->print(&cerr);
        //cerr << "[" << p.first << "," << p.second << "]" <<  endl;

        if (!stp->AddTConstraint2(p.first, p.second, 0, POS_INF)) {
            //cerr << "Vinculando start y end" << endl;
            return false;
        }

        // hacer depender de los puntos de inicio y fin del plan
        if (!stp->AddTConstraint2(0, p.first, 0, POS_INF)) {
            //cerr << "Vinculando con inicio plan" << endl;
            return false;
        }

        if (!stp->AddTConstraint2(p.second, 1, 0, POS_INF)) {
            //cerr << "Vinculando con fin plan" << endl;
            return false;
        }
    }
    return true;
}
;

bool Plan::applyNetConstraints(const vector<TCTR> * v, TPoints metp) {
    // fijar las restricciones impuestas sobre la duraci�n de las tareas objetivo.
    //
    vector<TCTR>::const_iterator i, e;
    pkey res;
    if (v) {
        e = v->end();
        for (i = v->begin(); i != e; i++) {
            pkey tpref(-1, 0);
            bool b = true;

            res = (*i).second->evaltp(state, 0, &tpref, &b);
            //cerr << (*i).first << " -- " << res.second << endl;
            switch ((*i).first) {
                //////////////////////////////////////////////////////////////////////////////////////
                //Operaciones con DURATION
                //////////////////////////////////////////////////////////////////////////////////////
                case EQ_DUR:
                    // (= ?duration res)
                    if (tpref.first != -1) {
                        *errflow << "Unable to evaluate time point expression."
                            << endl;
                        PrintCT pct(errflow, 0);
                        pct(*i);
                        *errflow << endl;
                        return false;
                    } else if (!stp->AddTConstraint2(metp.first, metp.second,
                                (int) res.second, (int) res.second))
                        return false;
                    break;
                case LEQ_DUR:
                    // (<= ?duration res)
                    if (tpref.first != -1) {
                        *errflow << "Unable to evaluate time point expression."
                            << endl;
                        PrintCT pct(errflow, 0);
                        pct(*i);
                        *errflow << endl;
                        return false;
                    } else if (!stp->AddTConstraint2(metp.first, metp.second, 0,
                                (int) res.second))
                        return false;
                    break;
                case LESS_DUR:
                    // (< ?duration res)
                    if (tpref.first != -1) {
                        *errflow << "Unable to evaluate time point expression."
                            << endl;
                        PrintCT pct(errflow, 0);
                        pct(*i);
                        *errflow << endl;
                        return false;
                    } else if (!stp->AddTConstraint2(metp.first, metp.second, 0,
                                (int) res.second - 1))
                        return false;
                    break;
                case GEQ_DUR:
                    // (>= ?duration res)
                    if (tpref.first != -1) {
                        *errflow << "Unable to evaluate time point expression."
                            << endl;
                        PrintCT pct(errflow, 0);
                        pct(*i);
                        *errflow << endl;
                        return false;
                    } else if (!stp->AddTConstraint2(metp.first, metp.second,
                                (int) res.second, POS_INF))
                        return false;
                    break;
                case GRE_DUR:
                    // (> ?duration res)
                    if (tpref.first != -1) {
                        *errflow << "Unable to evaluate time point expression."
                            << endl;
                        PrintCT pct(errflow, 0);
                        pct(*i);
                        *errflow << endl;
                        return false;
                    } else if (!stp->AddTConstraint2(metp.first, metp.second,
                                (int) res.second + 1, POS_INF))
                        return false;
                    break;
                    //////////////////////////////////////////////////////////////////////////////////////
                    //Operaciones con START
                    //////////////////////////////////////////////////////////////////////////////////////
                case EQ_START:
                    // (?start = ?tpres + res)
                    if (tpref.first != -1) {
                        int restp = extractTimePoint(tpref);
                        if (restp == -1) {
                            *errflow << "Invalid time point!!." << endl;
                            PrintCT pct(errflow, 0);
                            pct(*i);
                            *errflow << endl;
                            return false;
                        }
                        if (!stp->AddTConstraint2(restp, metp.first,
                                    (int) res.second, (int) res.second))
                            return false;
                    }
                    // (= ?start res)
                    else if (!stp->AddTConstraint2(0, metp.first, (int) res.second,
                                (int) res.second))
                        return false;
                    break;
                case LEQ_START:
                    // (?start <= ?tpres + res)
                    if (tpref.first != -1) {
                        int restp = extractTimePoint(tpref);
                        if (restp == -1) {
                            *errflow << "Invalid time point!!." << endl;
                            PrintCT pct(errflow, 0);
                            pct(*i);
                            *errflow << endl;
                            return false;
                        }
                        if (!stp->AddTConstraint2(metp.first, restp,
                                    (int) res.second, POS_INF))
                            return false;
                    }
                    // (<= ?start res)
                    else if (!stp->AddTConstraint2(0, metp.first, 0,
                                (int) res.second))
                        return false;
                    break;
                case LESS_START:
                    //cerr << "less_start" << endl;
                    // (?start < ?tpres + res)
                    if (tpref.first != -1) {
                        int restp = extractTimePoint(tpref);
                        if (restp == -1) {
                            *errflow << "Invalid time point!!." << endl;
                            PrintCT pct(errflow, 0);
                            pct(*i);
                            *errflow << endl;
                            return false;
                        }
                        //cerr << "arriba: " << res.second << endl;
                        if (!stp->AddTConstraint2(metp.first, restp,
                                    (int) res.second, POS_INF))
                            return false;
                    }
                    // (?start < res)
                    else if (!stp->AddTConstraint2(0, metp.first, 0,
                                (int) res.second - 1))
                        return false;
                    break;
                case GEQ_START:
                    // (?start >= ?tpres + res)
                    if (tpref.first != -1) {
                        int restp = extractTimePoint(tpref);
                        if (restp == -1) {
                            *errflow << "Invalid time point!!." << endl;
                            PrintCT pct(errflow, 0);
                            pct(*i);
                            *errflow << endl;
                            return false;
                        }
                        if (!stp->AddTConstraint2(restp, metp.first,
                                    (int) res.second, POS_INF))
                            return false;
                    }
                    // (>= ?start res)
                    else if (!stp->AddTConstraint2(0, metp.first, (int) res.second,
                                POS_INF))
                        return false;
                    break;
                case GRE_START:
                    //cerr << "gre_start" << endl;
                    // (?start > ?tpres + res)
                    if (tpref.first != -1) {
                        int restp = extractTimePoint(tpref);
                        if (restp == -1) {
                            *errflow << "Invalid time point!!." << endl;
                            PrintCT pct(errflow, 0);
                            pct(*i);
                            *errflow << endl;
                            return false;
                        }
                        if (!stp->AddTConstraint2(restp, metp.first,
                                    (int) res.second + 1, POS_INF))
                            return false;
                    }
                    // (> ?start res)
                    else if (!stp->AddTConstraint2(0, metp.first, (int) res.second
                                + 1, POS_INF))
                        return false;
                    break;
                    //////////////////////////////////////////////////////////////////////////////////////
                    //Operaciones con END
                    //////////////////////////////////////////////////////////////////////////////////////
                case EQ_END:
                    // (?end = ?tpres + res)
                    if (tpref.first != -1) {
                        int restp = extractTimePoint(tpref);
                        if (restp == -1) {
                            *errflow << "Invalid time point!!." << endl;
                            PrintCT pct(errflow, 0);
                            pct(*i);
                            *errflow << endl;
                            return false;
                        }
                        if (!stp->AddTConstraint2(metp.second, restp,
                                    (int) res.second, (int) res.second))
                            return false;
                    }
                    // (= ?end res)
                    else if (!stp->AddTConstraint2(0, metp.second,
                                (int) res.second, (int) res.second))
                        return false;
                    break;
                case LEQ_END:
                    // (?end <= ?tpres + res)
                    if (tpref.first != -1) {
                        int restp = extractTimePoint(tpref);
                        if (restp == -1) {
                            *errflow << "Invalid time point!!." << endl;
                            PrintCT pct(errflow, 0);
                            pct(*i);
                            *errflow << endl;
                            return false;
                        }
                        if (!stp->AddTConstraint2(metp.second, restp,
                                    (int) res.second, POS_INF))
                            return false;
                    }
                    // (<= ?end res)
                    else if (!stp->AddTConstraint2(0, metp.second, 0,
                                (int) res.second))
                        return false;
                    break;
                case LESS_END:
                    //cerr << "less_end" << endl;
                    // (?end < ?tpres + res)
                    if (tpref.first != -1) {
                        int restp = extractTimePoint(tpref);
                        if (restp == -1) {
                            *errflow << "Invalid time point!!." << endl;
                            PrintCT pct(errflow, 0);
                            pct(*i);
                            *errflow << endl;
                            return false;
                        }
                        if (!stp->AddTConstraint2(metp.second, restp,
                                    (int) res.second - 1, POS_INF))
                            return false;
                    }
                    // (< ?end res)
                    else if (!stp->AddTConstraint2(0, metp.second, 0,
                                (int) res.second - 1))
                        return false;
                    break;
                case GEQ_END:
                    // (?end >= ?tpres + res)
                    if (tpref.first != -1) {
                        int restp = extractTimePoint(tpref);
                        if (restp == -1) {
                            *errflow << "Invalid time point!!." << endl;
                            PrintCT pct(errflow, 0);
                            pct(*i);
                            *errflow << endl;
                            return false;
                        }
                        if (!stp->AddTConstraint2(restp, metp.second,
                                    (int) res.second, POS_INF))
                            return false;
                    }
                    // (>= ?end res)
                    else if (!stp->AddTConstraint2(0, metp.second,
                                (int) res.second, POS_INF))
                        return false;
                    break;
                case GRE_END:
                    //cerr << "gre_end" << endl;
                    // (?end > ?tpres + res)
                    if (tpref.first != -1) {
                        int restp = extractTimePoint(tpref);
                        if (restp == -1) {
                            *errflow << "Invalid time point!!." << endl;
                            PrintCT pct(errflow, 0);
                            pct(*i);
                            *errflow << endl;
                            return false;
                        }
                        //cerr << res.second << endl;
                        if (!stp->AddTConstraint2(restp, metp.second,
                                    (int) res.second + 1, POS_INF))
                            return false;
                    }
                    // (> ?end res)
                    else if (!stp->AddTConstraint2(0, metp.second, (int) res.second
                                + 1, POS_INF))
                        return false;
                    break;
            };
        }
    }
    return true;
}
;

bool Plan::applyDConstraints(const EvaluatedTCs * v, TPoints metp) {
    // fijar las restricciones impuestas sobre la duraci�n de las tareas
    // primitivas

    int value = 0;
    EvaluatedTCs::const_iterator i, e;
    if (v) {
        e = v->end();
        for (i = v->begin(); i != e; i++) {
            value = rint(i->second);
            switch (i->first) {
                ///////////////////////////////////////////////////////////////
                //Operaciones con DURATION
                ///////////////////////////////////////////////////////////////
                case EQ_DUR: // (= ?duration res)
                    if (!stp->AddTConstraint2(metp.first, metp.second, value, value))
                        return false;
                    break;
                case LEQ_DUR: // (<= ?duration res)
                    if (!stp->AddTConstraint2(metp.first, metp.second, 0, value))
                        return false;
                    break;
                case LESS_DUR: // (< ?duration res)
                    if (!stp->AddTConstraint2(metp.first, metp.second, 0, value - 1))
                        return false;
                    break;
                case GEQ_DUR:
                    if (!stp->AddTConstraint2(metp.first, metp.second, value,
                                POS_INF))
                        return false;
                    break;
                case GRE_DUR: // (> ?duration res)
                    if (!stp->AddTConstraint2(metp.first, metp.second, value + 1,
                                POS_INF))
                        return false;
                    break;
                default:
                    return false;
            }
        }
    }
    return true;
}
;

bool Plan::addCTConstraints(CompoundTask * ct, Method * met,
        StackNode * context) {
    TaskNetwork * tn;
    int miindex = context->taskid;
    // Estructuras para hacer el analisis de restriciones
    // Analisis de restriciones
    const vector<TCTR> * v = 0;

    // Inicializacion, capturar las estructuras de datos de las cuales dependo.
    //
    // mi padre, una taskheader
    CompoundTask * t = ct->getParent();
    // el metodo que me genero
    Method * m = ct->getParentMethod();

    // cerr << "Procesando compuesta: ";
    // ct->printHead(&cerr);
    context->tcnStackSize = stp->getUndoLevels();

    if (m) {
        // me genero durante una expansion
        tn = m->getTaskNetwork();
        v = tn->getTConstraints(tn->getIndexOf(ct->getParentHeader()));
        //m->getTaskNetwork()->printDebug(&cerr);
        //cerr << v->size() << " " << m <<  " " << index << endl;
    }
    else {
        // pertenezco a la rt inicial
        v = tasknetwork->getTConstraints(miindex);
    }

    // El vector v contiene las restricciones timed - goals sobre la tarea compuesta
    // si esta vacio no hay restricciones por lo que no generamos time-points
    if (!v->empty() || tasknetwork->isInmediate(miindex)) {
        stp->SetLevel();
        if (FLAG_VERBOSE) {
            *errflow << "(*** " << stack.size()
                << " ***) Saving Simple Temporal Network: "
                << stp->getUndoLevels() << endl;
        }

        // Generar los time-points para la tarea.
        if (!generateTPoints(ct)) {
            if (FLAG_VERBOSE) {
                *errflow << "Error generating time points" << endl;
            }
            return false;
        }
        TPoints metp = ct->getTPoints();

        // si soy una tarea inmediata
        // mi start se une a los end de mis tareas predecesoras
        if (tasknetwork->isInmediate(miindex)) {
            if (!addInmediateTConstraints(miindex,metp)) {
                return false;
            }
        }

        //cerr << "Tiene TPoints: " << metp.first << " " << metp.second << endl;
        //ct->printHead(&cerr);
        //cerr << endl;

        if (t) {
            TPoints ptpoint = t->getTPoints();

            // Si mi padre genero time points, me hago depdender de ellos.
            if (ptpoint.first > 0) {
                // cerr << "Aniadiendo vinculos" << endl;
                //ct->printHead(&cerr);
                //cerr << endl;
                //th->print(&cerr);
                // cerr << endl;
                //vinculos causales con la tarea que me expandio
                if (!stp->AddTConstraint2(ptpoint.first, metp.first, 0, POS_INF)) {
                    //cerr << "Vinculandome con papa, start" << endl;
                    if (FLAG_VERBOSE) {
                        *errflow << "Error adding time constraints with my (father) expanding task" << endl;
                    }
                    return false;
                }
                if (!stp->AddTConstraint2(metp.second, ptpoint.second, 0,
                            POS_INF)) {
                    //cerr << "Vinculandome con papa, end" << endl;
                    if (FLAG_VERBOSE) {
		        *errflow << "(### " << stack.size() << " ###) Error: Adding time constraints with my (father) expanding task." << endl;
                    }
                    return false;
                }
            }
        }

        //cerr << "Net constraints" << endl;
        if (!applyNetConstraints(v, metp)) {
            if (FLAG_VERBOSE) {
                *errflow << "(### " << stack.size() << " ###) Error: Applying user defined temporal constraints." << endl;
            }
            return false;
        }
    } else {
        if (t) {
            TPoints ptpoint = t->getTPoints();
            ct->setTPoints(ptpoint.first, ptpoint.second);
            //cerr << "Heredando TPoints: ";
            //ct->printHead(&cerr);
            //th->print(&cerr);
            //cerr << endl;
        }
    }
    return true;
}
;

bool Plan::makeOrderLinks(int tid, TPoints metp) {
    if (makePredOrderLinks(tid, metp)) {
        return makeSuccOrderLinks(tid, metp);
    } else
        return false;
}

bool Plan::makePredOrderLinks(int tid, TPoints metp) {

    // Obtener las tareas que me preceden y
    // hacer v�nculos de orden con ellas.
    intvit b, ee = tasknetwork->getPredEnd(tid + 1);
    for (b = tasknetwork->getPredBegin(tid + 1); b != ee; b++) {
        if ((*b) != 0) {
            const Task * tmp = tasknetwork->getTask((*b) - 1);
            if (!tmp->isTaskHeader()) {
                // ya se expandi� comprobar que no sea una
                // inline. En el caso de que lo sea, unir con las predecesoras
                // de la inline
                if (((PrimitiveTask *) tmp)->getInline()) {
                    if (!makePredOrderLinks((*b) - 1, metp))
                        return false;
                } else {
                    if (!stp->AddTConstraint2(
                                ((PrimitiveTask *) tmp)->getTPoints().second,
                                metp.first, 0, POS_INF))
                        return false;
                }
            }
        }
    }
    return true;
}

bool Plan::makeSuccOrderLinks(int tid, TPoints metp) {
    // Ahora con las sucesoras.
    intvit b, ee = tasknetwork->getSuccEnd(tid + 1);
    for (b = tasknetwork->getSuccBegin(tid + 1); b != ee; b++) {
        if ((*b) != 0) {
            const Task * tmp = tasknetwork->getTask((*b) - 1);
            if (!tmp->isTaskHeader()) {
                // ya se expandi� comprobar que no sea una
                // inline. En el caso de que lo sea, unir con las sucesoras
                // de la inline
                if (((PrimitiveTask *) tmp)->getInline()) {
                    if (!makeSuccOrderLinks((*b) - 1, metp))
                        return false;
                } else {
                    if (!stp->AddTConstraint2(metp.second,
                                ((PrimitiveTask *) tmp)->getTPoints().first, 0,
                                POS_INF))
                        return false;
                }
            }
        }
    }
    return true;
}
;

int Plan::searchTimeLine(StackNode * context, const VIntervals & intervals,
        int startpos) {
    VIcite ib, ie = intervals.end();
    int n;
    pair<int, int> tp, interval;
    bool correct = false;
    context->tl_tcnStackSize = stp->getUndoLevels();
    //
    // recorrer los v�nculos causales
    if (startpos < 0 || startpos >= (int) intervals.size()) {
        return -1;
    }
    Task * pt = (*(context->offspring))[context->task];
    if (pt->isPrimitiveTask()) {
        const CLTable * clt =
            causalTable.getConsumedLinks((PrimitiveTask *) pt);
        cltcite bc, ec;
        ec = clt->end();

        // recorremos las intersecciones de los intervalos conseguidos
        for (ib = (intervals.begin() + startpos); ib != ie && !correct; ib++) {
            // vamos avanzando hasta encontrar una interseccion cuya
            // red termporal asociada sea valida
            restoreTCN(context);
            stp->SetLevel();
            if (FLAG_VERBOSE) {
                *errflow << "(*** " << stack.size()
                    << " ***) Saving Simple Temporal Network: "
                    << stp->getUndoLevels() << endl;
            }
            correct = true;
            // recorrer todos los elementos del tl obtener el comienzo del intervalo
            // y a�adir a la rt
            //cerr << startpos++ << " " << (*ib).first << " " << (*ib).second << endl;
            for (bc = clt->begin(); bc != ec && correct; bc++) {
                if ((*bc)->isTimeLineLink()) {
                    TimeLineLiteralEffect
                        * tl =
                        ((TimeLineLiteralEffect *) (*bc)->getModificableLiteral());
                    n = tl->getIndexInterval((*ib).first);
                    tp = tl->getTPoint(n);
                    if (tp.first == -1) {
                        tp = make_pair(stp->Size(), stp->Size() + 1);
                        tl->addTPoint(n, tp);
#ifdef __tcnm_pc2_cl
                        stp->InsertTPoint(0);
                        stp->InsertTPoint(0);
#else
                        stp->InsertTPoint();
                        stp->InsertTPoint();
#endif
                        //cerr << "insertando tpoints para tl " << tp.first << endl;
                        //cerr << (*ib).first << endl;
                        //cerr << n << endl;
                        interval = tl->getInterval(n);
                        if (interval.first < 0)
                            interval.first = 0;
                        //cerr << interval.first << " iii " << interval.second << endl;
                        if (!stp->AddTConstraint2(0, tp.first, interval.first,
                                    interval.first)) {
                            correct = false;
                            //cerr << "fallo en uno" << endl;
                        }
                        if (correct && !stp->AddTConstraint2(0, tp.second,
                                    interval.second, interval.second)) {
                            //cerr << "fallo en dos" << endl;
                            correct = false;
                        }
                    }
                    //  cerr << "tl ";
                    //  tl->print(&cerr);
                    //  cerr << endl;
                    //  pt->printHead(&cerr);
                    //  cerr << "**** " << stp->Size() << " " << tp.first << " " << tp.second << endl;
                    if (correct && !stp->AddTConstraint2(tp.first,
                                ((PrimitiveTask *) pt)->getTPoints().first, 0,
                                POS_INF)) {
                        //cerr << "Fallo en A" << endl;
                        correct = false;
                    }
                    // es un at-start
                    if ((*bc)->getProtectionFirst()
                            == (*bc)->getProtectionSecond()) {
                        //cerr << "soy un atstart" << endl;
                        if (correct && !stp->AddTConstraint2(
                                    ((PrimitiveTask *) pt)->getTPoints().first,
                                    tp.second, 0, POS_INF)) {
                            //cerr << "Fallo en B" << endl;
                            correct = false;
                        }
                    } else if (correct && !stp->AddTConstraint2(
                                ((PrimitiveTask *) pt)->getTPoints().second,
                                tp.second, 0, POS_INF)) {
                        //cerr << ((PrimitiveTask *)pt)->tpoints.second << " " << tp.second << endl;
                        //stp->PrintHistory(cerr);
                        //cerr << "Fallo en C" << endl;
                        correct = false;
                    }

                }
            }

        }
        if (correct)
            return distance(intervals.begin(), ib);
        restoreTCN(context);
    }
    return -1;
}

void Plan::restoreTCN(StackNode * context) {
    //si se produjo alg�n error restaurar la rt y el tl
    context->undoSTP(stp, context->tl_tcnStackSize);

    Task * pt = (*(context->offspring))[context->task];

    if (pt->isPrimitiveTask()) {
        const CLTable * clt =
            causalTable.getConsumedLinks((PrimitiveTask *) pt);
        cltcite bc, ec;
        ec = clt->end();
        for (bc = clt->begin(); bc != ec; bc++) {
            if ((*bc)->isTimeLineLink()) {
                TimeLineLiteralEffect
                    * tl =
                    ((TimeLineLiteralEffect *) (*bc)->getModificableLiteral());
                tl->eraseTPoint(stp->Size());
            }
        }
    }
    //cerr << " " << stp->Size() << endl;;
}
;

// -----------------------------------------------------------------------------
template<class T, class TN>
struct SPrintPlan {
    SPrintPlan(ostream * os, const TN * tn, vector<pair<int, int> > * sched,
            vector<pair<time_t, time_t> > * schedt) {
        this->os = os;
        this->tn = tn;
        this->sched = sched;
        this->schedt = schedt;
    }
    ;

    void operator()(long t) const {
        const T * e = tn->getTask(t);
        if (e->isPrimitiveTask() && !((PrimitiveTask *) e)->getInline()) {
            if (sched) {
                pair<unsigned int, unsigned int> tpoints =
                    ((PrimitiveTask *) e)->getTPoints();
                ((PrimitiveTask *) e)->prettyPrint(os, tpoints, sched);
            } else if (schedt) {
                pair<unsigned int, unsigned int> tpoints =
                    ((PrimitiveTask *) e)->getTPoints();
                ((PrimitiveTask *) e)->prettyPrint(os, tpoints, schedt);
            } else
                ((PrimitiveTask *) e)->prettyPrintNoDur(os);
            *os << endl;
        }
    }
    ;

    ostream * os;
    const TN * tn;
    vector<pair<int, int> > * sched;
    vector<pair<time_t, time_t> > * schedt;
};

void Plan::printPlan(ostream * os) {
    if (hasPlan()) {
        if (domain->isTimed()) {
            if (!problem->FLAG_TIME_START || problem->FLAG_TIME_UNIT
                    == TU_UNDEFINED) {
                vector<pair<int, int> > v;
#ifdef __tcnm3
                stp->ResetSchedule();
#endif
                stp->Schedule(v);
                for_each(plan.begin(), plan.end(),
                        SPrintPlan<Task, TaskNetwork> (os, tasknetwork, &v, 0));
            } else {
                if (!schedule)
                    calculateSchedule();
                for_each(plan.begin(), plan.end(),
                        SPrintPlan<Task, TaskNetwork> (os, tasknetwork, 0,
                            schedule));
            }
        } else {
            for_each(plan.begin(), plan.end(), SPrintPlan<Task, TaskNetwork> (
                        os, tasknetwork, 0, 0));
        }
    }
}
;

// ----------------------------------------------------------------------------


void Plan::toxml(ostream * os, bool primitives, bool types, bool tcnm,
        bool compound, bool inlines) {
    if (hasPlan()) {
        XmlWriter * writer = new XmlWriter(os);
        toxml(writer, primitives, types, tcnm, compound, inlines);
        writer->flush();
        delete writer;
    }
}
;

void Plan::toxml(XmlWriter * writer, bool primitives, bool types, bool tcnm,
        bool compound, bool inlines) {
    if (hasPlan()) {
        char s[128];
        time_t start;
        if (domain->isTimed()) {
            // obtener el comienzo del plan
            if (!problem->FLAG_TIME_START)
                time(&start);
            else
                start = problem->FLAG_TIME_START;
            // convertir a formato de texto seg�n el flag dado
            strftime(s, 127, XML_TFORMAT.c_str(), localtime(&start));
        }

        int p = getNumberOfActions();
        // escribimos el tag raiz plan
        writer->startTag("plan") ->addAttrib("name", problem->getName())
	    ->addAttrib("primitives", p)
	    ->addAttrib("inlines", (int) (plan.size() - p))
	    ->addAttrib("planning_time", (float) ((TEND -TSTART)*1.0))
	    ->addAttrib("nodes",nodeCounter)
	    ->addAttrib("expansions",eCounter)
	    ->addAttrib("inferences",mCounter);

        if (domain->isTimed()) {
            writer->addAttrib("start-date", s) ->addAttrib("time-format",
                    XML_TFORMAT.c_str()) ->addAttrib("time-unit",
                    TimeUnit2String(problem->FLAG_TIME_UNIT));
        }

        // Calculamos el schedule si no se ha hecho ya.
        if (!schedule)
            calculateSchedule();

        if (types)
            domain->typeHierarchyToxml(writer);

        if (tcnm) {
            stp->toxml(writer);
        }
        tasknetwork->toxml(writer, primitives, compound, inlines);

        // cierre del tag plan
        writer->endTag();
    }
}
;

// --------------------------------------------------------------------------------------------
struct GenerateTimes {
    time_t start_time;
    int mul;
    vector<pair<time_t, time_t> > * container;
    int inf;

    GenerateTimes(time_t st, TimeUnit u, vector<pair<time_t, time_t> > * con) {
        con->clear();
        start_time = st;
        mul = TU_Transform[(int) u];
        container = con;
        inf = 1000000000;
    }
    ;

    void operator()(const pair<int, int> & t) {
        //cerr << t.first << " -- " << t.second << endl;
        if (t.first > inf && t.second > inf)
            container->push_back(make_pair(0, 0));
        else if (t.first > inf)
            container->push_back(make_pair(0, start_time + t.second * mul));
        else if (t.second > inf)
            container->push_back(make_pair(start_time + t.first * mul, 0));
        else
            container->push_back(make_pair(start_time + t.first * mul,
                        start_time + t.second * mul));
    }
    ;
};

vector<pair<time_t, time_t> > * Plan::ScaleTime(vector<pair<int, int> > & sol,
        time_t start_time, TimeUnit tu) {
    vector<pair<time_t, time_t> > * v = 0;
    v = new vector<pair<time_t, time_t> > ;
    for_each(sol.begin(), sol.end(), GenerateTimes(start_time, tu, v));
    return v;
}
;

// -----------------------------------------------------------------------------

void Plan::calculateSchedule(void) {
    time_t start;

    if (hasPlan()) {
        if (schedule) {
            delete schedule;
            schedule = 0;
        }
        if (domain->isTimed()) {
            if (!problem->FLAG_TIME_START)
                time(&start);
            else
                start = problem->FLAG_TIME_START;

            vector<pair<int, int> > s;
#ifdef __tcnm3
            stp->ResetSchedule();
#endif
            if (stp->Schedule(s))
                schedule = ScaleTime(s, start, problem->FLAG_TIME_UNIT);
        }
    }
}

int Plan::getNumberOfRoots(void) {
    if (hasPlan()) {
        generateETree();
        return etree->getNumberOfRoots();
    }
    return 0;
}
;

const Task * Plan::getRoot(int i) {
    if (hasPlan()) {
        generateETree();
        return etree->getRoot(i);
    } else
        return 0;
}
;

const NodeTaskTree * Plan::getNode(int k) {
    if (hasPlan()) {
        generateETree();
        return etree->getNode(k);
    } else
        return 0;
}
;

void Plan::generateETree(void) {
    if (hasPlan()) {
        if (!etree)
            etree = new ExpansionTree(&plan, tasknetwork);
    }
}
;

int Plan::getNumberOfActions(void) const {
    int contador = 0;
    if (hasPlan()) {
        plancite i, e = plan.end();
        const PrimitiveTask * t;
        for (i = plan.begin(); i != e; i++) {
            t = (const PrimitiveTask *) tasknetwork->getTask((*i));
            if (!t->isInline())
                contador++;
        }
    }
    return contador;
}
;

    pair<time_t, time_t> Plan::getTimeRef(int tpoint) {
        if (!schedule)
            calculateSchedule();
        if (schedule && tpoint < (int) schedule->size() && tpoint >= 0)
            return (*schedule)[tpoint];
        return make_pair(0, 0);
    }

    const StackNode * Plan::getContext(int index) const {
        if (index >= (int) stack.size() || index < 0)
            return 0;
        else
            return stack[index];
    }
;
