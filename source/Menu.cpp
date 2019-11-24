#include "Game.h"
#include "Menu.h"
#include "GUI.h"
#include "GameInput.h"

#include "Renderer.h"
#include "ObjectManager.h"
#include "SoundManager.h"

using namespace game;

using namespace irr;
using namespace irr::gui;
using namespace irr::video;
using namespace irr::core;

CMenu::CMenu(CGUI * gui, CGame * game) : GUI(gui), Game(game)
{
  bActive = false;

  data = new SMainMenuData();

#ifdef SOUND_CAUDIO
  backgroundMusic = (cAudio::IAudioSource*)NULL;
#endif

#ifdef SOUND_IRRKLANG
  backgroundMusic = (irrklang::ISound*)NULL;
#endif

  skirmishLevels.set_used(0);

  skirmishLevels.push_back("test");
  skirmishLevels.push_back("industrial");
  skirmishLevels.push_back("hill");
  skirmishLevels.push_back("temp");
  skirmishLevels.push_back("g_m1");

  currentlySelectedMenuID = EMI_UNDEFINED;
}

void CMenu::setActive(bool act)
{
  bActive = act;

  if(!bActive)
  {
    Game->getCore()->getRenderer()->getGUI()->clear();
    GUI->clear();
    setType(EMMT_UNDEFINED);

    // Hide and center the cursor
    Game->getInput()->setCursorPosition(
      irr::core::vector2di(
      GUI->screenSize.Width / 2,
      GUI->screenSize.Height / 2));
    Game->getInput()->setCursorVisible(false);
  }
}

void CMenu::playMusic(
  bool play,
  irr::core::stringc musicfile)
{
#ifdef SOUND_IRRKLANG

  if(!play)
  {
    if(backgroundMusic)
    {
      backgroundMusic->stop();
      backgroundMusic = (irrklang::ISound *) NULL;
    }
  }

  if(play)
  {
    playMusic(false);

    backgroundMusic = Game->getCore()->getSound()->getSound(musicfile.c_str());

    backgroundMusic->setVolume(0.76f);
    backgroundMusic->setIsLooped(true);
    backgroundMusic->setIsPaused(false);
  }
#endif

#ifdef SOUND_CAUDIO
  if(!play)
  {
    if(backgroundMusic)
    {
      backgroundMusic->stop();
      backgroundMusic = (cAudio::IAudioSource*) NULL;
    }
  }

  if(play)
  {
    // Stop any music that's playing
    playMusic(false);

    backgroundMusic = Game->getCore()->getSound()->getSoundResource(
      musicfile.c_str());

    backgroundMusic->setVolume(0.78f);
    backgroundMusic->play2d(true); // looped
  }

#endif

}

void CMenu::unloadAssets()
{
  for(u32 i=0; i < data->texture.size(); ++i)
  {
    data->texture[i]->drop();
    //driver->removeTexture(data.texture[i]);
  }

  data->texture.clear();
  data->texture.set_used(0);
}

