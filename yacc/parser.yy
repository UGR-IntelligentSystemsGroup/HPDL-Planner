%{
    using namespace std;

    #include "constants.hh"
    #include <string>
    #include <iostream>
    #include <stdio.h>
    #include <stdlib.h>
    #include <assert.h>
    #include <vector>
    #include <sys/resource.h>
    #include <math.h>
    #include <malloc.h>
    #include <ctype.h>
    #include "MyLexer.hh"
    #include "domain.hh"
    #include "problem.hh"
    #include "type.hh"
    #include "andGoal.hh"
    #include "papi.hh"
    #include "function.hh"
    #include "pyDefFunction.hh"
    #include "comparationGoal.hh"
    #include "fluentNumber.hh"
    #include "fluentOperator.hh"
    #include "fluentVar.hh"
    #include "forallgoal.hh"
    #include "existsgoal.hh"
    #include "foralleffect.hh"
    #include "cutgoal.hh"
    #include "sortgoal.hh"
    #include "undoARLiteralState.hh"
    #include "fluentEffect.hh"
    #include "wheneffect.hh"
    #include "fluentLiteral.hh"
    #include "literalgoal.hh"
    #include "orGoal.hh"
    #include "boundGoal.hh"
    #include "implyGoal.hh"
    #include "fluentConstant.hh"
    #include "timeInterval.hh"
    #include "printGoal.hh"
    #include "debugger.hh"
    #include "timeLineLitEffect.hh"
    #include "plan.hh"
    #include "textTag.hh"

    #define yytrue true
    #define yyfalse false
    #define DEBUG 0
    #define YYDEBUG 1
    #define YYMAXDEPTH INT_MAX

#ifdef PYTHON_FOUND
    #define PYTHON_FLAG 1
#else
    #define PYTHON_FLAG 0
#endif

// variables globales
// estas variables sirven para ir almacenando los valores
// que vamos capturando
ParameterContainer * container=0;
LDictionary * context=0;
LDictionary * oldContext=0;
vector<ContainerGoal *> gcontainer;
vector<ContainerEffect *> econtainer;
CompoundTask * cbuilding=0;
// Espera encontrar un n�mero
bool isNumber = false;
// Espera encontrar un at
bool AtExpected=false;
// determina si la acci�n que estamos parseando es o no durativa
bool isDurative = false;

// variable para construir los mensajes de error
char parerr[256];
// imprimir o no errores de tipos
bool errtypes=true;

int contador=0;

// Usar este objeto como interfaz con el debugger
extern Debugger * debugger;
// esperar o no tokens del debugger
bool inDebugContext= false;

// flags del parser
// evitar o no la mayor�a de los chequeos
bool fast_parsing = false;

inline int yylex(void)
{
    return lexer->yylex();
};

inline void yyerror(const char *s)
{
    lexer->LexerError(s);
};

inline void yywarning(const char *s)
{
    lexer->LexerWarning(s);
};

struct SearchLineInfo
{
    string fileName;
    int lineNumber;

    SearchLineInfo(const Type * t)
    {
        if(t->getFileId() != -1) {
        fileName = parser_api->files[t->getFileId()];
        lineNumber = t->getLineNumber();
        }
        else{
        fileName = "";
        lineNumber = 0;
        }
    }

    SearchLineInfo(const ConstantSymbol * c)
    {
        fileName = parser_api->files[c->getFileId()];
        lineNumber = c->getLineNumber();
    }

    SearchLineInfo(int mid) {
        Meta * m = parser_api->domain->metainfo[mid];
        fileName = parser_api->files[m->fileid];
        lineNumber = m->linenumber;
    }
};

// este operador verifica que no se dan definiciones
// de tipos redundantes
struct TestTypeTree {
    void operator()(Type * t) const {
        vector<Type *>::const_iterator i,e,j;
        e = t->getParentsEnd();
        if(t->getNumberOfParents() > 0)
        for(i = t->getParentsBegin(); i != e; i++){
                if((*i)->isSubTypeOf(t)){
                SearchLineInfo sli((*i));
                snprintf(parerr,256,"Ambiguous declaration type `%s' is subtype of `%s' near %d [%s].",(*i)->getName(),t->getName(),sli.lineNumber,sli.fileName.c_str());
                yyerror(parerr);
                }
                else{
                for(j = t->getParentsBegin(); j != e; j++)
                        if(i != j && (*i)->isSubTypeOf((*j))){
                        SearchLineInfo sli((*j));
                        snprintf(parerr,256,"Ambiguous declaration type `%s' is subtype of `%s' near %d [%s].",(*i)->getName(),(*j)->getName(),sli.lineNumber,sli.fileName.c_str());
                        yywarning(parerr);
                        }
                }
        }
    };

   void operator()(vector<Type *> * vt) const {
        vector<Type *>::const_iterator i,e,j;
        e = vt->end();
        for(i = vt->begin(); i != e; i++){
        for(j = vt->begin(); j != e; j++)
        if(i != j && (*i)->isSubTypeOf((*j))){
                SearchLineInfo sli((*j));
                snprintf(parerr,256,"Ambiguous declaration type `%s' is subtype of `%s' near %d [%s].",(*i)->getName(),(*j)->getName(),sli.lineNumber,sli.fileName.c_str());
                yywarning(parerr);
        }
        }
   };
};


// este operador verifica que un tipo est� correctamente
// definido
struct TestType {
   void operator()(Type * t) const {
       // Se trata del tipo number
       if(t->getId() == 0)
        return;

       SearchLineInfo sli(t);
       if(!sli.lineNumber) {
        snprintf(parerr,256,"Trying to use an undefined type `%s'.",t->getName());
        yyerror(parerr);
       }
       else{
        TestTypeTree()(t);
       }
   };
};

%}

%defines
%verbose
%define parse.error verbose
%debug
// %glr-parser

%union
{
        void * otype;
        const void * cotype;
        const char * type_string;
        double type_number;
        pair<int,float> * termtype;
        int type_int;
}

%token LEFTPAR
%token RIGHTPAR
%token PDDL_DEFINE
%token PDDL_DOMAIN
%token PDDL_DOMAINREF
%token PDDL_PROBLEM
%token PDDL_CONSTANTS
%token <type_string> PDDL_NAME
%token <type_string> PDDL_VAR
%token <type_string> PYTHON_CODE
%token <type_number> PDDL_NUMBER
%token <type_number> PDDL_DNUMBER
%token PDDL_REQUIREMENTS
%token PDDL_TYPES
%token PDDL_HYPHEN
%token PDDL_EITHER
// Esta es la lista de requires standard de pddl
%token PDDL_STRIPS
%token PDDL_TYPING
%token PDDL_NEGATIVE_PRECONDITIONS
%token PDDL_DISJUNCTIVE_PRECONDITIONS
%token PDDL_EQUALITY
%token PDDL_EXISTENTIAL_PRECONDITIONS
%token PDDL_UNIVERSAL_PRECONDITIONS
%token PDDL_QUANTIFIED_PRECONDITIONS
%token PDDL_CONDITIONAL_EFFECTS
%token PDDL_FLUENTS
%token PDDL_ADL
%token PDDL_DURATIVE_ACTIONS
%token PDDL_DERIVED_PREDICATES
%token PDDL_TIMED_INITIAL_LITERALS

%token PDDL_PREDICATES
%token PDDL_FUNCTIONS
%token PDDL_ACTION
%token PDDL_PARAMETERS
%token PDDL_NOT
%token PDDL_PRECONDITION
%token PDDL_IMPLY
%token PDDL_AND
%token PDDL_OR
%token PDDL_EXISTS
%token PDDL_FORALL

%token PLUS
// %token MINUS ---> PDDL_HYPHEN
%token DIVIDE
%token MULTIPLY
%token POW
%token ABS
%token SQRT
%token GREATHER
%token LESS
%token EQUAL
%token DISTINCT
%token GREATHER_EQUAL
%token LESS_EQUAL
%token PDDL_EFFECT
%token PDDL_ASSIGN
%token PDDL_SCALE_UP
%token PDDL_SCALE_DOWN
%token PDDL_INCREASE
%token PDDL_DECREASE
%token PDDL_WHEN
%token PDDL_GOAL
%token PDDL_AT
%token PDDL_ATSTART
%token PDDL_ATEND
%token PDDL_BETWEEN
%token PDDL_OBJECT
%token PDDL_INIT
%token PDDL_OVERALL
%token <type_string> PDDL_DURATIONVAR
%token <type_string> STARTVAR
%token <type_string> ENDVAR
%token PDDL_DERIVED
%token PDDL_CONDITION
%token PDDL_DURATION
%token PDDL_DURATIVE_ACTION
%token HTN_EXPANSION
%token META_TAGS
%token META
%token TAG
%token HTN_TASK
%token HTN_TASKS
%token HTN_ACHIEVE
%token HTN_METHOD
%token HTN_TASKSGOAL
%token HTN_INLINE
%token HTN_INLINECUT
%token <type_string> HTN_TEXT
%token LEFTBRAC
%token RIGHTBRAC
%token EXCLAMATION
%token RANDOM
%token SORTBY
%token ASC
%token DESC
%token PDDL_BIND
%token MAINTAIN
%token PPRINT
%token PDDL_AND_EVERY
// customization related
%token CUSTOMIZATION
%token TIMEUNIT
%token TIMESTART
%token TIMEFORMAT
%token TIMEHORIZON
%token RELTIMEHORIZON
%token THOURS
%token TMINUTES
%token TSECONDS
%token TDAYS
%token TMONTHS
%token TYEARS
%token PYTHON_INIT

%type <otype> term_name
%type <otype> typed_list
%type <otype> type
%type <otype> type_ref
%type <otype> type_ref_list
%type <otype> type_def_list
%type <otype> constant_def_list
%type <termtype> variable
%type <type_string> var
%type <otype> tag_element
%type <otype> tag_list
%type <otype> meta_list
%type <otype> goal_def
%type <otype> simple_goal_def
%type <otype> preconditions_def
%type <otype> effect_def
%type <otype> effect
%type <otype> atomic_formula_term_goal
%type <otype> simple_formula_term_goal
%type <otype> atomic_formula_term_effect
%type <otype> c_effect
%type <otype> p_effect
%type <otype> cond_effect
%type <otype> inlinetask
%type <otype> atomic_task_formula
%type <otype> task_def
%type <otype> task_element
%type <otype> task_list
%type <otype> method_body
%type <otype> task_network
%type <otype> init_el
%type <otype> literal_name
%type <otype> atomic_formula_name
%type <otype> action_def
%type <otype> durative_action_def
%type <otype> htn_task_def
%type <type_string> code
%type <otype> opt_type
%type <otype> f_comp
%type <otype> binary_comp
%type <otype> fluent_exp
%type <otype> silly_exp
%type <otype> binary_op
%type <type_int> unary_op
%type <otype> f_head
%type <otype> f_head_ref
%type <otype> derived_formula_skeleton
%type <otype> time_specifier
%type <type_int> order
%type <type_int> assign_op
%type <otype> timed_effect
%type <otype> timed_goal
%type <otype> literal_effect
%type <otype> time_point
%type <otype> number_time_point
%type <type_number> number
%type <type_number> optional_repetition
%type <otype> duration_constraints
%type <otype> simple_duration_constraint
%type <otype> sdur_constraint
%type <otype> sdur_constraint_list
%type <otype> sduration_constraint_list
%type <otype> pduration_constraints
%type <type_number> time_unit

// ---------------------------------------------------------------------
// A partir de aqu� todo son tokens del debugger.
%token DBG_DEBUG
%token DBG_QUIT
%token DBG_BREAKPOINT
%token DBG_WATCH
%token DBG_CONTINUE
%token DBG_HELP
%token <type_string> DBG_PATH
%token DBG_PRINT
%token DBG_DISPLAY
%token DBG_DESCRIBE
%token DBG_UNDISPLAY
%token DBG_STATE
%token DBG_AGENDA
%token DBG_PLAN
%token DBG_NEXT
%token DBG_NEXP
%token DBG_SET
%token DBG_VIEWER
%token DBG_DOTPATH
%token DBG_TMPDIR
%token DBG_PLOT
%token DBG_CAUSAL
%token DBG_MEM
%token DBG_SELECT
%token DBG_VERBOSE
%token DBG_ON
%token DBG_OFF
%token DBG_OPTIONS
%token DBG_TERMTABLE
%token DBG_PREDICATES
%token DBG_TASKS
%token DBG_ENABLE
%token DBG_DISABLE
%token DBG_EVAL
%type<otype> met_name
%token DBG_VERBOSITY
%token DBG_APPLY

%%
pddl_root:                      domain_definition
                                | problem_definition
                                | debug_sentence
                                ;

problem_definition:             LEFTPAR PDDL_DEFINE
                                problemName
                                domainRef
                                problemBody
                                RIGHTPAR
                                ;

domain_definition:              LEFTPAR PDDL_DEFINE
                                domainName
                                domain_definition_2
                                ;

domain_definition_2:            require_def  domain_definition_31
                                | domain_definition_31
                                {parser_api->domain->addRequirement("strips");}
                                ;
domain_definition_31:            customization_def domain_definition_32
                                | domain_definition_32
                                ;

domain_definition_32:           python_init domain_definition_3
                                | domain_definition_3
                                ;

domain_definition_3:            types_def domain_definition_4
                                | domain_definition_4
                                ;

domain_definition_4:            constants_def domain_definition_5
                                | domain_definition_5
                                ;

domain_definition_5:            predicates_def domain_definition_6
                                | domain_definition_6
                                ;

domain_definition_6:            functions_def domain_definition_7
                                | domain_definition_7
                                ;

domain_definition_7:            structure_def_list RIGHTPAR
                                {
                                // tras la declaraci�n de acciones
                                // comprobar que las tareas definidad en la
                                // red de tareas realmente est�n definidas
                                tasktablecit i,e =parser_api->domain->getEndTask();
                                methodcit j, em;
                                bool errors=false;
                                bool changes = true;
                                while(changes){
                                        changes = false;
                                        for(i=parser_api->domain->getBeginTask(); i!= e; i++){
                                        errors = false;
                                        if((*i).second->isCompoundTask()){
                                                em = ((CompoundTask *)(*i).second)->getEndMethod();
                                                for(j=((CompoundTask *)(*i).second)->getBeginMethod();j!= em; j++) {
                                                if(!(*j)->getTaskNetwork()->isWellDefined(errflow,&changes))
                                                {
                                                        errors = true;
                                                };
                                                }
                                        }
                                        if(errors) {
                                                SearchLineInfo sli((*i).second->getMetaId());
                                                snprintf(parerr,256,"In the task network of task `%s' defined near %d [%s].",(*i).second->getName(),sli.lineNumber,sli.fileName.c_str());
                                                yyerror(parerr);
                                        }
                                        }
                                }
                                }
                                | error
                                ;

domainName:                     LEFTPAR PDDL_DOMAIN term_name RIGHTPAR
                                {
                                    if(parser_api->domain->loaded)
                                    {
                                        snprintf(parerr,256,"There is a domain [%s] already loaded.",parser_api->domain->getName());
                                        yyerror(parerr);
                                        YYABORT;
                                    }
                                    parser_api->domain->setDomainName(((string *)$3)->c_str());
                                parser_api->domain->loaded=true;
                                    delete (string *)$3;
                                }
                                | error
                                ;

problemName:                    LEFTPAR PDDL_PROBLEM term_name RIGHTPAR
                                {
                                    if(!parser_api->domain || !parser_api->problem)
                                    {
                                        yyerror("No domain loaded.");
                                        YYABORT;
                                    }
                                    parser_api->problem->setProblemName(((string *)$3)->c_str());
                                    delete (string *)$3;
                                }
                                ;

domainRef:                      LEFTPAR PDDL_DOMAINREF term_name RIGHTPAR
                                {
                                    if(strcmp(parser_api->domain->getName(),((string *)$3)->c_str()))
                                    {
                                        snprintf(parerr,256,"The problem requires the domain [%s] and the domain loaded is [%s].",((string *)$3)->c_str(),parser_api->domain->getName());
                                        yyerror(parerr);
                                        YYABORT;
                                    }
                                    delete (string *)$3;
                                }
                                | error
                                ;

require_def:                    LEFTPAR PDDL_REQUIREMENTS require_key require_key_list RIGHTPAR
                                ;

require_key_list:               /* empty */
                                | require_key require_key_list
                                | error
                                ;

