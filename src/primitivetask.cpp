#include "primitivetask.hh"
#include <math.h>
#include "state.hh"
#include "papi.hh"
#include "problem.hh"
#include "domain.hh"
#include "undoARLiteralState.hh"
#include "undoChangeProducer.hh"
#include "function.hh"
#include "plan.hh"
#include "causalTable.hh"
#include "textTag.hh"
#include "unifier.hh"

extern int TU_Transform[6];

//Variable para controlar la impresion
extern bool PRINTING_EFFECT;
extern bool PRINTING_DURATIONS;
extern bool PRINTING_CONDITIONS;

extern string str2XML(string s);
extern vector<pair<time_t,time_t> > * schedule;


    PrimitiveTask::PrimitiveTask(int id, int mid)
:Task(id,mid), tpoints(0,0)
{
    precondition = 0;
    effect = 0;
    inlinet=0;
    tconstraints = 0;
    contextIndex = -1;
};

    PrimitiveTask::PrimitiveTask(int id, int mid, const KeyList * kl)
:Task(id,mid,kl), tpoints(0,0)
{
    precondition = 0;
    effect = 0;
    inlinet=0;
    tconstraints = 0;
    contextIndex = -1;
};

    PrimitiveTask::PrimitiveTask(const PrimitiveTask * o)
:Task(o->getId(),o->getMetaId(),o->getParameters()), tpoints(o->tpoints)
{
    if(o->precondition)
        precondition = (Goal *) o->precondition->clone();
    else
        precondition = 0;
    if(o->effect)
        effect = (Effect *) o->effect->clone();
    else
        effect = 0;
    if(o->tconstraints){
        this->tconstraints = new vector<TCTR>();
        for_each(o->tconstraints->begin(),o->tconstraints->end(),CloneVCT(this->tconstraints));
    }
    else
        tconstraints = 0;

    inlinet=o->inlinet;
    contextIndex = -1;
};

    PrimitiveTask::~PrimitiveTask() {
        if(precondition)
            delete precondition;
        if(effect)
            delete effect;
        if(tconstraints)
            for_each(tconstraints->begin(),tconstraints->end(),DeleteVCT());
    };

void PrimitiveTask::print(ostream * os, int indent) const {
    bool PRINT_NUMBERTYPE2, PRINT_OBJECTTYPE2, PRINT_DEFINEDTYPES2;
    string s(indent,' ');

    if(getInline()){
        // El c�digo comentado lo he cambiado ya que da errores al escribir el dominio poniendo !inline
        // donde tendr�a que poner inline
        /*            if(isInline())
         *os << s << "(:!inline " << endl;
         else
         *os << s << "(:inline " << endl;*/
        if(isInline())
            *os << s << "(:inline";
        if(precondition){
            *os << endl;
            Print<Goal>(os,indent+NINDENT)(precondition);
        }
        else
            *os << " ()";

        if(effect){
            *os << endl;
            Print<Effect>(os,indent+NINDENT)(effect);
            *os << s << ") " << endl;
        }
        else{
            if(precondition)
                *os << s << "())" << endl;
            else
                *os << " ())" << endl;
        }
    }
    else {
        //Guardamos los valores antiguos
        PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
        PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
        PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;

        //Fijamos los nuevos valores
        PRINT_NUMBERTYPE = true;
        PRINT_OBJECTTYPE = true;
        PRINT_DEFINEDTYPES = true;

        if(isDurative())
            *os << s << "(:durative-action " << getName();
        else
            *os << s << "(:action " << getName();
        *os <<  endl << s << " :parameters (";
        for_each(parameters.begin(),parameters.end(), PrintKey(os));
        *os << ")";
        //Recuperamos los valores anteriores
        PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
        PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;
        PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;

        if(isDurative()){
            *os << endl << s <<  " :duration";
            for_each(tconstraints->begin(),tconstraints->end(),PrintCT(os,1));
        }

        PRINTING_CONDITIONS = true;
        //Guardamos los valores antiguos
        PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
        PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
        PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;

        //Fijamos los nuevos valores
        PRINT_NUMBERTYPE = false;
        PRINT_OBJECTTYPE = false;
        PRINT_DEFINEDTYPES = false;

        if(isDurative())
            *os << endl << s << " :condition";
        else
            *os << endl << s << " :precondition";
        if(precondition){
            *os << endl;
            Print<Goal>(os,indent+NINDENT)(precondition);
        }
        else
            *os << " ()" << endl;
        PRINTING_CONDITIONS = false;

        PRINTING_EFFECT = true;
        *os << s << " :effect";
        if(effect){
            *os << endl;
            Print<Effect>(os,indent+NINDENT)(effect);
            *os << s << ")" << endl;
        }
        else{
            if(precondition)
                *os << s <<  "())" << endl;
            else
                *os << " ())" << endl;
        }
        PRINTING_EFFECT = false;

        //Recuperamos los valores anteriores
        PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
        PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;
        PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;
    }
}

