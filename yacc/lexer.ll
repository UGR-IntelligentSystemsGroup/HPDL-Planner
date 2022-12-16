%option outfile="src/lexer.cpp" noyywrap caseless c++
%{

using namespace std;

#include "constants.hh"
#include <string>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctime>
#include "parser.hh"
#include "MyLexer.hh"
#include "papi.hh"

static bool inComment = false;
static bool inScript = false;
static bool inStr = false;
static string script;
static string buffer;
extern bool AtExpected;
extern bool inDebugContext;
extern int TU_Transform[6];

extern MyLexer * lexer;

extern char * yytext;
extern int yylineno;
extern int yy_flex_debug;

void MyLexer::initErrorTable()
{

    dictionary["LEFTPAR"] = "`('";
    dictionary["RIGHTPAR"] = "`)'";
    dictionary["PDDL_DEFINE"] = "`define'";
    dictionary["CUSTOMIZATION"] = "`:customization'";
    dictionary["TIMEUNIT"] = "`:time-unit'";
    dictionary["TIMESTART"] = "`:time-start'";
    dictionary["TIMEFORMAT"] = "`:time-format'";
    dictionary["TIMEHORIZON"] = "`:time-horizon-limit'";
    dictionary["RELTIMEHORIZON"] = "`:time-horizon-relative'";
    dictionary["THOURS"] = "`:hours'";
    dictionary["TMINUTES"] = "`:minutes'";
    dictionary["TSECONDS"] = "`:seconds'";
    dictionary["TDAYS"] = "`:days'";
    dictionary["TMONTHS"] = "`:months'";
    dictionary["TYEARS"] = "`:years'";
    dictionary["PDDL_DOMAINREF"] = "`:domain'";
    dictionary["PDDL_DOMAIN"] = "`domain'";
    dictionary["PDDL_PROBLEM"] = "`problem'";
    dictionary["PDDL_NAME"] = "identificator";
    dictionary["$end"] = "end of file";
    dictionary["PDDL_REQUIREMENTS"]= "`:requirements'";
    dictionary["PDDL_STRIPS"] = "`:strips'";
    dictionary["PDDL_TYPING"] = "`:typing'";
    dictionary["PDDL_NEGATIVE_PRECONDITIONS"] = "`:negative-preconditions'";
    dictionary["PDDL_DISJUNCTIVE_PRECONDITIONS"] = "`:disjunctive-preconditions'";
    dictionary["PDDL_EQUALITY"] = "`:equality'";
    dictionary["PDDL_EXISTENTIAL_PRECONDITIONS"] = "`:existential-preconditions'";
    dictionary["PDDL_UNIVERSAL_PRECONDITIONS"] = "`:universal-preconditions'";
    dictionary["PDDL_QUANTIFIED_PRECONDITIONS"] = "`:quantified-preconditions'";
    dictionary["PDDL_CONDITIONAL_EFFECTS"] = "`:conditional-effects'";
    dictionary["PDDL_FLUENTS"] = "`:fluents'";
    dictionary["PDDL_ADL"] = "`:adl'";
    dictionary["PDDL_DURATIVE_ACTIONS"] = "`:durative-actions'";
    dictionary["PDDL_DERIVED_PREDICATES"] = "`:derived-predicates'";
    dictionary["PDDL_TIMED_INITIAL_LITERALS"] = "`:timed-initial-literals'";
    dictionary["PDDL_TYPES"] = "`types'";
    dictionary["PDDL_EITHER"] = "`either'";
    dictionary["PDDL_HYPHEN"] = "`-'";
    dictionary["PDDL_PREDICATES"] = "`:predicates'";
    dictionary["PDDL_FUNCTIONS"] = "`:functions'";
    dictionary["PDDL_VAR"] = "variable";
    dictionary["PDDL_ACTION"] = "`:action'";
    dictionary["PDDL_NOT"] = "`not'";
    dictionary["PDDL_PARAMETERS"] = "`:parameters'";
    dictionary["PDDL_PRECONDITION"] = ":precondition";
    dictionary["PDDL_IMPLY"] = "`imply'";
    dictionary["PDDL_AND"] = "`and'";
    dictionary["PDDL_OR"] = "`or'";
    dictionary["PDDL_EXISTS"] = "`exists'";
    dictionary["PDDL_FORALL"] = "`forall'";
    dictionary["PDDL_INIT"] = "`:init'";
    dictionary["PLUS"]="`+'";
    dictionary["MINUS"]="`-'";
    dictionary["DIVIDE"]="`/'";
    dictionary["MULTIPLY"]="`*'";
    dictionary["GREATHER"]="`>'";
    dictionary["LESS"]="`<'";
    dictionary["EQUAL"]="`='";
    dictionary["DISTINCT"]="`<>'";
    dictionary["GREATHER_EQUAL"]="`>='";
    dictionary["LESS_EQUAL"]="`<='";
    dictionary["PDDL_NUMBER"] = "number";
    dictionary["PDDL_DNUMBER"] = "number";
    dictionary["PDDL_EFFECT"] = "`:effect'";
    dictionary["PDDL_ASSIGN"] = "`assign'";
    dictionary["PDDL_BIND"] = "`bind'";
    dictionary["PDDL_SCALE_UP"] = "`scale-up'";
    dictionary["PDDL_SCALE_DOWN"] = "`scale-down'";
    dictionary["PDDL_INCREASE"] = "`increase'";
    dictionary["PDDL_DECREASE"] = "`decrease'";
    dictionary["PDDL_WHEN"] = "`when'";
    dictionary["PDDL_GOAL"] = "`:goal'";
    dictionary["PDDL_AT"] = "at";
    dictionary["PDDL_OBJECT"] = "`:objects'";
    dictionary["PDDL_OVERALL"] = "`over all'";
    dictionary["PDDL_DURATIONVAR"] = "`?duration'";
    dictionary["STARTVAR"] = "`?start'";
    dictionary["ENDVAR"] = "`?end'";
    dictionary["PDDL_DERIVED"] = "`:derived'";
    dictionary["PDDL_CONDITION"] = "`:condition'";
    dictionary["PDDL_ATSTART"] = "`at start'";
    dictionary["PDDL_DURATION"] = "`:duration'";
    dictionary["PDDL_DURATIVE_ACTION"] = ":durative-action";
    dictionary["PDDL_ATEND"] = "at end";
    dictionary["PDDL_CONSTANTS"] = ":constants";
    dictionary["HTN_EXPANSION"] = ":htn-expansion";
    dictionary["META_TAGS"] = ":metatags";
    dictionary["META"] = ":meta";
    dictionary["TAG"] = ":tag";
    dictionary["HTN_TASK"] = "`:task'";
    dictionary["HTN_TASKS"] = "`:tasks'";
    dictionary["HTN_ACHIEVE"] = "`:achieve'";
    dictionary["HTN_TEXT"] = "\"<string>\"";
    dictionary["HTN_METHOD"] = "`:method'";
    dictionary["HTN_TASKSGOAL"] = "`:tasks-goal'";
    dictionary["PYTHON_CODE"] = "<scripting code>";
    dictionary["HTN_INLINE"] = ":inline";
    dictionary["HTN_INLINECUT"] = ":!inline";
    dictionary["LEFTBRAC"] = "[";
    dictionary["RIGHTBRAC"] = "]";
    dictionary["EXCLAMATION"] = "!";
    dictionary["RANDOM"] = ":random";
    dictionary["SORTBY"] = ":sortby";
    dictionary["ASC"] = ":asc";
    dictionary["DESC"] = ":desc";
    dictionary["MAINTAIN"] = ":maintain";
    dictionary["PDDL_BETWEEN"] = "between";
    dictionary["POW"] = ":pow";
    dictionary["SQRT"] = ":sqrt";
    dictionary["ABS"] = ":abs";
    dictionary["PPRINT"] = ":print";
    dictionary["DBG_QUIT"] = "`quit'";
    dictionary["DBG_CONTINUE"] = "`continue'";
    dictionary["DBG_HELP"] = "`help'";
    dictionary["DBG_PATH"] = "`<path>'";
    dictionary["DBG_PRINT"] = "`print'";
    dictionary["DBG_DISPLAY"] = "`display'";
    dictionary["DBG_DESCRIBE"] = "`describe'";
    dictionary["DBG_UNDISPLAY"] = "`undisplay'";
    dictionary["DBG_STATE"] = "`state'";
    dictionary["DBG_AGENDA"] = "`agenda'";
    dictionary["DBG_PLAN"] = "`plan'";
    dictionary["DBG_VIEWER"] = "`viewer'";
    dictionary["DBG_SET"] = "`set'";
    dictionary["DBG_PLOT"] = "`plot'";
    dictionary["DBG_DOTPATH"] = "`dotpath'";
    dictionary["DBG_TMPDIR"] = "`tmpdir'";
    dictionary["DBG_BREAKPOINT"] = "`break'";
    dictionary["DBG_WATCH"] = "`break'";
    dictionary["DBG_MEM"] = "`mem'";
    dictionary["DBG_SELECT"] = "`select'";
    dictionary["DBG_OPTIONS"] = "`options'";
    dictionary["DBG_VERBOSE"] = "`verbose'";
    dictionary["DBG_ON"] = "`on'";
    dictionary["DBG_OFF"] = "`off'";
    dictionary["DBG_TERMTABLE"] = "`termtable'";
    dictionary["DBG_CAUSAL"] = "`causal'";
    dictionary["DBG_DEBUG"] = "`:debug:'";
    dictionary["DBG_TASKS"] = "`tasks'";
    dictionary["DBG_PREDICATES"] = "`predicates'";
    dictionary["DBG_ENABLE"] = "`enable'";
    dictionary["DBG_ENABLE"] = "`disable'";
    dictionary["DBG_NEXT"] = "`next'";
    dictionary["DBG_NEXP"] = "`nexp'";
    dictionary["DBG_EVAL"] = "`eval'";
    dictionary["DBG_VERBOSITY"] = "`verbosity'";
    dictionary["DBG_APPLY"] = "`apply'";
    dictionary["PDDL_AND_EVERY"] = "`and every'";
    dictionary["PYTHON_INIT"] = "`:PyInit'";
}

%}
Name        [a-zA-ZáéíóúÁÉÍÓÚÑñÈÜü\xe1\xe9\xed\xf3\xfa\xc1\xc9\xcd\xd3\xda\xd1\xf1\xdc\xfc][0-9a-zA-Z_\-áéíóúÁÉÍÓÚÑñÈÜü\.\xe1\xe9\xed\xf3\xfa\xc1\xc9\xcd\xd3\xda\xd1\xf1\xdc\xfc]*
VarName     "?"[a-zA-ZáéíóúÁÉÍÓÚÑñÈÜü\xe1\xe9\xed\xf3\xfa\xc1\xc9\xcd\xd3\xda\xd1\xf1\xdc\xfc][0-9a-zA-Z_\-áéíóúÁÉÍÓÚÑñÜü\.\xe1\xe9\xed\xf3\xfa\xc1\xc9\xcd\xd3\xda\xd1\xf1\xdc\xfc]*
Blanks      [ \t]+
NewLine     \r*\n
Number      [\+\-]*[0-9]+
Float        [\+\-]*[0-9]+\.[0-9]+
StStr        [\"\']
StrPath     [0-9a-zA-Z_\-áéíóúÁÉÍÓÚÑñ\/\.]+

%%
oacute            {cerr << "se encontro o" <<endl;}
";"                             {inComment = true;}
"{"                {
                if(!inComment) {
                    if(!inScript){
                    inScript = true;
                    script = "";
                    }
                    else
                    script += yytext;
                }
                }
"}"                             {
                if(!inComment) {
                    if(!inScript){
                    REJECT;
                    }
                    else {
                    script += "\n";
                    inScript = false;
                    buffer = script;
                    yylval.type_string = buffer.c_str();
                    return PYTHON_CODE;
                    }
                }
                                }
[^}\n]+                         {
                if(!inComment){
                    if(inScript)
                    script += yytext;
                    else
                    REJECT;
                }
                                }

