#include "unify.hh"
#include "unifier.hh"
#include "papi.hh"
#include "variablesymbol.hh"
#include "constantsymbol.hh"
#include "domain.hh"
#include "header.hh"

struct SubTypeOf
{
    SubTypeOf(const Type * super) {this->super = super;};

    const Type * super;

    bool operator()(const Type * t) const
    {
        return parser_api->domain->isSubType(t->getId(),super->getId());
    };
};

bool IsANumber(const Term * t){
    const Type * super  = parser_api->domain->getType(0);
    if(t->isObjectType() || (find_if(t->getBeginType(),t->getEndType(),SubTypeOf(super))!= t->getEndType()))
        return true;
    return false;
};

struct EqualTerm {
    bool operator()(pkey k1, pkey k2) const {
        const vector<Type *> * t1, * t2;

            //cerr << "-------------------------------------" << endl;
            //cerr << "[" << k1.first << "," << k1.second << "]" << endl;
            //cerr << "[" << k2.first << "," << k2.second << "]" << endl;
            //cerr << "-------------------------------------" << endl;

        if(k1.first == k2.first)
            if(k1.second == k2.second)
                return true;

        if(k1.first >= 0 && k2.first >= 0){
            if(k1.first == k2.first){
                return true;
            }
            else {
                return false;
            }
        }

        // es un numero
        if(k1.first == -1){
            if(k2.first < -1){
                if(IsANumber(parser_api->termtable->variables[-k2.first -2])){
                    return true;
                }
                else if(parser_api->termtable->variables[-k2.first -2]->isObjectType()){
                    return true;
                }
            }
            return false;
        }

        if(k1.first <= -2)
            t1 = parser_api->termtable->variables[-k1.first -2]->getTypes();
        else
            t1 = parser_api->termtable->constants[k1.first]->getTypes();

        if(k2.first == -1){
            if(k1.first < -1){
                if(t1->empty() || parser_api->termtable->variables[-k1.first -2]->hasTypeId(0)){
                    return true;
                }
            }
            return false;
        }

        if(k2.first <= -2)
            t2 = parser_api->termtable->variables[-k2.first -2]->getTypes();
        else
            t2 = parser_api->termtable->constants[k2.first]->getTypes();

        if(t2->empty())
            return true;

        typecit i,e,ee;
        e = t2->end();
        i = t2->begin();
        ee  = t1->end();
        while(i != e)
        {
            if(find_if(t1->begin(),t1->end(),SubTypeOf((*i))) != ee)
                return true;
            i++;
        }

        return false;
    };

    bool operator()(Term * k1, pkey k2) const
    {
        const vector<Type *> * t1, * t2;

        if(k2.first == -1) {
            if(k1->isConstant() && k2.second == atof(k1->getName())) {
                return true;
            }
            else {
                return false;
            }
        }

        if(k1->getId() >= 0 && k2.first >= 0) {
            return k1->getId() == k2.first;
        }

        t1 = k1->getTypes();

        if(k2.first <= -2) {
            t2 = parser_api->termtable->variables[-k2.first -2]->getTypes();
        }
        else {
            t2 = parser_api->termtable->constants[k2.first]->getTypes();
        }


        typecit i,e,ee;
        e = t2->end();
        i = t2->begin();
        ee  = t1->end();
        while(i != e) {
            if(find_if(t1->begin(),t1->end(),SubTypeOf((*i))) != ee) {
                return true;
            }
            i++;
        }

        return false;
    };
};

struct EqualTerm2
{
    Unifier * u;
    //Unifier * aux;

    EqualTerm2(Unifier * u) {this->u = u;};

