// EPOS-- Mutex Abstraction Declarations

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#ifndef __mutex_h
#define __mutex_h

#include <common/synchronizer.h>

__BEGIN_SYS

class Mutex: public Synchronizer_Common
{
private:
    typedef Traits<Thread> Traits_Thread;
    typedef Traits<Mutex> Traits;
    static const Type_Id TYPE = Type<Mutex>::TYPE;

public:
    Mutex() : _locked(false) { db<Mutex>(TRC) << "Mutex()\n"; }
    ~Mutex() { 
        db<Mutex>(TRC) << "~Mutex()\n"; 
	wakeup_all();
    }

    void lock() { 
	db<Mutex>(TRC) << "Mutex::lock()\n";
	if(Traits_Thread::active_scheduler)
		CPU::int_disable();

	if(tsl(_locked))
	    sleep();

	if(Traits_Thread::active_scheduler)
	    CPU::int_enable();
    }
    void unlock() { 
	db<Mutex>(TRC) << "Mutex::unlock()\n";
        if(Traits_Thread::active_scheduler)
	    CPU::int_disable();

	_locked = false;
	wakeup();

	if(Traits_Thread::active_scheduler)
	    CPU::int_enable(); 
    }

    static int init(System_Info *si);

private:
    volatile bool _locked;
};

__END_SYS

#endif