{StStr}                {
                if(!inStr) {
                    inStr = true;
                    buffer = "";
                }
                else
                {
                    inStr = false;
                    struct tm ti = {0,0,0,0,0,0,0,0,-1};
                    time_t tim = 0;
                    if(strptime(buffer.c_str(),parser_api->getTFormat(),&ti))
                    {
                    tim=mktime(&ti);
                    if(parser_api->getTStart() != 0){
                        yylval.type_number = ((double) rint(difftime(tim, parser_api->getTStart())));
                        yylval.type_number= rint(yylval.type_number/((1.0)*TU_Transform[(int)parser_api->getTUnit()]));
                    }
                    else
                        yylval.type_number = (double) tim;

                    //cerr << "-----> " << buffer << " se convirtió a: " << yylval.type_number <<  " " << parser_api->getTFormat() << endl;
                    return PDDL_DNUMBER;
                    }
                    else{
                    yylval.type_string = buffer.c_str();
                    return HTN_TEXT;
                    }
                }
                }

[^\n\"]+                        {
                    if(inStr)
                    buffer += yytext;
                    else
                    REJECT;
                                }

{NewLine}                       {if(inComment) inComment = false;
                                 if(inScript) script += "\n";
                                 if(inStr) buffer += "\n";
                                 lexer->lineNum++;}
