#ifndef PRIMITIVETASK_HH
#define PRIMITIVETASK_HH
#include "constants.hh"
#include "literal.hh"
#include "goal.hh"
#include "task.hh"
#include "effect.hh"
#include "andEffect.hh"
#include "literaleffect.hh"
#include "causal.hh"

using namespace std;

extern bool PRINT_NUMBERTYPE;
extern bool PRINT_OBJECTTYPE;
extern bool PRINT_DEFINEDTYPES;

class State;

enum TComparator {EQ_DUR,LEQ_DUR,GEQ_DUR,LESS_DUR,GRE_DUR,EQ_START,LEQ_START,GEQ_START,LESS_START,GRE_START,EQ_END,LEQ_END,GEQ_END,GRE_END,LESS_END};
typedef pair<TComparator,Evaluable*> TCTR;
typedef pair<TComparator, double> ETCTR;
typedef vector<ETCTR> EvaluatedTCs;

class PrimitiveTask: public Task
{
    public:

        /**
        * @brief Constructor
        * @param id es el identificador del nombre de la tarea.
        * @param mid Es el meta identificador de la tarea.
        */
        PrimitiveTask(int id, int mid);

        PrimitiveTask(int id, int mid, const KeyList * kl);

        PrimitiveTask(const PrimitiveTask * o);

        /**
        @brief Destructor
        */
        virtual ~PrimitiveTask();

        virtual bool isPrimitiveTask(void) const {return true;};

        /**
        * @brief Realiza una copia exacta de este objeto.
        * @description La memoria debe ser liberada por la funci�n llamadora con delete.
        * @author oscar
        */
        virtual Expression * clone(void) const;

        /**
        @brief Imprime el contenido del objeto por la salida estandard.
        @param os un flujo de salida, por defecto la salida estandard.
        */
        virtual void print(ostream * os, int intent=0) const;

        virtual void vcprint(ostream * os, int indent=0) const {print(os,indent);};

        /**
        * Escribe en un documento xml la informaci�n sobre la tarea.
        * @param writer El objeto en el que escribir.
        **/
        virtual void toxml(XmlWriter * writer) const {toxml(writer,true);};

        virtual void toxml(XmlWriter * writer, bool complete) const;

        virtual void vctoxml(XmlWriter * w) const {toxml(w);};

        virtual void printHead(ostream * os) const;

        void prettyPrint(ostream * os, pair<unsigned int,unsigned int> tpoints, const vector<pair<int,int> > * sched) const;

        void prettyPrintNoDur(ostream * os) const;

        void prettyPrint(ostream * os, pair<unsigned int,unsigned int> tpoints, const vector<pair<time_t,time_t> > * sched) const;

        /**
        @brief A�ade una nueva precondici�n a la tarea.
        @description g se a�ade por referencia al objeto.
        @param g La precondici�n a a�adir.
        @author oscar
        */
        inline void setPrecondition(Goal * g) {precondition = g;};

        inline Goal * getPrecondition(void) {return precondition;};

        /**
        @brief A�ade un efecto a la lista de efectos.
        @param e Puntero al efecto a a�adir.
        */
        inline void setEffect(Effect * e) {effect = e;};

        /**
        * @brief Devuelve el efecto apuntado por el iterador
        */
        inline const Effect * getEffect(void) const {return effect;};

        /**
        * Aplica los efectos de esta tarea primitiva al estado dado.
        * @param sta El estado sobre el que aplicaremos los efectos.
        * @param undo Vector que contendr� informaci�n necesaria para poder
        * deshacer los cambios realizados.
        * @param uf Unificador en el que almacenaremos los v�nculos causales
        * de los que dependen algunos efectos, por ejemplo el when. Si se pone
        * a null no se tendr� en consideraci�n.
        * @return true en caso de �xito al aplicar los efectos, false en otro
        * caso en cuyo caso deber�n deshacerse los cambios provocados por la
        * aplicaci�n de los efectos y realizar un backtrack.
        */
        bool applyEffects(State * sta, VUndo * undo, Unifier * uf);

        /**
        * Aplica los efectos almacenados en el vector de undo.
        * @param undo Vector que contiene los efectos por aplicar.
        * @param producer Quien ser� el responsable de los efectos.
        */
        static void commitEffects(State * sta, VUndo * undo, const PrimitiveTask * producer);

        virtual pkey getTermId(const char * name) const;

        virtual bool hasTerm(int id) const;

        virtual void renameVars(Unifier * u, VUndo * undo);

        inline void setInline(int v = 2) {inlinet=v;};

        inline bool isInline(void) const {return inlinet==2;};

        inline bool isInlineCut(void) const {return inlinet==1;};

        inline int getInline(void) const {return inlinet;};

        virtual bool isReachable(ostream * err) const;

        virtual bool provides(const Literal * l) const;

        /**
        * Comprueba si la acci�n es durativa.
        * @return true en caso de que la acci�n sea durativa
        */
        inline bool isDurative(void) const {return tconstraints != 0;};

        inline void setTPoints(unsigned int first, unsigned int second) {tpoints.first = first; tpoints.second = second;};

        inline TPoints getTPoints(void) const {return tpoints;};

        /**
         * Establece las restricciones sobre la duracion de la tarea
         */
        inline void setTConstraints(vector<TCTR> * constraints) {this->tconstraints = constraints;};

