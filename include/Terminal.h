#ifndef TERMINAL_HEADER_DEFINED
#define TERMINAL_HEADER_DEFINED

#include "GameDefines.h"

namespace game {

  enum E_TERMINAL_SCREEN_TYPE
  {
    ETST_MAIN = 0
  };

  class CTerminal
  {
  public:

    friend class CGUI;

    CTerminal(CGUI * gui, CGame * game);

    ~CTerminal();

    void update();

    void setActive(bool);

    bool isActive() { return b_Active; }

    bool isDrawn() { return b_Draw; }

    void reset();

    void setScreen(E_TERMINAL_SCREEN_TYPE type);

  protected:

    CGame *Game;

    CGUI * GUI;

    bool b_Active;

    bool b_Draw;
  };

}

#endif
