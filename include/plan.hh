#ifndef PLAN_HH
#define PLAN_HH

#include "constants.hh"
#include <list>
#include <vector>
#include "selector.hh"
#include "problem.hh"
#include "domain.hh"
#include "tcnm.hh"
#include "stacknode.hh"
#include "clock.hh"
#include "expansiontree.hh"

using namespace std;
using namespace Pocima;

// El estado en el que se encuentra el planificador.
enum Stage{SelectAgenda,SelectTask,SelectMethod,SelectUnification};

/**
 * Clase que implementa el proceso de planificacion y que almacena el plan
 * resultante
 */
class Plan{
    public:
    // ----------------------------------------------------------------------
    // Variables de entorno
    // ----------------------------------------------------------------------
    /** Limite maximo de expansiones permitidas durante la busqueda del plan
    Si se supera provocar� la terminaci�n del programa.
    Un valor <= 0 se toma como infinito. */
    int FLAG_EXPANSIONS_LIMIT;
    /** L�mite m�ximo de profundidad que se intentar� durante el plan
    Si se supera forzar� un backtracking.
    Un valor <= 0 se toma como infinito. */
    int FLAG_DEPTH_LIMIT;
    /** L�mite m�ximo para el tiempo de ejecucion en segundos. */
    int FLAG_TLIMIT;
    /** Controla el formato de salida de fechas en XML */
    string XML_TFORMAT;
    /** Controla los mensajes de depuraci�n que el planificador desplega por pantalla */
    int FLAG_VERBOSE;

    // ----------------------------------------------------------------------
    // Funciones
    // ----------------------------------------------------------------------

    /**
    * Constructor del plan
    * @param d El dominio a resolver.
    * @param p Su problema asociado.
    **/
    Plan(const Domain * d, const Problem * p);

    ~Plan(void);

    /**
    * @brief Devuelve un puntero al dominio del problema.
    */
    inline const Domain * getDomain(void) const {return domain;};

    /**
    * @brief Devuelve un puntero al problema a resolver.
    */
    inline const Problem * getProblem(void) const {return problem;};

    /**
    @brief Lanza el proceso de planificacion para tratar de resolver el problema.
    @param plan Es un vector vac�o donde se ir�n almacenando las acciones del plan. El plan
    devuelto es totalmente lineal. La memoria del vector pasada ya tiene que estar reservada
    antes de realizar la llamada.
    @return true en el caso de que se encontrara un plan, false en otro caso.
    */
    bool solve(void);

    /**
    * Esta funci�n sirve para imprimir el plan resultante.
    * @param os el flujo donde deseamos escribir.
    **/
    void printPlan(ostream * os);

    /**
    * Devuelve un documento XML con la descripci�n del plan resultante.
    * @param os el flujo donde deseamos escribir.
    * @param compound Se obtiene informaci�n tambi�n de las tareas abstractas.
    * @param inlines Se obtiene informaci�n tambi�n de las tareas inline.
    **/
    void toxml(ostream * os, bool primitives= true, bool types =false, bool tcnm=false, bool compound=false, bool inlines=false);

    void toxml(XmlWriter * writer, bool primitives = true, bool types=false, bool tcnm=false, bool compound=false, bool inlines=false);

    /**
    * Devuelve la red de tareas con la que estamos trabajando
    */
    const TaskNetwork * getTaskNetwork(void) const {return tasknetwork;};

    /**
    * Devuelve el plan una vez que hemos obtenido uno.
    */
    const TPlan * getPlan(void) const {return &plan;};

    /**
    * Devuelve la red de restricciones temporales.
    */
    const STP * getSTP(void) const {return stp;};

    /**
    * Devuelve el estado actual.
    */
    const State * getState(void) const {return state;};

    /**
    * Ojo se devuelve un puntero no protegido, tener cuidado
    * con c�mo se modifica el estado.
    **/
    State * getModificableState(void) {return state;};

    /**
    * Devuelve el estado actual.
    */
    const State * getCurrentState(void) const {return state;};

    /**
    * Establece los algoritmos de selecci�n.
    */
    inline void setSelector(Selector * s) {this->select = s;};

    /**
    * Devuelve el contexto inicial.
    **/
    inline const StackNode * getInitialContext(void) const {return stack.front();};

