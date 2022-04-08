#ifndef EXPANSIONTREE_HH
#define EXPANSIONTREE_HH   

#include "constants.hh"
#include <tr1/unordered_map>
#include "stacknode.hh"

using namespace std;

typedef struct NodeTaskTree{
    NodeTaskTree(const Task * t){
	task = t;
    };

    const Task * task;
    vector<const Task *> children;
}NodeTaskTree;

typedef std::tr1::unordered_map<long, NodeTaskTree *, std::tr1::hash<long>,equal_to<long> > TaskTreeDict;
typedef TaskTreeDict::iterator ttdite;
typedef TaskTreeDict::const_iterator ttdcite;

/**
 * Esta clase codifica el árbol de expansión, tras obtener un plan correcto después
 * del proceso de planificación.
 */
class ExpansionTree{
    public:
	/**
	 * Constructor de la clase. Se le pasan como parámetros, el plan
	 * obtenido, y la red de tareas resultante tras la expansión.
	 */
	ExpansionTree(const TPlan * plan, const TaskNetwork * tn);

	/**
	 * Destructor, libera la memoria ocupada por el objeto.
	 */
	~ExpansionTree();

	/**
	 * Devuelve el número de raíces del árbol de expansión.
	 */
	inline int getNumberOfRoots(void) const {return roots.size();};

	/**
	 * Devuelve la raíz iésima del árbol de expansión.
	 */
	inline const Task * getRoot(int i) const {if(i<0||i>=(int)roots.size()) return 0; return roots[i];};

	/**
	 * Devuelve un nodo dada una clave.
	 */
	const NodeTaskTree * getNode(int k) const;

    protected:
	/**
	 * En este diccionario se almacenan los índices de las tareas
	 * para un rápido acceso sobre las mismas.
	 */
	TaskTreeDict tree;
	vector<const Task *> roots;
};

#endif /* EXPANSIONTREE_HH */
