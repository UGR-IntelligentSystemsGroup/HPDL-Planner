/**
  * @file clock.h
  * @brief Implements classes related to time
  *
  * Precision depends on the system.
  * 
  * Module: PocimaUtils
  */


#ifndef __POC_CLOCK_H__
#define __POC_CLOCK_H__

using namespace std;

/**
 *  @brief Abstract chronometer.
 *
 *  Example:
 *  @include test_clock2.cxx
 *
 */
namespace Pocima{
class Chronometer {
        bool running; /**< The Chorno is running */
        float accumulated; /**< Accumulated time */
        float last; /**< last time where the Chrono was started */
    protected:
        /** @brief defines the meaning of the chrono 
         * return time from external clock (system clock, used time of the process...)
         */
        virtual float Current() const=0; /**< defines the meaning of the chrono */
    public:
        Chronometer() { Reset(); }
        virtual ~Chronometer() {}
        /** @brief stops and reset to 0.0 */
        void Reset() { running= false; accumulated= last= 0.0f; }
        /** @brief Starts or Continues. No-op if it's running*/
        void Start() { if (!running) { last= Current(); running= true;} }
        /** @brief Stop. No-op if it's stopped */
        void Stop() { if (running) {accumulated+=Current()-last;running= false;}}
        /** @brief Return true if the chrono is running */
        bool Running() const { return running;}
        /** @brief Return current time. The chrono can be running */
        float Time() const {
            if (running){
              return accumulated+Current()-last;
            }else return accumulated;
        }
        /** @brief Return type of time which the chrono is designed for */
        virtual const char * Description() const=0;
};

// ____________________________________________________________________________ 

/**
 *  @brief Chronometer for real Time.
 *
 *  Example:
 *  @include test_clock1.cxx
 *
 */
class ChronometerRealTime: public Chronometer {
        float Current() const;
    public:
        const char * Description() const {return "Real Time"; }
};

// ____________________________________________________________________________ 

/**
 *  @brief Chronometer for user's used time.
 *
 *  Example:
 *  @include test_clock1.cxx
 *
 */
class ChronometerUsedTime: public Chronometer {
        float Current() const;
    public:
        const char * Description() const {return "Used Time"; }
};

// ____________________________________________________________________________ 

/**
 *  @brief Chronometer for time used by the system.
 *
 *  Example:
 *  @include test_clock1.cxx
 *
 */
class ChronometerSystemTime: public Chronometer {
        float Current() const;
    public:
        const char * Description() const {return "System Time"; }
};

}


#endif

