#include <iomanip>
#include <sstream>
#include <iostream>
#include <deque>
#include "tcnm-pc2.hh"

using namespace std;

string DPrint(int i){
    ostringstream os;
    if(i == POS_INF){
	return "inf";
    }
    if(i == NEG_INF){
	return "-inf";
    }
    os << i;
    return os.str();
};

// Devuelve el valor imprimible de una restriccion pero teniendo en cuenta +oo y -oo
string PrintTConstraint(constr t);

// Equivalente a la suma pero que tiene en cuenta +oo y -oo
#define Plus(a,b) (a==POS_INF || b==POS_INF)? POS_INF : (a==NEG_INF||b==NEG_INF)? NEG_INF: a+b

STP::STP() {
   consistent=true;
   propagation_method=FLOYD;
   last_i = last_j = -1;
}


STP::~STP() {
  for(size_t i=0; i<Size(); i++) net[i].clear();
  net.clear();
}


bool STP::IsConsistent() {
  return consistent;
}

struct AddNewConstraint{
    constr t;
    void operator()(vector<constr> & v){
	v.push_back(POS_INF);
    }
};

int STP::InsertTPoint() {
   //cerr << "InsertTPoint();" << endl;
  vector <constr> p;
  constr tc=POS_INF;

  // A�ade una columna nueva desordenada con respecto al nuevo punto
  for_each(net.begin(),net.end(),AddNewConstraint());

  // A�ade una fila nueva
  net.push_back(p);
  net.back().insert(net.back().begin(),net.size(),tc);

  //Pone el valor net[nuevo][nuevo] a <UNORDERED,0>
  //setValue(Size()-1,Size()-1,0);
  net[Size()-1][Size()-1] = 0;
  return net.size();
}

bool STP::AddTConstraint(int i, int j, constr lower, constr upper, bool record) {
  //cerr << i << " " << j << " " << lower << " " << upper << endl;
    constr l, u;

    l=getmin(-lower, Query(j,i));
    u=getmin(upper, Query(i,j));
    if (-l <=  u && consistent) {
	setValue(i,j,u);
	setValue(j,i,l);
	// Detecta inconsistencias inmediatas.
	last_i = i;
	last_j = j;
	return true;
    }
    else
	return false;
}


// Propagaci�n de restricciones
bool STP::Closure() {
  //cerr << "stp->Closure();" << endl;
  bool res=false;
  switch (propagation_method) {
  case FLOYD:
    res = FloydWarshall();
    break;
  case PC2:
    res = PathConsistency2();
    break;
  }
  //if(!res) cerr << "cerr << \"Fallo en 2\" << endl; " << endl;
  return res;
}

// Propagaci�n de restricciones exacta
bool STP::FloydWarshall() {
  constr aux;

  consistent = true;
  for(size_t k=0; k<Size() && consistent; k++)
    for(size_t i=0; i<Size() && consistent; i++)
      for (size_t j=0; j<Size() && consistent; j++) {
	aux=getmin(Query(i,j),Plus(Query(i,k),Query(k,j)));
	setValue(i,j,aux);
	// Cuando detecta una inconsistencia sale inmediatamente sin esperar a que los bucles terminen
	// Mantiene el dato miembro de consistencia actualizado
	consistent = -Query(j,i) <= Query(i,j);
      }
  return consistent;
}

