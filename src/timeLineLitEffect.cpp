#include "timeLineLitEffect.hh"
#include <sstream>
#include <math.h>
#include "state.hh"
#include "undoARLiteralState.hh"
#include "problem.hh"
#include "papi.hh"

#ifdef USE_AC3
#include "tcnm-ac3.hh"
#endif

#ifdef USE_PC2
#include "tcnm-pc2.hh"
#endif

#ifdef USE_PC2_CL
#include "tcnm-pc2-cl.hh"
#endif

TimeLineLiteralEffect::TimeLineLiteralEffect(int id, int mid, bool p)
    :LiteralEffect(id,mid,p)
{
    duration = 0;
    gap = 0; 
    nmax = nmin = -1;
};

TimeLineLiteralEffect::TimeLineLiteralEffect(int id, int mid, const KeyList * param, bool p)
    :LiteralEffect(id,mid,param,p)
{
    duration = 0;
    gap = 0; 
    nmax = nmin = -1;
}

TimeLineLiteralEffect::TimeLineLiteralEffect(const TimeLineLiteralEffect * le) 
    :LiteralEffect(le), intervals(le->intervals)
{
    start = le->start;
    duration = le->duration;
    gap = le->getGap();
    nmax = le->nmax;
    nmin = le->nmin;
};

Expression * TimeLineLiteralEffect::clone(void) const
{
    return new TimeLineLiteralEffect(this);
}

Literal * TimeLineLiteralEffect::cloneL(void) const
{
    return new TimeLineLiteralEffect(this);
}

void TimeLineLiteralEffect::print(ostream * os, int indent) const
{
    string s(indent,' ');

    *os << s;
    if(isTimed()){
	*os << "(between " << start << " and " << start + duration << " ";
	if(gap) {
	    *os << "and every " << gap << " ";
	}
    }
    if(!polarity)
	*os << "(not ";
    headerPrint(os);
    if(!getPolarity())
	*os << ")";
    if(isTimed())
	*os << ")";
}

void TimeLineLiteralEffect::toxml(XmlWriter * writer) const{
    writer->startTag("predicate")
	->addAttrib("name",getName());

    if(!polarity)
	writer->addAttrib("polarity","negated");
    else 
	writer->addAttrib("polarity","affirmed");

    for_each(parameters.begin(),parameters.end(),ToXMLKey(writer));
    writer->endTag();
}

void TimeLineLiteralEffect::generateNIntervals(int min,int max){
    int v;
    if(gap == 0 && intervals.empty()){
	intervals.push_back(make_pair(start,start+duration));
    } else{
	if(min < nmin || nmin == -1){
	    // insertar por el inicio del vector es costoso, seguramente
	    // esta operaci�n sea m�s r�pida.
	    VIntervals aux;
	    if(nmin == -1)
		nmin = 0;

	    for(int i = nmin; i < min; i++){
		// cerr << i << endl;
		v = start + i*(duration+gap);
		aux.push_back(make_pair(v,v+duration));
	    }
	    aux.insert(aux.end(),intervals.begin(),intervals.end());
	    intervals.swap(aux);
	    nmin = min;
	}

	if(max > nmax || nmax == -1)
	    if(nmax == -1)
		nmax = 0;
	    for(int i = nmax; i < max; i++){
		v = start + i*(duration+gap);
		intervals.push_back(make_pair(v,v+duration));
	    }
	nmax = max;
    }
}

void TimeLineLiteralEffect::generateTIntervals(int min,int max){
    int v;
    if(gap == 0 && intervals.empty()){
	intervals.push_back(make_pair(start,start+duration));
    }
    else{
	if(min < intervals.front().first){
	    // Calcular el intervalo en el que cae min
	    int t = (int) floor((min - start)*1.0/(duration + gap));
	    if(t < nmin || nmin == -1){
		VIntervals aux;
		if(nmin == -1)
		    nmin = 0;
		for(int i = nmin; i < t; i++){
		    v = start + i*(duration+gap);
		    aux.push_back(make_pair(v,v+duration));
		}
		aux.insert(aux.end(),intervals.begin(),intervals.end());
		intervals.swap(aux);
		nmin = t;
	    }
	}

	if(max > intervals.back().second){
	    int t = (int) floor((max - start)*1.0/(duration + gap));
	    if(nmax == -1)
		nmax = 0;
	    for(int i = nmax; i < t; i++){
		v = start + i*(duration+gap);
		intervals.push_back(make_pair(v,v+duration));
	    }
	    nmax = t;
	}
    }
}

