#include "taskheader.hh"
#include <sstream>

TaskHeader::TaskHeader(int id, int mid)
    :Task(id,mid)
{
};

TaskHeader::TaskHeader(int id, int mid, const KeyList * v)
    :Task(id,mid,v)
{
};

TaskHeader::TaskHeader(const TaskHeader * t)
    :Task(t->id,t->getMetaId(),t->getParameters())
{
};

Expression *TaskHeader::clone(void) const
{
 return new TaskHeader(this);
};

void TaskHeader::print(ostream * os,int indent) const
{
	string s(indent,' ');

	*os << s << "(" << getName();
	for_each(parameters.begin(),parameters.end(), PrintKey(os));
	* os << ")" << endl;
};

void TaskHeader::toxml(XmlWriter * writer, bool complete) const{
    writer->startTag("task")
	->addAttrib("name",getName())
	->startTag("parameters");
    for_each(parameters.begin(),parameters.end(), ToXMLKey(writer));
    writer->endTag()
	->endTag();
};

pkey TaskHeader::getTermId(const char * name) const
{
    keylistcit i = searchTermName(name);
    if(i != parameters.end())
        return (*i);
    else
        return make_pair(-1,-1);
};

bool TaskHeader::hasTerm(int id) const
{
    return searchTermId(id) != parameters.end();
};

void TaskHeader::renameVars(Unifier * u, VUndo * undo)
{
    varRenaming(u,undo);
}