void PrimitiveTask::toxml(XmlWriter * writer, bool complete) const
{
    writer->startTag("primitive")
        ->addAttrib("name",getName())
        ->addAttrib("id",getId());


    if(current_plan && current_plan->hasPlan()){
        const TaskNetwork * tn = current_plan->getTaskNetwork();
        pair<time_t,time_t> i1, i2;
        char s[128];

        writer->addAttrib("indx",tn->getIndexOf(this));
        if(current_plan->getDomain()->isTimed()){
            i1 = current_plan->getTimeRef(tpoints.first);
            i2 = current_plan->getTimeRef(tpoints.second);

            if(i1.first != 0){
                strftime(s,127,current_plan->XML_TFORMAT.c_str(),localtime(&(i1.first)));
                writer->addAttrib("start",s);}

                if(i1.second != 0){
                    strftime(s,127,current_plan->XML_TFORMAT.c_str(),localtime(&(i1.second)));
                    writer->addAttrib("latest_start",s);}

                    if(i2.first != 0){
                        strftime(s,127,current_plan->XML_TFORMAT.c_str(),localtime(&(i2.first)));
                        writer->addAttrib("end",s);}

                        if(i2.second != 0){
                            strftime(s,127,current_plan->XML_TFORMAT.c_str(),localtime(&(i2.second)));
                            writer->addAttrib("latest_end",s);}

                            if(i1.first != 0 && i2.first != 0) {
                                float div = TU_Transform[current_plan->getProblem()->FLAG_TIME_UNIT];
                                float duration = (i2.first - i1.first)/div;
                                writer->addAttrib("duration",duration);
                            }


                            writer->addAttrib("start_point",(int)tpoints.first);
                            writer->addAttrib("end_point",(int)tpoints.second);
        }

        string output,value;
        tagv_cite tb, te = parser_api->domain->metainfo[getMetaId()]->getEndTags();
        for(tb = parser_api->domain->metainfo[getMetaId()]->getBeginTags(); tb!= te; tb++){
            writer->startTag("meta");
            writer->addAttrib("name",(*tb)->getName());
            value = ((TextTag *)(*tb))->getValue();
            if(current_plan->getDomain()->isTimed())
                output = replaceVars(value,&tpoints,current_plan->getSchedule());
            else
                output = replaceVars(value,0,0);
            writer->addCharacter(output);
            writer->endTag();
        }

        // obtener la definici�n de la tarea del dominio, para sacar los par�metros sin instanciar
        const Task * def = current_plan->getDomain()->getTaskMetaID(getId(),getMetaId());

        writer->startTag("parameters");
        // imprimir los par�metros
        pkey parameter,def_parameter;
        string name;
        ToXMLKey w(writer);
        for(int i=0; i<sizep();i++){
            parameter = getParameter(i);
            def_parameter = def->getParameter(i);
            name = parser_api->termtable->getTerm(def_parameter)->getName();
            name = name.substr(1);
            writer->startTag("parameter")
                ->addAttrib("pos",i)
                ->addAttrib("name",name);
            w(parameter);
            writer->endTag();
        }
        writer->endTag();

        // A�ado las tareas predecesoras y sucesoras
        const Task * t;
        // Predecesoras desde la red de tareas
        int indx = tn->getIndexOf(this);
        // En realidad buscamos los predecesores y los sucesores de la tarea indx+1 ya que al index que tenemos
        // hay que sumarle uno porque en la red de tareas se mete tarea como nodo inicial ficticia
        indx++;
        intvit b, e;
        e = tn->getPredEnd(indx);
        for(b=tn->getPredBegin(indx);b!=e;b++) {
            // Al index hay que restarle uno porque la red de tareas mete una tarea como nodo inicial ficticia que no existe
            if(tn->getNumOfNodes() > (*b) && (*b)>0) {
                int indx_tmp = tn->getIndexOf(tn->getTask(*b));
                indx_tmp--;
                writer->startTag("dependence")
                    ->addAttrib("type","ordering")
                    ->addAttrib("indx",indx_tmp)
                    ->addAttrib("id",tn->getTask(*b)->getId())
                    ->addAttrib("name",tn->getTask(*b)->getName())
                    ->endTag();
            }
        }


        // Predecesoras desde los vinculos causales
        const CLTable * ct;
        cltcite bt, et;

        ct = causalTable.getConsumedLinks(this);
        if(ct) {
            et = ct->end();
            for(bt=ct->begin();bt!=et;bt++) {
                t = (*bt)->getProducer();
                if(t) {
                    if(t->isPrimitiveTask()){
                        int indx_tmp = tn->getIndexOf(t);
                        writer->startTag("dependence")
                            ->addAttrib("type","causal_link")
                            ->addAttrib("indx",indx_tmp)
                            ->addAttrib("id",t->getId())
                            ->addAttrib("name",t->getName())
                            ->endTag();
                    }
                }
            }
        }
        // Obtener un listado de los efectos producidos por esta acc�n
        if(contextIndex != -1){
            writer->startTag("effect");
            const StackNode * context = current_plan->getContext(contextIndex);
            undocit i,e = context->stateChanges.end();
            for(i = context->stateChanges.begin(); i != e; i++)
                (*i)->toxml(writer);
            writer->endTag();
        }
    }
    else{
        writer->startTag("parameters");
        for_each(parameters.begin(),parameters.end(), ToXMLKey(writer));
        writer->endTag();

        writer->startTag("precondition");
        if(precondition){
            ToXML<Goal,XmlWriter> tx(writer);
            tx(precondition);
        }
        writer->endTag();

        writer->startTag("effect");
        if(effect){
            ToXML<Effect,XmlWriter> tx(writer);
            tx(effect);
        }
        writer->endTag();
    }

    writer->endTag();
}