require_key:                    PDDL_STRIPS
                                { parser_api->domain->addRequirement(":strips");}
                                |PDDL_TYPING
                                { parser_api->domain->addRequirement(":typing");}
                                |PDDL_NEGATIVE_PRECONDITIONS
                                { parser_api->domain->addRequirement(":negative-preconditions");}
                                |PDDL_DISJUNCTIVE_PRECONDITIONS
                                { parser_api->domain->addRequirement(":disjunctive-preconditions");}
                                |PDDL_EQUALITY
                                { parser_api->domain->addRequirement(":equality");}
                                |PDDL_EXISTENTIAL_PRECONDITIONS
                                { parser_api->domain->addRequirement(":existential-preconditions");}
                                |PDDL_UNIVERSAL_PRECONDITIONS
                                { parser_api->domain->addRequirement(":universal-preconditions");}
                                |PDDL_QUANTIFIED_PRECONDITIONS
                                { parser_api->domain->addRequirement(":quantified-preconditions");}
                                |PDDL_CONDITIONAL_EFFECTS
                                { parser_api->domain->addRequirement(":conditional-effects");}
                                |PDDL_FLUENTS
                                { parser_api->domain->addRequirement(":fluents");}
                                |PDDL_ADL
                                { parser_api->domain->addRequirement(":adl");}
                                |PDDL_DURATIVE_ACTIONS
                                { parser_api->domain->addRequirement(":durative-actions");}
                                |PDDL_DERIVED_PREDICATES
                                { parser_api->domain->addRequirement(":derived-predicates");}
                                |PDDL_TIMED_INITIAL_LITERALS
                                { parser_api->domain->addRequirement(":timed-initial-literals");}
                                | META_TAGS
                                { parser_api->domain->addRequirement(":metatags");}
                                |HTN_EXPANSION
                                { parser_api->domain->addRequirement(":htn-expansion");}
                                ;


term_name:                        PDDL_NAME
                                {
                                $$ = new string($1);
                                }
                                ;

types_def:                      LEFTPAR PDDL_TYPES typed_list RIGHTPAR
                                {
                                    if(!parser_api->domain->errtyping && !parser_api->domain->hasRequirement(":typing"))
                                    {
                                        parser_api->domain->errtyping = true;
                                        yyerror("Using a clause that requires `:typing' and is not declared in requirements clause");
                                    }
                                // verificamos que los tipos han sido definidos correctamente
                                if(!fast_parsing){
                                        typetablecit b = parser_api->domain->getBeginType();
                                        typetablecit e = parser_api->domain->getEndType();
                                        for_each(b,e,TestType());
                                }
                                parser_api->domain->buildTypeRelations();
                                }
                                ;

constants_def:                  LEFTPAR PDDL_CONSTANTS constant_list RIGHTPAR
                                ;


typed_list:                     /* empty */
                                {$$ = 0;}
                                | type_def_list
                                {
                                vector<Type *> * ptrTypes = (vector<Type *> *) $1;
                                vector<Type *>::iterator i,e;
                                const Type * tmp;
                                e = ptrTypes->end();
                                for(i=ptrTypes->begin(); i != e; i++){
                                        if((tmp=parser_api->domain->addType((*i))) != *i){
                                        SearchLineInfo sli(tmp);
                                        if(sli.lineNumber){
                                        snprintf(parerr,256,"The type `%s' is already defined. (previous definition near %d [%s].",tmp->getName(),sli.lineNumber,sli.fileName.c_str());
                                        yywarning(parerr);
                                        delete (*i);
                                        (*i) = 0;
                                        }
                                        else {
                                        Type * n = parser_api->domain->getModificableType(tmp->getId());
                                        n->setFileId(parser_api->fileid);
                                        n->setLineNumber(lexer->getLineNumber());
                                        }
                                        }
                                }
                                delete ptrTypes;
                                }
                                | type_def_list PDDL_HYPHEN {errtypes=false;} type {errtypes=true;} typed_list
                                {
                                vector<Type *> * ptrTypes = (vector<Type *> *) $1;
                                vector<Type *> * ptrParents = (vector<Type *> *) $4;
                                vector<Type *>::iterator i,e;
                                const Type * tmp;
                                e = ptrTypes->end();
                                for(i=ptrTypes->begin(); i != e; i++){
                                        if((tmp=parser_api->domain->addType((*i))) != *i){
                                        SearchLineInfo sli(tmp);
                                        if(sli.lineNumber){
                                        snprintf(parerr,256,"The type `%s' is already defined. (previous definition near %d [%s].",tmp->getName(),sli.lineNumber,sli.fileName.c_str());
                                        yywarning(parerr);
                                        delete (*i);
                                        (*i) = 0;
                                        }
                                        else {
                                        Type * n = parser_api->domain->getModificableType(tmp->getId());
                                        n->setFileId(parser_api->fileid);
                                        n->setLineNumber(lexer->getLineNumber());
                                        }
                                        }
                                        if(ptrParents) {
                                        Type * n = parser_api->domain->getModificableType((tmp)->getName());
                                        n->addSuperTypes(ptrParents);
                                        }
                                }
                                if(ptrParents)
                                        delete ptrParents;
                                delete ptrTypes;
                                }
                                | error
                                {$$=0;}
                                ;

constant_list:                  /* empty */
                                | constant_def_list
                                {delete (vector<ConstantSymbol *> *) $1;}

                                | constant_def_list PDDL_HYPHEN type constant_list
                                {
                                vector<ConstantSymbol *> * ptr = (vector<ConstantSymbol *> *) $1;
                                vector<Type *> * types = (vector<Type *> *) $3;
                                if(types){
                                        // Definimos el tipo para cada una de las constantes
                                        for_each(ptr->begin(),ptr->end(),bind2nd(mem_fun1_t<void,Term,const vector<Type *> *>(&Term::addTypes),types));
                                        // Para cada tipo a�adimos referencias inversas a las constantes
                                        // Esto sirve para por ejemplo en un forall obtener todas las constantes
                                        // de un tipo dado.
                                        for_each(types->begin(),types->end(),bind2nd(mem_fun1_t<void,Type,const vector<ConstantSymbol *> *>(&Type::addRefsBy),ptr));
                                        delete types;
                                }
                                delete ptr;
                                }
                                ;

constant_def_list:              PDDL_NAME
                                { vector<ConstantSymbol *> * ptr = new vector<ConstantSymbol *>;
                                  ConstantSymbol * n = new ConstantSymbol($1);
                                  // comprobamos que la constante no se encuentre ya definida
                                  ConstantSymbol * f = parser_api->termtable->getConstantFromName(n->getName());
                                  if(f != 0){
                                    // Comprobar que los tipos sean iguales a la hora de generar un
                                    // error o bien un warning
                                    SearchLineInfo sli(f);
                                    snprintf(parerr,256,"Redefinition of the constant `%s'. (previous definition before or in line %d [%s]).",n->getName(),sli.lineNumber,sli.fileName.c_str());
                                    yywarning(parerr);
                                    delete n;
                                  }
                                  else{
                                      ptr->push_back(n);
                                      parser_api->termtable->addConstant(n);
                                      n->setFileId(parser_api->fileid);
                                      n->setLineNumber(lexer->getLineNumber());
                                  }
                                  $$=ptr;
                                }
                                | constant_def_list PDDL_NAME
                                { vector<ConstantSymbol *> * ptr = (vector<ConstantSymbol *> *) $1;
                                  ConstantSymbol * n = new ConstantSymbol($2);
                                  // comprobamos que la constante no se encuentre ya definida
                                  ConstantSymbol * f = parser_api->termtable->getConstantFromName(n->getName());
                                  if(f != 0){
                                      // Comprobar que los tipos sean iguales a la hora de generar un
                                      // error o bien un warning
                                      SearchLineInfo sli(f);
                                      snprintf(parerr,256,"Redefinition of the constant `%s'. (previous definition before or in line %d [%s]).",n->getName(),sli.lineNumber,sli.fileName.c_str());
                                      yywarning(parerr);
                                      delete n;
                                 }
                                 else{
                                        ptr->push_back(n);
                                        parser_api->termtable->addConstant(n);
                                        n->setFileId(parser_api->fileid);
                                        n->setLineNumber(lexer->getLineNumber());
                                }
                                  $$=ptr;
                                }
                                ;


type:                           LEFTPAR PDDL_EITHER type_ref_list RIGHTPAR
                                {
                                $$ = $3;
                                }
                                | type_ref
                                { vector<Type *> * ptr = new vector<Type *> ;
                                if($1)
                                ptr->push_back((Type *)$1);
                                  $$=ptr;
                                }
                                | error
                                { vector<Type *> * ptr = new vector<Type *> ;
                                  $$=ptr;
                                }
                                ;

type_def_list:                        PDDL_NAME
                                { vector<Type *> * ptr = new vector<Type *>;
                                if(strcasecmp($1,"object")==0){
                                snprintf(parerr,256,"`object' is a built-in type and can't be redefined.");
                                yyerror(parerr);
                                }
                                else {
                                Type * t = new Type($1);
                                t->setFileId(parser_api->fileid);
                                t->setLineNumber(lexer->getLineNumber());
                                ptr->push_back(t);
                                }
                                  $$=ptr;
                                }
                                | type_def_list PDDL_NAME
                                { vector<Type *> * ptr = (vector<Type *> *) $1;
                                if(strcasecmp($2,"object")==0){
                                snprintf(parerr,256,"`object' is a built-in type and can't be redefined.");
                                yyerror(parerr);
                                }
                                else {
                                Type * t = new Type($2);
                                t->setFileId(parser_api->fileid);
                                t->setLineNumber(lexer->getLineNumber());
                                ptr->push_back(t);
                                }
                                  $$=ptr;
                                }
                                ;


type_ref:                        term_name
                                {
                                // buscamos la referencia al tipo en el dominio
                                string * s = (string *) $1;
                                if(strcasecmp(s->c_str(),"object") == 0) {
                                delete s;
                                $$ = 0;
                                }
                                else {
                                Type * t = parser_api->domain->getModificableType(s->c_str());
                                const Type * nt;
                                if(!t){
                                        // cuando ocurre esto el tipo se crea de todas las maneras
                                        // pero posiblemente se trate de un error que deber�n gestionar
                                        // las reglas padre. Observar que en caso de que se cree como
                                        // nuevo el tipo no tendr� ni fichero ni l�nea asociados.
                                        nt = parser_api->domain->addType(s->c_str());
                                        t = parser_api->domain->getModificableType(nt->getId());
                                        if(errtypes){
                                                snprintf(parerr,256,"Undeclared type `%s'.",s->c_str());
                                                yyerror(parerr);
                                        }
                                }
                                delete s;
                                    $$=t;
                                }
                                }
                                ;

type_ref_list:                        type_ref
                                { vector<const Type *> * ptr = new vector<const Type *> ;
                                if($1)
                                    ptr->push_back((const Type *)$1);
                                  $$=ptr;
                                }
                                | type_ref_list type_ref
                                { vector<const Type *> * ptr = (vector<const Type *> *) $1;
                                  if($2)
                                ptr->push_back((const Type *)$2);
                                  $$=ptr;
                                }
                                ;

predicates_def:                 LEFTPAR PDDL_PREDICATES  atomic_formula_skeleton_list RIGHTPAR
                                ;

atomic_formula_skeleton_list:   atomic_formula_skeleton
                                | atomic_formula_skeleton atomic_formula_skeleton_list
                                ;

atomic_formula_skeleton:        LEFTPAR term_name
                                {
                                  string * nameLit = (string *) $2;
                                  LiteralEffect * lit=0;
                                  Meta * mt=0;
                                  // buscamos si el literal ya est� definido en el diccionario de
                                  // nombres de literales
                                  ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),(const char *) nameLit->c_str());
                                  if(posit != (parser_api->domain->ldictionary).end()) {
                                      lit = new LiteralEffect(posit->second,parser_api->domain->metainfo.size());
                                      mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                      parser_api->domain->metainfo.push_back(mt);
                                  }
                                  else {
                                      lit = new LiteralEffect(idCounter++,parser_api->domain->metainfo.size());
                                      mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                      parser_api->domain->metainfo.push_back(mt);
                                      (parser_api->domain->ldictionary).insert(make_pair(lit->getName(), lit->getId()));
                                  }
                                  container = lit;
                                  delete nameLit;
                                }
                                variable_typed_list RIGHTPAR
                                {
                                  LiteralEffect * lit= (LiteralEffect *) container;
                                  container = 0;
                                  delete context;
                                  context=0;
                                  // buscar predicados duplicados
                                  int id = SearchDictionary(&(parser_api->domain->ldictionary),lit->getName())->second;
                                  LiteralTableRange r = parser_api->domain->getLiteralRange(id);
                                  bool duplicated=false;
                                  literaltablecit ite;
                                  for(ite = r.first; ite != r.second && !duplicated; ite++) {
                                      if(lit->sizep() == (*ite).second->sizep()) {
                                              // tenemos dos definiciones de predicado con el mismo nombre
                                              // y n�mero de argumentos ��No se como distinguirlos!!
                                              duplicated = true;
                                      }
                                  }

                                  if(!duplicated) {
                                      parser_api->domain->addLiteral(lit);
                                  }
                                  else {
                                      SearchLineInfo sli(lit->getMetaId());
                                      snprintf(parerr,256,"Conflicting predicate definition `%s' previous definition %d [%s].",lit->getName(),sli.lineNumber,sli.fileName.c_str());
                                      yyerror(parerr);
                                      delete lit;
                                  }
                                }
                                | error
                                ;

derived_formula_skeleton:       LEFTPAR term_name
                                {
                                  string * nameLit = (string *) $2;
                                  Axiom * lit=0;
                                  Meta * mt=0;
                                  // buscamos si el literal ya est� definido en el diccionario de
                                  // nombres de literales
                                  ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),(const char *) nameLit->c_str());
                                  if(posit != (parser_api->domain->ldictionary).end()) {
                                      lit = new Axiom(posit->second,parser_api->domain->metainfo.size());
                                      mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                      parser_api->domain->metainfo.push_back(mt);
                                  }
                                  else {
                                      lit = new Axiom(idCounter++,parser_api->domain->metainfo.size());
                                      mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                      parser_api->domain->metainfo.push_back(mt);
                                      (parser_api->domain->ldictionary).insert(make_pair(lit->getName(), lit->getId()));
                                  }
                                  container = lit;
                                  delete nameLit;
                                }
                                variable_typed_list RIGHTPAR
                                {
                                  Axiom * lit= (Axiom *) container;
                                  container = 0;
                                  // comprobaciones de correctitud
                                  // busco en el dominio los literales con el nombre capturado
                                  // y el n�mero de argumentos adecuado
                                  // Esto sirve para poner los tipos seg�n est�n definidos en la tabla de literales.
                                  int id = SearchDictionary(&(parser_api->domain->ldictionary),lit->getName())->second;
                                  LiteralTableRange r = parser_api->domain->getLiteralRange(id);
                                  bool unificacion=false;
                                  vector<Literal *> candidates;
                                  vector<Literal *>::const_iterator j;
                                  for(literaltablecit i = r.first; i != r.second && !unificacion; i++) {
                                      candidates.push_back((*i).second);
                                      Unifier u;
                                      if(unify3(lit->getParameters(),(*i).second->getParameters(),&u)){
                                          u.applyTypeSubstitutions(0);
                                          unificacion=true;
                                       }
                                  }
                                  if(!unificacion){
                                          snprintf(parerr,256,"(1) No matching predicate for `%s'.",lit->toString());
                                          yyerror(parerr);
                                          if(candidates.size() > 0) {
                                          *errflow << "Possible candidates:" << endl;
                                            for(j=candidates.begin();j!=candidates.end();j++) {
                                          SearchLineInfo sli((*j)->getMetaId());
                                               *errflow << "\t[" << sli.fileName << "]:" << sli.lineNumber;
                                          (*j)->printL(errflow,1);
                                          *errflow << endl;
                                            }
                                          }
                                  }
                                  parser_api->domain->addAxiom(lit);
                                  $$ = lit;
                                }
                                | error
                                {
                                  $$ =0;
                                }
                                ;


variable_typed_list:            {contador = 0;} var_typel
                                ;

var_typel:                        /* empty */
                                | var_typel variable
                                {
                                  // si tenemos un contenedor donde meter la variable
                                  // la a�adimos
                                  if(!container)
                                      *errflow << "(mensaje recordatorio) (-- Aqui deber�a haber un container --)" << endl;
                                  if(container){
                                      if(container->searchTermId($2->first) != container->parametersEnd()){
                                          // es raro que se tenga una variable repetida
                                          snprintf(parerr,256,"Duplicated variable: `%s'.",parser_api->termtable->getVariable(*$2)->getName());
                                          yywarning(parerr);
                                      }
                                      container->addParameter(*$2);
                                  }
                                }
                                | var_typel PDDL_HYPHEN type
                                {
                                  if(!container)
                                      *errflow << "(mensaje recordatorio) (-- Aqui deber�a haber un container --)" << endl;
                                  if(container && $3) {
                                      // recorremos hacia atr�s todas las variables insertadas anteriormente,
                                      // hasta encontrar la primera que no tiene tipo asignado.
                                      // A partir de este asignamos type
                                      vector<Type *> * vt = (vector<Type *> *)$3;
                                      if(!vt->empty()) {
                                          TestTypeTree()(vt);
                                          KeyList * kl = container->getModificableParameters();
                                          KeyList::iterator i,e;
                                          e = kl->end();
                                          for(i=kl->begin() + contador;i!=e;i++) {
                                                  parser_api->termtable->getVariable(*i)->addTypes2(vt);
                                          }
                                      }
                                      delete vt;
                                      contador = container->getModificableParameters()->size();
                                  }
                                  else if(!$3) {
                                      snprintf(parerr,256,"Type expected after `-'.");
                                      yywarning(parerr);
                                  }
                                }
                                ;

