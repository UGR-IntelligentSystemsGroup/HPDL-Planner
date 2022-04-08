#ifndef CAUSAL_HH
#define CAUSAL_HH

#include "constants.hh"
#include <utility>
#include <vector>
#include <stdexcept>

#define ATEND INT_MAX

using namespace std;

class LiteralEffect;
class Comparable;
class Task;
class PrimitiveTask;

typedef pair<unsigned int, unsigned int> Protection;

/**
 * Esta clase representa los enlaces causales entre
 * dos tareas.
 */
class Causal{
    public:
	Causal(LiteralEffect * ref, const Protection & p);
	Causal(LiteralEffect * ref, const Protection * p);
	Causal(const Causal & o);
	Causal(const Causal * o);

	Causal * clone(void) const {return new Causal(this);};

	/**
	 * Es un enlace normal.
	 */
	bool isNormalLink(void) const;
	/**
	 * Es un enlace con un predicado fluent.
	 */
	bool isFluentLink(void) const;
	/**
	 * Es un enlace con un predicado del timeline.
	 */
	bool isTimeLineLink(void) const;

	inline float getTime(void) const {return time;};
	inline void setTime(float t) {time = t;};
	inline const LiteralEffect * getLiteral(void) const {return literal;};
	inline LiteralEffect * getModificableLiteral(void) const {return literal;};

	inline const PrimitiveTask * getProducer(void) const {return producer;};
	inline void setProducer(const PrimitiveTask * pt) {producer=pt;};

	inline const Task * getConsumer(void) const {return consumer;};
	inline void setConsumer(const Task * pt) {consumer=pt;};

	inline unsigned int getProtectionFirst(void) const {return protection.first;};
	inline unsigned int getProtectionSecond(void) const {return protection.second;};

	/**
	 * Imprime una descripción del vínculo causal en un flujo
	 * dado.
	 * @param os el flujo de salida.
	 */
	void print(ostream * os) const;

        /**
         * Chequea la consistencia del vinculo
         **/
        void checkConsistency(void) {
            if (literal == 0)  throw std::logic_error("Causal::checkConsistency (1) Null pointer");
            if (consumer == 0) throw std::logic_error("Causal::checkConsistency (3) Null pointer");
        };

    protected:
	LiteralEffect * literal;
	const PrimitiveTask * producer;
	const Task * consumer;
	// El intervalo de protección que exige la precondición
	Protection protection;
	float time;
};

typedef vector<Causal *> CLTable;
typedef CLTable::const_iterator cltcite;
typedef CLTable::iterator cltite;

#endif
