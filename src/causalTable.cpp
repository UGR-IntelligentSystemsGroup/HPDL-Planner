#include "causalTable.hh"
#include "task.hh"
#include "primitivetask.hh"
#include <stdexcept>
#include <sstream>

CausalTable causalTable; 

void CausalTable::clean(void){
    cldite i, e;
    clid_ite ii, ie;

    ie = invConsumed.end();
    for(ii=invConsumed.begin();ii!=ie;ii++){
        (*ii).second->clear();
        delete (*ii).second;
    }
    invConsumed.clear();

    e = consumed.end();
    for(i=consumed.begin();i!=e;i++){
        (*i).second->clear();
        delete (*i).second;
    }
    consumed.clear();

    e = produced.end();
    for(i=produced.begin();i!=e;i++){
        (*i).second->clear();
        delete (*i).second;
    }
    produced.clear();
};

CausalTable::~CausalTable(void){
    this->clean();
};


struct AddCRef{
    CLDictionary * prod;
    CLTable * v;

    AddCRef(CLTable * a, CLDictionary * p){
        v = a;
        prod = p;
    };

    void operator()(Causal * c){
        v->push_back(c);
        cldite j = prod->find(c->getProducer());
        //c.print(&cerr);
        if(j != prod->end())
            (*j).second->push_back(c);
    };
};

UndoCLinks * CausalTable::addCausalLinks(const Task * consumer, CLTable * t){
    // Insertamos en los consumidos, tal cual, solo hay que copiar el vector.
    // en cuanto a los producidos hay que añadir las nuevas referencias.
    UndoCLinks * u = new UndoCLinks;

    CLTable * a = new CLTable;

    //cerr << "#####################" << endl;
    //cerr << "INSERTANDO: " << consumer << endl;
    //consumer->printHead(&cerr);
    //print(&cerr);
    //cerr << "#####################" << endl;

    if (consumer == 0) {
        throw std::logic_error("CausalTable::addCausalLinks Null pointer");
    }
    consumed[consumer] = a;
    produced[consumer] = new CLTable;
    u->key = consumer;

    if(t)
        for_each(t->begin(),t->end(),AddCRef(a,&produced));

    cltcite bc,ec;
    if(t){
        ec = t->end();
        // recorremos los vínculos causales que no forman parte del timeline
        for(bc=t->begin();bc!=ec;bc++){
            if(invConsumed.find((*bc)->getLiteral()->getId()) == invConsumed.end()){
                invConsumed.insert(make_pair((*bc)->getLiteral()->getId(),new CLTable));
            }
            invConsumed[(*bc)->getLiteral()->getId()]->push_back((*bc));
        }
    }

    //print(&cerr);
//    try {
//        checkConsistency();
//    }
//    catch(std::exception & e){
//        cerr << e.what() << endl;
//        cerr << "INSERTANDO" << endl;
//        consumer->printHead(&cerr);
//        print(&cerr);
//        abort();
//    }
    return u;
};

