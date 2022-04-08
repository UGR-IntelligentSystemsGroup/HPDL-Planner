#include "controlrules.hh"
#include <math.h>
#include "debugger.hh"
#include "domain.hh"
#include "undoElement.hh"
#include "undoChangeValue.hh"
#include "function.hh"
#include "fluentEffect.hh"
#include "plan.hh"

Controlrules * controlrules;

Controlrules::Controlrules(void) {
	id = 1;
}

Controlrules::~Controlrules(void) {
}


// CONTROL SOBRE EL PUNTO DE DECISION BT1
int Controlrules::selectPermutableTask(StackNode *context) {
	
	//Obtenemos las distintas opciones
	int sucount;
	sucount = context->agenda.size();
	
	//Hacemos la seleccion
	int suselected;
	suselected = context->agenda.at(sucount-1).first;

	//*errflow << "Numero de Selecciones unordered: " << sucount << " elegimos la " << suselected << endl;
	
	return suselected;
}

// CONTROL SOBRE EL PUNTO DE DECISION BT2
int Controlrules::selectTaskExpansion(StackNode *context) {

	//Obtenemos las distintas opciones
	int ecount;
	ecount = context->offspring->size();
	
	//Hacemos la seleccion
	int eselected;
	eselected = ecount - 1;
	
	//*errflow << "Numero de Expansiones Posibles: " << ecount << " elegimos la " << eselected << endl;
	
	return eselected;

}

// CONTROL SOBRE EL PUNTO DE DECISION BT3
int Controlrules::selectMethod(StackNode *context) {
	
	// Obtenemos las distintas opciones
	int mcount;
	mcount = context->methods->size();
	
	//Hacemos la seleccion
	int mselected;
	mselected = 0;
	
	//*errflow << "Numero de Metodos Posibles: " << mcount << " elegimos el " << mselected << endl;
	
	return mselected;
}


// CONTROL SOBRE EL PUNTO DE DECISION BT4
int Controlrules::selectUnification(StackNode *context) {
	
	//Obtenemos las distintas opciones
	int ucount;
	ucount = context->utable->countUnifiers();
		
	//Hacemos la seleccion
	int uselected;
	uselected = 0;	
		
	//*errflow << "Numero de Unificaciones Posible: " << ucount << " elegimos la " << uselected << " Tarea(" << context->task << " " << context->taskid << ")" << endl;
	
	return uselected;
}
