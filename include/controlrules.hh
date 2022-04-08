 #ifndef CONTROLRULES_HH
#define CONTROLRULES_HH 
 
 #include "constants.hh"
#include <iostream>
#include <stdlib.h>
#include "debugger.hh"
#include "termTable.hh"
#include "pythonWrapper.hh"
#include <time.h>
#include "papi.hh"
#include "problem.hh"
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include "clock.hh"

using namespace std;

class Controlrules
{

	public:
		Controlrules(void);
		~Controlrules();
		
	int selectPermutableTask(StackNode *context);
	
	int selectTaskExpansion(StackNode *context);
	
	int selectMethod(StackNode *context);
	
	int selectUnification(StackNode *context);
	
	int id;
	
};

#endif
