#include "unifierTable.hh"

UnifierTable::~UnifierTable()
{
    eraseAll();
}

void UnifierTable::eraseAll(void)
{
    for_each(utable.begin(),utable.end(),Delete<Unifier>());
    utable.clear();
}

void UnifierTable::addUnifiers(UnifierTable * t)
{
    for_each(t->getUnifierBegin(),t->getUnifierEnd(),AddV<Unifier>(&utable));
    t->clearUnifiers();
}

void UnifierTable::print(ostream * os) const
{
    for_each(utable.begin(),utable.end(),Print<Unifier>(os));
}

void UnifierTable::cut(void)
{
    if(!utable.empty()){
	for_each(utable.begin()+1,utable.end(),Delete<Unifier>());
	utable.erase(utable.begin()+1,utable.end());
    }
}

UnifierTable::UnifierTable() {};

UnifierTable::UnifierTable(const UnifierTable * o){
    for_each(o->utable.begin(), o->utable.end(), CloneV<Unifier>(&utable));
};

