#include "xmlwriter.hh"

XmlWriter::XmlWriter(ostream * out){
    flow = out;
    attribs = 0;
    text = 0;
    indent = 3;
    open = false;
    (*flow) << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" standalone=\"yes\"?>" << endl;
};

XmlWriter::~XmlWriter(void){
    flush();
};

XmlWriter * XmlWriter::startTag(const char * name){
    string in(stk.size()*indent,' ');
    if(open){
	if(attribs){
	    (*flow) << attribs->str();
	    delete attribs;
	    attribs = 0;
	}
	(*flow) << ">" << endl;
	if(text){
	    (*flow) << in << text->str() << endl;
	    delete text;
	    text = 0;
	}
    }
    open = true;
    stk.push(name);
    (*flow) << in << "<" << name;
    return this;
};

XmlWriter * XmlWriter::endTag(void){
    if(!stk.empty()){
	string in((stk.size()-1)*indent,' ');
	if(open){
	    if(attribs){
		(*flow) << attribs->str();
		delete attribs;
		attribs = 0;
	    }
	    (*flow) << ">" << endl;
	    if(text){
		(*flow) << in << text->str() << endl;
		delete text;
		text = 0;
	    }
	}
	(*flow) << in << "</" << stk.top() << ">" << endl;
	open = false;
	stk.pop();
    }
    return this;
};


XmlWriter * XmlWriter::addAttrib(const char * name, const char * value){
    if(!attribs)
	attribs = new ostringstream;
    (*attribs) << " " << name << "=\"";
    escapeXml(attribs,value); 
    (*attribs) << "\"";
    return this;
};

XmlWriter * XmlWriter::addAttrib(const char * name, const string & value){
    if(!attribs)
	attribs = new ostringstream;
    (*attribs) << " " << name << "=\"";
    escapeXml(attribs,value.c_str()); 
    (*attribs) << "\"";
    return this;
};

XmlWriter * XmlWriter::addAttrib(const char * name, int value){
    if(!attribs)
	attribs = new ostringstream;
    (*attribs) << " " << name << "=\"" << value << "\"";
    return this;
};

XmlWriter * XmlWriter::addAttrib(const char * name, double value){
    if(!attribs)
	attribs = new ostringstream;
    (*attribs) << " " << name << "=\"" << value << "\"";
    return this;
};

XmlWriter * XmlWriter::addCharacter(const char * value){
    if(!text)
	text = new ostringstream;
    escapeXml(text,value);
    return this;
};

XmlWriter * XmlWriter::addCharacter(const string & value){
    if(!text)
	text = new ostringstream;
    escapeXml(text,value.c_str());
    return this;
};

void XmlWriter::flush(){
    endTag();
    while(!stk.empty()){
	string in(stk.size()*indent,' ');
	(*flow) << in << "</" << stk.top() << ">" << endl;
	stk.pop();
    }
};

void XmlWriter::escapeXml(ostream * flow, const char * content){
    while(true){
	switch(*content){
	    case '\0':
		return;
	    case '<':
		(*flow) << "&lt;";
		break;
	    case '>':
		(*flow) << "&gt;";
		break;
	    case '&':
		(*flow) << "&amp;";
		break;
	    case '\"':
		(*flow) << "&quot;";
		break;
	    case '\'':
		(*flow) << "&apos;";
		break;
	    default:
		(*flow) << *content;
	}
	content++;
    }
};

