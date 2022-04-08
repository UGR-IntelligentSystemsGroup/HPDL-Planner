#include "domain.hh"
#include "primitivetask.hh"
#include "compoundtask.hh"
#include "termTable.hh"
#include "MyLexer.hh"
#include "constants.hh"

//Variables globales para controlar la impresion
bool PRINTING_FUNCTIONS = false;
bool PRINTING_BINDS = false;
bool PRINTING_COMPARATIONGOAL = false;
bool PRINTING_CONSTANTSLIST = false;
bool PRINTING_PREDICATELIST = false;
bool PRINTING_EFFECT = false;
bool PRINTING_TASKSNETWORK = false;
bool PRINTING_SORTGOAL = false;
bool PRINTING_DURATIONS = false;
bool PRINTING_PRECONDITIONS = false;
bool PRINTING_CONDITIONS = false;
bool PRINTING_FORALLPARAMETERS = false;
bool PRINTING_COMPARABLE = false;
bool PRINTING_HEADER_FUNCTION = false;
bool PRINTING_PARAMETERS = false;
bool PRINTING_TYPELIST = false;
bool PRINT_NUMBERTYPE = false;
bool PRINT_OBJECTTYPE = false;
bool PRINT_DEFINEDTYPES = false;
bool PRINT_CONSTANTTYPE = false;

Domain::~Domain()
{
    for_each(metainfo.begin(),metainfo.end(),Delete<Meta>());
    for_each(tasktable.begin(),tasktable.end(),Delete<Task>());
    for_each(literaltable.begin(),literaltable.end(),Delete<Literal>());
    for_each(axiomtable.begin(),axiomtable.end(),Delete<Axiom>());
    for_each(typetable.begin(),typetable.end(),Delete<Type>());
}

Domain::Domain(void) {
    errtyping = false;
    errfluents = false;
    errdisjuntive = false;
    errexistential = false;
    erruniversal = false;
    errnegative = false;
    errconditionals = false;
    errmetatags = false;
    errhtn = false;
    errder = false;
    errdurative=false;
    loaded=false;
    ntypes=0;
    initTypes();
    istimed = false;
    MAX_THORIZON = 0;
    REL_THORIZON = 1000;
    TFORMAT= "%d/%m/%y %H:%M:%S";
    FLAG_TIME_UNIT = TU_UNDEFINED;
    FLAG_TIME_START = 0;
};

Domain::Domain(const char * n)
:name(n) {
    errtyping = false;
    errfluents = false;
    errdisjuntive = false;
    errexistential = false;
    erruniversal = false;
    errnegative = false;
    errconditionals = false;
    errmetatags = false;
    errhtn = false;
    errder = false;
    errdurative = false;
    loaded=false;
    ntypes=0;
    initTypes();
    istimed = false;
    MAX_THORIZON = 0;
    REL_THORIZON = 1000;
    TFORMAT= "%d/%m/%y %H:%M:%S";
    FLAG_TIME_UNIT = TU_UNDEFINED;
};

void Domain::initTypes(void) {
    // antes de gestionar los tipos a�ado el tipo number
    Type * t = new Type();
    t->setName("number");
    t->setFileId(-1);
    t->setLineNumber(0);
    addType(t);
}


void Domain::addLiteral(Literal * l) {
    literaltable.insert(LiteralTable::value_type(l->getId(),l));
}

int Domain::countLiteralElements(int id) const
{
    return literaltable.count(id);
}

literaltablecit Domain::getNextLiteral(LiteralTableRange range, literaltablecit it) const
{
    if((*it).second == 0)
        return range.second;
    ++it;
    if(it != range.second)
        return it;
    return range.second;
}

bool Domain::deleteLiteral(literaltablecit it)
{
    pair<LiteralTable::iterator, LiteralTable::iterator> r = literaltable.equal_range((*it).second->getId());
    LiteralTable::iterator i;

    for(i = r.first; i != r.second;++i)
    {
        if((*i).second == (*it).second)
        {
            literaltable.erase(i);
            return false;
        }
    }
    return true;
}

void Domain::addAxiom(Axiom * l) {
    axiomtable.insert(AxiomTable::value_type(l->getId(),l));
}

int Domain::countAxiomElements(int id) const
{
    return axiomtable.count(id);
}

axiomtablecit Domain::getNextAxiom(AxiomTableRange range, axiomtablecit it) const
{
    if((*it).second == 0)
        return range.second;
    ++it;
    if(it != range.second)
        return it;
    return range.second;
}

