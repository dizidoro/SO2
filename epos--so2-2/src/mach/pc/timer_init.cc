// EPOS-- PC Timer Mediator

// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-NoDerivs License. To view a copy of this license, 
// visit http://creativecommons.org/licenses/by-nc-nd/2.0/ or send a letter to 
// Creative Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

#include <mach/pc/timer.h>

__BEGIN_SYS

int PC_Timer::init(System_Info * si)
{
    db<PC_Timer>(TRC) << "PC_Timer::init()\n";

    return 0;
}

__END_SYS
