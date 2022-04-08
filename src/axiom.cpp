#include "axiom.hh"

extern bool PRINT_NUMBERTYPE;
extern bool PRINT_OBJECTTYPE;
extern bool PRINT_DEFINEDTYPES;

Axiom::Axiom(const Axiom * other) 
    :Header(other) 
{
    goal=0;
    code=0;
    if(other->goal)
	goal = (Goal *) other->goal->clone();
    else if(other->code)
	code = other->code;
};

void Axiom::print(ostream * os, int indent) const
{
    bool PRINT_NUMBERTYPE2, PRINT_OBJECTTYPE2, PRINT_DEFINEDTYPES2;
	
	string s(indent,' ');

	*os << s << "(:derived ";
	headerPrint(os);
	*os << endl;
	if(goal) {
    
		//Guardamos los valores antiguos
		PRINT_NUMBERTYPE2 = PRINT_NUMBERTYPE;
		PRINT_OBJECTTYPE2 = PRINT_OBJECTTYPE;
		PRINT_DEFINEDTYPES2 = PRINT_DEFINEDTYPES;
		
		//Fijamos los nuevos valores
		PRINT_NUMBERTYPE = false;
		PRINT_OBJECTTYPE = false;
		PRINT_DEFINEDTYPES = false;
		
		goal->print(os,indent+NINDENT);
		
		//Recuperamos los valores anteriores
		PRINT_NUMBERTYPE = PRINT_NUMBERTYPE2;
		PRINT_OBJECTTYPE = PRINT_OBJECTTYPE2;
		PRINT_DEFINEDTYPES = PRINT_DEFINEDTYPES2;
	
	}else if(code)
	    *os << s << "   {python}" << endl;
	else
	    *os << s << "   ()" << endl;
	*os << endl << s << ")";
};


void Axiom::toxml(ostream * os) const{
    XmlWriter * writer = new XmlWriter(os);
    toxml(writer);
    writer->flush();
    delete writer;
};

void Axiom::toxml(XmlWriter * writer) const{
    writer->startTag("derived_predicate")
	->addAttrib("name",getName());

    for_each(parameters.begin(),parameters.end(),ToXMLKey(writer));
    if(goal)
	goal->toxml(writer);

    writer->endTag();
}

UnifierTable * Axiom::test(const State * state, const Unifier * context, bool pol, Protection * p)
{
    UnifierTable * ut;
    if(goal != 0){
	ut =goal->getUnifiers(state,context,pol,p);
	return ut;
    }
    else {
	if(!code){
	    if(!pol)
		return 0;
	    else {
		ut = new UnifierTable();
		if(context)
		    ut->addUnifier(context->clone());
		else
		    ut->addUnifier(new Unifier());
		return ut;
	    }
	}
	else{
	    ut = new UnifierTable();
	    double res;
	    if(parser_api->wpython.exec(code,this,context,ut,&res)){
		if(pol)
		    return ut;
		else {
		    delete ut;
		    return 0;
		}
	    }
	    else {
		if(pol)
		    return 0;
		else {
		    delete ut;
		    ut = new UnifierTable();
		    ut->addUnifier(new Unifier());
		    return ut;
		}
	    }
	}
    }
    return 0;
};

bool Axiom::setCode(const char * c) {

    if(!(code= parser_api->wpython.parse(this,c)))
    {
	*errflow << "Error: PyFunction::setCode(): Unable to parse python axiom: " << endl;
	return false;
    }
    return true;
}; 
