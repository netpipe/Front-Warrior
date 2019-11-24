#ifndef TIMER_HEADER_DEFINED
#define TIMER_HEADER_DEFINED

#include <irrlicht.h>

#include "Engine.h"

namespace engine {

  const irr::u16 MAX_TIMER_COUNT = 8;

  struct STimer
  {
    irr::u32 hours;
    irr::u32 minutes;
    irr::u32 seconds;

    irr::u32 nextupdate;

    bool paused;
    bool decrease;

    void reset()
    {
      paused = true;
      decrease = true;
      hours = 0;
      minutes = 0;
      seconds = 0;
      nextupdate = 0;
    }
  };

  class CTimer
  {
  public:

    CTimer(CCore * core);

    ~CTimer();

    void resetAll() {
      for(irr::u16 i=0; i < MAX_TIMER_COUNT; ++i)
        timers[i].reset();
    }

    void setTimer(irr::u16 timerID, irr::u32 hours, irr::u32 minutes, irr::u32 seconds) {
      timers[timerID].hours = hours;
      timers[timerID].minutes = minutes;
      timers[timerID].seconds = seconds;
      timers[timerID].nextupdate = Core->time.total + 1000;
    }

    void pauseTimer(irr::u16 timerID, bool paused) {
      timers[timerID].paused = paused;
    }

    bool isTimerPaused(irr::u16 timerID) { return timers[timerID].paused; }

    void getTimerValues(irr::u16 timerID, irr::u32 &hours, irr::u32 &minutes, irr::u32 &seconds);

    void update();

  private:

    CCore * Core;

    irr::core::array<STimer> timers;
  };

}

#endif
