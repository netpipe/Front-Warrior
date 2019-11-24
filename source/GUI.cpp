#include "Game.h"
#include "GUI.h"
#include "Menu.h"
#include "Terminal.h"
#include "CharacterManager.h"
#include "Inventory.h"
#include "Weapon.h"
#include "Player.h"
#include "Physics.h"
#include "ObjectManager.h"
#include "Camera.h"
#include "GameInput.h"

#include "Renderer.h"
#include "Camera.h"
#include "Clock.h"

using namespace game;
using namespace irr::video;

CGUI::CGUI(CGame*game, irr::core::dimension2d<irr::u16> screensize) : Game(game)
{
  // Nullify pointers just to be on the safe side
  for(irr::u16 i=0; i<TEX_COUNT; ++i)
    textures[i] = (irr::video::ITexture*) NULL;

  screenSize = screensize;

  m_Menu = new CMenu(this, Game);
  m_Terminal = new CTerminal(this, Game);

  crosshairObject.icons = 0;

  screenFader = (irr::gui::IGUIInOutFader*)NULL;
}

CGUI::~CGUI()
{
  delete m_Menu;
}


void CGUI::init()
{

}

void CGUI::update()
{
  if(screenFader)
  {
    if(screenFader->isReady())
    {
      switch(fadeID)
      {
      case EGFI_TERMINAL_ACTIVATE:
        m_Menu->playMusic(true, "data/sounds/music/terminal.ogg");
      break;

      case EGFI_TERMINAL_DEACTIVATE:
        m_Menu->playMusic(false);
      break;
      }

      screenFader->remove();
      screenFader = (irr::gui::IGUIInOutFader*) NULL;
    }
  }
}