functions_def:                  LEFTPAR PDDL_FUNCTIONS function_typed_list RIGHTPAR
                                {
                                    if(!parser_api->domain->errfluents && !parser_api->domain->hasRequirement(":fluents"))
                                    {
                                        parser_api->domain->errfluents = true;
                                        yyerror("Using a clause that requires `:fluents' and is not declared in requirements clause");
                                    }
                                }
                                ;

function_typed_list:            /* empty */
                                // gestionar contexto
                                | function_typed_list function_def
                                ;

function_def:                 LEFTPAR term_name
                                {
                                string * nameLit = (string *) $2;
                                PyDefFunction * lit=0;
                                Meta * mt=0;
                                // buscamos si el literal ya est� definido en el diccionario de
                                // nombres de literales
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),(const char *) nameLit->c_str());
                                if(posit != (parser_api->domain->ldictionary).end()) {
                                lit = new PyDefFunction(posit->second,parser_api->domain->metainfo.size());
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                }
                                else
                                {
                                lit = new PyDefFunction(idCounter++,parser_api->domain->metainfo.size());
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                (parser_api->domain->ldictionary).insert(make_pair(lit->getName(), lit->getId()));
                                }
                                container = lit;
                                delete nameLit;
                                context = new LDictionary;
                                }
                                variable_typed_list RIGHTPAR opt_type code
                                {
                                PyDefFunction * lit= (PyDefFunction *) container;
                                container = 0;
                                delete context;
                                context=0;
                                // buscar predicados duplicados
                                int id = SearchDictionary(&(parser_api->domain->ldictionary),lit->getName())->second;
                                LiteralTableRange r = parser_api->domain->getLiteralRange(id);
                                bool duplicated=false;
                                literaltablecit ite;
                                for(ite = r.first; ite != r.second && !duplicated; ite++)
                                {
                                if(lit->sizep() == (*ite).second->sizep())
                                        // tenemos dos definiciones de predicado con el mismo nombre
                                        // y n�mero de argumentos ��No se como distinguirlos!!
                                        duplicated = true;
                                }

                                if(!duplicated){
                                parser_api->domain->addLiteral(lit);
                                }
                                else {
                                SearchLineInfo sli(lit->getMetaId());
                                snprintf(parerr,256,"Conflicting function definition `%s' previous definition %d [%s].",lit->getName(),sli.lineNumber,sli.fileName.c_str());
                                yyerror(parerr);
                                delete lit;
                                }
                                if($7){
                                if(!lit->setCode($7)){
                                snprintf(parerr,256,"Error in Python script code. Function: %s.",lit->getName());
                                yyerror(parerr);
                                }
                                }
                                vector<Type *> * types = (vector<Type *> *) $6;
                                if(types){
                                        // en pddl estandard se supone que el tipo es un n�mero
                                        // pero lo dejamos abierto para la extensi�n. De momento
                                        // se ignora el tipo en las funciones.
                                        lit->addTypes(types);
                                        delete types;
                                }
                                }
                                ;

opt_type:                        /*empty*/
                                {
                                Type * number = parser_api->domain->getModificableType("number");
                                vector<Type *> * vt = new vector<Type *>;
                                vt->push_back(number);
                                $$=vt;
                                }
                                | PDDL_HYPHEN type
                                {$$=$2;}
                                ;

code:                           /*empty*/
                                {$$ = 0;}
                                | PYTHON_CODE
                                {
                                    static string code = "";
                                code = $1;
                                    if(PYTHON_FLAG) {
                                        $$ = code.c_str();
                                    }
                                    else {
                                        yyerror("Parser compiled without Python support. Install python and recompile.");
                                        $$ = 0;
                                    }
                                 }
                                ;

structure_def_list:             /* empty */
                                |  structure_def  structure_def_list
                                ;

structure_def:                   action_def
                                | durative_action_def
                                | derived_def
                                {
                                  if(!parser_api->domain->errder && !parser_api->domain->hasRequirement(":derived-predicates"))
                                  {
                                        parser_api->domain->errhtn = true;
                                        yyerror("Using a clause that requires `:derived-predicates' and is not declared in requirements clause");
                                  }
                                }
                                | htn_task_def
                                | error
                                ;

