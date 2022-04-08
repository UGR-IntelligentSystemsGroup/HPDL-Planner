#include "textTag.hh"

// función para hacer el procesado de los metatags de texto interpretado
string processTextTag(const string & input, const ParameterContainer * pc) {
    string output;
    vector<pair<pair<int,int>, string> > counters;
    keylistcit bk, ek;
    char modificador = 'n';
    bool detected_modificador = false;

    // Inicializar la estructura de contadores
    if(pc){
	ek = pc->parametersEnd();
	for(bk = pc->parametersBegin(); bk!=ek; bk++){
	    output = parser_api->termtable->getTerm(*bk)->getName();
	    output = output.substr(1,output.size());
	    counters.push_back(make_pair(make_pair(0,output.size()),output));
	}
    }
    counters.push_back(make_pair(make_pair(0,5),"start"));
    counters.push_back(make_pair(make_pair(0,3),"end"));
    counters.push_back(make_pair(make_pair(0,8),"duration"));

    ostringstream os;
    string::const_iterator b, e = input.end(),mark;
    vector<pair<pair<int,int>,string> >::iterator cb, ce = counters.end();
    bool searching;
    int longest;
    bool inVar = false;

    //for(cb = counters.begin();cb!=ce;cb++){
    //    cerr << (*cb).second << endl;
    //}

    //cerr << "++++" << input << endl;
    for(b=input.begin();b!=e;b++){
	// Mientras no encontremos una interrogación se copia la entrada directamente en
	// la salida
	//cerr << "Examinando: " << (*b) << endl;
	if(!inVar){
	    detected_modificador = false;
	    if(*b == '?'){
		//cerr << "Detectado inicio variable "<< endl;
		mark = b;
		// Estamos en una variable, es posible una substitución
		inVar = true;
		// Detectando modificadores
		if((b+1) != e && (b+2) != e && *(b+1) == '%'){
		    modificador = *(b+2);
		    b = b+2;
		    detected_modificador = true;
		}
	    }
	    else {
		//cerr << os.str() << endl;
		//cerr << "--> " << (*b) << endl;
		os << *b;
	    }
	}
	else{
	    // Recorremos todas las cadenas a la vez buscando las cadenas que podemos
	    // sustituir.
	    searching = false;
	    for(cb = counters.begin();cb!=ce;cb++){
		if((*cb).first.first != -1){
		    if((*cb).first.first < (*cb).first.second){
			//cerr << (*b) << " " << toupper(*b) << endl;
			//cerr << (*cb).second[(*cb).first.first] << " " << toupper((*cb).second[(*cb).first.first]) << endl;
			if(toupper((*cb).second[(*cb).first.first]) == toupper(*b)){
			    // Hasta ahora la cadena va coincidiendo
			    (*cb).first.first++;
			    searching = true;
			}
			else{
			    // Marcamos la cadena como inválida
			    (*cb).first.first = -1;
			}
		    }
		}
	    }

	    if(!searching){
		// Ninguna de las cadenas coincide o ya se sustituyó una cadena
		inVar = false;

		// Recorremos de nuevo el vector, reinicializando los contadores
		// y buscando la cadena más larga que se ha encontrado completamente
		longest = -1;

		for(cb = counters.begin();cb!=ce;cb++){
		    if((*cb).first.first == (*cb).first.second){
			if (longest == -1)
			    longest = distance(counters.begin(),cb);
			else if(counters[longest].first.first < (*cb).first.first)
			    // Ya había una sustitución pero la nueva sustución es más larga
			    // que la anterior.
			    longest = distance(counters.begin(),cb);
		    }
		    (*cb).first.first = 0;
		}

		if(longest != -1){
		    // Hemos encontrado una substitución
		    // sustituimos y colocamos el puntero para
		    // que siga después de la sustitución
		    if(detected_modificador){
			os << "$" << modificador << (longest +1);
			b = mark + (counters[longest].first.second + 2);
		    }
		    else{
			os << "$" << (longest +1);
			b = mark + (counters[longest].first.second);
		    }
		    //cerr << os.str() << endl;
		    //cerr << "Se sustituyó: " << longest << endl;
		    //cerr << "--> " << "$" << longest << endl;
		    //cerr << "B apunta a: " << (*b) << endl;
		}
		else {
		    // No hay sustituciones, escribimos todo lo que nos dejamos
		    // atrás
		    os << input.substr(distance(input.begin(),mark),distance(mark,b)+1);
		    //cerr << "--> " << input.substr(distance(input.begin(),mark),distance(input.begin(),b)) << endl;
		}
	    }
	}
    }


    if(inVar){
	// Se acabó la cadena mientras estabamos buscando una substitución, es necesario escribir
	// lo que queda al final.
	longest = -1;

	for(cb = counters.begin();cb!=ce;cb++){
	    if((*cb).first.first == (*cb).first.second){
		if (longest == -1)
		    longest = distance(counters.begin(),cb);
		else if(counters[longest].first.first < (*cb).first.first)
		    longest = distance(counters.begin(),cb);
	    }
	    (*cb).first.first = 0;
	}

	if(longest != -1){
	    if(detected_modificador){
		os << "$" << modificador << (longest +1);
		b = mark + (counters[longest].first.second+2);
	    }
	    else{
		os << "$" << (longest +1);
		b = mark + (counters[longest].first.second);
	    }
	    b++;
	    if(b!=e)
		// aún así puede quedar algo por el final
		os << input.substr(distance(input.begin(),b),distance(input.begin(),e));
	}
	else {
	    os << input.substr(distance(input.begin(),mark),distance(input.begin(),b));
	}
    }

    output = os.str();
    //exit(0);
    return output;
};

TextTag::TextTag(const char * n)
    :Tag(n){
};