void PrimitiveTask::printHead(ostream * os) const
{
    *os << ":action (" << getName();
    for_each(parameters.begin(),parameters.end(), PrintKey(os));
    *os << ")" << endl;
};

void PrimitiveTask::prettyPrint(ostream * os, pair<unsigned int,unsigned int> tpoints,  const vector<pair<int,int> > * sched) const
{
    int start = (*sched)[tpoints.first].first;
    int end = (*sched)[tpoints.second].first;
    char cad[32];

    // obtener la cadena a procesar
    const Tag * t = parser_api->domain->metainfo[getMetaId()]->getTag("prettyprint");
    if(t == 0){
        *os << ":action (" << getName();
        for_each(parameters.begin(),parameters.end(), PrintKey(os));
        *os << ")";
        if (start >= 0){
            snprintf(cad,31,"%d",start);
            *os << " start: " << cad;
            snprintf(cad,31,"%d",end);
            *os << " end: " << cad;
        }
        return;
    }

    string s = ((TextTag *)t)->getValue();

    *os << replaceVars2(s,&tpoints,sched);
};

void PrimitiveTask::prettyPrintNoDur(ostream * os) const {
    // obtener la cadena a procesar
    const Tag * t = parser_api->domain->metainfo[getMetaId()]->getTag("prettyprint");
    if(t == 0){
        *os << ":action (" << getName();
        for_each(parameters.begin(),parameters.end(), PrintKey(os));
        *os << ")";
        return;
    }

    string s = ((TextTag *)t)->getValue();

    *os << replaceVars(s,0,0);
};

