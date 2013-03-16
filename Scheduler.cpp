/*********************************************************************
  Scheduler.h - Arduino scheduler.
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

#include "Scheduler.h"
#include "WProgram.h"


// -------------------------------------------------------
/** Constructor.
 *
 * 
 * 
 */

Scheduler::
Scheduler() 
{
  m_next = this;
  m_prev = this;  

  m_baseTime = millis();

  // schedule our timer wrap manager
  Schedule (& m_timerWrap);

}


// ----------------------------------------------------------------------------
/** Destructor.
 *
 * This method stops and cleans up a scheduler. 
 *
 */

Scheduler::
~Scheduler()
{

}


// -------------------------------------------------------
/** Run the scheduler.
 *
 * This is the main method that runs the scheduler loop.  It
 * dispatches all chores that have expired. Chores are
 * automatically rescheduled.
 * 
 */

void Scheduler::
RunScheduler ()
{

  // Check to see if there is something to run.
  // delta in milli-secs, signed
  while (1)
    {
      int32_t delta (this->m_next->m_targetTime - GetCurrentTime());
      if (delta <= 1 )
        {
          // time to dispatch the chore
          SchedulerChore * chore = this->m_next;
          chore->Remove();
          
          // Test for orphaned chore - do not run orphaned chore
          if (chore->m_parent != 0)
            {
              // Activate the chore.
              chore->Run();

              Reschedule(chore); // immediately reschedule
            }
        }
      else // no chores to dispatch
        {
          break;
        }
    } // end while
}


// ----------------------------------------------------------------------------
/** Schedule a chore.
 *
 * This method schedules a \e new chore. The chore's execution
 * time is set to its recurrence interval plus the current time,
 * so it will execute the specified number of seconds from now.
 * 
 * @param[in] chore - chore to schedule
 *
 * @retval 0 - chore scheduled
 * @retval -1 - chore not scheduled
 */

int Scheduler::
Schedule (SchedulerChore * chore)
{
  // Make sure chore is not active
  if (chore->m_parent != 0)
    {
      return -1;
    }

  // calculate execution time
  chore->m_targetTime = GetCurrentTime() + chore->m_interval;

  Insert (chore);

  return (0);
}


// ----------------------------------------------------------------------------
/** Reschedule a chore.
 *
 * This method reschedules a chore that has already run. The
 * chore's execution time is calculated by adding the recurrence
 * interval to the chore's last execution time.  In this way,
 * the chore's execution time does not slip due to system
 * delays.
 * 
 * @param[in] chore - chore to reschedule
 *
 * @retval 0 - chore reshceduled
 * @retval -1 - chore not attached to scheduler
 */

int Scheduler::
Reschedule (SchedulerChore * chore)
{

  // Make sure chore is  active
  if (chore->m_parent == 0)
    {
      return -1;
    }

  // calculate execution time
  chore->m_targetTime += chore->m_interval;

  Insert (chore);
  return (0);
}


// ------------------------------------------------------------------
/** Insert chore into schedule list.
 *
 *
 */

void Scheduler::
Insert (SchedulerChore * chore)
{
  chore->m_parent = this; // assign ownership
  SchedulerChore * ptr;

  // Insert new chore into the list
  for (ptr = m_next; ptr != this; ptr = ptr->m_next)
    {
      if (*chore < *ptr)
        {
          chore->InsertBefore (ptr);
        }
    } // end for

  if (ptr == this)
    {
      // insert at end
      chore->InsertBefore (this);
    }
}


// ----------------------------------------------------------------------------
/** Abort a scheduled chore.
 *
 * This method forcably removes the specified chore from the
 * scheduling list.
 *
 * @param[in] chore - the chore to abort.
 *
 * @retval 0 - chore removed
 * @retval -1 - chore not in list
 */

int Scheduler::
AbortChore (SchedulerChore * chore)
{
  
  if (m_parent != 0)
    {
      // remove from list
      chore->Remove();
    }
  
  // always unlink from scheduler
  chore->m_parent = 0;      
  
  return (0);
}


// ----------------------------------------------------------------------------
/** Get current time in milli-seconds.
 *
 * This method returns the current time in milli-seconds from the
 * start of the epoch.
 *
 */

uint32_t Scheduler::
GetCurrentTime() const
{
  return (millis() - m_baseTime);
}


// ==================================================================
// TimerWrap methods
//

TimerWrap::
TimerWrap()
{
  m_targetTime = 0x40000000;
}

void TimerWrap::
Run()
{
  // adjust the base time
  m_targetTime += 0x40000000;

  // adjust all chore time values
  for (SchedulerChore * ptr = m_next; ptr != this; ptr = ptr->m_next)
    {
      // subtract 0x40000000 from target time
      ptr->m_targetTime &= 0x3fffffff;
    } // end for
}


// ============================================================================
// Scheduler Chore methods
//

// ----------------------------------------------------------------------------
/** Constructor. 
 *
 * A new chore object is created and initialized.  Since no
 * scheduling interval is specified, a zero is used and the
 * interval must be set before the chore is scheduled.
 */

SchedulerChore::
SchedulerChore()
  : m_parent(0),
    m_targetTime (0),
    m_interval(0),
    m_next(0), m_prev(0)
{ 

}


// ----------------------------------------------------------------------------
/** Constructor. 
 *
 * An object with the specified interval is created.  If no
 * scheduling interval is specified, a zero is used and the
 * interval must be set before the chore is scheduled.
 *
 * @param[in] inter - scheduling interval in milli-seconds
 */

SchedulerChore::
SchedulerChore(uint32_t inter)
  : m_parent(0),
    m_targetTime (0),
    m_interval(inter),
    m_next(0), m_prev(0)
{
  // limit interval
  m_interval &= 0x0fffffff;
}


// ----------------------------------------------------------------------------
/** Destructor.
 *
 * This method cleans up a chore before the storage is released.
 * If we are still connected to a parent scheduler, then we
 * abort this chore to make sure we will never be scheduled
 * again. If the chore is running at this time, there is not
 * much we can do about synchronizing the termination because we
 * would have to make some wild assumptions about the run time
 * of this chore. Therefore, synchronizing object destruction
 * with chore termination is the responsibility of the derived
 * class.
 */

SchedulerChore::
~SchedulerChore()
{
  m_parent = 0; // orphan this chore
}


// ----------------------------------------------------------------------------
/** Abort this chore. 
 *
 * This method aborts this chore by removing it from the
 * schedulers list of chores, if it is in there.
 */

int SchedulerChore::
AbortChore() 
{
  if (m_parent != 0)
    {
      return m_parent->AbortChore (this);
    }
  else
    {
      return (-1);
    }
}


// ------------------------------------------------------------------
/** Insert before a chore.
 *
 * This method inserts this chore into tthe linked list of
 * chores before the specified chore.
 */

void SchedulerChore::
InsertBefore (SchedulerChore * c)
{
  c->m_next = this;
  c->m_prev = this->m_prev;

  this->m_prev->m_next = c;
  this->m_prev = c;  
}


// ------------------------------------------------------------------
/** Remove this chore from list.
 *
 *
 */

void SchedulerChore::
Remove()
{
      m_prev->m_next = this->m_next;
      m_next->m_prev = this->m_prev;

      m_next = 0;
      m_prev = 0;
}

// Local Variables:
// mode: c++
// fill-column: 64
// end:
