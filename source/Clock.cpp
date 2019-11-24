#include "Core.h"
#include "Clock.h"

using namespace engine;

CTimer::CTimer(CCore * core) : Core(core)
{
  timers.set_used(MAX_TIMER_COUNT);
}

CTimer::~CTimer()
{

}

void CTimer::getTimerValues(irr::u16 timerID, irr::u32 &hours, irr::u32 &minutes, irr::u32 &seconds)
{
  hours = timers[timerID].hours;
  minutes = timers[timerID].minutes;
  seconds = timers[timerID].seconds;
}

void CTimer::update()
{
  irr::u32 currentTime = Core->time.total;

  for(irr::u16 timerIdx = 0; timerIdx < MAX_TIMER_COUNT; ++timerIdx)
  {
    if(currentTime >= timers[timerIdx].nextupdate)
    {
      if(timers[timerIdx].seconds == 0) {
        if(timers[timerIdx].minutes == 0) {
          if(timers[timerIdx].hours > 0) {
            timers[timerIdx].hours -= 1;
            timers[timerIdx].minutes = 60;
          }
        }
        timers[timerIdx].seconds = 60;
        timers[timerIdx].minutes -= 1;
      }

      timers[timerIdx].seconds -= 1;

      timers[timerIdx].nextupdate = currentTime + 1000;
    }
  }
}