    /**
    * Devuelve el plan en su estado actual.
    **/
    inline const TPlan * getCurrentPlan(void) const {return &plan;};

    /**
    * Esta funci�n es de uso interno. No deber�a llamarse ya que es potencialmente
    * peligrosa.
    **/
    int deleteFromState(int id, const KeyList * params);

    /**
    * Esta funci�n es de uso interno. No deber�a llamarse ya que es potencialmente
    * peligrosa.
    * @return 0 en caso de �xito, otro n�mero si error.
    **/
    int deleteFromState(const Literal * l);

    /**
    * Esta funci�n es de uso interno. No deber�a llamarse ya que es potencialmente
    * peligrosa.
    **/
    int addToState(Literal * l);

    /**
    * Devuelve la agenda.
    **/
    inline const StackNode * getCurrentContext(void) const {if(stack.empty()) return 0; else return stack.back();};

    /**
    * Devuelve el contexto actual en el que se encuentra el planificador.
    **/
    inline const VAgenda * getCurrentAgenda(void) const {if(stack.empty()) return 0; else return &(stack.back()->agenda);};

    /**
    * Imprime las estadisticas sobre tiempo de planificacion y dem�s en el flujo
    * pasado como argumento.
    */
    void printStatistics(ostream * flow) const;

    /**
    * Devuelve el momento en el que se encuentra el planificador.
    **/
    inline Stage getStage(void) const {return stage;};

    /**
    * Devuelve true si se encontr� un plan.
    **/
    inline bool hasPlan(void) const {return hasplan;};

    /**
    * Devuelve el n�mero de ra�ces del �rbol de expansi�n.
    */
    int getNumberOfRoots(void);

    /**
    * Devuelve la ra�z i�sima del �rbol de expansi�n.
    */
    const Task * getRoot(int i);

    /**
    * Devuelve un nodo dada una clave.
    */
    const NodeTaskTree * getNode(int k);

    /**
    * Devuelve el n�mero de acciones primitivas del plan obtenido.
    */
    int getNumberOfActions(void) const;

    /**
    * Devuelve la referencia temporal del schedule para un time-point determinado.
    * La funci�n devolver� un valor indeterminado, si el plan no es temporal y no
    * est� terminado.
    **/
    pair<time_t,time_t> getTimeRef(int tpoint);

    /**
    * Devuelve el schedule del plan si hay uno calculado.
    * LLamar a calculateSchedule para que lo calcule.
    * Debe de haberse obtenido un plan para llamar a esta funci�n.
    **/
    vector<pair<time_t,time_t> > * getSchedule(void) {return schedule;};

    /**
    * Calcula el scheduling para las tareas una vez que
    * se ha obtenido un plan.
    * Establece en la variable global schedule los tiempos
    * de comienzo y finalizaci�n de cada tarea encontrados.
    */
    void calculateSchedule(void);

    /**
    * Devuelve el �ndice del contexto actual de planificacion.
    **/
    inline int getContextIndex(void) const {return (int)stack.size();};

    /**
    * Devuelve el contexto dado un �ndice de posici�n..
    **/
    const StackNode * getContext(int index) const;

    protected:
    /**
    * El dominio de planificacion.
    */
    const Domain * domain;
    /**
    * El problema a resolver.
    */
    const Problem * problem;

    /** Estructura para el estado actual */
    State * state;

    /** Estructura que mantiene informaci�n sobre la red de tareas actual */
    TaskNetwork * tasknetwork;

    /** Vector donde guardamos los identificadores de las tareas que se van
    * a�adiendo al plan */
    TPlan plan;

    /** pila de llamadas "recursivas", mantiene el estado del planificador
    a lo largo de la ejecucion en cada uno de los nodos del �rbol de b�squeda. */
    vector<StackNode *> stack;

    /** Estructura para mantener la red de restricciones temporales */
    STP * stp;

    /** Funciones para la selecci�n de diversas alternativas durante
    * la exploraci�n del �rbol de b�squeda */
    Selector * select;

    /** Contador de los nodos generados durante la exploraci�n */
    int nodeCounter;

    /** Contador de expansiones */
    int eCounter;

