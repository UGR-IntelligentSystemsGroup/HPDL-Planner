#ifndef __tcnm3
#define __tcnm3

// Headers
#include <climits>
#include <vector>
#include <ostream>
#include <queue>
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


// Predefined values. By default infinity,
// but they might be used with the max known temporal horizont
#define MAXBOUND INT_MAX/2
#define NEG_INF -MAXBOUND
#define POS_INF MAXBOUND

// Constants for dependencies
#define NIL -1

// Tipos intermedios
typedef struct{
  int i, j;  // Indices de los time points
  constr lower, upper; // Valores de la restriccion
  int lb, ub;  // Cursores a las cadenas de dependencias lb y ub (indices de time points)
  constr blower, bupper; // Backups de los valores clave
  int blb, bub;
} constraint_row;

typedef struct {
  constr lower, upper; // Valores de STP[0, timepoint]
  int lb, ub;  // Cursores a las cadenas de dependencias (indices de restricciones)
  constr earliest, latest; // Valores resultantes del schedule
  int blb, bub;
  constr blower, bupper;
} timepoint_row;

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
  inline size_t Size(void) const {return TimePoints.size();};

  /**
     @author Luis
     @brief Devuelvelos l�mites [lower_i, upper_i] del time point i-esimo
     @param (i) :  El �ndice que se est� consultando
     @return Un par de valores [\a lower_i, ,\a upper_i]
   */
  pair<constr,constr> Query(int i);
  constr Query(int i, int j);


  /**
     @author Luis
     @brief Comprueba la consistencia de las restricciones registradas en la red. Hay que tener en cuenta que algunas restricciones pueden estar induciendo alguna violaci�n que no se detectar� hasta que no se propaguen. Para que este resultado sea absolutamente fiable antes hay que propagar las restricciones, �sto se hace manualmente llamando a la funci�n STP::Closure()
     @return @a true si la red es consistente, @a false si es inconsistente
   */
  bool IsConsistent();

  /**
     @author Luis
     @brief Resetea la red despu�s de haberse detectado una inconsistencia.
     Pone todos los ub y lb a NIL, los lower a 0 y los upper a POS_INF,
     Luego ser� necesario llamar a STP::Closure()
   */
  void Reset();

  void Backup();
  void Restore();

  /**
     @author Luis
     @brief Inserta un nuevo time point en la red. Los l�mites que tiene por defecto este nuevo time point son [0, +oo)
     @return El �ndice que ocupa el nuevo time point insertado
   */
  int InsertTPoint();

  /**
     @author Luis
     @brief Inserta una nueva restricci�n temporal entre dos time points. Una vez a�adida una o m�s restricciones, ser� necesario propaparlas con STP::Closure()
     @param i,j Los time points en cuesti�n
     @param [lower,upper] Los l�mites de la restricci�n temporal
   */
  void AddTConstraint(int i, int j, constr lower, constr upper);
  bool AddTConstraint2(int i, int j, constr lower, constr upper, bool record=true);


  /**
    @author Luis
    @brief Calcula el cierre transitivo de la red de restricciones, es decir, propaga las restricciones.
    @return @a true si la red es consistente y @a false si la red es inconsistente.
   */
  bool Closure(); // Solo propagar las ultimas restricciones

  /**
     @author Luis
     @brief Esta funci�n resetea los valores l�mite de un schedule. Se debe llamar SIEMPRE antes que la funcion STP::Schedule() bien sea para scheduling como para rescheduling
   */
  void ResetSchedule();

  /**
     @author Luis
     @brief Procedimiento de extracci�n de soluciones para un STP. Se puede utilizar para extraer soluciones (scheduling) o para completar una solucion parcialmente extraida (rescheduling). Una soluci�n asocia un par de valores <e,l> a cada timepoint. El valor @a e es el earliest time y el @l el lates time. Para ello se utiliza el vector @a solution. Si esta vacio lo rellena del todo, si est� parcialmente lleno rellena solo lo que est� vacio (una posici�n esta vacia si contiene un par <-1,-1>, y est� asignada si contiene un par <e,l> e,l >= 0)
     @return @a true si se ha podido extraer una solucion o @a false si no existe solucion
   */
  bool Schedule(vector <pair<constr, constr> > &solution);

  /**
     @author Luis
     @brief Establece un nuevo nivel de deshacer
     @return El nivel establecido
   */
  size_t SetLevel(void);
  /**
     @author Luis
     @brief Deshace las �ltimas modificacions hasta el nivel anterior de desahcer. Hay que decir que no se replica memoria y que los cambios de deshacer son incrementales.
   */
  void UnSetLevel(void);

  /**
     @author Luis
     @brief devuelve el n�mero de niveles de deshacer que haya activos. Por defecto siempre hay uno, que es el que restablece el STP a su estadio inicial (sin puntos ni restricciones)
   */
  size_t getUndoLevels(void);


  /**
    @author Luis
    @brief Imprime la red de restricciones m�trica en el stream que se pasa como argumento (se supone de texto).
    @param co El stream de salida al que va la impresi�n
    */
  void PrintValues(ostream &co);

  /**
    @author Palao
    @brief Nos devuelve una descripci�n XML de la red
    @param co El stream de salida al que va la impresi�n
    */
  void toxml(ostream &co) const;

  void toxml(XmlWriter * writer) const;

  // Funciones de propagaci�n privadas
  //
  // MOVER A private:  !!
  //
  bool ReviseConstraints(size_t constraint, bool &bi, bool &bj);
  bool ReviseSchedule(size_t constraint, bool &bi, bool &bj);
  bool Closure(queue <size_t>& q); // Propagar un conjunto de restricciones
  bool ClosureSchedule(queue <size_t>& q); // Propagar un conjunto de restricciones durante el schedule
  // Comprueba que no hay ciclos lb/ub que contengan al nodo node
  bool Follow_lb(size_t node, vector <bool> &visited);
  bool Follow_ub(size_t node, vector <bool> &visited);
  // Extracci�n de sub�rboles de dependencias (necesario para undos)
  void sub_net_arc_lb(size_t node, queue <size_t> &q);
  void sub_net_arc_ub(size_t node, queue <size_t> &q);
  // Actualizaci�n de restricciones al undo
  void UpdateSubNetwork(queue <size_t> & q);
  // Quita la ultima restriccion
  void PopConstraint();
  // Quita el �ltimo Time point
  void PopTimePoint();


private:
  // Memoria de constraints
  vector <constraint_row> Constraints;
  // Memoria de time points
  vector <timepoint_row> TimePoints;
  // Hist�rico de niveles (#timepoints, #constraints)
  vector <pair <size_t, size_t> > history;

  bool consistent; // Flag de consistencia
  int propagation_method;  // M�todo de propagaci�n
  size_t constraints_processed;  // Se procesan las primeras
};


#endif
