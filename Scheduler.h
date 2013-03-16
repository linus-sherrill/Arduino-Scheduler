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

#if !defined (Scheduler_H_)
#define Scheduler_H_

#include <inttypes.h>

//
// forward declarations
//
class Scheduler;

// ----------------------------------------------------------------------------
/** Scheduled chore.
*
* This class is the abstract base class for all chores.  A chore
* is an object that is activated after a specific delay. Chores
* are automatically rescheduled by default so they are
* recurrring activities.
*
* This could be enhanced to allow for one time chores and to
* allow the chore to cancel itself.
*/

class SchedulerChore
{
public:

  SchedulerChore ();
  SchedulerChore (uint32_t inter);
  virtual ~SchedulerChore();

  bool operator== (const SchedulerChore & rhs) const
  { return (m_targetTime == rhs.m_targetTime); }

  // compares execution times
  bool operator< (const SchedulerChore & rhs) const
  { return (m_targetTime < rhs.m_targetTime); }

  /// Return scheduling interval for this chore.
  uint32_t Interval() const { return m_interval; }

  /// Set reschedule interval for this chore.
  void Interval (uint32_t inter) { m_interval = inter & 0x0fffffff; }

  int AbortChore();


protected:
  void InsertBefore (SchedulerChore * c);
  void Remove();


private:
  friend class Scheduler;
  friend class TimerWrap;

  /** Procedure to run periodically.  This method is the do-it
   * function for this chore.  All derived classes must supply
   * the implementation.
   */
  virtual void Run () = 0;

  Scheduler * m_parent;

  /// Next scheduled run time, seconds.
  uint32_t m_targetTime;

  /// Repeat time in milli-seconds
  uint32_t  m_interval;

  // List linking fields
  SchedulerChore * m_next;
  SchedulerChore * m_prev;

  // NON_COPYABLE
  SchedulerChore (const SchedulerChore &);
  const SchedulerChore & operator= (const SchedulerChore &);
}; 



class TimerWrap : public SchedulerChore
{
public:
  TimerWrap(); 
  virtual ~TimerWrap() { }
  
private:
  virtual void Run();
};


// ----------------------------------------------------------------------------
/** Time based chore scheduler.
*
* This class represents a procedure that schedules periodic
* chores.  Chores are scheduled with some recurring interval
* specified in milli-seconds.  A chore will be activated at some
* point after its expiration time, but the best effort is make
* to be as prompt as possible.
*
* The expiration time is calculated by adding the recurrence
* interval to the current time when the chore is scheduled.  The
* resolution of the scheduler time is in milli-seconds.
* 
* The scheduler is designed to be called in the loop() function
* to dispatch chores.
*
* The clock is a 32 bit counter which gives about 49.71 days before
* it will wrap. Currently there is no handling for this timer
* wrapping, so beware.
*
* Example:
\code

class my_chore : public SchedulerChore
{
public:
    my_chore() { }
    virtual ~my_chore() { }

private:
    virtual void Run () 
    {
        // do something

    }
};

// -----------------------------------------------

// define the scheduler
Scheduler  the_scheduler;

// . . . . 
void setup()
{
  // make chore with 100 msec cycle
  my_chore xs* the_chore (new my_chore (100));

  // start the chore running at its defined rate.
  the_scheduler.Schedule (the_chore);

  // . . . 
}



void loop()
{

  // See if anything needs to be dispatched
  the_scheduler.RunScheduler();

  // . . .

}

\endcode
*/

class Scheduler
  : public SchedulerChore
{
public:
  Scheduler ();
  virtual ~Scheduler();
    
  void RunScheduler ();

  int Schedule (SchedulerChore * chore);
  int AbortChore (SchedulerChore * chore);

protected:
  int Reschedule (SchedulerChore * chore);
  uint32_t GetCurrentTime() const;
  void Insert (SchedulerChore * chore);


private:
  virtual void Run () { }  // from chore

  TimerWrap m_timerWrap;

  uint32_t m_baseTime;

  // NON_COPYABLE
  Scheduler (const Scheduler &);
  const Scheduler & operator= (const Scheduler &);
}; 


#endif	// scheduler_H_


// Local Variables:
// mode: c++
// fill-column: 64
// end:
