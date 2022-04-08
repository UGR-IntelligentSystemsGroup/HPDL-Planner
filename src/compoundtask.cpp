#include "compoundtask.hh"
#include "method.hh"
#include "domain.hh"
#include "plan.hh"
#include "constants.hh"
#include "textTag.hh"

extern bool PRINTING_PARAMETERS;
extern bool PRINT_NUMBERTYPE;
extern bool PRINT_OBJECTTYPE;
extern bool PRINT_DEFINEDTYPES;

extern bool PRINTING_PARAMETERS;
extern bool PRINT_NUMBERTYPE;
extern bool PRINT_OBJECTTYPE;
extern bool PRINT_DEFINEDTYPES;

template<class T>
struct CloneVM
{
    CloneVM(vector<T *> * v, const CompoundTask * a) {this->v = v;arg = a;};
    void operator()(const T * e)
    {
        v->push_back((T *) e->clone(arg));
    };
    vector<T *> * v;
    const CompoundTask * arg;
};

CompoundTask::CompoundTask(int id, int mid)
    :Task(id,mid), tpoints(0,0) {
    first = false;
    random = false;
}

CompoundTask::CompoundTask(int id, int mid, const KeyList * v)
    :Task(id,mid,v), tpoints(0,0) {
    first = false;
    random = false;
}

CompoundTask::CompoundTask(const CompoundTask * other)
    :Task(other->getId(),other->getMetaId(),other->getParameters()), tpoints(other->tpoints) {
    for_each(other->methods.begin(),other->methods.end(),CloneVM<Method>(&methods,other));
    first = other->first;
    random = other->random;
}

CompoundTask::~CompoundTask(void)
{
    for_each(methods.begin(),methods.end(),Delete<Method>());
};

void CompoundTask::addMethod(Method * m)
{
  methods.push_back(m);
}

void CompoundTask::print(ostream * os, int indent) const
{
	bool PRINT_NUMBERTYPE2, PRINT_OBJECTTYPE2, PRINT_DEFINEDTYPES2;
	string s(indent,' ');

	*os << s << "(:task " << getName() << endl;
	
	PRINTING_PARAMETERS = true;
	
	//Guardamos los valores antiguos
	PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
	PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
	PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;
	
	//Fijamos los nuevos valores
	PRINT_NUMBERTYPE = true;
	PRINT_OBJECTTYPE = true;
	PRINT_DEFINEDTYPES = true;
	
	*os << s << "   :parameters (";
	for_each(parameters.begin(),parameters.end(), PrintKey(os));
	*os << ")" << endl;

	//Recuperamos los valores anteriores
	PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
	PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;
	PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;
	
	PRINTING_PARAMETERS = false;

	
	if(isFirst())
		*os << "(!" << endl;
	for_each(methods.begin(),methods.end(),Print<Method>(os,indent + NINDENT));
	*os << s << ")" << endl;
	if(isFirst())
		*os << ")" << endl;

}

void CompoundTask::toxml(XmlWriter * writer, bool complete) const
{
    //string s(indent,' ');

    //*os << s << "<task name=\"" << getName() << "\">" << endl;
    //for_each(parameters.begin(),parameters.end(), ToXMLKey(os,indent+3));
    //for_each(methods.begin(),methods.end(),ToXML<Method>(os,indent + 3));
    //*os << s << "</task>" << endl;
}

//void CompoundTask::getxmlDescription(ostream * os, int indent) const{
//    string in(indent,' ');
//    const Task * th = this->getParentHeader();
//    if(!th)
//	th = this;
//
//    *os << in << "<compound name='" << getName() << "' id='"<< (int) this << "'>" << endl;
//    *os << in << "   <parameters>" << endl;
//    for_each(th->beginp(),th->endp(), ToXMLParam(os,indent + 6));
//    *os << in << "   </parameters>" << endl;
//    if(eTree){
//	const NodeTaskTree * t = eTree->getNode((int)this);
//	vector<const Task *>::const_iterator i, e = t->children.end();
//	for(i=t->children.begin();i!=e;i++){
//	    *os << in << "   <child id='"<< (int) (*i) <<"' type='";
//	    if((*i)->isPrimitiveTask())
//		*os << "primitive'/>" << endl; 
//	    else
//		*os << "compound'/>" << endl; 
//	}
//    }
//    *os << in << "</compound>" << endl;
//};

void CompoundTask::printHead(ostream * os) const
{
    *os << ":task (" << getName();
    for_each(parameters.begin(),parameters.end(), PrintKey(os));
    * os << ")" << endl;
}

void CompoundTask::prettyPrint(ostream * os) const
{
    // obtener la cadena a procesar
    const Tag * t = parser_api->domain->metainfo[getMetaId()]->getTag("prettyprint"); 
    if(t == 0){
	printHead(os);
	return;
    }

    string s = ((TextTag *)t)->getValue();

    string var="";
    string output="";
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
		if(v <= 0 || v > (int) parameters.size())
		    output = output + "$" + var;
		else {
		    output = output + parser_api->termtable->getTerm(parameters[v - 1])->getName();
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
	if(v <= 0 || v > (int) parameters.size())
	    output = output + "$" + var;
	else {
	    output = output + parser_api->termtable->getTerm(parameters[v - 1])->getName();
	}
    }
    *os << output << endl;
};

Expression * CompoundTask::clone(void) const
{
   // CompoundTask * clon = new CompoundTask(this->getId(),this->getMetaId(),&parameters);
   // for_each(methods.begin(),methods.end(),CloneVM<Method>(&(clon->methods),this));
   // return clon;
   return new CompoundTask(this);
}

pkey CompoundTask::getTermId(const char * name) const
{
    pkey result(-1,-1);
    for(methodcit j = methods.begin(); j!= methods.end(); j++)
    {
        result = (*j)->getTermId(name);
        if(result.first != -1)
            return result;
    }
    return result;
}

bool CompoundTask::hasTerm(int id) const
{
    const_mem_fun1_t<bool,Unifiable,int > f(&Unifiable::hasTerm);
    return find_if(methods.begin(),methods.end(),bind2nd(f,id)) != methods.end();
}

void CompoundTask::renameVars(Unifier * u, VUndo * undo)
{
    methodcit i, e = methods.end();
    for(i=methods.begin();i!=e;i++)
	(*i)->renameVars(u,undo);
}

bool CompoundTask::isReachable(ostream * err) const
{

    methodcit i, e = methods.end();

    for(i=methods.begin();i!=e; i++){
	if(!(*i)->isReachable(err)) {
	    *err << "In task: " << getName() << " defined near or in: ";
	    *err << "[" << parser_api->files[parser_api->domain->metainfo[getMetaId()]->fileid] << "]";
	    *err << ":" << parser_api->domain->metainfo[getMetaId()]->linenumber << endl;
	    return false;
	}
    }
    return true;
};

bool CompoundTask::provides(const Literal * l) const
{
    if(find_if(methods.begin(),methods.end(),bind2nd(const_mem_fun1_t<bool,Method,const Literal *> (&Method::provides),l)) !=  methods.end())
	return true;
    else
	return false;
};


