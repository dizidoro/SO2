// EPOS-- Thread Abstraction Implementation

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#include <thread.h>
#include <mmu.h>

__BEGIN_SYS

// Class attributes
Thread * volatile Thread::_running;
Thread * Thread::_idle;
Thread::Queue Thread::_ready;
Thread::Queue Thread::_suspended;

// Methods
Thread::Queue::Element * Thread::link_sync() { return &_link_sync; }
void Thread::sync_queue(Thread::Queue * queue){ _sync_queue = queue; }

int Thread::join() {
    db<Thread>(TRC) << "Thread::join(this=" << this
		    << ",state=" << _state << ", _who_joined=" <<_who_joined << ")\n";
    
    if(Traits::active_scheduler)
	CPU::int_disable();
   
    
    if(_who_joined == 0){//se _who_joined ainda nao foi setado
    	_who_joined = _running;
    }
    else{
	if(Traits::active_scheduler)
	    CPU::int_enable();
	return -1; //indicar erro! Modificar valor!
    }
	

    if(_state != FINISHING)
	_running->suspend();
    db<Thread>(TRC) << "Depois do suspend this=" << this
		    << ",state=" << _state << ")\n";
    
    if(_who_joined->_state == LOST)
	return -2;
    
    if(Traits::active_scheduler)
	CPU::int_enable();

     return *((int *)_stack);
}

void Thread::pass() {
    db<Thread>(TRC) << "Thread::pass(this=" << this << ")\n";

    if(Traits::active_scheduler)
	CPU::int_disable();

    Thread * old;
    if(_running->_state == IDLE)
        old = _idle;
    else {
        old = _running;
        old->_state = READY;
        _ready.insert(&old->_link);
        _state = RUNNING;
    }

    _ready.remove(this); //inocuo se this nao estiver na fila
    _running = this;

//     old->_context->save(); // can be used to force an update
    db<Thread>(INF) << "old={" << old << "," 
		    << *old->_context << "}\n";
    db<Thread>(INF) << "new={" << _running << "," 
		    << *_running->_context << "}\n";
	
    CPU::switch_context(&old->_context, _context);

    if(Traits::active_scheduler)
	CPU::int_enable();
}

void  Thread::suspend() {
    db<Thread>(TRC) << "Thread::suspend(this=" << this << ")\n";

    if(Traits::active_scheduler)
	CPU::int_disable();

    if(_state != IDLE) {
        if(_running != this)
	    _ready.remove(this);
        _running->_state = SUSPENDED;
        _suspended.insert(&_running->_link);
    }

    if(!_ready.empty()) {
	_running = _ready.remove()->object();
	_running->_state = RUNNING;

// 	_context->save(); // can be used to force an update
	db<Thread>(INF) << "old={" << this << "," 
			<< *_context << "}\n";
	db<Thread>(INF) << "new={" << _running << "," 
			<< *_running->_context << "}\n";

	CPU::switch_context(&_context, _running->_context);
    } else if (_running->_state != IDLE){
        _running = 0;   
	CPU::switch_context(&_context, _idle->_context);
    }

    if(Traits::active_scheduler)
	CPU::int_enable();
}	    

void  Thread::resume() {
    db<Thread>(TRC) << "Thread::resume(this=" << this << ")\n";

    if(Traits::active_scheduler)
	CPU::int_disable();

    if(_state != IDLE) {
        _suspended.remove(this);
        _state = READY;
        _ready.insert(&_link);
    }

    if(Traits::active_scheduler)
	CPU::int_enable();
}

void Thread::yield() {
    db<Thread>(TRC) << _running << " Thread::yield()\n";
    if(Traits::active_scheduler)
	CPU::int_disable();

    
    if(!_ready.empty()) {
        Thread * old;
        if(_running->_state == IDLE)
            old = _idle;
        else {
	    old = _running;
	    old->_state = READY;
	    _ready.insert(&old->_link);
        }

	_running = _ready.remove()->object();
	_running->_state = RUNNING;

        //if(old != _running) {

// 	    old->_context->save(); // can be used to force an update
	    db<Thread>(INF) << "old={" << old << "," 
			<< *old->_context << "}\n";
	    db<Thread>(INF) << "new={" << _running << "," 
			<< *_running->_context << "}\n";

            CPU::switch_context(&old->_context, _running->_context);
        //}
    }

    if(Traits::active_scheduler)
	CPU::int_enable();
}

void Thread::exit(int status)
{
    db<Thread>(TRC) << "Thread::exit(status=" << status <<", running state=" << _running->_state << ", running=" << _running << " )\n";
  
    if(Traits::active_scheduler)
	CPU::int_disable();
    	

//    if(_ready.empty() && !_suspended.empty())
//	idle(); // implicitly re-enables interrupts

    if(Traits::active_scheduler)
	CPU::int_disable();
     
    if((_running->_who_joined != 0) && _suspended.search(_running->_who_joined)){
        _running->_who_joined->resume();
        _running->_who_joined = 0;
    }
    
    if(Traits::active_scheduler)
	CPU::int_disable();	

    if(!_ready.empty()) {
	Thread * old = _running;
	old->_state = FINISHING;

	*((int *)(void *)old->_stack) = status;

	_running = _ready.remove()->object();
	_running->_state = RUNNING;

// 	old->_context->save(); // can be used to force an update
	db<Thread>(INF) << "old={" << old << "," 
			<< *old->_context << "}\n";
	db<Thread>(INF) << "new={" << _running << "," 
			<< *_running->_context << "}\n";

	CPU::switch_context(&old->_context, _running->_context);
    } else if(!_suspended.empty()){
        Thread * old = _running;
        CPU::switch_context(&old->_context, _idle->_context);
    } else {
        db<Thread>(WRN) << "The last thread in the system has exited!\n";
        db<Thread>(WRN) << "Halting the CPU ...\n";

        CPU::int_disable();
        CPU::halt(); // this must be turned into a conf-feature (reboot, halt)
    }

    if(Traits::active_scheduler)
       CPU::int_enable();
}

void Thread::idle() {
    while (true) {
        db<Thread>(TRC) << "Thread::idle()\n";

        db<Thread>(WRN) << "There are no runnable threads at the moment!\n";
        db<Thread>(WRN) << "Halting the CPU ...\n";

        CPU::int_enable();
        CPU::halt();
    }
}

__END_SYS
