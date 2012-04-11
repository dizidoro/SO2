// EPOS-- Synchronizer Abstractions Common Package

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#ifndef __synchronizer_h
#define __synchronizer_h

#include <system/config.h>
#include <cpu.h>
#include <thread.h>

__BEGIN_SYS

class Synchronizer_Common
{
protected:
    Synchronizer_Common() {}
    
private:
    static const bool busy_waiting = Traits<Thread>::busy_waiting;

protected:
    // Atomic operations
    bool tsl(volatile bool & lock) { return CPU::tsl(lock); }
    int inc(volatile int & number) { return CPU::finc(number); }
    int dec(volatile int & number) { return CPU::fdec(number); }

    // Thread operations
    void sleep() {
	if(!busy_waiting) // configurable feature
	    Thread::yield();
    }
    void wakeup() {
	if(!busy_waiting) // configurable feature
	    ; // a real wakeup comes here
    }
    void wakeup_all() {
	if(!busy_waiting) // configurable feature
	    ; // a real wakeup_all comes here
    }
};

__END_SYS

#endif
