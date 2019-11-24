#include "Game.h"
#include "Inventory.h"
#include "Weapon.h"
#include "CharacterManager.h"
#include "GUI.h"
#include "Menu.h"
#include "Terminal.h"
#include "TracedWeapon.h"
#include "GameInput.h"

#include "Renderer.h"
#include "Configuration.h"
#include "Input.h"
#include "Camera.h"
#include "ObjectManager.h"
#include "Player.h"
#include "Maths.h"
#include "Renderer.h"
#include "Clock.h"

using namespace game;
using namespace engine;

// Local variables
bool l_restoreWeaponAimAfterReloading = false;
bool l_weaponArmSoundPlayed = false;
bool l_playerRelatedUpdates;

CGame::CGame(engine::CCore * core)
{
  wPlayer = (CWeapon*)NULL;
  preloadedMeshes.set_used(0);

  GUI = (CGUI*) NULL;

  Core = core;
}

CGame::~CGame()
{
  delete Core;
}

void CGame::close()
{

  // Drop preloaded meshes
  /*for(irr::u32 i=0; i<preloadedMeshes.size(); ++i)
  {
    // Their textures also need to be dropped

    // for(irr::u32 m=0; m < preloadedMeshes[i]->getMeshBufferCount(); ++m)
    //{
    //  for(irr::u32 t=0; t < 4; ++t)
    //    if(preloadedMeshes[i]->getMeshBuffer(m)->getMaterial().getTexture(t))
    //      preloadedMeshes[i]->getMeshBuffer(m)->getMaterial().getTexture(t)->drop();
    //}

    preloadedMeshes[i]->drop();
  }*/

  preloadedMeshes.clear();

  if(GUI)
    GUI->clear(true);

  delete GUI;
  delete characters;
  delete Input;

  for(irr::u16 i=0; i< weaponsParameters.size(); ++i)
    delete weaponsParameters[i];

  weaponsParameters.clear();

  Core->getRenderer()->getDevice()->closeDevice();
  Core->getRenderer()->getDevice()->drop();
}

void CGame::init()
{
  // Init all systems
  irr::u32 initRes = Core->init();

  // All sub-systems keep a pointer of "this"
  // which is the main game class

  // Create the character manager which we use to create the player,
  // bots and spawn them later on
  characters = new CCharacterManager(this);

  // GUI system needs to know the size of the screen
  GUI = new CGUI(
    this,
    Core->getConfiguration()->getVideo()->windowSize);

  // All input is handled on game side
  Input = new CGameInput(this);

  // Let the Irrlicht renderer know of our event receiver
  // so it can receive events
  Core->getRenderer()->getDevice()->setEventReceiver(Input);

  // Show the main loading screen
  GUI->enableLoadingScreen(game::ELS_STARTUP);

  GUI->getMenu()->playMusic(true, "data/sounds/music/theme.ogg");

#ifdef ENGINE_DEVELOPMENT_MODE
  printf("Preloading stuff ... ");
#endif

  GUI->loadUITextures();
  GUI->loadFonts();
  loadMiscSounds();

#ifdef ENGINE_DEVELOPMENT_MODE
  printf("ok!\n");
#endif

  Core->getRenderer()->getDevice()->getFileSystem()->addZipFileArchive("data/levels/test.zip");
  Core->getRenderer()->getDevice()->getFileSystem()->addZipFileArchive("data/levels/industrial.zip");
  Core->getRenderer()->getDevice()->getFileSystem()->addZipFileArchive("data/levels/hill.zip");
  Core->getRenderer()->getDevice()->getFileSystem()->addZipFileArchive("data/levels/g_m1.zip");

  // Check if user wants to go to skirmish mode directly
  if(Core->commandLineParameters.hasParam("-skirmish"))
  {
    SGameParameters gameParams;
    gameParams.Type = game::EGT_SKIRMISH_ANNIHILATION;

    // default values
    gameParams.TeamID = 0;
    gameParams.Class = 0;
    gameParams.Skirmish.BotCount = 5;
    gameParams.Skirmish.TimeHours = 0;
    gameParams.Skirmish.TimeMinutes = 45;

    // Parameters can be user-specified
    if(Core->commandLineParameters.hasParam("-team"))
        gameParams.TeamID = atoi(Core->commandLineParameters.getParamValue("-team").c_str());

    if(Core->commandLineParameters.hasParam("-class"))
        gameParams.Class = atoi(Core->commandLineParameters.getParamValue("-class").c_str());

    gameParams.Level = Core->commandLineParameters.getParamValue("-level");

    start(gameParams);
  }
  // In all other cases go to the main menu
  else
  {
    Input->setCursorTexture("data/2d/cursor.png");
    Input->setCursorVisible(true);

    // Load images the menu uses
    GUI->getMenu()->loadAssets();

    GUI->getMenu()->setActive(true);
    GUI->getMenu()->setType(game::EMMT_DEFAULT);
    GUI->getMenu()->changeTo(game::EMI_MAIN_MENU);

  }


}