    /** Contador de inferencias realizadas */
    int mCounter;

    /** Tiempo en el que se comenz� la ejecucion del planificador. */
    clock_t TSTART;

    /** Tiempo en el que se termin� la ejecucion del planificador. */
    clock_t TEND;

    /** Otros cronos */
    ChronometerRealTime c1;
    ChronometerUsedTime c2;
    ChronometerSystemTime c3;

    /** Contadores para recolectar estad�sticas */
    int cortados;
    int CL;
    int directos;

    /**
    * El estado en el que se encuentra el planificador.
    */
    Stage stage;

    /**
    * Resultado de hacer el schedule
    **/
    vector<pair<time_t,time_t> > * schedule;

    /**
    * Flag que indica si se encontro un plan.
    **/
    bool hasplan;

    /**
    * Esta estructura codifica el arbol de expansion.
    */
    ExpansionTree * etree;

    /**
    * Borra la memoria reservada por las estructuras de datos
    */
    void reset(void);

    /**
    * Inicializa las estructuras de datos antes de realizar la planificacion.
    */
    void init(void);

    /**
    * bucle principal del algoritmo de planificacion.
    * @param context Es el contexto actual de ejecucion.
    * @return El siguiente contexto de ejecucion.
    **/
    StackNode * solve(StackNode * context);

    /**
    * Funci�n que selecciona una tarea de la lista disponible
    * en la agenda.
    * @param context Es el contexto actual de ejecucion.
    * @return true si se realizo con exito, false si se produjo alg�n fallo
    */
    bool selectTask(StackNode * context);

    /**
    * Selecciona alguna de las unificaciones posibles.
    * @param context Es el contexto actual de ejecucion.
    * @return true si se realizo con exito, false si se produjo alg�n fallo
    */
    bool selectUnification(StackNode * context);

    /** funciones para el manejo del tl. */

    bool addPTConstraints(PrimitiveTask * pt, StackNode * context, const Unifier * uf);

    bool addCTConstraints(CompoundTask * pt, Method * met, StackNode * context);

    bool addInmediateTConstraints(int miindex, TPoints & metp);

    /**
    * Realiza un an�lisis de la estructura causal del plan, para a�adir restricciones
    * a la red temporal, y garantizar el correcto paralelismo
    * entre las acciones primitivas.
    */
    bool causalLinkAnalisys(PrimitiveTask * pt, StackNode * context);
    bool CLThreatAnalisys(PrimitiveTask * pt, StackNode * context);

    /**
    * Aplica las restricciones temporales definidas en la red de tareas.
    */
    bool applyNetConstraints(const vector<TCTR> * v, TPoints metp);
    /**
    * Aplica las restricciones temporales definidas en la duracion de una
    * tarea primitiva
    */
    bool applyDConstraints(const EvaluatedTCs * v, TPoints metp);

    int searchTimeLine(StackNode * context, const VIntervals & intervals, int startpos);

    void restoreTCN(StackNode *);

    bool makeOrderLinks(int tid,TPoints metp);
    bool makeSuccOrderLinks(int tid, TPoints metp);
    bool makePredOrderLinks(int tid, TPoints metp);
    void setTimeVars(const Unifier * uf, const TPoints * tp, const Unifiable * t);

    /**
    * Bucle principal del planificador. Se van seleccionando contextos de la pila
    * de contextos para su procesamiento, hasta que se alcance un plan v�lido o
    * bien la pila quede vacia debido a la cantidad de backtrackins.
    */
    bool mainLoop(void);

    /**
    * Genera los time-points para una tarea determinada.
    **/
    bool generateTPoints(Task * t);

    /**
    * Realiza o establece el cambio de unidades de tiempo, para una determinada soluci�n
    * obtenida con Schedule.
    * @param solution es un vector generado por el schedule de la stp.
    * @param start_time Momento que se tomar� como instante 0.
    * @param tu Unidad de tiempo que tomar para escalar
    */
    vector<pair<time_t,time_t> > * ScaleTime(vector<pair<int,int> > & solution, time_t start_time, TimeUnit tu=TU_HOURS);

    /**
    * Genera la estructura que codifica el �rbol de expansi�n.
    */
    void generateETree(void);
};

extern Plan * current_plan;

#endif

