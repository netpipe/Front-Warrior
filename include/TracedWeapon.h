#ifndef TRACED_WEAPON_HEADER_DEFINED
#define TRACED_WEAPON_HEADER_DEFINED

#include "Weapon.h"

namespace game {

  class CTracedWeapon : public CWeapon
  {
  public:

    CTracedWeapon(CGame* game)
    {
      Game = game;
    }

    bool fire();

  private:

  };

}

#endif