action_def:                     LEFTPAR
                                    PDDL_ACTION term_name
                                {
                                context = new LDictionary;
                                string * name = (string *) $3;
                                PrimitiveTask * priTask=0;
                                Meta * mt = 0;
                                // buscamos en el diccionario si la acci�n ya tiene un identificador
                                // asociado, en cuyo caso lo reutilizamos
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),name->c_str());
                                      if(posit != (parser_api->domain->ldictionary).end()) {
                                        priTask = new PrimitiveTask(posit->second,parser_api->domain->metainfo.size());
                                        mt = new Meta(name->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                }
                                      else
                                      {
                                        priTask = new PrimitiveTask(idCounter++,parser_api->domain->metainfo.size());
                                        mt = new Meta(name->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                          (parser_api->domain->ldictionary).insert(make_pair(priTask->getName(),priTask->getId()));
                                      }
                                container = priTask;
                                delete name;
                                $<otype>$ = priTask;
                                }
                                    PDDL_PARAMETERS LEFTPAR variable_typed_list RIGHTPAR
                                meta_list
                                {
                                        // si hay alg�n tag
                                        if($9){
                                        TagVector * tv = (TagVector *) $9;
                                        tagv_ite tb, te = tv->end();
                                        int mid = ((PrimitiveTask *) container)->getMetaId();
                                        for(tb = tv->begin();tb!=te;tb++){
                                        parser_api->domain->metainfo[mid]->addTag(*tb);
                                        }
                                        tv->clear();
                                        delete tv;
                                        }
                                        container = 0;
                                }
                                    preconditions_def
                                    effect_def
                                RIGHTPAR
                                {
                                PrimitiveTask * priTask= (PrimitiveTask *) $<otype>4;
                                priTask->setPrecondition((Goal *) $11);
                                priTask->setEffect((Effect *) $12);
                                parser_api->domain->addTask(priTask);
                                delete context;
                                context = 0;
                                }
                                ;

htn_task_def:                   LEFTPAR
                                    HTN_TASK term_name
                                {
                                context = new LDictionary;
                                string * name = (string *) $3;
                                CompoundTask * compTask=0;
                                Meta * mt=0;
                                // buscamos en el diccionario si la acci�n ya tiene un identificador
                                // asociado, en cuyo caso lo reutilizamos
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),name->c_str());
                                      if(posit != (parser_api->domain->ldictionary).end()) {
                                        compTask = new CompoundTask(posit->second,parser_api->domain->metainfo.size());
                                        mt = new Meta(name->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                }
                                      else
                                      {
                                        compTask = new CompoundTask(idCounter++,parser_api->domain->metainfo.size());
                                        mt = new Meta(name->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                          (parser_api->domain->ldictionary).insert(make_pair(compTask->getName(),compTask->getId()));
                                      }
                                container = compTask;
                                delete name;
                                cbuilding = compTask;
                                }
                                    PDDL_PARAMETERS LEFTPAR variable_typed_list RIGHTPAR
                                meta_list
                                    methods_def_body
                                RIGHTPAR
                                {
                                if($9){
                                        TagVector * tv = (TagVector *) $9;
                                        tagv_ite tb, te = tv->end();
                                        int mid = ((CompoundTask *) container)->getMetaId();
                                        for(tb = tv->begin();tb!=te;tb++)
                                        parser_api->domain->metainfo[mid]->addTag(*tb);
                                        tv->clear();
                                        delete tv;
                                }
                                if(!parser_api->domain->errhtn && !parser_api->domain->hasRequirement(":htn-expansion"))
                                {
                                        parser_api->domain->errhtn = true;
                                        yyerror("Using a clause that requires `:htn-expansion' and is not declared in requirements clause");
                                }
                                CompoundTask * compTask= cbuilding;
                                parser_api->domain->addTask(compTask);
                                delete context;
                                context = 0;
                                cbuilding=0;
                                }
                                ;


duration_constraints:           simple_duration_constraint
                                {
                                vector<TCTR> * v = new vector<TCTR>;
                                TCTR * ele = (TCTR *) $1;
                                v->push_back(*ele);
                                $$= v;
                                }
                                | LEFTPAR PDDL_AND sduration_constraint_list RIGHTPAR
                                {
                                vector<TCTR> * v = (vector<TCTR> *) $3;
                                $$= v;
                                }
                                | LEFTPAR RIGHTPAR
                                {
                                vector<TCTR> * v = new vector<TCTR>;
                                $$= v;
                                }
                                ;

sdur_constraint:                LEFTPAR EQUAL PDDL_DURATIONVAR fluent_exp RIGHTPAR
                                {
                                    static TCTR p;
                                    p = make_pair(EQ_DUR,(Evaluable *)$4);
                                    $$ = &p;
                                }
                                | LEFTPAR LESS_EQUAL PDDL_DURATIONVAR fluent_exp RIGHTPAR
                                {
                                    static TCTR p;
                                    p = make_pair(LEQ_DUR,(Evaluable *)$4);
                                    $$ = &p;
                                }
                                | LEFTPAR GREATHER_EQUAL  PDDL_DURATIONVAR  fluent_exp RIGHTPAR
                                {
                                    static TCTR p;
                                    p = make_pair(GEQ_DUR,(Evaluable *)$4);
                                    $$ = &p;
                                }
                                | LEFTPAR LESS PDDL_DURATIONVAR fluent_exp RIGHTPAR
                                {
                                    static TCTR p;
                                    p = make_pair(LESS_DUR,(Evaluable *)$4);
                                    $$ = &p;
                                }
                                | LEFTPAR GREATHER  PDDL_DURATIONVAR  fluent_exp RIGHTPAR
                                {
                                    static TCTR p;
                                    p = make_pair(GRE_DUR,(Evaluable *)$4);
                                    $$ = &p;
                                }

simple_duration_constraint:     sdur_constraint
                                {
                                    $$ = $1;
                                }
                                | LEFTPAR EQUAL STARTVAR fluent_exp RIGHTPAR
                                {
                                static TCTR p;
                                p = make_pair(EQ_START,(Evaluable *)$4);
                                $$ = &p;
                                }
                                | LEFTPAR LESS_EQUAL STARTVAR fluent_exp RIGHTPAR
                                {
                                static TCTR p;
                                p = make_pair(LEQ_START,(Evaluable *)$4);
                                $$ = &p;
                                }
                                | LEFTPAR GREATHER_EQUAL STARTVAR fluent_exp RIGHTPAR
                                {
                                static TCTR p;
                                p = make_pair(GEQ_START,(Evaluable *)$4);
                                $$ = &p;
                                }
                                | LEFTPAR LESS STARTVAR fluent_exp RIGHTPAR
                                {
                                static TCTR p;
                                p = make_pair(LESS_START,(Evaluable *)$4);
                                $$ = &p;
                                }
                                | LEFTPAR GREATHER STARTVAR fluent_exp RIGHTPAR
                                {
                                static TCTR p;
                                p = make_pair(GRE_START,(Evaluable *)$4);
                                $$ = &p;
                                }
                                | LEFTPAR EQUAL ENDVAR fluent_exp RIGHTPAR
                                {
                                static TCTR p;
                                p = make_pair(EQ_END,(Evaluable *)$4);
                                $$ = &p;
                                }
                                | LEFTPAR LESS_EQUAL ENDVAR fluent_exp RIGHTPAR
                                {
                                static TCTR p;
                                p = make_pair(LEQ_END,(Evaluable *)$4);
                                $$ = &p;
                                }
                                | LEFTPAR GREATHER_EQUAL ENDVAR fluent_exp RIGHTPAR
                                {
                                static TCTR p;
                                p = make_pair(GEQ_END,(Evaluable *)$4);
                                $$ = &p;
                                }
                                | LEFTPAR LESS ENDVAR fluent_exp RIGHTPAR
                                {
                                static TCTR p;
                                p = make_pair(LESS_END,(Evaluable *)$4);
                                $$ = &p;
                                }
                                | LEFTPAR GREATHER ENDVAR fluent_exp RIGHTPAR
                                {
                                static TCTR p;
                                p = make_pair(GRE_END,(Evaluable *)$4);
                                $$ = &p;
                                }
                                ;

sduration_constraint_list:      simple_duration_constraint
                                {
                                vector<pair<int,Evaluable *> > * v = new vector<pair<int,Evaluable *> >;
                                v->push_back(*((pair<int,Evaluable *> *) $1));
                                $$= v;
                                }
                                | sduration_constraint_list simple_duration_constraint
                                {
                                vector<pair<int,Evaluable *> > * v = (vector<pair<int,Evaluable *> > *) $1;
                                v->push_back(*((pair<int,Evaluable *> *) $2));
                                $$= v;
                                }
                                ;

methods_def_body:               method_list
                                | LEFTPAR EXCLAMATION method_list RIGHTPAR
                                {
                                cbuilding->setFirst();
                                }
                                | LEFTPAR RANDOM EXCLAMATION method_list RIGHTPAR
                                {
                                cbuilding->setFirst();
                                cbuilding->setRandom();
                                }
                                | LEFTPAR EXCLAMATION RANDOM method_list RIGHTPAR
                                {
                                cbuilding->setFirst();
                                cbuilding->setRandom();
                                }
                                | LEFTPAR RANDOM method_list RIGHTPAR
                                {
                                cbuilding->setRandom();
                                }
                                ;

method_list:                    method_body
                                | method_list method_body
                                ;

method_body:                    LEFTPAR
                                    HTN_METHOD
                                    term_name
                                {container=cbuilding;}
                                meta_list
                                {container=0;}
                                {
                                string * name = (string *) $3;
                                // comprobar que el m�todo no se haya definido con anterioridad
                                methodcit i,e = cbuilding->getEndMethod();
                                bool definido=false;
                                for(i=cbuilding->getBeginMethod();i!=e && !definido;i++)
                                        if(!strcasecmp((*i)->getName(),name->c_str())){
                                        definido = true;
                                        break;
                                        }
                                if(definido){
                                        SearchLineInfo sli((*i)->getMetaId());
                                        snprintf(parerr,256,"The method `%s' is already defined for this task in or before %d [%s].",(*i)->getName(),sli.lineNumber,sli.fileName.c_str());
                                        yywarning(parerr);
                                }
                                Method * method=0;
                                      method = new Method(parser_api->domain->metainfo.size(),cbuilding);
                                Meta * mt = new Meta(name->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                if($5){
                                        TagVector * tv = (TagVector *) $5;
                                        tagv_ite tb, te = tv->end();
                                        int mid = method->getMetaId();
                                        for(tb = tv->begin();tb!=te;tb++)
                                        parser_api->domain->metainfo[mid]->addTag(*tb);
                                        tv->clear();
                                        delete tv;
                                }
                                delete name;

                                // forzamos a que cada m�todo tenga su propio contexto
                                oldContext = context;
                                context = new LDictionary();
                                // A�adimos al contexto las variables que tengo
                                keylistcit j, k = method->parametersEnd();
                                for(j=method->parametersBegin();j!=k;j++)
                                        context->insert(make_pair(parser_api->termtable->getVariable((*j))->getName(),(*j).first));
                                $<otype>$ = method;
                                }
                                    preconditions_def
                                    HTN_TASKS task_network
                                RIGHTPAR
                                {
                                Method * method = (Method *) $<otype>7;
                                method->setPrecondition((Goal *) $8);
                                method->setTaskNetwork((TaskNetwork *) $10);
                                cbuilding->addMethod(method);
                                delete context;
                                context = oldContext;
                                oldContext = 0;
                                }
                                ;

meta_list:                        /*empty */
                                {
                                $$ = 0;
                                }
                                |
                                META LEFTPAR tag_list RIGHTPAR
                                {
                                $$ = $3;
                                }
                                ;

tag_list:                        /*empty*/
                                {
                                $$ = new TagVector();
                                }
                                | tag_list tag_element
                                {
                                TagVector * tv = (TagVector *) $1;
                                tv->push_back((Tag *)$2);
                                $$ = tv;
                                }
                                ;

tag_element:                        LEFTPAR TAG PDDL_NAME
                                {
                                const char * name = (const char *) $3;
                                TextTag * mt = new TextTag(name);
                                $<otype>$ = mt;
                                }
                                HTN_TEXT RIGHTPAR
                                {
                                TextTag * mt = (TextTag *) $<otype>4;
                                    string text = (const char *) $5;

                                // Fijar el valor
                                if(container){
                                        string value = processTextTag(text, container);
                                        mt->setValue(value.c_str());
                                }
                                else{
                                        mt->setValue(text.c_str());
                                }

                                    $$ = mt;
                                    if(!parser_api->domain->errmetatags && !parser_api->domain->hasRequirement(":metatags"))
                                    {
                                        parser_api->domain->errmetatags = true;
                                        yyerror("Using a clause that requires `:metatags' and is not declared in requirements clause");
                                    }
                                }
                                ;

task_def:                       atomic_task_formula
                                {$$=$1;}
                                | LEFTPAR HTN_ACHIEVE goal_def RIGHTPAR
                                {$$=0; *errflow << "No implementado achieve" << endl;}
                                | inlinetask
                                {$$ =$1;}
                                ;

inlinetask:                        LEFTPAR HTN_INLINE
                                {
                                PrimitiveTask * priTask=0;
                                Meta * mt = 0;
                                // buscamos en el diccionario si la acci�n ya tiene un identificador
                                // asociado, en cuyo caso lo reutilizamos
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),":inline");
                                      if(posit != (parser_api->domain->ldictionary).end()) {
                                        priTask = new PrimitiveTask(posit->second,parser_api->domain->metainfo.size());
                                        mt = new Meta(":inline",lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                }
                                      else
                                      {
                                        priTask = new PrimitiveTask(idCounter++,parser_api->domain->metainfo.size());
                                        mt = new Meta(":inline",lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                          (parser_api->domain->ldictionary).insert(make_pair(priTask->getName(),priTask->getId()));
                                      }
                                priTask->setInline();
                                $<otype>$ = priTask;
                                }
                                goal_def effect RIGHTPAR
                                {
                                        PrimitiveTask * priTask= (PrimitiveTask *) $<otype>3;
                                        priTask->setPrecondition((Goal *) $4);
                                        priTask->setEffect((Effect *) $5);
                                        $<otype>$ = priTask;
                                }
                                | LEFTPAR HTN_INLINECUT
                                {
                                PrimitiveTask * priTask=0;
                                Meta * mt = 0;
                                // buscamos en el diccionario si la acci�n ya tiene un identificador
                                // asociado, en cuyo caso lo reutilizamos
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),":!inline");
                                      if(posit != (parser_api->domain->ldictionary).end()) {
                                        priTask = new PrimitiveTask(posit->second,parser_api->domain->metainfo.size());
                                        mt = new Meta(":!inline",lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                }
                                      else
                                      {
                                        priTask = new PrimitiveTask(idCounter++,parser_api->domain->metainfo.size());
                                        mt = new Meta(":!inline",lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                          (parser_api->domain->ldictionary).insert(make_pair(priTask->getName(),priTask->getId()));
                                      }
                                priTask->setInline(2);
                                $<otype>$ = priTask;
                                }
                                goal_def effect RIGHTPAR
                                {
                                        PrimitiveTask * priTask= (PrimitiveTask *) $<otype>3;
                                        priTask->setPrecondition((Goal *) $4);
                                        priTask->setEffect((Effect *) $5);
                                        $$ = priTask;
                                }
                                ;

atomic_task_formula:            LEFTPAR term_name
                                {
                                string * name = (string *) $2;
                                TaskHeader * th=0;
                                MetaTH * mt = 0;
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),name->c_str());
                                if(posit != (parser_api->domain->ldictionary).end()) {
                                th = new TaskHeader(posit->second,parser_api->domain->metainfo.size());
                                mt = new MetaTH(name->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back((Meta *)mt);
                                }
                                else
                                {
                                th = new TaskHeader(idCounter++,parser_api->domain->metainfo.size());
                                mt = new MetaTH(name->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back((Meta *)mt);
                                (parser_api->domain->ldictionary).insert(make_pair(th->getName(), th->getId()));
                                }
                                container = th;
                                delete name;
                                contador=0;
                                }
                                term_list RIGHTPAR
                                {
                                // se deja para una comprobaci�n posterior el ver
                                // si el th se corresponde con alguna tarea del dominio
                                TaskHeader * th= (TaskHeader *) container;
                                container = 0;
                                $$ = th;
                                }
                                ;

task_network:                   task_element
                                {
                                $$ = $1;
                                }
                                | LEFTPAR RIGHTPAR
                                {
                                // esto es una noop
                                Meta * mt = 0;
                                PrimitiveTask * priTask;
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),":!inline");
                                    if(posit != (parser_api->domain->ldictionary).end()) {
                                        priTask = new PrimitiveTask(posit->second,parser_api->domain->metainfo.size());
                                        mt = new Meta(":inline",lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                }
                                    else
                                    {
                                        priTask = new PrimitiveTask(idCounter++,parser_api->domain->metainfo.size());
                                        mt = new Meta(":inline",lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                          (parser_api->domain->ldictionary).insert(make_pair(priTask->getName(),priTask->getId()));
                                    }
                                priTask->setInline();
                                TaskNetwork * tn = new TaskNetwork(priTask);
                                $$ = tn;
                                }
                                ;


task_list:                      task_list task_element
                                {
                                vector<TaskNetwork *> * vt = (vector<TaskNetwork *> *) $1;
                                vt->push_back((TaskNetwork *) $2);
                                $$ = vt;
                                }
                                | task_element
                                {
                                vector<TaskNetwork *> * vt = new vector<TaskNetwork *>;
                                vt->push_back((TaskNetwork *) $1);
                                $$ = vt;
                                }
                                ;

task_element:                   task_def
                                {
                                $$ = new TaskNetwork((Task *) $1);
                                }
                                | EXCLAMATION task_def
                                {
                                TaskNetwork * tn = new TaskNetwork((Task *) $2);
                                tn->setInmediate(0,true);
                                $$ = tn;
                                }
                                | LESS duration_constraints task_list GREATHER
                                {
                                vector<TCTR> * v = (vector<TCTR> *) $2;
                                vector<TCTR>::iterator iv, ev = v->end();
                                vector<TaskNetwork *> * vt = (vector<TaskNetwork *> *) $3;

                                TaskNetwork * tn = vt->front();
                                vector<TaskNetwork *>::iterator i, e = vt->end();
                                for(i = vt->begin() + 1; i != e; i++) {
                                        tn->merge(*i);
                                        delete (*i);
                                }
                                delete vt;

                                if(v){
                                        for(iv = v->begin();iv!=ev;iv++){
                                        tn->addTConstraint((*iv));
                                        }
                                        delete v;
                                }

                                intvit j, je = tn->getSuccEnd(0);
                                for(j=tn->getSuccBegin(0);j!=je;j++)
                                        tn->setBTTask((*j)-1);
                                $$ = tn;
                                }
                                | LESS task_list GREATHER
                                {
                                vector<TaskNetwork *> * vt = (vector<TaskNetwork *> *) $2;

                                TaskNetwork * tn = vt->front();
                                vector<TaskNetwork *>::iterator i, e = vt->end();
                                for(i = vt->begin() + 1; i != e; i++) {
                                        tn->merge(*i);
                                        delete (*i);
                                }
                                delete vt;
                                intvit j, je = tn->getSuccEnd(0);
                                for(j=tn->getSuccBegin(0);j!=je;j++)
                                        tn->setBTTask((*j)-1);
                                $$ = tn;
                                }
                                | LEFTPAR duration_constraints task_list RIGHTPAR
                                {
                                vector<TCTR> * v = (vector<TCTR> *) $2;
                                vector<TCTR>::iterator iv, ev = v->end();
                                vector<TaskNetwork *> * vt = (vector<TaskNetwork *> *) $3;

                                TaskNetwork * tn = vt->front();
                                vector<TaskNetwork *>::iterator i, e = vt->end();
                                for(i = vt->begin() + 1; i != e; i++) {
                                        tn->join(*i);
                                        delete (*i);
                                }
                                delete vt;

                                if(v){
                                        for(iv = v->begin();iv!=ev;iv++){
                                        tn->addTConstraint((*iv));
                                        }
                                        delete v;
                                }

                                $$ = tn;
                                }
                                | LEFTPAR task_list RIGHTPAR
                                {
                                vector<TaskNetwork *> * vt = (vector<TaskNetwork *> *) $2;

                                TaskNetwork * tn = vt->front();
                                vector<TaskNetwork *>::iterator i, e = vt->end();
                                for(i = vt->begin() + 1; i != e; i++) {
                                        tn->join(*i);
                                        delete (*i);
                                }
                                delete vt;
                                $$ = tn;
                                }
                                | LEFTBRAC duration_constraints task_list RIGHTBRAC
                                {
                                vector<TCTR> * v = (vector<TCTR> *) $2;
                                vector<TCTR>::iterator iv, ev = v->end();
                                vector<TaskNetwork *> * vt = (vector<TaskNetwork *> *) $3;

                                TaskNetwork * tn = vt->front();
                                vector<TaskNetwork *>::iterator i, e = vt->end();
                                for(i = vt->begin() + 1; i != e; i++) {
                                        tn->merge(*i);
                                        delete (*i);
                                }
                                delete vt;

                                if(v){
                                        for(iv = v->begin();iv!=ev;iv++){
                                        tn->addTConstraint((*iv));
                                        }
                                        delete v;
                                }

                                $$ = tn;
                                }
                                | LEFTBRAC task_list RIGHTBRAC
                                {
                                vector<TaskNetwork *> * vt = (vector<TaskNetwork *> *) $2;

                                TaskNetwork * tn = vt->front();
                                vector<TaskNetwork *>::iterator i, e = vt->end();
                                for(i = vt->begin() + 1; i != e; i++) {
                                        tn->merge(*i);
                                        delete (*i);
                                }
                                delete vt;
                                $$ = tn;
                                }
                                ;

preconditions_def:               /* empty */
                                { $$ = 0;}
                                | PDDL_PRECONDITION goal_def
                                { $$ = $2;}
                                ;

goal_def:                       LEFTPAR RIGHTPAR
                                { $$ = 0;}
                                | LEFTPAR EXCLAMATION goal_def RIGHTPAR
                                {
                                CutGoal * cg = new CutGoal();
                                cg->setGoal((Goal *)$3);
                                $<otype>$ = cg;
                                }
                                | LEFTPAR SORTBY
                                {
                                SortGoal * sg = new SortGoal();
                                container = sg;
                                $<otype>$ = sg;
                                }
                                v_list
                                {
                                container = 0;
                                }
                                goal_def RIGHTPAR
                                {
                                SortGoal * sg = (SortGoal *) $<otype>3;
                                Goal * g = (Goal *) $6;
                                sg->setGoal(g);
                                // Buscamos que la variable por la que queremos ordenar al menos
                                // aparezca en la precondicion.
                                keylistcit i, e = sg->endp();
                                for(i=sg->beginp();i!=e;i++){
                                        if(!g->hasTerm((*i).first)) {
                                        snprintf(parerr,256,"The variable you are trying to sort by (%s), doesn't appear in goal!.",parser_api->termtable->getVariable((*i))->getName());
                                        yyerror(parerr);
                                        }
                                }
                                $<otype>$ = sg;
                                }
                                | LEFTPAR PDDL_BIND variable
                                {
                                $<otype>$ = new FluentVar((pkey *)$3);
                                }
                                fluent_exp RIGHTPAR
                                {
                                FluentVar * fv = (FluentVar *) $<otype>4;
                                BoundGoal * bg = new BoundGoal(fv,(Evaluable *)$5);
                                    if(!parser_api->domain->errfluents && !parser_api->domain->hasRequirement(":fluents"))
                                    {
                                        parser_api->domain->errfluents = true;
                                        yyerror("Using a clause that requires `:fluents' and is not declared in requirements clause");
                                    }
                                $$ = bg;
                                }
                                | LEFTPAR PPRINT simple_goal_def RIGHTPAR
                                {
                                PrintGoal * pg = new PrintGoal();
                                pg->setGoal((Goal *) $3);
                                $$ = pg;
                                }
                                | LEFTPAR PPRINT HTN_TEXT RIGHTPAR
                                {
                                PrintGoal * pg = new PrintGoal();
                                pg->setStr($3);
                                $$ = pg;
                                }
                                | LEFTPAR PPRINT
                                {
                                PrintGoal * pg = new PrintGoal();
                                container = pg;
                                $<otype>$ = pg;
                                contador = 0;
                                }
                                term_list RIGHTPAR
                                {
                                container = 0;
                                $$ = $<otype>3;
                                }
                                | simple_goal_def
                                {
                                $$ = $1;
                                }
                                | timed_goal
                                {
                                $$ = $1;
                                }
                                ;

simple_goal_def:                LEFTPAR PDDL_AND
                                {
                                AndGoal * ag = new AndGoal();
                                gcontainer.push_back(ag);
                                $<otype>$ = ag;
                                }
                                goal_def_list
                                {
                                gcontainer.pop_back();
                                }
                                RIGHTPAR
                                {
                                AndGoal * ag = (AndGoal *) $<otype>3;
                                $$ = ag;
                                }
                                | LEFTPAR PDDL_OR
                                {
                                OrGoal * ag = new OrGoal();
                                gcontainer.push_back(ag);
                                $<otype>$ = ag;
                                }
                                goal_def_list
                                {
                                gcontainer.pop_back();
                                }
                                RIGHTPAR
                                {
                                OrGoal * ag = (OrGoal *) $<otype>3;
                                $$ = ag;
                                }
                                | LEFTPAR PDDL_NOT goal_def RIGHTPAR
                                { Goal * g = (Goal *) $3;
                                if(g){
                                if(g->getPolarity())
                                g->setPolarity(false);
                                else
                                g->setPolarity(true);
                                }
                                /* negative-preconditions */
                                if(!parser_api->domain->errnegative && !parser_api->domain->hasRequirement(":negative-preconditions"))
                                {
                                        parser_api->domain->errnegative = true;
                                yyerror("Using a clause that requires `:negative-preconditions' and is not declared in requirements clause");
                                }
                                $$ = g;
                                }
                                | LEFTPAR PDDL_IMPLY goal_def goal_def RIGHTPAR
                                {
                                $$ = new ImplyGoal((Goal *) $3, (Goal *) $4);
                                    if(!parser_api->domain->errdisjunctive && !parser_api->domain->hasRequirement(":disjunctive-preconditions"))
                                    {
                                        parser_api->domain->errdisjunctive = true;
                                        yyerror("Using a clause that requires `:disjunctive-preconditions' and is not declared in requirements clause");
                                    }
                                }
                                | LEFTPAR PDDL_EXISTS
                                {
                                ExistsGoal * fag = new ExistsGoal();
                                container = fag;
                                $<otype>$ = fag;
                                }
                                    LEFTPAR variable_typed_list RIGHTPAR
                                {
                                container = 0;
                                }
                                  goal_def RIGHTPAR
                                {
                                ExistsGoal * fag = (ExistsGoal *) $<otype>3;
                                Goal * g = (Goal *) $8;
                                fag->setGoal(g);
                                /* universal-preconditions */
                                    if(!parser_api->domain->errexistential && !parser_api->domain->hasRequirement(":existential-preconditions"))
                                    {
                                        parser_api->domain->errexistential = true;
                                        yyerror("Using a clause that requires `:existential-preconditions' and is not declared in requirements clause");
                                    }
                                $$ = fag;
                                }
                                | LEFTPAR PDDL_FORALL
                                {
                                // creamos el forall, y lo ponemos como
                                // el elemento que va a contener a las variables
                                ForallGoal * fag = new ForallGoal();
                                container = fag;
                                $<otype>$ = fag;
                                }
                                    LEFTPAR variable_typed_list RIGHTPAR
                                {
                                container = 0;
                                }
                                  goal_def RIGHTPAR
                                {
                                ForallGoal * fag = (ForallGoal *) $<otype>3;
                                Goal * g = (Goal *) $8;
                                fag->setGoal(g);
                                /* universal-preconditions */
                                    if(!parser_api->domain->erruniversal && !parser_api->domain->hasRequirement(":universal-preconditions"))
                                    {
                                        parser_api->domain->erruniversal = true;
                                        yyerror("Using a clause that requires `:universal-preconditions' and is not declared in requirements clause");
                                    }
                                $$ = fag;
                                }
                                | f_comp
                                { $$ = $1;}
                                | atomic_formula_term_goal
                                { $$ = (LiteralGoal *) $1;}
                                ;

timed_goal:                     LEFTPAR time_specifier simple_goal_def RIGHTPAR
                                {
                                Goal * g = (Goal *) $3;
                                TimeInterval * ti = (TimeInterval *) $2;
                                g->setTime(ti);
                                    if(!parser_api->domain->errdurative && !parser_api->domain->hasRequirement(":durative-actions"))
                                    {
                                        parser_api->domain->errdurative = true;
                                        yyerror("Using a clause that requires `:durative-actions' and is not declared in requirements clause");
                                    }
                                    if(!isDurative) {
                                        yyerror("Using a durative goal inside a non durative action.");
                                    }
                                $$ = g;
                                };

order:                                /* empty */
                                {
                                $$ = DEFAULT_CRITERIA;
                                }
                                | ASC
                                {
                                $$ = SASC;
                                }
                                | DESC
                                {
                                $$ = SDESC;
                                }
                                ;

v_list:                                v_crit
                                | v_crit v_list
                                ;

v_crit:                                variable order
                                {
                                pkey ret = *$1;
                                SortGoal * sg = (SortGoal *) container;
                                sg->addCriteria((SortType)$2);
                                sg->addParameter(ret);
                                };

goal_def_list:                  /* empty */
                                | goal_def_list goal_def
                                {
                                if($2){
                                ContainerGoal * lc = (ContainerGoal *) gcontainer.back();
                                lc->addGoalByRef((Goal *) $2);

                                }
                                }
                                ;

atomic_formula_term_effect:     literal_effect
                                { $$ = (LiteralEffect *) $1;}
                                | LEFTPAR MAINTAIN literal_effect RIGHTPAR
                                { LiteralEffect * lit = (LiteralEffect *) $3;
                                lit->setMaintain(true);
                                $$ = lit;
                                }
                                ;



literal_effect:                        LEFTPAR term_name
                                {
                                string * nameLit = (string *) $2;
                                LiteralEffect * lit=0;
                                Meta * mt=0;
                                // buscamos si el literal ya est� definido en el diccionario de
                                // nombres de literales (deber�a estarlo)
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),nameLit->c_str());
                                if(posit != (parser_api->domain->ldictionary).end()) {
                                lit = new LiteralEffect(posit->second,parser_api->domain->metainfo.size());
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                }
                                else
                                {
                                lit = new LiteralEffect(idCounter++,parser_api->domain->metainfo.size());
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                (parser_api->domain->ldictionary).insert(make_pair(lit->getName(), lit->getId()));
                                }
                                container = lit;
                                delete nameLit;
                                contador = 0;
                                }
                                term_list RIGHTPAR
                                {
                                LiteralEffect * lit= (LiteralEffect *) container;
                                container = 0;
                                // comprobaciones de correctitud
                                // busco en el dominio los literales con el nombre capturado
                                  // y el n�mero de argumentos adecuado
                                  // Esto sirve para poner los tipos seg�n est�n definidos en la tabla de literales.
                                int id = SearchDictionary(&(parser_api->domain->ldictionary),lit->getName())->second;
                                  LiteralTableRange r = parser_api->domain->getLiteralRange(id);
                                bool unificacion=false;
                                vector<Literal *> candidates;
                                vector<Literal *>::const_iterator j;
                                  for(literaltablecit i = r.first; i != r.second && !unificacion; i++)
                                  {
                                candidates.push_back((*i).second);
                                     Unifier u;
                                     if(unify3(lit->getParameters(),(*i).second->getParameters(),&u)){
                                        u.applyTypeSubstitutions(0);
                                        unificacion=true;
                                     }
                                  }
                                if(!unificacion){
                                        snprintf(parerr,256,"(2) No matching predicate for `%s'.",lit->toString());
                                        yyerror(parerr);
                                        if(candidates.size() > 0) {
                                        *errflow << "Possible candidates:" << endl;
                                          for(j=candidates.begin();j!=candidates.end();j++) {
                                        SearchLineInfo sli((*j)->getMetaId());
                                             *errflow << "\t[" << sli.fileName << "]:" << sli.lineNumber;
                                        (*j)->printL(errflow,1);
                                        *errflow << endl;
                                          }
                                        }
                                }
                                $$ = lit;
                                }
                                ;