const char * PrimitiveTask::replaceVars(string s, const pair<unsigned int,unsigned int> * tpoints, const vector<pair<time_t,time_t> > * sched) const{
    static string output;
    char cad[128];
    char modificador = 'n';
    bool has_modificador = false;

    time_t plan_start;
    if(!current_plan->getProblem()->FLAG_TIME_START)
        time(&plan_start);
    else
        plan_start = current_plan->getProblem()->FLAG_TIME_START;

    //cout << s << endl;

    output = "";
    string var="";
    string::iterator b, e = s.end();
    bool invar=false;
    int v = 0;
    for(b=s.begin();b!=e;b++){
        if(invar) {
            if(isdigit(*b)){
                var.push_back(*b);
            }
            else {
                // sustituir por el argumento
                v = atoi(var.c_str());
                if(v == (int) parameters.size()+1 && tpoints){
                    time_t start = (*sched)[tpoints->first].first;
                    if (start >= 0){
                        if(has_modificador && modificador == 'i'){
                            snprintf(cad,31,"%d",(int)rint(start/(1.0*TU_Transform[current_plan->getProblem()->FLAG_TIME_UNIT])));
                        }
                        else
                            strftime(cad,127,current_plan->getProblem()->TFORMAT.c_str(),localtime(&start));
                        output = output + cad;
                    }
                }
                else if(v == (int) parameters.size() + 2 && tpoints){
                    time_t end = (*sched)[tpoints->second].first;
                    if (end >= 0){
                        if(has_modificador && modificador == 'i'){
                            snprintf(cad,31,"%d",(int)rint(end/(1.0*TU_Transform[current_plan->getProblem()->FLAG_TIME_UNIT])));
                        }
                        else
                            strftime(cad,127,current_plan->getProblem()->TFORMAT.c_str(),localtime(&end));
                        output = output + cad;
                    }
                }
                else if(v == (int) parameters.size() + 3 && tpoints){
                    time_t start = (*sched)[tpoints->first].first;
                    time_t end = (*sched)[tpoints->second].first;
                    if (start >= 0 && end >= 0){
                        snprintf(cad,31,"%f",(difftime(end,start)/(1.0*TU_Transform[current_plan->getProblem()->FLAG_TIME_UNIT])));
                        output = output + cad;
                    }
                }
                else if(v <= 0 || v > (int) parameters.size())
                    output = output + "$" + var;
                else {
                    if(parameters[v-1].first == -1){
                        if(has_modificador && modificador == 'd'){
                            time_t mtime = plan_start + (int) (parameters[v-1].second*TU_Transform[current_plan->getProblem()->FLAG_TIME_UNIT]);
                            strftime(cad,127,current_plan->getProblem()->TFORMAT.c_str(),localtime(&mtime));
                        }
                        else
                            snprintf(cad,31,"%f",parameters[v-1].second);
                        output = output + cad;
                    }
                    else{
                        //parser_api->termtable->print(&cerr);
                        output = output + parser_api->termtable->getTerm(parameters[v - 1].first)->getName();
                    }
                }
                output.push_back(*b);
                invar = false;
            }
        }
        else if (*b == '$') {
            invar = true;
            var = "";
            has_modificador = false;
            if((b+1) != e && isalpha(*(b+1))){
                has_modificador = true;
                b++;
                modificador = *(b);
            }
        }
        else {
            output.push_back(*b);
        }
    }
    // evitar el problema de que justamente acabes con una variable.
    if(invar) {
        // sustituir por el argumento
        v = atoi(var.c_str());
        if(v == (int) parameters.size()+1 && tpoints){
            time_t start = (*sched)[tpoints->first].first;
            if (start >= 0){
                strftime(cad,127,current_plan->getProblem()->TFORMAT.c_str(),localtime(&start));
                output = output + cad;
            }
        }
        else if(v == (int) parameters.size() + 2 && tpoints){
            time_t end = (*sched)[tpoints->second].first;
            if (end >= 0){
                strftime(cad,127,current_plan->getProblem()->TFORMAT.c_str(),localtime(&end));
                output = output + cad;
            }
        }
        else if(v == (int) parameters.size() + 3 && tpoints){
            time_t start = (*sched)[tpoints->first].first;
            time_t end = (*sched)[tpoints->second].first;
            if (start >= 0 && end >= 0){
                snprintf(cad,31,"%f",difftime(end,start));
                output = output + cad;
            }
        }
        else if(v <= 0 || v > (int) parameters.size())
            output = output + "$" + var;
        else {
            if(parameters[v-1].first == -1){
                snprintf(cad,31,"%f",parameters[v-1].second);
                output = output + cad;
            }
            else{
                //parser_api->termtable->print(&cerr);
                output = output + parser_api->termtable->getTerm(parameters[v - 1].first)->getName();
            }
        }
    }

    // sustituci�n de las variables
    Unifier * u = getUnifier();
    if (u){

        // hay que llevar las sustituciones en dos cadenas de forma simult�nea
        // para controlar que el planner no es case-sensitive
        string upper_output;
        unsigned int len;
        unsigned int index;
        const char * sname;
        const char * varname;
        char upper_var[124] = "";
        char buffer[128] = "";
        char dbuffer[128] = "";
        char var_buffer[128] = "";

        string::iterator si, send = output.end();
        for (si = output.begin(); si!=send; si++)
            upper_output.push_back(toupper(*si));


        //cerr << upper_output << endl;
        subsit upos, uend = u->end();
        for(upos=u->begin();upos!=uend;upos++){
            if(upos->second.first == -1){

                time_t mtime = plan_start + (int) (upos->second.second*TU_Transform[current_plan->getProblem()->FLAG_TIME_UNIT]);
                strftime(dbuffer,127,current_plan->getProblem()->TFORMAT.c_str(),localtime(&mtime));
                snprintf(buffer,31,"%f",upos->second.second);

                // capturar el nombre de la variable
                varname = parser_api->termtable->getTerm(upos->first)->getName();

                snprintf(upper_var,123,"%s",varname);
                len = strlen(upper_var);
                for(unsigned int i=0; i<len; i++) {
                    upper_var[i] = toupper(upper_var[i]);
                }

                strncpy(var_buffer,upper_var,123);
                //cerr << var_buffer << endl;

                index = 0;
                len = strlen(var_buffer);
                index = upper_output.find(var_buffer,index);
                while (index < output.size()){
                    output.replace(index,len,buffer);
                    upper_output.replace(index,len,buffer);
                    index = upper_output.find(var_buffer,index);
                }

                snprintf(var_buffer,127,"$%%I%s",upper_var);
                //cerr << var_buffer << endl;
                index = 0;
                len = len + 2;
                index = upper_output.find(var_buffer,index);
                while (index < output.size()){
                    output.replace(index,len,buffer);
                    upper_output.replace(index,len,buffer);
                    index = upper_output.find(var_buffer,index);
                }

                var_buffer[2] = 'D';
                //cerr << var_buffer << endl;
                index = 0;
                index = upper_output.find(var_buffer,index);
                while (index < output.size()){
                    output.replace(index,len,dbuffer);
                    upper_output.replace(index,len,dbuffer);
                    index = upper_output.find(var_buffer,index);
                }
            }
            else{
                varname = parser_api->termtable->getTerm(upos->first)->getName();
                // capturar el nombre de la variable
                sname = parser_api->termtable->getTerm(upos->second.first)->getName();
                //cerr << varname << endl;

                snprintf(var_buffer,127,"%s",varname);
                len = strlen(var_buffer);
                for(unsigned int i=0; i<len; i++)
                    var_buffer[i] = toupper(var_buffer[i]);
                //cerr << var_buffer << endl;
                index = 0;
                index = upper_output.find(var_buffer,index);
                while (index < output.size()){
                    output.replace(index,len,sname);
                    upper_output.replace(index,len,sname);
                    index = upper_output.find(var_buffer,index);
                }
            }
        }
    }
    return output.c_str();
}

