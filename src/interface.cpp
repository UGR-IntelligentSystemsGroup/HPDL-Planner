#include "interface.hh"
#include "domain.hh"

//Maximo numero de tareas
#define NUM_MAX_TASK 100000
#define NUM_MAX_TYPES 100000
#define NUM_MAX_PREDICATES 100000
#define NUM_MAX_AXIOMS 100000

//Caracter delimitador para separar elementos en listas formadas por char*
#define DELIM "@"

//Variable global del dominio cargado
Domain * domain_interface;
TermTable * termtable;
//Variable global del api para leer el dominio
PAPI *api_interface;


/**************************************************************
********** Funciones de Inicializacion y configuracion ********
***************************************************************/

void startInterface(void) {
	// Se inicializa el flujo de errores global
	errflow = new ostringstream();
}

bool loadDomain(char *domainf) {
	bool r;

	domain_interface = new Domain();
	api_interface = new PAPI();

	api_interface->parse_domain(domainf,true);

	if(api_interface->errors)
		r = false;
	else {
		domain_interface = api_interface->domain;
		termtable = api_interface -> termtable;
		r = true;
		}

	if(r)
		while(!domain_interface->loaded);

	return r;

}


/**************************************************************
******************* Funciones de Lectura **********************
***************************************************************/

// Devolvemos el nombre del dominio
char * getName(void) {

	if (domain_interface->loaded)
		return (char *) domain_interface->getName();
	else
		return "";

}

// Devolvemos una cadena de texto con informacion de los tipos, de la forma
// "id:nombre_tipo"
const char * getTypes(void) {
	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	typetablecit b, e;
	//Variable temporal para guardar IDs
	char id[NUM_MAX_TYPES]="";

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador a la ultima tarea
	e = domain_interface->getEndType();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginType();b!=e;b++) {
		// Obtenemos los IDs en forma de string
		//La funcion gcvt convierte un int a un char*
		gcvt((int)(*b)->getId(), NUM_MAX_TYPES ,id);

		s = s + id;
		s = s + DELIM; // Separador
		s = s + (*b)->getName() + DELIM + DELIM;

		}

	return s.c_str();

}

// Devolvemos una cadena de texto con informacion de los tipos raiz, de la forma
// "id:nombre_tipo"
const char * getRootTypes(void) {
	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	typetablecit b, e;
	//Variable temporal para guardar IDs
	char id[NUM_MAX_TYPES]="";

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador a la ultima tarea
	e = domain_interface->getEndType();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginType();b!=e;b++) {

		if((*b)->isRoot()) {
			// Obtenemos los IDs en forma de string
			//La funcion gcvt convierte un int a un char*
			gcvt((int)(*b)->getId(), NUM_MAX_TYPES ,id);

			s = s + id;
			s = s + DELIM; // Separador
			s = s + (*b)->getName() + DELIM + DELIM;
		}
	}
	return s.c_str();
}

// Devolvemos una cadena de texto con informacion de los tipos raiz, de la forma
// "id:nombre_tipo"
const char * getChildsOfType(int ParentId) {
	bool isChild = false;
	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	typetablecit b, e;
	vector< Type * >::const_iterator pb, pe;

	//Variable temporal para guardar IDs
	char id[NUM_MAX_TYPES]="";

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador a la ultima tarea
	e = domain_interface->getEndType();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginType();b!=e;b++) {

		// Buscamos entre todos los padres del tipo si esta el padre buscado
		isChild = false;
		pe = (*b)->getParentsEnd();
		for(pb=(*b)->getParentsBegin();pb!=pe;pb++) {
			if((*pb)->getId() == ParentId)
				isChild = true;
		}

		if(isChild) {
			// Obtenemos los IDs en forma de string
			//La funcion gcvt convierte un int a un char*
			gcvt((int)(*b)->getId(), NUM_MAX_TYPES ,id);

			s = s + id;
			s = s + DELIM; // Separador
			s = s + (*b)->getName() + DELIM + DELIM;
		}
	}
	return s.c_str();
}