void CMenu::loadAssets()
{
  IVideoDriver* driver = Game->getCore()->getRenderer()->getVideoDriver();
  IGUIEnvironment* guienv = Game->getCore()->getRenderer()->getGUI();

  // Save flag
  bool Previous32BitState = driver->getTextureCreationFlag(ETCF_ALWAYS_32_BIT);

  // Force some texture settings
  driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
  driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);

  data->texture.push_back( driver->getTexture("data/2d/menu/mainmenu.jpg") ); // 0
  data->texture.push_back( driver->getTexture("data/2d/menu/mainmenubuttons.png") );
  data->texture.push_back( driver->getTexture("data/2d/menu/bottom1.png") );
  data->texture.push_back( driver->getTexture("data/2d/menu/bottom2.png") );
  data->texture.push_back( driver->getTexture("data/2d/menu/goverment.png") );
  data->texture.push_back( driver->getTexture("data/2d/menu/nova.png") ); // 5
  data->texture.push_back( driver->getTexture("data/2d/menu/mainmenubuttons2.png") );
  data->texture.push_back( driver->getTexture("data/2d/menu/armsrace.png") );
  data->texture.push_back( driver->getTexture("data/2d/menu/goverment128.png") );
  data->texture.push_back( driver->getTexture("data/2d/menu/nova128.png") );
  data->texture.push_back( driver->getTexture("data/2d/menu/top.png") ); // 10
  data->texture.push_back( driver->getTexture("data/2d/menu/mainmenubuttons3.png") );
  data->texture.push_back( driver->getTexture("data/2d/menu/goverment128_grayed.png") );
  data->texture.push_back( driver->getTexture("data/2d/menu/nova128_grayed.png") );
  data->texture.push_back( driver->getTexture("data/2d/menu/thumbnailmissing.png") );
  data->texture.push_back( driver->getTexture("data/2d/menu/arrow_right.png") ); // 15
  data->texture.push_back( driver->getTexture("data/2d/menu/arrow_left.png") );
  data->texture.push_back( driver->getTexture("data/2d/menu/go.png") );

  for(u32 i=0; i < data->texture.size(); ++i)
    data->texture[i]->grab();

  // Restore previous state of the flags
  driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, Previous32BitState);
  driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

  IGUIFont *menuFont = GUI->getFont(2);

  guienv->getSkin()->setFont(menuFont, EGDF_BUTTON);
  guienv->getSkin()->setFont(menuFont, EGDF_WINDOW);
  guienv->getSkin()->setFont(menuFont, EGDF_MENU);
  guienv->getSkin()->setFont(menuFont, EGDF_TOOLTIP);
  guienv->getSkin()->setFont(menuFont, EGDF_DEFAULT);

  /*for (u32 i=0; i<EGDC_COUNT ; ++i)
  {
    SColor col =guienv->getSkin()->getColor((EGUI_DEFAULT_COLOR)i);
    col.setAlpha(255);
    guienv->getSkin()->setColor((EGUI_DEFAULT_COLOR)i, col);
  }*/

  // Combo box active item background
  guienv->getSkin()->setColor(EGDC_HIGH_LIGHT, SColor(255, 210,221,113));

  // Combo box active item text
  guienv->getSkin()->setColor(EGDC_HIGH_LIGHT_TEXT, SColor(255, 34,72,128));

  guienv->getSkin()->setColor(EGDC_3D_HIGH_LIGHT, SColor(255, 181,188,176));

  return;
}