bool Domain::deleteAxiom(axiomtablecit it)
{
    pair<AxiomTable::iterator, AxiomTable::iterator> r = axiomtable.equal_range((*it).second->getId());
    AxiomTable::iterator i;

    for(i = r.first; i != r.second;++i)
    {
        if((*i).second == (*it).second)
        {
            axiomtable.erase(i);
            return false;
        }
    }
    return true;
}

void Domain::addTask(Task * task) {
    tasktable.insert(TaskTable::value_type(task->getId(),task));
}

int Domain::countTaskElements(int id) const
{
    return tasktable.count(id);
}

tasktablecit Domain::getNextTask(TaskTableRange range, tasktablecit it) const
{
    if((*it).second == 0)
        return range.second;
    ++it;
    if(it != range.second)
        return it;
    return range.second;
}

bool Domain::deleteTask(tasktablecit it) {
    pair<TaskTable::iterator, TaskTable::iterator> r = tasktable.equal_range((*it).second->getId());
    TaskTable::iterator i;

    for(i = r.first; i != r.second;++i) {
        if((*i).second == (*it).second) {
            tasktable.erase(i);
            return false;
        }
    }
    return true;
}

const Type * Domain::addType(const char * t) {
    Type * nuevo;

    assert(t != 0);
    ldictionaryit i = tdictionary.find(t);

    if((i != tdictionary.end()))
        return typetable[(*i).second];

    nuevo = new Type(t,typetable.size());
    typetable.push_back(nuevo);
    tdictionary.insert(make_pair(nuevo->getName(),nuevo->getId()));
    return nuevo;
}

const Type * Domain::addType(Type * type) {
    ldictionaryit i;
    i = tdictionary.find(type->getName());
    if((i != tdictionary.end())){
        // el tipo ya se defini� con anterioridad
        return typetable[(*i).second];
    }
    // es la primera vez que aparece el tipo
    else {
        type->setId(typetable.size());
        // inserci�n en la tabla de tipos, y en el diccionario de tipos
        typetable.push_back(type);
        tdictionary.insert(make_pair(type->getName(),type->getId()));
        return (const Type *) type;
    }
};

const Type * Domain::getType(const char * t)
{
    return getModificableType(t);
}

Type * Domain::getModificableType(const char * t) {
    ldictionaryit encontrado;

    encontrado = tdictionary.find(t);
    if(encontrado == tdictionary.end()) {
        return 0;
    }
    return typetable[(*encontrado).second];
}

void Domain::setSuperType(const Type * base, Type * supertype) {
    assert(base != 0);
    assert(supertype != 0);

    Type * encontrado;

    encontrado = typetable[base->getId()];
    encontrado->addSuperType(supertype);
}

vector<Task *> * Domain::getUnifyTask(TaskHeader * th,
        vector<VUndo *> * undo) const {
    MetaTH * mt = (MetaTH *) parser_api->domain->metainfo[th->getMetaId()];
    vector<Task *> * vt = new vector<Task *>;
    Task * tcopy;
    Unifier * u=0;
    if(mt->candidates.size() == 1) {
        if(mt->candidates[0]->isPrimitiveTask()) {
            tcopy = (Task *) mt->candidates[0]->clone();
            // en el preprocesamiento (ver Tasknetwork->isWellDefined), ya
            // se comprob� que hab�a unificaci�n, y se comprob� que los tipos
            // encajaban.
            // A�ado las sustituciones necesarias.
            u = new Unifier();
            keylistcit p,e = th->endp();
            keylistcit b = mt->candidates[0]->beginp();
            for(p = th->beginp();p!= e; p++,b++){
                u->addSubstitution((*b).first,(*p));
            }
            //    VUndo * vr = new VUndo ;
            //    tcopy->renameVars(u,vr);
            //    undo->push_back(vr);
            tcopy->renameVars(u,0);
            undo->push_back(0);
            delete u;
            vt->push_back(tcopy);
            return vt;
        }
    }

    vector<Task *>::reverse_iterator i,e = mt->candidates.rend();
    for(i=mt->candidates.rbegin();i!=e;i++){
        if((*i)->isPrimitiveTask()){
            u = new Unifier();
            if(unify2((*i)->getParameters(),th->getParameters(),u)) {
                //    cerr << "###############################################" << endl;
                //    u->print(&cerr,0);
                //    cerr << "###############################################" << endl;
                tcopy = (Task *) (*i)->clone();
                VUndo * vr = new VUndo ;
                //tcopy->renameVars(u,vr);
                tcopy->renameVars(u,0);
                u->applyTypeSubstitutions(vr);
                //tcopy->print(&cerr,0);
                vt->push_back(tcopy);
                undo->push_back(vr);
            }
            delete u;
        }
        else {
            if(unify(th->getParameters(),(*i)->getParameters())) {
                //    cerr << "###############################################" << endl;
                //    u->print(&cerr,0);
                //    cerr << "###############################################" << endl;
                tcopy = (Task *) (*i)->clone();
                VUndo * vr = new VUndo ;
                //tcopy->print(&cerr,0);
                vt->insert(vt->begin(),tcopy);
                undo->insert(undo->begin(),vr);
            }
        }
    }
    return vt;
};