"(:import"[ \t]*\"[^\n\"]+\"")" {
                    string filename = yytext;
                    int pos = filename.find("\"",0);
                    filename = filename.substr(pos+1,filename.size()-(pos +3));
                    lexer->switchTo(filename);
                }
"("                             {return LEFTPAR;}
")"                             {return RIGHTPAR;}
"define"                        {return PDDL_DEFINE;}
":customization"                {return CUSTOMIZATION;}
":time-unit"            {return TIMEUNIT;}
":time-start"                   {return TIMESTART;}
":time-format"                  {return TIMEFORMAT;}
":time-horizon-limit"           {return TIMEHORIZON;}
":time-horizon-relative"        {return RELTIMEHORIZON;}
":hours"            {return THOURS;}
":minutes"                {return TMINUTES;}
":seconds"            {return TSECONDS;}
":days"                {return TDAYS;}
":months"            {return TMONTHS;}
":years"            {return TYEARS;}
":domain"                       {return PDDL_DOMAINREF;}
"domain"                        {return PDDL_DOMAIN;}
"problem"                       {return PDDL_PROBLEM;}
":constants"                    {return PDDL_CONSTANTS;}
":requirements"                 {return PDDL_REQUIREMENTS;}
":strips"                       {return PDDL_STRIPS;}
":typing"                       {return PDDL_TYPING;}
":negative-preconditions"       {return PDDL_NEGATIVE_PRECONDITIONS;}
":disjunctive-preconditions"     {return PDDL_DISJUNCTIVE_PRECONDITIONS;}
":equality"                     {return PDDL_EQUALITY;}
":existential-preconditions"    {return PDDL_EXISTENTIAL_PRECONDITIONS;}
":universal-preconditions"      {return PDDL_UNIVERSAL_PRECONDITIONS;}
":quantified-preconditions"     {return PDDL_QUANTIFIED_PRECONDITIONS;}
":conditional-effects"          {return PDDL_CONDITIONAL_EFFECTS;}
":fluents"                      {return PDDL_FLUENTS;}
":adl"                          {return PDDL_ADL;}
":durative-actions"             {return PDDL_DURATIVE_ACTIONS;}
":derived-predicates"           {return PDDL_DERIVED_PREDICATES;}
":timed-initial-literals"       {return PDDL_TIMED_INITIAL_LITERALS;}
":htn-expansion"                {return HTN_EXPANSION;}
":metatags"            {return META_TAGS;}
":meta"                {return META;}
":tag"                {return TAG;}
":inline"            {return HTN_INLINE;}
":!inline"            {return HTN_INLINECUT;}
":types"                        {return PDDL_TYPES;}
":PyInit"                       {return PYTHON_INIT;}
"either"                        {return PDDL_EITHER;}
":predicates"                   {return PDDL_PREDICATES;}
":functions"                    {return PDDL_FUNCTIONS;}
":action"                       {return PDDL_ACTION;}
":parameters"                   {return PDDL_PARAMETERS;}
":precondition"                 {return PDDL_PRECONDITION;}
":effect"                       {return PDDL_EFFECT;}
":sortby"                       {return SORTBY;}
":asc"                            {return ASC;}
":desc"                {return DESC;}
":maintain"            {return MAINTAIN;}
":init"                         {return PDDL_INIT;}
"not"                           {return PDDL_NOT;}
"and"                           {return PDDL_AND;}
"or"                            {return PDDL_OR;}
"imply"                         {return PDDL_IMPLY;}
"exists"                        {return PDDL_EXISTS;}
"forall"                        {return PDDL_FORALL;}
"when"                          {return PDDL_WHEN;}
"scale-up"                      {return PDDL_SCALE_UP;}
"scale-down"                    {return PDDL_SCALE_DOWN;}
"assign"                        {return PDDL_ASSIGN;}
"increase"                      {return PDDL_INCREASE;}
"decrease"                      {return PDDL_DECREASE;}
"bind"                          {return PDDL_BIND;}
"+"                             {return PLUS;}
"*"                             {return MULTIPLY;}
"/"                             {return DIVIDE;}
"<="                            {return LESS_EQUAL;}
"="                             {return EQUAL;}
"<"                             {return LESS;}
">"                             {return GREATHER;}
">="                            {return GREATHER_EQUAL;}
"!="                            {return DISTINCT;}
"<>"                            {return DISTINCT;}
":pow"                {return POW;};
":abs"                {return ABS;};
":sqrt"                {return SQRT;};
":print"            {return PPRINT;};
":goal"                         {return PDDL_GOAL;}
"at"                            {
                if(AtExpected)
                    return PDDL_AT;
                else
                    REJECT;
                }