// Propagaci�n aproximada: path-consistency Algoritmo PC-2, incremental
//
// oscar: Modificaciones a�adidas
// El objetivo es quitar el mayor n�mero de instrucciones dentro del bucle,
// para ello o bien las saco, o bien trato de simplificarlas
// indudablemente la legibilidad del algoritmo queda da�ada, pero se consiguen
// rascar algunos valiosos segundos
//
bool STP::PathConsistency2() {
    vector<pair<int,int> > queue; // Cola de caminos
    unsigned int ite;

    int i, j, k, s; // Indices
    bool sumar;
    constr l1,u1,l2,u2,l3,u3,laux,uaux; // Intervalos

    // el tama�o de la red es siempre el mismo
    // el �ndice k al inicializarse siempre vale lo mismo
    s=Size();

    // Busca cu�l es la primera restricci�n que no est� actualizada
    queue.push_back(make_pair(last_i,last_j));
    ite = 0;

    // Inicializa la cola de caminos pendientes q
    // Inserta los caminos a revisar (k,i,j) y (k,j,i), k!= i != j
    while(ite < queue.size()){
	j = queue[ite].first;
	k = queue[ite].second;
	i= 0;
	//cerr << "nuevo" <<  endl;
	while(i == k || i== j) i++;
	sumar = false;

	while (i < s) {
	    if(!sumar){
		swap(k,j);
		sumar = true;
	    } else {
		swap(k,j);
		sumar = false;
	    }

	   // cerr << i << " " << j << " " << k << " " << endl;
	   // getchar();

	    u1 = Query(i,j);
	    l1 = -Query(j,i);
	    u2 = Query(i,k);
	    l2 = -Query(k,i);
	    u3 = Query(k,j);
	    l3 = -Query(j,k);
	    // [l2,u2] = [l2,u2]+[l3,u3]
	    u2 = Plus(u2,u3);
	    l2 = Plus(l2,l3);
	    // [laux,uaux] = [l1,l2] \cap [l2,u2]
	    uaux = getmin(u1,u2);
	    laux = getmax(l1,l2);
	    // Si la intersecci�n es nula, entonces inconsistencia
	    if (uaux < laux)
		consistent=false;
	    else {
		// Si [laux,uaux] cambia con respecto a [l1,u1] entonces actualizar y encolar nuevas propagaciones
		setValue(i,j,uaux);
		setValue(j,i,-laux);
		//cerr << i << ": " << j << ": " << uaux << endl;
		//cerr << j << ": " << i << ": " << laux << endl;;
		//cerr << u1 << "::" << l1 << endl;
		if (uaux != u1 || laux != l1) {
		    queue.push_back(make_pair(i,j));
		    //cerr << i << " -- " << j << endl;
		}
	    }
	    if(!sumar){
		i++;
		while(i == k || i== j) i++;
	    }
	    //    ite++;
	}
	ite++;
    }
    //cerr << Size() << " tps, Iteraciones: " << ite << endl;
    //last_update++;
    // Devuelve el flag interno de consistencia
    return consistent;
}

/*
// Propagaci�n aproximada: path-consistency Algoritmo PC-2, incremental
bool STP::PathConsistency2() {
  deque <int> q1, q2, q3; // Cola de caminos
  size_t i, j, k; // Indices
  constr l1,u1,l2,u2,l3,u3,laux,uaux; // Intervalos

      i = last_i;
      j = last_j;
      // Inicializa la cola de caminos pendientes q
      // Inserta los caminos a revisar (k,i,j) y (k,j,i), k!= i != j
      for (k=0; k<Size(); k++)
	if (k != i && k != j) {
	  // A�ade (k,i,j)
	  q1.push_back(k);
	  q2.push_back(i);
	  q3.push_back(j);
	  // A�ade (k,j,i)
	  q1.push_back(k);
	  q2.push_back(j);
	  q3.push_back(i);
	}
      // Bucle de propagaci�n PC-2
      while (!q1.empty() && consistent) {
	//cerr << "PC2-Iteration"<<endl;
	// Extrae un camino de la cola
	i=q1.front();
	k=q2.front();
	j=q3.front();
	    //cerr << i << " " << j << " " << k << " " << endl;
	    //getchar();
	q1.pop_front();
	q2.pop_front();
	q3.pop_front();
	// Comprueba si las restricciones cambian
	u1 = Query(i,j);
	l1 = -Query(j,i);
	u2 = Query(i,k);
	l2 = -Query(k,i);
	u3 = Query(k,j);
	l3 = -Query(j,k);
	// [l2,u2] = [l2,u2]+[l3,u3]
	u2 = Plus(u2,u3);
	l2 = Plus(l2,l3);
	// [laux,uaux] = [l1,l2] \cap [l2,u2]
	uaux = min(u1,u2);
	laux = max(l1,l2);
	// Si la intersecci�n es nula, entonces inconsistencia
	if (uaux < laux)
	  consistent=false;
	else {
	  // Si [laux,uaux] cambia con respecto a [l1,u1] entonces actualizar y encolar nuevas propagaciones
	  setValue(i,j,uaux);
	  setValue(j,i,-laux);
		//cerr << i << ": " << j << ": " << uaux << endl;
		//cerr << j << ": " << i << ": " << laux << endl;;
		//cerr << u1 << "::" << l1 << endl;
	  if (uaux != u1 || laux != l1) {
	    for (size_t l=0; l< Size(); l++)
	      if (l != i && l != j) {
		// A�ade (l,i,j)
		q1.push_back(l);
		q2.push_back(i);
		q3.push_back(j);
		// A�ade (l,j,i)
		q1.push_back(l);
		q2.push_back(j);
		q3.push_back(i);
	      }
		//cerr << i << " -- " << j << endl;
	  }
	} // if (uaux < laux) else
      }
  return consistent;
}*/

