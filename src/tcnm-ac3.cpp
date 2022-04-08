#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <sstream>

#include "tcnm-ac3.hh"

using namespace std;

void PintaCola(string message, queue<size_t> q);


string DPrint(int i){
    ostringstream os;
        if(i >= POS_INF){
            return "inf";
    }
        if(i <= NEG_INF){
            return "-inf";
    }
    os << i;
    return os.str();
};


STP::STP() {
   consistent=true;
   Constraints.clear();
   TimePoints.clear();
   constraints_processed = 0;
   history.clear();
   history.push_back(pair<size_t,size_t>(0,0));
}

STP::~STP() {
   Constraints.clear();
   TimePoints.clear();
}

void STP::Reset() {
  for (size_t i=0; i<Size(); i++) {
    TimePoints[i].lower=0;
    TimePoints[i].upper= (i==0 ? 0 : POS_INF);
    TimePoints[i].lb = TimePoints[i].ub = NIL;
  }
  for (size_t i=0; i<Constraints.size(); i++)
    Constraints[i].lb = Constraints[i].ub = NIL;
  constraints_processed=0;
  consistent = true;
}

void STP::Backup() {
  for (size_t i=0; i<Size(); i++) {
    TimePoints[i].blower=TimePoints[i].lower;
    TimePoints[i].bupper=TimePoints[i].upper;
    TimePoints[i].blb=TimePoints[i].lb;
    TimePoints[i].bub=TimePoints[i].ub;
  }
  for (size_t i=0; i<Constraints.size(); i++) {
    Constraints[i].blower=Constraints[i].lower;
    Constraints[i].bupper=Constraints[i].upper;
    Constraints[i].blb=Constraints[i].lb;
    Constraints[i].bub=Constraints[i].ub;
  }
}

void STP::Restore() {
  for (size_t i=0; i<Size(); i++) {
    TimePoints[i].lower=TimePoints[i].blower;
    TimePoints[i].upper=TimePoints[i].bupper;
    TimePoints[i].lb=TimePoints[i].blb;
    TimePoints[i].ub=TimePoints[i].bub;
  }
  for (size_t i=0; i<Constraints.size(); i++) {
    Constraints[i].lower=Constraints[i].blower;
    Constraints[i].upper=Constraints[i].bupper;
    Constraints[i].lb=Constraints[i].blb;
    Constraints[i].ub=Constraints[i].bub;
  }
}

pair<constr,constr> STP::Query(int i){
  return pair<constr,constr>(TimePoints[i].lower,TimePoints[i].upper);
}

constr STP::Query(int i, int j){
  if (j != 0) {
    cerr << "*** ERROR *** Llamada err�nea a la antigua funci�n Query (" << i << ", " << j << ");" << endl;
    exit(1);
  }
  else
    return -Query(i).first;
}


int STP::InsertTPoint() {
    //cerr << "InsertTPoint();" << endl;
  timepoint_row newr;
  newr.lower = 0;
  if (Size() > 0)
    newr.upper = POS_INF;
  else
    newr.upper = 0;
  newr.lb = NIL;
  newr.ub = NIL;
  TimePoints.push_back(newr);
  return Size()-1;
}


void STP::AddTConstraint(int i, int j, constr lower, constr upper) {
  //cerr << i << " " << j << " " << lower << " " << upper << endl;
  constraint_row newr;
  newr.i = i;
  newr.j = j;
  newr.lower = lower;
  newr.upper = upper;
  newr.lb = NIL;
  newr.ub = NIL;
  Constraints.push_back(newr);
}

bool STP::AddTConstraint2(int i, int j, constr lower, constr upper, bool record) {
  //cerr << "AddTConstraint2(" << i << "," << j << "," << DPrint(lower) << "," << DPrint(upper) << ");" << endl;
  AddTConstraint(i,j,lower,upper);
  return Closure();
}

