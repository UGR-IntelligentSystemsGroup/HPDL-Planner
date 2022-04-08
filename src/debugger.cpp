#include "debugger.hh"
#include <sstream>
#include <fstream>
#include <errno.h>
#include "task.hh"
#include "domain.hh"
#include "problem.hh"
#include "papi.hh"
#include "undoChangeValue.hh"
#include "causalTable.hh"

#ifdef HAVE_LIBREADLINE
#  if defined(HAVE_READLINE_READLINE_H)
#    include <readline/readline.h>
#  elif defined(HAVE_READLINE_H)
#    include <readline.h>
#  else /* !defined(HAVE_READLINE_H) */
extern char *readline ();
#  endif /* !defined(HAVE_READLINE_H) */
char *cmdline = NULL;
#else /* !defined(HAVE_READLINE_READLINE_H) */
/* no readline */
#endif /* HAVE_LIBREADLINE */

#ifdef HAVE_READLINE_HISTORY
#  if defined(HAVE_READLINE_HISTORY_H)
#    include <readline/history.h>
#  elif defined(HAVE_HISTORY_H)
#    include <history.h>
#  else /* !defined(HAVE_HISTORY_H) */
extern void add_history ();
extern int write_history ();
extern int read_history ();
#  endif /* defined(HAVE_READLINE_HISTORY_H) */
/* no history */
#endif /* HAVE_READLINE_HISTORY */

bool FLAG_DEBUG=false;
bool FLAG_RDEBUG=false;
Debugger * debugger = 0;
extern int yyparse();

vector<string> autocompletion;

void Debugger::init(void)
{
    autocompletion.clear();
    autocompletion.push_back("continue");
    autocompletion.push_back("quit");
    autocompletion.push_back("state");
    autocompletion.push_back("help");
    autocompletion.push_back("plan");
    autocompletion.push_back("next");
    autocompletion.push_back("agenda");
    autocompletion.push_back("display");
    autocompletion.push_back("undisplay");
    autocompletion.push_back("plot");
    autocompletion.push_back("exit");
    autocompletion.push_back("set");
    autocompletion.push_back("viewer");
    autocompletion.push_back("dotpath");
    autocompletion.push_back("describe");
    autocompletion.push_back("break");
    autocompletion.push_back("watch");
    autocompletion.push_back("select");
    autocompletion.push_back("causal");
    autocompletion.push_back("enable");
    autocompletion.push_back("disable");
    autocompletion.push_back("eval");
    autocompletion.push_back("apply");
    autocompletion.push_back("verbosity");

    literaltablecit i, e =parser_api->domain->getEndLiteral();

    for(i=parser_api->domain->getBeginLiteral(); i != e; i++) {
        autocompletion.push_back((*i).second->getName());
    }

    typetablecit it, et = parser_api->domain->getEndType();
    for(it=parser_api->domain->getBeginType(); it != et; it++) {
        autocompletion.push_back((*it)->getName());
    }

    tasktablecit b, ee = parser_api->domain->getEndTask();
    for(b=parser_api->domain->getBeginTask(); b != ee; b++) {
        autocompletion.push_back((*b).second->getName());
    }
};

bool StrCmp(const string &s1, const char* s2) 
{
    int len = strlen(s2);
    return strncasecmp(s1.c_str(), s2, len) == 0;
}

/** 
 * Esta funci�n realiza el autorellenado del comando al pulsar TAB en la l�nea
 * de comandos del debugger.
 */
char * command_generator (const char * text,int state)
{
    static int list_index=0;

    if (!state)
        list_index = 0;

    while((list_index < (int) autocompletion.size()) && (list_index >= 0)) {
        if(StrCmp(autocompletion[list_index],text)) {
            return (strdup(autocompletion[list_index++].c_str()));
        }
        list_index++;
    }

    return ((char *)NULL);
}

Debugger::Debugger(void){
    next = true;
    nexp = false;
    rl_completion_entry_function = command_generator;
    tmpdir = "./";
    viewerCommand = "/usr/bin/display";
    dotPath = "/usr/bin/dot";
    debugger = this;
    setFReadLine(readline);
}