void CGame::checkPlayerInteractions()
{
  if(Input->isKeyPressedOnce(irr::KEY_KEY_F))
  {
    if(GUI->getCrosshairObject()->type == EOT_INTERACTABLE
    && GUI->getCrosshairObject()->interactable == true
    && GUI->getTerminal()->isActive() == false)
    {
      irr::u32 id = GUI->getCrosshairObject()->elementNumber;

      if(GUI->getCrosshairObject()->name == "Console")
        GUI->getTerminal()->setActive(true);


    }
  }

  //if(Input->isKeyPressedOnce(irr::KEY_KEY_L))
    //Core->getConfiguration()->getVideo()->grassRange ++;

}

void CGame::run()
{
  // The main cycle which updates all aspects of the game
  while(Core->isRunning() == true)
  {
    // Update Irrlicht core
    Core->getRenderer()->getDevice()->run();

    // Game is updated when the window is focused
    // TODO : check focus only when rendering,
    // so the game would still be updated
    if(Core->getRenderer()->getDevice()->isWindowFocused())
    {
      // Update characters, vehicles and other level objects
      update();

      // Updates the physics bodies and renders the scene
      Core->render();

      // Render game GUI and menus and the in-game HUD
      // We need to render GUI after the scene because of the drawing order
      updateGUI();

      checkPlayerInteractions();

      // Closes the Irrlicht renderer for this frame
      Core->finalizeUpdate();
    }
    else
    {
      // Let other processes run
      Core->getRenderer()->getDevice()->yield();
    }
  }
}


void CGame::loadMiscModels()
{
  irr::scene::ISceneManager * SceneManager = Core->getRenderer()->getSceneManager();
  irr::video::IVideoDriver * VideoDriver = Core->getRenderer()->getVideoDriver();

  printf("Loading meshes ... ");

  preloadedMeshes.push_back(SceneManager->getMesh("data/foilage/firtree/fir1.ms3d"));
  preloadedMeshes.push_back(SceneManager->getMesh("data/foilage/firtree/fir2.ms3d"));
  preloadedMeshes.push_back(SceneManager->getMesh("data/foilage/firtree/fir3.ms3d"));

  preloadedMeshes.push_back(SceneManager->getMesh("data/base1/base1.b3d"));
  preloadedMeshes.push_back(SceneManager->getMesh("data/base1/powerplant1.b3d"));
  preloadedMeshes.push_back(SceneManager->getMesh("data/base1/tower1.b3d"));

  preloadedMeshes.push_back(SceneManager->getMesh("data/assets/console/console.ms3d"));

  preloadedMeshes.push_back(SceneManager->getMesh("data/chars/male/soldier.ms3d"));

  // Load bullet marks
  preloadedMeshes.push_back(SceneManager->getMesh("data/particles/decals/bullethole1.ms3d"));
  VideoDriver->getTexture("data/particles/decals/bullethole_metal.png");
  VideoDriver->getTexture("data/particles/decals/bullethole_stone.png");
  VideoDriver->getTexture("data/particles/decals/bullethole_wood.png");
  VideoDriver->getTexture("data/particles/decals/bullethole_sandbag.png");

  preloadedMeshes.push_back(SceneManager->getMesh("data/foilage/grass1.ms3d"));
  preloadedMeshes.push_back(SceneManager->getMesh("data/foilage/grass2.ms3d"));
  preloadedMeshes.push_back(SceneManager->getMesh("data/foilage/grass3.ms3d"));
  preloadedMeshes.push_back(SceneManager->getMesh("data/foilage/grass5.ms3d"));
  preloadedMeshes.push_back(SceneManager->getMesh("data/foilage/grass6.ms3d"));
  preloadedMeshes.push_back(SceneManager->getMesh("data/foilage/weeds/fern.ms3d"));
  preloadedMeshes.push_back(SceneManager->getMesh("data/foilage/weeds/fern2.ms3d"));
  preloadedMeshes.push_back(SceneManager->getMesh("data/foilage/weeds/weed1.ms3d"));
  preloadedMeshes.push_back(SceneManager->getMesh("data/foilage/weeds/weed2.ms3d"));
  preloadedMeshes.push_back(SceneManager->getMesh("data/foilage/weeds/weed3.ms3d"));

  // Grab preloaded meshes
  /*for(irr::u32 i=0; i<preloadedMeshes.size(); ++i)
  {
    preloadedMeshes[i]->grab();

    // Their textures also need to be grabbed
    for(irr::u32 m=0; m < preloadedMeshes[i]->getMeshBufferCount(); ++m)
    {
      for(irr::u32 t=0; t < 4; ++t)
        if(preloadedMeshes[i]->getMeshBuffer(m)->getMaterial().getTexture(t))
          preloadedMeshes[i]->getMeshBuffer(m)->getMaterial().getTexture(t)->grab();
    }
  }*/

  preloadedMeshes.clear();

  printf("ok!\n");
}


