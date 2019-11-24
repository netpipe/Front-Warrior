#include "Game.h"
#include "Terminal.h"
#include "Menu.h"
#include "GUI.h"
#include "GameInput.h"

#include "Renderer.h"
#include "ObjectManager.h"

using namespace game;

using namespace irr;
using namespace irr::gui;
using namespace irr::video;
using namespace irr::core;

CTerminal::CTerminal(CGUI * gui, CGame * game) : Game(game), GUI(gui)
{
  reset();
}

CTerminal::~CTerminal()
{

}

void CTerminal::reset()
{
  b_Active = false;
  b_Draw = false;
}

void CTerminal::setActive(bool active)
{
  if(b_Active == active)
    return;

  if(active)
    GUI->fade(EGFI_TERMINAL_ACTIVATE);
  else
    GUI->fade(EGFI_TERMINAL_DEACTIVATE);

  b_Active = active;
  b_Draw = active;


  if(b_Active)
  {
    Game->getInput()->setCursorTexture("data/2d/cursor.png");
    Game->getInput()->setCursorVisible(true);

    setScreen(ETST_MAIN);

    Game->getCore()->getObjects()->setAmbientSoundsPaused(true);
  }
  else
  {
    Game->getInput()->setCursorPosition(GUI->screenSize/2);
    Game->getInput()->setCursorVisible(false);

    Game->getCore()->getRenderer()->getGUI()->clear();

    Game->getCore()->getObjects()->setAmbientSoundsPaused(false);
  }
}

void CTerminal::setScreen(E_TERMINAL_SCREEN_TYPE type)
{
  Game->getCore()->getRenderer()->getGUI()->clear();

  irr::u32 buttonX = 50;
  irr::u32 buttonY = 80;

  irr::u16 exitBtnX = GUI->screenSize.Width - 150;
  irr::u16 exitBtnY = GUI->screenSize.Height - 150;

  IGUIButton* ExitButton = Game->getCore()->getRenderer()->getGUI()->addButton(
      rect<s32>(exitBtnX, exitBtnY, exitBtnX+128, exitBtnY+128), 0, TERMINAL_EXIT_BUTTON);
  ExitButton->setImage(GUI->textures[TEX_TERMINAL_EXIT_BUTTON]);
  ExitButton->setDrawBorder(false);
  ExitButton->setUseAlphaChannel(true);

  if(type == ETST_MAIN)
  {
    IGUIButton* RefillButton = Game->getCore()->getRenderer()->getGUI()->addButton(
        rect<s32>(buttonX, buttonY, buttonX+128, buttonY+128), 0, TERMINAL_REFILL_BUTTON);
    RefillButton->setImage(GUI->textures[TEX_TERMINAL_REFILL_ICON]);
    RefillButton->setDrawBorder(false);
    RefillButton->setUseAlphaChannel(true);

    IGUIStaticText *RefillText = Game->getCore()->getRenderer()->getGUI()->addStaticText(
      L"Repletes your ammo and health.",
      rect<s32>(buttonX+145, buttonY+20, buttonX+500, buttonY+90));
    RefillText->setOverrideFont(GUI->fonts[0]);
    RefillText->setOverrideColor(SColor(255, 255,255,255));

    buttonY += 140;

    IGUIButton* ChangeTeamButton = Game->getCore()->getRenderer()->getGUI()->addButton(
        rect<s32>(buttonX, buttonY, buttonX+128, buttonY+128), 0, TERMINAL_CHANGE_TEAM_BUTTON);
    ChangeTeamButton->setImage(GUI->textures[TEX_TERMINAL_CHANGE_TEAM_ICON]);
    ChangeTeamButton->setDrawBorder(false);
    ChangeTeamButton->setUseAlphaChannel(true);

    IGUIStaticText *ChangeTeamText = Game->getCore()->getRenderer()->getGUI()->addStaticText(
      L"Start playing for the opposite team. Score and other stats will be reset.",
      rect<s32>(buttonX+145, buttonY+20, buttonX+500, buttonY+90));
    ChangeTeamText->setOverrideFont(GUI->fonts[0]);
    ChangeTeamText->setOverrideColor(SColor(255, 255,255,255));

  }

}

void CTerminal::update()
{
  if(b_Draw == false)
    return;

  irr::video::IVideoDriver *VDriver = Game->getCore()->getRenderer()->getVideoDriver();

  irr::core::dimension2d<irr::u16> windowSize = GUI->screenSize;

  u32 sw = windowSize.Width;
  u32 sh = windowSize.Height;

  VDriver->draw2DImage(
    GUI->textures[TEX_TERMINAL_BACKGROUND],
    rect<s32>(0,0, windowSize.Width, windowSize.Height),
    rect<s32>(0,0,1024,1024), 0, 0, true);

  return;
}