void TimeLineLiteralEffect::merge(VIntervals & other,int  min, int max){
    
    if(min < 0 || max < 0){
	other.clear();
	return;
    }
    if(min >= POS_INF || max >= POS_INF){
	other.clear();
	return;
    }
    int tmin = (int) floor(((min - start)*1.0)/(duration + gap));
    if(tmin < 0)
	tmin = 0;
    int tmax = (int) ceil(((max - start)*1.0)/(duration + gap));
    generateNIntervals(tmin,tmax);
    if (intervals.size() && intervals[0].first <0)
	intervals[0].first = 0;
    /*for(int i =0; i < (int) intervals.size(); i++)
	cerr << intervals[i].first << " -- " << intervals[i].second << endl;*/
    //cerr << min << " " << max << " " << tmin << " " << tmax << endl;
    //cerr << start << " " << duration << " " << gap << endl;

    if(tmin > 0 && gap == 0){
	other.clear();
	return;
    }

    if(other.empty()) {
	if(gap)
	    other.insert(other.end(),intervals.begin()+tmin,intervals.begin()+tmax);
	else
	    other.push_back(*(intervals.begin()));
	return;
    }

    VIcite i = intervals.begin() + tmin;
    VIcite ie = intervals.end();
    VIcite j, je = other.end();
    for(j=other.begin();j!= je && (*j).first < min; j++);

    int imin, imax;

    VIntervals * ret = new VIntervals();
    while(i != ie && j != je && (*j).first < max && (*i).first < max){
	if((*i).second < (*j).first)
	    i++;
	else if((*j).second < (*i).first)
	    j++;
	else{
	    // hay intersecci�n entre los intervalos
	    // me quedo con el m�ximo de los m�nimos
	    if((*i).first < (*j).first)
		imin = (*j).first;
	    else
		imin = (*i).first;

	    // y con el m�nimo de los m�ximos 
	    if((*i).second > (*j).second){
		imax = (*j).second;
		j++;
	    }
	    else{
		imax = (*i).second;
		i++;
	    }

	    ret->push_back(make_pair(imin,imax));
	}
    }

    ret->swap(other);
    delete ret;
};

void TimeLineLiteralEffect::addTPoint(int n, pair<int,int> tp){
    tpoints.push_back(make_pair(n,tp));
};

int TimeLineLiteralEffect::getIndexInterval(int t) const{
    int r = (int) floor((1.0*(t - start))/(duration + gap));
    if(r <0)
	return 0;
    else
	return r;
};

struct EQUAL_FIRST
{
    int param;
    EQUAL_FIRST(int p) {param=p;};

    bool operator()(const pair<int,pair<int,int> > & p) const {
	return p.first == param;
    };
};

pair<int,int> TimeLineLiteralEffect::getTPoint(int n) const{
    vector<pair<int,pair<int,int> > >::const_iterator i = find_if(tpoints.begin(),tpoints.end(),EQUAL_FIRST(n));
    if (i != tpoints.end()){
	return (*i).second;
    }
    return make_pair(-1,-1);
};


struct GEQ_SECOND
{
    int param;
    GEQ_SECOND(int p) {param=p;};
    bool operator()(pair<int,pair<int,int> > & p) const{
	return p.second.first >= param;
    };
};

void TimeLineLiteralEffect::eraseTPoint(int t){
    tpoints.erase(remove_if(tpoints.begin(),tpoints.end(),GEQ_SECOND(t)),tpoints.end());
}

