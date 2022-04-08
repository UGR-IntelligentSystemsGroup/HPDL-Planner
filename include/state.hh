#ifndef STATE_HH
#define STATE_HH

#include "literal.hh"
#include "unifier.hh"
#include "goal.hh"
#include "termTable.hh"

using namespace std;

typedef pair<iscit, iscit> ISTable_range;
typedef pair<isit, isit> ISTable_mrange;

class State
{
public:
  /**
   * @brief A�ade un predicado a la tabla. No hace una copia, s�lo asigna el puntero, cuidad�n con esto
   * @param symbol el literal a a�adir.
   * @return true si se ha asignado correctamente. False en otro caso (es decir, si ya existe)
   */
  isit addLiteral(Literal * symbol);

  /**
   * @brief Cuenta el n�mero de predicados existentes con el nombre dado.
   * @param El nombre del predicado
   * @return El n�mero de predicados
   */
  int countElements(int id) const;

  /**
   * @brief Devuelve los iteradores necesarios para recorrer los predicados con
   * un determinado nombre (clave).
   * @description Puede haber varios predicados con el mismo nombre pero distinto
   * n�mero de argumentos. Esta funci�n sirve para recorrerlos.
   * @return el rango.
   */
  ISTable_range getRange(int id) const;

  inline ISTable_mrange getModificableRange(int id) {return Literaltable.equal_range(id);};

  /**
   * @brief Devuelve un iterador con el contenido del primer predicado.
   * @param range El rango devuelto por la funci�n getRange
   * @see getLiteral
   * @see getRange
   * @return un iterador
   */
  iscit getFirstLiteral(ISTable_range range);

  iscit getEndLiteral(ISTable_range range);

  inline iscit getBeginLiteral(void) const {return Literaltable.begin();};
  inline iscit getEndLiteral(void) const {return Literaltable.end();};

  /**
   * @brief Devuelve un iterador con el contenido del i-�simo predicado.
   * @param range El rango devuelto por la funci�n getRange
   * @param it, el iterador devuelto por una llamada anterior a getFirstLiteral o
   * a getNextLiteral
   * @see getLiteral
   * @see getFirstLiteral
   * @see getRange
   * @return un iterador
   */
  iscit getNextLiteral(ISTable_range range, iscit it);

  /**
   * @brief Devuelve el predicado apuntado por un iterador.
   * @description Hay que mantener especial cuidado de no alterar el puntero.
   * La memoria es liberada por la tabla de predicados.
   * @param it el iterador que deseamos referenciar 
   * @see getFirstLiteral
   * @return 0 en caso de que no haya predicado, o que sea el �ltimo,
   * en otro caso un puntero a �l.
   */
  const Literal * getLiteral(iscit it) const;

  /**
   * @brief No deterministicamente devuelve un predicado de la tabla de predicados que
   * coincida con el nombre, si �ste existe.
   * @description Hay que mantener especial cuidado de no alterar el puntero.
   * La memoria es liberada por la tabla de predicados.
   * @param name la clave del predicado
   * @return 0 en caso de que no se encuentre el predicado.
   */
  Literal * getModificableLiteral(int id);

  const Literal * getLiteral(int id) const;

  /**
   * @brief Borra el elemento apuntado por el iterador.
   * @param it El iterador
   * @return true en caso de fallo, false en otro caso
   */
  inline void deleteLiteral(isit it) {Literaltable.erase(it);};

  /**
   * @brief Devuelve el n�mero de elementos en la tabla de predicados
   */
  inline int size(void) {return Literaltable.size();}

  /**
   * @brief Pinta todas las claves de la tabla de predicados.
   */
  void printKeys(ostream *) const;

  /**
   * @brief Pinta el estado en la salida pasada como argumento.
   */
  void print(ostream *) const;

  /**
   * @brief Borra todos los elementos de la tabla de predicados (estado)
   */
  void deleteAll(void);

  virtual ~State(void);

  /**
    Ojo, no modifiques la estructura si no sabes bien lo que estas
    haciendo!!
  */
  inline ISTable * getLiteralTable(void) {return &Literaltable;};

  /*
   * Esta funci�n no hace nada. Simplemente recorre todos los elementos
   * del estado. Sirve para comprobar que los valores que se encuentran
   * en la tabla de literales son correctos, y detectar posibles fallos
   */
  void test(void) const;

  /**
   * Realiza un clon de este objeto
   **/
  State * clone(void) const;

protected:

  ISTable Literaltable;

};

#endif