const char * PrimitiveTask::replaceVars2(string s, const pair<unsigned int,unsigned int> * tpoints, const vector<pair<int,int> > * sched) const{
    int start = (*sched)[tpoints->first].first;
    int end = (*sched)[tpoints->second].first;
    char cad[32];

    string var="";
    static string output;
    string::iterator b, e = s.end();
    bool invar=false;
    int v = 0;

    output ="";
    for(b=s.begin();b!=e;b++){
        if(invar) {
            if(isdigit(*b)){
                var.push_back(*b);
            }
            else {
                // sustituir por el argumento
                v = atoi(var.c_str());
                if(v == (int)parameters.size()+1){
                    if (start >= 0){
                        snprintf(cad,31,"%d",start);
                        output = output + cad;
                    }
                }
                else if(v == (int)parameters.size() + 2){
                    if (end >= 0){
                        snprintf(cad,31,"%d",end);
                        output = output + cad;
                    }
                }
                else if(v == (int)parameters.size() + 3){
                    int dur = end - start;
                    if (dur >= 0){
                        snprintf(cad,31,"%d",(int)rint(dur/(1.0*TU_Transform[current_plan->getProblem()->FLAG_TIME_UNIT])));
                        output = output + cad;
                    }
                }
                else if(v <= 0 || v > (int) parameters.size())
                    output = output + "$" + var;
                else {
                    if(parameters[v-1].first == -1){
                        snprintf(cad,31,"%f",parameters[v-1].second);
                        output = output + cad;
                    }
                    else{
                        //parser_api->termtable->print(&cerr);
                        output = output + parser_api->termtable->getTerm(parameters[v - 1].first)->getName();
                    }
                }
                output.push_back(*b);
                invar = false;
            }
        }
        else if (*b == '$') {
            invar = true;
            var = "";
        }
        else {
            output.push_back(*b);
        }
    }
    // evitar el problema de que justamente acabes con una variable.
    if(invar) {
        // sustituir por el argumento
        v = atoi(var.c_str());
        if(v == (int)parameters.size()+1){
            if (start >= 0){
                snprintf(cad,31,"%d",start);
                output = output + cad;
            }
        }
        else if(v == (int)parameters.size() + 2){
            if (end >= 0){
                snprintf(cad,31,"%d",end);
                output = output + cad;
            }
        }
        else if(v == (int)parameters.size() + 3){
            int dur = end - start;
            if (dur >= 0){
                snprintf(cad,31,"%d",(int)rint(dur/(1.0*TU_Transform[current_plan->getProblem()->FLAG_TIME_UNIT])));
                output = output + cad;
            }
        }
        else if(v <= 0 || v > (int) parameters.size())
            output = output + "$" + var;
        else {
            if(parameters[v-1].first == -1){
                snprintf(cad,31,"%f",parameters[v-1].second);
                output = output + cad;
            }
            else{
                //parser_api->termtable->print(&cerr);
                output = output + parser_api->termtable->getTerm(parameters[v - 1].first)->getName();
            }
        }
    }
    return output.c_str();
}

