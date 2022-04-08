#include "fluentLiteral.hh"
#include "function.hh"
#include "pyDefFunction.hh"
#include "domain.hh"
#include "constantsymbol.hh"

void FluentLiteral::printEvaluable(ostream * os, int indent) const
{
    string s(indent,' ');

    *os << s;
    headerPrint(os);
}

void FluentLiteral::toxmlEvaluable(XmlWriter * writer) const {
    writer->startTag("function")
    ->addAttrib("name",getName());

    for_each(parameters.begin(),parameters.end(),ToXMLKey(writer));

    writer->endTag();
}

pkey FluentLiteral::compGetTermId(const char * name) const
{
    keylistcit i = searchTermName(name);
    if(i != parameters.end())
        return (*i);
    else
        return make_pair(-1,-1);
};

bool FluentLiteral::compHasTerm(int id) const
{
    return searchTermId(id) != parameters.end();
}

void FluentLiteral::compRenameVars(Unifier * u, VUndo * undo)
{
    varRenaming(u,undo);
}

pkey FluentLiteral::eval(const State* sta, const Unifier* u, Function ** f) const {
    // cerr << "evaluando: " << this->getName() << endl;
    // buscar la funcion que me corresponde en el estado
    ISTable_range range2 = sta->getRange(getId());
    for(iscit i = range2.first;i!=range2.second;i++){
        if(equal(parameters.begin(),parameters.end(),(*i).second->getParameters()->begin(),EqualPair(u))){
            if((*i).second->isFunction()){
                pkey ret = ((Function *)(*i).second)->getValue();
                //cerr << "[ " << ret.first << "," << ret.second << "]" << endl;
                if(f != 0){
                    *f = (Function *) (*i).second;
                }
                return ret;
            }
        }
    }

    LiteralTableRange range = parser_api->domain->getLiteralRange(getId());
    for(iscit i = range.first;i!=range.second;i++){
        if(unify(getParameters(),(*i).second->getParameters())){
            if((*i).second->isFunction()){
                if(((PyDefFunction *)(*i).second)->isPython()) {
                    // llamar a python para que evalue la funcion.
                    double result=0;
                    if(parser_api->wpython.exec(((PyDefFunction *)(*i).second)->getCode(),this,u,0,&result)){
                        return make_pair(-1,result);
                    }
                    else
                        return make_pair(INT_MAX,0);
                }
            }
        }
    }

    // no se encontro una funcion que unifique, en el estado
    // cerr << "No encontrada!" << endl;
    return make_pair(INT_MAX,0);
};


bool FluentLiteral::isType(const Type * t) const{
    // buscar la funcion que me corresponde en el estado
    LiteralTableRange range = parser_api->domain->getLiteralRange(getId());
    iscit i;
    for(i=range.first;i!=range.second;i++){
    if(unify(getParameters(),(*i).second->getParameters())){
        if((*i).second->isFunction()){
        return ((PyDefFunction *)(*i).second)->isType(t);
        }
    }
    }
    return false;
};

pkey FluentLiteral::evaltp(const State* sta, const Unifier* u, pkey * tp, bool * pol) const {
    // buscar la funcion que me corresponde en el estado
    ISTable_range range2 = sta->getRange(getId());
    for(iscit i = range2.first;i!=range2.second;i++){
        if(equal(parameters.begin(),parameters.end(),(*i).second->getParameters()->begin(),EqualPair(u))){
           if((*i).second->isFunction()){
              pkey ret =  ((Function *)(*i).second)->getValue();
              //cerr << "[ " << ret.first << "," << ret.second << "]" << endl;
              //cerr << this->getName() << endl;
              if(isTimePoint(ret)){
                  if(tp->first != -1){
                      *errflow << "FluentLiteral:: Error: Two or more time points in the expression are not allowed." << endl;
                      exit(-1);
                  }
                  *tp = ret;
                  return make_pair(-1,0);
              }
              return ret;
          }
       }
    }

    LiteralTableRange range = parser_api->domain->getLiteralRange(getId());
    for(iscit i = range.first;i!=range.second;i++){
    if(unify(getParameters(),(*i).second->getParameters())){
        if((*i).second->isFunction()){
        if(((PyDefFunction *)(*i).second)->isPython()) {
            // llamar a python para que evalue la funci�n.
            double result=0;
            if(parser_api->wpython.exec(((PyDefFunction *)(*i).second)->getCode(),this,u,0,&result)){
            return make_pair(-1,result);
            }
            else
            return make_pair(INT_MAX,0);
        }
        }
    }
    }

    // no se encontr� una funci�n que unifique, en el estado...
    return make_pair(INT_MAX,0);
};

Evaluable * FluentLiteral::cloneEvaluable(void) const {
    return new FluentLiteral(this);
};

Literal * FluentLiteral::cloneL(void) const
{
    return new FluentLiteral(this);
};

