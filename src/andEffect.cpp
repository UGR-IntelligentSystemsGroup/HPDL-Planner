#include "andEffect.hh"
#include <sstream>
#include <string>

AndEffect::AndEffect(const AndEffect * ae)
    :ContainerEffect(ae)
{
    for_each(ae->effects.begin(),ae->effects.end(),CloneV<Effect>(&effects));
};

AndEffect::~AndEffect(void)
{
    clear();
}

void AndEffect::clear(void)
{
    for_each(effects.begin(),effects.end(),Delete<Effect>());
    effects.clear();
}

Expression * AndEffect::clone(void) const
{
    return new AndEffect(this);
}

void AndEffect::print(ostream * os, int indent) const
{
    string s(indent,' ');

    *os << s;
    *os << "(and" << endl;
    for_each(effects.begin(),effects.end(),Print<Effect>(os,indent + 3));
    *os << s;
    *os << ")" << endl;
}

void AndEffect::toxml(XmlWriter * writer) const{
    writer->startTag("and");
    for_each(effects.begin(),effects.end(),ToXML<Effect,XmlWriter>(writer));
    writer->endTag();
}

bool AndEffect::apply(State *sta, VUndo * undo, Unifier * uf)
{
  effectcit it, e;
  e = getEnd();
  for (it = getBegin(); it != e; it++)
      if(!(*it)->apply(sta,undo,uf))
	  return false;
  return true;
}

pkey AndEffect::getTermId(const char * name) const
{
    pkey result(-1,-1);
    effectcit i,e;
    e = getEnd();
    for(i = getBegin(); i != e; i++)
    {
        result = (*i)->getTermId(name);
        if(result.first != -1)
            return result;
    }
    return result;
}

bool AndEffect::hasTerm(int id) const
{
    const_mem_fun1_t<bool,Unifiable,int > f(&Unifiable::hasTerm);
    return find_if(effects.begin(),effects.end(),bind2nd(f,id)) != effects.end();
}

void AndEffect::renameVars(Unifier * u, VUndo * undo)
{
    effectcit i, e = effects.end();
    for(i=effects.begin();i!=e;i++)
	(*i)->renameVars(u,undo);
}

bool AndEffect::provides(const Literal * l) const
{
  effectcit i, e = effects.end();

  for(i = effects.begin(); i != e; i++)
      if((*i)->provides(l))
	  return true;
  return false;
};