void Domain::addRequirement(const char * requirement) {
    if(!hasRequirement(requirement))
        requirements.push_back(requirement);
    if(!istimed && hasRequirement(":durative-actions"))
        istimed = true;
}

bool Domain::hasRequirement(const char * requirement)
{
    vector<string>::const_iterator i;
    for(i=requirements.begin(); i!= requirements.end();i++)
        if(0==(*i).compare(requirement))
            return true;

    //special cases (implies).
    if(strcmp(requirement,":strips") == 0 && hasRequirement(":adl"))
        return true;
    if(strcmp(requirement,":typing") == 0 && hasRequirement(":adl"))
        return true;
    if(strcmp(requirement,":negative-preconditions") == 0 && (hasRequirement(":adl") || hasRequirement(":quantified-preconditions")))
        return true;
    if(strcmp(requirement,":disjuntive-preconditions") == 0 && (hasRequirement(":adl") || hasRequirement(":quantified-preconditions")))
        return true;
    if(strcmp(requirement,":equality") == 0 && hasRequirement(":adl"))
        return true;
    if(strcmp(requirement,":quantified-preconditions") == 0 && hasRequirement(":adl"))
        return true;
    if(strcmp(requirement,":conditional-effects") == 0 && hasRequirement(":adl"))
        return true;
    if(strcmp(requirement,":durative-actions") == 0 && hasRequirement(":timed-initial-literals")){
        return true;
    }

    return false;
}

void Domain::print(ostream * out, int nindent) const
{

    string s(nindent,' ');

    *out << s << "(define (domain " << name << ")\n";
    // print the requeriment list
    *out << s << "   (:requirements\n";
    for_each(requirements.begin(),requirements.end(),Print<string>(out,nindent + 2*NINDENT));
    *out << s << "   )\n\n";

    //print de types definitition

    PRINTING_TYPELIST = true;
    *out << s << "   (:types\n";

    for_each(typetable.begin(),typetable.end(),PrintTree(out,nindent + 2*NINDENT));
    *out << s << "   )\n\n";
    PRINTING_TYPELIST = false;

    // A partir de aqui no se indica mas el tipo de dato object
    PRINT_OBJECTTYPE = false;


    PRINTING_CONSTANTSLIST = true;
    constablecit cb, ce;
    ce = parser_api->termtable->constants.end();
    *out << s << "   (:constants\n";
    for(cb=parser_api->termtable->constants.begin();cb!=ce;cb++) {
        (*cb)->print(out, nindent);
    }
    *out << s << "   )\n\n";
    PRINTING_CONSTANTSLIST = false;

    PRINTING_PREDICATELIST = true;
    literaltablecit b, e;
    e = this->getEndLiteral();
    *out << s << "   (:predicates" << endl;
    for(b=this->getBeginLiteral();e!=b;b++){
        (*b).second->printL(out, nindent);
        *out << endl;
    }
    *out << s << "   )\n\n" << endl;
    PRINTING_PREDICATELIST = false;


    PRINTING_FUNCTIONS = true;
    e = this->getEndLiteral();
    *out << s << "   (:functions" << endl;
    for(b=this->getBeginLiteral();e!=b;b++){
        (*b).second->printL(out, nindent);
    }
    *out << s << "   )\n\n" << endl;
    PRINTING_FUNCTIONS = false;

    for_each(axiomtable.begin(),axiomtable.end(),Print<Axiom>(out,nindent + NINDENT));

    PRINT_CONSTANTTYPE = false;
    PRINT_NUMBERTYPE = false;
    PRINT_OBJECTTYPE = false;
    PRINT_DEFINEDTYPES = false;
    PRINT_CONSTANTTYPE = false;
    tasktablecit tb, te;
    te = this->getEndTask();
    for(tb=this->getBeginTask();tb!=te;tb++) {
        if((*tb).second->isCompoundTask())
            (*tb).second->print(out, nindent + NINDENT);
    }

    te = this->getEndTask();
    for(tb=this->getBeginTask();tb!=te;tb++) {
        if((*tb).second->isPrimitiveTask())
            (*tb).second->print(out, nindent + NINDENT);
    }

    *out << s << ")";

};

void Domain::typeHierarchyToxml(XmlWriter * writer) const{
    writer->startTag("types");
    typetablecit i, e = typetable.end();
    for(i=typetable.begin();i!=e;i++)
        if((*i)->isRoot())
            (*i)->toxml(writer,false,true);
    writer->endTag();
};