bool STP::Schedule(vector <pair<int,int> > &solution) const {
  size_t i, j;
  constr lower, upper;
  bool result=true;

  // Si no se pasa una solucion parcial, se genera una nueva
  if (solution.empty()) {
    // Pone todos los elementos a -1
    for(i=0; i<Size(); i++)
      solution.push_back(make_pair(-1,-1));
  }
  // El punto inicial es siempre la referencia absoluta 0
  solution[0]=make_pair(0,0);
  // Calcula la interseccion
  for (i=1; i<Size() && result; i++) {
    lower=NEG_INF;
    upper=POS_INF;
    for (j=0; j<Size();j++)
      if (solution[j].first>=0) {
	upper = min(upper,Plus(solution[j].first,Query(j,i)));
	lower = getmax(lower,Plus(solution[j].first,-Query(i,j)));
      }
    result = (lower <= upper);
    solution[i]=make_pair(lower,upper); // Seg�n el m�todo (eet, let, etc...)
  }
  return result;
}

// Imprime valores m�tricos
void STP::PrintValues(ostream &co, bool classic) {
  for (size_t i=0; i<Size(); i++) {
    for (size_t j=0; j<Size(); j++)
      if (classic)
	co << PrintTConstraint(Query(i,j));
      else
	co << "[" << setw(3) << PrintTConstraint(-Query(j,i)) << "," << PrintTConstraint(Query(i,j)) << "]";
    co << endl;
  }
}


string PrintTConstraint(constr t) {
  ostringstream s;

  switch(t) {
  case POS_INF:
    s << "+oo";
    break;
  case NEG_INF:
    s << "-oo";
    break;
  default:
    s << setw(3) << t;
  }
  return s.str();
}

bool STP::Overlap(int tp1_1, int tp1_2, int tp2_1, int tp2_2) {
  int l1,u1,l2,u2;
  l1 = -Query(tp1_2,tp1_1);
  u1 = Query(tp1_1,tp1_2);
  l2 = -Query(tp2_2,tp2_1);
  u2 = Query(tp2_1,tp2_2);
  if (u1 < l2 || u2 < l1)
    return false;
  else
    return true;
}


void STP::SetLevel(void){
    //cerr << "SetLevel();" << endl;
    // crear las filas, sin inicializar
    AdjMatrix * clon = new AdjMatrix(net.size());
    AdjMatrix::iterator i,j, e =clon->end();
    for(i = clon->begin(), j = net.begin(); i != e; i++,j++)
	(*i).insert((*i).begin(),(*j).begin(),(*j).end());
    history.push(clon);
    //PrintValues(cerr,true);
//    for(int i = 0; i < (int) Size(); i++)
//	cerr << i << " -- " << DPrint(-Query(i,0)) << "," << DPrint(Query(0,i)) << endl;
};

void STP::UnSetLevel(void){
    //cerr << "UnSetLevel();" << endl;
    if(!history.empty()){
	AdjMatrix * m = history.top();
	net.swap(*m);
	history.pop();
	delete m;
    }
    //PrintValues(cerr,true);
}

bool STP::AddTConstraint2(int i, int j, constr lower, constr upper, bool record)
{
    //cerr << "if(!stp->AddTConstraint(" << i << "," << j<< "," << lower<< "," << upper<< "," << record << "))" << endl;
    //cerr << "\t{cerr << \"Fallo en 1\";}" << endl;
  //cerr << "AddTConstraint2(" << i << "," << j << "," << DPrint(lower) << "," << DPrint(upper) << ");" << endl;
    if(!AddTConstraint(i,j,lower,upper,record)){
	//cerr << "FALLE" << endl;
	return false;
    }
    if(!Closure()){
	//cerr << "FALLE" << endl;
	return false;
    }
    return true;
};