void CGUI::drawUI()
{
  irr::video::IVideoDriver* driver = Game->getCore()->getRenderer()->getVideoDriver();

  engine::CBaseCharacter *player = (engine::CBaseCharacter*)Game->getCharacters()->getPlayer();
  CInventory *inventory = (game::CInventory*)player->getInventory();
  SInventoryItem selectedItem = inventory->getItem(inventory->getSelectedItem());
  SAmmoClip *selected_clip = NULL;
  CWeapon *weap = NULL;

  irr::u32 centerX = screenSize.Width / 2;
  irr::u32 centerY = screenSize.Height / 2;

  if(crosshairObject.showCrosshair)
  {
    irr::scene::ISceneCollisionManager* col_man =
      Game->getCore()->getRenderer()->getSceneManager()->getSceneCollisionManager();

    irr::core::position2d<irr::s32> targetPos2D = col_man->getScreenCoordinatesFrom3DPosition(
        crosshairObject.hit_position);

    targetPos2D.X -= 16;
    targetPos2D.Y -= 16;



    if(crosshairObject.icons == 0)
    {
      driver->draw2DImage(
        textures[TEX_CROSSHAIR_FRIENDLY],
        targetPos2D,
        irr::core::rect<irr::s32>(0,0,32,32),
        0,
        SColor(225,255,255,255),
        true);
    }
    else
    {
      if(crosshairObject.icons & ECI_HAND)
      driver->draw2DImage(
        textures[TEX_HAND_ICON],
        irr::core::position2d<irr::s32>(targetPos2D.X, targetPos2D.Y),
        irr::core::rect<irr::s32>(0,0,32,32),
        0, irr::video::SColor(225, 255, 255, 255), true);
    }


    if(crosshairObject.visible)
    {
      fonts[1]->draw(
        crosshairObject.name,
        irr::core::rect<irr::s32>(targetPos2D.X+32, targetPos2D.Y+2, targetPos2D.X+200, targetPos2D.Y+30),
        SColor(255,255,255,255),
        false);

      if(crosshairObject.showHealthbar)
      {
        driver->draw2DImage(
          textures[TEX_HEALTHBAR_BG],
          irr::core::position2d<irr::s32>(targetPos2D.X+32, targetPos2D.Y+20),
          irr::core::rect<irr::s32>(0,0,64,8), 0, irr::video::SColor(128, 255, 255, 255), true);

        driver->draw2DImage(
          textures[TEX_HEALTHBAR],
          irr::core::position2d<irr::s32>(targetPos2D.X+33, targetPos2D.Y+20),
          irr::core::rect<irr::s32>(0,0,irr::s32(60*crosshairObject.healthLevel),8),
          0, irr::video::SColor(225, 255, 255, 255), true);
      }
    }
  }

  centerX = screenSize.Width - 112;

  // Background credits and time
  driver->draw2DImage(textures[TEX_HUD_BG_2],
    irr::core::position2d<irr::s32>(centerX-128, 4),
    irr::core::rect<irr::s32>(0,0,256,64), 0, SColor(225,255,255,255), true);

  // Credit
  driver->draw2DImage(
    textures[TEX_CREDIT_ICON],
    irr::core::position2d<irr::s32>(centerX-115, 6),
    irr::core::rect<irr::s32>(0,0,32,32),
    0,
    SColor(225,255,255,255),
    true);

  // Time
  driver->draw2DImage(
    textures[TEX_TIME_ICON],
    irr::core::position2d<irr::s32>(centerX+78, 7),
    irr::core::rect<irr::s32>(0,0,32,32),
    0,
    SColor(225,255,255,255),
    true);



  irr::core::stringw creditStr = irr::core::stringw(player->getStats()->credit);

  fonts[0]->draw(
    creditStr.c_str(),
    irr::core::rect<irr::s32>(centerX-74, 13, centerX-40, 30),
    SColor(128,0,0,0),
    false);

  fonts[0]->draw(
    creditStr.c_str(),
    irr::core::rect<irr::s32>(centerX-75, 12, centerX-40, 30),
    SColor(255,255,255,255),
    false);

  irr::u32 timeLeft_H, timeLeft_M, timeLeft_S;
  irr::s16 timerXOffset = 0;

  Game->getCore()->getTimer()->getTimerValues(0, timeLeft_H, timeLeft_M, timeLeft_S);

  irr::core::stringw timerStr = L"";

  if(timeLeft_H != 99)
  {
    if(timeLeft_H < 10) timerStr += "0";
    timerStr += timeLeft_H;

    timerStr += ":";

    if(timeLeft_M < 10) timerStr += "0";
    timerStr += timeLeft_M;

    timerStr += ":";

    if(timeLeft_S < 10) timerStr += "0";
    timerStr += timeLeft_S;

    timerXOffset = -8;
  }
  else
  {
    timerStr = L"--:--:--";
    timerXOffset = 10;
  }

  fonts[0]->draw(
    timerStr.c_str(),
    irr::core::rect<irr::s32>(centerX+15+timerXOffset, 13, centerX+90, 30),
    SColor(128,0,0,0),
    false);

  fonts[0]->draw(
    timerStr.c_str(),
    irr::core::rect<irr::s32>(centerX+16+timerXOffset, 12, centerX+90, 30),
    SColor(255,255,255,255),
    false);

  // Time
  //wchar_t timeStr[8];
  //Game->FormatTime(timeStr, s32(Game->GameClock.getTime(1)), s32(Game->GameClock.getTime(0)));


  //swprintf(guiTextStr, L"0:%s", timeStr);
  //fonts[0]->draw(guiTextStr, rect<s32>(centerX+35, 8, centerX+100, 30), SColor(255,255,255,255), false);


  //
  // Show ammo
  //

  if(selectedItem.Type == INV_WEAPON) {
    weap = (CWeapon*)selectedItem.Data;
    selected_clip = weap->getActiveClip();
  }


  irr::core::stringw ammoInClip;
  irr::core::stringw ammoClipCount;

  if(selected_clip != NULL) {
    ammoInClip = irr::core::stringw(selected_clip->ammo);
    ammoClipCount = irr::core::stringw(weap->getClipCount()-1);
  }
  else {
    ammoInClip = L"0";
    ammoClipCount = L"0";
  }

  // Background for ammo
  driver->draw2DImage(textures[TEX_HUD_BG],
    irr::core::position2d<irr::s32>(screenSize.Width-135, screenSize.Height-50),
    irr::core::rect<irr::s32>(0,0,256,64), 0, SColor(225,255,255,255), true);

  // Background for player health
  driver->draw2DImage(textures[TEX_HUD_BG_VERTICAL],
    irr::core::position2d<irr::s32>(5, screenSize.Height-160),
    irr::core::rect<irr::s32>(0,0,44,256), 0, SColor(225,255,255,255), true);

  // Background for player stamina
  driver->draw2DImage(textures[TEX_HUD_BG_VERTICAL],
    irr::core::position2d<irr::s32>(47, screenSize.Height-140),
    irr::core::rect<irr::s32>(0,0,44,256), 0, SColor(225,255,255,255), true);




  // Ammo
  driver->draw2DImage(textures[TEX_AMMO_ICON],
    irr::core::position2d<irr::s32>(screenSize.Width-130, screenSize.Height-45),
    irr::core::rect<irr::s32>(0,0,32,32), 0, SColor(225,255,255,255), true);

  // Clips
  driver->draw2DImage(textures[TEX_CLIP_ICON],
    irr::core::position2d<irr::s32>(screenSize.Width-28, screenSize.Height-45),
    irr::core::rect<irr::s32>(0,0,32,32), 0, SColor(225,255,255,255), true);

  // Health
  driver->draw2DImage(textures[TEX_HEALTH_ICON],
    irr::core::position2d<irr::s32>(7, screenSize.Height-158),
    irr::core::rect<irr::s32>(0,0,32,32), 0, SColor(225,255,255,255), true);

  // Stamina
  driver->draw2DImage(textures[TEX_STAMINA_ICON],
    irr::core::position2d<irr::s32>(48, screenSize.Height-137),
    irr::core::rect<irr::s32>(0,0,32,32), 0, SColor(225,255,255,255), true);

  // Stamina-bar
  /*driver->draw2DImage(textures[TEX_STAMINA_BAR],
    irr::core::position2d<irr::s32>(71, screenSize.Height-36),
    irr::core::rect<irr::s32>(0,0,64,16), 0, SColor(225,255,255,255), true);*/

  // TEXT SHADOW
  fonts[3]->draw(
    ammoInClip,
    irr::core::rect<irr::s32>(screenSize.Width-91,screenSize.Height-40,screenSize.Width-50,screenSize.Height),
    SColor(128,0,0,0), false);

  fonts[3]->draw(
    ammoClipCount,
    irr::core::rect<irr::s32>(screenSize.Width-45,screenSize.Height-40,screenSize.Width,screenSize.Height),
    SColor(128,0,0,0), false);

  // TEXT FOREGROUND
  fonts[3]->draw(
    ammoInClip,
    irr::core::rect<irr::s32>(screenSize.Width-92,screenSize.Height-41,screenSize.Width-50,screenSize.Height),
    SColor(255,255,255,255), false);

  fonts[3]->draw(
    ammoClipCount,
    irr::core::rect<irr::s32>(screenSize.Width-46,screenSize.Height-41,screenSize.Width,screenSize.Height),
    SColor(255,255,255,255), false);

  //SCharacterClassParameters *player_class_params =
    //Game->cClassParameters[Game->getCharacters()->getPlayer()->getParameters()->Class];

  // Health background
  driver->draw2DRectangle(
    irr::video::SColor(128, 8,8,8),
    irr::core::rect<irr::s32>(15, screenSize.Height-122, 31, screenSize.Height-6));

  // Health level
  irr::f32 health_ = player->getParameters()->Health / player->getParameters()->HealthMax;

  driver->draw2DRectangle(
    irr::video::SColor(150, 170,130,135),
    irr::core::rect<irr::s32>(17, irr::s32(screenSize.Height-(120*health_)),29, screenSize.Height-8));


  // Stamina background
  driver->draw2DRectangle(
    irr::video::SColor(128, 8,8,8),
    irr::core::rect<irr::s32>(57, screenSize.Height-98, 73, screenSize.Height-6));

  // Stamina level
  irr::f32 stamina_ = player->getParameters()->Stamina / player->getParameters()->StaminaMax;

  driver->draw2DRectangle(
    irr::video::SColor(150, 130,130,180),
    irr::core::rect<irr::s32>(59, irr::s32(screenSize.Height-(96*stamina_)), 71, screenSize.Height-8));


}

