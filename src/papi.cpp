#include "papi.hh"
#include <string>
#include "MyLexer.hh"
#include "problem.hh"
#include "domain.hh"

char ver_str[] = ""
"\nSiadex Planner\nOscar García, Luis Castillo, Juan Fdez-Olivares\n\n"
"Parsing process: Ignacio Vellido\n";

// el objeto que contiene el analizador l�xico
MyLexer * lexer=0;
int idCounter;
ostream * errflow=0;
PAPI * parser_api=0;

// Matriz para hacer transformaciones temporales.
int TU_Transform[6] = {1,60,3600,86400,31536000,60};

extern int yyparse(void);

PAPI::PAPI(void)
{
    lexer =0;
    domain = 0;
    problem = 0;
    termtable = new TermTable();
    this->errors = false;
    fileid=-1;
    srand((unsigned int)time(0));
    ostr = 0;
    n_problem_errors=0;
    n_problem_warnings=0;
    n_domain_errors=0;
    n_domain_warnings=0;
};

bool PAPI::parse(const char * domainfile, const char * problemfile, bool fast)
{
    setFastMode(fast);
    domain=loadDomain(domainfile);
    if(domain && !this->errors){
    problem=loadProblem(problemfile);
    }
    if(domain && problem && !this->errors) {
    if(!fast) {
        *errflow << "( ----- ) Performing reachability test." << endl;
        tasktablecit i, e = domain->getEndTask();
        for(i = domain->getBeginTask(); i != e; i++)
        (*i).second->isReachable(errflow);
        *errflow << "( ----- )" << endl;
    }
    return true;
    }
    return false;
};

bool PAPI::parse_domain(const char * domainfile, bool fast)
{
    setFastMode(fast);
    domain=loadDomain(domainfile);
    if(domain && !this->errors)
    return true;
    else
    return false;
};

bool PAPI::parse_problem(const char * problemfile, bool fast)
{
    setFastMode(fast);
    if(domain){
    problem=loadProblem(problemfile);
    }
    if(domain && problem && !this->errors)
    return true;
    else
    return false;
};

bool PAPI::parse(istream * domainfile, istream * problemfile, bool fast)
{
    setFastMode(fast);
    domain=loadDomain(domainfile,"domain");
    if(domain && !this->errors){
    problem =loadProblem(problemfile,"problem");
    }
    if(domain && problem && !this->errors)
    {
    if(!fast) {
        *errflow << "( ----- ) Performing reachability test." << endl;
        tasktablecit i, e = domain->getEndTask();
        for(i = domain->getBeginTask(); i != e; i++)
        (*i).second->isReachable(errflow);
        *errflow << "( ----- )" << endl;
    }
    return true;
    }
    else return false;
}

PAPI::~PAPI(void)
{
    if(lexer)
    delete lexer;
    lexer = 0;
    if(problem)
    delete problem;
    if(domain)
    delete domain;
    if(termtable)
    delete termtable;
    if(ostr)
    delete ostr;
};

ifstream * openfile(const char * fileName)
{
    assert(fileName != NULL);
    //*errflow << "Loading file: " << fileName << "...\n";
    ifstream * tmp = new ifstream();
    tmp->open(fileName,ios::in);
    if(!tmp->is_open())
    {
        *errflow << "Unable to open file: " << fileName << "\n";
    return 0;
    }
    return tmp;
}

ofstream * openwfile(const char * fileName)
{
    assert(fileName != NULL);
    // *errflow << "Loading file: " << fileName << "...\n";
    ofstream * tmp = new ofstream();
    tmp->open(fileName,ios::out);
    if(!tmp->is_open())
    {
        *errflow << "Unable to open file: " << fileName << "\n";
    return 0;
    }
    return tmp;
}

Domain * PAPI::loadDomain(const char * domainfile)
{
    ifstream * domainf =NULL;
    domainf = openfile(domainfile);
    domain= 0;
    if(domainf){
    domain= loadDomain(domainf,domainfile);
    delete domainf;
    domainf =0;
    }
    else
    this->errors = true;
    return domain;
}