void CGame::updatePlayerWeapon()
{
  // Weapon object exists
  if(wPlayer != NULL)
  {
    // The weapon the player is holding is being reloaded ..
    if(wPlayer->isReloading())
    {
      // If decreaseReloadTimer returns true reload has been completed
      if(wPlayer->decreaseReloadTimer(Core->time.delta))
      {
        // If the weapon was in close-up mode before reloading,
        // restore it's state.
        if(l_restoreWeaponAimAfterReloading)
        {
          Core->getCamera()->toggleWeaponCloseUpAim();
          l_restoreWeaponAimAfterReloading = false;
        }

        // The weapon is shown again and ammo is updated
        Core->getCamera()->pushObjectForward();
        wPlayer->refreshActiveClip();
      }
      // Weapon is reloaded but it's not moving yet
      else if(Core->getCamera()->getObjectMovingDirection() == 0)
      {
        // Remembers that weapon is in close-up mode and goes into normal mode
        if(Core->getCamera()->isWeaponAimedCloseUp())
        {
          Core->getCamera()->toggleWeaponCloseUpAim();
          l_restoreWeaponAimAfterReloading = true;
        }

        // Move out of view
        Core->getCamera()->pullObjectBack();
      }
    } else {

      if(Core->getCamera()->getObjectMovingDirection() == 2)
      {
        if(!l_weaponArmSoundPlayed) {
          Core->getSound()->playSound2D(
            weaponsParameters[wPlayer->getType()]->fireSounds.getLast().c_str(),
            false,
            0.80f);

          l_weaponArmSoundPlayed = true;
        }
      }
      else {
        l_weaponArmSoundPlayed = false;
      }
    }
  }
}

void CGame::update()
{
  l_playerRelatedUpdates =
    (characters->getPlayer() != NULL && GUI->getMenu()->getType() != EMMT_IN_GAME &&
    GUI->getTerminal()->isActive() == false);

  // Updates level objects and other things
  Core->update();

  if(Input->isKeyPressedOnce(irr::KEY_KEY_1))
    setPlayerInventorySelectedItem(0);
  else if(Input->isKeyPressedOnce(irr::KEY_KEY_2))
    setPlayerInventorySelectedItem(1);


  if(Input->isKeyPressedOnce(irr::KEY_KEY_M))
  {
    irr::u32 totalRAM=0,availRAM=0;
    irr::u32 meshCache = Core->getRenderer()->getSceneManager()->getMeshCache()->getMeshCount();
    irr::u32 texCnt = Core->getRenderer()->getVideoDriver()->getTextureCount();

    Core->getRenderer()->getDevice()->getOSOperator()->getSystemMemory(&totalRAM, &availRAM);

    printf("\tRAM: %d\n\tMesh-cache: %d\n\tTextures: %d\n", availRAM, meshCache, texCnt);
  }

  // Escape toggles up the in-game menu
  // (only if we're in game and not in main menu(EMMT_DEFAULT))
  if(Input->isKeyPressedOnce(irr::KEY_ESCAPE))
  {
    if(GUI->getMenu()->getType() != EMMT_DEFAULT
    && GUI->getTerminal()->isActive() == false)
    {
      if(Core->commandLineParameters.hasParam("-skirmish"))
      {
         Core->requestClose();
         return;
      }


      // Close - stops the music and resumes ambient sound
      if(GUI->getMenu()->isActive())
      {
        GUI->getMenu()->setActive(false);

        GUI->getMenu()->playMusic(false);

        Core->setPaused(false);

        Core->getObjects()->setAmbientSoundsPaused(false);
      }
      // Set the menu type and other things
      // * Plays the menu music and stops the ambient sound
      // * Shows the cursor
      else
      {
        GUI->getMenu()->setType(game::EMMT_IN_GAME);
        GUI->getMenu()->changeTo(game::EMI_IN_GAME_MENU);
        GUI->getMenu()->setActive(true);
        GUI->getMenu()->playMusic(true, "data/sounds/music/menu2.ogg");

        Input->setCursorPosition(Core->getConfiguration()->getVideo()->windowSize/2);
        Input->setCursorTexture("data/2d/cursor.png");
        Input->setCursorVisible(true);

        Core->setPaused(true);

        Core->getObjects()->setAmbientSoundsPaused(true);
      }
    }
    else if(GUI->getTerminal()->isActive() == true)
    {
      GUI->getTerminal()->setActive(false);
    }
  }


  if(l_playerRelatedUpdates)
  {
    characters->getPlayer()->update();

    Core->getObjects()->checkDoorProximity(
      characters->getPlayer()->getBody()->PhysicsBody->getPositionBody());

    /*characters->getPlayer()->getBody()->PhysicsBody->drawDebug(
      Core->getRenderer()->getVideoDriver());*/
  }

  return;
}