void Domain::toxml(ostream * out) const{
    XmlWriter * writer = new XmlWriter(out);
    toxml(writer);
    writer->flush();
    delete writer;
}

void Domain::toxml(XmlWriter * writer) const {
    writer->startTag("domain")
    ->addAttrib("name",name);

    writer->startTag("requirements");
    vector<string>::const_iterator reqb,reqe;
    reqe = requirements.end();
    for(reqb=requirements.begin(); reqb != reqe; reqb++)
        writer->startTag("requierement")
        ->addAttrib("name",*reqb)
        ->endTag();
    writer->endTag();

    typeHierarchyToxml(writer);

    writer->startTag("predicates");
    for_each(literaltable.begin(),literaltable.end(),ToXMLL<Literal,XmlWriter>(writer));
    writer->endTag();
    writer->startTag("axioms");
    for_each(axiomtable.begin(),axiomtable.end(),ToXML<Axiom,XmlWriter>(writer));
    writer->endTag();
    writer->startTag("tasks");
    for_each(tasktable.begin(),tasktable.end(),ToXML<Task,XmlWriter>(writer));
    writer->endTag();

    writer->endTag();
};

ldictionaryit SearchDictionary(LDictionary * dict, const char * key) {
//    ldictionaryit it;
//    it = dict->find(key);
//    if(it==dict->end()) {
//        cerr << "Fallo buscando: " << key << endl;
//        ldictionaryit b = dict->begin(), e = dict->end();
//        while(b!=e) {
//            cerr << b->first << ":" << b->second << endl;
//            b++;
//        }
//    }
    return dict->find(key);
};

struct MakeRelations{
    TypeRelations * typerelations;
    int ntypes;

    MakeRelations(TypeRelations * v, int n) {typerelations = v; ntypes=n;};

    void operator()(const Type * ref){
        vector<Type *>::const_iterator beg,end;

        if((*typerelations)[ref->getId()*ntypes + ref->getId()])
            // este ya se ha explorado
            return;

        // relacion reflexiva
        (*typerelations)[ref->getId()*ntypes + ref->getId()] = 1;

        MakeRelations mk(typerelations,ntypes);

        end = ref->getParentsEnd();
        beg = ref->getParentsBegin();
        for(;beg != end; beg++){
            // relaci�n directa
            (*typerelations)[ref->getId()*ntypes+(*beg)->getId()] = 1;
            // construyo las relaciones de mi padre
            mk((*beg));
            // a�ado las relaciones inferidas
            for(int i =0;i<ntypes;i++)
                if((*typerelations)[(*beg)->getId()*ntypes+i])
                    (*typerelations)[ref->getId()*ntypes+i] = 1;
        }
    }
};

void Domain::buildTypeRelations(void){
    this->ntypes = typetable.size();
    typerelations.clear();
    typerelations.insert(typerelations.begin(),this->ntypes*this->ntypes,0);
    for_each(typetable.begin(),typetable.end(),MakeRelations(&typerelations,this->ntypes));
    //printTypeRelations(&cerr);
};

struct Internal_Print{
    ostream * os;
    int c;
    Internal_Print(ostream * out) {os = out; c=0;};

    void operator()(const Type * t){
        *os << t->getName() << " [" << t->getId() << "]" << endl;
    }

    void operator()(const vector<bool> * v){
        int i,s = v->size();
        if(c==0){
            *os << "    ";
            for(i=0;i<s;i++)
                if(i < 10){
                    *os << "   " << i;
                }
                else if(i < 100){
                    *os << "  " << i;
                }
                else if(i < 1000){
                    *os << " " << i;
                }
                else{
                    *os <<  i;
                }
            *os << endl;
        }
        if(c < 10){
            *os << "   " << c;
        }
        else if(c < 100){
            *os << "  " << c;
        }
        else if(c < 1000){
            *os << " " << c;
        }
        else{
            *os <<  c;
        }
        c++;
        for(i=0;i<s;i++)
            *os <<  "   " << (*v)[i];
        *os << endl;
    }
};

void Domain::printTypeRelations(ostream * os) const{
    for_each(typetable.begin(),typetable.end(),Internal_Print(os));
    //for_each(typerelations.begin(),typerelations.end(),Internal_Print(os));
};

const Task * Domain::getTaskMetaID(int id, int metaID) const{
    tasktablecit b, e=tasktable.equal_range(id).second;
    for(b=tasktable.equal_range(id).first;b!=e;b++){
        if((*b).second->getMetaId() == metaID)
            return (*b).second;
    }
    return 0;
};