Debugger::~Debugger()
{
    if(lexer)
        delete lexer;
    lexer=0;
    dictionary.clear();
    while(displayList.size() >= 0)
    {
        delete displayList[0];
        displayList[0] = 0;
        displayList.erase(displayList.begin());
    }
    while(breakpointList.size() >= 0)
    {
        delete breakpointList[0];
        breakpointList[0] = 0;
        breakpointList.erase(breakpointList.begin());
    }
}


int Debugger::prompt(Plan * plan)
{
    option =-1;
    init();
    bool stop = false;
    if(next)
        stop = true;
    if(nexp && (plan->getStage()==SelectTask))
        stop = true;
    stop = (stop || testBreakpoints(plan->getStage(),plan->getCurrentContext()));
    if(stop)
    {
        cont = false;
        next = false;
        nexp =false;
        this->plan = plan;
        while(!cont && !next && !nexp)
        {
            if(!lexer)
                lexer = new MyLexer();
            lexer->switch_streams(preprocessLine(),errflow);
            yyparse();
        }

    }
    return option;
}

istream * Debugger::preprocessLine(void){
    istringstream * out = new istringstream();

    for(int i=0; i < (int) displayList.size() && enabledDisplays[i]; i++)
    {
        *errflow << "Display " << i << "::";
        displayList[i]->print(errflow);
        *errflow << endl;
        printSymbol(displayList[i]);
    }
    string read="";
    char * cad;
    cad = read_line("debug:> ");
    if(cad)
    {
        read = ":debug: ";
        read +=	+ cad;
        if(*cad)
        {
            add_history(cad);
        }
        else
        {
            HIST_ENTRY * hisnod = history_get(history_base + history_length -1);
            if(hisnod != 0)
            {
                read = ":debug:";
                read += hisnod->line;
            }
        }
        free(cad);
        cad = 0;
    }
    out->str(read);
    return out;
}

void Debugger::printState(void) const
{
    plan->getState()->print(errflow);
}

void Debugger::printAgenda(void) const{
    const VAgenda * agenda = plan->getCurrentAgenda();
    VAgenda::const_iterator kk, jj;

    plan->getTaskNetwork()->printAgenda(errflow,0,agenda);

    *errflow << "===========" << endl;
    if(agenda->size() > 0){
        *errflow << endl << "Not explored (" << agenda->size() << "): ";
        kk = agenda->end();
        for(jj = agenda->begin(); jj != kk; jj++)
            *errflow << " " << (*jj).first;
        *errflow <<  endl;
    }
    else{
        *errflow << "Agenda is empty" << endl;
    }
}

void Debugger::printPlan(void) const
{
    for_each(plan->getCurrentPlan()->begin(),plan->getCurrentPlan()->end(),PrintHead<Task,TaskNetwork>(errflow,0,plan->getTaskNetwork()));
}

void Debugger::printDisplays(void) const
{
    for(int i=0; i < (int) displayList.size(); i++)
    {
        *errflow << "Display " << i << "::";
        displayList[i]->print(errflow);
        *errflow << endl;
    }
}

void Debugger::printLiteral(const LiteralGoal * lg) const{

    // literales del estado que coinciden con mi nombre
    ISTable_range  r = plan->getState()->getRange(lg->getId());
    // axiomas del dominio que coinciden con mi nombre
    AxiomTableRange ra = parser_api->domain->getAxiomRange(lg->getId());

    bool matching=false;

    iscit i, e;
    axiomtablecit ba, ea;

    if(lg->getPolarity())
    {
        e = r.second;
        for(i = r.first; i != e; i++)
            if((*i).second->getPol()){
                if(unify((*i).second->getParameters(),lg->getParameters())){
                    matching = true;
                    (*i).second->printL(errflow,0);
                    *errflow << endl;
                }
            }

        ea = ra.second;
        for(ba = ra.first; ba!= ea; ba++)
            if(unify((*ba).second->getParameters(),lg->getParameters())){
                matching = true;
                (*ba).second->print(errflow,0);
                *errflow << endl;
            }

        TPlan::const_iterator j, k = plan->getCurrentPlan()->end();
        for(j = plan->getCurrentPlan()->begin(); j != k; j++)
        {
            const Task * t = plan->getTaskNetwork()->getTask(*j);
            if(strcasecmp(t->getName(), lg->getName()) == 0)
            {
                if(unify(t->getParameters(),lg->getParameters()))
                {
                    matching = true;
                    *errflow << "Found in plan: " << endl;
                    t->print(errflow);
                    *errflow << endl;
                }
            }
        }

        VAgenda::const_iterator kk, jj;
        kk = plan->getCurrentAgenda()->end();
        for(jj = plan->getCurrentAgenda()->begin(); jj != kk; jj++)
        {
            const Task * t = plan->getTaskNetwork()->getTask((*jj).first);
            if(strcasecmp(t->getName(), lg->getName()) == 0)
            {
                if(unify(t->getParameters(),lg->getParameters()))
                {
                    matching = true;
                    *errflow << "Found in agenda: " << endl;
                    t->print(errflow);
                    *errflow << endl;
                }
            }
        }

    }
    else
    {
        e = r.second;
        for(i = r.first;i != e; i++)
            if(unify((*i).second->getParameters(),lg->getParameters()))
                if(!(*i).second->getPol()){
                    (*i).second->printL(errflow,0);
                    *errflow << endl;
                    matching = true;
                }
    }

};