void CGame::updateGUI()
{
  if(l_playerRelatedUpdates)
  {
    // Check crosshair object
    GUI->checkCrosshairObject();

    // Draw crosshair, health bar, ammo and so on.
    GUI->drawUI();

    updatePlayerWeapon();
  }

  GUI->update();

  // If purchase terminal is active, draw it
  GUI->getTerminal()->update();

  // Updates & draws the menus (checks if menu is currently enabled)
  GUI->getMenu()->update();

  /* Renders Irrlicht GUI elements (buttons, edit boxes, etc.)

     This is done in the game update function because
     getMenu()->update() draws the background and other images
     and we need to draw GUI elements on top of these but
     BEFORE drawing the cursor.
  */
  Core->getRenderer()->getGUI()->drawAll();

  // Draw the cursor (if visible)
  Input->drawCursor();
}

void CGame::loadExternalWeaponData()
{
  irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader("data/weapons.dat");
  bool elementOpen = false;
  weaponsParameters.set_used(0);
  SWeaponParameters *w;

  while(xml && xml->read())
  {
    switch(xml->getNodeType())
    {
      case irr::io::EXN_ELEMENT:
      {
        if(!strcmp("weapon", xml->getNodeName()) && !elementOpen) {
          elementOpen = true;
          w = new SWeaponParameters();
          w->w_class = (E_WEAPON_CLASS)xml->getAttributeValueAsInt("class");
        }

        if(!strcmp("firerate", xml->getNodeName()))
          w->fireRate = xml->getAttributeValueAsFloat("value");
        else if(!strcmp("clipsize", xml->getNodeName()))
          w->clipSize = xml->getAttributeValueAsInt("value");
        else if(!strcmp("reloadtime", xml->getNodeName()))
          w->reloadTime = xml->getAttributeValueAsFloat("value");
        else if(!strcmp("startclipsdefault", xml->getNodeName()))
          w->startClips = xml->getAttributeValueAsInt("value");
        else if(!strcmp("recoil", xml->getNodeName()))
          w->recoilForce = xml->getAttributeValueAsFloat("value");
        else if(!strcmp("firerange", xml->getNodeName()))
          w->fireRange = xml->getAttributeValueAsFloat("value");
        else if(!strcmp("mesh1", xml->getNodeName())) {
          w->f_mesh = irr::core::stringc(xml->getAttributeValue("value"));

          if(w->f_mesh != "")
            preloadedMeshes.push_back(Core->getRenderer()->getSceneManager()->getMesh(w->f_mesh.c_str()));
        }
        else if(!strcmp("mesh2", xml->getNodeName())) {
          w->t_mesh = irr::core::stringc(xml->getAttributeValue("value"));

          if(w->t_mesh != "")
            preloadedMeshes.push_back(Core->getRenderer()->getSceneManager()->getMesh(w->t_mesh.c_str()));
        }
        else if(!strcmp("muzzle", xml->getNodeName())) {
          w->f_muzzle = irr::core::stringc(xml->getAttributeValue("value"));

          if(w->f_muzzle != "")
            preloadedMeshes.push_back(Core->getRenderer()->getSceneManager()->getMesh(w->f_muzzle.c_str()));
        }
        else if(!strcmp("positionfirst", xml->getNodeName())) {
          w->f_position.X = xml->getAttributeValueAsFloat("x");
          w->f_position.Y = xml->getAttributeValueAsFloat("y");
          w->f_position.Z = xml->getAttributeValueAsFloat("z");
        }
        else if(!strcmp("scalefirst", xml->getNodeName())) {
          w->f_scale.X = xml->getAttributeValueAsFloat("x");
          w->f_scale.Y = xml->getAttributeValueAsFloat("y");
          w->f_scale.Z = xml->getAttributeValueAsFloat("z");
        }
        else if(!strcmp("scalethird", xml->getNodeName())) {
          w->t_scale.X = xml->getAttributeValueAsFloat("x");
          w->t_scale.Y = xml->getAttributeValueAsFloat("y");
          w->t_scale.Z = xml->getAttributeValueAsFloat("z");
        }
        else if(!strcmp("firesound", xml->getNodeName())) {
          irr::core::stringc fire_snd = irr::core::stringc(xml->getAttributeValue("value"));
          w->fireSounds.push_back(fire_snd);
        }
        else if(!strcmp("armsound", xml->getNodeName())) {
          irr::core::stringc fire_snd = irr::core::stringc(xml->getAttributeValue("value"));
          w->fireSounds.push_back(fire_snd);
        }
        else if(!strcmp("reloadsound", xml->getNodeName())) {
          irr::core::stringc reload_snd = irr::core::stringc(xml->getAttributeValue("value"));
          w->reloadSounds.push_back(reload_snd);
        }


      }

      case irr::io::EXN_ELEMENT_END:
      {
        if(!strcmp("weapon", xml->getNodeName()) && elementOpen) {
          elementOpen = false;
          weaponsParameters.push_back(w);
        }
      }
    }
  }

  delete xml;
}

