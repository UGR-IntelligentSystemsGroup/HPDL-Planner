#ifndef CONSTANTSYMBOL_H
#define CONSTANTSYMBOL_H
#include "constants.hh"
#include "term.hh"
#include <string>
#include <iostream>

using namespace std;

class ConstantSymbol: public Term
{
    public:
        ConstantSymbol(const char * n, int id) :Term(id), name(n) {lineNumber=0; fileId=-1;};

        ConstantSymbol(const char * n, int id, const vctype * v) :Term(id,v), name(n) {lineNumber=0; fileId=-1;};

	ConstantSymbol(const ConstantSymbol * other) :Term(other), name(other->getName()) {lineNumber=other->lineNumber; fileId=other->fileId;};

        ConstantSymbol(const char * n) :Term(-1), name(n) {lineNumber=0; fileId=-1;};

	ConstantSymbol(void) :Term() {lineNumber=0; fileId=-1; name = "";};

        virtual Term * clone(void) const;

        virtual bool isConstant(void) const {return true;};

        /**
          @brief Imprime una descripci�n del contenido de la constante.
          @param ostream Flujo de salida en donde escribir. Por defecto cout.
         */
        virtual void print(ostream * os, int indent = 0) const;

        virtual void toxml(XmlWriter * writer) const;

	inline int getLineNumber(void) const {return lineNumber;};

	inline void setLineNumber(int i) {lineNumber = i;};

	inline int getFileId(void) const {return fileId;};

	inline void setFileId(int i) {fileId = i;};

	inline const char * getName(void) const {return name.c_str();};

	inline void setName(const char * n) {name = n;};

    protected:
	int lineNumber;
	int fileId;
	string name;
};

/**
 * Comprueba si un s�mbolo de constante codifica un tp
 * @param p la clave del simbolo.
 * @return true en caso afirmativo
 */
bool isTimePoint(const pkey & p);

/**
 * Devuelve el identificador de un time point, dada
 * una pkey.
 * @param p la clave del simbolo.
 * @return el id si la transformaci�n es correcta -1 en otro caso
 */
int extractTimePoint(const pkey & p);

#endif
