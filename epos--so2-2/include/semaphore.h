// EPOS-- Semaphore Abstraction Declarations

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#ifndef __semaphore_h
#define __semaphore_h

#include <common/synchronizer.h>
#
__BEGIN_SYS

class Semaphore: public Synchronizer_Common
{
private:
    typedef Traits<Thread> Traits_Thread;
    typedef Traits<Semaphore> Traits;
    static const Type_Id TYPE = Type<Semaphore>::TYPE;

public:
    Semaphore(int v = 1) : _value(v) {
	db<Semaphore>(TRC) << "Semaphore(value= " << _value << ")\n";
    }
    ~Semaphore() {
	db<Semaphore>(TRC) << "~Semaphore()\n";
        wakeup_all();
    }

    void p() { 
	db<Semaphore>(TRC) << "Semaphore::p(value=" << _value << ")\n";
	if(Traits_Thread::active_scheduler)
	    CPU::int_disable();
	        
	dec(_value);
	if(_value < 0){
            db<Semaphore>(TRC) << "SLEEP p(value=" << _value << ")\n";
	    sleep();
        }
	
	if(Traits_Thread::active_scheduler)
	    CPU::int_enable();
    }
    void v() {
	db<Semaphore>(TRC) << "Semaphore::v(value=" << _value << ")\n";
	if(Traits_Thread::active_scheduler)
	    CPU::int_disable();

	if(inc(_value) < 1)
	    wakeup();
	
	if(Traits_Thread::active_scheduler)
	    CPU::int_enable();
    }

    static int init(System_Info *si);

private:
    volatile int _value;
};

__END_SYS

#endif