void CGUI::fade(E_GUI_FADE_ID id, irr::u32 time, irr::video::SColor color)
{
  screenFader = Game->getCore()->getRenderer()->getGUI()->addInOutFader();
  screenFader->setColor(color);
  screenFader->fadeIn(time);
  screenFader->grab();

  fadeID = id;
}

void CGUI::loadFonts()
{
  fonts.push_back(Game->getCore()->getRenderer()->getGUI()->getFont("data/fonts/font1.png")); // 0
  fonts.push_back(Game->getCore()->getRenderer()->getGUI()->getFont("data/fonts/fonttarget.png")); // 1
  fonts.push_back(Game->getCore()->getRenderer()->getGUI()->getFont("data/fonts/smallfont2.png")); // 2
  fonts.push_back(Game->getCore()->getRenderer()->getGUI()->getFont("data/fonts/bighudfont.png")); // 3
}

void CGUI::clear(bool close)
{
  if(close)
  {
    if(m_Menu)
    {
      // Stop the music
      m_Menu->playMusic(false);

      // Unload textures
      m_Menu->unloadAssets();
    }

    for(irr::u16 i=0; i<TEX_COUNT; ++i)
      if(textures[i])
        textures[i]->drop();
  }
}

void CGUI::loadUITextures()
{
  irr::video::IVideoDriver* driver = Game->getCore()->getRenderer()->getVideoDriver();

  textures[TEX_CROSSHAIR_FRIENDLY] = driver->getTexture("data/2d/crosshair1.png");
  textures[TEX_CROSSHAIR_ENEMY] = driver->getTexture("data/2d/crosshair0.png");
  textures[TEX_TIME_ICON] = driver->getTexture("data/2d/icons/time.png");
  textures[TEX_CREDIT_ICON] = driver->getTexture("data/2d/icons/credit.png");
  textures[TEX_OVERLAY_BINOCULARS] = driver->getTexture("data/2d/binoculars.png");
  textures[TEX_OVERLAY_SNIPER] = driver->getTexture("data/2d/sniperScope1.png");
  textures[TEX_AMMO_ICON] = driver->getTexture("data/2d/icons/ammo2.png");
  textures[TEX_CLIP_ICON] = driver->getTexture("data/2d/icons/clip.png");
  textures[TEX_HEALTH_ICON] = driver->getTexture("data/2d/icons/health.png");
  textures[TEX_STAMINA_ICON] = driver->getTexture("data/2d/icons/stamina.png");
  textures[TEX_HUD_BG] = driver->getTexture("data/2d/hud_bg.png");
  textures[TEX_HUD_BG_VERTICAL] = driver->getTexture("data/2d/hud_bg_vertical.png");
  textures[TEX_HUD_BG_2] = driver->getTexture("data/2d/hud_bg_thinner.png");
  textures[TEX_HEALTHBAR_BG] = driver->getTexture("data/2d/healthbarbg.png");
  textures[TEX_HEALTHBAR] = driver->getTexture("data/2d/healthbar.png");
  textures[TEX_HAND_ICON] = driver->getTexture("data/2d/icons/hand.png");
  textures[TEX_TERMINAL_BACKGROUND] = driver->getTexture("data/2d/terminal/background.jpg");
  textures[TEX_TERMINAL_EXIT_BUTTON] = driver->getTexture("data/2d/terminal/exit.png");
  textures[TEX_TERMINAL_REFILL_ICON] = driver->getTexture("data/2d/terminal/refill.png");
  textures[TEX_TERMINAL_CHANGE_TEAM_ICON] = driver->getTexture("data/2d/terminal/changeteam.png");
  textures[TEX_TERMINAL_CHANGE_CHARACTER_ICON] = driver->getTexture("data/2d/terminal/change.png");
  textures[TEX_TERMINAL_BUY_ICON] = driver->getTexture("data/2d/terminal/buy_medtank.png");

  for(irr::u16 i=0; i<TEX_COUNT; ++i)
    if(textures[i])
      textures[i]->grab();

}

