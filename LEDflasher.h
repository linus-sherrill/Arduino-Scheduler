/*********************************************************************

  Copyright (c) 2009 Linus Sherrill.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

***********************************************************************/

#if !defined (LEDflasher_H_)
#define LEDflasher_H_

#include <Scheduler.h>

// ------------------------------------------------------------------
/** LED Flasher chore.
 *
 * This class is a LED flasher chore. The half-cycle time is
 * specified when an object is created.
 */

class LEDflasher
  : public SchedulerChore
{
public:
  LEDflasher(int pin)
    : m_pin(pin),
      m_state(0)
  { 
    pinMode (m_pin, OUTPUT);
  }
  
  
private:
  virtual void Run()
  {
    m_state ^= 1; // toggle state  
    digitalWrite (m_pin, m_state);
  }
  
  
  int m_pin : 4;
  int m_state : 1
};

#endif

// Local Variables:
// mode: c++
// fill-column: 64
// end:

