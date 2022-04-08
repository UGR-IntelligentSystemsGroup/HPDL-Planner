#ifndef DEBUGGER_HH
#define DEBUGGER_HH 

#include "constants.hh"
#include <string>
#include <iostream>
#include <tr1/unordered_map>
#include <ctype.h>
#include "MyLexer.hh"
#include "constants.hh"
#include "displayElement.hh"
#include "state.hh"
#include "tasknetwork.hh"
#include "literalgoal.hh"
#include "stacknode.hh"
#include "causalTable.hh"
#include "selector.hh"
#include "plan.hh"


typedef std::tr1::unordered_map<std::string, std::string , icaseDJBHash, ieqstr> Dictionary;

using namespace std;



/*!
 * Esta clase implementa una analizador lexico, usando flex.
 */
class Debugger: public Selector
{
    public:
        Debugger(void); 
        ~Debugger();

        /**
          @brief Imprime un mensaje de error.
          @param msg  El mensaje de error
          @return */
        void printError(const char* msg);

        int prompt(Plan * p);

        void printState(void) const;

        void printAgenda(void) const;

        void printPlan(void) const;

        void printTermtable(void) const;

	/**
	 * Imprime todas las cosas que haya en el estado
	 * que unifiquen con el literal dado como argumento.
	 * @param lg El literal a buscar
	 */
        void printLiteral(const LiteralGoal * lg) const;

	/**
	 * Imprime el contenido del display.
	 */
        void printSymbol(const DisplayElement * de) const;

        void printOptions(void) const;

	void printPredicates(void) const;

	void printTasks(void) const;

        void displaySymbol(DisplayElement * exp);

        void describeSymbol(DisplayElement * exp);

        void describeSymbol(const char * exp);

        void printDisplays(void) const;

        void undisplay(int);

        void display(int index);

        void printBreakpoints(void) const;

        void printBreakpoint(int index) const;

        void select(int option);

	void init(void);

        Dictionary dictionary;

        void plotPlan(void);

        void plotCausal(void);

	/**
	 * Pone un breakpoint donde parar al depurador.
	 */
	void setBreakpoint(DisplayElement * exp);

	/**
	 * Reactiva un breakpoint previamente deshabilitado. 
	 */
	void enableBreakpoint(int index);

	/**
	 * Reactiva un breakpoint previamente deshabilitado. 
	 */
	void disableBreakpoint(int index);

	/**
	 * Testea si un breakpoint se ha disparado.
	 */
	bool testBreakpoints(Stage s, const StackNode * n);

	void eval(const Goal * g) const;

	void apply(Effect * e) const;

	virtual int selectFromAgenda(Plan * p);

	virtual int selectTask(Plan * p);

	virtual int selectMethod(Plan * p);

	virtual int selectUnification(Plan * p);

	void setFReadLine(char * (* fptr) (const char *)){read_line = fptr;};

	/**
	 * Establece la lista de opciones que puede seleccionar el usuario desde la consola.
	 */
	inline void setOptions(const vector<string> * tmp) {options = tmp;};
	// ---------------------------------------
	// Variables de uso interno, no tocar.
	// ---------------------------------------
        // flag para hacer que el debuger termine y continue
        // la ejecuci�n
        bool cont;

        // flag para comprobar si se ejecuto la orden next
        bool next;
	
        // flag para comprobar si se ejecuto la orden nextexp
        bool nexp;

        string tmpdir;

        string viewerCommand;

        string dotPath; 

        int option;

    private:
        /*
            Gesti�n de las entradas del usuario
        */
        istream * preprocessLine(void);

        vector<DisplayElement *> displayList;
        vector<DisplayElement *> breakpointList;
        vector<bool> enabledDisplays;
        vector<bool> enabledBreakpoints;

	/** Mantiene la lista de opciones vigentes */
	const vector<string> * options;

	/**
	 * Una copia local del plan.
	 */
	Plan * plan;

	/**
	 * Este puntero a funci�n se utiliza por el debugger para capturar una �rden,
	 * desde la l�nea de comandos o desde cualquier otro lugar. La funci�n recibe como
	 * argumentos el prompt, y espera recibir una cadena con la orden escrita.
	 * Por defecto usa la librer�a readline para hacer la captura.
	 **/
	char * (* read_line) (const char *);
};

#endif
