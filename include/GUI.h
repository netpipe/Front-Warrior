#ifndef GUI_HEADER_DEFINED
#define GUI_HEADER_DEFINED

#include "GameClasses.h"
#include "GameDefines.h"

namespace game {

  class CGUI
  {
  public:

    // Allow CMenu class to access protected memebers
    friend class CMenu;
    friend class CTerminal;

    CGUI(CGame*, irr::core::dimension2d<irr::u16>);

    ~CGUI();

    void enableLoadingScreen(E_LOADING_SCREEN, irr::u16 state = 0);

    void disableLoadingScreen();

    void clear(bool close=false);

    //! Draw UI for player (crosshair, healthbar, ammo, etc.)
    void drawUI();

    void update();

    void init();

    void loadUITextures();

    void loadFonts();

    void checkCrosshairObject();

    void fade(E_GUI_FADE_ID id, irr::u32 time=320, irr::video::SColor color=irr::video::SColor(255, 0,0,0));

    irr::gui::IGUIFont *getFont(irr::u32 font_id)
    {
      if(font_id >= fonts.size()) return NULL;

      return fonts[font_id];
    }

    CTerminal *getTerminal() { return m_Terminal; }

    CMenu *getMenu() { return m_Menu; }

    CGame * getGame() { return Game; }

    SCrosshairObjectData *getCrosshairObject() { return &crosshairObject; }

#ifdef PHYSICS_IRR_ODE
    irr::ode::CIrrOdeGeomRay *getCrosshairRay() { return crosshairObjectCheckRay; }
#endif

  protected:

    irr::video::IVideoDriver * m_VideoDriver;

    irr::scene::ISceneCollisionManager * m_SceneCollisionManager;

    CGame * Game;

    CMenu *m_Menu;

    CTerminal *m_Terminal;

    irr::video::ITexture *textures[TEX_COUNT];

    irr::core::array<irr::gui::IGUIFont*> fonts;

    irr::core::dimension2d<irr::u16> screenSize;

    irr::gui::IGUIInOutFader* screenFader;

    E_GUI_FADE_ID fadeID;

    SCrosshairObjectData crosshairObject;
  };

}

#endif
