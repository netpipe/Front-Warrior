#ifndef MAIN_MENU_HEADER_DEFINED
#define MAIN_MENU_HEADER_DEFINED

#include "GameDefines.h"
#include "SoundManager.h"

namespace game {

  class CMenu
  {
  public:

    CMenu(CGUI *, CGame *);

    ~CMenu()
    {
      delete data;
    }

    void update();

    void refresh();

    void loadAssets();

    void unloadAssets();

    void changeTo(EMENU_IDENTIFIERS NewTab, bool update=true);

    irr::u32 getMenuLevel(EMENU_IDENTIFIERS menu) { return tabLevel[menu]; }

    irr::u32 getCurrentMenuLevel() { return tabLevel[currentlySelectedMenuID]; }

    void setMenuLevel(EMENU_IDENTIFIERS menu, irr::u32 level);

    EMENU_IDENTIFIERS getSelectedMenuID() { return currentlySelectedMenuID; }

    void playMusic(bool play, irr::core::stringc musicfile="");

    void setType(E_MAIN_MENU_TYPE type) { m_Type = type; }

    E_MAIN_MENU_TYPE getType() { return m_Type; }

    void setActive(bool act);

    bool isActive() { return bActive; }

    void skirmish_selectTeam(E_TEAMS team);

    void skirmish_selectLevel(irr::s16 next);

    SMainMenuData * getData() { return data; }

    irr::core::array<irr::core::stringc> missionLevels;
    irr::core::array<irr::core::stringc> skirmishLevels;

  private:

    E_MAIN_MENU_TYPE m_Type;

    EMENU_IDENTIFIERS currentlySelectedMenuID;

    irr::u32 tabLevel[EMI_COUNT];

    SMainMenuData *data;

#ifdef SOUND_CAUDIO
    cAudio::IAudioSource* backgroundMusic;
#endif

#ifdef SOUND_IRRKLANG
    irrklang::ISound * backgroundMusic;
#endif

    CGUI * GUI;

    CGame * Game;

    bool bActive;
  };

}

#endif
