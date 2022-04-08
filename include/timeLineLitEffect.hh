#ifndef TIMELINE_LITERALEFFECT_HH
#define TIMELINE_LITERALEFFECT_HH

#include "constants.hh"
#include "literaleffect.hh"

using namespace std;
extern int MAX_THORIZON;
typedef vector<pair<int,int> > VIntervals;
typedef VIntervals::iterator VIite;
typedef VIntervals::const_iterator VIcite;

class TimeLineLiteralEffect : public LiteralEffect 
{
    public:

        /**
          @brief Constructor.
          @param n nombre del literal.
          @param p La polaridad, por defecto positiva.
         */
        TimeLineLiteralEffect(int id, int mid, bool p=true);

        TimeLineLiteralEffect(int id, int mid, const KeyList * param, bool p);

	TimeLineLiteralEffect(const TimeLineLiteralEffect * le);

        /**
          @brief Destructor
         */
        virtual ~TimeLineLiteralEffect() {};

        /**
        * @brief realiza una copia exacta a this.
        */
        virtual Expression * clone(void) const;

        virtual Literal * cloneL(void) const;

        /**
        @brief Imprime el contenido del objeto.
        @param indent el n�mero de espacios a dejar antes de la cadena.
        @param os Un flujo de salida por defecto la salida estandard.
        */
        virtual void print(ostream * os, int indent=0) const;

        virtual void vcprint(ostream * os, int indent=0) const {print(os,indent);};

        virtual void printL(ostream * os, int indent=0) const {this->print(os,indent);};

        virtual void toxmlL(XmlWriter * writer) const {this->toxml(writer);};

        virtual void toxml(XmlWriter * writer) const;

	/**
	 * Comprueba si el literal se encuentra temporizado.
	 */
	virtual bool isTimeLine(void) const {return true;};
	
	/**
	 * Establece la diferencia entre el inicio y el fin del intervalo.
	 */
	inline void setInterval(int s, int e) {start = s; duration = (e-s);};

	inline pair<int,int> getInterval(int n=0) const {if(gap <=0) assert(n==0); return make_pair(start + n*(duration+gap),start+duration + n*(duration+gap));};

	/**
	 * Establece el tiempo tras el cual se vuelve a hacer cierto el intervalo.
	 */
	inline void setGap(int g) {gap = g;};

	inline int getGap(void) const {return gap;};
	
	/** 
	 * Realiza la intersecci�n de dos series de intervalos
	 * determinado.
	 * @param other El intervalo con el que intersectar, estar� vacio la primera vez.
	 * @param min El valor m�nimo para el intervalo.
	 * @param max El valor m�ximo para el intervalo.
	 */
	void merge(VIntervals & other,int  min, int max);

	/**
	 * Devuelve el �ndice del intervalo que corresponde
	 * al tiempo t.
	 */
	int getIndexInterval(int t) const;

	/**
	 * Devuelve el tpoint asociado al intervalo n.
	 * -1 en caso de que no exista
	 */
	pair<int,int> getTPoint(int n) const;

	/**
	 * Registra un tp contra el literal del tl.
	 * @param n el �ndice del intervalo contra el que vamos a registrar.
	 * @param tp el �ndice del timepoint
	 */
	void addTPoint(int n, pair<int,int> tp);

	/**
	 * Borra todos los timepoints con �ndice mayor que el dado.
	 */
	void eraseTPoint(int t);

    protected:
	int start;
	int duration;
	int gap;

	// variables for not generating again the intervals
	int nmin;
	int nmax;
	
	/**
	 * Generar intervalos desde min hasta max, basandonos en el valor de n (numero de intervalo)
	 */
	void generateNIntervals(int min,int max);

	/**
	 * Generar intervalos desde min hasta max, en un intervalo de tiempo. 
	 */
	void generateTIntervals(int min,int max);

	VIntervals intervals;

	/**
	 * Vector para almacenar los timepoints registrados contra este literal del tl
	 */
	vector<pair<int,pair<int,int> > > tpoints;
};

#endif
