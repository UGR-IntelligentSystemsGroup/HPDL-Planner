#ifndef TEXTTAG_HH
#define TEXTTAG_HH   

#include "constants.hh"
#include "tag.hh" 
#include "parameterContainer.hh"

using namespace std;

/** 
 * función para hacer el procesado de los metatags de texto interpretado.
 * @param input La cadena a procesar.
 * @param pc El óbjeto que contiene una lista de parámetros con nombre que podemos
 * sustuir en la cadena dada.
 **/
string processTextTag(const string & input, const ParameterContainer * pc);

/**
 * Esta clase implementa una meta-propiedad que contiene una cadena de texto.
 **/
class TextTag : public Tag{
    public:
	/**
	 * Constructor.
	 * @param n el nombre que tendra la meta-propiedad.
	 **/
	TextTag(const char * n);

	inline void setValue(const char * s) {value =s;};

	inline const char * getValue(void) const {return value.c_str();};

    protected:
	/// El contenido de la meta-propiedad
	string value;
};
#endif /* METATEXT_HH */