void CGame::loadInventoryObjects()
{
  irr::scene::ISceneManager * SceneManager = Core->getRenderer()->getSceneManager();

  printf("Loading inventory objects ... ");

  characters->loadExternalCharacterClassData();
  loadExternalWeaponData();
  loadWeaponSounds();

  printf("ok!\n");
}

bool instantPlacement = false;

void CGame::setPlayerInventorySelectedItem(irr::u32 i)
{
  CPlayer *player = characters->getPlayer();

  if(player == NULL
  || Core->getCamera()->isFPSObjectReady() == false) return;

  if(wPlayer)
    if(wPlayer->isReloading()) return;

  CInventory *inventory = (CInventory*)player->getInventory();

  if(inventory->getSelectedItem() == i) return;

  if(Core->getCamera()->isWeaponAimedCloseUp())
    Core->getCamera()->toggleWeaponCloseUpAim();

  inventory->setSelectedItem(i);

  irr::core::vector3df objectPos;
  irr::core::vector3df objectScale;
  irr::scene::IMesh *objectMesh;

  SInventoryItem selectedItem = inventory->getItem(inventory->getSelectedItem());

  if(selectedItem.Type == INV_WEAPON)
  {
    CWeapon *weap = (CWeapon*)selectedItem.Data;

    objectMesh = Core->getRenderer()->getSceneManager()->getMesh(
      weaponsParameters[weap->getType()]->f_mesh.c_str());
    objectPos = weaponsParameters[weap->getType()]->f_position;
    objectScale = weaponsParameters[weap->getType()]->f_scale;

    setCurrentPlayerWeapon(weap);
  }

  objectPos *= 1.25f;
  objectScale *= 1.25f;


  Core->getCamera()->setFPSViewObject(
    objectMesh,
    objectPos,
    objectScale,
    instantPlacement);

  player->getBody()->InventoryObjectNode = (irr::scene::IAnimatedMeshSceneNode*)Core->getCamera()->getFPSViewObjectNode();

  return;
}



SInventoryItem CGame::createInventoryWeapon(
  E_WEAPON_TYPE type,
  irr::scene::ISceneNode *node)
{
  SWeaponCreationParameters params;
  params.type = type;
  params.node = node;
  params.w_class = weaponsParameters[type]->w_class;

  CWeapon *weapon;

  if(params.w_class == EWC_TRACED)
    weapon = new CTracedWeapon(this);

  weapon->setParams(params);

  SInventoryItem item;

  item.Type = INV_WEAPON;
  item.Data = weapon;

  return item;
}

void CGame::clearAll()
{
  wPlayer = (CWeapon*)NULL;
  characters->clearAll();
  GUI->clear();
}