atomic_formula_term_goal:       LEFTPAR term_name
                                {
                                string * nameLit = (string *) $2;
                                LiteralGoal * lit=0;
                                Meta * mt = 0;
                                // buscamos si el literal ya est� definido en el diccionario de
                                // nombres de literales (deber�a estarlo)
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),nameLit->c_str());
                                if(posit != (parser_api->domain->ldictionary).end()) {
                                lit = new LiteralGoal(posit->second,parser_api->domain->metainfo.size());
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                }
                                else
                                {
                                lit = new LiteralGoal(idCounter++,parser_api->domain->metainfo.size());
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                (parser_api->domain->ldictionary).insert(make_pair(lit->getName(), lit->getId()));
                                }
                                container = lit;
                                delete nameLit;
                                contador = 0;
                                }
                                term_list RIGHTPAR
                                {
                                LiteralGoal * lit= (LiteralGoal *) container;
                                container = 0;
                                // comprobaciones de correctitud
                                // busco en el dominio los literales con el nombre capturado
                                  // y el n�mero de argumentos adecuado
                                  // Esto sirve para poner los tipos seg�n est�n definidos en la tabla de literales.
                                int id = SearchDictionary(&(parser_api->domain->ldictionary),lit->getName())->second;
                                  LiteralTableRange r = parser_api->domain->getLiteralRange(id);
                                bool unificacion=false;
                                vector<Literal *> candidates;
                                vector<Literal *>::const_iterator j;
                                  for(literaltablecit i = r.first; i != r.second && !unificacion; i++)
                                  {
                                candidates.push_back((*i).second);
                                     Unifier u;
                                     if(unify3(lit->getParameters(),(*i).second->getParameters(),&u)){
                                        u.applyTypeSubstitutions(0);
                                        unificacion=true;
                                     }
                                  }
                                if(!unificacion){
                                        snprintf(parerr,256,"(3) No matching predicate for `%s'.",lit->toString());
                                        yyerror(parerr);
                                        if(candidates.size() > 0) {
                                        *errflow << "Possible candidates:" << endl;
                                          for(j=candidates.begin();j!=candidates.end();j++) {
                                        SearchLineInfo sli((*j)->getMetaId());
                                             *errflow << "\t[" << sli.fileName << "]:" << sli.lineNumber;
                                        (*j)->printL(errflow,1);
                                        *errflow << endl;
                                          }
                                        }
                                }
                                $$ = lit;
                                }
                                ;


term_list:                      /* empty */
                                | term_list term
                                ;

term:                           term_name
                                {
                                string * c = (string *) $1;
                                if(!container)
                                *errflow << "(mensaje recordatorio) (-- Aqui deber�a haber un container --)" << endl;
                                if(container){
                                // la constante deber�a haberse definido con anterioridad, en otro caso
                                // se trata de un error
                                ldictionaryit posit = (parser_api->domain->cdictionary).find(c->c_str());
                                if(posit != (parser_api->domain->ldictionary).end()) {
                                        // devolver el pkey de la constante
                                        container->addParameter(make_pair((*posit).second, 0));
                                        contador = container->getModificableParameters()->size()+1;
                                }
                                else {
                                     snprintf(parerr,256,"Undefined constant `%s'.",c->c_str());
                                     yyerror(parerr);
                                     ConstantSymbol * n = new ConstantSymbol(c->c_str(),-1);
                                     n->setLineNumber(lexer->getLineNumber());
                                     n->setFileId(parser_api->fileid);
                                     container->addParameter(parser_api->termtable->addConstant(n));
                                     contador = container->getModificableParameters()->size()+1;
                                }
                                }
                                delete c;
                                }
                                | variable
                                {
                                if(!container)
                                *errflow << "(mensaje recordatorio) (-- Aqui deber�a haber un container --)" << endl;
                                if(container){
                                container->addParameter(*$1);
                                if(container->getModificableParameters()->empty())
                                        contador = 0;
                                }
                                }
                                | variable PDDL_HYPHEN type
                                {
                                if(!container)
                                *errflow << "(mensaje recordatorio) (-- Aqui deber�a haber un container --)" << endl;
                                if(container && $3){
                                // recorremos hacia atr�s todas las variables insertadas anteriormente,
                                // hasta encontrar la primera que no tiene tipo asignado.
                                // A partir de este asignamos type
                                if(container->getModificableParameters()->empty())
                                        contador = 0;
                                container->addParameter(*$1);
                                vector<Type *> * vt = (vector<Type *> *)$3;
                                if(!vt->empty()) {
                                TestTypeTree()(vt);
                                KeyList * kl = container->getModificableParameters();
                                KeyList::iterator i,e;
                                e = kl->end();
                                for(i=kl->begin() + contador;i!=e;i++)
                                {
                                        if(parser_api->termtable->isVariable(*i)){
                                        if(!parser_api->termtable->getVariable(*i)->specializeTypes(vt)){
                                                snprintf(parerr,256,"Unable to specialize the type(s) for variable `%s'.",parser_api->termtable->getVariable(*i)->getName());
                                                yyerror(parerr);
                                        }
                                        }
                                }
                                }
                                delete vt;
                                contador = container->getModificableParameters()->size();
                                }
                                else if(!$3) {
                                        snprintf(parerr,256,"Type expected after `-'.");
                                        yywarning(parerr);
                                }
                                }
                                | number
                                {
                                if(!container)
                                *errflow << "(mensaje recordatorio) (-- Aqui deber�a haber un container --)" << endl;
                                if(container){
                                container->addParameter(make_pair(-1,$1));
                                contador = container->getModificableParameters()->size() +1;
                                }
                                }
                                ;

number:                                PDDL_NUMBER
                                {$$=$1;}
                                | PDDL_DNUMBER
                                {$$=$1;}

var:                                PDDL_VAR
                                {
                                $$ = $1;
                                }
                                | PDDL_DURATIONVAR
                                {
                                $$ =$1;
                                }
                                | STARTVAR
                                {
                                $$ = $1;
                                }
                                | ENDVAR
                                {
                                $$ = $1;
                                }
                                ;

variable:                       var
                                {
                                  // miramos si la variable se define en un contexto en concreto
                                  // para dar el mismo identificador, por ejemplo en el literal
                                  // (l ?x ?y ?x) en el contexto del literal las dos variables ?x
                                  // que aparecen tienen el mismo identificador. Esto sirve para
                                  // que cuando cambie el valor de una de las ?x se cambien todas.
                                  static pkey id;
                                  id.first=-1;
                                  id.second=0;

                                  if(context){
                                      ldictionaryit posit = context->find($1);

                                      if(posit==context->end()){
                                          // no se encontr� ninguna variable igual en el contexto
                                          // se crea una nueva
                                          VariableSymbol * v = new VariableSymbol(-1,parser_api->domain->metainfo.size());
                                          Meta * mt = new Meta($1,lexer->getLineNumber(),parser_api->fileid);
                                          id = parser_api->termtable->addVariable(v);
                                          parser_api->domain->metainfo.push_back(mt);
                                          context->insert(make_pair(parser_api->termtable->getVariable(id)->getName(),id.first));
                                      }
                                      else {
                                          id.first=(*posit).second;
                                      }
                                  }
                                  else {
                                       VariableSymbol * v = new VariableSymbol(-1,parser_api->domain->metainfo.size());
                                       Meta * mt = new Meta($1,lexer->getLineNumber(),parser_api->fileid);
                                       id = parser_api->termtable->addVariable(v);
                                       parser_api->domain->metainfo.push_back(mt);
                                  }
                                  // devolvemos la variable
                                  $$ = &id;
                                }
                                ;

f_comp:                         LEFTPAR binary_comp fluent_exp fluent_exp RIGHTPAR
                                {
                                ComparationGoal * cg = (ComparationGoal *) $2;
                                Evaluable * first, * second;
                                first = (Evaluable *) $3;
                                second = (Evaluable *) $4;
                                cg->setFirst(first);
                                cg->setSecond(second);
                                $$ = cg;
                                }
                                ;

binary_comp:                    GREATHER
                                {
                                ComparationGoal * cg =  new ComparationGoal();
                                cg->setComparator(CGREATHER);
                                $$ = cg;
                                }
                                | LESS
                                {
                                ComparationGoal * cg =  new ComparationGoal();
                                cg->setComparator(CLESS);
                                $$ = cg;
                                }
                                | EQUAL
                                {
                                ComparationGoal * cg =  new ComparationGoal();
                                cg->setComparator(CEQUAL);
                                $$ = cg;
                                }
                                | GREATHER_EQUAL
                                {
                                ComparationGoal * cg =  new ComparationGoal();
                                cg->setComparator(CGREATHER_EQUAL);
                                $$ = cg;
                                }
                                | LESS_EQUAL
                                {
                                ComparationGoal * cg =  new ComparationGoal();
                                cg->setComparator(CLESS_EQUAL);
                                $$ = cg;
                                }
                                | DISTINCT
                                {
                                ComparationGoal * cg =  new ComparationGoal();
                                cg->setComparator(CDISTINCT);
                                $$ = cg;
                                }
                                ;

fluent_exp:                      number
                                {
                                FluentNumber * f = new FluentNumber($1);
                                isNumber = true;
                                $$ = f;
                                }
                                | LEFTPAR binary_op fluent_exp fluent_exp RIGHTPAR
                                { FluentOperator * fo = (FluentOperator *) $2;
                                Evaluable * first, * second;
                                first = (Evaluable *) $3;
                                second = (Evaluable *) $4;
                                const Type * number = parser_api->domain->getType("number");
                                if(!first->isType(number)) {
                                snprintf(parerr,256,"Operand `%s' is not of number type.",first->toStringEvaluable());
                                yyerror(parerr);
                                }
                                if(!second->isType(number)) {
                                snprintf(parerr,256,"Operand `%s' is not of number type.",second->toStringEvaluable());
                                yyerror(parerr);
                                }
                                fo->setFirst(first);
                                fo->setSecond(second);
                                $$ = fo;
                                }
                                | LEFTPAR unary_op fluent_exp silly_exp RIGHTPAR
                                {
                                FluentOperator * fo = new FluentOperator((Operation)$2);
                                Evaluable * first, * second;
                                first = (Evaluable *) $3;
                                second = (Evaluable *) $4;
                                const Type * number = parser_api->domain->getType("number");
                                if(!first->isType(number)) {
                                        snprintf(parerr,256,"Operand `%s' is not of number type.",first->toStringEvaluable());
                                        yyerror(parerr);
                                }
                                if(second)
                                        if(!second->isType(number)) {
                                        snprintf(parerr,256,"Operand `%s' is not of number type.",second->toStringEvaluable());
                                        yyerror(parerr);
                                        }
                                fo->setFirst(first);
                                if(second)
                                        fo->setSecond(second);
                                if(second && $2 != OSUBSTRACT) {
                                        snprintf(parerr,256,"Using two arguments in a operator that only needs one: %s.",fo->toStringEvaluable());
                                        yyerror(parerr);
                                }
                                $$ = fo;
                                }
                                | f_head_ref
                                {
                                $$ = (FluentLiteral *) $1;
                                }
                                | variable
                                {
                                FluentVar * v = new FluentVar($1);
                                // comprobar que la variable sea de tipo number
                                if(!v->isType(parser_api->domain->getType("number"))){
                                        // Dar warning si es objeto.
                                        if(v->isObjectType()) {
                                        parser_api->termtable->getVariable(v->getId())->specializeTypes(parser_api->domain->getModificableType("number"));
                                        snprintf(parerr,256,"Seting type of `%s' to number.",parser_api->termtable->getVariable(v->getId())->getName());
                                        yywarning(parerr);
                                        }
                                        else
                                        parser_api->termtable->getVariable(v->getId())->specializeTypes(parser_api->domain->getModificableType("number"));
                                }
                                $$ = v;
                                }
                                | term_name
                                {
                                string * c = (string *) $1;
                                FluentConstant * f;
                                ldictionaryit posit = (parser_api->domain->cdictionary).find(c->c_str());
                                if(posit != (parser_api->domain->ldictionary).end()) {
                                        // devolver el pkey de la constante
                                        f = new FluentConstant(make_pair((*posit).second, 0));
                                }
                                else {
                                    snprintf(parerr,256,"Undefined constant `%s'.",c->c_str());
                                    yyerror(parerr);
                                    ConstantSymbol * n = new ConstantSymbol(c->c_str(),-1);
                                    n->setLineNumber(lexer->getLineNumber());
                                    n->setFileId(parser_api->fileid);
                                    f = new FluentConstant(parser_api->termtable->addConstant(n));
                                }
                                delete c;
                                $$ = f;
                                }
                                ;

