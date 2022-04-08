#ifndef UNIFIERTABLE_H
#define UNIFIERTABLE_H

#include "constants.hh"
#include <iostream>
#include <vector>
#include <algorithm>
#include "unifier.hh"

using namespace std;

typedef vector<Unifier *> unifierTable;
typedef unifierTable::const_iterator unifiercit;
typedef unifierTable::iterator unifierit;

class UnifierTable 
{

    protected:
        unifierTable utable; /**< @brief Vector de Substitutions*/

    public: 
	/**
	 * @brief Constructor por defecto.
	 */
	UnifierTable();

	/**
	 * @brief Constructor de copia.
	 */
	UnifierTable(const UnifierTable * o);

        /**
          @brief Destructor
         */  
        ~UnifierTable();

        /**
          @brief Anade una Unifier al unificador
          @param uni: Unifier que anadiremos al unificador
         */
        inline void addUnifier(Unifier * uni) {utable.push_back(uni);};

        /**
           @brief iterador al primer elemento para recorrer la estructura.
        */
        inline unifierit getUnifierBegin(void) {return utable.begin();};

        /**
           @brief iterador al elemento uno pasado al �ltimo para recorrer la estructura.
        */
        inline unifierit getUnifierEnd(void) {return utable.end();};

        inline Unifier * getback(void) {Unifier * u = utable.back(); utable.pop_back(); return u;};

	/**
	 * Borra el unificador en la posici�n pos.
	 * /param pos la posici�n a borrar
	 */
        void erase(int pos) {unifierit i = utable.begin() + pos; Unifier * u = (*i); utable.erase(i); delete u;};

	/**
	 * Borra todos los unificadores
	 */
        void eraseAll(void);

        Unifier *  getUnifierAt(int pos) {unifierit i = utable.begin() + pos; return (*i);};
        
        /**
            @brief Dado un iterador menor que el getUnifierEnd() devuelve su objeto asociado
        */
        inline const Unifier * getUnifier(unifierit i) {return (*i);}; 

        /**
            @brief a�ade los unificadores de ut a this. Ut se queda vac�a de unificadores en el proceso.
        */
        void addUnifiers(UnifierTable * t);

        /**
            @brief Funci�n de uso interno, no llamar directamente
        */
        inline void clearUnifiers(void) {utable.clear();};

        inline bool isEmpty(void) {return utable.empty();};

        void print(ostream * os) const;

        inline int countUnifiers(void) {return utable.size();};

	// Elimina todos los elementos de la tabla de unificaciones excepto el primero
	void cut(void);
};

#endif