void PrimitiveTask::prettyPrint(ostream * os, pair<unsigned int,unsigned int> tpoints, const vector<pair<time_t,time_t> > * sched ) const
{
    time_t start = (*sched)[tpoints.first].first;
    time_t end = (*sched)[tpoints.second].first;

    //time_t lstart = (*sched)[tpoints.first].second;
    //time_t lend = (*sched)[tpoints.second].second;

    //if(start == 0){
    //    output = output + "[inf,";
    //}
    //else{
    //    strftime(cad,127,current_plan->getProblem()->TFORMAT.c_str(),localtime(&start));
    //    output = output + "[" + cad + ",";
    //}

    //if(end == 0){
    //    output = output + "inf] ";
    //}
    //else{
    //    strftime(cad,127,current_plan->getProblem()->TFORMAT.c_str(),localtime(&end));
    //    output = output + cad + "] ";
    //}

    //if(lstart == 0){
    //    output = output + "[inf,";
    //}
    //else{
    //    strftime(cad,127,current_plan->getProblem()->TFORMAT.c_str(),localtime(&lstart));
    //    output = output + "[" + cad + ",";
    //}

    //if(lend == 0){
    //    output = output + "inf] ";
    //}
    //else{
    //    strftime(cad,127,current_plan->getProblem()->TFORMAT.c_str(),localtime(&lend));
    //    output = output + cad + "] ";
    //}

    // obtener la cadena a procesar
    const Tag * t = parser_api->domain->metainfo[getMetaId()]->getTag("prettyprint");
    char cad[128];
    if(t == 0){
        *os << ":action (" << getName();
        for_each(parameters.begin(),parameters.end(), PrintKey(os));
        *os << ")";
        if (start >= 0){
            strftime(cad,127,current_plan->getProblem()->TFORMAT.c_str(),localtime(&start));
            *os << " start: " << cad;
            strftime(cad,127,current_plan->getProblem()->TFORMAT.c_str(),localtime(&end));
            *os << " end: " << cad;
        }
        return;
    }

    string s = ((TextTag *)t)->getValue();
    *os << replaceVars(s,&tpoints,sched);
};