"between"                       {return PDDL_BETWEEN;}
":objects"                      {return PDDL_OBJECT;}
"over"{Blanks}+"all"            {return PDDL_OVERALL;}
"?duration"                     {
                    buffer = "?duration";
                    yylval.type_string = buffer.c_str();
                    return PDDL_DURATIONVAR;
                }
"?start"            {
                    buffer = "?start";
                    yylval.type_string = buffer.c_str();
                    return STARTVAR;
                }
"?end"                            {
                    buffer = "?end";
                    yylval.type_string = buffer.c_str();
                    return ENDVAR;
                }
":derived"                      {return PDDL_DERIVED;}
":condition"                    {return PDDL_CONDITION;}
"at"{Blanks}+"start"            {return PDDL_ATSTART;}
":duration"                     {return PDDL_DURATION;}
":durative-action"              {return PDDL_DURATIVE_ACTION;}
"at"{Blanks}+"end"              {return PDDL_ATEND;}
"and"{Blanks}+"every"           {return PDDL_AND_EVERY;}
":task"                         {return HTN_TASK;}
":tasks"                        {return HTN_TASKS;}
":achieve"                      {return HTN_ACHIEVE;}
":method"                       {return HTN_METHOD;}
":tasks-goal"                   {return HTN_TASKSGOAL;}
":random"                       {return RANDOM;}

