#ifndef DISPLAYELEMENT
#define DISPLAYELEMENT

#include <vector>
#include "term.hh"
#include "termTable.hh"
#include "papi.hh"
#include "goal.hh"

using namespace std;

class DisplayElement
{
    public:
        string name;
        bool pol;
        Goal * goal;

        DisplayElement(void)
        {
            name = "";
            pol = true;
            goal = 0;
        };

        ~DisplayElement(void)
        {
	    if(goal)
		delete goal;
        };

        void print(ostream * os) const
        {
            if(goal){
		goal->print(os,0);
            }
            else
                *os << name;

        };
};
#endif
