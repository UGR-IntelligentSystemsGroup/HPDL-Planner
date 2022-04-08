#ifndef HEADER_HH
#define HEADER_HH

using namespace std;

#include "parameterContainer.hh"

class Header :public ParameterContainer
{
    public:
        /**
          @brief Constructor para la clase header
        */
        Header(int i, int mid)  {id=i;metaid=mid;};

        Header(int i, int mid, const KeyList * v)
            : ParameterContainer(v) 
        {
	    id = i;
	    metaid = mid;
        };

	Header(void) {id=-1;metaid=-1;};

	Header(const Header * h) :ParameterContainer(h) {id= h->getId(); metaid = h->metaid;};

        virtual ~Header(void) {};

        virtual void headerPrint(ostream * os) const
        {
            //*os << "(" << getName() << " [" << getId() << "]";
            *os << "(" << getName();
            for_each(parameters.begin(),parameters.end(),PrintKey(os));
            *os << ")";
        };

	virtual int getId(void) const {return id;};

	virtual void setId(int i) {id = i;};

	virtual int getMetaId(void) const {return metaid;};

	virtual void setMetaId(int i) {metaid = i;};

	virtual const char * getName(void) const;

	virtual void setName(const char * n);

    protected:
	int metaid;
	int id;

};

#endif