Expression * PrimitiveTask::clone(void) const
{
    return new PrimitiveTask(this);
}

bool PrimitiveTask::applyEffects(State * sta, VUndo * undo, Unifier * uf)
{
    bool v = true;
    if(effect)
        v = effect->apply(sta,undo,uf);
    contextIndex = current_plan->getContextIndex();
    commitEffects(sta,undo,this);
    return v;
}

void PrimitiveTask::commitEffects(State * sta, VUndo * undo, const PrimitiveTask * producer)
{
    // los efectos no insertan nada en el estado s�lo eliminan, esto es
    // as� porque he detectado errores cuando la inserci�n se realiza
    // antes del borrado. Se pueden borrar cosas que acabas de insertar
    // Sup�n por ejemplo el siguiente efecto
    // (and (literal ?a) (not literal ?b))
    // si ?a y ?b han unificado a la misma constante llamese c,
    // si la inserci�n se hace antes del borrado (lo que ocurr�a con el
    // antiguo algoritmo recursivo que resolv�a los efectos por orden)
    // puedes eliminar (literal c) lo que no esperas en la mayor�a
    // de los casos cuando escribes el dominio.
    ISTable_mrange range;
    Literal * l;
    isit j;

    if(!current_plan->getDomain()->isTimed()){
        undoit i,e = undo->end();
        for(i = undo->begin(); i != e; i++) {
            if((*i)->isUndoARLiteralState()) {
                if(((UndoARLiteralState *) *i)->wasAdded()) {
                    l = ((UndoARLiteralState *) *i)->getLiteral();
                    range = sta->getModificableRange(l->getId());
                    j=find_if(range.first,range.second,EqualLit(l->getParameters()));
                    /* El literal no est� en el estado,  lo a�ado */
                    while((j=find_if(range.first,range.second,EqualLit(l->getParameters()))) != range.second){
                        if((*j).second->isLEffect()){
                            if(((LiteralEffect *)(*j).second)->isMaintain())
                                ((LiteralEffect *)l)->setMaintain();
                            sta->getLiteralTable()->erase(j);
                        }
                        range = sta->getModificableRange(l->getId());
                    }
                    l->setProducer(producer);
                    sta->addLiteral(l);
                }
            }
        }
    }
    else
    {
        VUndo undos2Add;

        undoit i,e = undo->begin() + undo->size();
        for(i = undo->begin(); i != e; i++) {
            if((*i)->isUndoARLiteralState()) {
                l = ((UndoARLiteralState *) *i)->getLiteral();
                if(((UndoARLiteralState *) *i)->wasAdded()) {
                    range = sta->getModificableRange(l->getId());
                    while((j=find_if(range.first,range.second,EqualLit(l->getParameters()))) != range.second){
                        if((*j).second->isLEffect()){
                            if(((LiteralEffect *)(*j).second)->isMaintain())
                                ((LiteralEffect *)l)->setMaintain();
                            sta->getLiteralTable()->erase(j);
                        }
                        range = sta->getModificableRange(l->getId());
                    }
                    l->setProducer(producer);
                    sta->addLiteral(l);
                }
            }
            else if((*i)->isUndoChangeValue()){
                l = (Function *) ((UndoChangeValue *) *i)->getTarget();
                // necesito registrar los cambios de productor de las funciones
                // para posteriormente deshacerlos. Al contrario que los literales
                // que se generan/borran cada vez que se introducen en el estado, las
                // funciones (fluents) permanecen durante toda la ejecuci�n, por eso
                // es importante restaurarlas completamente a sus estado original tras
                // un backtracking.
                undos2Add.push_back(new UndoChangeProducer(l,l->getProducer()));
                l->setProducer(producer);
            }
        }

        int s = undo->size();
        for(int k=0; k<s; k++) {
            e = undo->begin() + k;
            if((*e)->isUndoARLiteralState()) {
                l = ((UndoARLiteralState *) *e)->getLiteral();
                if(!((UndoARLiteralState *) *e)->wasAdded()) {
                    range = sta->getModificableRange(l->getId());
                    // si no se ha a�adido nada que anule la negaci�n del literal
                    if((j=find_if(range.first,range.second,EqualLit(l->getParameters()))) == range.second){
                        LiteralEffect * notl = (LiteralEffect *) ((LiteralEffect *)l)->clone();
                        notl->setPolarity(false);
                        undo->push_back(new UndoARLiteralState(notl,true));
                        notl->setProducer(producer);
                        sta->addLiteral(notl);
                    }
                }
            }
        }
        undo->insert(undo->end(),undos2Add.begin(),undos2Add.end());
    }
}


