#ifndef PLAYER_HEADER_DEFINED
#define PLAYER_HEADER_DEFINED

#include "BaseCharacter.h"
#include "Inventory.h"

namespace game {

  class CPlayer : public engine::CBaseCharacter
  {
  public:

    friend class CCharacterManager;

    CPlayer(CGame * game, engine::SCharacterCreationParameters params);

    ~CPlayer()
    {
      delete (game::CInventory*)inventory;
    }

    virtual void update();

    virtual void fire();

    virtual void refill();

  private:

    CGame * Game;

  };
}

#endif
