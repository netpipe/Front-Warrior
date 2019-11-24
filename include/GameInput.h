#ifndef GAME_INPUT_HEADER_DEFINED
#define GAME_INPUT_HEADER_DEFINED

#include "Input.h"
#include "GameClasses.h"

namespace game {

  class CGameInput : public engine::CInput
  {
    public:
      CGameInput(CGame *);

      virtual bool OnEvent(const irr::SEvent& event);

      void drawCursor();

      void setCursorTexture(const irr::c8 *texturename);

      void setCursorPosition(irr::core::position2d<irr::s32> pos);

      inline void setCursorPosition(irr::core::dimension2d<irr::u16> pos) {
          setCursorPosition(irr::core::position2di(pos.Width, pos.Height));
      }

      bool isCursorInRect(irr::core::rect<irr::s32> rectangle);

    private:

      CGame * Game;
  };

}

#endif
