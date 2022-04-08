#ifndef MyLexer_hh
#define MyLexer_hh

#include "constants.hh"
#include <string>
#include <iostream>
#include <tr1/unordered_map>
#include <ctype.h>
#ifdef yyFlexLexer
#undef yyFlexLexer
#endif
#include <FlexLexer.h>

typedef std::tr1::unordered_map<std::string, std::string , icaseDJBHash, ieqstr> str_hash;
typedef str_hash::const_iterator str_hashcite;
typedef str_hash::iterator str_hashite;

#define MAX_INCLUDE_DEPTH 10

typedef struct LexerContext{
    string fileName;
    ifstream * flow;
    struct yy_buffer_state * buffer;
    int line;
} LexerContext;

/*!
 * Esta clase implementa una analizador lexico, usando flex.
 */


class MyLexer: public yyFlexLexer
{
    public:
        /*! La línea por la que va leyendo el analizador */
        int lineNum;

	inline int getLineNumber(void) const {return lineNum;};

	inline void setLineNumber(int ln) {lineNum = ln;};

        MyLexer(istream* arg_yyin=0, ostream* arg_yyout=0);

        ~MyLexer();

        /**
          @brief Imprime un mensaje de error.
          @param msg  El mensaje de error
          @return */
        void LexerError(const char* msg);

        void LexerWarning(const char* msg);

        /**
          @brief Establece el nombre del fichero sobre el que se esta realizando el parsing.
          Unicamente es necesario para generar los mensajes de error.
          @param file  El nombre del fichero.
          @return */
        inline void setFileName(const char * afile) {fileName = afile;};

        inline const char * getFileName(void) const {return fileName.c_str();};

	/**
	 * Cambia a otro flujo, dado como un nombre de fichero
	 */
	void switchTo(string file);

	/**
	 * restaura al estado anterior.
	 * @return false en caso de que la pila de flujos esté vacía
	 */
	bool restore(void);

        /**
          @brief Get the number of errors during the parsing.
          @return */
        int getErrors(void) const {return n_errors;};

        int getWarnings(void) const {return n_warnings;};

        /** Count the number of errors during the parsing */
        int n_errors;
        int n_warnings;

    private:
        /*! Nombre del fichero, para generar errores en pantalla */
        string fileName;
        /*! Mantiene un diccionario de palabras a sustituir en los mensajes de error */
        str_hash dictionary;

        /*! Esta función es llamada por el constructor de MyLexer para construir un
          contenedor hash para los símbolos de error. Cada vez que se encuentra una
          de las claves en un mensaje de error son sustituidas por su valor. De esa
          forma conseguimos mensajes de error mas explicativos.
         */
        void initErrorTable();

        /*! Esta función sustituye las palabras en un mensaje por sus sinónimos en
            el diccionario.
            @return El mensaje con las palabras cambiadas. La cadena devuelta es
            estática al método por lo que no hay que liberarla. LLamar por segunda
            vez al método, altera la cadena devuelta por primera vez.
         */
        const char * replace(const char * msg);

	vector<LexerContext> stack;
};

extern MyLexer * lexer;

#endif