        /**
         * Eval�a las expresiones contenidas en el vector de restricciones
         * temporales tconstraitns y devuelve un vector con el operador y
         * el resultado de la evaluaci�n para poder a�adirlo a la STN
         * Liberar la memoria del vector devuelto
         * @return null en caso de error
         */
         EvaluatedTCs * evalDurationExp(
                        const State * state,
                        const Unifier * context) const;
    protected:
    /** La precondici�n de la tarea, si dispone de alguna */
        Goal * precondition;
        /** El efecto */
        Effect * effect;
        /** Marca que la tarea es de tipo inline */
        int inlinet;

        /**
         * Este vector almacena las restricciones sobre la duraci�n
         */
        vector<TCTR> * tconstraints;

        /**
        * �ndice que marca el contexto en el que se aplicaron los efectos de esta acci�n.
        **/
        int contextIndex;

        TPoints tpoints;

        /**
        * Sustituye las variables que aparcen en la cadena s con el formato por su correspondiente
        * valor de constante.
        **/
        const char * replaceVars(string s, const pair<unsigned int,unsigned int> * tpoints, const vector<pair<time_t,time_t> > * sched) const;
        const char * replaceVars2(string s, const pair<unsigned int,unsigned int> * tpoints, const vector<pair<int,int> > * sched) const;
};

struct CloneVCT
{
    CloneVCT(vector<TCTR> * v) {this->v = v;};
    void operator()(const TCTR & e)
    {
        v->push_back(make_pair(e.first,e.second->cloneEvaluable()));
    };
    vector<TCTR> * v;
};

struct DeleteVCT
{
    void operator()(TCTR & e)
    {
        delete e.second;
        e.second = 0;
    };
};

struct PrintCT
{
    PrintCT(ostream * os, int ind) :s(ind,' ') {this->os = os;};
    void operator()(const TCTR & e)
    {

        bool PRINT_NUMBERTYPE2, PRINT_OBJECTTYPE2, PRINT_DEFINEDTYPES2;

        //Guardamos los valores antiguos
        PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
        PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
        PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;

        //Fijamos los nuevos valores
        PRINT_NUMBERTYPE = false;
        PRINT_OBJECTTYPE = false;
        PRINT_DEFINEDTYPES = false;

        switch (e.first){
        case LEQ_DUR:
                *os << s << "(<= ?duration ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case LESS_DUR:
                *os << s << "(< ?duration ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case GRE_DUR:
                *os << s << "(> ?duration ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case EQ_DUR:
                *os << s << "(= ?duration ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case GEQ_DUR:
                *os << s << "(>= ?duration ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case LEQ_START:
                *os << s << "(<= ?start ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case LESS_START:
                *os << s << "(< ?start ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case GRE_START:
                *os << s << "(> ?start ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case EQ_START:
                *os << s << "(= ?start";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case GEQ_START:
                *os << s << "(>= ?start ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case LEQ_END:
                *os << s << "(<= ?end ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case EQ_END:
                *os << s << "(= ?end ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case GEQ_END:
                *os << s << "(>= ?end ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case GRE_END:
                *os << s << "(> ?end ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        case LESS_END:
                *os << s << "(< ?end ";
                e.second->printEvaluable(os,0);
                *os << ")" << endl;
                break;
        }

        //Recuperamos los valores anteriores
        PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;
        PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
        PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;

        };
    ostream * os;
    string s;
};

struct RVCT
{
    RVCT(Unifier * u, VUndo * vu) {this->u = u; this->vu = vu;};
    void operator()(const TCTR & e)
    {
        e.second->compRenameVars(u,vu);
    };

    VUndo * vu;
    Unifier * u;
};

struct PrintCTE
{
    PrintCTE(ostream * os, int ind) :s(ind,' ') {this->os = os;};
    void operator()(const ETCTR & e)
    {
        switch (e.first){
            case LEQ_DUR:
                    *os << s << "(<= ?duration " << e.second << ")" << endl;
                    break;
            case LESS_DUR:
                    *os << s << "(< ?duration " << e.second << ")" << endl;
                    break;
            case GRE_DUR:
                    *os << s << "(> ?duration " << e.second << ")" << endl;
                    break;
            case EQ_DUR:
                    *os << s << "(= ?duration " << e.second << ")" << endl;
                    break;
            case GEQ_DUR:
                    *os << s << "(>= ?duration " << e.second << ")" << endl;
                    break;
            case LEQ_START:
                    *os << s << "(<= ?start " << e.second << ")" << endl;
                    break;
            case LESS_START:
                    *os << s << "(< ?start " << e.second << ")" << endl;
                    break;
            case GRE_START:
                    *os << s << "(> ?start " << e.second << ")" << endl;
                    break;
            case EQ_START:
                    *os << s << "(= ?start " << e.second << ")" << endl;
                    break;
            case GEQ_START:
                    *os << s << "(>= ?start " << e.second << ")" << endl;
                    break;
            case LEQ_END:
                    *os << s << "(<= ?end " << e.second << ")" << endl;
                    break;
            case EQ_END:
                    *os << s << "(= ?end " << e.second << ")" << endl;
                    break;
            case GEQ_END:
                    *os << s << "(>= ?end " << e.second << ")" << endl;
                    break;
            case GRE_END:
                    *os << s << "(> ?end " << e.second << ")" << endl;
                    break;
            case LESS_END:
                    *os << s << "(< ?end " << e.second << ")" << endl;
                    break;
        }
    };
    ostream * os;
    string s;
};

#endif

