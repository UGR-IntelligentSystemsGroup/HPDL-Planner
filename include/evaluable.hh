#ifndef EVALUABLE_HH
#define EVALUABLE_HH

using namespace std;

#include "constants.hh"
#include "termTable.hh"

class State;
class Unifier;

/**
 * Esta clase representa a los elementos que puedan ser utilizados
 * tanto en los operadores aritm�ticos como de comparaci�n.
*/
class Evaluable
{
    public:
        /**
        * El constructor de copia.
        */
        Evaluable(const Evaluable * other) {};

        /**
        * El constructor por defecto
        */
        Evaluable(void) {};

        /**
        * El destructor.
        */
        virtual ~Evaluable(void) {};

        /**
        * Operador para clonar la clase.
        * @return Una copia exacta a this.
        */
        virtual Evaluable * cloneEvaluable(void) const = 0;

        /**
        * Imprime en un flujo de salida una descripci�n textual
        * de la clase
        * @param os Un flujo de salida.
        * @param Una indentaci�n (n�mero de espacios en blanco al comienzo de
        * cada linea.
        */
        virtual void printEvaluable(ostream * os,int indent=0) const = 0;

        /**
        * Devuelve en un documento xml la descripci�n del objeto.
        * @param os El flujo en el que escribir.
        **/
        virtual void toxmlEvaluable(ostream * os) const{
            XmlWriter * writer = new XmlWriter(os);
            toxmlEvaluable(writer);
            writer->flush();
            delete writer;
        };

        /**
        * Devuelve en un documento xml la descripci�n del objeto.
        * @param writer el objeto donde escribir.
        **/
        virtual void toxmlEvaluable(XmlWriter * writer) const = 0;

        /**
        * Dado un identificador devuelve la key asociada.
        */
        virtual pkey compGetTermId(const char * name) const = 0;

        /**
        * Comprueba si el comparador tiene un t�rmino con
        * el identificador pasado como argumento.
        */
        virtual bool compHasTerm(int id) const = 0;

        /**
        * Realiza el renombrado de variables.
        */
        virtual void compRenameVars(Unifier * u, VUndo * undo) = 0;

        /**
        * Realiza la evaluaci�n de un objeto evaluable con el fin de hacer una
        * comparaci�n o una asignaci�n.
        * En caso de error durante la evaluaci�n el primer elemento del par
        * devuelto tendr� valor INT_MAX
        * @param state El estado actual de planificaci�n.
        * @param contex El contexto de planificaci�n.
        * @return un pkey con el resultado de la evaluaci�n.
        */
        virtual pkey eval(const State * state, const Unifier * context) const = 0;

        /**
        * Realiza la evaluaci�n de un objeto evaluable con el fin de hacer una
        * comparaci�n o una asignaci�n.
        * Esta evaluaci�n es especial para dar tratamiento a los time points.
        * En caso de error durante la evaluaci�n el primer elemento del par
        * devuelto tendr� valor INT_MAX
        * @param state El estado actual de planificaci�n.
        * @param contex El contexto de planificaci�n.
        * @param tp Es un par�metro de salida. Contendr� un time point si
        * durante la evaluaci�n se encuentra una referencia a dicho elemento.
        * (espera inicializado a (-1,0))
        * @param pol Es un par�metro de salida. Si es true el tp no va afectado
        * por un signo negativo, false en otro caso. (espera inicializado a true)
        * @return un pkey con el resultado de la evaluaci�n.
        */
        virtual pkey evaltp(const State * state, const Unifier * context, pkey * tp, bool * pol) const = 0;

        /**
        * Convierte la descripci�n textual del objeto
        * en un string.
        * @return Un string con la descripci�n del objeto.
        */
        virtual const char * toStringEvaluable(void) const {static string s; ostringstream os; printEvaluable(&os,0); s = os.str(); return s.c_str();};

        /**
        * Funci�n para facilitar la identificaci�n de las
        * clases hijas.
        */
        virtual bool isFluentNumber(void) const {return false;};

        /**
        * Funci�n para facilitar la identificaci�n de las
        * clases hijas.
        */
        virtual bool isType(const Type *t) const = 0;

        /**
        * Funci�n para facilitar la identificaci�n de las
        * clases hijas.
        */
        virtual bool isFluentLiteral(void) const {return false;};
};

#endif