bool STP::ReviseConstraints(size_t constraint, bool &bi, bool &bj) {
  constraint_row c=Constraints[constraint];

  bi=false; bj = false;
  // Explora los l�mites inferiores
  if (TimePoints[c.j].lower < TimePoints[c.i].lower+c.lower) {
    bj = true;
    if (TimePoints[c.j].lb != NIL) {
      Constraints[TimePoints[c.j].lb].lb = NIL;
    }
    TimePoints[c.j].earliest=TimePoints[c.j].lower = TimePoints[c.i].lower+c.lower;
    TimePoints[c.j].lb = constraint;
    Constraints[constraint].lb=c.j;
  }
  else
    if (TimePoints[c.i].lower < TimePoints[c.j].lower-c.upper) {
      bi = true;
      if (TimePoints[c.i].lb != NIL) {
    Constraints[TimePoints[c.i].lb].lb = NIL;
      }
      TimePoints[c.i].earliest=TimePoints[c.i].lower = TimePoints[c.j].lower-c.upper;
      TimePoints[c.i].lb = constraint;
      Constraints[constraint].lb=c.i;
    }
  // Explora los l�mites superiores
  if (TimePoints[c.j].upper > TimePoints[c.i].upper+c.upper) {
    bj = true;
    if (TimePoints[c.j].ub != NIL) {
      Constraints[TimePoints[c.j].ub].ub = NIL;
    }
    TimePoints[c.j].latest=TimePoints[c.j].upper = TimePoints[c.i].upper+c.upper;
    TimePoints[c.j].ub = constraint;
    Constraints[constraint].ub=c.j;
  }
  else
    if (TimePoints[c.i].upper > TimePoints[c.j].upper-c.lower) {
      bi = true;
      if (TimePoints[c.i].ub != NIL) {
    Constraints[TimePoints[c.i].ub].ub = NIL;
      }
      TimePoints[c.i].latest=TimePoints[c.i].upper = TimePoints[c.j].upper-c.lower;
      TimePoints[c.i].ub = constraint;
      Constraints[constraint].ub=c.i;
    }
  return (TimePoints[c.i].lower <= TimePoints[c.i].upper and TimePoints[c.j].lower <= TimePoints[c.j].upper);
}



bool STP::Closure() {
  queue <size_t> q;

  // Mete en la cola las constraints sin procesar
  for (size_t i=constraints_processed; i<Constraints.size(); i++)
    q.push(i);

  consistent = Closure(q);
//   if (consistent)
//     constraints_processed = Constraints.size();
  return consistent;
}

bool STP::Closure(queue <size_t> & source) {
  bool revisei, revisej;
  size_t c, cs=0, cq=0;
  queue <size_t> q;
  bool from_source;

  // Bucle principal
  while ((!q.empty() || !source.empty())  and consistent) {
    // Va sacando las restricciones en la cola source una a una y propagandolas por
    // completo antes de pasar a la siguiente
    if (q.empty()) {
      c = cs = source.front();
      source.pop();
      constraints_processed++;
      // Hace copia de seguridad de los valores de la red
      // Si la restriccion c da una inconsistencia
      // Ser� posible recuperar la red muy r�pido
      Backup();
      from_source=true;
    }
    else {
      c = cq = q.front();
      q.pop();
      from_source=false;
    }
    consistent = ReviseConstraints (c, revisei, revisej);
    // Dependencias

    // Shorcut Amedeo
    if (!from_source && cq == cs) {
      if (revisei) {
    vector <bool> visitadosl(Size(), false);
    consistent = !Follow_lb(Constraints[c].i, visitadosl);
    if (consistent) {
    vector <bool> visitadosu(Size(), false);
    consistent = !Follow_ub(Constraints[c].i, visitadosu);
    }
      }
      if (revisej && consistent) {
    vector <bool> visitadosl(Size(), false);
    consistent = !Follow_lb(Constraints[c].j, visitadosl);
    if (consistent) {
    vector <bool> visitadosu(Size(), false);
    consistent = !Follow_ub(Constraints[c].j, visitadosu);
    }
      }
    }
    // Propagacion al resto de timepoints
    for (size_t i=0; i<constraints_processed && consistent; i++)
      if (i != c) {
    if (revisei and
        (Constraints[c].i == Constraints[i].i || Constraints[c].i == Constraints[i].j))
        q.push(i);
    if (revisej and
        (Constraints[c].j == Constraints[i].i || Constraints[c].j == Constraints[i].j))
        q.push(i);
      }
  }
  return consistent;
}