void CGame::returnToMainMenu()
{
  Core->getObjects()->clearAll();

  Input->setCursorTexture("data/2d/cursor.png");
  Input->setCursorVisible(true);

  GUI->getMenu()->playMusic(true, "data/sounds/music/theme.ogg");
  GUI->getMenu()->setType(EMMT_DEFAULT);
  GUI->getMenu()->changeTo(EMI_MAIN_MENU);
  GUI->getMenu()->setActive(true);

  clearAll();

#ifdef PHYSICS_NEWTON
  Core->getPhysics()->clear();
#endif

}

void CGame::start(SGameParameters gameParams)
{
  gameParameters = gameParams;

  GUI->getMenu()->setActive(false);
  GUI->getMenu()->setType(EMMT_UNDEFINED);

  Core->getRenderer()->getGUI()->clear();

  GUI->enableLoadingScreen(game::ELS_LOAD_LEVEL, 0); // Loading level geom.

  loadInventoryObjects();
  loadMiscModels();

  //if(gameParameters.Type == EGT_SKIRMISH)
  {
    // Load the level
    Core->getObjects()->loadLevel(gameParameters.Level);
  }

  if(Core->commandLineParameters.hasParam("-camera"))
  {
    Core->getCamera()->setType(ECT_SPECTATOR);
  }

  if(!Core->commandLineParameters.hasParam("-disable_characters"))
  {
    GUI->enableLoadingScreen(game::ELS_LOAD_LEVEL, 5);

    // Create player
    SCharacterCreationParameters playerParameters;
    playerParameters.TeamID = (E_TEAMS)gameParameters.TeamID;
    playerParameters.Class = (E_CHARACTER_CLASSES)gameParameters.Class;

    CPlayer *player = characters->createPlayer(playerParameters);

    if(!Core->commandLineParameters.hasParam("-camera"))
    {
      Core->getCamera()->setType(ECT_FIRST_PERSON);
      Core->getCamera()->setFollowedCharacter(player);
    }

    // Create bots
    // TODO

    /// Center the cursor
    Input->setCursorPosition(Core->getConfiguration()->getVideo()->windowSize/2);

    GUI->enableLoadingScreen(game::ELS_LOAD_LEVEL, 6);

    /// Create inventory for player
    CInventory *inv = new CInventory();

    //inv->addItem(gameParameters.TeamID == E_TEAM1 ? INV_GOV_MACHINE_GUN : INV_NOVA_MACHINE_GUN);
    //inv->addItem(gameParameters.TeamID == E_TEAM1 ? INV_GOV_SNIPER_RIFLE : INV_NOVA_SNIPER_RIFLE);

    irr::scene::ISceneNode *playerWeaponNode = Core->getCamera()->getFPSViewObjectNode();

    inv->addItem( createInventoryWeapon(INV_GOV_MACHINE_GUN, playerWeaponNode) );
    inv->addItem( createInventoryWeapon(INV_NOVA_MACHINE_GUN, playerWeaponNode) );

    characters->getPlayer()->setInventory((void*)inv);

    instantPlacement = true;
    setPlayerInventorySelectedItem(0);
    instantPlacement = false;
  }

  //Core->getRenderer()->getVideoDriver()->setAmbientLight(irr::video::SColorf(1.f, 0.1f, 0.1f, 0.1f));

  Core->setPaused(false);

  GUI->getMenu()->playMusic(false);
  GUI->getTerminal()->reset();
  Core->getSound()->playSound2D("data/sounds/vocal/welcome.ogg", false, 0.87f);

  if(characters->getPlayer())
    characters->spawn(characters->getPlayer());

  GUI->disableLoadingScreen();
  GUI->init();

  if(gameParameters.Type == EGT_SKIRMISH_ANNIHILATION
  || gameParameters.Type == EGT_SKIRMISH_CAPTURE_FLAG)
  {
    printf("Setting timer to %d hours %d minutes ... ",
      gameParameters.Skirmish.TimeHours,
      gameParameters.Skirmish.TimeMinutes);

    Core->getTimer()->setTimer(
      TIMER_SKIRMISH,
      gameParameters.Skirmish.TimeHours,
      gameParameters.Skirmish.TimeMinutes,
      0); // intital time

    Core->getTimer()->pauseTimer(TIMER_SKIRMISH, false); // start it

    printf("ok!\n");

    characters->getPlayer()->getStats()->credit = gameParameters.Skirmish.StartCredit;
  }
  else
  {
    Core->getTimer()->setTimer(0, 99, 99, 00);
    Core->getTimer()->pauseTimer(0, true);
  }

  return;
}

