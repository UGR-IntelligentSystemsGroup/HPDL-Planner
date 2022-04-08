/**
  * @file clock.cxx
  *
  *  Module: PocimaUtils
  */

#include <clock.hh>
#include <sys/times.h> // times, ...
#include <sys/resource.h>
#include <unistd.h> // sysconf

namespace Pocima {

// ____________________________________________________________________________ 

float ChronometerRealTime::Current() const
{
    static long int clk_tck = sysconf(_SC_CLK_TCK); // Clocks per second
    tms aux;  // No me funciona la funci�n clock.....
    return float(times(&aux))/clk_tck;

}

// ____________________________________________________________________________ 

float ChronometerUsedTime::Current() const
{
    rusage tmp;
    getrusage(RUSAGE_SELF,&tmp);
    return tmp.ru_utime.tv_sec+tmp.ru_utime.tv_usec/1000000.0f;
}


// ____________________________________________________________________________ 

float ChronometerSystemTime::Current() const
{
    rusage tmp;
    getrusage(RUSAGE_SELF,&tmp);
    return tmp.ru_stime.tv_sec+tmp.ru_stime.tv_usec/1000000.0f;
}


} // namespace

