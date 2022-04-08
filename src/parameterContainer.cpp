#include "parameterContainer.hh"
#include "undoChangeValue.hh"
#include "unifier.hh"
#include "termTable.hh"

ParameterContainer::ParameterContainer(const KeyList * v) 
{
    parameters.insert(parameters.begin(),v->begin(),v->end());
};

ParameterContainer::ParameterContainer(const ParameterContainer * pc) 
{
    parameters.insert(parameters.begin(),pc->parameters.begin(),pc->parameters.end());
};

ParameterContainer::~ParameterContainer(void)
{

    //cerr << "==================================================================" << endl;
    //parser_api->termtable->print(&cerr);
    // si hay una variable que nos referencia eliminar dicha referencia
    keylistit i, e = parameters.end();
    for(i=parameters.begin() ;i!= e;i++) {
	if((*i).first < -1) {
	    parser_api->termtable->getVariable((*i))->removeReference(this);
	    //cerr << "(ee)";
	    //parser_api->termtable->getVariable(*i)->print(&cerr);
	    //cerr << " ";
	}
    }
    //cerr << "Erased: " << this << endl;
    //cerr << "==================================================================" << endl;
    //parser_api->termtable->print(&cerr);
    //cerr << "==================================================================" << endl;
};

void ParameterContainer::varRenaming(Unifier *u, VUndo * undo){
    int pos = 0;
    keylistit i,e;
    pkey p;

    e = parameters.end();
    for(i=parameters.begin() ;i!= e;i++, pos++){
        if((*i).first < -1){
	    // si queremos guardar informaci�n para posteriormente
	    // deshacer los cambios provocados por el renaming
            if(u->getSubstitution((*i).first,&p)){
		if(undo) 
		    undo->push_back(new UndoChangeValue(this,pos,(*i),p));
		(*i) = p;
		if((*i).first < -1)
		    parser_api->termtable->getVariable(*i)->addReference(new UndoChangeValue(this,pos,*i,*i));
	    }
            else
            {
                //se crea una nueva entrada en la tabla de t�rminos
                pkey nv = parser_api->termtable->addVariable((VariableSymbol *) parser_api->termtable->getVariable((*i))->clone());
		if(undo)
		    undo->push_back(new UndoChangeValue(this,pos,(*i),nv));
                parser_api->termtable->getVariable(nv)->addReference(new UndoChangeValue(this,pos,nv,nv));
                u->addSubstitution((*i).first,nv);
                (*i) = nv; 
            }
        }
    }
};

void ParameterContainer::setVar(int pos, pkey &newval)
{
    setParameter(pos,newval);
}; 
   
