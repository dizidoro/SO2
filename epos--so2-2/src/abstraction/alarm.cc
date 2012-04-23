// EPOS-- Alarm Abstraction Implementation

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#include <alarm.h>
#include <display.h>

__BEGIN_SYS

// Class attributes
Timer Alarm::_timer;
volatile Alarm::Tick Alarm::_elapsed;
Alarm::Handler Alarm::_master;
Alarm::Tick Alarm::_master_ticks;
Alarm::Queue Alarm::_requests;
Alarm::Semaphore_Queue Alarm::_delays;

// Methods
Alarm::Alarm(const Microseconds & time, const Handler & handler, int times)
    : _ticks((time + period() / 2) / period()),
      _times(times), _link(this), _handler(handler),
      _handler_thread(&(Alarm::handler_wrapper), this, Thread::SUSPENDED)
{
    db<Alarm>(TRC) << "Alarm(t=" << time << ",h=" << (void *)handler
		   << ",x=" << times << ")\n";
    
    if(_ticks)
	_requests.insert(&_link);
    else
	_handler_thread.resume();
}

Alarm::~Alarm() {
    db<Alarm>(TRC) << "~Alarm()\n";
    _requests.remove(this);
}

void Alarm::master(const Microseconds & time, const Handler & handler)
{
    db<Alarm>(TRC) << "master(t=" << time << ",h="
		   << (void *)handler << ")\n";

    _master = handler;
    _master_ticks = (time + period() / 2) / period();
}

void Alarm::delay(const Microseconds & time)
{
    db<Alarm>(TRC) << "delay(t=" << time << ")\n";
   
    
    Tick t = _elapsed + time / period();

    if(t) {
        Semaphore semaphore(0);
        Semaphore_Queue::Element link(&semaphore, t);
        _delays.insert(&link);
        semaphore.p();
    }
}

void Alarm::timer_handler(void)
{
    
    static Tick next_request;
    static Tick next_delay;

    static Thread * handler;
    static Semaphore * semaphore;

    _elapsed++;
    
    if(Traits_Alarm::visible) {
	Display display;
	int lin, col;
	display.position(&lin, &col);
	display.position(0, 79);
	display.putc(_elapsed);
	display.position(lin, col);
    }

    if(_master_ticks) {
	if(!(_elapsed % _master_ticks))
	    _master();
    }

    if(next_request)
	next_request--;
    else {
	if(handler)
	    handler->resume();
	if(_requests.empty())
	    handler = 0;
	else {
            Queue::Element * e = _requests.remove();
	    Alarm * alarm = e->object();
	    next_request = alarm->_ticks;
	    handler = &(alarm->_handler_thread);
	    if(alarm->_times != -1)
		alarm->_times--;
	    if(alarm->_times) {
			e->rank(alarm->_ticks);
			_requests.insert(e);
	    }
            
	}
    }

    if(next_delay)
        next_delay--;
    else {
        if(semaphore)
            semaphore->v();
        if(_delays.empty())
            semaphore = 0;
        else {
            Semaphore_Queue::Element * se = _delays.remove();
            semaphore = se->object();
            next_delay = se->rank();
        }
    }
}

int Alarm::handler_wrapper(Alarm * alarm){
    while(true) {
        db<Alarm>(TRC) << "executing handler(t=" << alarm->_times << ")\n";
        alarm->_handler();

        if(!alarm->_times)
            break;

        alarm->_handler_thread.suspend();
    }

    return 0;
}

__END_SYS