// Inicializar visited a todo false
bool STP::Follow_lb(size_t node, vector <bool> &visited) {
  bool found = false;
  size_t c;
  // Comprueba que node no est� visitado
  found = visited[node];
  // Explora sus hijos en la lista de sucesores
  if (!found) {
    visited[node] = true;
    //    cerr << "(" << node << ")";
    // Si tiene padres por el lb los sigue
    c = TimePoints[node].lb;
    if ((int) c != NIL) {
      if (Constraints[c].i == (int) node)
    found = Follow_lb(Constraints[c].j, visited);
      else
    found = Follow_lb(Constraints[c].i, visited);
    }
  }
  return found;
}

// Inicializar visited a todo false
bool STP::Follow_ub(size_t node, vector <bool> &visited) {
  bool found = false;
  size_t c;
  // Comprueba que node no est� visitado
  found = visited[node];
  // Explora sus hijos en la lista de sucesores
  if (!found) {
    visited[node] = true;
    //    cerr << "(" << node << ")";
    // Si tiene padres por el lb los sigue
    c = TimePoints[node].ub;
    if ((int) c != NIL) {
      if (Constraints[c].i == (int) node)
    found = Follow_ub(Constraints[c].j, visited);
      else
    found = Follow_ub(Constraints[c].i, visited);
    }
  }
  return found;
}

bool STP::IsConsistent() {
  return consistent;
}

bool STP::ReviseSchedule(size_t constraint, bool &bi, bool &bj) {
  constraint_row c=Constraints[constraint];

  bi=false; bj = false;
  // Explora los l�mites inferiores
  if (TimePoints[c.j].earliest < TimePoints[c.i].earliest+c.lower) {
    bj = true;
    TimePoints[c.j].earliest = TimePoints[c.i].earliest+c.lower;
  }
  else
    if (TimePoints[c.i].earliest < TimePoints[c.j].earliest-c.upper) {
      bi = true;
      TimePoints[c.i].earliest = TimePoints[c.j].earliest-c.upper;
    }
  // Explora los l�mites superiores
  if (TimePoints[c.j].latest > TimePoints[c.i].latest+c.upper) {
    bj = true;
    TimePoints[c.j].latest = TimePoints[c.i].latest+c.upper;
  }
  else
    if (TimePoints[c.i].latest > TimePoints[c.j].latest-c.lower) {
      bi = true;
      TimePoints[c.i].latest = TimePoints[c.j].latest-c.lower;
    }
  return (TimePoints[c.i].earliest <= TimePoints[c.i].latest and TimePoints[c.j].earliest <= TimePoints[c.j].latest);
}


// Propaga las restricciones en el schedule
bool STP::ClosureSchedule(queue <size_t> & q) {
  bool revisei, revisej;
  size_t c;
  bool consistent_schedule = true;

  // Bucle principal
  while (!q.empty() and consistent_schedule) {
    c = q.front();
    q.pop();
    consistent_schedule = ReviseSchedule (c, revisei, revisej);

    // Propagacion al resto de timepoints
    for (size_t i=0; i<Constraints.size(); i++)
      if (i != c) {
    if (revisei and
        (Constraints[c].i == Constraints[i].i || Constraints[c].i == Constraints[i].j))
        q.push(i);
    if (revisej and
        (Constraints[c].j == Constraints[i].i || Constraints[c].j == Constraints[i].j))
        q.push(i);
      }
  }
  return consistent_schedule;
}


bool STP::Schedule(vector <pair<constr, constr> > &solution) {
  size_t i, j;
  bool result=true;
  queue <size_t> q; // Cola de restricciones afectadas por la (posible) asignaci�n

  // Si no se pasa una solucion parcial, se genera una nueva
  if (solution.empty()) {
    // Pone todos los elementos a -1
    for(i=0; i<Size(); i++)
      solution.push_back(pair<constr,constr>(-1,-1));
  }

  // El punto inicial es siempre la referencia absoluta 0
  solution[0]=pair<constr,constr>(0,0);
  // Selecciona los arcos que pueden verse alterados
  // Por cada punto fijo en solution[] se a�aden a la cola de propagaci�n
  // Todos los arcos que acaben o empiecen en un punto fijo
  // O solo los arcos que est�n activos para el valor que se haya fijado
  for (i=1; i<Size(); i++)
    if (solution[i].first >= 0) { // Si es un punto fijo
      // Pasarlo al schedule
      TimePoints[i].earliest = TimePoints[i].latest = solution[i].first;
      // A�ade todas las restricciones que sean adyacentes y que est�n activas
      for(j=0; j<Constraints.size(); j++)
    if (Constraints[j].i == (int) i || Constraints[j].j == (int) i)
    q.push(j);
    }
  // Propaga los cambios originados por la existencia de puntos fijos
  result=ClosureSchedule(q);

  // Ahora fija el resto de puntos y los propaga
  for(i=0; i<Size() && result; i++)
    if (solution[i].first==-1) {
      solution[i].first = TimePoints[i].earliest;
      solution[i].second = TimePoints[i].latest;
      for(j=0; j<Constraints.size(); j++)
    if (Constraints[j].i == (int) i || Constraints[j].j == (int) i)
    q.push(j);
      result = ClosureSchedule(q);
    }
  return result;
}