Domain * PAPI::loadDomain(istream * flow, const char * debugname)
{
    lexer = new MyLexer(flow,errflow);
    lexer->setFileName(debugname);
    fileid = files.size();
    files.push_back(debugname);
    domain = new Domain();

    if(yyparse() || lexer->getErrors()){
    this->errors = true;
    }

    n_domain_warnings = lexer->getWarnings();
    n_domain_errors = lexer->getErrors();

    if(getNDomainErrors() || getNDomainWarnings()){
    *errflow << "################################################################################" << endl;
        *errflow << getNDomainErrors() << " errors " << getNDomainWarnings() << " warnings parsing domain file: " << debugname << endl;
    *errflow << "--------------------------------------------------------------------------------" << endl;
    }
    delete lexer;
    lexer = 0;
    return domain ;
}

Problem * PAPI::loadProblem(const char * problemfile)
{
    ifstream * problemf =NULL;
    problemf = openfile(problemfile);
    problem = 0;
    if(problemf) {
    problem =loadProblem(problemf,problemfile);
    delete problemf;
    problemf =NULL;
    }
    else
    this->errors =true;

    return problem;
}

Problem * PAPI::loadProblem(istream * flow, const char * debugname){
    lexer = new MyLexer(flow,errflow);
    lexer->setFileName(debugname);
    fileid = files.size();
    files.push_back(debugname);
    problem = new Problem(domain);


    if(yyparse() || lexer->getErrors()){
    this->errors = true;
    }

    n_problem_warnings = lexer->getWarnings();
    n_problem_errors = lexer->getErrors();

    if(getNProblemErrors() || getNProblemWarnings()){
    *errflow << "################################################################################" << endl;
        *errflow << getNProblemErrors() << " errors " << getNProblemWarnings() << " warnings parsing problem file: " << debugname << endl;
    *errflow << "--------------------------------------------------------------------------------" << endl;
    }

    delete lexer;
    lexer = 0;
    return problem;
}

const char * PAPI::ver(void) const
{
    return ver_str;
};

void PAPI::resetOutStream(void){
    if(ostr)
    delete ostr;
    ostr = new ostringstream();
    errflow = ostr;
};

const char * PAPI::readOutStream(void){
    if(ostr)
    return ostr->str().c_str();
    else return 0;
};

const char * PAPI::getTFormat(void)const {
    if(problem)
    return problem->TFORMAT.c_str();
    else
    return domain->TFORMAT.c_str();
}

time_t PAPI::getTStart(void) const {
    if(problem)
    return problem->FLAG_TIME_START;
    else
    return domain->FLAG_TIME_START;
}

TimeUnit PAPI::getTUnit(void) const {
    if(problem)
    return problem->FLAG_TIME_UNIT;
    else
    return domain->FLAG_TIME_UNIT;
}

void PAPI::setFlagTUnit(TimeUnit tu){
    if(problem)
    problem->FLAG_TIME_UNIT = tu;
    else
    domain->FLAG_TIME_UNIT = tu;
}

void PAPI::setTFormat(string s){
    if(problem)
    problem->TFORMAT = s;
    else
    domain->TFORMAT = s;
}

void PAPI::setMTHorizon(int t){
    if(problem)
    problem->MAX_THORIZON = t;
    else
    domain->MAX_THORIZON = t;
}

void PAPI::setRTHorizon(int t){
    if(problem)
    problem->REL_THORIZON = t;
    else
    domain->REL_THORIZON = t;
}

void PAPI::setTStart(time_t t){
    if(problem)
    problem->FLAG_TIME_START = t;
    else
    domain->FLAG_TIME_START = t;
};

string TimeUnit2String(const TimeUnit t){
    switch(t){
    case TU_MINUTES:
        return "minutes";
    case TU_HOURS:
        return "hours";
    case TU_SECONDS:
        return "seconds";
    case TU_YEARS:
        return "years";
    case TU_MONTHS:
        return "months";
    case TU_DAYS:
        return "days";
    case TU_UNDEFINED:
        return "minutes";
    }
    return "";
};