void Debugger::printSymbol(const DisplayElement * de) const
{
    if(de->goal)
        printLiteral((LiteralGoal *) de->goal);
    else{
        if(strcasecmp(de->name.c_str(),"agenda") == 0)
        {
            printAgenda();
            return;
        }
        if(strcasecmp(de->name.c_str(),"plan") == 0)
        { 
            printPlan();
            return;
        }
        if(strcasecmp(de->name.c_str(),"state") == 0)
        {
            printState();
            return;
        }
        if(strcasecmp(de->name.c_str(),"termtable") == 0)
        {
            printTermtable();
            return;
        }
    }
}

void Debugger::displaySymbol(DisplayElement * exp)
{
    displayList.push_back(exp);
    enabledDisplays.push_back(true);
}

void Debugger::printOptions(void) const
{
    if(options)
    {
        vector<string>::const_iterator i,e;
        e = options->end();
        int c = 0;
        for(i = options->begin(); i != e; i++, c++)
        {
            *errflow << "[" << c << "] " << (*i) << endl;
        }
    }
    else
    {
        *errflow << "Next task(s) in agenda: ";
        VAgenda::const_iterator i;
        for(i = plan->getCurrentAgenda()->begin(); i != plan->getCurrentAgenda()->end(); i++){
            *errflow << (*i).first << " ";
        }
        *errflow << endl;
    }
}

void Debugger::select(int option)
{
    if(options)
    {
        if(option >= 0 && option < (int) options->size())
            this->option = option;
        else
            *errflow << "Error: Option must be between [0 and " << options->size() << ")." << endl;
    }
    else
    {
        bool find = false;
        for(VAgenda::const_iterator i = plan->getCurrentAgenda()->begin(); !find && i != plan->getCurrentAgenda()->end(); i++)
        {
            if((*i).first == option)
            {
                find = true;
                this->option = option;
            }
        }
        if(!find)
            *errflow << "Selected task not in agenda." << endl;
    }
}

void Debugger::describeSymbol(DisplayElement * exp)
{
    if(exp->goal){
        LiteralGoal * lg = (LiteralGoal *) exp->goal;
        LiteralTableRange  r = parser_api->domain->getLiteralRange((parser_api->domain->ldictionary).find(lg->getName())->second);

        for(literaltablecit i = r.first; i != r.second; i++)
        {
            if(unify((*i).second->getParameters(),lg->getParameters()))
            {
                // imprimir el literal
                *errflow << "Found in State: " << endl;
                (*i).second->printL(errflow);
                *errflow << endl;
            }
        }

        TaskTableRange tr = parser_api->domain->getTaskRange((parser_api->domain->ldictionary).find(lg->getName())->second);

        for(tasktablecit k = tr.first; k != tr.second; k++)
        {
            if(unify((*k).second->getParameters(),lg->getParameters()))
            {
                (*k).second->print(errflow);
                *errflow << endl;
            }
        }
    }
}