void CMenu::update()
{
  if(!bActive) return;

  irr::video::IVideoDriver *VDriver = Game->getCore()->getRenderer()->getVideoDriver();

  irr::core::dimension2d<irr::u16> windowSize = GUI->screenSize;

  u32 sw = windowSize.Width;
  u32 sh = windowSize.Height;

  // Draw background image
  if(m_Type == EMMT_DEFAULT)
  {
    VDriver->draw2DImage(
      data->texture[0],
      rect<s32>(0,0, windowSize.Width, windowSize.Height),
      rect<s32>(0,0,1024,1024), 0, 0, true);

    ITexture* bottom_tex = (currentlySelectedMenuID == EMI_MAIN_MENU) ?
      data->texture[2] : data->texture[3];

    VDriver->draw2DImage(
      bottom_tex,
      rect<s32>(sw-1024,sh-64,sw,sh),
      rect<s32>(0,0, 1024, 64), 0, 0, true);

    VDriver->draw2DImage(
      data->texture[10],
      rect<s32>(0,0,1024,64),
      rect<s32>(0,0, 1024, 64), 0, 0, true);
  }
  else if(m_Type == EMMT_IN_GAME)
  {
    VDriver->draw2DRectangle(
      irr::video::SColor(128, 8,8,8),
      irr::core::rect<irr::s32>(0, 0, sw, sh));
  }




  irr::s32 btnx = sw-266, btny = (sh/2)-128;

  if(Game->getInput()->isLeftButtonClicked() == true)
  {
      // Return to main menu
      if(Game->getInput()->isCursorInRect(rect<s32>(sw-250,sh-26,sw, sh)))
      {
          if(currentlySelectedMenuID != EMI_MAIN_MENU && m_Type == EMMT_DEFAULT)
          {
            SEvent event;
            event.EventType = EET_USER_EVENT;
            event.UserEvent.UserData1 = EMI_MAIN_MENU;
            Game->getCore()->getRenderer()->getDevice()->postEventFromUser( event );
          }
      }
      // ITEM 1
      else if(Game->getInput()->isCursorInRect(rect<s32>(btnx,btny+0,btnx+245, btny+36)))
      {
          if(currentlySelectedMenuID == EMI_MAIN_MENU && m_Type == EMMT_DEFAULT)
          {
            SEvent event;
            event.EventType = EET_USER_EVENT;
            event.UserEvent.UserData1 = EMI_SINGLE_PLAYER;
            Game->getCore()->getRenderer()->getDevice()->postEventFromUser( event );
          }
          else if(currentlySelectedMenuID == EMI_SINGLE_PLAYER && m_Type == EMMT_DEFAULT)
          {
            SEvent event;
            event.EventType = EET_USER_EVENT;

            if(tabLevel[EMI_SINGLE_PLAYER] == 1) {
              event.UserEvent.UserData1 = EMI_SINGLE_PLAYER_CAMPAIGN;
            }

            Game->getCore()->getRenderer()->getDevice()->postEventFromUser( event );
          }
          else if(currentlySelectedMenuID == EMI_IN_GAME_MENU && m_Type == EMMT_IN_GAME)
          {
            Game->getCore()->getSound()->playSound2D("data/sounds/menu/filter.wav", false, 0.95f);

            setActive(false);

#ifdef SOUND_CAUDIO
            Game->getCore()->getObjects()->getAmbientSound()->play();
#endif

            playMusic(false);
          }


      }
      // ITEM 2
      else if(Game->getInput()->isCursorInRect(rect<s32>(btnx,btny+44,btnx+245, btny+80)))
      {
          if(currentlySelectedMenuID == EMI_MAIN_MENU && m_Type == EMMT_DEFAULT)
          {
            /*SEvent event;
            event.EventType = EET_USER_EVENT;
            event.UserEvent.UserData1 = EMI_LOADGAME;
            Core->getRenderer()->getDevice()->postEventFromUser( event );

            screenFader = Core->getRenderer()->getGUI()->addInOutFader();
            screenFader->setColor(video::SColor(0, 0,0,0));
            screenFader->fadeIn(320);*/
          }
          else if(currentlySelectedMenuID == EMI_SINGLE_PLAYER && m_Type == EMMT_DEFAULT)
          {
            SEvent event;
            event.EventType = EET_USER_EVENT;

            if(tabLevel[EMI_SINGLE_PLAYER] == 1) {
              event.UserEvent.UserData1 = EMI_SINGLE_PLAYER_SKIRMISH;
            }

            Game->getCore()->getRenderer()->getDevice()->postEventFromUser( event );
          }
      }
      // Options
      else if(Game->getInput()->isCursorInRect(rect<s32>(btnx,btny+89,btnx+245, btny+124)))
      {
          if(currentlySelectedMenuID == EMI_MAIN_MENU && m_Type == EMMT_DEFAULT)
          {
            SEvent event;
            event.EventType = EET_USER_EVENT;
            event.UserEvent.UserData1 = EMI_OPTIONS;
            Game->getCore()->getRenderer()->getDevice()->postEventFromUser( event );
          }
      }
      // Exit
      else if(Game->getInput()->isCursorInRect(rect<s32>(btnx,btny+132,btnx+245, btny+168)))
      {
          if(currentlySelectedMenuID == EMI_MAIN_MENU && m_Type == EMMT_DEFAULT)
          {
            SEvent event;
            event.EventType = EET_USER_EVENT;
            event.UserEvent.UserData1 = EMI_EXIT;
            Game->getCore()->getRenderer()->getDevice()->postEventFromUser( event );

            Game->getCore()->getSound()->playSound2D("data/sounds/menu/filter.wav", false, 0.9f);
          }
      }
      // ITEM 6
      else if(Game->getInput()->isCursorInRect(rect<s32>(btnx,btny+220,btnx+245, btny+255)))
      {
        if(currentlySelectedMenuID == EMI_IN_GAME_MENU && m_Type == EMMT_IN_GAME)
        {
          Game->returnToMainMenu();
          Game->getCore()->getSound()->playSound2D("data/sounds/menu/filter.wav", false, 0.9f);
        }
      }
  }

  return;
}

void CMenu::changeTo(EMENU_IDENTIFIERS NewTab, bool update)
{
  // Already at selected level
  if(currentlySelectedMenuID == NewTab && bActive) return;

  currentlySelectedMenuID = NewTab;

  if(update)
    refresh();
}

void CMenu::setMenuLevel(EMENU_IDENTIFIERS menu, irr::u32 level)
{
  tabLevel[menu] = level;

  refresh();
}

