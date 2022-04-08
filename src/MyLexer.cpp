#include "MyLexer.hh"
#include <sstream>
#include <algorithm>

#ifdef BUFFER_STACK
#define yy_current_buffer *yy_buffer_stack
#endif

extern ifstream * openfile(const char * fileName);

MyLexer::MyLexer(istream* arg_yyin, ostream* arg_yyout)
    :yyFlexLexer(arg_yyin,arg_yyout)
{
    initErrorTable();
    lineNum =1;
    n_errors=0;
    n_warnings=0;
    fileName = "";
}

MyLexer::~MyLexer(void)
{
}

void MyLexer::LexerError(const char * msg)
{
    n_errors++;
    *errflow << fileName << ":" << lineNum << ": error: " << replace(msg) << endl;
}

void MyLexer::LexerWarning(const char * msg)
{
    n_warnings++;
    *errflow << fileName << ":" << lineNum << ": warning: " << msg << endl;
}

const char * MyLexer::replace(const char * msg)
{
    ostringstream out;
    static string s;
    string word;

    int sizemsg = strlen(msg);
    int ini = -1;
    str_hashcite p;

    // este bucle extrae palabras en buffer separadas por espacios retornos de
    // carro o tabulaciones...
    // Si la palabra es encontrada en el diccionario es sustituida por su sin�nimo.
    // en otro caso se mantiene la misma palabra.
    for(int i=0; i<sizemsg; i++) {
	// hay un espacio
	if(!isalnum(msg[i]) && msg[i]!='_' && msg[i]!='$') {
	    if(ini != -1){
		word.assign(&(msg[ini]),(i - ini));
		if(strcmp(word.c_str(),"PDDL_NAME")==0)
		    out << "`" << yytext <<  "'" << msg[i];
		else if((p = dictionary.find(word.c_str())) != dictionary.end())
		    out << (*p).second << msg[i];
		else
		    out << word << msg[i];
	    }
	    else
		out << msg[i];
	    ini = -1;
	}
	else if (ini == -1){
	    ini = i;
	}
    }

    // aun queda alguna palabra por analizar.
    if(ini != -1)
    {
	word.assign(&(msg[ini]),(strlen(msg) - ini));
	if(strcmp(word.c_str(),"PDDL_NAME")==0)
	    out << " " << yytext << "'";
	else if((p = dictionary.find(word.c_str())) != dictionary.end())
	    out << " " << (*p).second;
	else
	    out << " " << word;
    }
    s = out.str();
    return s.c_str();
}

void MyLexer::switchTo(string file){
    if(stack.size() >= MAX_INCLUDE_DEPTH){
	*errflow << fileName << ":" << lineNum << "Maximun allowed nested imports reached." << endl;
	exit(EXIT_FAILURE);
    }
    string path = "";
    if(file[0] != '/'){
	path = fileName.substr(0,fileName.find_last_of("/") + 1);
    }
    file = path + file;
    ifstream * flow = openfile(file.c_str());
    if(!flow){
	*errflow << fileName << ":" << lineNum << endl;
	exit(EXIT_FAILURE);
    }
    fileName = file;
    lineNum = 1;
    LexerContext c;
    c.fileName = fileName;
    c.line = lineNum;
    c.flow = flow;
    c.buffer = yy_current_buffer;
    stack.push_back(c);

    yy_switch_to_buffer(yy_create_buffer((istream*)flow,256));
};

bool MyLexer::restore(void){
    if(stack.empty())
	return false;

    LexerContext c;
    c = stack.back();
    stack.pop_back();
    fileName = c.fileName;
    lineNum = c.line;
    yy_delete_buffer(yy_current_buffer);
    yy_switch_to_buffer(c.buffer);
    return true;
};