bool CausalTable::eraseCausalLinks(const Task * consumer){
    bool error = false;


    //cerr << "BORRANDO: " << consumer << endl;
    //consumer->printHead(&cerr);
    //print(&cerr);
    //cerr << "#####################" << endl;

    cldite i, k;
    cltite j, je;
    cltite t, te;
    clid_ite kk;
    Causal * ptr;
    const Task * tp;
    bool encontrado;
    const LiteralEffect * l;

    // Recorremos los vínculos causales consumidos. Se buscan
    // las tareas que los producen, y se elimina el vínculo
    // causal de su tabla hash.
    i = consumed.find(consumer);
    if(i != consumed.end()){
        je = (*i).second->end();
        for(j=(*i).second->begin(); j != je; j++){
            ptr = (*j);
            tp = (const Task *) ptr->getProducer();
            k = produced.find(tp);
            if (k != produced.end()) {
                // ya he capturado la tarea que produce el
                // vínculo. Ahora hay que buscar en su vector
                // asociado el vínculo causal y borrarlo.
                te = (*k).second->end();
                t = (*k).second->begin();
                encontrado = false;
                while(t != te) {
                    t = std::find(t,te,ptr);
                    if (t != te) {
                        t = (*k).second->erase(t);
                        encontrado = true;
                        te = (*k).second->end();
                    }
                }
                if(!encontrado){
                    throw std::logic_error("CausalTable::eraseCausalLinks (1) Causal Link structure is inconsistent!");
                }
            } // if produced

            // Lo mismo con la tabla que tiene como índice
            // el literal consumido.
            l = (const LiteralEffect *) ptr->getLiteral();
            kk = invConsumed.find(l->getId());
            if (kk != invConsumed.end()) {
                // Tengo el vínculo. Ahora buscar el vínculo causal
                // asociado a la tarea que estamos eliminando y
                // borrarlo.
                te = (*kk).second->end();
                t = (*kk).second->begin();
                encontrado = false;
                while(t != te) {
                    t = std::find(t,te,ptr);
                    if (t != te) {
                        t = (*kk).second->erase(t);
                        encontrado = true;
                        te = (*kk).second->end();
                    }
                }
                if(!encontrado){
                    throw std::logic_error("CausalTable::eraseCausalLinks (2) Causal Link structure is inconsistent!");
                }
            } // if invConsumed
        } // del for
    } // del if
    else {
        error = true;
    }

    i = consumed.find(consumer);
    if (i != consumed.end()){
        delete (*i).second;
        consumed.erase(i);
    }
    else {
        error = true;
    }

    i = produced.find(consumer);
    if (i != produced.end()){
        if(!((*i).second->empty())){
            //cerr << "BORRANDO: " <<  consumer << endl;
            //consumer->printHead(&cerr);
            //printProduced(&cerr,consumer);
            //cerr << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" <<endl;
            throw std::logic_error("CausalTable::eraseCausalLinks: (3) Causal Link structure is inconsistent!");
        }
        delete (*i).second;
        produced.erase(i);
    }
    else {
        error = true;
    }
    //cerr << "................................" << endl;
    //printConsumed(&cerr,consumer);
    //print(&cerr);
//    try {
//        checkConsistency();
//    }
//    catch(std::exception & e){
//        cerr << e.what() << endl;
//        cerr << "BORRANDO" << endl;
//        consumer->printHead(&cerr);
//        print(&cerr);
//        abort();
//    }

    return error;
};

const CLTable* CausalTable::getConsumedLinks(const Task * p) const{
    cldcite i;

    i= consumed.find(p);
    if(i != consumed.end())
        return (*i).second;
    else
        return 0;
};

const CLTable * CausalTable::getLinks(const LiteralEffect * e) const{
    cclid_ite i;

    i= invConsumed.find(e->getId());
    if(i != invConsumed.end())
        return (*i).second;
    else
        return 0;
};

void CausalTable::plot(ostream * os) const{
    cldcite i, ie;
    cltcite j, je;
    const Task * t;
    const Causal * c;

    *os << "digraph plan {" << endl;
    *os << "n0" << "[label=\"" << "initial state" << "\",color=red,shape=box]" << endl;

    ie = consumed.end();
    for(i = consumed.begin(); i != ie; i++){
        if(!(*i).second->empty()){
            c = (*i).second->front();
            t = c->getConsumer();
            //cerr << t << endl;
            if(t->isPrimitiveTask())
                *os << "n" << t << "[label=\"" << t->getName() << "\",color=green,shape=box]" << endl;
            else 
                *os << "n" << t << "[label=\"" << t->getName() << "\",color=blue,shape=box]" << endl;
        }
    }

    ie = consumed.end();
    for(i = consumed.begin(); i != ie; i++){
        je = (*i).second->end();
        for(j = (*i).second->begin(); j != je; j++){
            *os << "n" << (*j)->getProducer() << " -> " << "n" << (*j)->getConsumer();
            *os << " [label=\"";
            (*j)->getLiteral()->printL(os ,0);
            *os << "\",color=black,fontsize=8]" << endl;
        }
    }

    *os << "}";
};

struct PrintCausal{
    ostream * os;

    PrintCausal(ostream * st){os=st;};
    void operator()(const Causal & c){
        c.print(os);
    };
};

void CausalTable::print(ostream * os) const{
    cldcite i, ie = consumed.end();

    *os << "------------- Casual Link structure -------------" << endl;
    *os << "------------- Consumed -------------" << endl;
    for(i = consumed.begin(); i != ie; i++){
        *os << "key--> " << (*i).first;
        (*i).first->printHead(os);
        *os << endl;
        for_each((*i).second->begin(),(*i).second->end(), PrintCausal(os));
    }
    *os << "------------- Produced -------------" << endl;
    ie = produced.end();
    for(i = produced.begin(); i != ie; i++){
        *os << "key--> " << (*i).first;
        (*i).first->printHead(os);
        *os << endl;
        for_each((*i).second->begin(),(*i).second->end(), PrintCausal(os));
    }
    *os << "----------- eof Casual Link structure -----------" << endl;
};

void CausalTable::printConsumed(ostream * os, const Task * t) const{
    cldcite i;
    i = consumed.find(t);

    if (i != consumed.end())
        for_each((*i).second->begin(),(*i).second->end(), PrintCausal(os));
};

