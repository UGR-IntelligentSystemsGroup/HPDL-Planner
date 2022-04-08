/**
 * This class define de API for accessing the parser information from an
 * external program
 */

#ifndef PAPI_HH
#define PAPI_HH

#include "constants.hh"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <vector>
#include "unify.hh"
#include "pythonWrapper.hh"
class Domain;
class Problem;
class TermTable;

using namespace std;

ifstream * openfile(const char * fileName);
ofstream * openwfile(const char * fileName);

// Manejo de las unidades de tiempo
enum TimeUnit {TU_SECONDS=0, TU_MINUTES, TU_HOURS, TU_DAYS, TU_MONTHS, TU_YEARS, TU_UNDEFINED};

/**
 * Convierte una unidad temporal en un string.
 **/
string TimeUnit2String(const TimeUnit t);

/**
 * Estructura que almacena la api del planificador y del parser.
 * contiene metainformaci�n esencial para el proceso de planificaci�n
 * y el parseado.
 * Es una clase base a partir de la cual se puede extraer toda la informaci�n
 * necesaria.
**/
class PAPI {
    public:
    /**
    * El constructor de la clase
    **/
    PAPI(void);

    /**
    * El destructor.
    * Libera la memoria ocupada por el dominio y el problema y otras
    * estructuras de datos
    **/
        virtual ~PAPI(void);

    /**
    * Realiza el parseo del dominio y del problema.
    * @param domainfile El fichero de dominio.
    * @param problemfile El fichero de problema.
    * @param fast si es true hace un parseado r�pido, sin hacer test de alcanzabilidad
    * ni otros.
    * @return true si hubo �xito, error en otro caso.
    **/
        bool parse(const char * domainfile, const char * problemfile, bool fast=false);

    /**
    * Realiza el parseo del dominio y del problema.
    * @param domainfile El flujo de dominio.
    * @param problemfile El flujo de problema.
    * @param fast si es true hace un parseado r�pido, sin hacer test de alcanzabilidad
    * ni otros.
    * @return true si hubo �xito, error en otro caso.
    **/
        bool parse(istream * domainfile, istream * problemfile, bool fast=false);

    /**
    * Realiza el parseo del dominio.
    * @param domainfile El fichero de dominio.
    * @param fast si es true hace un parseado r�pido, sin hacer test de alcanzabilidad
    * ni otros.
    * @return true si hubo �xito, error en otro caso.
    **/
        bool parse_domain(const char * domainfile, bool fast=false);

    /**
    * Realiza el parseo del problema.
    * Es necesario que se haya cargado el dominio previamente.
    * @param problemfile El fichero de problema.
    * @param fast si es true hace un parseado r�pido, sin hacer test de alcanzabilidad
    * ni otros.
    * @return true si hubo �xito, error en otro caso.
    **/
        bool parse_problem(const char * problemfile, bool fast=false);

    /**
    * Establecer o no el parseado r�pido sin hacer ning�n tipo de test.
    **/
        inline void setFastMode(bool v=true) {fastMode = v;};

    /**
    * Devuelve si estamos o no en modo de parseado r�pido
    **/
    inline bool isFastMode(void) {return fastMode;};

    /**
    * Devuelve el objeto de dominio cargado a partir de un fichero.
    **/
    inline const Domain * getDomain(void) const {return domain;};

    /**
    * Devuelve el objeto de problema cargado a partir de un fichero.
    **/
    inline const Problem * getProblem(void) const {return problem;};

    Domain * domain;
    Problem * problem;
    TermTable * termtable;

    // estructura para mantener los ficheros de los cuales
    // hemos leido la informaci�n
    vector<string> files;
    // identificador del fichero actualmente abierto
    // -1 indica ninguno
    int fileid;

    // clase que contiene el int�rprete de python
    PythonWrapper wpython;


    /**
    * Imprime la versi�n de SIADEX utilizada.
    */
    const char * ver(void) const;

    /**
    * Redirige la salida a pantalla a un flujo interno
    */
    void resetOutStream(void);

    /**
    * Lee lo �ltimo escrito en el flujo de salida interno
    */
    const char * readOutStream(void);

    /**
    * Devuelve la configuraci�n temporal en el plan actual.
    */
    const char * getTFormat(void) const;

    time_t getTStart(void) const;

    TimeUnit getTUnit(void) const;

    /**
    * Establece par�metros de la configuraci�n temporal.
    **/
    void setFlagTUnit(TimeUnit tu);

    void setTFormat(string s);

    void setMTHorizon(int t);

    void setRTHorizon(int t);

    void setTStart(time_t t);

    // flag para indicar si hubo alg�n tipo de error durante el parseado
    // del pddl.
    bool errors;

    /**
    * Devuelve el n�mero de errores detectados en el fichero de problema
    **/
    inline int getNProblemErrors(void) const {return n_problem_errors;};

    /**
    * Devuelve el n�mero de warnings detectados en el fichero de problema
    **/
    inline int getNProblemWarnings(void) const {return n_problem_warnings;};

    /**
    * Devuelve el n�mero de errores detectados en el fichero de dominio
    **/
    inline int getNDomainErrors(void) const {return n_domain_errors;};

    /**
    * Devuelve el n�mero de warnings detectados en el fichero de dominio
    **/
    inline int getNDomainWarnings(void) const {return n_domain_warnings;};

    protected:
        Domain *  loadDomain(const char * domainfile);
        Problem * loadProblem(const char * domainfile);
    Domain * loadDomain(istream * flow, const char * debugname);
    Problem * loadProblem(istream * flow, const char * debugname);
        bool fastMode;
    ostringstream * ostr;
    int n_problem_errors;
    int n_problem_warnings;
    int n_domain_errors;
    int n_domain_warnings;
};

extern PAPI * parser_api;

#endif