irr::video::ITexture* titleScreen = (irr::video::ITexture*)NULL;
irr::video::ITexture* teamIcon = (irr::video::ITexture*)NULL;
irr::video::ITexture* levelIcon = (irr::video::ITexture*)NULL;
irr::video::ITexture* loadingText = (irr::video::ITexture*)NULL;
irr::video::ITexture* step1_geom = (irr::video::ITexture*)NULL;
irr::video::ITexture* step2_grp = (irr::video::ITexture*)NULL;
irr::video::ITexture* step3_phys = (irr::video::ITexture*)NULL;
irr::video::ITexture* step4_grass = (irr::video::ITexture*)NULL;
irr::video::ITexture* step5_inv = (irr::video::ITexture*)NULL;
irr::video::ITexture* step6_pl = (irr::video::ITexture*)NULL;

void CGUI::enableLoadingScreen(E_LOADING_SCREEN type, irr::u16 state)
{
  Game->getCore()->getRenderer()->getDevice()->run();

  // Unload any textures before loading new ones
  //disableLoadingScreen();

  // Variables
  irr::video::IVideoDriver* driver = Game->getCore()->getRenderer()->getVideoDriver();
  irr::gui::IGUIEnvironment* guienv = Game->getCore()->getRenderer()->getGUI();
  irr::core::dimension2d<irr::u16> windowSize = screenSize;

  // Let's begin constructing our loading screen!
  driver->beginScene(true, true, irr::video::SColor(255,0,0,0));

  // When loading textures dont't create mip maps
  driver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);

  // Title screen
  if(type == ELS_STARTUP)
  {
    titleScreen = driver->getTexture("data/2d/menu/titlescreen.jpg");

    driver->draw2DImage(
      titleScreen,
      irr::core::rect<irr::s32>(0,0, windowSize.Width, windowSize.Height),
      irr::core::rect<irr::s32>(0,0,1024,1024), 0, 0, true);
  }
  // When loading a level
  else if(type == ELS_LOAD_LEVEL)
  {
    irr::core::stringc levelImageFile = "";
    irr::core::stringw tipText = L"";

    irr::u32 levelImageHeight = irr::u32(windowSize.Height / 1.5);
    irr::u32 levelicon_y = (windowSize.Height / 2) - (levelImageHeight/2);

    if(state == 0)
    {
      levelImageFile = "data/levels/";
      levelImageFile += Game->getParameters().Level;
      levelImageFile += "/large.jpg";

      if(Game->getParameters().Type == EGT_SKIRMISH_ANNIHILATION)
      {
        tipText =
        L"ANNIHILATION\nIn this mode, Goverment and Nova teams battle to destroy the opposite base. "
        "To succeed both teams need to use a wide array of vehicles and weapons and find weakness "
        "in the enemy defence line.";
      }
      else if(Game->getParameters().Type == EGT_SKIRMISH_CAPTURE_FLAG)
      {
        tipText =
        L"CAPTURE THE FLAG\nCapture the flag from the enemy and return it safely to home base.";
      }


      if(levelImageFile != "")
        levelIcon = driver->getTexture(levelImageFile.c_str());

      teamIcon = Game->getParameters().TeamID == E_TEAM1
        ? driver->getTexture("data/2d/menu/goverment.png")
        : driver->getTexture("data/2d/menu/nova.png");

      loadingText = driver->getTexture("data/2d/menu/loading.jpg");

      step1_geom = driver->getTexture("data/2d/icons/levelgeom.png");
      step2_grp = driver->getTexture("data/2d/icons/grouping.png");
      step3_phys = driver->getTexture("data/2d/icons/physics.png");
      step4_grass = driver->getTexture("data/2d/icons/grass.png");
      step5_inv = driver->getTexture("data/2d/icons/inventory.png");
      step6_pl = driver->getTexture("data/2d/icons/player_.png");

      irr::gui::IGUIStaticText *tipStaticText =
        guienv->addStaticText(tipText.c_str(),
        irr::core::rect<irr::s32>(10,10,windowSize.Width-10,levelicon_y+50), false, true);

      tipStaticText->setOverrideFont(Game->getGUI()->getFont(0));
      tipStaticText->setOverrideColor(SColor(255, 255,255,255));
      tipStaticText->enableOverrideColor(true);
    }

    irr::u32 teamicon_y = (windowSize.Height / 2) - 128;
    irr::u32 teamicon_x = (windowSize.Width / 2) - 128;

    if(levelIcon != NULL)
    {
      driver->draw2DImage(
          levelIcon,
          irr::core::rect<irr::s32>(0, levelicon_y, windowSize.Width, levelicon_y + levelImageHeight),
          irr::core::rect<irr::s32>(0,0,1024,512), 0, 0, true);
    }

    guienv->drawAll();

    driver->draw2DImage(
        teamIcon,
        irr::core::rect<irr::s32>(teamicon_x, teamicon_y, teamicon_x+256, teamicon_y+256),
        irr::core::rect<irr::s32>(0,0,256,256), 0, 0, true);

    driver->draw2DImage(
        loadingText,
        irr::core::rect<irr::s32>(5, windowSize.Height-64, 5+256, windowSize.Height),
        irr::core::rect<irr::s32>(0,0,256,64), 0, 0, true);

    irr::s32 l_off = 10;

    driver->draw2DImage(
        step1_geom,
        irr::core::rect<irr::s32>(
          windowSize.Width-32-l_off,
          windowSize.Height-32-l_off,
          windowSize.Width-l_off,
          windowSize.Height-l_off),
        irr::core::rect<irr::s32>(0,0,32,32), 0, 0, true);

    if(state >= 1)
      driver->draw2DImage(
          step2_grp,
          irr::core::rect<irr::s32>(
            windowSize.Width-64-l_off*2,
            windowSize.Height-32-l_off,
            windowSize.Width-32-l_off*2,
            windowSize.Height-l_off),
          irr::core::rect<irr::s32>(0,0,32,32), 0, 0, true);

    if(state >= 2)
      driver->draw2DImage(
          step3_phys,
          irr::core::rect<irr::s32>(
            windowSize.Width-96-l_off*3,
            windowSize.Height-32-l_off,
            windowSize.Width-64-l_off*3,
            windowSize.Height-l_off),
          irr::core::rect<irr::s32>(0,0,32,32), 0, 0, true);

    if(state >= 3)
      driver->draw2DImage(
          step4_grass,
          irr::core::rect<irr::s32>(
            windowSize.Width-128-l_off*4,
            windowSize.Height-32-l_off,
            windowSize.Width-96-l_off*4,
            windowSize.Height-l_off),
          irr::core::rect<irr::s32>(0,0,32,32), 0, 0, true);

    if(state >= 5)
      driver->draw2DImage(
          step5_inv,
          irr::core::rect<irr::s32>(
            windowSize.Width-160-l_off*5,
            windowSize.Height-32-l_off,
            windowSize.Width-128-l_off*5,
            windowSize.Height-l_off),
          irr::core::rect<irr::s32>(0,0,32,32), 0, 0, true);

    if(state >= 6)
      driver->draw2DImage(
          step6_pl,
          irr::core::rect<irr::s32>(
            windowSize.Width-192-l_off*6,
            windowSize.Height-32-l_off,
            windowSize.Width-160-l_off*6,
            windowSize.Height-l_off),
          irr::core::rect<irr::s32>(0,0,32,32), 0, 0, true);



  }

  // Restore mip map creation flag to default
  driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, true);

  driver->endScene();

  return;
}