silly_exp:                        /*empty*/
                                {$$=0;}
                                | fluent_exp
                                {$$=$1;}
                                ;

unary_op:                        PDDL_HYPHEN
                                {$$=(int)OSUBSTRACT;}
                                | ABS
                                {$$=(int)UABS;}
                                | SQRT
                                {$$=(int)USQRT;}
                                ;

binary_op:                      PLUS
                                {
                                FluentOperator * fo = new FluentOperator(OADD);
                                $$ = fo;
                                }
                                | MULTIPLY
                                {
                                FluentOperator * fo = new FluentOperator(OTIMES);
                                $$ = fo;
                                }
                                | DIVIDE
                                {
                                FluentOperator * fo = new FluentOperator(ODIVIDE);
                                $$ = fo;
                                }
                                | POW
                                {
                                FluentOperator * fo = new FluentOperator(OPOW);
                                $$ = fo;
                                }
                                /*| PDDL_HYPHEN
                                {
                                FluentOperator * fo = new FluentOperator(OSUBSTRACT);
                                $$ = fo;
                                }*/
                                ;


f_head:                                /*term_name
                                {
                                string * nameLit = (string *) $1;
                                Function * lit=0;
                                Meta * mt = 0;
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),(const char *) nameLit->c_str());
                                if(posit != (parser_api->domain->ldictionary).end()) {
                                        lit = new Function(posit->second,parser_api->domain->metainfo.size(),false,0);
                                        mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                }
                                else
                                {
                                lit = new Function(idCounter++,parser_api->domain->metainfo.size(),false,0);
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                (parser_api->domain->ldictionary).insert(make_pair(lit->getName(), lit->getId()));
                                }
                                delete nameLit;
                                // comprobaciones de correctitud
                                // busco en el dominio los literales con el nombre capturado
                                  // y el n�mero de argumentos adecuado
                                  // Esto sirve para poner los tipos seg�n est�n definidos en la tabla de literales.
                                int id = SearchDictionary(&(parser_api->domain->ldictionary),lit->getName())->second;
                                  LiteralTableRange r = parser_api->domain->getLiteralRange(id);
                                bool unificacion=false;
                                vector<Literal *> candidates;
                                vector<Literal *>::const_iterator j;
                                  for(literaltablecit i = r.first; i != r.second && !unificacion; i++)
                                  {
                                candidates.push_back((*i).second);
                                     Unifier u;
                                     if(unify3(lit->getParameters(),(*i).second->getParameters(),&u)){
                                        u.applyTypeSubstitutions(0);
                                        if(((Function *)(*i).second)->isPython()){
                                        lit->setPython();
                                        lit->setCode(((Function *)(*i).second)->getCode());
                                        }
                                        unificacion=true;
                                     }
                                  }
                                if(!unificacion){
                                        snprintf(parerr,256,"(4) No matching predicate for `%s'.",lit->toString());
                                        yyerror(parerr);
                                        if(candidates.size() > 0) {
                                        *errflow << "Possible candidates:" << endl;
                                          for(j=candidates.begin();j!=candidates.end();j++) {
                                        SearchLineInfo sli((*j)->getMetaId());
                                             *errflow << "\t[" << sli.fileName << "]:" << sli.lineNumber;
                                        (*j)->printL(errflow,1);
                                        *errflow << endl;
                                          }
                                        }
                                }
                                $$ = lit;
                                }
                                | LEFTPAR term_name */
                                LEFTPAR term_name
                                {
                                string * nameLit = (string *) $2;
                                Function * lit=0;
                                Meta * mt = 0;
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),(const char *) nameLit->c_str());
                                if(posit != (parser_api->domain->ldictionary).end()) {
                                        lit = new Function(posit->second,parser_api->domain->metainfo.size(),false,0);
                                        mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                }
                                else
                                {
                                lit = new Function(idCounter++,parser_api->domain->metainfo.size(),false,0);
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                (parser_api->domain->ldictionary).insert(make_pair(lit->getName(), lit->getId()));
                                }
                                container = lit;
                                delete nameLit;
                                contador=0;
                                }
                                term_list RIGHTPAR
                                {
                                Function * lit= (Function *) container;
                                container = 0;
                                // comprobaciones de correctitud
                                // busco en el dominio los literales con el nombre capturado
                                  // y el n�mero de argumentos adecuado
                                  // Esto sirve para poner los tipos seg�n est�n definidos en la tabla de literales.
                                int id = SearchDictionary(&(parser_api->domain->ldictionary),lit->getName())->second;
                                  LiteralTableRange r = parser_api->domain->getLiteralRange(id);
                                bool unificacion=false;
                                vector<Literal *> candidates;
                                vector<Literal *>::const_iterator j;
                                  for(literaltablecit i = r.first; i != r.second && !unificacion; i++)
                                  {
                                candidates.push_back((*i).second);
                                     Unifier u;
                                     if(unify3(lit->getParameters(),(*i).second->getParameters(),&u)){
                                        u.applyTypeSubstitutions(0);
                                        //if(((Function *)(*i).second)->isPython()){
                                        //    lit->setPython();
                                        //    lit->setCode(((Function *)(*i).second)->getCode());
                                        //}
                                        unificacion=true;
                                     }
                                  }
                                if(!unificacion){
                                        snprintf(parerr,256,"(5) No matching predicate for `%s'.",lit->toString());
                                        yyerror(parerr);
                                        if(candidates.size() > 0) {
                                        *errflow << "Possible candidates:" << endl;
                                          for(j=candidates.begin();j!=candidates.end();j++) {
                                        SearchLineInfo sli((*j)->getMetaId());
                                             *errflow << "\t[" << sli.fileName << "]:" << sli.lineNumber;
                                        (*j)->printL(errflow,1);
                                        *errflow << endl;
                                          }
                                        }
                                }
                                $$ = lit;
                                }
                                ;

f_head_ref:                        /*LEFTPAR term_name RIGHTPAR
                                {
                                string * nameLit = (string *) $3;
                                FluentLiteral * lit=0;
                                Meta * mt = 0;
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),(const char *) nameLit->c_str());
                                if(posit != (parser_api->domain->ldictionary).end()) {
                                        lit = new FluentLiteral(posit->second,parser_api->domain->metainfo.size());
                                        mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                }
                                else
                                {
                                lit = new FluentLiteral(idCounter++,parser_api->domain->metainfo.size());
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                (parser_api->domain->ldictionary).insert(make_pair(lit->getName(), lit->getId()));
                                }
                                delete nameLit;
                                // comprobaciones de correctitud
                                // busco en el dominio los literales con el nombre capturado
                                  // y el n�mero de argumentos adecuado
                                  // Esto sirve para poner los tipos seg�n est�n definidos en la tabla de literales.
                                int id = SearchDictionary(&(parser_api->domain->ldictionary),lit->getName())->second;
                                  LiteralTableRange r = parser_api->domain->getLiteralRange(id);
                                bool unificacion=false;
                                vector<Literal *> candidates;
                                vector<Literal *>::const_iterator j;
                                  for(literaltablecit i = r.first; i != r.second && !unificacion; i++)
                                  {
                                candidates.push_back((*i).second);
                                     Unifier u;
                                     if(unify3(lit->getParameters(),(*i).second->getParameters(),&u)){
                                        u.applyTypeSubstitutions(0);
                                        unificacion=true;
                                     }
                                  }
                                if(!unificacion){
                                        snprintf(parerr,256,"(6) No matching predicate for `%s'.",lit->toString());
                                        yyerror(parerr);
                                        if(candidates.size() > 0) {
                                        *errflow << "Possible candidates:" << endl;
                                          for(j=candidates.begin();j!=candidates.end();j++) {
                                        SearchLineInfo sli((*j)->getMetaId());
                                             *errflow << "\t[" << sli.fileName << "]:" << sli.lineNumber;
                                        (*j)->printL(errflow,1);
                                        *errflow << endl;
                                          }
                                        }
                                }
                                $$ = lit;
                                }*/
                                //| LEFTPAR term_name
                                LEFTPAR term_name
                                {
                                string * nameLit = (string *) $2;
                                FluentLiteral * lit=0;
                                Meta * mt = 0;
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),(const char *) nameLit->c_str());
                                if(posit != (parser_api->domain->ldictionary).end()) {
                                        lit = new FluentLiteral(posit->second,parser_api->domain->metainfo.size());
                                        mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                }
                                else
                                {
                                lit = new FluentLiteral(idCounter++,parser_api->domain->metainfo.size());
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                (parser_api->domain->ldictionary).insert(make_pair(lit->getName(), lit->getId()));
                                }
                                container = lit;
                                delete nameLit;
                                contador = 0;
                                }
                                term_list RIGHTPAR
                                {
                                FluentLiteral * lit= (FluentLiteral *) container;
                                container = 0;
                                // comprobaciones de correctitud
                                // busco en el dominio los literales con el nombre capturado
                                // y el n�mero de argumentos adecuado
                                // Esto sirve para poner los tipos seg�n est�n definidos en la tabla de literales.
                                int id = SearchDictionary(&(parser_api->domain->ldictionary),lit->getName())->second;
                                  LiteralTableRange r = parser_api->domain->getLiteralRange(id);
                                bool unificacion=false;
                                vector<Literal *> candidates;
                                vector<Literal *>::const_iterator j;
                                  for(literaltablecit i = r.first; i != r.second && !unificacion; i++)
                                  {
                                candidates.push_back((*i).second);
                                     Unifier u;
                                     if(unify3(lit->getParameters(),(*i).second->getParameters(),&u)){
                                        u.applyTypeSubstitutions(0);
                                        unificacion=true;
                                     }
                                  }
                                if(!unificacion){
                                        snprintf(parerr,256,"(7) No matching predicate for `%s'.",lit->toString());
                                        yyerror(parerr);
                                        if(candidates.size() > 0) {
                                        *errflow << "Possible candidates:" << endl;
                                          for(j=candidates.begin();j!=candidates.end();j++) {
                                        SearchLineInfo sli((*j)->getMetaId());
                                             *errflow << "\t[" << sli.fileName << "]:" << sli.lineNumber;
                                        (*j)->printL(errflow,1);
                                        *errflow << endl;
                                          }
                                        }
                                }
                                $$ = lit;
                                }
                                ;


effect_def:                     /* empty */
                                {$$ = 0;}
                                | PDDL_EFFECT  effect
                                {$$ =$2;}
                                ;

effect:                         LEFTPAR RIGHTPAR
                                {$$=0;}
                                | LEFTPAR PDDL_AND
                                {
                                AndEffect * ae = new AndEffect();
                                econtainer.push_back(ae);
                                }
                                c_effect_list RIGHTPAR
                                {
                                AndEffect * ae = (AndEffect *) econtainer.back();
                                econtainer.pop_back();
                                $$ = ae;
                                }
                                | c_effect
                                { $$= $1;}
                                ;

timed_effect:                        LEFTPAR time_point p_effect RIGHTPAR
                                {
                                Effect * e = (Effect *) $3;
                                Evaluable * ti = (Evaluable *) $2;
                                if(e->isLiteralEffect()){
                                        LiteralEffect * le = ((LiteralEffect *)e);
                                        le->setTime(ti);
                                }
                                else{
                                        ((FluentEffect *)e)->setTime(ti);
                                }
                                    if(!parser_api->domain->errdurative && !parser_api->domain->hasRequirement(":durative-actions"))
                                    {
                                        parser_api->domain->errdurative = true;
                                        yyerror("Using a clause that requires `:durative-actions' and is not declared in requirements clause");
                                    }
                                    if(!isDurative)
                                    {
                                        yyerror("Using a durative effect inside a non durative action.");
                                    }
                                $$ = e;
                                };


c_effect:                       LEFTPAR
                                    PDDL_FORALL
                                {
                                // creamos el forall, y lo ponemos como
                                // el elemento que va a contener a las variables
                                ForallEffect * fae = new ForallEffect();
                                container = fae;
                                $<otype>$ = fae;
                                }
                                LEFTPAR variable_typed_list RIGHTPAR
                                {
                                container = 0;
                                }
                                    effect
                                RIGHTPAR
                                {
                                ForallEffect * fae = (ForallEffect *) $<otype>3;
                                Effect * e = (Effect *) $8;
                                fae->setEffect(e);
                                    if(!parser_api->domain->errconditionals && !parser_api->domain->hasRequirement(":conditional-effects"))
                                    {
                                        parser_api->domain->errconditionals = true;
                                        yyerror("Using a clause that requires `:conditional-effects' and is not declared in requirements clause");
                                    }
                                $$ = fae;
                                }
                                | LEFTPAR PDDL_WHEN goal_def cond_effect RIGHTPAR
                                {
                                WhenEffect * we = new WhenEffect((Goal *)$3,(Effect *)$4);
                                    if(!parser_api->domain->errconditionals && !parser_api->domain->hasRequirement(":conditional-effects"))
                                    {
                                        parser_api->domain->errconditionals = true;
                                        yyerror("Using a clause that requires `:conditional-effects' and is not declared in requirements clause");
                                    }
                                $$ = we;
                                }
                                | p_effect
                                {$$ = $1;}
                                | timed_effect
                                { $$= $1;}
                                ;

c_effect_list:                  /* empty */
                                | c_effect_list c_effect
                                {
                                if($2){
                                ContainerEffect * lc = (ContainerEffect *) econtainer.back();
                                lc->addEffectByRef((Effect *) $2);
                                }
                                }
                                ;

p_effect:                       LEFTPAR assign_op f_head_ref fluent_exp RIGHTPAR
                                {
                                FluentEffect * fe = new FluentEffect((FOperation)$2,(FluentLiteral *)$3,(Evaluable *)$4);
                                    if(!parser_api->domain->errfluents && !parser_api->domain->hasRequirement(":fluents"))
                                    {
                                        parser_api->domain->errfluents = true;
                                        yyerror("Using a clause that requires `:fluents' and is not declared in requirements clause");
                                    }
                                $$ = fe;
                                }
                                | LEFTPAR PDDL_NOT atomic_formula_term_effect RIGHTPAR
                                {
                                LiteralEffect * le = (LiteralEffect *) $3;
                                if(le->getPolarity())
                                        le->setPolarity(false);
                                $$=le;
                                }
                                | atomic_formula_term_effect
                                {$$=$1;}
                                ;

p_effect_list:                  /* empty */
                                | p_effect_list p_effect
                                {
                                if($2){
                                ContainerEffect * lc = (ContainerEffect *) econtainer.back();
                                lc->addEffectByRef((Effect *) $2);
                                }
                                }
                                ;

cond_effect:                    LEFTPAR PDDL_AND
                                {
                                AndEffect * ae = new AndEffect();
                                econtainer.push_back(ae);
                                }
                                p_effect_list RIGHTPAR
                                {
                                AndEffect * ae = (AndEffect *) econtainer.back();
                                econtainer.pop_back();
                                $$ = ae;
                                }
                                | p_effect
                                {$$ = $1;}
                                ;

assign_op:                      PDDL_ASSIGN
                                {
                                $$ = (int) FASSIGN;
                                }
                                | PDDL_SCALE_UP
                                {
                                $$ = (int) FSCALEUP;
                                }
                                | PDDL_SCALE_DOWN
                                {
                                $$ = (int) FSCALEDOWN;
                                }
                                | PDDL_INCREASE
                                {
                                $$ = (int) FINCREASE;
                                }
                                | PDDL_DECREASE
                                {
                                $$ = (int) FDECREASE;
                                }
                                ;

problemBody:                    require_def problemBody21
                                | problemBody21
                                ;

problemBody21:                  customization_def problemBody22
                                | problemBody22
                                ;

problemBody22:                        python_init problemBody2
                                | problemBody2
                                ;

problemBody2:                   object_declaration problemBody3
                                | problemBody3
                                ;

problemBody3:                   init {context = new LDictionary;} goal {delete context; context=0;}
                                ;

//problemBody3:                   init {context = new LDictionary;} goal {delete context; context=0;} problemBody4
//                                ;
//
//problemBody4:                   metric_spec problemBody5
//                                | problemBody5
//                                ;
//
//problemBody5:                   /*empty */
//                                | length_spec
//                                ;
//
object_declaration:             LEFTPAR PDDL_OBJECT constant_list RIGHTPAR
                                ;

init:                           LEFTPAR PDDL_INIT
                                init_el_list RIGHTPAR
                                ;

