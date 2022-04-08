#ifndef TAG_HH
#define TAG_HH   

#include "constants.hh"
#include <vector>

using namespace std;

/**
 * Esta clase define meta-propiedades que pueden ser utilizadas por
 * una acción u otro elemento de SIADEX, para incluir conocimiento no necesariamente
 * empleado en el proceso de planificación. Pero útil para, por ejemplo, un análisis 
 * posterior del plan resultante.
 **/
class Tag{
    public:
	/**
	 * Constructor.
	 * @param n el nombre que tendra la meta-propiedad.
	 **/
	Tag(const char * n) {name = n;};

	inline const char * getName(void) const {return name.c_str();};

	inline void setName(const char * n) {name = n;};

    protected:
	/// El nombre de la meta-propiedad
	string name;
};

// Algunos typedefs útiles
typedef vector<Tag *> TagVector;
typedef TagVector::const_iterator tagv_cite;
typedef TagVector::iterator tagv_ite;

#endif /* TAG_HH */