void CGUI::disableLoadingScreen()
{
  /*step1_geom->drop();
  step2_grp->drop();
  step3_phys->drop();
  step4_grass->drop();
  step5_inv->drop();
  step6_pl->drop();*/

  Game->getCore()->getRenderer()->getGUI()->clear();

  if(titleScreen) {
    Game->getCore()->getRenderer()->getVideoDriver()->removeTexture(titleScreen);
    titleScreen = (irr::video::ITexture*) NULL;
  }

  Game->getCore()->getRenderer()->getVideoDriver()->removeTexture(step1_geom);
  Game->getCore()->getRenderer()->getVideoDriver()->removeTexture(step2_grp);
  Game->getCore()->getRenderer()->getVideoDriver()->removeTexture(step3_phys);
  Game->getCore()->getRenderer()->getVideoDriver()->removeTexture(step4_grass);
  Game->getCore()->getRenderer()->getVideoDriver()->removeTexture(step5_inv);
  Game->getCore()->getRenderer()->getVideoDriver()->removeTexture(step6_pl);

  if(levelIcon) {
    Game->getCore()->getRenderer()->getVideoDriver()->removeTexture(levelIcon);
    levelIcon = (irr::video::ITexture*) NULL;
  }

  return;
}

bool useCameraAsRaySource = false;