// Devolvemos las constantes
const char * getConstants(void) {
	//Valor devuelto
	static string s;
	s = "";
	//Iteradores
	constablecit b, e;

	//Variable temporal para guardar IDs
	char id[NUM_MAX_TYPES]="";

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador a la ultima tarea
	e = termtable->constants.end();

	// Recorremos todas las tareas
	for(b=termtable->constants.begin();b!=e;b++){
		// Obtenemos los IDs en forma de string
		//La funcion gcvt convierte un int a un char*
		gcvt((int)(*b)->getId(), NUM_MAX_TYPES ,id);
		s = s + id;
		s = s + DELIM; // Separador
		s = s + DELIM; // Separador
	}
	return s.c_str();
}



// Devolvemos una cadena de texto con informacion de los tipos raiz, de la forma
// "id:nombre_tipo"
const char * getTermName(int tid) {
	//Valor devuelto
	static string s;
	s = "";

	Term *t = termtable->getTerm(tid);

	s = (t)->getName();

	return s.c_str();
}


// Devolvemos una cadena de texto con informacion de los tipos raiz, de la forma
// "id:nombre_tipo"
bool isFunction(int pid) {
	bool value = false;
	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	literaltablecit b, e;
	keylistcit  pb, pe;

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador al ultimo elemento
		e = domain_interface->getEndLiteral();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginLiteral();e!=b;b++){

		if(!(*b).second->isFunction()) {
			if((int)(*b).second->getId() == pid) {
				value = true;
			}
		}
	}
	return value;
}

// Devolvemos una cadena de texto con informacion de los tipos raiz, de la forma
// "id:nombre_tipo"
const char * getLiteralName(int pid) {
	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	literaltablecit b, e;
	keylistcit  pb, pe;

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador al ultimo elemento
		e = domain_interface->getEndLiteral();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginLiteral();e!=b;b++){
		if((int)(*b).second->getId() == pid) {
			s = s + (*b).second->getName();
		}
	}
	return s.c_str();
}



// Devolvemos una cadena de texto con informacion de los tipos raiz, de la forma
// "id:nombre_tipo"
const char * getLiteralArgsList(int pid) {
	bool first;

	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	literaltablecit b, e;
	keylistcit  pb, pe;

	//Variable temporal para guardar IDs
	char id[NUM_MAX_PREDICATES]="";
	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador al ultimo elemento
		e = domain_interface->getEndLiteral();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginLiteral();e!=b;b++){
		if((int)(*b).second->getId() == pid) {
			// Agregamos los argumentos del predicado
			first = true;
			pe = (*b).second->endp();
			for(pb=(*b).second->beginp();pb!=pe;pb++) {
				//TermTable
				Term *  tt = termtable->getTerm(*pb);
				gcvt((int)(tt)->getId(), NUM_MAX_PREDICATES ,id);
				if(!first)
					s = s + DELIM; // Separador
				else
					first = false;
				s = s + id;
			}
		}
	}
	return s.c_str();
}


// Devolvemos una cadena de texto con informacion de los tipos raiz, de la forma
// "id:nombre_tipo"
const char * getLiterals(void) {
	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	literaltablecit b, e;
	keylistcit  pb, pe;

	//Variable temporal para guardar IDs
	char id[NUM_MAX_PREDICATES]="";

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador al ultimo elemento
		e = domain_interface->getEndLiteral();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginLiteral();e!=b;b++){
		// Obtenemos los IDs en forma de string
		//La funcion gcvt convierte un int a un char*
		gcvt((int)(*b).second->getId(), NUM_MAX_PREDICATES ,id);
		s = s + id;
		s = s + DELIM;
		s = s + DELIM;
	}
	return s.c_str();
}