void Debugger::describeSymbol(const char * name)
{
    *errflow << endl << endl << "Describe " << name << endl;
    LiteralTableRange  r = parser_api->domain->getLiteralRange((parser_api->domain->ldictionary).find(name)->second);

    for(literaltablecit i = r.first; i != r.second; i++)
    {
        (*i).second->printL(errflow);
        *errflow << endl;
    }

    TaskTableRange tr = parser_api->domain->getTaskRange((parser_api->domain->ldictionary).find(name)->second);

    for(tasktablecit k = tr.first; k != tr.second; k++)
    {
        (*k).second->print(errflow);
        *errflow << endl;
    }

    const Type * typ = parser_api->domain->getType(name);
    if(typ)
    {
        typ->print(errflow);
        *errflow << endl;
    }
};

void Debugger::undisplay(int index) {
    if(index >= 0  && 0 <= displayList.size()) {
        enabledDisplays[index] = false;
    }
}

void Debugger::display(int index) {
    if(index >= 0 && 0 <= displayList.size()) {
        enabledDisplays[index] = true;
    }
}

void Debugger::setBreakpoint(DisplayElement * exp) {
    breakpointList.push_back(exp);
    enabledBreakpoints.push_back(true);
};

bool Debugger::testBreakpoints(Stage stage,const StackNode * n ){
    int i, s = breakpointList.size();
    for(i=0; i<s; i++){
        if(enabledBreakpoints[i]){
            Unifier u;
            UnifierTable * ut;
            if(breakpointList[i]->goal){
                if((ut = breakpointList[i]->goal->getUnifiers(plan->getState(),&u,true,0))){
                    if(ut && !ut->isEmpty()){
                        *errflow << "Breakpoint " << i << " reached." << endl;
                        return true;
                    }
                    if(ut)
                        delete ut;
                }
                if(stage == SelectTask){
                    if(breakpointList[i]->goal->isLiteral() && (breakpointList[i]->name.size() == 0)){
                        LiteralGoal * lg = (LiteralGoal *) breakpointList[i]->goal;
                        const Task * father = plan->getTaskNetwork()->getTask(n->taskid);
                        if(father->getId() == lg->getId())
                            if(unify(father->getParameters(),lg->getParameters())){
                                *errflow << "Breakpoint " << i << " reached." << endl;
                                return true;
                            }
                    }
                }
                else if(stage == SelectUnification){
                    if(breakpointList[i]->goal->isLiteral() && (breakpointList[i]->name.size() > 0)){
                        LiteralGoal * lg = (LiteralGoal *) breakpointList[i]->goal;
                        const Task * father = plan->getTaskNetwork()->getTask(n->taskid);
                        if(father->isCompoundTask() && father->getId() == lg->getId()){
                            const Method * m = (*(n->methods))[n->mpos];
                            if(strcasecmp(m->getName(),breakpointList[i]->name.c_str()) == 0)
                                if(unify(father->getParameters(),lg->getParameters())){
                                    *errflow << "Breakpoint " << i << " reached." << endl;
                                    return true;
                                }
                        }
                    }
                }
            }
        }
    }
    return false;
};

void Debugger::eval(const Goal * g) const {
    Unifier u;
    UnifierTable * ut;

    if(g){
        *errflow << "Evaluating: ";
        g->print(errflow);
        *errflow << endl;
        if((ut = g->getUnifiers(plan->getState(),&u,true,0))){
            if(ut && !ut->isEmpty()){
                ut->print(errflow);
            }
            else
                *errflow << "No unification" << endl;
            if(ut)
                delete ut;
        }
        else
            *errflow << "No unification" << endl;
    }
};

struct PrintUndoElementln2
{
    ostream * os;
    PrintUndoElementln2(ostream * o) {os=o;};

    void operator()(const UndoElement * u) const {
        if(u->isUndoChangeValue()) {
            *os << "Changing value of fluent: ";
            u->print(os);
        }
        else
            u->print(os);
        *os << endl;
    };
};

void Debugger::apply(Effect * e) const {
    Unifier u;
    VUndo undo;

    if(e){
        if(e->apply(plan->getModificableState(),&undo,0)){
            PrimitiveTask::commitEffects(plan->getModificableState(),&undo,0);
            for_each(undo.begin(),undo.end(),PrintUndoElementln2(errflow));
        }
    }
};