void CGame::loadMiscSounds()
{
  surfaceHitSoundsStone.push_back("data/sounds/hitsounds/stone/0.wav");
  surfaceHitSoundsStone.push_back("data/sounds/hitsounds/stone/1.wav");
  surfaceHitSoundsStone.push_back("data/sounds/hitsounds/stone/2.wav");
  surfaceHitSoundsStone.push_back("data/sounds/hitsounds/stone/3.wav");
  surfaceHitSoundsStone.push_back("data/sounds/hitsounds/stone/4.wav");
  surfaceHitSoundsStone.push_back("data/sounds/hitsounds/stone/5.wav");
  surfaceHitSoundsStone.push_back("data/sounds/hitsounds/stone/6.wav");
  surfaceHitSoundsStone.push_back("data/sounds/hitsounds/stone/7.wav");
  surfaceHitSoundsStone.push_back("data/sounds/hitsounds/stone/8.wav");
  surfaceHitSoundsStone.push_back("data/sounds/hitsounds/stone/9.wav");

  surfaceHitSoundsWood.push_back("data/sounds/hitsounds/wood/0.wav");
  surfaceHitSoundsWood.push_back("data/sounds/hitsounds/wood/1.wav");
  surfaceHitSoundsWood.push_back("data/sounds/hitsounds/wood/2.wav");
  surfaceHitSoundsWood.push_back("data/sounds/hitsounds/wood/3.wav");
  surfaceHitSoundsWood.push_back("data/sounds/hitsounds/wood/4.wav");
  surfaceHitSoundsWood.push_back("data/sounds/hitsounds/wood/5.wav");
  surfaceHitSoundsWood.push_back("data/sounds/hitsounds/wood/6.wav");
  surfaceHitSoundsWood.push_back("data/sounds/hitsounds/wood/7.wav");

  surfaceHitSoundsMetal.push_back("data/sounds/hitsounds/metal/0.wav");
  surfaceHitSoundsMetal.push_back("data/sounds/hitsounds/metal/1.wav");
  surfaceHitSoundsMetal.push_back("data/sounds/hitsounds/metal/2.wav");
  surfaceHitSoundsMetal.push_back("data/sounds/hitsounds/metal/3.wav");
  surfaceHitSoundsMetal.push_back("data/sounds/hitsounds/metal/4.wav");
  surfaceHitSoundsMetal.push_back("data/sounds/hitsounds/metal/5.wav");
  surfaceHitSoundsMetal.push_back("data/sounds/hitsounds/metal/6.wav");

  surfaceHitSoundsCanvas.push_back("data/sounds/hitsounds/canvas/0.wav");
  surfaceHitSoundsCanvas.push_back("data/sounds/hitsounds/canvas/1.wav");
  surfaceHitSoundsCanvas.push_back("data/sounds/hitsounds/canvas/2.wav");
  surfaceHitSoundsCanvas.push_back("data/sounds/hitsounds/canvas/3.wav");
  surfaceHitSoundsCanvas.push_back("data/sounds/hitsounds/canvas/4.wav");
  surfaceHitSoundsCanvas.push_back("data/sounds/hitsounds/canvas/5.wav");
  surfaceHitSoundsCanvas.push_back("data/sounds/hitsounds/canvas/6.wav");
  surfaceHitSoundsCanvas.push_back("data/sounds/hitsounds/canvas/7.wav");
  surfaceHitSoundsCanvas.push_back("data/sounds/hitsounds/canvas/8.wav");

  surfaceHitSoundsGround.push_back("data/sounds/hitsounds/ground/0.wav");
  surfaceHitSoundsGround.push_back("data/sounds/hitsounds/ground/1.wav");
  surfaceHitSoundsGround.push_back("data/sounds/hitsounds/ground/2.wav");
  surfaceHitSoundsGround.push_back("data/sounds/hitsounds/ground/3.wav");
  surfaceHitSoundsGround.push_back("data/sounds/hitsounds/ground/4.wav");

  Core->getSound()->preloadSounds(surfaceHitSoundsStone);
  Core->getSound()->preloadSounds(surfaceHitSoundsWood);
  Core->getSound()->preloadSounds(surfaceHitSoundsMetal);
  Core->getSound()->preloadSounds(surfaceHitSoundsCanvas);
  Core->getSound()->preloadSounds(surfaceHitSoundsGround);

  Core->getSound()->preloadSound("data/sounds/ambient/hum1.ogg");
  Core->getSound()->preloadSound("data/sounds/ambient/transformer.ogg");
  Core->getSound()->preloadSound("data/sounds/ambient/eerie.ogg");
  Core->getSound()->preloadSound("data/sounds/ambient/lightbuzz.ogg");
  Core->getSound()->preloadSound("data/sounds/ambient/trees.ogg");
  Core->getSound()->preloadSound("data/sounds/ambient/day.ogg");
  Core->getSound()->preloadSound("data/sounds/ambient/amb_berlin_ext.ogg.ogg");
}