// Devolvemos una cadena de texto con informacion de los tipos raiz, de la forma
// "id:nombre_tipo"
const char * getPredicates(void) {
	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	literaltablecit b, e;
	keylistcit  pb, pe;

	//Variable temporal para guardar IDs
	char id[NUM_MAX_PREDICATES]="";

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador al ultimo elemento
		e = domain_interface->getEndLiteral();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginLiteral();e!=b;b++){

		if(!(*b).second->isFunction()) {
			// Obtenemos los IDs en forma de string
			//La funcion gcvt convierte un int a un char*
			gcvt((int)(*b).second->getId(), NUM_MAX_PREDICATES ,id);

			s = s + id;
			s = s + DELIM; // Separador
/*			s = s + (*b).second->getName() + DELIM;

			// Agregamos los argumentos del predicado
			pe = (*b).second->endp();
			for(pb=(*b).second->beginp();pb!=pe;pb++) {
				//TermTable
				Term *  tt = termtable->getTerm(*pb);
				gcvt((int)(tt)->getId(), NUM_MAX_PREDICATES ,id);
				s = s + id;
				s = s + DELIM; // Separador
				s = s + (tt)->getName() + DELIM;
			}*/

			// Fin de los parametros y por consiguiente de este predicado
			s = s + DELIM;

		}
	}
	return s.c_str();
}

// Devolvemos una cadena de texto con informacion de los tipos raiz, de la forma
// "id:nombre_tipo"
const char * getAxioms(void) {
	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	axiomtablecit  b, e;
	//Variable temporal para guardar IDs
	char id[NUM_MAX_AXIOMS]="";

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador al ultimo elemento
		e = domain_interface->getEndAxiom();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginAxiom();e!=b;b++){
			// Obtenemos los IDs en forma de string
			//La funcion gcvt convierte un int a un char*
			gcvt((int)(*b).second->getId(), NUM_MAX_AXIOMS ,id);
			s = s + id;
			s = s + DELIM;
			s = s + DELIM;
	}
	return s.c_str();
}


// Devolvemos una cadena de texto con informacion de los tipos raiz, de la forma
// "id:nombre_tipo"
const char * getAxiomName(int pid) {
	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	axiomtablecit b, e;
	keylistcit  pb, pe;

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador al ultimo elemento
		e = domain_interface->getEndAxiom();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginAxiom();e!=b;b++){
		if((int)(*b).second->getId() == pid) {
			s = s + (*b).second->getName();
		}
	}
	return s.c_str();
}



// Devolvemos una cadena de texto con informacion de los tipos raiz, de la forma
// "id:nombre_tipo"
const char * getAxiomArgsList(int pid) {
	bool first;

	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	axiomtablecit b, e;
	keylistcit  pb, pe;

	//Variable temporal para guardar IDs
	char id[NUM_MAX_PREDICATES]="";
	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador al ultimo elemento
		e = domain_interface->getEndAxiom();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginAxiom();e!=b;b++){
		if((int)(*b).second->getId() == pid) {
			// Agregamos los argumentos del predicado
			first = true;
			pe = (*b).second->endp();
			for(pb=(*b).second->beginp();pb!=pe;pb++) {
				//TermTable
				Term *  tt = termtable->getTerm(*pb);
				gcvt((int)(tt)->getId(), NUM_MAX_PREDICATES ,id);
				if(!first)
					s = s + DELIM; // Separador
				else
					first = false;
				s = s + id;
			}
		}
	}
	return s.c_str();
}

// Devolvemos una cadena de texto con informacion de los tipos raiz, de la forma
// "id:nombre_tipo"
const char * getFunctions(void) {
	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	literaltablecit b, e;
	//Variable temporal para guardar IDs
	char id[NUM_MAX_PREDICATES]="";

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador al ultimo elemento
		e = domain_interface->getEndLiteral();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginLiteral();e!=b;b++){

		if((*b).second->isFunction()) {
			// Obtenemos los IDs en forma de string
			//La funcion gcvt convierte un int a un char*
			gcvt((int)(*b).second->getId(), NUM_MAX_PREDICATES ,id);

			s = s + id;
			s = s + DELIM;
			s = s + DELIM;
		}
	}
	return s.c_str();
}



// Devolvemos una cadena de texto con informacion de las tareas, de la forma
// "id:meta_id:nombre_tarea"
const char * getTaskName(int tid, int mid) {
	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	tasktablecit b, e;

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador a la ultima tarea
	e = domain_interface->getEndTask();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginTask();b!=e;b++) {

		if ((*b).second->getId() == tid && (*b).second->getMetaId() == mid)
			s = s + (*b).second->getName();

		}

	return s.c_str();

}


