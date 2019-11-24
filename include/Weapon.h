#ifndef WEAPON_HEADER_DEFINED
#define WEAPON_HEADER_DEFINED

#include "Inventory.h"

namespace game {

  const irr::f32 MINIMUM_DISTANCE_BETWEEN_BULLET_DECALS = 0.0070f;

  class CWeapon : public CInventoryBaseObject
  {
  public:

    CWeapon()
    {
      timeLastShot = 0.f;
      timeReload = 0.f;
      bIsReloading = false;
      ammoClips.set_used(0);
      muzzle = (irr::scene::IMeshSceneNode*) NULL;
    }

    ~CWeapon()
    {
      for(irr::u16 i=0; i<ammoClips.size(); ++i)
        delete ammoClips[i];

      ammoClips.clear();
    }

    void setParams(SWeaponCreationParameters params)
    {
      type = params.type;
      node = params.node;
      w_class = params.w_class;

      init();
    }

    E_WEAPON_TYPE getType() { return type; }

    E_WEAPON_CLASS getClass() { return w_class; }

    irr::scene::ISceneNode *getNode() { return node; }

    virtual void init();

    virtual bool fire() { }

    virtual bool reload();

    virtual void checkForHits();

    virtual void createMuzzle();

    void refill();

    bool isReloading() { return bIsReloading; }

    bool decreaseReloadTimer(irr::f32 time)
    {
      timeReload -= time;
      if(timeReload <= 0) {
        bIsReloading = false;
        timeReload = 0.f;
        return true;
      }
      return false;
    }

    irr::u32 getClipCount() {
      irr::u32 count = 0;

      for(irr::u16 i=0; i<ammoClips.size(); ++i)
        if(ammoClips[i]->empty == false)
          count ++;

      return count;
    }

    irr::u32 getAverageAmmoInClips();

    SAmmoClip *getActiveClip()
    {
      if(ammoClips.size() == 0 || selectedClipIndex == -1)
        return NULL;

      return ammoClips[selectedClipIndex];
    }

    SAmmoClip *getClip(irr::u16 index)
    {
      if(index >= ammoClips.size())
        return NULL;

      return ammoClips[index];
    }

    void refreshActiveClip()
    {
      if(ammoClips[selectedClipIndex]->ammo == 0)
        ammoClips[selectedClipIndex]->empty = true;
        //ammoClips.erase(selectedClipIndex);

      selectedClipIndex = nextSelectedClipIndex;
    }

  protected:

    CGame * Game;

    irr::core::array<SAmmoClip *> ammoClips;

    irr::s16 selectedClipIndex, nextSelectedClipIndex;

    irr::scene::IMeshSceneNode* muzzle;

    //! What kind of weapon is it?
    E_WEAPON_TYPE type;

    ///! Weapon class: traced (bullets), projectile-launcher
    E_WEAPON_CLASS w_class;

    //! Pointer to the graphical node of the weapon
    irr::scene::ISceneNode *node;

    irr::f32 timeLastShot, timeReload;

    bool bIsReloading;

    irr::s16 findHighestAmmoClip();
  };

}

#endif
