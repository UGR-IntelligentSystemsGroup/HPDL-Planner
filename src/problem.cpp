#include "problem.hh"
#include <math.h>
#include "domain.hh"

Problem::Problem(const Domain * domain)
{
    state=new State(); 
    tasknetwork=0; 
    MAX_THORIZON = domain->MAX_THORIZON;
    REL_THORIZON = domain->REL_THORIZON;
    TFORMAT= domain->TFORMAT;
    FLAG_TIME_UNIT = domain->FLAG_TIME_UNIT;
    FLAG_TIME_START = domain->FLAG_TIME_START;
};

Problem::~Problem()
{
    if(state)
	delete state;
    if(tasknetwork)
	delete tasknetwork;
    state=0;
    tasknetwork=0;
}