void CMenu::skirmish_selectTeam(E_TEAMS team)
{
  irr::gui::IGUIElement *root = Game->getCore()->getRenderer()->getGUI()->getRootGUIElement();

  irr::gui::IGUIButton *team1button =
    (irr::gui::IGUIButton *)root->getElementFromId(
      MM_SINGLE_PL_SIDE_GOVERMENT_BTN, true);

  irr::gui::IGUIButton *team2button =
    (irr::gui::IGUIButton *)root->getElementFromId(
      MM_SINGLE_PL_SIDE_NOVA_BTN, true);

  if(team == E_TEAM1)
  {
    team1button->setImage(data->texture[8]);
    team2button->setImage(data->texture[13]);
  }
  else if(team == E_TEAM2)
  {
    team2button->setImage(data->texture[9]);
    team1button->setImage(data->texture[12]);
  }

  data->skirmish_team = team;
}

void CMenu::skirmish_selectLevel(irr::s16 next)
{
  irr::gui::IGUIElement *root = Game->getCore()->getRenderer()->getGUI()->getRootGUIElement();

  irr::gui::IGUIImage *levelThumbnail =
    (irr::gui::IGUIImage *)root->getElementFromId(
      MM_SINGLE_PL_SKIRMISH_LEVEL_THUMBNAIL_IMAGE, true);

  irr::gui::IGUIStaticText *levelNameText =
    (irr::gui::IGUIStaticText *)root->getElementFromId(
      MM_SINGLE_PL_SKIRMISH_LEVEL_NAME_TEXT, true);

  data->skirmish_level_index += next;

  if(data->skirmish_level_index < 0)
    data->skirmish_level_index = skirmishLevels.size()-1;
  else if(data->skirmish_level_index >= skirmishLevels.size())
    data->skirmish_level_index = 0;

  if(levelNameText)
  {
    irr::core::stringw levelNameString = L"Name: ";
    levelNameString += skirmishLevels[data->skirmish_level_index];
    levelNameText->setText(levelNameString.c_str());
  }

  if(levelThumbnail)
  {
    // Get the small.jpg from the level folder
    irr::core::stringc thumbnailPath = "data/levels/";
    thumbnailPath += skirmishLevels[data->skirmish_level_index];
    thumbnailPath += "/small.jpg";

    irr::video::ITexture *thumbTexture =
      Game->getCore()->getRenderer()->getVideoDriver()->getTexture(thumbnailPath.c_str());

    if(thumbTexture) {
      levelThumbnail->setImage(thumbTexture);
    }
    // If texture is not found, display the default one
    else {
      levelThumbnail->setImage(data->texture[14]);
    }

    levelThumbnail->setUseAlphaChannel(true);
  }
}

bool firstLoad = false;