// Devolvemos una cadena de texto con informacion de las tareas, de la forma
// "id:meta_id:nombre_tarea"
bool existsTask(int tid, int mid) {
	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	tasktablecit b, e;

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador a la ultima tarea
	e = domain_interface->getEndTask();

	bool value = false;
	// Recorremos todas las tareas
	for(b=domain_interface->getBeginTask();b!=e;b++) {

		if ((*b).second->getId() == tid && (*b).second->getMetaId() == mid)
			value = true;

		}

	return value;

}

// Devolvemos una cadena de texto con informacion de las tareas, de la forma
// "id:meta_id:nombre_tarea"
const char * getTasksNames(void) {
	//Valor devuelto
	static string s;
	s = "";

	//Iteradores
	tasktablecit b, e;
	//Variable temporal para guardar IDs
	char id[NUM_MAX_TASK]="";
	char metaid[NUM_MAX_TASK]="";

	// Esperamos a que el dominio este cargado
	while(!domain_interface->loaded);

	// Obtenemos el apuntador a la ultima tarea
	e = domain_interface->getEndTask();

	// Recorremos todas las tareas
	for(b=domain_interface->getBeginTask();b!=e;b++) {
		// Obtenemos los IDs en forma de string
		//La funcion gcvt convierte un int a un char*
		gcvt((int)(*b).second->getId(), NUM_MAX_TASK ,id);
		gcvt((int)(*b).second->getMetaId(), NUM_MAX_TASK ,metaid);

		s = s + id;
		s = s + DELIM; // Separador
		s = s + metaid;
		s = s + DELIM; // Separador
		s = s + (*b).second->getName() + DELIM + DELIM;

		}

	return s.c_str();

}

//Devolvemos verdadero si la tarea es compuesta y false en caso contrario
bool isCompoundTask(int id, int metaid) {

	const Task * t = domain_interface->getTaskMetaID(id, metaid);

	if((*t).isCompoundTask())
		return true;
	else
		return false;

}


//Nos da la lista de precondiciones del metodo
char * getMethodName(int id, int metaid, int m_metaid) {
	//Resultado
	static string s;
	s = "";

	//Iteradores
	methodcit b, e;
	tasklistcit tb, te;

	const Task * t = domain_interface->getTaskMetaID(id, metaid);

	if((*t).isPrimitiveTask()) {
		// Si no es una tarea primitiva no tiene precondiciones
		s = "";
	}
	else {
		// Obtenemos el metodo compuesto
		const CompoundTask * t = (const CompoundTask *) domain_interface->getTaskMetaID(id, metaid);

		// Obtenemos los apuntadores al primer y al ultimo metodo para recorrerlos
		b=t->getBeginMethod();
		e = t->getEndMethod();

		while((*b)->getMetaId() != m_metaid && b!=e) b++;

		// Hemos encontrado el m�todo buscado
		if((*b)->getMetaId() == m_metaid)
			s = s + (*b)->getName();
	}
		return (char *) s.c_str();

}



//Nos devuelve la lista de metodos asociados a la tarea compuesta
char * getMethodsFromCompoundTask(int id, int metaid) {
	//Resultado
	static string s;
	s = "";

	//Iteradores
	methodcit b, e;

	//Variable temporal para guardar IDs
	char t_id[NUM_MAX_TASK]="";
	char t_metaid[NUM_MAX_TASK]="";
	char m_metaid[NUM_MAX_TASK]="";

	const Task * t = domain_interface->getTaskMetaID(id, metaid);

	if((*t).isPrimitiveTask()) {
		// Si no es una tarea primitiva no tiene precondiciones
		return "";
	}
	else {

		// Obtenemos el metodo compuesto
		const CompoundTask * t = (const CompoundTask *) domain_interface->getTaskMetaID(id, metaid);

		// Obtenemos el apuntador al ultimo methodo
		e = t->getEndMethod();

		// Recorremos todos los metodos
		for(b=t->getBeginMethod();b!=e;b++) {
			// Obtenemos los IDs en forma de string
			//La funcion gcvt convierte un int a un char*

			// metaid
			gcvt(id, NUM_MAX_TASK ,t_id);
			gcvt(metaid, NUM_MAX_TASK ,t_metaid);
			gcvt((*b)->getMetaId(), NUM_MAX_TASK ,m_metaid);

			s = s + t_id;
			s = s + DELIM; // Separador
			s = s + t_metaid;
			s = s + DELIM; // Separador
			s = s + m_metaid;
			s = s + DELIM + DELIM; // Separador
// 			s = s + (*b)->getName() + DELIM + DELIM;
		}

		return (char *) s.c_str();
	}
}