    // Atenci�n, este operador no unificar� si en k2 se encuentra una variable, aunque puede
    // que exista una unificaci�n correcta.
    bool operator()(pkey k1, pkey k2) 
    {

        // comprobar si hay alguna susitituci�n previa de k1 en las
        // unificaciones
        //cerr << k1.first << "," << k1.second << "," << k2.first << "," << k2.second << endl; 
        if(k1.first < -1){
            u->getSubstitution(k1.first,&k1);
        }
        //cerr << k1.first << "," << k1.second << "," << k2.first << "," << k2.second << endl; 
        //aux->print(&cerr,0);

        if(k1.first >= 0){
            // el primer t�rminio es una constante
            // s�lo puede unificar con otra constante
            if(k1.first == k2.first)
                return true;
            else
                return false;
        }
        else if(k1.first == -1) {
            // el primer elemento es un n�mero
            // s�lo unificar� con otro n�mero
            if((k2.first == -1) && (k1.second == k2.second))
                return true;
            else
                return false;
        }
        else{
            VariableSymbol * v = parser_api->termtable->getVariable(k1.first);
            if(k2.first == -1){
                // solo puedo unificar si la variable con la que unifico es num�rica.
                if(IsANumber(v)){
                    u->addSubstitution(k1.first,k2);
                    return true;
                }
            }
            else {
                // solo se unifica si los tipos de k2 encajan en k1
                if(v->isObjectType()) {
                    u->addSubstitution(k1.first,k2);
                    return true;
                }
                else {
                    const vector<Type *> * t1 = v->getTypes();
                    const vector<Type *> * t2 = parser_api->termtable->getConstant(k2.first)->getTypes();
                    typecit i, e = t1->end(), ee = t2->end();
                    for(i=t1->begin();i!=e;i++)
                        if(find_if(t2->begin(),ee,SubTypeOf((*i))) != ee){
                            u->addSubstitution(k1.first,k2);
                            return true;
                        }
                }
            }
        }
        return false;
    };
};

struct EqualTerm3
{
    Unifier * u;

    EqualTerm3(Unifier * u) {this->u = u;};

    bool operator()(pkey k1, pkey k2) const {
        const vector<Type *> * t1, * t2;

         //cerr << "Comparando: [" << k1.first << "," << k1.second << "] " << "[" << k2.first << "," << k2.second << "]" << endl;
        // si son iguales, hemos terminado
        if(k1.first == k2.first) {
            if(k1.first >= 0) {
                return true;
            }
            else if(k1.second == k2.second) {
                return true;
            }
        }

        if(k1.first <= -2) {
            t1 = parser_api->termtable->variables[-k1.first -2]->getTypes();
        }
        else {
            return false;
        }

        // es un numero
        // el type "number" siempre tiene id 0
        if(k2.first == -1){
            if(k1.first < -1){
                if(IsANumber(parser_api->termtable->variables[-k1.first -2])){
                    u->addSubstitution(k1.first,k2);
                    return true;
                }
            }
            return false;
        }

        if(k2.first <= -2){
            // El segundo es una variable.
            // Si hay unificaci�n se producir� substituci�n de tipos.
            t2 = parser_api->termtable->variables[-k2.first -2]->getTypes();
            if(t2->empty())
            {
                // el elemento que toca de v2 es de tipo object.
                // Por lo tanto unifica con lo que sea. 
                u->addSubstitution(k1.first,k2);
                if(!t1->empty())
                    u->addTSubstitution2(k2.first,t1);
                return true;
            }
            else if(!t1->empty())
            {
                // Los elementos que tocan de v1 y v2 est�n tipados 
                typecit i,e,ee,j;
                e = t2->end();
                i = t2->begin();
                ee  = t1->end();
                vector<Type *> * ts = new vector<Type *>;
                while(i != e)
                {
                    if((j=find_if(t1->begin(),t1->end(),SubTypeOf((*i)))) != ee)
                        if(find(ts->begin(),ts->end(),*j) == ts->end())
                            ts->push_back(*j);
                    i++;
                }
                if(ts->empty())
                {
                    delete ts;
                    return false;
                }
                else
                {
                    u->addSubstitution(k1.first,k2);
                    u->addTSubstitution(k2.first,ts);
                    return true;
                }
            }
            // cualquier otro caso equivale a no unificacion
            return false;
        }
        else {
            // esto no es verdad, pero es necesario para que la
            // unificaci�n funcione con los casos que quiero 
            // que funcione
            if(t1->empty()){
                u->addSubstitution(k1.first,k2);
                return true;
            }

            // el segundo es una constante
            t2 = parser_api->termtable->constants[k2.first]->getTypes();

            typecit i,e,ee;
            e = t1->end();
            i = t1->begin();
            ee  = t2->end();
            while(i != e)
            {
                if(find_if(t2->begin(),ee,SubTypeOf((*i))) != ee){
                    u->addSubstitution(k1.first,k2);
                    return true;
                }
                i++;
            }

            return false;
        }
    };
};

struct EqualTerm4
{
    Unifier * u;

    EqualTerm4(Unifier * u) {this->u = u;};