init_el:                        literal_name
                                {
                                parser_api->problem->addToInitialState((LiteralEffect *)$1);
                                //parser_api->problem->getInitialContext()->stateChanges.push_back(new UndoARLiteralState((Literal *)$1,true));
                                }
                                | LEFTPAR EQUAL f_head number RIGHTPAR
                                {
                                Function * f = (Function *) $3;
                                f->setValue($4);
                                parser_api->problem->addToInitialState(f);
                                //parser_api->problem->getInitialContext()->stateChanges.push_back(new UndoARLiteralState(f,true));
                                if(!parser_api->domain->errfluents && !parser_api->domain->hasRequirement(":fluents"))
                                    {
                                        parser_api->domain->errfluents = true;
                                        yyerror("Using a clause that requires `:fluents' and is not declared in requirements clause");
                                    }
                                }
                                | LEFTPAR {isNumber = false;} number_time_point literal_name RIGHTPAR
                                {
                                /* Comprobar que la expresi�n dada por el time specifier
                                * es un n�mero */
                                Evaluable * n = (Evaluable *) $3;
                                LiteralEffect * l = (LiteralEffect *) $4;
                                if(!isNumber){
                                        snprintf(parerr,256,"Expecting a number in time initialization: `%s'",l->getName());
                                        yyerror(parerr);
                                }
                                else {
                                        l->setTime(n);
                                }
                                    /*requires timed-initial-literals */
                                if(!parser_api->domain->errdurative && !parser_api->domain->hasRequirement(":durative-actions"))
                                {
                                        parser_api->domain->errdurative = true;
                                        yyerror("Using a clause that requires `:timed-initial-literals' and is not declared in requirements clause");
                                }
                                parser_api->problem->addToInitialState(l);
                                //parser_api->problem->getInitialContext()->stateChanges.push_back(new UndoARLiteralState(l,true));
                                }
                                | LEFTPAR PDDL_BETWEEN number
                                PDDL_AND number
                                optional_repetition
                                literal_name
                                RIGHTPAR
                                {
                                LiteralEffect * l = (LiteralEffect *) $7;
                                TimeLineLiteralEffect * tl = new TimeLineLiteralEffect(l->getId(),l->getMetaId(),l->getParameters(),l->getPolarity());
                                delete l;
                                if(!parser_api->domain->errdurative && !parser_api->domain->hasRequirement(":durative-actions"))
                                {
                                        parser_api->domain->errdurative = true;
                                        yyerror("Using a clause that requires `:timed-initial-literals' and is not declared in requirements clause");
                                }
                                tl->setInterval((int)$3,(int)$5);
                                tl->setGap((int)$6);
                                parser_api->problem->addToInitialState(tl);
                                //parser_api->problem->getInitialContext()->stateChanges.push_back(new UndoARLiteralState(tl,true));
                                $$ = 0;
                                }
                                ;

optional_repetition:                PDDL_AND_EVERY number
                                {
                                if($2 < 0){
                                        yyerror("Expecting a positive number.");
                                        $$ = 0;
                                }
                                else
                                        $$ = $2;
                                }
                                | /*empty */
                                {
                                $$ = 0;
                                }
                                ;

init_el_list:                   /*empty*/
                                | init_el_list init_el
                                ;

goal:                           LEFTPAR PDDL_GOAL goal_def RIGHTPAR
                                { *errflow << "Los goal 'planos' de pddl no est�n soportados." << endl;}
                                | LEFTPAR
                                    HTN_TASKSGOAL
                                    meta_list
                                    HTN_TASKS task_network
                                RIGHTPAR
                                {
                                parser_api->problem->setInitialGoal((TaskNetwork *)$5);
                                   if(!parser_api->domain->errhtn && !parser_api->domain->hasRequirement(":htn-expansion"))
                                   {
                                        parser_api->domain->errhtn = true;
                                        yyerror("Using a clause that requires `:htn-expansion' that is not declared in requirements clause");
                                   }
                                if($3){
                                TagVector * tv = (TagVector *) $3;
                                tagv_ite tb, te = tv->end();
                                for(tb = tv->begin();tb!=te;tb++)
                                        parser_api->problem->meta.addTag((*tb));
                                tv->clear();
                                delete tv;
                                }

                                // verificar que todos los objetivos expuestos en
                                // la red de tareas son alcanzables.
                                bool changes=true;
                                while(changes){
                                changes = false;
                                if(!((TaskNetwork *) $5)->isWellDefined(errflow,&changes))
                                {
                                        snprintf(parerr,256,"In the task network of goal specification.");
                                        yyerror(parerr);
                                }
                                }

                                }
                                ;

literal_name:                   atomic_formula_name
                                {$$ = (LiteralEffect *) $1;}
                                | LEFTPAR PDDL_NOT atomic_formula_name RIGHTPAR
                                {
                                LiteralEffect * lit = (LiteralEffect *) $3;
                                lit->setPolarity(false);
                                $$ = lit;
                                }
                                ;

atomic_formula_name:            LEFTPAR term_name
                                {
                                string * nameLit = (string *) $2;
                                LiteralEffect * lit=0;
                                Meta * mt = 0;
                                // buscamos si el literal ya est� definido en el diccionario de
                                // nombres de literales (deber�a estarlo)
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),nameLit->c_str());
                                if(posit != (parser_api->domain->ldictionary).end()) {
                                lit = new LiteralEffect(posit->second,parser_api->domain->metainfo.size());
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                }
                                else
                                {
                                lit = new LiteralEffect(idCounter++,parser_api->domain->metainfo.size());
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                (parser_api->domain->ldictionary).insert(make_pair(lit->getName(), lit->getId()));
                                }
                                container = lit;
                                delete nameLit;
                                contador = 0;
                                }
                                term_list RIGHTPAR
                                {
                                LiteralEffect * lit= (LiteralEffect *) container;
                                container = 0;
                                // comprobaciones de correctitud
                                // busco en el dominio los literales con el nombre capturado
                                  // y el n�mero de argumentos adecuado
                                  // Esto sirve para poner los tipos seg�n est�n definidos en la tabla de literales.
                                int id = SearchDictionary(&(parser_api->domain->ldictionary),lit->getName())->second;
                                  LiteralTableRange r = parser_api->domain->getLiteralRange(id);
                                bool unificacion=false;
                                vector<Literal *> candidates;
                                vector<Literal *>::const_iterator j;
                                  for(literaltablecit i = r.first; i != r.second && !unificacion; i++)
                                  {
                                candidates.push_back((*i).second);
                                     if(unify(lit->getParameters(),(*i).second->getParameters())){
                                        unificacion=true;
                                     }
                                  }
                                if(!unificacion){
                                        snprintf(parerr,256,"(8) No matching predicate for `%s'.",lit->toString());
                                        yyerror(parerr);
                                        if(candidates.size() > 0) {
                                        *errflow << "Possible candidates:" << endl;
                                          for(j=candidates.begin();j!=candidates.end();j++) {
                                        SearchLineInfo sli((*j)->getMetaId());
                                             *errflow << "\t[" << sli.fileName << "]:" << sli.lineNumber;
                                        (*j)->printL(errflow,1);
                                        *errflow << endl;
                                          }
                                        }
                                }
                                $$ = lit;
                                }
                                ;