//Nos devuelve la lista de tareas asociada al metodo
char * getTaskListFromMethod(int id, int metaid, int m_metaid) {
	//Resultado
	static string s;
	s = "";

	//Iteradores
	methodcit b, e;
	tasklistcit tb, te;

	//Variable temporal para guardar IDs
	char t_id[NUM_MAX_TASK]="";
	char t_metaid[NUM_MAX_TASK]="";

	//Vector unificaciones (no se usa pero es necesario)
	vector<VUndo * > * u = new vector<VUndo*>;

	const Task * t = domain_interface->getTaskMetaID(id, metaid);
	Task * t2;

	if((*t).isPrimitiveTask()) {
		// Si no es una tarea primitiva no tiene precondiciones
		s = "";
	}
	else {
		// Obtenemos el metodo compuesto
		const CompoundTask * t = (const CompoundTask *) domain_interface->getTaskMetaID(id, metaid);

		// Obtenemos los apuntadores al primer y al ultimo metodo para recorrerlos
		b=t->getBeginMethod();
		e = t->getEndMethod();

		while((*b)->getMetaId() != m_metaid && b!=e) {
			b++;
		}

		// Hemos encontrado el metodo buscado
		if((*b)->getMetaId() == m_metaid) {

			//Obtenemos la red de tareas asociadas
			TaskNetwork * tn = (*b)->getTaskNetwork();

			// Obtenemos el apuntador a la ultima tarea de la red
			te = tn->getEndTask();
			// Recorremos todas las tareas de la red
			for(tb=tn->getBeginTask();tb!=te;tb++) {
				// Obtenemos los IDs en forma de string
				//La funcion gcvt convierte un int a un char*

					// Obtenemos la cabecera de la tarea de la red de tareas
					TaskHeader * th = new TaskHeader((const TaskHeader*) *tb);

					// Comprobamos que no sea un inline
					if(strcmp(th->getName(),":inline")!=0) {

						// Hacemos una unificacion de la cabecera con las tareas del dominio
						//para obtener la tarea a la que refiere la cabecera
						vector<Task *>* tv = domain_interface->getUnifyTask(th, u);

						//Como solo puede referir a una obtenemos la primera tarea del
						//vector de unificaciones posibles devuelto por el algoritmo de unificacion del dominio
						t2 = (*tv)[0];

						//Concatenamos a la solucion la tarea obtenida de la forma 'id:metaid:nombretarea'
						gcvt((int)(*t2).getId(), NUM_MAX_TASK ,t_id);
						gcvt((int)(*t2).getMetaId(), NUM_MAX_TASK ,t_metaid);
						s = s + t_id;
						s = s + DELIM; // Separador
						s = s + t_metaid;
						s = s + DELIM + DELIM; // Separador
						//s = s + (*t2).getName() + DELIM + DELIM;
					}
			}

		}
		else{
			s = "";
		}

	}
		return (char *) s.c_str();
}