void CausalTable::printProduced(ostream * os, const Task * t) const{
    cldcite i;
    i = produced.find(t);

    if (i != produced.end())
        for_each((*i).second->begin(),(*i).second->end(), PrintCausal(os));
};

bool CausalTable::hasCausalLinks(const Task * a, const Task * b) const{
    bool res = false;
    cldcite i;
    cltcite j, je;

    i = produced.find(a);
    if (i != produced.end()){
        je = (*i).second->end();
        for(j = (*i).second->begin(); j != je; j++)
            if((*j)->getConsumer() == b)
                return true;
    }

    i = consumed.find(a);
    if (i != consumed.end()){
        je = (*i).second->end();
        for(j = (*i).second->begin(); j != je; j++)
            if((*j)->getProducer() == b)
                return true;
    }

    return res;
};

void CausalTable::checkConsistency(void) const {
    cldcite i,e;
    cltcite j, je, k, ke;
    const Task * tmp;
    const Literal * lit;
    bool found;
    std::ostringstream buffer;

    // mirar que no haya punteros a null
    // y que todos los producidos sean consumidos
    i = produced.begin();
    e = produced.end();
    while(i!=e){
        //if ((*i).first == 0) throw std::logic_error("CausalTable::checkConsistency (1) Null pointer");
        je = (*i).second->end();
        for(j = (*i).second->begin(); j != je; j++) {
            (*j)->checkConsistency();
            if(((const Task *)(*j)->getProducer()) != (const Task *) (*i).first){
                buffer << "CausalTable::checkConsistency Causal link producer task mismatch: "
                    << (*j)->getProducer() << " : "
                    << (*i).first;
                throw std::logic_error(buffer.str());
            };
            tmp = (*j)->getConsumer();
            if(consumed.find(tmp) == consumed.end()) {
                buffer << "CausalTable::checkConsistency Consumer task not found in structure: " 
                   << "(" << tmp << ") ";
                tmp->printHead(&buffer);
                throw std::logic_error(buffer.str());
            }
        }
        i++;
    }

    // mirar que no haya punteros a null
    // y que todos los consumidos sean producidos 
    i = consumed.begin();
    e = consumed.end();
    while(i!=e){
        if ((*i).first == 0) throw std::logic_error("CausalTable::checkConsistency (2) Null pointer");
        je = (*i).second->end();
        for(j = (*i).second->begin(); j != je; j++) {
            (*j)->checkConsistency();
            if ((*j)->getConsumer() != (*i).first) {
                buffer << "CausalTable::checkConsistency Causal link consumer task mismatch: "
                    << (*j)->getProducer() << " : "
                    << (*i).first;
                throw std::logic_error(buffer.str());
            }
            tmp = (*j)->getProducer();
            if (tmp != 0) {
                if (produced.find(tmp) == produced.end()) {
                    buffer << "CausalTable::checkConsistency Producer task not found in structure: "
                       << "(" << tmp << ") ";
                       tmp->printHead(&buffer);
                    throw std::logic_error(buffer.str());
                }
            }
        }
        i++;
    }

    // chequear la estructura inversa
    cclid_ite ii, ie;
    ii = invConsumed.begin();
    ie = invConsumed.end();
    while(ii!=ie) {
        if ((*ii).first == 0) throw std::logic_error("CausalTable::checkConsistency (3) Null pointer");
        je = (*ii).second->end();
        for(j = (*ii).second->begin(); j != je; j++) {
            (*j)->checkConsistency();
            if ((*j)->getLiteral()->getId() != (*ii).first){
                buffer << "CausalTable::checkConsistency Causal link literal mismatch: "
                    << (*j)->getLiteral()->getId() << " : "
                    << (*ii).first;
                throw std::logic_error(buffer.str());
            }
            lit = (*j)->getLiteral();
            tmp = (*j)->getConsumer();
            if (consumed.find(tmp) == consumed.end()) {
                buffer << "CausalTable::checkConsistency Consumer task not found: "
                   << "(" << tmp << ") ";
                tmp->printHead(&buffer);
                throw std::logic_error(buffer.str());
            }
            i = consumed.find(tmp);
            ke = (*i).second->end();
            found = false;
            for(k = (*i).second->begin();k!=ke && !found;k++) {
                if((*k)->getLiteral() == lit) {
                    found = true;
                }
            }
            if (!found) {
                buffer << "CausalTable::checkConsistency Literal not found in consumed structure: "
                    << "(" << lit << ") ";
                lit->printL(&buffer);
                throw std::logic_error(buffer.str());
            };
        }

        ii++;
    };
};