    bool operator()(pkey k1, pkey k2) const
    {
        const vector<Type *> * t1, * t2;

        //cerr << "Comparando: [" << k1.first << "," << k1.second << "] " << "[" << k2.first << "," << k2.second << "]" << endl;

        // si son iguales, hemos terminado
        if(k1.first == k2.first) {
            if(k1.first >= 0) {
                return true;
	    }
            else if(k1.second == k2.second) {
                return true;
	    }
	}

        // El segundo siempre es una variable
        if(k2.first <= -2) {
            t2 = parser_api->termtable->variables[-k2.first -2]->getTypes();
	}
        else {
            return false;
	}

        // es un numero
        // el type "number" siempre tiene id 0
        if(k1.first == -1){
            if(k2.first < -1){
                if(IsANumber(parser_api->termtable->variables[-k2.first -2])){
                    return true;
                }
            }
            return false;
        }

        if(k1.first <= -2){
            // El primero es una variable.
            // Si hay unificaci�n se producir� intersecci�n de tipos.
            t1 = parser_api->termtable->variables[-k1.first -2]->getTypes();

            // los dos son tipo object, hay unificaci�n
            if(t1->empty() && t2->empty())
                return true;

            if(t2->empty() || t1->empty())
            {
                // Alguno de los dos es tipo object.
                // La intersecci�n ser� la de los tipos del que no sea tipo object.
                if(!t2->empty())
                    //cerr << "a�adidos por vacio" << endl;
                    u->addTSubstitution2(k1.first,t2);

                return true;
            }
            else 
            {

                // Los elementos que tocan de v1 y v2 est�n tipados 
                // Realizamos la intersecci�n de los mismos
                typecit i,e,ee,j;
                e = t2->end();
                i = t2->begin();
                ee  = t1->end();
                // Si los tipos de ambos son iguales hemos terminado

                vector<Type* > * ts = new vector<Type *>;
                while(i != e)
                {
                    j = t1->begin();
                    while((j=find_if(j,ee,SubTypeOf((*i)))) != ee){
                        if(find(ts->begin(),ts->end(),*j) == ts->end()){
                            ts->push_back(*j);
                        }
                        j++;
                    }
                    i++;
                }
                e = t1->end();
                i = t1->begin();
                ee  = t2->end();
                while(i != e)
                {
                    j = t2->begin();
                    while((j=find_if(j,ee,SubTypeOf((*i)))) != ee){
                        if(find(ts->begin(),ts->end(),*j) == ts->end()){
                            ts->push_back(*j);
                        }
                        j++;
                    }
                    i++;
                }
                if(ts->empty())
                {
                    delete ts;
                    return false;
                }
                else
                {
                    typeit te = ts->end();
                    typeit ti = ts->begin();
                    i = t1->begin();
                    ee = t1->end();
                    while(i != ee){
                        if(find_if(ti,te,bind2nd(mem_fun(&Type::equal),(*i))) == te)
                            break;
                        i++;
                    }
                    if(i == ee){
                        delete ts;
                        return true;
                    }
                    //cerr << "a�adidos por uni�n" << endl;
                    u->addTSubstitution(k1.first,ts);
                    return true;
                }
            }
        }
        else {
            // el primero es una constante
            t1 = parser_api->termtable->constants[k1.first]->getTypes();

            typecit i,e,ee;
            e = t2->end();
            i = t2->begin();
            if(e == i)
                return true;

            ee  = t1->end();
            while(i != e){
                if(find_if(t1->begin(),ee,SubTypeOf((*i))) != ee)
                    return true;
                i++;
            }

            return false;
        }
    };
};


bool unify(const vector<pkey> * v1, const vector<pkey> * v2) {
    if(v1->size() != v2->size())
        return false;
    return equal(v1->begin(),v1->end(),v2->begin(),EqualTerm());
};

bool unify(const vector<Term *> * v1, const vector<pkey> * v2) {
    if(v1->size() != v2->size())
        return false;
    return equal(v1->begin(),v1->end(),v2->begin(),EqualTerm());
};

bool unify(const vector<pkey> * v1, const vector<pkey> * v2, Unifier * u) {
    if(v1->size() != v2->size())
        return false;

    int os = u->size();
    if(equal(v1->begin(),v1->end(),v2->begin(),EqualTerm2(u))){
        return true;
    }
    else{
        u->erase(u->size() - os);
        return false;
    }
};

bool unify2(const vector<pkey> * v1, const vector<pkey> * v2, Unifier * u)
{
    if(v1->size() != v2->size())
        return false;
    if(!equal(v1->begin(),v1->end(),v2->begin(),EqualTerm3(u)))
        return false;
    return true;
};

bool unify3(const vector<pkey> * v1, const vector<pkey> * v2, Unifier * u) {
    if(v1->size() != v2->size()) {
        return false;
    }
    if(!equal(v1->begin(),v1->end(),v2->begin(),EqualTerm4(u))) {
        return false;
    }
    return true;
};


