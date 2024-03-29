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
    Queue<Thread> _waiting;

protected:
    
    // Atomic operations
    bool tsl(volatile bool & lock) { return CPU::tsl(lock); }
    int inc(volatile int & number) { return CPU::finc(number); }
    int dec(volatile int & number) { return CPU::fdec(number); }


    // Thread operations

    void sleep() {
	if(!busy_waiting) {
       	    Thread * running = Thread::running();
	    running->sync_queue(&_waiting);
            _waiting.insert(running->link_sync());
            running->suspend(); //implicitamente reabilita as interrupcoes
        }
	if(Traits<Thread>::active_scheduler)
            CPU::int_enable();
    }
    void wakeup() {
        if(!busy_waiting) {// configurable feature
            if(!_waiting.empty()){
	        Thread * released = _waiting.remove()->object();
		released->resume(); //implicitamente reabilita as interrupcoes
	    }
        }
	if(Traits<Thread>::active_scheduler)
            CPU::int_enable();
    }
    void wakeup_all() {
        if(!busy_waiting) {// configurable feature
        
            Thread * released;
            while(!_waiting.empty()) {
                released = _waiting.remove()->object();
                released->resume(); //implicitamente reabilita as interrupcoes
            }
        }
     
        if(Traits<Thread>::active_scheduler)
            CPU::int_enable();
        
    }
};

__END_SYS

#endif
