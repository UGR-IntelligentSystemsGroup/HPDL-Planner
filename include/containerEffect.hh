#ifndef CONTAINEREFFECT_HH
#define CONTAINEREFFECT_HH

#include "effect.hh"

using namespace std;

/**
  * @author  Oscar
  * @description Esta clase representa la base para todos los posibles objetivos del planificador.
*/

class ContainerEffect : public Effect 
{
    public:
	ContainerEffect(const ContainerEffect * o) :Effect(o) {};
	ContainerEffect(void) :Effect() {};
	virtual ~ContainerEffect(void) {};

        virtual void addEffect(const Effect * e) = 0;
        virtual void addEffectByRef(Effect * e) = 0;
};

#endif

