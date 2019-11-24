#ifndef BOT_HEADER_DEFINED
#define BOT_HEADER_DEFINED

#include "Engine.h"
#include "BaseCharacter.h"

namespace game {

  class CBot : public engine::CBaseCharacter
  {
  public:

    CBot(engine::SCharacterCreationParameters params);

    virtual void update();

    virtual void fire();

    virtual void refill();

  private:


  };

}

#endif
