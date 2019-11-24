#ifndef CHARACTER_MANAGER_HEADER_DEFINED
#define CHARACTER_MANAGER_HEADER_DEFINED

#include "Engine.h"
#include "GameClasses.h"
#include "GameDefines.h"

namespace game {

  class CCharacterManager
  {
  public:

    friend class CPlayer;

    CCharacterManager(CGame * game)
    {
      bots.set_used(0);
      m_Player = (CPlayer*)NULL;

      Game = game;
    }

    ~CCharacterManager()
    {
      clearAll();
    }

    void spawn(engine::CBaseCharacter*);

    CPlayer * createPlayer(engine::SCharacterCreationParameters parameters);

    CBot * createBot(engine::SCharacterCreationParameters parameters);

    //! Get the player
    CPlayer * getPlayer() { return m_Player; }

    //! Get bot from list by id
    CBot * getBotByIndex(irr::u8 id) { return bots[id]; }

    //! How many bots are there
    irr::u16 getBotCount() { return bots.size(); }

    void clearAll();

    void loadExternalCharacterClassData();

  private:

    CGame * Game;

    CPlayer * m_Player;

    irr::core::array<CBot*> bots;

    irr::core::array<SCharacterClassParameters *> cClassParameters;
  };

}

#endif

