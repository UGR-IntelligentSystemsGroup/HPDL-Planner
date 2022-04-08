#ifndef META_HH
#define META_HH   
#include "constants.hh"
#include "tag.hh"

using namespace std;

/**
 * Esta clase define meta-propiedades que pueden ser utilizadas por
 * una acción u otro elemento de SIADEX, para incluir conocimiento no necesariamente
 * empleado en el proceso de planificación. Pero útil para, por ejemplo, un análisis 
 * posterior del plan resultante.
 **/
class Meta{
    public:
	/**
	 * Constructor por defecto.
	 **/
	Meta(void){
	    linenumber = 0;
	    fileid = -1;
	};

	/**
	 * Constructor.
	 * @param n el nombre que tendra la meta-propiedad.
	 * @param line La línea donde se declaró el elemento.
	 * @param file El fichero donde se declaró el elemento.
	 **/
	Meta(const char * n, int line, int file) 
	{name=n; linenumber=line; fileid=file;};

	virtual ~Meta(void) {
	    clear();
	};

	// nombre del símbolo
	string name;
	// línea en la que fue definido
	int linenumber;
	// identificador del fichero donde se definió
	int fileid;

	virtual void clear(void){
	    for_each(tags.begin(),tags.end(),Delete<Tag>());
	};

	inline void addTag(Tag * t){
	    tags.push_back(t);
	};

	inline int getNumberOfTags(void) const{
	    return tags.size();
	};

	inline const Tag * getTag(int index) const{
	    if(index >= 0 && index < (int) tags.size()){
		return tags[index];
	    }
	    else
		return 0;
	};

	inline tagv_cite getBeginTags(void) const{
	    return tags.begin();
	};

	inline tagv_cite getEndTags(void) const{
	    return tags.end();
	};

	const Tag * getTag(const char * name) const {
	    tagv_cite b, e = tags.end();
	    for(b = tags.begin();b!=e;b++){
		if(strcasecmp(name,(*b)->getName())== 0)
		    return (*b);
	    };
	    return 0;
	};

    protected:
	/**
	 * Este vector almacena los metatags definidos para
	 * un elemento determinado.
	 **/
	TagVector tags;
};


/**
 * las taskHeader tienen su propia estructura de metainformación
 * para almacenar información de preprocesamiento, tal como cuales
 * son la tareas que me pueden satisfacer.
 **/
class MetaTH: public Meta
{
    public:
	vector<Task *> candidates;

	MetaTH(const char * n, int line, int file) 
	    :Meta(n,line,file) {};

	virtual ~MetaTH(void) {
	    candidates.clear();
	};
};

// Algunos typedefs útiles
typedef vector<Meta *> MetaInfo;
typedef MetaInfo::const_iterator metai_cite;
typedef MetaInfo::iterator metai_ite;

#endif /* META_HH */
