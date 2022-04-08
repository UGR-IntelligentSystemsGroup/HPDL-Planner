#include "literal.hh"
#include "goal.hh"
#include <sstream>
#include "state.hh"
#include "constants.hh"
#include "domain.hh"
#include "papi.hh"
#include "problem.hh"
#include "literaleffect.hh"
#include "literalgoal.hh"
#include "plan.hh"

int mCounter=0;

    LiteralGoal::LiteralGoal(int id, int mid, bool p)
:Goal(), Literal(id,mid)
{
    polarity = p;
}

    LiteralGoal::LiteralGoal(int id, int mid, const KeyList * param, bool p)
:Goal(), Literal(id,mid,param)
{
    polarity = p;
}

    LiteralGoal::LiteralGoal(const LiteralGoal * other) 
:Goal(other), Literal(other)
{
    polarity = other->polarity;
};

Expression *LiteralGoal::clone(void) const
{
    return new LiteralGoal(this);
}

void LiteralGoal::print(ostream * os, int indent) const
{
    string s(indent,' ');

    *os << s;
    if(isTimed()){
        *os << "(";
        time->print(os,0);
        *os << " ";
    }
    if(!polarity)
        *os << "(not ";
    headerPrint(os);
    if(!getPolarity())
        *os << ")";
    if(isTimed())
        *os << ")";
}

void LiteralGoal::toxml(XmlWriter * writer) const{
    writer->startTag("predicate")
        ->addAttrib("name",getName());

    if(!polarity)
        writer->addAttrib("polarity","negated");
    else 
        writer->addAttrib("polarity","affirmed");

    if(isTimed())
        time->toxml(writer);

    for_each(parameters.begin(),parameters.end(),ToXMLKey(writer));

    writer->endTag();
};