pkey PrimitiveTask::getTermId(const char * name) const
{
    keylistcit i = searchTermName(name);
    if(i != parameters.end())
        return (*i);

    pkey result(-1,-1);
    if(precondition){
        result = precondition->getTermId(name);
        if(result.first != -1)
            return result;
    }

    if(effect){
        result = effect->getTermId(name);
        if(result.first != -1)
            return result;
    }

    if(tconstraints) {
        vector<TCTR>::const_iterator b = tconstraints->begin(), e = tconstraints->end();
        while(b != e) {
            result = b->second->compGetTermId(name);
            if(result.first != -1)
                return result;
            b++;
        }
    }

    return result;
}

bool PrimitiveTask::hasTerm(int id) const
{
    keylistcit i = searchTermId(id);
    if(i != parameters.end())
        return true;

    if(precondition)
        if(precondition->hasTerm(id))
            return true;

    if(effect)
        if(effect->hasTerm(id))
            return true;

    if(tconstraints) {
        vector<TCTR>::const_iterator b = tconstraints->begin(), e = tconstraints->end();
        while(b != e) {
            if(b->second->compHasTerm(id))
                return true;
            b++;
        }
    }

    return false;
}

void PrimitiveTask::renameVars(Unifier * u, VUndo * undo) {
    varRenaming(u,undo);

    if(precondition)
        precondition->renameVars(u,undo);

    if(effect)
        effect->renameVars(u,undo);

    if(tconstraints)
        for_each(tconstraints->begin(),tconstraints->end(),RVCT(u,undo));
}

bool PrimitiveTask::isReachable(ostream * err) const
{
    if(precondition) {
        if(!precondition->isReachable(err,true)){
            *err << "In action: " << getName() << " defined near or in: ";
            *err << "[" << parser_api->files[parser_api->domain->metainfo[getMetaId()]->fileid] << "]";
            *err << ":" << parser_api->domain->metainfo[getMetaId()]->linenumber << endl;
            return false;
        }
        else
            return true;
    }
    return true;
};

bool PrimitiveTask::provides(const Literal * l) const
{
    if(effect)
        return effect->provides(l);
    return false;
};


EvaluatedTCs * PrimitiveTask::evalDurationExp(const State * state, const Unifier * context) const {
    vector<TCTR>::const_iterator b = tconstraints->begin(), e = tconstraints->end();
    EvaluatedTCs * evaluated = new EvaluatedTCs();
    pkey ret;
    while(b != e) {
        ret = b->second->eval(state,context);
        if(ret.first == INT_MAX){
            delete evaluated;
            return 0;
        }
        evaluated->push_back(make_pair(b->first,ret.second));
        b++;
    }
    return evaluated;
};