":debug:"           {return DBG_DEBUG;}
"quit"              {if(inDebugContext) return DBG_QUIT; else REJECT;}
"exit"              {if(inDebugContext) return DBG_QUIT; else REJECT;}
"continue"          {if(inDebugContext) return DBG_CONTINUE; else REJECT;}
"c"                 {if(inDebugContext) return DBG_CONTINUE; else REJECT;}
"next"              {if(inDebugContext) return DBG_NEXT; else REJECT;}
"n"                 {if(inDebugContext) return DBG_NEXT; else REJECT;}
"nexp"              {if(inDebugContext) return DBG_NEXP; else REJECT;}
"ne"                {if(inDebugContext) return DBG_NEXP; else REJECT;}
"help"              {if(inDebugContext) return DBG_HELP; else REJECT;}
"h"                 {if(inDebugContext) return DBG_HELP; else REJECT;}
"print"             {if(inDebugContext) return DBG_PRINT; else REJECT;}
"p"                 {if(inDebugContext) return DBG_PRINT; else REJECT;}
"inspect"           {if(inDebugContext) return DBG_PRINT; else REJECT;}
"display"           {if(inDebugContext) return DBG_DISPLAY; else REJECT;}
"d"                 {if(inDebugContext) return DBG_DISPLAY; else REJECT;}
"describe"          {if(inDebugContext) return DBG_DESCRIBE; else REJECT;}
"undisplay"         {if(inDebugContext) return DBG_UNDISPLAY; else REJECT;}
"dotpath"           {if(inDebugContext) return DBG_DOTPATH; else REJECT;}
"tmpdir"            {if(inDebugContext) return DBG_TMPDIR; else REJECT;}
"state"             {if(inDebugContext) return DBG_STATE; else REJECT;}
"agenda"            {if(inDebugContext) return DBG_AGENDA; else REJECT;}
"plan"              {if(inDebugContext) return DBG_PLAN; else REJECT;}
"plot"              {if(inDebugContext) return DBG_PLOT; else REJECT;}
"set"               {if(inDebugContext) return DBG_SET; else REJECT;}
"viewer"            {if(inDebugContext) return DBG_VIEWER; else REJECT;}
"break"            {if(inDebugContext) return DBG_BREAKPOINT; else REJECT;}
"b"                 {if(inDebugContext) return DBG_BREAKPOINT; else REJECT;}
"watch"            {if(inDebugContext) return DBG_WATCH; else REJECT;}
"w"                 {if(inDebugContext) return DBG_WATCH; else REJECT;}
"eval"            {if(inDebugContext) return DBG_EVAL; else REJECT;}
"enable"        {if(inDebugContext) return DBG_ENABLE; else REJECT;}
"disable"        {if(inDebugContext) return DBG_DISABLE; else REJECT;}
"causal"            {if(inDebugContext) return DBG_CAUSAL; else REJECT;}
"mem"               {if(inDebugContext) return DBG_MEM; else REJECT;}
"select"            {if(inDebugContext) return DBG_SELECT; else REJECT;}
"options"           {if(inDebugContext) return DBG_OPTIONS; else REJECT;}
"verbose"           {if(inDebugContext) return DBG_VERBOSE; else REJECT;}
"on"                {if(inDebugContext) return DBG_ON; else REJECT;}
"off"               {if(inDebugContext) return DBG_OFF; else REJECT;}
"termtable"         {if(inDebugContext) return DBG_TERMTABLE; else REJECT;}
"tasks"            {if(inDebugContext) return DBG_TASKS; else REJECT;}
"predicates"        {if(inDebugContext) return DBG_PREDICATES; else REJECT;}
"verbosity"         {if(inDebugContext) return DBG_VERBOSITY; else REJECT;}
"apply"            {if(inDebugContext) return DBG_APPLY; else REJECT;}

{Number}                        {
                                 yylval.type_number = (double) strtod(yytext,NULL);
                                 return PDDL_DNUMBER;}
{Float}                {
                                 yylval.type_number = (double) strtod(yytext,NULL);
                                 return PDDL_NUMBER;}

{Name}                          {
                                 buffer = yytext;
                                 yylval.type_string = buffer.c_str();
                                 return PDDL_NAME;}
{VarName}                       {
                                 buffer = yytext;
                                 yylval.type_string = buffer.c_str();
                                 return PDDL_VAR;}
{StrPath}           {
                     if(inDebugContext) {
            buffer = yytext;
                        yylval.type_string = buffer.c_str();
                        return DBG_PATH;
            }
            else REJECT;
                    }
"["                             {return LEFTBRAC;}
"]"                             {return RIGHTBRAC;}
"!"                {return EXCLAMATION;}
"-"                             {return PDDL_HYPHEN;}
{Blanks}                        {}
.                               {
                                 buffer = "Not a valid character: `";
                                 buffer += yytext;
                                 buffer += "'";
                                 lexer->LexerError(buffer.c_str());
                                }

<<EOF>>           {
            if(!lexer->restore())
            yyterminate();
        }
%%