UnifierTable * LiteralGoal::getUnifiers(const State * state, const Unifier * context, bool inheritPolarity, Protection * protection) const {
    // literales del estado que coinciden con mi nombre
    ISTable_range  r = state->getRange(getId());
    // axiomas del dominio que coinciden con mi nombre
    AxiomTableRange ra = parser_api->domain->getAxiomRange(getId());

    UnifierTable * ut = 0, * ut2=0;
    Unifier * u = 0, * aux=0;
    iscit i, e;
    axiomtablecit ba, ea;

    bool pol = true;
    // determinar si el predicado est� negado
    if((!getPolarity() || !inheritPolarity) && (!(!getPolarity() && !inheritPolarity)))
        pol = false;

    Protection  p;
    if(time) {
        p.first = time->evalStart(context,state);
        p.second = time->evalEnd(context,state);
        protection = &p;
    }

    // Esta afirmado
    if(pol) {
        //print(&cerr);
        //cerr << endl;
        ut = new UnifierTable();
        e = r.second;
        for(i = r.first; i != e; i++) {
            if((*i).second->getPol()){
                //(*i).second->printL(&cerr);
                //cerr << endl;
                if(!u) {
                    if(context)
                        u = context->clone();
                    else
                        u = new Unifier();
                }
                if(unify(getParameters(),(*i).second->getParameters(),u)) {
                    //cerr << "OK!!" << endl;

                    // registrar el v�nculo causal
                    u->createCLTable();
                    Causal * c = new Causal((LiteralEffect *)(*i).second,protection);
                    if(c->isNormalLink())
                        c->setTime(((LiteralEffect *)(*i).second)->evalTime());
                    u->addCL(c);

                    ut->addUnifier(u);
                    mCounter++;
                    u = 0;
                }
                //else {
                //    cerr << "FAIL!" << endl;
                //}
            }
        }
        if(u){
            delete u;
            u=0;
        }
        // recorremos los distintos axiomas del dominio
        ea = ra.second;
        for(ba = ra.first; ba!= ea; ba++){
            // tenemos una unificaci�n con uno de los axiomas.
            // Testeamos su veracidad
            if(unify(getParameters(),(*ba).second->getParameters())){
                // forzamos la sustituci�n de las variables libres,
                // del axioma
                // por las del literal
                bool valid = true;
                u = new Unifier();
                const KeyList * par = (*ba).second->getParameters();
                int s = (int) parameters.size();
                pkey pk, pk2;
                for(int i =0; i< s && valid; i++){
                    if(parser_api->termtable->isConstant(parameters[i]) || parser_api->termtable->isNumber(parameters[i])){
                        if(u->getSubstitution(((*par)[i]).first,&pk2)){
                            if(parameters[i].first != pk2.first || parameters[i].second != pk2.second){
                                valid = false;
                            }
                        }
                        else
                            u->addSubstitution(((*par)[i]).first,parameters[i]);
                    }
                    else if(context) {
                        if(context->getSubstitution(parameters[i].first,&pk)) {
                            if(u->getSubstitution(((*par)[i]).first,&pk2)){
                                if(pk.first != pk2.first || pk2.second != pk.second){
                                    valid = false;
                                }
                            }
                            else {
                                u->addSubstitution(((*par)[i]).first,pk);
                            }
                        }
                    }
                }
                if(valid){
                    ut2 = (((*ba).second)->test(state,u,true,protection));
                    // a�adimos las unificaciones que se han producido
                    // si las hay.
                    if(ut2) {
                        unifierit ite, itend;
                        itend = ut2->getUnifierEnd();
                        for(ite = ut2->getUnifierBegin(); ite != itend; ite++) {
                            /*cerr << "---------------------" << endl;
                              (*ite)->print(&cerr,0);
                              cerr << "---------------------" << endl;*/
                            if(context) {
                                aux = context->clone();
                            }
                            else {
                                aux = new Unifier();
                            }
                            for(int i =0; i< (int)parameters.size(); i++){
                                if(parser_api->termtable->isVariable(parameters[i])){
                                    pkey pk;
                                    if(!aux->getSubstitution(parameters[i].first,&pk)){
                                        if((*ite)->getSubstitution((*par)[i].first,&pk)) 
                                            aux->addSubstitution(parameters[i].first,pk);
                                    }
                                }
                            }
                            aux->addCLTable((*ite));
                            ut->addUnifier(aux);
                        }
                        delete ut2;
                        ut2=0;
                    }
                    delete u;
                    u=0;
                }
                else {
                    delete u;
                    u=0;
                }
            }
        }
    }
    // Esta negado
    else {
        ut = new UnifierTable();

        if(context)
            aux = context->clone();
        else
            aux = new Unifier();

        e = r.second;
        for(i = r.first; i != e; i++){
            if(!u) {
                if(context) {
                    u = context->clone();
                }
                else {
                    u = new Unifier();
                }
            }
            if(unify(getParameters(),(*i).second->getParameters(),u)) {
                delete u;
                u = 0;
                if(!(*i).second->getPol())
                {
                    aux->createCLTable();
                    // Se a�ade un v�nculo causal con el literal negado
                    Causal * c = new Causal((LiteralEffect *)(*i).second,protection);
                    if(c->isNormalLink())
                        c->setTime(((LiteralEffect *)(*i).second)->evalTime());
                    aux->addCL(c);
                    mCounter++;
                }
                else {
                    // si hay algo que unifique no negado
                    // fallamos
                    delete aux;
                    delete ut;
                    return 0;
                }
            }
        }
        if(u){
            delete u;
            u=0;
        }

        // recorremos los distintos axiomas del dominio
        ea = ra.second;
        for(ba = ra.first; ba!= ea; ba++){
            // tenemos una unificaci�n con uno de los axiomas.
            // Testeamos su veracidad
            bool unificaciones = false;
            if(unify(getParameters(),(*ba).second->getParameters())){
                // forzamos la sustituci�n de las variables libres,
                // del axioma
                // por las del literal
                unificaciones = true;
                u = new Unifier();
                int s = (int)parameters.size();
                const KeyList * par = (*ba).second->getParameters();
                bool valid = true;
                pkey pk,pk2;
                for(int i =0; i<s && valid; i++){
                    if(parser_api->termtable->isConstant(parameters[i]) || parser_api->termtable->isNumber(parameters[i])){
                        if(u->getSubstitution(((*par)[i]).first,&pk2)){
                            if(parameters[i].first != pk2.first || pk2.second != parameters[i].second) {
                                valid = false;
                            }
                        }
                        else {
                            u->addSubstitution(((*par)[i]).first,parameters[i]);
                        }
                    }
                    else if(context) {
                        if(context->getSubstitution(parameters[i].first,&pk)) { 
                            if(u->getSubstitution(((*par)[i]).first,&pk2)){
                                if(pk.first != pk2.first || pk2.second != pk.second) {
                                    valid = false;
                                }
                            }
                            else {
                                u->addSubstitution(((*par)[i]).first,pk);
                            }
                        }
                    }
                }
                if(valid) {
                    // testamos la veracidad del axioma
                    ut2 = (((*ba).second)->test(state,u,true,protection));
                    if(ut2) {
                        // hay unificaciones, la negaci�n es falsa
                        delete u;
                        u = 0;
                        delete ut2;
                        ut2=0;
                        return 0;
                    }
                    else{
                        delete u;
                        u=0;
                    }
                }
                else {
                    // no hay unificaci�n con el axioma porque los par�metros no coinciden
                    delete u;
                    u=0;
                }
            }
            if (unificaciones){
                // no hay unificaci�n la negaci�n por tanto es cierta.
                if(u) {
                    delete u;
                }
                if(context) {
                    aux = context->clone();
                }
                else {
                    aux = new Unifier();
                }
                ut->addUnifier(aux);
                return ut;
            }
        }

        if(u){
            delete u;
            u=0;
        }
        if(!aux->getCLTable() || aux->getCLTable()->empty()){
            // Se ha hecho una unificaci�n con un predicado negado del estado
            // inicial, ese predicado no existe, por lo tanto no puedo hacer
            // un v�nculo causal con �l. Para solucionarlo a�ado en tiempo de
            // ejecuci�n un nuevo literal negado al estado inicial.
            // Pero solo sino tiene variables libres
            // ESTO ES UN EFECTO COLATERAL !!
            if(!hasVariables()){
                LiteralEffect * le = new LiteralEffect(getId(),getMetaId(),getParameters(),false);
                current_plan->addToState(le);
                aux->createCLTable();
                Causal * c = new Causal(le,protection);
                if(c->isNormalLink())
                    c->setTime(-1);
                aux->addCL(c);
            }
        }
        ut->addUnifier(aux);


    }

    return ut; 
}

