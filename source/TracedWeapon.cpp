#include "Game.h"
#include "Core.h"
#include "Renderer.h"
#include "TracedWeapon.h"
#include "SoundManager.h"
#include "Maths.h"

using namespace game;
using namespace engine;

bool CTracedWeapon::fire()
{
  irr::f32 currentTime = Game->getCore()->getRenderer()->getTimer()->getTime();
  irr::f32 timeDiff = currentTime - timeLastShot;

  if(timeDiff >= Game->weaponsParameters[type]->fireRate)
  {
    timeLastShot = currentTime;

    if(selectedClipIndex != -1)
    {
      if(bIsReloading == false && ammoClips[selectedClipIndex]->ammo > 0)
      {
        SWeaponParameters *weap = Game->weaponsParameters[type];

        //
        // Play weapon firing sound

        Game->getCore()->getSound()->playSound2D(
          weap->fireSounds[Game->getCore()->getMath()->getRandomInt(0, weap->fireSounds.size()-2)].c_str(),
          false, // no looping
          0.62f,  // vol
          0.0f,  // pan (center)
          false); // create a new sound resource

        //
        // Create muzzle effect

        createMuzzle();

        //
        // Check for hits

        checkForHits();

        //
        // Deal with ammo

        ammoClips[selectedClipIndex]->ammo -= 1;

        if(ammoClips[selectedClipIndex]->ammo == 0 /* TODO : AND game-settings: auto-reload ON*/) {
          if(!reload())
            return false;
        }

        return true;
      }
    }
    else {
      Game->getCore()->getSound()->playSound2D(
        "data/sounds/weapon/dry_fire.wav",
        false,
        0.55f);
    }
  }

  return false;
}
