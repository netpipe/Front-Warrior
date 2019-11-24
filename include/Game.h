#ifndef GAME_HEADER_DEFINED
#define GAME_HEADER_DEFINED

#include "Core.h"

#include "GameClasses.h"
#include "GameDefines.h"

namespace game {

  class CGame
  {
  public:

    CGame(engine::CCore * core);

    ~CGame();

    void init();

    void update();

    void run();

    void start(SGameParameters);

    void close();

    void returnToMainMenu();

    CGUI *getGUI(){ return GUI; }

    CCharacterManager *getCharacters() { return characters; }

    CGameInput * getInput() { return Input; }

    SGameParameters getParameters() { return gameParameters; }

    inline engine::CCore * getCore() { return Core; }

    SInventoryItem createInventoryWeapon(
      E_WEAPON_TYPE type,
      irr::scene::ISceneNode *node);

    void setCurrentPlayerWeapon(CWeapon *w) { wPlayer = w; }

    CWeapon* getCurrentPlayerWeapon() { return wPlayer; }

    void playSurfaceHitSound(E_BODY_MATERIAL_TYPE material, irr::core::vector3df &position);

    void createSurfaceHitParticles(
      const irr::c8* particle_texture_file,
      irr::core::vector3df position,
      irr::core::vector3df normal,
      irr::f32 particle_scale,
      irr::f32 particle_velocity,
      irr::f32 fade_time,
      irr::u32 emit_min=15,
      irr::u32 emit_max=20,
      irr::u32 random_angle=40,
      irr::scene::ISceneNode *emitter_parent=(irr::scene::ISceneNode*)NULL);

    irr::core::array<SWeaponParameters *> weaponsParameters;

  protected:

    CCharacterManager *characters;

    engine::CCore * Core;

    CGUI *GUI;

    CGameInput * Input;

    SGameParameters gameParameters;

    void clearAll();

    void updateGUI();

    void checkPlayerInteractions();

    void loadInventoryObjects();

    void setPlayerInventorySelectedItem(irr::u32 i);

    void loadExternalWeaponData();

    void updatePlayerWeapon();

    CWeapon *wPlayer;

    void loadMiscSounds();

    void loadWeaponSounds();

    void loadMiscModels();

    // These meshes have to be dropped before closing the game.
    // That is done in "close" function.
    irr::core::array<irr::scene::IMesh*> preloadedMeshes;

    irr::core::array<const irr::c8*>
      surfaceHitSoundsStone,
      surfaceHitSoundsWood,
      surfaceHitSoundsMetal,
      surfaceHitSoundsCanvas,
      surfaceHitSoundsGround;
  };

}

#endif
