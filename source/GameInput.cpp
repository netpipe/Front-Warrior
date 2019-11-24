#include "Input.h"
#include "GameInput.h"
#include "Game.h"
#include "GUI.h"
#include "Menu.h"
#include "CharacterManager.h"
#include "Player.h"
#include "Terminal.h"

#include "Core.h"
#include "Renderer.h"

using namespace game;

CGameInput::CGameInput(CGame* game)
{
  m_CursorTexture = (irr::video::ITexture*)NULL;

  Key.reset();
  Mouse.reset();

  Game = game;
}

bool CGameInput::OnEvent(const irr::SEvent& event)
{
  if(event.EventType == irr::EET_GUI_EVENT)
  {
    // Delegate GUI events also to game interfaces the user has created
    /*for(irr::u16 i=0; i<interfaces.size(); ++i)
    {
      interfaces[i].OnEvent(event);
    }*/

    irr::gui::IGUIEnvironment *guienv = Game->getCore()->getRenderer()->getGUI();
    irr::gui::IGUIElement *root = guienv->getRootGUIElement();

   	irr::s32 type = event.GUIEvent.EventType;
    irr::s32 id = event.GUIEvent.Caller->getID();

    if(type == irr::gui::EGET_BUTTON_CLICKED)
    {
      if(id == game::MM_SINGLE_PL_SIDE_GOVERMENT_BTN)
      {
        if(Game->getGUI()->getMenu()->getMenuLevel(game::EMI_SINGLE_PLAYER) == 2)
        {
          Game->getGUI()->getMenu()->setMenuLevel(game::EMI_SINGLE_PLAYER, 3);
          Game->getGUI()->getMenu()->refresh();
        }
        else
        {
          Game->getGUI()->getMenu()->skirmish_selectTeam(game::E_TEAM1);
          Game->getCore()->getSound()->playSound2D("data/sounds/menu/select.wav", false, 0.8f);
        }
      }
      else if(id == game::MM_SINGLE_PL_SIDE_NOVA_BTN)
      {
        if(Game->getGUI()->getMenu()->getMenuLevel(game::EMI_SINGLE_PLAYER) == 2)
        {
          Game->getGUI()->getMenu()->setMenuLevel(game::EMI_SINGLE_PLAYER, 3);
          Game->getGUI()->getMenu()->refresh();
        }
        else
        {
          Game->getGUI()->getMenu()->skirmish_selectTeam(game::E_TEAM2);
          Game->getCore()->getSound()->playSound2D("data/sounds/menu/select.wav", false, 0.8f);
        }
      }
      else if(id == game::MM_SINGLE_PL_SKIRMISH_START_GAME_BTN)
      {
        E_GAME_TYPES gtype;
        irr::u16 timeSelection = 0;

        irr::gui::IGUIComboBox *gametypeselection =
          (irr::gui::IGUIComboBox *)root->getElementFromId(
            MM_SINGLE_PL_SKIRMISH_GAME_TYPE_SELECT, true);

        irr::gui::IGUIComboBox *timeselect =
          (irr::gui::IGUIComboBox *)root->getElementFromId(
            MM_SINGLE_PL_SKIRMISH_TIME_SELECT, true);

        irr::gui::IGUIComboBox *creditselect =
          (irr::gui::IGUIComboBox *)root->getElementFromId(
            MM_SINGLE_PL_SKIRMISH_CREDIT_SELECT, true);

        timeSelection = timeselect->getSelected();

        if(gametypeselection->getSelected() == 0)
          gtype = game::EGT_SKIRMISH_ANNIHILATION;
        else if(gametypeselection->getSelected() == 1)
          gtype = game::EGT_SKIRMISH_CAPTURE_FLAG;

        game::SGameParameters gameParams;
        gameParams.Type = gtype;
        gameParams.TeamID = Game->getGUI()->getMenu()->getData()->skirmish_team;
        gameParams.Class = 0;
        gameParams.Skirmish.BotCount = 5;

        irr::core::stringc creditString = creditselect->getItem(creditselect->getSelected());

        gameParams.Skirmish.StartCredit = atoi(creditString.c_str());

        irr::u8 h, m;

        if(timeSelection == 0) {
          h = m = 99;
        } else if(timeSelection == 1) {
          h = 0;
          m = 15;
        } else if(timeSelection == 2) {
          h = 0;
          m = 30;
        } else if(timeSelection == 3) {
          h = 0;
          m = 45;
        } else if(timeSelection == 4) {
          h = 1;
          m = 0;
        } else if(timeSelection == 5) {
          h = 1;
          m = 30;
        }


        gameParams.Skirmish.TimeHours = h;
        gameParams.Skirmish.TimeMinutes = m;

        gameParams.Level = Game->getGUI()->getMenu()->skirmishLevels[Game->getGUI()->getMenu()->getData()->skirmish_level_index];

        Game->getInput()->setCursorVisible(false);

        Game->start(gameParams);
      }
      else if(id == MM_SINGLE_PL_SKIRMISH_LEVEL_NEXT_BUTTON)
      {
        Game->getGUI()->getMenu()->skirmish_selectLevel(1); // +1 next
        Game->getCore()->getSound()->playSound2D("data/sounds/menu/select.wav", false, 0.8f);
      }
      else if(id == MM_SINGLE_PL_SKIRMISH_LEVEL_PREV_BUTTON)
      {
        Game->getGUI()->getMenu()->skirmish_selectLevel(-1); // -1 prev
        Game->getCore()->getSound()->playSound2D("data/sounds/menu/select.wav", false, 0.8f);
      }

      /*
        TERMINAL
      */

      if(id == TERMINAL_REFILL_BUTTON)
      {
        Game->getCore()->getSound()->playSound2D("data/sounds/menu/select.wav", false, 0.8f);
        Game->getCharacters()->getPlayer()->refill();
      }
      if(id == TERMINAL_CHANGE_TEAM_BUTTON)
      {
        Game->getCore()->getSound()->playSound2D("data/sounds/menu/select.wav", false, 0.8f);

        // Switch teams
        Game->getCharacters()->getPlayer()->getParameters()->TeamID =
          (Game->getCharacters()->getPlayer()->getParameters()->TeamID == E_TEAM1) ? E_TEAM2 : E_TEAM1;

        Game->getCharacters()->getPlayer()->getStats()->score = 0;
        Game->getCharacters()->getPlayer()->getStats()->kills = 0;
        Game->getCharacters()->getPlayer()->getStats()->deaths = 0;
        Game->getCharacters()->getPlayer()->getStats()->credit = Game->getParameters().Skirmish.StartCredit;

        // Respawn
        Game->getCharacters()->spawn(Game->getCharacters()->getPlayer());
        Game->getCharacters()->getPlayer()->refill();

        // Close terminal
        Game->getGUI()->getTerminal()->setActive(false);
      }
      else if(id == TERMINAL_EXIT_BUTTON)
      {
        Game->getGUI()->getTerminal()->setActive(false);
      }

    }
  }
  else if(event.EventType == irr::EET_USER_EVENT)
  {
    // Delegate user-definied-events also to game interfaces the user has created
    /*for(irr::u16 i=0; i<interfaces.size(); ++i)
    {
      interfaces[i].OnEvent(event);
    }*/

    switch(event.UserEvent.UserData1)
    {
      case game::EMI_MAIN_MENU:
      {
        Game->getGUI()->getMenu()->changeTo(game::EMI_MAIN_MENU);
      }
      break;

      case game::EMI_SINGLE_PLAYER:
      {
        Game->getGUI()->getMenu()->changeTo(game::EMI_SINGLE_PLAYER, false);
        Game->getGUI()->getMenu()->setMenuLevel(game::EMI_SINGLE_PLAYER, 1);
        //Game->getGUI()->getMenu()->refresh();
      }
      break;

      case game::EMI_SINGLE_PLAYER_CAMPAIGN:
      {
        Game->getGUI()->getMenu()->setMenuLevel(game::EMI_SINGLE_PLAYER, 2);
      }
      break;

      case game::EMI_SINGLE_PLAYER_SKIRMISH:
      {
        Game->getGUI()->getMenu()->setMenuLevel(game::EMI_SINGLE_PLAYER, 4);
      }
      break;

      case game::EMI_LOADGAME:
      {
        Game->getGUI()->getMenu()->changeTo(game::EMI_LOADGAME);
      }
      break;

      case game::EMI_MULTIPLAYER:
      {
         Game->getGUI()->getMenu()->changeTo(game::EMI_MULTIPLAYER);
      }
      break;

      case game::EMI_OPTIONS:
      {
        Game->getGUI()->getMenu()->changeTo(game::EMI_OPTIONS);
      }
      break;

      case game::EMI_EXIT:
        Game->getCore()->requestClose();
      break;
    }
  }
  else if (event.EventType == irr::EET_KEY_INPUT_EVENT)
  {
    Key.KeyCodeInUse[event.KeyInput.Key] = event.KeyInput.PressedDown;

    if(event.KeyInput.Shift)
      Key.KeyCodeInUse[irr::KEY_SHIFT] = true;
    else
      Key.KeyCodeInUse[irr::KEY_SHIFT] = false;

    if(event.KeyInput.Key == irr::KEY_F1)
      Game->returnToMainMenu();

  }
  else if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
  {
    switch(event.MouseInput.Event)
    {
      case irr::EMIE_LMOUSE_PRESSED_DOWN: Mouse.leftButton = true; break;
      case irr::EMIE_RMOUSE_PRESSED_DOWN: Mouse.rightButton = true; break;
      case irr::EMIE_MMOUSE_PRESSED_DOWN: Mouse.middleButton = true; break;
      case irr::EMIE_LMOUSE_LEFT_UP: Mouse.leftButton = false; break;
      case irr::EMIE_RMOUSE_LEFT_UP: Mouse.rightButton = false; break;
      case irr::EMIE_MMOUSE_LEFT_UP: Mouse.middleButton = false; break;
      case irr::EMIE_MOUSE_MOVED:
        Mouse.oldX = Mouse.X;
        Mouse.oldY = Mouse.Y;
        Mouse.X = event.MouseInput.X;
        Mouse.Y = event.MouseInput.Y;
        Mouse.hasMoved = true;
      break;
      case irr::EMIE_MOUSE_WHEEL: Mouse.wheelValue += event.MouseInput.Wheel; break;
      default: break;
    }
  }

  return false;
}