void CMenu::refresh()
{
  /*if(Game->getCore()->getRenderer()->getGUI())
  {
    screenFader = Game->getCore()->getRenderer()->getGUI()->addInOutFader();
    screenFader->setColor(video::SColor(0, 0,0,0));
    screenFader->fadeIn(320);
  }*/

  if(firstLoad)
    Game->getCore()->getSound()->playSound2D("data/sounds/menu/filter.wav", false, 0.9f);

  firstLoad = true;

  //SaveTabState(CurrentTabId);
  Game->getCore()->getRenderer()->getGUI()->clear();

  s32 ButtonX, ButtonY;

  irr::core::dimension2d<irr::u16> windowSize = GUI->screenSize;

  u32 sw = windowSize.Width;
  u32 sh = windowSize.Height;

    switch(currentlySelectedMenuID)
    {
        case EMI_MAIN_MENU:
        {
            Game->getCore()->getRenderer()->getGUI()->addImage(
                data->texture[1],
                position2d<s32>(sw-266, (sh/2)-128),
                true);


        };
        break;

        case EMI_MULTIPLAYER:
        {
            /*ButtonX = s32(System->GameSettings.screenSize.Width * 0.0830f);
            ButtonY = s32(System->GameSettings.screenSize.Height * 0.0858f);

            s32 ButtonX2 = System->GameSettings.screenSize.Width - ButtonX;
            s32 ButtonY2 = System->GameSettings.screenSize.Height - ButtonY-30;

            IGUIComboBox *ServerSource = System->getGUI()->addComboBox(
                rect<s32>(ButtonX, ButtonY, ButtonX+100, ButtonY+20), 0, MM_MULTIPLAYER_SERVER_SOURCE_SELECT);
            ServerSource->addItem(L"LAN");
            ServerSource->addItem(L"Internet");

            ButtonY += 30;

            u32 TableWidth = ButtonX2 - ButtonX;

            IGUITable* ServerTable = System->getGUI()->addTable(
            rect<s32>(ButtonX, ButtonY, ButtonX2, ButtonY2),
            0, MM_MULTIPLAYER_SERVERS_TABLE, true);
            ServerTable->addColumn(L"Server name");
            ServerTable->addColumn(L"Clients");
            ServerTable->addColumn(L"Map");
            ServerTable->addColumn(L"Type");
            ServerTable->setColumnWidth(0, u32(TableWidth*0.6));
            ServerTable->setColumnWidth(1, u32(TableWidth*0.1));
            ServerTable->setColumnWidth(2, u32(TableWidth*0.15));
            ServerTable->setColumnWidth(3, u32(TableWidth*0.15));*/
        }
        break;

        case EMI_SINGLE_PLAYER:
        {
            // Game type selection
            if(tabLevel[EMI_SINGLE_PLAYER] == 1)
            {
              Game->getCore()->getRenderer()->getGUI()->addImage(
                  data->texture[6],
                  position2d<s32>(sw-266, (sh/2)-128),
                  true);
            }
            else if(tabLevel[EMI_SINGLE_PLAYER] == 2)
            {
                ButtonX = (windowSize.Width/2)-32;
                ButtonY = (windowSize.Height/2);

                IGUIButton* Team1Button = Game->getCore()->getRenderer()->getGUI()->addButton(
                    rect<s32>(ButtonX-256, ButtonY-128, ButtonX, ButtonY+128), 0, MM_SINGLE_PL_SIDE_GOVERMENT_BTN);
                Team1Button->setImage(data->texture[4]);
                Team1Button->setDrawBorder(false);
                Team1Button->setUseAlphaChannel(true);

                ButtonX = (windowSize.Width/2)+32;

                IGUIButton* Team2Button =Game->getCore()->getRenderer()->getGUI()->addButton(
                    rect<s32>(ButtonX, ButtonY-128, ButtonX+256, ButtonY+128), 0, MM_SINGLE_PL_SIDE_NOVA_BTN);
                Team2Button->setImage(data->texture[5]);
                Team2Button->setDrawBorder(false);
                Team2Button->setUseAlphaChannel(true);

            }
            // Mission selection screen
            else if(tabLevel[EMI_SINGLE_PLAYER] == 3)
            {
                IGUIListBox *list = Game->getCore()->getRenderer()->getGUI()->addListBox(
                    rect<s32>(50,64,300,sh-128),
                    0,MM_SINGLE_PL_MISSION_SELECT, true);

                missionLevels.set_used(0);

                list->addItem(L"Training");
                missionLevels.push_back("training.xml");

                list->addItem(L"Mission 1 - New sherif in town");
                missionLevels.push_back("mission1.xml");

                list->setSelected(0);

                IGUIButton* StartButton = Game->getCore()->getRenderer()->getGUI()->addButton(
                    rect<s32>(50, sh-110, sw-50, sh-70), 0, MM_SINGLE_PL_START_GAME_BTN,
                    L"Play selected mission");

                /*System->getGUI()->addImage(
                    Data.Texture[Data_SinglePlayer.Team + 4],
                    position2d<s32>(sw-300, 48),
                    true);*/

                IGUIStaticText * MissionText = Game->getCore()->getRenderer()->getGUI()->addStaticText(
                    L"",
                    rect<s32>(310,sh-256,sw-50,sh-128),
                    false, true,
                    0, MM_SINGLE_PL_MISSION_BRIEFING_TEXT);

            }
            else if(tabLevel[EMI_SINGLE_PLAYER] == 4)
            {
              /*IGUIImage* GameTypeImage = Core->getRenderer()->getGUI()->addImage(rect<s32>(50,70,50+128,70+128));
              GameTypeImage->setImage(data.texture[7]);
              GameTypeImage->setUseAlphaChannel(true);

              Core->getRenderer()->getGUI()->addStaticText(L"Game mode", rect<s32>(190, 90, 300, 110),0,1);

              IGUIComboBox *GameTypeSelect = Core->getRenderer()->getGUI()->addComboBox(
                  rect<s32>(190, 115, 400, 140), 0, MM_SINGLE_PL_SKIRMISH_GAME_TYPE_SELECT);
              GameTypeSelect->addItem(L"Arms race");
              GameTypeSelect->addItem(L"Infiltration");*/


              // 756 - 50
              // w: 706

              irr::s32 centerX = irr::s32(sw - 706)/2;
              irr::s32 offsetY = irr::s32(sh-600)/6;


              Game->getCore()->getRenderer()->getGUI()->addStaticText(
                L"Faction",
                rect<s32>(centerX + 37, offsetY+70, centerX + 168, offsetY+86), 0,1);

              IGUIButton* Team1Button = Game->getCore()->getRenderer()->getGUI()->addButton(
                  rect<s32>(centerX, offsetY+100, centerX + 128,offsetY+228), 0, MM_SINGLE_PL_SIDE_GOVERMENT_BTN);
              Team1Button->setImage(data->texture[8]);
              Team1Button->setDrawBorder(false);
              Team1Button->setUseAlphaChannel(true);

              IGUIButton* Team2Button = Game->getCore()->getRenderer()->getGUI()->addButton(
                  rect<s32>(centerX,offsetY+240, centerX + 128,offsetY+368), 0, MM_SINGLE_PL_SIDE_NOVA_BTN);
              Team2Button->setImage(data->texture[9]);
              Team2Button->setDrawBorder(false);
              Team2Button->setUseAlphaChannel(true);

              Game->getCore()->getRenderer()->getGUI()->addStaticText(
                L"Level",
                rect<s32>(centerX + 558, offsetY+65, centerX + 620, offsetY+80), 0,1);

              // Left arrow (arrow images are 64x32)

              IGUIButton* LeftButton = Game->getCore()->getRenderer()->getGUI()->addButton(
                  rect<s32>(centerX + 456,offsetY+60, centerX + 520,offsetY+92), 0, MM_SINGLE_PL_SKIRMISH_LEVEL_PREV_BUTTON);
              LeftButton->setImage(data->texture[16]);
              LeftButton->setDrawBorder(false);
              LeftButton->setUseAlphaChannel(true);

              // Right arrow

              IGUIButton* RightButton = Game->getCore()->getRenderer()->getGUI()->addButton(
                  rect<s32>(centerX + 636,offsetY+60, centerX + 700,offsetY+92), 0, MM_SINGLE_PL_SKIRMISH_LEVEL_NEXT_BUTTON);
              RightButton->setImage(data->texture[15]);
              RightButton->setDrawBorder(false);
              RightButton->setUseAlphaChannel(true);

              // Level image

              Game->getCore()->getRenderer()->getGUI()->addImage(
                  data->texture[14],
                  position2d<s32>(centerX+450, offsetY+100), true, 0,
                  MM_SINGLE_PL_SKIRMISH_LEVEL_THUMBNAIL_IMAGE);

              // Level name (file name currently, not the real name)

              Game->getCore()->getRenderer()->getGUI()->addStaticText(
                L"name",
                rect<s32>(centerX + 450, offsetY+365, centerX + 700, offsetY+385),
                false, true, 0,MM_SINGLE_PL_SKIRMISH_LEVEL_NAME_TEXT);

              // Game type selection

              Game->getCore()->getRenderer()->getGUI()->addStaticText(
                L"Game type",
                rect<s32>(centerX + 180, offsetY+70, centerX + 350, offsetY+90), 0,1);

              IGUIComboBox *GameTypeSelect = Game->getCore()->getRenderer()->getGUI()->addComboBox(
                  rect<s32>(centerX + 180, offsetY+95, centerX + 370, offsetY+120), 0,
                  MM_SINGLE_PL_SKIRMISH_GAME_TYPE_SELECT);

              GameTypeSelect->addItem(L"Annihilation");
              GameTypeSelect->addItem(L"Capture the flag");

              // Victory condition selection

              Game->getCore()->getRenderer()->getGUI()->addStaticText(
                L"Victory condition",
                rect<s32>(centerX + 180, offsetY+140, centerX + 350, offsetY+160), 0,1);

              IGUIComboBox *VictoryCondSelect = Game->getCore()->getRenderer()->getGUI()->addComboBox(
                  rect<s32>(centerX + 180, offsetY+165, centerX + 370, offsetY+190), 0,
                  MM_SINGLE_PL_SKIRMISH_VICTORY_CONDITION_SELECT);

              VictoryCondSelect->addItem(L"10 flags captured");
              VictoryCondSelect->addItem(L"15 flags captured");
              VictoryCondSelect->addItem(L"30 flags captured");
              VictoryCondSelect->addItem(L"45 flags captured");
              VictoryCondSelect->addItem(L"50 flags captured");

              // Time limit selection

              Game->getCore()->getRenderer()->getGUI()->addStaticText(
                L"Time limit",
                rect<s32>(centerX+180, offsetY+210, centerX+350, offsetY+230), 0,1);

              IGUIComboBox *TimeSelect = Game->getCore()->getRenderer()->getGUI()->addComboBox(
                  rect<s32>(centerX+180, offsetY+235, centerX+310, offsetY+260), 0,
                  MM_SINGLE_PL_SKIRMISH_TIME_SELECT);

              TimeSelect->addItem(L"No time limit");
              TimeSelect->addItem(L"0h 15m");
              TimeSelect->addItem(L"0h 30m");
              TimeSelect->addItem(L"0h 45m");
              TimeSelect->addItem(L"1h 00m");
              TimeSelect->addItem(L"1h 30m");

              TimeSelect->setSelected(3);

              // Team size selection

              Game->getCore()->getRenderer()->getGUI()->addStaticText(
                L"Team size",
                rect<s32>(centerX+180, offsetY+280, centerX+280, offsetY+300), 0,1);

              IGUIComboBox *TeamSizeSelect = Game->getCore()->getRenderer()->getGUI()->addComboBox(
                  rect<s32>(centerX+180, offsetY+305, centerX+260, offsetY+330), 0);

              TeamSizeSelect->addItem(L"3");
              TeamSizeSelect->addItem(L"4");
              TeamSizeSelect->addItem(L"5");
              TeamSizeSelect->addItem(L"6");

              TeamSizeSelect->setSelected(1);

              // Player class selection

              Game->getCore()->getRenderer()->getGUI()->addStaticText(
                L"Player class",
                rect<s32>(centerX+180, offsetY+350, centerX+350, offsetY+370), 0,1);

              IGUIComboBox *PlayerClassSelect = Game->getCore()->getRenderer()->getGUI()->addComboBox(
                  rect<s32>(centerX+180, offsetY+375, centerX+350, offsetY+400), 0);

              PlayerClassSelect->addItem(L"Random selection");
              PlayerClassSelect->addItem(L"Soldier");
              PlayerClassSelect->addItem(L"Heavy soldier");
              PlayerClassSelect->addItem(L"Engineer");
              PlayerClassSelect->addItem(L"Medic");
              PlayerClassSelect->addItem(L"Spy");

              // Intial credit selection

              Game->getCore()->getRenderer()->getGUI()->addStaticText(
                L"Start credit",
                rect<s32>(centerX+180, offsetY+420, centerX+350, offsetY+440), 0,1);

              IGUIComboBox *CreditSelect = Game->getCore()->getRenderer()->getGUI()->addComboBox(
                  rect<s32>(centerX+180, offsetY+445, centerX+260, offsetY+470), 0,
                  MM_SINGLE_PL_SKIRMISH_CREDIT_SELECT);

              CreditSelect->addItem(L"0");
              CreditSelect->addItem(L"100");
              CreditSelect->addItem(L"250");
              CreditSelect->addItem(L"500");
              CreditSelect->addItem(L"1000");

              CreditSelect->setSelected(3);

              //Game->getCore()->getRenderer()->getGUI()->addStaticText(L"Level", rect<s32>(190, 410, 300, 430),0,1);

              /*IGUIComboBox *GameLevelSelect = Game->getCore()->getRenderer()->getGUI()->addComboBox(
                  rect<s32>(190, 435, 400, 460), 0, MM_SINGLE_PL_SKIRMISH_LEVEL_SELECT);

              for(irr::u16 i=0; i<skirmishLevels.size(); ++i)
              {
                GameLevelSelect->addItem(irr::core::stringw(skirmishLevels[i]).c_str());
              }*/

              // position2d<s32>(centerX+450, offsetY+100)

              IGUIButton* StartButton = Game->getCore()->getRenderer()->getGUI()->addButton(
                  rect<s32>(centerX+514, offsetY+430, centerX+514+128, offsetY+494), 0,
                  MM_SINGLE_PL_SKIRMISH_START_GAME_BTN);

              StartButton->setImage(data->texture[17]);
              StartButton->setDrawBorder(false);
              StartButton->setUseAlphaChannel(true);


              skirmish_selectTeam(E_TEAM1);
              skirmish_selectLevel(0);

            }
            else if(tabLevel[EMI_SINGLE_PLAYER] == 3)
            {
                // Player name
                /*ButtonX = s32(System->GameSettings.screenSize.Width * 0.0830f);
                ButtonY = s32(System->GameSettings.screenSize.Height * 0.0858f);

                System->getGUI()->addStaticText(L"Player name",rect<s32>(ButtonX, ButtonY+2, ButtonX+85, ButtonY+25),0,1);
                System->getGUI()->addEditBox(Data_SinglePlayer.Name.c_str(),
                    rect<s32>(ButtonX+85, ButtonY, ButtonX+200, ButtonY+20), true, 0, MM_SINGLE_PL_NAME_TB);

                // Team selection buttons
                ButtonY += 30;

                System->getGUI()->addStaticText(L"Team", rect<s32>(ButtonX, ButtonY+2, ButtonX+85, ButtonY+25),0,1);
                IGUIComboBox *TeamComboBox = System->getGUI()->addComboBox(
                    rect<s32>(ButtonX+85, ButtonY, ButtonX+190, ButtonY+20), 0, MM_SINGLE_PL_TEAM_CB);
                TeamComboBox->addItem(L"Team 1");
                TeamComboBox->addItem(L"Team 2");
                TeamComboBox->setSelected( Data_SinglePlayer.Team );

                // Team selection buttons
                ButtonY += 30;

                System->getGUI()->addStaticText(L"Class", rect<s32>(ButtonX, ButtonY+2, ButtonX+85, ButtonY+25),0,1);
                IGUIComboBox *ClassComboBox = System->getGUI()->addComboBox(
                    rect<s32>(ButtonX+85, ButtonY, ButtonX+210, ButtonY+20), 0, MM_SINGLE_PL_CLASS_CB);
                ClassComboBox->addItem(L"Soldier");
                ClassComboBox->addItem(L"Heavy soldier");
                ClassComboBox->addItem(L"Medic");
                ClassComboBox->addItem(L"Engineer");
                ClassComboBox->addItem(L"Undercover");
                ClassComboBox->setSelected( Data_SinglePlayer.Class );

                #ifdef EARLY_TESTING_RELEASE
                ClassComboBox->setEnabled(false);
                #endif

                // Weapons
                ButtonY += 30;

                System->getGUI()->addStaticText(L"Primary\nweapon", rect<s32>(ButtonX, ButtonY-5, ButtonX+85, ButtonY+50),0,1);

                IGUIComboBox *PrimaryWeaponComboBox = System->getGUI()->addComboBox(
                    rect<s32>(ButtonX+85, ButtonY, ButtonX+260, ButtonY+20), 0, MM_SINGLE_PL_PRIMARY_WPN_CB);
                PrimaryWeaponComboBox->addItem(L"Rocket launcher");
                PrimaryWeaponComboBox->addItem(L"Plasma gun");
                PrimaryWeaponComboBox->setSelected( Data_SinglePlayer.PrimaryWeapon );

                #ifdef EARLY_TESTING_RELEASE
                PrimaryWeaponComboBox->setEnabled(false);
                #endif

                // Level selection
                ButtonY += 30;

                System->getGUI()->addStaticText(L"Level", rect<s32>(ButtonX, ButtonY+2, ButtonX+85, ButtonY+25),0,1);

                IGUIComboBox *LevelComboBox = System->getGUI()->addComboBox(
                    rect<s32>(ButtonX+85, ButtonY, ButtonX+260, ButtonY+20), 0, MM_SINGLE_PL_LEVEL_SELECT);

                // Start game button
                ButtonX = s32(System->GameSettings.screenSize.Width * 0.6487f);
                ButtonY = s32(System->GameSettings.screenSize.Height * 0.8138f);

                IGUIButton* StartButton = System->getGUI()->addButton(
                    rect<s32>(ButtonX, ButtonY, ButtonX+256, ButtonY+64), 0, MM_SINGLE_PL_START_GAME_BTN);
                StartButton->setImage(System->getVideoDriver()->getTexture("data/2d/menu/mainMenuStartGame.png"));
                StartButton->setDrawBorder(false);
                StartButton->setUseAlphaChannel(true); */
            }


        }
        break;

        case EMI_IN_GAME_MENU:
        {
            Game->getCore()->getRenderer()->getGUI()->addImage(
                data->texture[11],
                position2d<s32>(sw-266, (sh/2)-128),
                true);
        };
        break;
    }
    return;
}