void Debugger::disableBreakpoint(int index) {
    if(index >= 0  && 0 <= breakpointList.size()) {
        enabledBreakpoints[index] = false;
    }
}

void Debugger::enableBreakpoint(int index) {
    if(index >= 0 && 0 <= breakpointList.size()) {
        enabledBreakpoints[index] = true;
    }
}


void Debugger::printBreakpoints(void) const
{
    for(int i=0; i < (int) breakpointList.size(); i++)
    {
        *errflow << "Breakpoint " << i << "::";
        if(enabledBreakpoints[i])
            *errflow << "<enabled>";
        else
            *errflow << "<disabled>";
        breakpointList[i]->print(errflow);
        *errflow << endl;
    }
}

void Debugger::printBreakpoint(int index) const
{
    if(index >= 0 && index < (int) breakpointList.size())
    {
        *errflow << "Breakpoint " << index << "::";
        breakpointList[index]->print(errflow);
        *errflow << endl;
    }
}

void Debugger::printTermtable(void) const
{
    parser_api->termtable->print(errflow);
};

void Debugger::plotPlan(void)
{
    ofstream * tmp = new ofstream();
    tmp->open((tmpdir + "out.dot").c_str(),ios::out);
    if(!tmp->is_open())
    {
        *errflow << "Unable to open file: " << tmpdir + "out.dot" << "\n";
        return;
    }

    *tmp << "digraph plan {" << endl;

    int counter =0;
    for(tasklistcit i = plan->getTaskNetwork()->getBeginTask(); i!= plan->getTaskNetwork()->getEndTask(); i++)
    {
        if((*i)->isTaskHeader())
        {
            *tmp << "n" << counter << "[label=\"" << (*i)->getName() << "\",color=red]" << endl;
        }
        else if((*i)->isPrimitiveTask())
        {
            *tmp << "n" << counter << "[label=\"" << (*i)->getName() << "\",color=green,shape=box]" << endl;
        }
        counter++;
    }

    int size = plan->getTaskNetwork()->getNumOfNodes();
    for(int k =0; k<size; k++)
        for(int j =0; j<size; j++)
        {
            if( plan->getTaskNetwork()->inmediatelyAfter(k,j))
                *tmp << "n" << k << "->" << "n" << j << endl;
        }

    *tmp << "}";
    delete tmp;

    string command = dotPath + " -Tpng -o" + tmpdir + "out.png " + tmpdir + "out.dot";
    int ret = system(command.c_str());
    command = viewerCommand + " " + tmpdir + "out.png &";
    ret = system(command.c_str());
}

void Debugger::plotCausal(void)
{
    ofstream * tmp = new ofstream();
    tmp->open((tmpdir + "out.dot").c_str(),ios::out);
    if(!tmp->is_open())
    {
        *errflow << "Unable to open file: " << tmpdir + "out.dot" << "\n";
        return;
    }

    causalTable.plot(tmp);
    delete tmp;

    string command = dotPath + " -Tpng -o" + tmpdir + "out.png " + tmpdir + "out.dot";
    int ret = system(command.c_str());
    command = viewerCommand + " " + tmpdir + "out.png &";
    ret = system(command.c_str());
}

void Debugger::printPredicates(void) const{
    for_each(parser_api->domain->getBeginLiteral(),parser_api->domain->getEndLiteral(),PrintL<Literal>(errflow));
};

void Debugger::printTasks(void) const{
    for_each(parser_api->domain->getBeginTask(),parser_api->domain->getEndTask(),PrintHead<Task,TaskNetwork>(errflow));
};

int Debugger::selectFromAgenda(Plan * p){
    int r = prompt(p);
    if(r == -1)
        return plan->getCurrentAgenda()->back().first;
    else return r;
};

int Debugger::selectTask(Plan * p){
    int r = prompt(p);
    if (r == -1)
        return options->size() -1;
    else
        return r;
};

int Debugger::selectMethod(Plan * p){
    int r= prompt(p);
    if (r == -1)
        return 0;
    else
        return r;
};

int Debugger::selectUnification(Plan * p) {
    int r = prompt(p);
    if (r== -1)
        return 0;
    else
        return r;
};


