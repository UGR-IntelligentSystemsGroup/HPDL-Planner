#include "function.hh"

Function::Function(int id, int mid, bool p, double val) 
    :LiteralEffect(id,mid), value(-1,val) 
{
};

Function::Function(int id, int mid, bool p, double val, const KeyList * parameters) 
    :LiteralEffect(id,mid,parameters,p), value(-1,val) 
{
};

Function::Function(const Function *c) 
    :LiteralEffect(c) 
{
    value = c->getValue();
};

Literal * Function::cloneL(void) const{
    return new Function(this);
};

void Function::printL(ostream * os, int indent) const 
{
    string s(indent,' '); 
    *os << s << "(= ";
    headerPrint(os);
    *os << s;
    parser_api->termtable->print(value,os);
    *os << ")";
};

void Function::toxml(XmlWriter * writer) const {
    writer->startTag("function")
	->addAttrib("name",getName())
	->startTag("value");

    ToXMLKey s(writer);
    s(value);
    writer->endTag();
    for_each(parameters.begin(),parameters.end(),s);
    writer->endTag();
};

