#include "method.hh"
#include <string>
#include <iostream>
#include <sstream>
#include "domain.hh"
#include "constants.hh"

//Variable para controlar la impresion
extern bool PRINTING_PRECONDITIONS;

Method::Method(int mid, const CompoundTask *t, TaskNetwork * tn)
    :ParameterContainer(t)
{
    Unifier u;
    // se a�aden referencias en la tabla de variables a los par�metros 
    // recogidos de la tarea a la cual est� asociada el m�todo.
    compoundtask = t;
    tasknetwork = tn;
    precondition = 0;
    metaid = mid;
    renameVars(&u,0);
}

Method::Method(const CompoundTask * ct,const Method * other )
    :ParameterContainer(other)
{
    Unifier u;
    compoundtask = ct;
    precondition = 0;
    tasknetwork = 0;
    metaid = other->getMetaId();
    
    if(other->precondition)
        setPrecondition((Goal *) other->precondition->clone());

    if(other->tasknetwork)
        setTaskNetwork(other->tasknetwork->clone());
    renameVars(&u,0);
}


Method::~Method(void)
{
  if(precondition)
      delete precondition;
  if(tasknetwork)
      delete tasknetwork;
}


void Method::print(ostream * os, int indent) const
{
	bool PRINT_NUMBERTYPE2, PRINT_OBJECTTYPE2, PRINT_DEFINEDTYPES2;
    string s(indent,' ');

    *os << s << "(" <<  ":method " << getName() << endl;
    PRINTING_PRECONDITIONS = true;
	
	
	//Guardamos los valores antiguos
	PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
	PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
	PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;	
	
	//Fijamos los nuevos valores
	PRINT_NUMBERTYPE = true;
	PRINT_OBJECTTYPE = true;
	PRINT_DEFINEDTYPES = true;	
		
	*os << s << ":precondition" << endl;
    if(precondition)
		Print<Goal>(os,indent+6)(precondition);
	else
    	*os << s << "( )" << endl;
	
	//Recuperamos los valores anteriores
	PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;
	PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
	PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;	
	
	PRINTING_PRECONDITIONS = false;

    if(tasknetwork)
	tasknetwork->print(os,indent);
	    
	*os << s << ")";
}

void Method::toxml(ostream * os) const{
    XmlWriter * writer = new XmlWriter(os);
    toxml(writer);
    writer->flush();
    delete writer;
};

void Method::toxml(XmlWriter * writer) const{
    writer->startTag("method")
	->addAttrib("name",getName())
	->startTag("precondition");
    if(precondition){
	ToXML<Goal,XmlWriter> s(writer);
	s(precondition);
    }

    writer->endTag();

    if(tasknetwork)
	tasknetwork->toxml(writer);

    writer->endTag();
};

Method * Method::clone(const CompoundTask * ct) const
{
    return new Method(ct,this);
}

pkey Method::getTermId(const char * name) const
{
    //cerr << name << endl;
    keylistcit i = searchTermName(name);
    if(i != parameters.end())
        return (*i);

    pkey result(-1,-1);
    if(precondition)
    {
        result = precondition->getTermId(name);
        if(result.first != -1)
            return result;
    }

    if(tasknetwork) 
        result =  tasknetwork->getTermId(name);

    //cerr << result.first << endl;
    return result;
}

bool Method::hasTerm(int id) const
{
    keylistcit i = searchTermId(id);
    if(i != parameters.end())
        return true;

    if(precondition)
        if(precondition->hasTerm(id))
            return true;

    if(tasknetwork) 
        if(tasknetwork->hasTerm(id))
            return true;

    return false;
}

void Method::renameVars(Unifier * u, VUndo * undo)
{
    varRenaming(u,undo);

    if(precondition)
        precondition->renameVars(u,undo);
    if(tasknetwork)
        tasknetwork->renameVars(u,undo);
}

const char * Method::getName(void) const 
{return parser_api->domain->getMetaName(metaid);};

void Method::setName(const char * n) 
{parser_api->domain->setMetaName(metaid,n);};

bool Method::isReachable(ostream * err) const
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

bool Method::provides(const Literal * l) const
{
    tasklistcit i = tasknetwork->getBeginTask();
    tasklistcit e = tasknetwork->getEndTask();

    while((i=find_if(i,e,mem_fun(&Task::isPrimitiveTask))) != e){
	if(((PrimitiveTask *)(*i))->provides(l))
	    return true;
	i++;
    }
    return false;
};