void CGUI::checkCrosshairObject()
{
  crosshairObject.visible = false;
  crosshairObject.showCrosshair = true;
  crosshairObject.showHealthbar = true;
  crosshairObject.interactable = false;
  crosshairObject.icons = 0;

  irr::scene::ISceneNode *rayNode = (irr::scene::ISceneNode *) NULL;

  if(Game->getCurrentPlayerWeapon() == NULL || useCameraAsRaySource)
  {
    if(Game->getCore()->getCamera()->getNode())
    {
      rayNode = Game->getCore()->getCamera()->getNode();
    }
  }
  else
  {
    if(Game->getCurrentPlayerWeapon()->isReloading()
    || Game->getCore()->getCamera()->getObjectMovingDirection() != 0)
    {
      crosshairObject.showCrosshair = false;
      return;
    }
    else
    {
      rayNode = Game->getCurrentPlayerWeapon()->getNode();
    }
  }

  if(rayNode == NULL) return;

  irr::core::vector3df direction = irr::core::vector3df(0.f, 0.f, 1000.f);
  irr::core::matrix4 rotationMatrix;
  rotationMatrix.setRotationDegrees(rayNode->getAbsoluteTransformation().getRotationDegrees());

  rotationMatrix.transformVect(direction);

  engine::physics::SRayCastParameters crosshairRay;

  crosshairRay.excluded.push_back(
    Game->getCharacters()->getPlayer()->getBody()->PhysicsBody->getShapeID());

  crosshairRay.line.start = rayNode->getAbsolutePosition();
  crosshairRay.line.end = crosshairRay.line.start + direction;

  engine::physics::SRayCastResult rayResult = Game->getCore()->getPhysics()->getRayCollision(crosshairRay);

#ifdef PHYSICS_NEWTON

  crosshairObject.hit_position = crosshairRay.line.end;

  if(rayResult.body != NULL)
  {
    game::SObjectData *obj_data = (game::SObjectData*)rayResult.body->getUserData();

    irr::u32 container_id = obj_data->container_id;
    irr::u32 element_id = obj_data->element_id;

    crosshairObject.type = obj_data->type;

    useCameraAsRaySource = false;

    crosshairObject.elementNumber = container_id;

    if(crosshairObject.type == EOT_BUILDING)
    {
      crosshairObject.name = obj_data->name;
      crosshairObject.healthLevel = 1.0f;
      crosshairObject.visible = true;
    }
    else if(crosshairObject.type == EOT_INTERACTABLE)
    {
      crosshairObject.name = obj_data->name;
        //Game->getCore()->getObjects()->getInteractableById(container_id)->getMergedItem(0)->name;

      crosshairObject.showHealthbar = false;
      crosshairObject.visible = true;

      irr::core::vector3df viewPosition;
      viewPosition.set(crosshairRay.line.start.X, 0, crosshairRay.line.start.Z);

      irr::core::vector3df position =
        Game->getCore()->getObjects()->getInteractableById(container_id)->getBody()->getOriginalPosition();

      position.Y = 0;

      if(irr::u32(viewPosition.getDistanceFrom(position)) <= 3) {
        crosshairObject.icons |= ECI_HAND;
        useCameraAsRaySource = true;
        crosshairObject.interactable = true;
      }
    }

    crosshairObject.hit_position = rayResult.position;
  }

  crosshairObject.distance = crosshairRay.line.start.getDistanceFrom(crosshairObject.hit_position);
  Game->getCore()->getCamera()->setDistanceToWall(crosshairObject.distance);

  if(crosshairObject.icons == 0 && Game->getCore()->getCamera()->getObjectMovingDirection() != 0)
  {
    Game->getCore()->getCamera()->pushObjectForward(true);
  }
  else if(crosshairObject.icons != 0 && Game->getCore()->getCamera()->getObjectMovingDirection() == 0)
  {
    Game->getCore()->getCamera()->pullObjectBack(false, true);
  }

#endif
}
