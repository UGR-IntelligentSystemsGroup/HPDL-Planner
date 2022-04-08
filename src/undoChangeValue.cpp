#include "undoChangeValue.hh"
#include "variablesymbol.hh"
#include "parameterContainer.hh"
#define ATEND INT_MAX

void UndoChangeValue::undo(void)
{
    // Restaurar el valor antiguo
    //cerr << "==================================================================" << endl;
    //parser_api->termtable->print(&cerr);
    //cerr << "UNDO ";
    //parser_api->termtable->getVariable(val.first)->print(&cerr);
    //cerr << " " << target << endl;
    target->setVar(pos,val);
    if(val.first < -1)
	parser_api->termtable->getVariable(val.first)->addReference(this);
    // Las variables tienen un mecanismo de referencias inversas para acelerar
    // el proceso de sustituci�n. En el caso de que el t�rmino a sustituir fuera
    // una variable al hacer el undo volvemos a a�adir la referencia.
    //cerr << "==================================================================" << endl;
    //parser_api->termtable->print(&cerr);
    //cerr << "==================================================================" << endl;
};

void UndoChangeValue::print(ostream * os) const
{
    target->vcprint(os,0);
    *os << " at pos [" << pos << "] put value: ";
    parser_api->termtable->print(val,os);
};

void UndoChangeValue::toxml(XmlWriter * writer) const{
    writer->startTag("changeValue")
	->addAttrib("pos",pos);
    if(time== -1 || time >= ATEND)
	writer->addAttrib("at","end");
    else
	writer->addAttrib("at",time);

    ToXMLKey w(writer);

    writer->startTag("from");
    w(val);
    writer->endTag();

    writer->startTag("to");
    w(new_val);
    writer->endTag();

    target->vctoxml(writer);
    writer->endTag();
};


