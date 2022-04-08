#include "literal.hh"
#include "goal.hh"
#include <sstream>
#include "state.hh"
#include "constants.hh"
#include "domain.hh"
#include "papi.hh"
#include "problem.hh"
#include "literaleffect.hh"

Literal::Literal(int id, int mid)
    :Header(id,mid)
{
    producer = 0;
}

Literal::Literal(int id, int mid, const KeyList * param)
    :Header(id,mid,param)
{
    producer = 0;
}

Literal::Literal(const Literal * other) 
    :Header(other)
{
    producer = other->producer;
};

void Literal::printL(ostream * os, int indent) const
{
	string s(indent,' ');
	*os << s;
	if (producer)
	    *os << "[" << producer->getName() << "] ";

	headerPrint(os);
}

void Literal::toxmlL(XmlWriter * writer) const
{
    writer->startTag("predicate")
	->addAttrib("name",getName());

    for_each(parameters.begin(),parameters.end(),ToXMLKey(writer));
    writer->endTag();
};

pkey Literal::getTermIdL(const char * name) const
{
    keylistcit i = searchTermName(name);
    if(i != parameters.end())
        return (*i);
    else
        return make_pair(-1,-1);
}

bool Literal::hasTermL(int id) const
{
    return searchTermId(id) != parameters.end();
}

void Literal::renameVarsL(Unifier * u, VUndo * undo)
{
    varRenaming(u,undo);
}

