#ifndef TASK_H
#define TASK_H

#include "constants.hh"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "variablesymbol.hh"
#include "termTable.hh"
#include "unifiable.hh"
#include "expression.hh"
#include "header.hh"
#include "literal.hh"
#include "goal.hh"

/** Esta estructura sirve para mantener entre otras cosas los maintains
 * que deben ser satisfechos
 */
//typedef vector<Goal *> vGoal;
//typedef vGoal::const_iterator vgoalcit;
//typedef vGoal::const_iterator vgoalit;

typedef pair<unsigned int, unsigned int> TPoints;

using namespace std;

class Method;
class CompoundTask;
class TaskHeader;

/**
 * Esta es la clase base para todas las tareas, ya sean primitivas
 * o abstractas.
*/

class Task : public Expression, public Header
{
    public:

        /**
         * @brief Constructor
         * @param name el nombre de la tarea
         */
        Task(int id, int mid) :Header(id,mid) {parent=0; parentMethod=0;parentHeader=0; uni =0;};

        Task(int id, int mid, const KeyList * v) :Header(id,mid,v) {parent=0;parentMethod=0;parentHeader=0;uni=0;};

        virtual ~Task(void) {};

        /**
         * @brief Funcion abstracta que define si la tarea es primitiva
         */
        inline virtual bool isPrimitiveTask(void) const {return false;};
        /**
         * @brief Funcion abstracta que define si la tarea es concreta
         */
        inline virtual bool isCompoundTask(void) const {return false;};;
        /**
         * @brief Funcion abstracta que define si la tarea es un objetivo
         */
        inline virtual bool isGoalTask(void) const {return false;};
        /**
         * @brief La funci�n es una descripci�n de acci�n
         */
        inline virtual bool isTaskHeader(void) const {return false;};

        inline virtual bool isDurative(void) const {return false;};

        /**
         * @brief Genera una cadena descriptiva con el contenido de la tarea
         * Solo imprime las cabeceras
         */
        virtual void printHead(ostream * os) const = 0;

        /**
        * Ampliamos los par�metros que se le pueden pasar a la funci�n toxml.
        * Se puede obtener una descripci�n mas simple (s�lo cabecera) o completa.
        * @param complete marca si se quiere una descripci�n detallada.
        **/
        virtual void toxml(XmlWriter * writer, bool complete) const = 0;

        virtual void toxml(XmlWriter * writer) const {toxml(writer,true);};

        virtual pkey getTermId(const char * name) const = 0;

        /**
        * Testea si una vez cargado el dominio y el problema,
        * puede haber alguna precondici�n que no sea alcanzable.
        * Es un an�lisis muy simple pero puede ayudar a descubrir algunos
        * errores.
        */
        virtual bool isReachable(ostream * err) const = 0;

        /**
        * Comprueba si eventualmente una acci�n podr�a proporcionar
        * un literal dado.
        */
        virtual bool provides(const Literal * l) const = 0;

        /**
        * Devuelve la tarea que gener� a this.
        * null en caso de que no exista.
        */
        inline CompoundTask * getParent(void) const {return parent;};

        /**
        * Establece la tarea que gener� a this.
        */
        inline void setParent(CompoundTask * t) {parent= t;};

        /**
        * Devuelve el m�todo de la tarea padre que gener� a this.
        * null en caso de que no exista.
        */
        inline Method * getParentMethod(void) const {return parentMethod;};

        /**
        * Establece el m�todo de la tarea padre que gener� a this.
        */
        inline void setParentMethod(Method * m) {parentMethod= m;};

        /**
        * Devuelve la cabecera a la que sustitu� en la red de tareas.
        * null en caso de que no exista.
        */
        inline TaskHeader * getParentHeader(void) const {return parentHeader;};

        /**
        * Establece la cabecera a la que sustitu� en la red de tareas.
        */
        inline void setParentHeader(TaskHeader * h) {parentHeader= h;};

        /**
        * Devuelve el unificador que se utiliz� para sustituir a mis variables
        * en caso de que est� totalemente instanciada
        * null en caso de que no exista.
        */
        inline Unifier * getUnifier(void) const {return uni;};

        /**
        * Establece el unificador que se utiliz� para sustituir a mis variables
        * en caso de que est� totalemente instanciada
        * null en caso de que no exista.
        */
        inline void setUnifier(Unifier * u) {uni = u;};

    protected:
        /** Lista de v�nculos protegidos que el planificador debe mantener
        * durante la expansi�n de las tareas.
        * En el caso de que el maintain se encuentre en una tarea compuesta
        * las tareas que se encuentren en su red de tareas asociada, heredaran
        * los v�nculos a mantener.
        * En el caso de que el maintain llegue a una tarea primitiva, previa
        * herencia desde una tarea compuesta, la tarea primitiva al aplicar
        * los efectos debe de verificar que los maintains no son violados. */
//        vGoal * maintainList;

        /**
        * Quien es la tarea que me genero o null si no me gener� nadie.
        */
        CompoundTask * parent;

        /**
        * El m�todo por cuya expansi�n me gener�.
        */
        Method * parentMethod;

        /**
        * La TaskHeader a la que sustitu� en la red de tareas.
        */
        TaskHeader * parentHeader;

        /**
        * El unificador que se utiliz� para hacer la sustituci�n de mis variables.
        */
        Unifier * uni;

};

#endif