void CGameInput::drawCursor()
{
  Mouse.hasMoved = false;

  if(b_CursorVisible == false || !m_CursorTexture) return;

  irr::core::position2d<irr::s32> mouseP = getCursorPosition();

  Game->getCore()->getRenderer()->getVideoDriver()->draw2DImage(
    m_CursorTexture,
    irr::core::rect<irr::s32>(mouseP.X, mouseP.Y, mouseP.X+32, mouseP.Y+32),
    irr::core::rect<irr::s32>(0,0,31,31), 0, 0, true);

  return;
}

void CGameInput::setCursorTexture(const irr::c8 *texturename)
{
  m_CursorTexture = Game->getCore()->getRenderer()->getVideoDriver()->getTexture(texturename);

  Game->getCore()->getRenderer()->getVideoDriver()->makeColorKeyTexture(
    m_CursorTexture,
    irr::core::position2d<irr::s32>(31,0));

  return;
}

void CGameInput::setCursorPosition(irr::core::position2d<irr::s32> pos)
{
  Game->getCore()->getRenderer()->getDevice()->getCursorControl()->setPosition(pos.X, pos.Y);
}

bool CGameInput::isCursorInRect(irr::core::rect<irr::s32> rectangle)
{
  irr::u32 mX = Mouse.X;
  irr::u32 mY = Mouse.Y;

  if(mX >= rectangle.UpperLeftCorner.X && mY >= rectangle.UpperLeftCorner.Y &&
    mX <= rectangle.LowerRightCorner.X && mY <= rectangle.LowerRightCorner.Y)
    return true;
  else
    return false;
}