void STP::ResetSchedule() {
  for (size_t i=0; i<Size(); i++) {
    TimePoints[i].earliest=TimePoints[i].lower;
    TimePoints[i].latest = TimePoints[i].upper;
  }
}

size_t STP::SetLevel() {
    //cerr << "SetLevel();" << endl;
  history.push_back(pair<size_t,size_t>(TimePoints.size(), Constraints.size()));
  //for(int i=0;i<(int)Size();i++)
  //    cerr << i << " -- " << DPrint(Query(i).first) << "," << DPrint(Query(i).second) << endl;

  return getUndoLevels();
}

size_t STP::getUndoLevels(void) {
  return history.size();
}

void STP::UnSetLevel() {
    //cerr << "UnSetLevel();" << endl;
  if (history.size() > 1) {
    // Si la red es inconsistente por la ultima restriccion impuesta
    // deshacer este ultimo cambio antes de proceder
    if (!IsConsistent()) {
      constraints_processed --;
      Restore();
      consistent=true;
      Constraints.pop_back();
    }
    // Una vez restaurada al estado anterior a la inconsistencia
    // proceder al undo seg�n DYNASTP sobre una red consistente
    while (Constraints.size() > history.back().second)
      PopConstraint();
    while (TimePoints.size() > history.back().first)
      PopTimePoint();
    constraints_processed = Constraints.size();
    history.pop_back();
  }
}


void STP::PopTimePoint() {
  if (!TimePoints.empty())
    TimePoints.pop_back();
}

void STP::PopConstraint() {
  queue <size_t> sub_net_arcs;
  size_t c;
  int lb , ub;

  if (!Constraints.empty()) {
    c = Constraints.size()-1;
    lb = Constraints[c].lb;
    ub = Constraints[c].ub;
//     cerr << endl << "***(387) [" << Query(387).first << ", " << Query(387).second << "]" << endl;
//     cerr << "PopConstraint(): ";
//     cerr << Constraints[c].lb << " " << Constraints[c].ub << endl;
    // Si la constraint est� activa
    if (lb != NIL || ub != NIL) {
      Constraints.pop_back();
      if (lb != NIL) {
    TimePoints[lb].lower = 0;
    TimePoints[lb].lb = NIL;
    sub_net_arc_lb((size_t) lb,sub_net_arcs);
      }
      if (ub != NIL) {
    TimePoints[ub].upper = POS_INF;
    TimePoints[ub].ub = NIL;
    sub_net_arc_ub((size_t) ub,sub_net_arcs);
      }

//       PintaCola("Cola de revisi�n: ", sub_net_arcs);
      UpdateSubNetwork(sub_net_arcs);
//       cerr << "***(387) [" << Query(387).first << ", " << Query(387).second << "]" << endl;

    }
    else
      Constraints.pop_back();
  }
}


//Obtiene la subrama de lb que tiene como root a node
void STP::sub_net_arc_lb(size_t node, queue <size_t> &q) {
  size_t i;

  for (i=0; i< Constraints.size(); i++) {
    if (Constraints[i].i == (int) node) {
      q.push(i);
      if (Constraints[i].lb >= 0 && Constraints[i].lb != (int) node) {
    TimePoints[Constraints[i].lb].lower = 0;
    TimePoints[Constraints[i].lb].lb = NIL;
    Constraints[i].lb = NIL;
    sub_net_arc_lb(Constraints[i].j, q);
      }
    }
    if (Constraints[i].j == (int) node) {
      q.push(i);
      if (Constraints[i].lb >= 0 && Constraints[i].lb != (int) node) {
    TimePoints[Constraints[i].lb].lower = 0;
    TimePoints[Constraints[i].lb].lb = NIL;
    Constraints[i].lb=NIL;
    sub_net_arc_lb(Constraints[i].i, q);
      }
    }
  }
}

