#ifndef __tcnm_pc2_cl
#define __tcnm_pc2_cl

// Headers
#include "constants.hh"
#include <climits>
#include <vector>
#include <ostream>
#include <vector>
#include <stack>
#include "xmlwriter.hh"

//
using namespace std;

// Types
typedef int constr;

// Funciones auxiliares
//inline constr min(constr t1, constr t2) {if (t1 < t2) return t1; else return t2;};
//inline constr max(constr t1, constr t2) {if (t1 > t2) return t1; else return t2;};
#define  getmin(t1,t2)  t1 < t2 ? t1 : t2
#define  getmax(t1,t2)  t1 > t2 ? t1 : t2


// Types of propagation
#define FLOYD 0
#define PC2 1

// Predefined values
#define MAXBOUND INT_MAX
#define NEG_INF -MAXBOUND
#define POS_INF MAXBOUND

const int LEVEL=-2;
const int POINT=-1;

// Undos
struct undo_record {
  int i, j, value;
};

typedef vector<constr> AdjMatrixRow;
typedef vector<AdjMatrixRow> AdjMatrix;

class Task;
typedef vector<const Task *> InvRef;

/**
   @author Luis
   @brief Manejo de temporal constraint networks. Recoge dependencias cualitativas y cuantitativas entre time points. Cada time point se supone con un �ndice entero (i=0, ..., n-1) donde n=n�mero de time points de la red. Por tanto las relaciones entre time points est�n referidas al uso de dos �ndices en una matriz de adyacencia cuadrada (i,j) donde cada posici�n es un dato de tipo TConstraint.
 */
class STP{
 public:
  STP();
  ~STP();
  /**
     @author Luis
     @brief Devuelve el tama�o de la red, es decir, el n�mero de time points
     @return Un n�mero entero (size_t)
   */
  inline size_t Size(void) const {return net.size();};

  /**
     @author Luis
     @brief Devuelve una referencia al elemento i-j de la matriz de adyacencia.
     @param (i, j) :  La posici�n de la red que se est� consultando, es decir, la dependencia que existe entre el time point @a i y el time point @a j
     @return Una referencia a un TConstraint situado en (i,j)
   */
  inline constr Query(int i, int j) const {return net[i][j];};

  inline void setValue(int i, int j, constr value){net[i][j]=value;};

  /**
     @author Luis
     @brief Comprueba la consistencia de las restricciones registradas en la red. Hay que tener en cuenta que algunas restricciones pueden estar induciendo alguna violaci�n que no se detectar� hasta que no se propaguen. Para que este resultado sea absolutamente fiable antes hay que propagar las restricciones, �sto se hace manualmente llamando a la funci�n STP::Closure()
     @return @a true si la red es consistente, @a false si es inconsistente
   */
  bool IsConsistent();

  /**
     @author Luis
     @brief Inserta un nuevo time point en la red. Para ello amplia la matriz de adyacencia en una fila m�s y en una columna m�s. Los nuievos datos se rellenan de la siguiente forma.
     - matriz[@a i][@a nuevacolumna] = (-oo, +oo), es decir, est�n desordenados
     - matriz[@a nuevafila][i] = (-oo, +oo)
     - matriz[@a nuevafila][@a nuevafila] = [0,0] La diagonal principal de la matriz siempre es [0,0]
     @param t Es la tarea que posee el time-point. Es necesario para hacer referencias
     inversas (de time-point a tarea)
     @return El �ndice que ocupa el nuevo time point insertado
   */
  int InsertTPoint(const Task * t);

