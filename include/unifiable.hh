#ifndef UNIFIABLE_HH
#define UNIFIABLE_HH

#include "undoElement.hh"

using namespace std;

/** Esta clase trata de representar una interfaz com�n para todos aquellos objetos que
 * sobre los cuales se puede aplicar el resultado de una unificaci�n.
 */
class Unifiable
{
    public:

	virtual ~Unifiable(void) {};

	/**
	 * Realiza un renombrado de las variables, al mismo tiempo va aplicando
	 * las sustituciones de variables que hay en u.
	 * @param u Estructura para ir almacenando las sustituciones que vamos realizando
	 * @param undo Estructura para ser capaces de deshacer los cambios que se van
	 * haciendo durante el renombrado. Puede ser null.
	 */
	virtual void renameVars(Unifier * u,VUndo * undo){};

	/** Dado el nombre de una variable o una constante, devuelve su identificador asociado
	 * ��No busca t�rminos de tipo number!!. Si se devuelve pair<-1,...> no debe interpretarse
	 * como un n�mero sino como que el elemento no ha sido encontrado.
	 * @param name el nombre del t�rmino a buscar
	 * @return la pkey del t�rmino
	*/
	virtual pair<int,float> getTermId(const char * name) const { return pair<int, float>(0,0.0); };

	/** devuelve true si el objeto contiene el identificador pasado como argumento, en
	 * alguna de sus variables o constantes.
	 * @param id El identificador del t�rmino buscado.
	 * @return true si contenemos el identificador de t�rmino buscado.
	 */
	virtual bool hasTerm(int id) const { return false; };
};

#endif
