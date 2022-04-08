#ifndef CAUSALTABLE_HH
#define CAUSALTABLE_HH   

#include "constants.hh"
#include "causal.hh"
#include <tr1/unordered_map>
#include "undoCLinks.hh"

typedef std::tr1::unordered_map<const Task *, CLTable *> CLDictionary;
typedef CLDictionary::const_iterator cldcite;
typedef CLDictionary::iterator cldite;

typedef std::tr1::unordered_map<int, CLTable *> CLIDictionary;
typedef CLIDictionary::const_iterator cclid_ite;
typedef CLIDictionary::iterator clid_ite;

/**
 * Esta clase gestiona la tabla de vínculos causales entre acciones
 * del plan de forma que puedan ser accedidas de una forma fácil y
 * eficiente.
 *
 * Discusión sobre la eficiencia:
 *
 * Se usa una doble tabla hash para almacenar las estructura de vínculos
 * causales. Como key para acceder a las tablas hash se usa el puntero
 * de la tarea cuyos vínculos queremos consultar.
 * - La eficiencia de la inserción depende del número de vínculos causales
 *   consumidos por una tarea. Hay que crear nuevas entradas en las tablas hash
 *   y añadir las nuevas referencias.
 * - El borrado es el proceso más lento, ya que requiere el asegurarse
 *   de no cometer inconsistencias. Depende del número de vínculos a borrar.
 * - El acceso a los vínculos tanto producidos como consumidos es de tiempo
 *   constante (acceso hash). Recorrer los vínculos es o(n) donde n es el
 *   número de vínculos.
 */
class CausalTable{
    public:
	/**
	 * El destructor de la clase.
	 */
	~CausalTable(void);

	/**
	 * Borra todos los enlaces almacenados en la tabla.
	 * Libera toda la memoria.
	 */
	void clean(void);

	/**
	 * Añade un vínculo causal.
	 * NOTA: Esta función y en general esta estructura de datos están diseñadas
	 * específicamente para encajar dentro del proceso que sigue el planificador.
	 * Se espera que la CLTable pasada como argumento se haya generado durante un
	 * proceso de unificación, por lo tanto la memoria usada por esta estructura
	 * no será liberada (ni copiada) observar que se pasa como un puntero constante.
	 * Si el unificador libera la cltable en algún momento puede dejar la tabla
	 * de vínculos causales incosistente. Estos riesgos como siempre se toman
	 * por motivios de eficiencia.
	 * @param consumer es la acción que consume los enlaces. Puede ser una tarea 
	 * primitiva (lo más normal) aunque también podría ser una acción compuesta.
	 * @param t es la tabla que almacena los vínculos generados tras una unificación
	 * @return Una estructura con información para poder deshacer los cambios provocados
	 * sobre la estructura causal.
	 */
	UndoCLinks * addCausalLinks(const Task * consumer, CLTable * t);

	/**
	 * Borra los vínculos causales consumidos por la acción pasada
	 * como argumento. Esta operación podría generar inconsistencias en la estructura
	 * en cuyo caso se abortará.
	 * @param La acción consumidora de los vínculos.
	 * @return Devuelve true en caso de que se encontrara la tarea y se eliminara
	 * correctamente.
	 */
	bool eraseCausalLinks(const Task * consumer);

	/**
	 * Devuelve un vector con los vínculos causales que parten de la
	 * acción. null en caso de que no existan.
	 * @param p La acción
	 * @return el vector null si no tiene
	 */
	const CLTable * getProducedLinks(const PrimitiveTask * p) const;

	/**
	 * Devuelve un vector con los vínculos causales que consume la
	 * acción. null en caso de que no existan.
	 * @param p La acción
	 * @return el vector null si no tiene
	 */
	const CLTable * getConsumedLinks(const Task * p) const;

	/**
	 * Imprime la estructura causal por pantalla
	 * @param os Un flujo de salida.
	 */
	void print(ostream * os) const;

	/**
	 * Imprime los vínculos consumidos por una tarea dada
	 * en un flujo. 
	 * @param os Un flujo de salida.
	 */
	void printConsumed(ostream * os, const Task * t) const;

	/**
	 * Imprime los vínculos producidos por una tarea dada
	 * en un flujo. 
	 * @param os Un flujo de salida.
	 */
	void printProduced(ostream * os, const Task * t) const;

	/**
	 * Escribe la estructura causal en formato interpretable
	 * por dot, que puede ser visualizado de forma gráfica.
	 * @param os Un flujo de salida.
	 */
	void plot(ostream * os) const;

	/**
	 * Determina si hay una relación causal entre dos tareas.
	 */
	bool hasCausalLinks(const Task * a, const Task * b) const;

	/**
	 * Devuelve la lista de vínculos que usan un literal determinado.
	 */
	const CLTable * getLinks(const LiteralEffect * e) const;

        /**
         * Testea la consistencia de la estructura causal, util
         * para depuración, es un proceso lento
         **/
        void checkConsistency(void) const;

    protected:

	/** 
	 * Estrurcutra hash que almacena los vínculos causales producidos 
	 * por una determinada tarea.
	 */
	CLDictionary produced;

	/** 
	 * Estrurcutra hash que almacena los vínculos causales consumidos 
	 * por una determinada tarea.
	 */
	CLDictionary consumed;

	/**
	 * Esta estructura mantiene la estructura inversa a los vinculos
	 * consumidos, se accede por el identificador del literal y te
	 * da la lista de tareas que consumen ese vínculo.
	 */
	CLIDictionary invConsumed;

};

/**
 * La tabla de vínculos causales es declarada como una variable global
 * a todo el programa. Incluyendo este fichero se tiene acceso inmediato
 * a dicha tabla
 */
extern CausalTable causalTable; 

#endif /* CAUSALTABLE_HH */