pkey LiteralGoal::getTermId(const char * name) const
{
    pkey ret;
    ret = getTermIdL(name);
    if(ret.first == -1 && isTimed())
        return time->getTermId(name);
    return ret;

}

bool LiteralGoal::hasTerm(int id) const
{
    if(searchTermId(id) != parameters.end())
        return true;
    if(isTimed() && time->hasTerm(id))
        return true;
    return false;
}

void LiteralGoal::renameVars(Unifier * u, VUndo * undo)
{
    varRenaming(u,undo);
    if(isTimed())
        time->renameVars(u,undo);
}

bool LiteralGoal::isReachable(ostream * err, bool inheritPolarity) const
{
    // literales del estado que coinciden con mi nombre
    ISTable_range  r = parser_api->getProblem()->getInitialState()->getRange(getId());
    // axiomas del dominio que coinciden con mi nombre
    AxiomTableRange ra = parser_api->domain->getAxiomRange(getId());

    iscit i, e;
    axiomtablecit ba, ea;

    bool pol = true;
    // determinar si el predicado est� negado
    if((!getPolarity() || !inheritPolarity) && (!(!getPolarity() && !inheritPolarity)))
        pol = false;

    if(pol){
        e = r.second;
        for(i = r.first; i != e; i++) {
            if((*i).second->getPol())
                if(unify(getParameters(),(*i).second->getParameters()))
                    return true;
        }

        // recorremos los distintos axiomas del dominio
        ea = ra.second;
        for(ba = ra.first; ba!= ea; ba++){
            // tenemos una unificaci�n con uno de los axiomas.
            // Testeamos su veracidad
            if(unify(getParameters(),(*ba).second->getParameters()))
                return true;
        }

        // testear con las tareas del dominio.
        tasktablecit i, e = parser_api->domain->getEndTask();
        for(i = parser_api->domain->getBeginTask(); i != e; i++)
            if((*i).second->provides(this))
                return true;

        *err << "Warning Literal: ";
        print(err,0);
        *err << " may be not reachable, defined near or in: ";
        *err << "[" << parser_api->files[parser_api->domain->metainfo[getMetaId()]->fileid] << "]";
        *err << ":" << parser_api->domain->metainfo[getMetaId()]->linenumber << endl;
        return false;
    }
    return true;
};