//Nos da la lista de precondiciones de la tarea primitiva
char * getPreconditionsFromTask(int id, int metaid) {
	//Resultado
	static string s;
	s = "";


	//Variable temporal para guardar IDs
	char cid[NUM_MAX_TASK]="";
	char cmetaid[NUM_MAX_TASK]="";


	const Task * t = domain_interface->getTaskMetaID(id, metaid);

	if(!(*t).isPrimitiveTask()) {
		// Si es una tarea primitiva no tiene metodos asociados
		return "";
	}
	else {

		// Obtenemos el metodo compuesto
		PrimitiveTask * t = (PrimitiveTask *) domain_interface->getTaskMetaID(id, metaid);


		Goal * g = t->getPrecondition();


		// Obtenemos los IDs en forma de string
		//La funcion gcvt convierte un int a un char*
		gcvt(id, NUM_MAX_TASK ,cid);
		gcvt(metaid, NUM_MAX_TASK ,cmetaid);

		s = s + cid;
		s = s + DELIM; // Separador
		s = s + cmetaid;
		s = s + DELIM; // Separador

		if(g!=NULL)
			s = s + g->toString() + DELIM + DELIM;
		else
			s = s + "sin_precondiciones" + DELIM + DELIM;

		return (char *) s.c_str();

	}

}

//Nos da la lista de efectos de la tarea primitiva
char * getEffectsFromTask(int id, int metaid) {
	//Resultado
	static string s;
	s = "";


	//Variable temporal para guardar IDs
	char cid[NUM_MAX_TASK]="";
	char cmetaid[NUM_MAX_TASK]="";

	const Task * t = domain_interface->getTaskMetaID(id, metaid);

	if(!(*t).isPrimitiveTask()) {
		// Si es una tarea primitiva no tiene metodos asociados
		return "";
	}
	else {

		// Obtenemos el metodo compuesto
		PrimitiveTask * t = (PrimitiveTask *) domain_interface->getTaskMetaID(id, metaid);


		const Effect * e = t->getEffect();

		// Obtenemos los IDs en forma de string
		//La funcion gcvt convierte un int a un char*
		gcvt(id, NUM_MAX_TASK ,cid);
		gcvt(metaid, NUM_MAX_TASK ,cmetaid);

		s = s + cid;
		s = s + DELIM; // Separador
		s = s + cmetaid;
		s = s + DELIM; // Separador

		if(e!=NULL)
			s = s + e->toString() + DELIM + DELIM;
		else
			s = s + "sin_efectos" + DELIM + DELIM;


		return (char *) s.c_str();

	}

}

//Nos da la lista de precondiciones del metodo
char * getPreconditionsFromMethod(int id, int metaid, int m_metaid) {
	//Resultado
	static string s;
	s = "";

	//Iteradores
	methodcit b, e;
	tasklistcit tb, te;

	//Variable temporal para guardar IDs
	char t_id[NUM_MAX_TASK]="";
	char t_metaid[NUM_MAX_TASK]="";
	char mm_metaid[NUM_MAX_TASK]="";

	const Task * t = domain_interface->getTaskMetaID(id, metaid);

	if((*t).isPrimitiveTask()) {
		// Si no es una tarea primitiva no tiene precondiciones
		s = "";
	}
	else {
		// Obtenemos el metodo compuesto
		const CompoundTask * t = (const CompoundTask *) domain_interface->getTaskMetaID(id, metaid);



		// Obtenemos los apuntadores al primer y al ultimo metodo para recorrerlos
		b=t->getBeginMethod();
		e = t->getEndMethod();

		while((*b)->getMetaId() != m_metaid && b!=e) {
			b++;
		}


		// Hemos encontrado el metodo buscado
		if((*b)->getMetaId() == m_metaid) {

			Goal * g = (*b)->getPrecondition();

			// Obtenemos los IDs en forma de string
			//La funcion gcvt convierte un int a un char*
			gcvt(id, NUM_MAX_TASK ,t_id);
			gcvt(metaid, NUM_MAX_TASK ,t_metaid);
			gcvt(m_metaid, NUM_MAX_TASK ,mm_metaid);

			s = s + t_id;
			s = s + DELIM; // Separador
			s = s + t_metaid;
			s = s + DELIM; // Separador
			s = s + mm_metaid;
			s = s + DELIM; // Separador

			if(g!=NULL)
				s = s + g->toString() + DELIM + DELIM;
			else
				s = s + "sin_precondiciones" + DELIM + DELIM;
		}
		else{
			s = "";
		}

	}
		return (char *) s.c_str();

}


