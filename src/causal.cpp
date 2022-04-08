#include "causal.hh"
#include "literal.hh"
#include "literaleffect.hh"
#include "primitivetask.hh"

Causal::Causal(LiteralEffect * ref, const Protection & p){
    protection = p;
    producer = ref->getProducer();
    consumer = 0;
    time = ATEND;
    assert(ref != 0);
    literal = ref;
};

Causal::Causal(LiteralEffect * ref, const Protection * p){
    if(p)
	protection = *p;
    else {
	protection.first = 0;
	protection.second = 0;
    }
    producer = ref->getProducer();
    consumer = 0;
    time = ATEND;
    assert(ref != 0);
    literal = ref;
};

Causal::Causal(const Causal & o) {
    assert(o.literal != 0);
    literal = o.literal;
    producer = o.producer;
    consumer = o.consumer;
    protection = o.protection;
    time = o.time ;
};

Causal::Causal(const Causal * o){
    assert(o->literal != 0);
    literal = o->literal;
    producer = o->producer;
    consumer = o->consumer;
    protection = o->protection;
    time = o->time ;
};

bool Causal::isNormalLink(void) const {
    //cerr << "** Leyendo ** " << literal << endl;
    assert(literal != 0);
    return (! ((LiteralEffect *)literal)->isTimeLine() && !((LiteralEffect *)literal)->isFunction());
};

bool Causal::isFluentLink(void) const {
    return literal->isFunction();
};

bool Causal::isTimeLineLink(void) const {
    if(literal->isLEffect())
	return ((LiteralEffect *)literal)->isTimeLine();
    return false;
};

void Causal::print(ostream * os) const{
    if(producer) {
        *os << "(" << producer << ")";
	producer->printHead(os);
    }
    else {
	*os << "(init)";
    }
    *os << " \t(" << literal << ")::: ";
    literal->printL(os,0);
    *os << " \t( " << consumer << ")--> ";
    consumer->printHead(os);
    *os << endl;
};

