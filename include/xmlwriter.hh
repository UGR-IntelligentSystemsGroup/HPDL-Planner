#ifndef XMLWRITER_HH
#define XMLWRITER_HH

#include "constants.hh"
#include <iostream>
#include <stack>
#include <string>
#include <sstream>

using namespace std;

/**
 * Esta clase trata de facilitar el trabajo de escribir información en formato XML.
 * Realiza adecuadamente el escape de carácteres especiales y la apertura y cierre
 * de tags.
 */
class XmlWriter{
    public:
	/**
	 * El constructor.
	 * @param os El flujo donde se escribirá el xml. No debe cerrarse mientras
	 * XmlWriter no haya sido liberado.
	 **/
	XmlWriter(ostream * os);

	/**
	 * El destructor de la clase.
	 **/
	~XmlWriter(void);

	/**
	 * Abre un elemento.
	 * @param el nombre del elemento.
	 **/
	XmlWriter * startTag(const char * name);

	/**
	 * Cierra el último elemento abierto. 
	 **/
	XmlWriter * endTag();

	/**
	 * Cierra todos los tags abiertos hasta ahora.
	 **/
	void flush();

	/**
	 * Añade un atributo al tag abierto actualmente.
	 * @param name El nombre.
	 * @param value el contenido.
	 **/
	XmlWriter * addAttrib(const char * name, const char * value);

	/**
	 * Añade un atributo al tag abierto actualmente.
	 * @param name El nombre.
	 * @param value el contenido.
	 **/
	XmlWriter * addAttrib(const char * name, int value);

	/**
	 * Añade un atributo al tag abierto actualmente.
	 * @param name El nombre.
	 * @param value el contenido.
	 **/
	XmlWriter * addAttrib(const char * name, double value);

	/**
	 * Añade un atributo al tag abierto actualmente.
	 * @param name El nombre.
	 * @param value el contenido.
	 **/
	XmlWriter * addAttrib(const char * name, const string & value);

	/**
	 * Añade datos de contenido "character data" a un tag abierto.
	 * @param value el contenido.
	 **/
	XmlWriter * addCharacter(const char * value);

	/**
	 * Añade datos de contenido "character data" a un tag abierto.
	 * @param value el contenido.
	 **/
	XmlWriter * addCharacter(const string & value);

	/**
	 * Establece el número de espacios a dejar cada vez que se abra un
	 * nuevo tag.
	 * @param n el número de espacios a dejar.
	 **/
	inline void setDefaultIndent(int n) {indent = n;};

    protected:
	/** El flujo donde escribiremos la información */
	ostream * flow;
	/** La pila de tags abiertos */
	stack<string> stk;
	/** La lista de atributos del tag actual */
	ostringstream * attribs;
	/** El character data del tag actual */
	ostringstream * text;
	/** Número de indentaciones por defecto */
	int indent;
	/** Flag para marcar si un tag se encuentra abierto */
	bool open;

	/**
	 * Esta función realiza el escapado de los carácteres reservados en
	 * xml
	 * @param flow el flujo donde escribir.
	 * @param content el contenido a escapar y escribir en el flujo.
	 **/
	void escapeXml(ostream * flow, const char * content);
};

#endif /* XMLWRITER_HH */