  /**
     @author Luis
     @brief Inserta una nueva restricci�n temporal entre dos time points. Intersecta esta restricci�n con la que ya hubiese definida entre (i,j)
     @param i,j Los time points en cuesti�n
     @param [lower,upper] Los l�mites de la restricci�n temporal de forma que
     - matriz[i][j] = upper
     - matriz[j][i] = -lower
     Esto significa que la relaci�n m�trica entre los time points (i,j) est� restringida al intervalo num�rico [lower,upper]. Por tanto la relaci�n dual entre (j,i) est� en [-upper,-lower]
     @param record (opcional) determina si se quiere registrar esta restricci�n o no. La razon por la que he metido esto es para poder restaurar la relacion de orden despu�s de hacer un UNDO, pero es una utilidad general. Por omisi�n vale @a true
     @return Esta funcion se puede utilizar para detectar inconsistencias temporales inmediatas en el caso de que la
     restriccion que se est� poniendo sea inconsistente con la que exist�a entre el mismo par de puntos temporales.
     Es una detecci�n muy r�pida, pero, obviamente no detecta todas las inconsistencias,
     es decir, podemos poner una restricci�n consistente entre el par de puntos temporales dados,
     pero que al propagar produzca una inconsistencia. Este tipo de inconsistencias no las detecta.
     Por ejemplo, supongamos que entre los puntos @a i y @a j existe la restriccion @e [l,u] y que
     se pone la restricci�n @e [l2,u2] de forma que la intersecci�n de @e [l,u] y de @e [l2,u2] es vac�a.
     Esto es una inconsistencia immediata que no se puede reparar por mucho que se propague.
     Pues bien, este es el tipo de inconsistencias detectadas por este m�todo.
     Devuelve @a true si no se ha detectado una inconsistencia inmediata y false si se ha detectado.
     �� OJO !! Si esta funci�n devuelve true no significa que la red sea consistente.
     Para tener completa certeza de que la red es consistente primero hay que propagar todas
     las restricciones con STP::Closure()
   */
  bool AddTConstraint(int i, int j, constr lower, constr upper, bool record=true);

  bool AddTConstraint2(int i, int j, constr lower, constr upper, bool record=true);

  /**
     @author Luis
     @brief Define el m�todo de propaci�n de restricciones que ser� utilizado posteriormente por STP::Closure(). De esta forma se pueden construir algoritmos completos que se propaguen de una forma u otra sin m�s que cambiar el m�todo de propagaci�n. Por ahora hay dos m�todos
     - FLOYD. Metodo exacto, es el algoritmo de Floyd Warshall, de orden O(n^3) n=n�mero de time points. Este m�todo se puede utilizar en cualquier momento, es decir, se pueden insertar un conjunto de restricciones consecutivas y, solo al final, propagarlas con un resultado correcto
     - PC2. M�todo aproximado, incremental y muy r�pido. Es el algoritmo Path-Consistency-2 (PC-2). Para que el resultado de esta propagaci�n sea correcto es necesario que se propagen las restricciones cada vez que se inserta una nueva restricci�n, ya que, de otro caso, el resultado ser� incorrecto.
  */
  void setPropagationMethod(int method){propagation_method=method%2;}

  /**
     @author Luis
     @brief Consultar el m�todo de propagaci�n establecido
     @return FLOYD o PC2, seg�n sea el m�todo de propagaci�n que se haya establecido.
  */
  int getPropagationMethod(){return propagation_method;}

  /**
    @author Luis
    @brief Calcula el cierre transitivo de la red de restricciones, es decir, propaga las restricciones.
    Esta es la �nica funci�n que propaga restricciones, el resto solo las consultan o las modifican individualmente.
    Por tanto, en un proceso normal es necesario a�adir restricciones (con STP::InsertTConstraint o con STP::AddTConstraint)
    y, manualmente, llamar a STP::Closure. Existen dos formas de realizar la propagaci�n.
    @return @a true si la red es consistente y @a false si la red es inconsistente.
    Independientemente del m�todo de propagaci�n utilizado (ver STP::setPropagationMethod())
    el resultado que se devuelve es correcto.
   */
  bool Closure();

