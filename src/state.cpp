#include "state.hh"
#include "goal.hh"
#include "literaleffect.hh"

Literal * State::getModificableLiteral(int id)
{
    ISTable::iterator i = Literaltable.find(id);
    if(i == Literaltable.end())
        return NULL;
    return i->second;
}

const Literal * State::getLiteral(int id) const
{
    iscit i = Literaltable.find(id);
    if(i == Literaltable.end())
        return NULL;
    return i->second;
}

void State::printKeys(ostream * out) const {
    iscit i; 
    int c = Literaltable.size();
    *out << "Number of elements: " << c << endl;
    for(i=Literaltable.begin(); i != Literaltable.end();i++) {
        *out << (*i).first << " " << endl; 
    }
}
void State::test(void) const{
    iscit i; 
    for(i=Literaltable.begin(); i != Literaltable.end();i++) {
	(*i).second->getId();
    }
    // int c = Literaltable.size();
    //cerr << "Number of elements: " << c << endl;
};

void State::print(ostream * out) const {
    for_each(Literaltable.begin(),Literaltable.end(),PrintL<Literal>(out,0));
}

void State::deleteAll(void)
{
    Literaltable.clear();
}

State::~State(void)
{
    deleteAll();
}

isit State::addLiteral(Literal * symbol)
{
    return Literaltable.insert(ISTable::value_type(symbol->getId(),symbol));
}

int State::countElements(int id) const
{
    return Literaltable.count(id);
}

ISTable_range State::getRange(int id) const {
    return Literaltable.equal_range(id);
}

iscit State::getFirstLiteral(ISTable_range range)
{
    iscit i = range.first; 
    if(i != range.second)
        return i;
    return range.second;
}

iscit State::getEndLiteral(ISTable_range range)
{
  return range.second;
}

iscit State::getNextLiteral(ISTable_range range, iscit it)
{
    if((*it).second == 0)
        return range.second;
    ++it;
    if(it != range.second)
        return it;
    return range.second;
}

const Literal * State::getLiteral(iscit it) const
{
    if((*it).second != 0)
        return (*it).second;
    return 0;
}

struct CopiaTLiteral{
    CopiaTLiteral(ISTable * container){
	this->container = container;
    };

    void operator()(const std::pair<const int, Literal*>& element){
	container->insert(make_pair(element.first,element.second->cloneL()));
    };

    ISTable * container;
};

State * State::clone(void) const{
    State * s = new State();
    for_each(Literaltable.begin(),Literaltable.end(),CopiaTLiteral(&(s->Literaltable)));
    return s;
}