//Nos da la el n�mero de parametros de una tarea primtiiva
int getNumOfParameters(int id, int metaid) {
	keylistcit e, b;
	//Resultado
	int num = 0;

	const Task * t = domain_interface->getTaskMetaID(id, metaid);

	e = t->parametersEnd();
	for(b=t->parametersBegin();b!=e;b++)
		num++;

	return num;


}

//Nos da la el n�mero de parametros de una tarea primtiiva
char * getParameter(int id, int metaid, int index) {
	//Resultado
	static string s;
	s = "";

	//Parametro
	pkey p;

	//Termino
	Term *te;

	if(getNumOfParameters(id, metaid)>index) {

		const Task * t = domain_interface->getTaskMetaID(id, metaid);

		p = t->getParameter(index);

		te = termtable->getTerm((const pkey) p);

		s = s + te->getName();

	}

	return (char *) s.c_str();

}


/**************************************************************
******************* Funciones de Escritura ********************
***************************************************************/

// Devolvemos el nombre del dominio
void setTaskName(int id, int metaid, const char *n) {

	tasktablecit tb, te;
	tasklistcit tnb, tne;
	methodcit mb, me;



	if (domain_interface->loaded) {

		// CAMBIAMOS EL NOMBRE DE LAS CABECERAS QUE APUNTAN A LA TAREA

		te = domain_interface->getEndTask();
		// Recorremos todas las tareas del dominio
		for(tb=domain_interface->getBeginTask();te!=tb;tb++) {

			Task * t = (Task *) domain_interface->getTask(tb);

			// Si es compuesta recorremos sus metodos
			if((*t).isCompoundTask()) {

				CompoundTask *ct = (CompoundTask *) t;

				// Obtenemos los apuntadores al primer y al ultimo metodo para recorrerlos
				me = ct->getEndMethod();

				// Recorremos todos los metodos de la tarea compuesta
				for(mb=ct->getBeginMethod();me!=mb;mb++) {

					//Obtenemos la red de tareas asociadas
					TaskNetwork * tn = (*mb)->getTaskNetwork();

					// Obtenemos el apuntador a la ultima tarea de la red
					tne = tn->getEndTask();
					// Recorremos todas las tareas de la red
					for(tnb=tn->getBeginTask();tnb!=tne;tnb++) {
							// Obtenemos la cabecera de la tarea de la red de tareas
							TaskHeader * th = new TaskHeader((const TaskHeader*) *tnb);
							// Comprobamos que provenga de la misma tarea que de la que editamos el nombre
							// y si es asi cambiamos el nombre
							if(th->getId() == id) {
								(*th).setName(n);
							}
					}



				}

			}

		}

		// CAMBIAMOS EL NOMBRE DE LA TAREA
		Task * t = (Task *) domain_interface->getTaskMetaID(id, metaid);
		(*t).setName(n);

	}

}



/**************************************************************
******************* Funciones de Varias  **********************
***************************************************************/

char * printDomain(void) {
	//Valor devuelto
	static string s;
	s = "";
	//Iteradores
	tasktablecit b, e;

	// Obtenemos el apuntador a la ultima tarea
	e = domain_interface->getEndTask();

	//Recorremos todas las tareas, primero en busqueda de las compuestas y luego en busqueda de las primitivas
	s = "Tareas Compuestas:\n";

	//printf("Tareas Compuestas:\n");
	for(b=domain_interface->getBeginTask();b!=e;b++) {
		if((*b).second->isCompoundTask())
			//printf("%s\n", (*b).second->getName());
			s = s + (*b).second->getName() + "\n";
	}

	//printf("\nTareas Simples:\n");
	s = s + "\nTareas Simples:\n";
	for(b=domain_interface->getBeginTask();b!=e;b++) {
		if((*b).second->isPrimitiveTask())
			//printf("%s\n", (*b).second->getName());
			s = s + (*b).second->getName() + "\n";
	}

	return (char *) s.c_str();

}

char * domainToPDDL(int ind) {
	static ostringstream *ost = new ostringstream();

	static string s;
	s = "";

	domain_interface->print(ost, ind);

	s = (string) ost->str();

	printf("%s\nFIN", s.c_str());

	return (char *) s.c_str();

}

char * domainWarnings() {
	return "Not Implemented";

}