// Obtiene la subrama de ub que tiene como root a node
void STP::sub_net_arc_ub(size_t node, queue <size_t> &q) {
  size_t i;

  for (i=0; i< Constraints.size(); i++) {
    if (Constraints[i].i == (int) node)  {
      q.push(i);
      if (Constraints[i].ub >= 0 && Constraints[i].ub != (int) node) {
    TimePoints[Constraints[i].ub].upper = POS_INF;
    TimePoints[Constraints[i].ub].ub = NIL;
    Constraints[i].ub=NIL;
    sub_net_arc_ub(Constraints[i].j, q);
      }
    }
    if (Constraints[i].j == (int) node) {
      q.push(i);
      if (Constraints[i].ub >= 0 && Constraints[i].ub != (int) node) {
    TimePoints[Constraints[i].ub].upper = POS_INF;
    TimePoints[Constraints[i].ub].ub = NIL;
    Constraints[i].ub=NIL;
    sub_net_arc_ub(Constraints[i].i, q);
      }
    }
  }
}



void STP::UpdateSubNetwork(queue <size_t> & q) {
  bool revisei, revisej;
  size_t c;

  // Bucle principal
  while (!q.empty()) {
    c = q.front();
    q.pop();
    ReviseConstraints (c, revisei, revisej);

    // Propagacion al resto de timepoints
    for (size_t i=0; i<Constraints.size() && consistent; i++)
      if (i != c) {
    if (revisei and
        (Constraints[c].i == Constraints[i].i || Constraints[c].i == Constraints[i].j))
        q.push(i);
    if (revisej and
        (Constraints[c].j == Constraints[i].i || Constraints[c].j == Constraints[i].j))
        q.push(i);
      }
  }
}


void STP::PrintValues(ostream &co) {

  co << "Time Points (" << Size() << ")" << endl;
  for (size_t i=0; i<Size(); i++)
    co << "   (" << i << ")  [" << TimePoints[i].lower << "," << TimePoints[i].upper << "]  <" <<
      TimePoints[i].lb << "> <" << TimePoints[i].ub << ">" << endl;


  co << "Constraints (" << Constraints.size() << ")" << endl;
  for (size_t i=0; i<Constraints.size(); i++) {
    if (i < constraints_processed)
      co << "   (P)";
    else
      co << "   ---";
    co << " (" << i << ")  (" << Constraints[i].i << ", " << Constraints[i].j << ")   [" <<
      Constraints[i].lower << "," <<  Constraints[i].upper << "]  <" <<
       Constraints[i].lb << "> <" <<  Constraints[i].ub << ">" << endl;
  }
  co << "Levels (" << history.size() << ")" << endl;
  for (size_t i=0; i<history.size(); i++) {
    co << " (" << i << ")  [-" << history[i].first << "-" << history[i].second << "-]" << endl;
  }
}

void STP::toxml(ostream &co) const {
    XmlWriter * writer = new XmlWriter(&co);
    toxml(writer);
    writer->flush();
    delete writer;
}

void STP::toxml(XmlWriter * writer) const{
    // Marcamos el inicio de la descripci�n XML de la red
    writer->startTag("tcnm")
    ->addAttrib("size",(int)Size())
    ->addAttrib("pos_inf",(int)POS_INF)
    ->addAttrib("neg_inf",(int)NEG_INF);

    // A�adimos todas las restricciones que han sido impuestas en la red
    for(size_t i=0;i<Constraints.size();i++) {
    writer->startTag("constraint")
        ->addAttrib("p1",(int)Constraints[i].i)
        ->addAttrib("p2",(int)Constraints[i].j)
        ->addAttrib("lower",(int)Constraints[i].lower)
        ->addAttrib("upper",(int)Constraints[i].upper)
        ->endTag();
    }

    // Marcamos el final de la descripci�n XML de la red
    writer->endTag();
}

void PintaCola(string message, queue<size_t> q) {
  queue <size_t> copy(q);

  cerr << endl << message;
  while (!copy.empty()) {
    cerr << copy.front() << " - " ;
    copy.pop();
  }
  cerr << endl;
}
