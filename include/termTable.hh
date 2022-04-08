#ifndef TERMTABLE_H
#define TERMTABLE_H

#include "constants.hh"
#include <string>
#include "term.hh"
#include "unifiable.hh"

using namespace std;

class VariableSymbol;
class ConstantSymbol;

typedef pair<int,float> pkey;
typedef vector<ConstantSymbol *> constantTable;
typedef vector<VariableSymbol *> varTable;

// limite superior de variables basura que pueden existir
// en la tabla de variables
extern int LIMIT_GARBAGE;

template<class T1, class T2>
struct PrintPair{
    PrintPair(ostream * os) {out = os;};
    void operator()(const pair<T1,T2> & element){
	*out << "(" << element.first << "," << element.second << ")" << endl;
    }
    ostream * out;
};

/**
 * Esta clase almacena la informaci�n necesaria para gestionar tanto las
 * variables como las constantes que se est�n utilizando durante el proceso
 * de planificaci�n.
 */
class TermTable
{
    public:

        ~TermTable(void) {eraseAll();};

        pkey addVariable(VariableSymbol * v);

        pkey addConstant(const char * name);

	pkey addConstant(ConstantSymbol * c);

        inline VariableSymbol * getVariable(const pkey &k) {return variables[-2 -(k.first)];};

        inline VariableSymbol * getVariable(int index) {return variables[-2 -(index)];};

        inline ConstantSymbol * getConstant(const pkey &k) {return constants[k.first];};

        inline ConstantSymbol * getConstant(int index) {return constants[index];};

	inline Term * getTerm(int id) {if(id >= 0){return (Term *) getConstant(id);} else if(id < -1){ return (Term *) getVariable(id);} else return 0;};
	
	inline Term * getTerm(const pkey &k) {return getTerm(k.first);};

	// devuelve null en caso de que no se encuentre
        ConstantSymbol * getConstantFromName(const char * name);

        inline bool isVariable(const pkey &k) {return k.first < -2;};

        inline bool isConstant(const pkey &k) {return k.first >= 0;};

        inline bool isNumber(const pkey &k) {return k.first == -1;};
 
        void print(ostream * out ) const;

        void print(const pkey &key, ostream * out, int nindent=0 ) const;

	/**
	 * Lanza el recolector de basura contra la tabla de t�rminos.
	 * La tabla de t�rminos va acumulando variables a medida que el plan va avanzando.
	 * Cuando se realiza un backtracking las variables no son eliminadas autom�ticamente
	 * a no ser que se lance el recolector de basura. La idea es no realizar operaciones
	 * innecesarias.
	 * @param index Indice de la tabla te variables a partir del cual todo ser� eliminado.
	 */
	void performGarbageCollection(int index);

        // Mantengo abiertas las estructuras de datos para que se pueda acceder a ellas
        // de la manera mas comoda posible. 
        // pero OJO se debe de guardar especial cuidado en como se realizan las modificaciones
        // sobre las mismas.
        // este vector sirve para mantener referencias a las (constantes)
        constantTable constants;
        // y este a las variables
        varTable variables;

protected:
        void eraseAll(void);
};

#endif