  /**
     @author Luis
     @brief Algoritmo de consistencia local, path-consistency PC-2. Es una condici�n necesaria para la consistencia, es decir, si este algoritmo devuelve @a true es que la red es consistente, aunque es posible que las restricciones sean m�s cerradas de lo que devuelve el algoritmo (recordad que es un algoritmo aproximado). Pero devuelve @a false (inconsistencia) si y solo si la red es realmente inconsistente. Adem�s de ser m�s r�pido que STP::Closure(), permite el crecimiento incremental, es decir, cada vez que se a�ade una restricci�n no es necesario propagar todas las restricciones, sino solamente las que cambian.
     @return @a true si la propagaci�n ha sido correcta, @a false si se ha detectado una inconsistencia
   */
  bool PathConsistency2();

  /**
      @author Luis
      @brief Algoritmo de propagaci�n de FLoyd Warshall (all-pairs-shortest-path). Es exacto.
     @return @a true si la propagaci�n ha sido correcta, @a false si se ha detectado una inconsistencia
  */
  bool FloydWarshall();

  /**
     @author Luis
     @brief Procedimiento de extracci�n de soluciones para un STP. Se puede utilizar para extraer soluciones (scheduling) o para completar una solucion parcialmente extraida (rescheduling). Para ello se utiliza el vector @a solution. Si esta vacio lo rellena del todo, si est� parcialmente lleno rellena solo lo que est� vacio (una posici�n esta vacia si tiene un valor -1, y est� asignada si tiene un valor >= 0)
     @return @a true si se ha podido extraer una solucion o @a false si no existe solucion
   */
  bool Schedule(vector <pair<int,int> > &solution) const;


  /**
    @author Luis
    @brief Imprime la red de restricciones m�trica en el stream que se pasa como argumento (se supone de texto).
    @param co El stream de salida al que va la impresi�n
    @param classic Determina el tipo de salida.
    - @a classic = @a true La salida es simplificada, es decir, matriz[i][j] \\in (-oo,+oo)
    - @a classic = @a false La salida es extendida, es decir, matriz[i][j] = [-matriz(j,i), +matriz(i,j)]
    */
  void PrintValues(ostream &co, bool classic=false);

  /**
    @author Luis
    @brief Esta funci�n detecta si dos intervalos especificados por los time points [tp1_1,tp1_2] y [tp2_1,tp2_2] se solapan
    @param   tp1_1
    @param   tp1_2 Los �ndices de los time points del primer intervalo
    @param tp2_1
    @param tp2_2 Los �ndices de los time points del segundo intervalo
    @return @a true si los dos intervalos se solapan, aunque sea en un �nico punto, y @a false si son disjuntos.
    */
  bool Overlap(int tp1_1, int tp1_2, int tp2_1, int tp2_2);

  void SetLevel(void);
  void UnSetLevel(void);
  inline int getUndoLevels(void) const {return history.size();};

  /**
   * Establecer el flag que permite utilizar la informaci�n de contexto
   * (v�nculos causales) para podar las propagaciones que se realizan
   * en la red de tareas.
   */
  inline void useContextInfo(bool b = true) {FlagContextInfo = b;};

  /**
    @author Palao
    @brief Nos devuelve una descripci�n XML de la red
    @param co El stream de salida al que va la impresi�n
    */
  void toxml(ostream &co) const {cerr << "STP::toxml No implementado" << endl;};

  void toxml(XmlWriter * writer) const {cerr << "STP::toxml No implementado" << endl;};

private:
  stack<AdjMatrix *> history;
  AdjMatrix net;  // La matriz de adyacencia de la relaci�n de orden
  bool consistent; // Flag de consistencia
  int propagation_method;  // M�todo de propagaci�n
  int last_i, last_j;
  bool FlagContextInfo; // Usar informaci�n de contexto para podar las propagaciones
  InvRef table; // tabla para mantener las referencias inversas, (de tp a tarea)
  vector<vector<int> > * constraints; // Este vector mantiene las parejas de timepoints sobre los cuales
  // se ha definido expl�citamente una restricci�n.
  vector<vector<vector<int> > * > constHistory;
};


#endif