durative_action_def:            LEFTPAR
                                    PDDL_DURATIVE_ACTION term_name
                                {
                                context = new LDictionary;
                                string * name = (string *) $3;
                                PrimitiveTask * priTask=0;
                                Meta * mt = 0;
                                isDurative = true;
                                // buscamos en el diccionario si la acci�n ya tiene un identificador
                                // asociado, en cuyo caso lo reutilizamos
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),name->c_str());
                                      if(posit != (parser_api->domain->ldictionary).end()) {
                                        priTask = new PrimitiveTask(posit->second,parser_api->domain->metainfo.size());
                                        mt = new Meta(name->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                }
                                      else
                                      {
                                        priTask = new PrimitiveTask(idCounter++,parser_api->domain->metainfo.size());
                                        mt = new Meta(name->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                        parser_api->domain->metainfo.push_back(mt);
                                          (parser_api->domain->ldictionary).insert(make_pair(priTask->getName(),priTask->getId()));
                                      }
                                container = priTask;
                                delete name;
                                $<otype>$ = priTask;
                                }
                                PDDL_PARAMETERS LEFTPAR variable_typed_list RIGHTPAR
                                meta_list
                                {
                                    container = 0;
                                    // si hay alg�n tag
                                    if($9){
                                        TagVector * tv = (TagVector *) $9;
                                        tagv_ite tb, te = tv->end();
                                        int mid = ((PrimitiveTask *) $<otype>4)->getMetaId();
                                        for(tb = tv->begin();tb!=te;tb++){
                                                parser_api->domain->metainfo[mid]->addTag(*tb);
                                        }
                                        tv->clear();
                                        delete tv;
                                    }
                                }
                                PDDL_DURATION pduration_constraints
                                {
                                 if($12){
                                     ((PrimitiveTask *) $<otype>4)->setTConstraints((vector<TCTR> *) $12);
                                 }
                                }
                                PDDL_CONDITION goal_def
                                PDDL_EFFECT effect
                                RIGHTPAR
                                {
                                 isDurative = false;
                                 PrimitiveTask * priTask= (PrimitiveTask *) $<otype>4;
                                 priTask->setPrecondition((Goal *) $15);
                                 priTask->setEffect((Effect *) $17);
                                 parser_api->domain->addTask(priTask);
                                 delete context;
                                 context = 0;
                                 if(!parser_api->domain->errdurative && !parser_api->domain->hasRequirement(":durative-actions")){
                                     parser_api->domain->errdurative = true;
                                     yyerror("Using a clause that requires `:durative-actions' and is not declared in requirements clause");
                                 }
                                };
sdur_constraint_list:           sdur_constraint
                                {
                                vector<pair<int,Evaluable *> > * v = new vector<pair<int,Evaluable *> >;
                                v->push_back(*((pair<int,Evaluable *> *) $1));
                                $$= v;
                                }
                                | sdur_constraint_list sdur_constraint
                                {
                                vector<pair<int,Evaluable *> > * v = (vector<pair<int,Evaluable *> > *) $1;
                                v->push_back(*((pair<int,Evaluable *> *) $2));
                                $$= v;
                                }
                                ;

pduration_constraints:          sdur_constraint
                                {
                                 vector<TCTR> * v = new vector<TCTR>;
                                 TCTR * ele = (TCTR *) $1;
                                 v->push_back(*ele);
                                 $$= v;
                                }
                                | LEFTPAR PDDL_AND sdur_constraint_list RIGHTPAR
                                {
                                 vector<TCTR> * v = (vector<TCTR> *) $3;
                                 $$= v;
                                }
                                | LEFTPAR RIGHTPAR
                                {
                                 vector<TCTR> * v = new vector<TCTR>;
                                 $$= v;
                                };

time_specifier:                 time_point
                                {
                                TimeInterval * ti = new TimeInterval();
                                ti->setStart((Evaluable *)$1);
                                $$ = ti;
                                }
                                | PDDL_OVERALL
                                {
                                TimeInterval * ti = new TimeInterval();
                                ti->setStart((double)ATSTART);
                                ti->setEnd((double)ATEND);
                                $$ = ti;
                                }
                                | PDDL_BETWEEN time_point PDDL_AND time_point
                                {
                                TimeInterval * ti = new TimeInterval();
                                ti->setStart((Evaluable *)$2);
                                ti->setEnd((Evaluable *)$4);
                                $$ = ti;
                                isNumber = false;
                                }
                                ;

time_point:                        PDDL_ATSTART
                                {
                                $$ = new FluentNumber((double)ATSTART);
                                }
                                | PDDL_ATEND
                                {
                                $$ = new FluentNumber((double)ATEND);
                                }
                                | number_time_point
                                { $$ = $1;}
                                ;

number_time_point:                {AtExpected = true;} PDDL_AT {AtExpected = false;} fluent_exp
                                {
                                Evaluable * c = (Evaluable *) $4;
                                const Type * number = parser_api->domain->getType("number");
                                if(!c->isType(number)) {
                                snprintf(parerr,256,"Expression `%s' is not numeric.",c->toStringEvaluable());
                                yyerror(parerr);
                                }
                                $$ = c;
                                }
                                ;

derived_def:                    LEFTPAR PDDL_DERIVED
                                {
                                context = new LDictionary;
                                }
                                derived_body RIGHTPAR
                                {

                                delete context;
                                context = 0;
                                }
                                ;

derived_body:                   derived_formula_skeleton goal_def
                                {

                                if($1)
                                {
                                        Axiom * a = (Axiom *) $1;
                                        if($2)
                                        a->setGoal((Goal *)$2);
                                }
                                else
                                {
                                        if($2)
                                        delete (Goal *) $2;
                                }
                                }
                                | derived_formula_skeleton code
                                {

                                if($1)
                                {
                                        Axiom * a = (Axiom *) $1;
                                        if($2){
                                        if(!a->setCode($2)){
                                                snprintf(parerr,256,"Error in Python script code. Axiom: %s.",a->getName());
                                                yyerror(parerr);
                                        }
                                        }
                                }
                                }

// ------------------------------------------------------------------------------------
// A partir de aqu� todo son reglas para el parser del debugger
// integrado
debug_sentence:                    {inDebugContext=true;} DBG_DEBUG command {inDebugContext=false;}
                        {
                                YYACCEPT;
                        }
                        | error
                        {
                                YYABORT;
                        }
                        ;

command:                    DBG_QUIT
                            {
                                exit(EXIT_SUCCESS);
                            }
                            | help
                            | DBG_CONTINUE
                            {
                                debugger->cont = true;
                            }
                            | print
                            | display
                            | undisplay
                            | plot
                            | set
                            | breakpoint
                            | describe
                        | eval
                        | apply
                            | DBG_NEXT
                            {
                                debugger->next = true;
                            }
                            | DBG_NEXP
                            {
                                debugger->nexp = true;
                            }
                            | DBG_MEM
                            {
                                struct mallinfo myinfo=mallinfo();
                                cerr << "Memory allocated (bytes): " << myinfo.hblkhd << " Memory chunks occupied: " << myinfo.uordblks << endl;
                            }
                            | DBG_SELECT PDDL_DNUMBER
                            {
                                debugger->select((int)$2);
                            }
                            | DBG_VERBOSE DBG_ON
                            {
                                current_plan->FLAG_VERBOSE = 1;
                            }
                            | DBG_VERBOSE DBG_OFF
                            {
                                current_plan->FLAG_VERBOSE = 0;
                            }
                            ;

help:                       DBG_HELP
                            {
                                cerr << "Type `help <command>' to obtain detailed information about a command." << endl;
                                cerr << "Avaliable commands:" <<endl;
                                cerr << "print          display        quit           plot"<<endl;
                                cerr << "continue       undisplay      next           set" <<endl;
                                cerr << "describe       break          disable        enable"<<endl;
                                cerr << "nexp           eval           watch          apply"<<endl;
                                cerr << "<enter> executes the last command." << endl;
                            }
                            | DBG_HELP DBG_CONTINUE
                            {
                                cerr << "Command: `continue'. Shortcut: `c'" << endl << endl;
                                cerr << "Description: Continues the execution until a breakpoint or the end of the program is reached." << endl;
                                cerr << "See also: `next'" << endl;
                            }
                            | DBG_HELP DBG_QUIT
                            {
                                cerr << "Command: `quit' or `exit'" << endl << endl;
                                cerr << "Description: Terminates the program execution." << endl;
                            }
                            | DBG_HELP DBG_PRINT
                            {
                                cerr << "Command: `print'. Shortcut: `p'" << endl << endl;
                                cerr << "Description: Prints information about the planning context." << endl;
                                cerr << "\t`print state':\tPrints the current state." << endl;
                                cerr << "\t`print agenda':\tPrints the current agenda." << endl;
                                cerr << "\t`print plan':\tPrints the ongoing plan." << endl;
                                cerr << "\t`print options':\tPrints the options you can peform on next step." << endl;
                                cerr << "\t`print termtable':\tPrints the internal term table." << endl;
                                cerr << "\t`print tasks':\tPrints all the tasks defined in the domain." << endl;
                                cerr << "\t`print predicates':\tPrints all the avaliable predicates defined in the domain." << endl;
                                cerr << "\t`print <predicate-expression>':\tPrints all the predicates in the current state, or the tasks in current plan, that match the given expression." << endl;
                            }
                            | DBG_HELP DBG_DISPLAY
                            {
                                cerr << "Command: `display'. Shortcut `d'." << endl << endl;
                                cerr << "Description: Display information about the current planning context every step until is undisplayed." << endl;
                                cerr << "\t`display':\tShows information about the current displays." << endl;
                                cerr << "\t`display <number>':\tActivates the display of given number." << endl;
                                cerr << "\t`display state':\tDisplays the current state." << endl;
                                cerr << "\t`display agenda':\tDisplays the current agenda." << endl;
                                cerr << "\t`display plan':\tDisplays the ongoing plan." << endl;
                                cerr << "\t`display termtable':\tDisplays the internal term table." << endl;
                                cerr << "\t`display <predicate-expression>':\tDisplays all the predicates in the current state, or the tasks in current plan, that match the given expression." << endl;
                                cerr << "See also: `print', `undisplay'" << endl;
                            }
                        | DBG_HELP DBG_UNDISPLAY
                            {
                                cerr << "Command: `undisplay <number>'." << endl << endl;
                                cerr << "Description: Deactivates the display of given number." << endl;
                        }
                        | DBG_HELP DBG_NEXT
                            {
                                cerr << "Command: `next'. ShortCut: `n'" << endl << endl;
                                cerr << "Description: Advance one more step." << endl;
                                cerr << "See also: `continue'" << endl;
                        }
                        | DBG_HELP DBG_NEXP
                            {
                                cerr << "Command: `nexp' (next expansion). ShortCut: `ne'" << endl << endl;
                                cerr << "Description: Advance until a new task is chosen." << endl;
                                cerr << "See also: `continue', `next'" << endl;
                        }
                        | DBG_HELP DBG_PLOT
                            {
                                cerr << "Command: `plot'." << endl << endl;
                                cerr << "Description: Graphically shows information about the current plan." << endl;
                                cerr << "You need to have installed the 'dot' program avaliable at http://www.graphviz.org." << endl;
                                cerr << "You also need to correct set up some environment variables." << endl;
                                cerr << "\t`plot plan':\tShows the current plan graph." << endl;
                                cerr << "\t`plot causal':\tShows the graph of the causal link structure in the plan." << endl;
                                cerr << "See also: `set'" << endl;
                        }
                        | DBG_HELP DBG_SET
                            {
                                cerr << "Command: `set'." << endl << endl;
                                cerr << "Description: Sets some environment variables." << endl;
                                cerr << "\t`set <variable>':\tShows the current value of the given variable." << endl;
                                cerr << "\t`set <variable> = <value>':\tSets a new value for the given variable." << endl;
                                cerr << "\tAvaliable variables:" << endl;
                                cerr << "\t\t`verbosity: Level of verbosity (0=nothing,1=shy,2=normal(default),3=promiscuous)." << endl;
                                cerr << "\t\t`viewer: Path to a program able to display png format images." << endl;
                                cerr << "\t\t`tmpdir: Directory to store temporary information." << endl;
                                cerr << "\t\t`dotpath: Path to the dot program." << endl;
                                cerr << "See also: `plot'" << endl;
                        }
                            | DBG_HELP DBG_DESCRIBE
                            {
                                cerr << "Command: `describe'." << endl << endl;
                                cerr << "Description: Shows detailed information about a structure defined in the domain." << endl;
                                cerr << "\t`describe <predicate-expression>':\tPrint the description in the domain relative to the predicate expression. It can be a predicate or a task." << endl;
                                cerr << "See also: `print', `display'" << endl;
                            }
                            | DBG_HELP DBG_ENABLE
                            {
                                cerr << "Command: `enable <number>'." << endl << endl;
                                cerr << "Description: Reactivates a previously disabled breakpoint or watch." << endl;
                                cerr << "<number> is de id of the breakpoint or watch to enable." << endl;
                                cerr << "See also: `break', `watch', `disable'." << endl;
                            }
                            | DBG_HELP DBG_DISABLE
                            {
                                cerr << "Command: `disable <number>'." << endl << endl;
                                cerr << "Description: Deactivates a breakpoint or watch." << endl;
                                cerr << "<number> is de id of the breakpoint or watch to disable." << endl;
                                cerr << "See also: `break', `watch', `enable'." << endl;
                            }
                            | DBG_HELP DBG_WATCH
                            {
                                cerr << "Command: `watch <precondition>'. Shortcut `s'" << endl << endl;
                                cerr << "Description: Defines a condition where the debugger will stop." << endl;
                                cerr << "If ithe watch is enabled, the debugger will stop every time the condition produce one or more valid unifications." << endl;
                                cerr << "See also: `break', `disable', `enable'." << endl;
                            }
                            | DBG_HELP DBG_BREAKPOINT
                            {
                                cerr << "Command: `break'. Shortcut `b'" << endl << endl;
                                cerr << "Description: Manages the stablished breakpoints." << endl;
                                cerr << "\t`break':\tLists all defined breakpoints." << endl;
                                cerr << "\t`break <number>':\tPrints breakpoint whith given id." << endl;
                                cerr << "\t`break <predicate>':\tDefines a new breakpoint. <predcate> can be a task definition or a simple predicate." << endl;
                                cerr << "See also: `watch', `disable', `enable'." << endl;
                            }
                            | DBG_HELP DBG_EVAL
                            {
                                cerr << "Command: `eval <precondition>'." << endl << endl;
                                cerr << "Description: Evaluates the given expression and prints the produced unifications." << endl;
                            }
                            | DBG_HELP DBG_APPLY
                            {
                                cerr << "Command: `apply <effect>'." << endl << endl;
                                cerr << "Description: Applies the given effect." << endl;
                                cerr << "Be cautious with this command is dangerous." << endl;
                                cerr << "See also: `eval'." << endl;
                            }
                        ;

print:                      DBG_PRINT DBG_STATE
                            {
                                debugger->printState();
                            }
                            | DBG_PRINT {inDebugContext=false;context=new LDictionary();} simple_formula_term_goal {inDebugContext=true; delete context; context=0;}
                            {
                                LiteralGoal * lg = (LiteralGoal *) $3;
                                if($3){
                                debugger->printLiteral(lg);
                                delete lg;
                                }
                            }
                            | DBG_PRINT DBG_AGENDA
                            {
                                debugger->printAgenda();
                            }
                            | DBG_PRINT DBG_PLAN
                            {
                                debugger->printPlan();
                            }
                            | DBG_PRINT DBG_OPTIONS
                            {
                                debugger->printOptions();
                            }
                            | DBG_PRINT DBG_TERMTABLE
                            {
                                debugger->printTermtable();
                            }
                        | DBG_PRINT DBG_TASKS
                        {
                                debugger->printTasks();
                        }
                        | DBG_PRINT DBG_PREDICATES
                        {
                                debugger->printPredicates();
                        }
                            ;

display:                    DBG_DISPLAY DBG_STATE
                            {
                                DisplayElement * de = new DisplayElement();
                                de->name = "state";
                                debugger->displaySymbol(de);
                            }
                            | DBG_DISPLAY {inDebugContext=false;context= new LDictionary();} simple_formula_term_goal {inDebugContext=true; delete context; context=0;}
                            {
                                DisplayElement * de = new DisplayElement();
                                if($3){
                                de->goal = (LiteralGoal *) $3;
                                debugger->displaySymbol(de);
                                }
                                else
                                delete de;
                            }
                            | DBG_DISPLAY DBG_AGENDA
                            {
                                DisplayElement * de = new DisplayElement();
                                de->name = "agenda";
                                debugger->displaySymbol(de);
                            }
                            | DBG_DISPLAY DBG_PLAN
                            {
                                DisplayElement * de = new DisplayElement();
                                de->name = "plan";
                                debugger->displaySymbol(de);
                            }
                            | DBG_DISPLAY
                            {
                                debugger->printDisplays();
                            }
                            | DBG_DISPLAY PDDL_DNUMBER
                            {
                                debugger->display((int)$2);
                            }
                            | DBG_DISPLAY DBG_TERMTABLE
                            {
                                DisplayElement * de = new DisplayElement();
                                de->name = "termtable";
                                debugger->displaySymbol(de);
                            }
                            ;

plot:                       DBG_PLOT DBG_PLAN
                            {
                                debugger->plotPlan();
                            }
                            | DBG_PLOT DBG_CAUSAL
                            {
                                debugger->plotCausal();
                            };

undisplay:                  DBG_UNDISPLAY PDDL_DNUMBER
                            {
                                debugger->undisplay((int)$2);
                            }
                            ;


set:                        DBG_SET DBG_VIEWER
                            {
                                cerr << debugger->viewerCommand << endl;
                            }
                            |  DBG_SET DBG_VIEWER EQUAL DBG_PATH
                            {
                                debugger->viewerCommand = $4;
                            }
                            | DBG_SET DBG_TMPDIR
                            {
                                cerr << debugger->tmpdir << endl;
                            }
                            | DBG_SET DBG_TMPDIR EQUAL DBG_PATH
                            {
                                debugger->tmpdir = $4;
                            }
                            | DBG_SET DBG_DOTPATH
                            {
                                cerr << debugger->dotPath << endl;
                            }
                            | DBG_SET DBG_DOTPATH EQUAL DBG_PATH
                            {
                                debugger->dotPath = $4;
                            }
                            | DBG_SET DBG_VERBOSITY
                        {
                                cerr << current_plan->FLAG_VERBOSE << endl;
                        }
                            | DBG_SET DBG_VERBOSITY EQUAL PDDL_DNUMBER
                        {
                                if($4 >= 0 && $4 <= 3)
                                current_plan->FLAG_VERBOSE = (int) $4;
                        }
                            ;

breakpoint:                 DBG_BREAKPOINT
                            {
                                debugger->printBreakpoints();
                            }
                            | DBG_WATCH {inDebugContext=false;context= new LDictionary();} goal_def {inDebugContext=true;delete context; context=0;}
                            {
                                DisplayElement * de = new DisplayElement();
                                de->goal = (Goal *) $3;
                                debugger->setBreakpoint(de);
                            }
                        | DBG_BREAKPOINT PDDL_DNUMBER
                            {
                                debugger->printBreakpoint((int)$2);
                            }
                        | DBG_ENABLE PDDL_DNUMBER
                        {
                                debugger->enableBreakpoint((int) $2);
                        }
                        | DBG_DISABLE PDDL_DNUMBER
                        {
                                debugger->disableBreakpoint((int) $2);
                        }
                            | DBG_BREAKPOINT {inDebugContext=false;context= new LDictionary();} simple_formula_term_goal  met_name {inDebugContext=true; delete context; context=0;}
                        {
                                DisplayElement * de = new DisplayElement();
                                de->goal = (Goal *) $3;
                                if($3){
                                if($4){
                                        de->name = *((string *) $4);
                                        delete (string *) $4;
                                }
                                debugger->setBreakpoint(de);
                                }
                                else{
                                delete de;
                                }
                        }
                        ;

met_name:                    /*empty*/
                        {
                                $$ = 0;
                        }
                        | PDDL_NAME
                        {
                                $$ = new string($1);
                        };

eval:                            DBG_EVAL {inDebugContext=false;context= new LDictionary();} goal_def {inDebugContext=true;delete context; context=0;}
                            {
                                debugger->eval((Goal *) $3);
                                if($3)
                                delete (Goal *) $3;
                            }
                        ;

apply:                            DBG_APPLY {inDebugContext=false;context= new LDictionary();} effect {inDebugContext=true;delete context; context=0;}
                            {
                                debugger->apply((Effect *) $3);
                                if($3)
                                delete (Effect *) $3;
                            }
                        ;


describe:                   DBG_DESCRIBE {inDebugContext=false;context = new LDictionary();} simple_formula_term_goal {inDebugContext=true; delete context; context=0;}
                            {

                                DisplayElement * de = new DisplayElement();
                                de->goal = (LiteralGoal *) $3;
                                if($3){
                                debugger->describeSymbol(de);
                                }
                                delete de;
                            }
                            ;

simple_formula_term_goal:   LEFTPAR term_name
                        {
                                string * nameLit = (string *) $2;
                                LiteralGoal * lit=0;
                                Meta * mt = 0;
                                // buscamos si el literal ya est� definido en el diccionario de
                                // nombres de literales (deber�a estarlo)
                                ldictionaryit posit = SearchDictionary(&(parser_api->domain->ldictionary),nameLit->c_str());
                                if(posit != (parser_api->domain->ldictionary).end()) {
                                lit = new LiteralGoal(posit->second,parser_api->domain->metainfo.size());
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                }
                                else
                                {
                                snprintf(parerr,256,"Undefined name: `%s'.",nameLit->c_str());
                                yyerror(parerr);
                                lit = new LiteralGoal(idCounter,parser_api->domain->metainfo.size());
                                mt = new Meta(nameLit->c_str(),lexer->getLineNumber(),parser_api->fileid);
                                parser_api->domain->metainfo.push_back(mt);
                                (parser_api->domain->ldictionary).insert(make_pair(lit->getName(), lit->getId()));
                                }
                                container = lit;
                                delete nameLit;
                                contador = 0;
                                }
                                term_list RIGHTPAR
                                {
                                LiteralGoal * lit= (LiteralGoal *) container;
                                container = 0;
                                // comprobaciones de correctitud
                                // busco en el dominio los literales con el nombre capturado
                                  // y el n�mero de argumentos adecuado
                                  // Esto sirve para poner los tipos seg�n est�n definidos en la tabla de literales.
                                int id = SearchDictionary(&(parser_api->domain->ldictionary),lit->getName())->second;
                                  LiteralTableRange r = parser_api->domain->getLiteralRange(id);
                                bool unificacion=false;
                                vector<Literal *> candidates;
                                  for(literaltablecit i = r.first; i != r.second && !unificacion; i++)
                                  {
                                candidates.push_back((*i).second);
                                     if(unify((*i).second->getParameters(),lit->getParameters())){
                                        unificacion=true;
                                     }
                                  }
                                  TaskTableRange tr = parser_api->domain->getTaskRange(id);
                                vector<Task *> candidates2;
                                  for(tasktablecit k = tr.first; k != tr.second && !unificacion; k++)
                                  {
                                candidates2.push_back((*k).second);
                                     if(unify((*k).second->getParameters(),lit->getParameters())){
                                        unificacion=true;
                                     }
                                  }
                                if(!unificacion){
                                snprintf(parerr,256,"No matching predicate or action for: `%s'.",lit->toString());
                                yyerror(parerr);
                                if(candidates.size() > 0) {
                                        *errflow << "Possible candidate predicates:" << endl;
                                        vector<Literal *>::const_iterator j;
                                        for(j=candidates.begin();j!=candidates.end();j++) {
                                        SearchLineInfo sli((*j)->getMetaId());
                                        (*j)->printL(errflow,0);
                                        *errflow << endl;
                                        }
                                }
                                if(candidates2.size() > 0) {
                                        *errflow << "Possible candidate tasks:" << endl;
                                        vector<Task *>::const_iterator j;
                                        for(j=candidates2.begin();j!=candidates2.end();j++) {
                                        SearchLineInfo sli((*j)->getMetaId());
                                        (*j)->printHead(errflow);
                                        *errflow << endl;
                                        }
                                }
                                delete lit;
                                lit = 0;
                                }
                                $$ = lit;
                                }
                                ;

customization_def:                LEFTPAR CUSTOMIZATION customization_body RIGHTPAR
                                ;

customization_body:                /* empty */
                                | customization_list
                                ;

customization_list:                customization_element
                                | customization_list customization_element
                                ;

customization_element:                LEFTPAR EQUAL TIMEUNIT time_unit RIGHTPAR
                                {
                                parser_api->setFlagTUnit((TimeUnit) $4);
                                }
                                | LEFTPAR EQUAL TIMEFORMAT HTN_TEXT RIGHTPAR
                                {
                                parser_api->setTFormat($4);
                                }
                                | LEFTPAR EQUAL TIMEHORIZON PDDL_DNUMBER RIGHTPAR
                                {
                                parser_api->setMTHorizon((int) rint($4));
                                }
                                | LEFTPAR EQUAL RELTIMEHORIZON PDDL_DNUMBER RIGHTPAR
                                {
                                parser_api->setRTHorizon((int) rint($4));
                                }
                                | LEFTPAR EQUAL TIMESTART PDDL_DNUMBER RIGHTPAR
                                {
                                if(parser_api->getTStart() != 0){
                                        snprintf(parerr,256,":time-start redefinition.");
                                        yyerror(parerr);
                                }
                                parser_api->setTStart((time_t) $4);
                                };

python_init:                        LEFTPAR PYTHON_INIT code RIGHTPAR
                                {
#ifdef PYTHON_FOUND
                                if(parser_api->wpython.loadStr($3)){
                                        yyerror("While parsing :pyinit.");
                                }
#else
                                yyerror("Parser compiled without Python support. Install python and recompile.");
#endif
                                }

time_unit:                        THOURS
                                {
                                $$ = TU_HOURS;
                                }
                                | TMINUTES
                                {
                                $$ = TU_MINUTES;
                                }
                                | TSECONDS
                                {
                                $$ = TU_SECONDS;
                                }
                                | TDAYS
                                {
                                $$ = TU_DAYS;
                                }
                                | TYEARS
                                {
                                $$ = TU_YEARS;
                                }
                                | TMONTHS
                                {
                                $$ = TU_MONTHS;
                                }
                                ;
%%
