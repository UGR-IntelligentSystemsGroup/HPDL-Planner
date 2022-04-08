#ifndef UNIFY_HH
#define UNIFY_HH

#include "constants.hh"
#include "termTable.hh"

/** Esta funci�n verifica si hay unificaci�n entre dos vectores de keys.
 Es asim�trica, es decir, el orden de los par�metros afecta al resultado.
 Para que se produzca unificaci�n el tipo de un elemento de v1 debe ser
 subtipo de un tipo del elemento con el que estamos comparando de v2.
 /param v1 primer vector
 /param v2 segundo vector
 */
bool unify(const vector<pkey> * v1, const vector<pkey> * v2);
bool unify(const vector<Term *> * v1, const vector<pkey> * v2);
/** Esta variante devuelve una tabla de unificaciones con las sustituciones que
 es necesario aplicar para realizar la unficaci�n.
 Esta unificaci�n supone que los elementos del vector v2 son constantes o n�meros.
 La funci�n no altera u si no hay una unificaci�n correcta.
 Las substituciones de variables son del tipo substituye elemento de v1 por elemento de v2
 No hay sustituci�n de tipos.
 Si hay alguna sustituci�n pendiente en u, sobre una variable que vamos a usar se aplica
 con anterioridad.
 /param v1 primer vector
 /param v2 segundo vector
 /param u unificador
*/
bool unify(const vector<pkey> * v1, const vector<pkey> * v2, Unifier * u);
/** Por �ltimo esta �ltima variante permite hacer unificaciones con un vector v2
 en el que pueden existir variables. �Ojo! es menos eficiente que las anteriores
 y nuevamente no es lo mismo escribir unify(v1,v2) que unify(v2,v1), nuevamente los elementos
 de v1 elemento a elemento deben ser de un subtipo del elemento de v2.
 La funci�n puede alterar el contenido de u incluso si no hay una unificaci�n v�lida.
 Las substituciones de tipos afectan a las variables de v2.
 Las substituciones de variables son del tipo substituye elemento de v1 por elemento de v2
 /param v1 primer vector
 /param v2 segundo vector
 /param u unificador
 */
bool unify2(const vector<pkey> * v1, const vector<pkey> * v2, Unifier * u);
/** esta unificaci�n es un poco especial. 
 Se suponen que todos los elementos de v2 son variables.
 Si el elemento que toca de v1 es una constante tiene que ser subtipo del elemento de v1.
 Si el elemento que toca de v1 es una variable, se realiza la intersecci�n de sus tipos.
 Esta unificacion solo almacena substituciones de tipos en u.
 Las substituciones de tipos afectan a las variables de v1.
 El contenido de u puede ser alterado incluso si no hay una unificaci�n v�lida.
 /param v1 primer vector
 /param v2 segundo vector
 /param u unificador
 */
bool unify3(const vector<pkey> * v1, const vector<pkey> * v2, Unifier * u);

#endif