void CGame::loadWeaponSounds()
{
  for(irr::u16 i=0; i<weaponsParameters.size(); ++i)
  {
    Core->getSound()->preloadSounds(weaponsParameters[i]->fireSounds);
    Core->getSound()->preloadSounds(weaponsParameters[i]->reloadSounds);
  }
}

void CGame::createSurfaceHitParticles(
  const irr::c8* particle_texture_file,
  irr::core::vector3df position,
  irr::core::vector3df normal,
  irr::f32 particle_scale,
  irr::f32 particle_velocity,
  irr::f32 fade_time,
  irr::u32 emit_min,
  irr::u32 emit_max,
  irr::u32 random_angle,
  irr::scene::ISceneNode *emitter_parent)
{
  irr::u32 fade_time_in_ms = irr::u32(fade_time*1000);

  irr::scene::ISceneNodeAnimator* del =
    Core->getRenderer()->getSceneManager()->createDeleteAnimator(fade_time_in_ms);
  irr::scene::IParticleSystemSceneNode* ps =
    Core->getRenderer()->getSceneManager()->addParticleSystemSceneNode(false);

  ps->addAnimator(del);
  del->drop();

  irr::scene::IParticleEmitter* em = ps->createPointEmitter(
    normal*particle_velocity,   // initial direction
    emit_min, emit_max,                                    // emit rate
    irr::video::SColor(0,255,255,255),       // darkest color
    irr::video::SColor(0,255,255,255),       // brightest color
    fade_time_in_ms-30,fade_time_in_ms,random_angle,                             // min and max age, angle
      irr::core::dimension2df(0.7f,0.7f)*particle_scale,      // min size
      irr::core::dimension2df(1.0f,1.0f)*particle_scale);     // max size

  ps->setParticlesAreGlobal(true);
  ps->setEmitter(em); // this grabs the emitter
  em->drop(); // so we can drop it here without deleting it

  if(emitter_parent)
    ps->setParent(emitter_parent);

  ps->setPosition(position);
  ps->setScale(irr::core::vector3df(1,1,1));
  ps->setMaterialFlag(irr::video::EMF_LIGHTING, false);
  ps->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, false);
  ps->setMaterialTexture(0,
    Core->getRenderer()->getVideoDriver()->getTexture(particle_texture_file));
  ps->setMaterialType(
    (irr::video::E_MATERIAL_TYPE)Core->getRenderer()->getShaders()->createParticleFadeOutShader(fade_time));

}

void CGame::playSurfaceHitSound(
  E_BODY_MATERIAL_TYPE material,
  irr::core::vector3df &position)
{
  const irr::c8 *hit_sound = NULL;

  switch(material) {
    case EBMT_STONE:
      hit_sound = surfaceHitSoundsStone[Core->getMath()->getRandomInt(0,surfaceHitSoundsStone.size()-1)];
    break;
    case EBMT_WOOD:
      hit_sound = surfaceHitSoundsWood[Core->getMath()->getRandomInt(0,surfaceHitSoundsWood.size()-1)];
    break;
    case EBMT_METAL:
      hit_sound = surfaceHitSoundsMetal[Core->getMath()->getRandomInt(0,surfaceHitSoundsMetal.size()-1)];
    break;
    case EBMT_SANDBAG:
      hit_sound = surfaceHitSoundsCanvas[Core->getMath()->getRandomInt(0,surfaceHitSoundsCanvas.size()-1)];
    break;
    case EBMT_GROUND:
      hit_sound = surfaceHitSoundsGround[Core->getMath()->getRandomInt(0,surfaceHitSoundsGround.size()-1)];
    break;

    default:
      hit_sound = surfaceHitSoundsStone[Core->getMath()->getRandomInt(0,surfaceHitSoundsStone.size()-1)];
    break;
  }

  if(hit_sound != NULL)
    Core->getSound()->playSound3D(hit_sound, position, false, 5.7f, 0.75f, 3.45f);

}
