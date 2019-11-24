#include "Game.h"
#include "Player.h"
#include "CharacterManager.h"
#include "GameInput.h"

#include "Renderer.h"
#include "Configuration.h"
#include "Camera.h"
#include "TracedWeapon.h"
#include "Maths.h"

using namespace game;

bool headCenter = true;


CPlayer::CPlayer(CGame* game, engine::SCharacterCreationParameters params) : Game(game)
{
  parameters.TeamID = params.TeamID;
  parameters.Class = params.Class;

  Core = Game->getCore();
}

irr::f32 runRollValue=0.f;
irr::f32 rollAdd = 0.f;

void CPlayer::update()
{
  SCharacterClassParameters *c_class_params = Game->getCharacters()->cClassParameters[parameters.Class];
  irr::f32 time = Core->time.delta;
  irr::IrrlichtDevice *device = Core->getRenderer()->getDevice();
  irr::core::dimension2d<irr::u16> ScreenSize = Core->getConfiguration()->getVideo()->windowSize;

  CGameInput *input = Game->getInput();

  // Player is in air when it's jumping or when it's been falling for longer than
  // 0.15 seconds.
  bool isPlayerInAir =
    (parameters.States & engine::ECS_JUMPING
    || (parameters.States & engine::ECS_FALLING && fallingTime >= 0.15f));

#ifdef PHYSICS_NEWTON

  irr::core::vector3df currentForce = body.PhysicsBody->getForce();
  fallingVelocity.set(currentForce.X, 0, currentForce.Z);

  body.PhysicsBody->setForce(irr::core::vector3df(0,0,0));
  body.PhysicsBody->setVelocity(irr::core::vector3df(0,0,0));
  body.PhysicsBody->setOmega(irr::core::vector3df(0,0,0));
#endif

  //if(Game->isPaused())
    //return;

  //
  // Crouch
  //

  if(input->isKeyPressedOnce(irr::KEY_KEY_C)
  && !(parameters.States & engine::ECS_JUMPING)
  && !(parameters.States & engine::ECS_FALLING))
  {
    // If already crouching, stand up
    if(parameters.States & engine::ECS_CROUCHING)
      stand();
    // If not, crouch
    else
      crouch();
  }

  //
  // Lie down
  //

  if(input->isKeyPressedOnce(irr::KEY_KEY_X)
  && !(parameters.States & engine::ECS_JUMPING)
  && !(parameters.States & engine::ECS_FALLING))
  {
    // If already crouching, stand up
    if(parameters.States & engine::ECS_LYING)
      stand();
    // If not, crouch
    else
      lie();
  }


  //
  // Get direction vector
  //

  irr::core::vector3df dir = irr::core::vector3df(0,0,0);

  if(input->isKeyHeldDown(irr::KEY_KEY_W)) dir.Z = 1;
  else if(input->isKeyHeldDown(irr::KEY_KEY_S)) dir.Z = -1;

  if(input->isKeyHeldDown(irr::KEY_KEY_A)) dir.X = -1;
  else if(input->isKeyHeldDown(irr::KEY_KEY_D)) dir.X = 1;

  engine::CBaseCharacter::checkForStairs(dir);
  engine::CBaseCharacter::update();

  if(irr::s32(body.PhysicsBody->getPosition().Y) < -50)
    Game->getCharacters()->spawn(this);

  //
  // Jump
  //

  if(input->isKeyPressedOnce(irr::KEY_SPACE))
  {
    // If crouching, first stand up
    if(parameters.States & engine::ECS_CROUCHING)
      stand();
    else if(parameters.States & engine::ECS_LYING)
      crouch();
    // Then jump!
    else
      jump();
  }



  //
  // Move the player
  //

  // When jumping or falling, current velocity is slowly faded out
  // So when you run and jump, while in air, you decelerate.
  if(isPlayerInAir)
  {
    move(engine::NULLVECTOR, 0.f, true);
  }
  else
  {
    if(dir != irr::core::vector3df(0,0,0))
    {
      // Check if the player is running
      if((input->isKeyHeldDown(irr::KEY_SHIFT) && parameters.Stamina >= 2.0f)
      && !(parameters.States & engine::ECS_CROUCHING)
      && !(parameters.States & engine::ECS_LYING)
      && !(parameters.States & engine::ECS_ON_STAIRS)
      && !(parameters.States & engine::ECS_FALLING))
      {
        parameters.States |= engine::ECS_RUNNING;

        runSpeedFactor = fmin(runSpeedFactor + (1.15f * time), 1.0);
      }
      else
      {
        runSpeedFactor -= (1.35 + (parameters.States & engine::ECS_ON_STAIRS)*0.45f) * time;

        if(runSpeedFactor < 0.0f)
        {
          runSpeedFactor = 0.0f;
          parameters.States &= ~engine::ECS_RUNNING;
        }
      }

      move(dir, c_class_params->move_speed);
    }
    else
    {
      parameters.States &= ~engine::ECS_MOVING;

      runSpeedFactor = 0.f;
    }
  }

  //
  // Tilt the head to look around corners
  //

  bool aiming = Core->getCamera()->isWeaponAimedCloseUp();

  // We need to subtract the roll we added on previous frame,
  // so it's back where it started from.
  irr::f32 roll = Core->getCamera()->getRoll() - rollAdd;

  if(runSpeedFactor >= 0.03) {
    if(!isPlayerInAir)
      runRollValue += runSpeedFactor * time * (irr::f32(Core->getMath()->getRandomInt(90,103))/10);
  } else {
    runRollValue = 0.f;
  }


  if(input->isKeyHeldDown(irr::KEY_KEY_E) && !aiming)
  {
    roll = fmax(roll - (35 * time), -engine::MAX_HEAD_ROLL);

    headCenter = false;
  }
  else if(input->isKeyHeldDown(irr::KEY_KEY_Q) && !aiming)
  {
    roll = fmin(roll + (35 * time), engine::MAX_HEAD_ROLL);

    headCenter = false;
  }
  else
  {
    if(headCenter == false)
    {
      if(roll > 0)
      {
        roll -= 40 * time;
        if(roll <= 0) { roll = 0; headCenter = true; }
      }
      else if(roll < 0)
      {
        roll += 40 * time;
        if(roll >= 0) { roll = 0; headCenter = true; }
      }
    }
  }

  roll += rollAdd = sin(runRollValue) * 1.36f;

  Core->getCamera()->setRoll(roll);

  //
  // Rotate player by mouse
  //

  irr::core::position2d<irr::s32> mousePosition = device->getCursorControl()->getPosition();
  irr::f32 mouseSens = Core->getConfiguration()->getControls()->mouseSensitivity;

  irr::f32 tilt = Core->getCamera()->getTilt();

  tilt += ((mousePosition.Y - ScreenSize.Height/2) * mouseSens);

  tilt = fmin(tilt, 70-fabs(roll*1.5f)); // looking down
  tilt = fmax(tilt, -(75-fabs(roll*1.5f))); // looking up

  Core->getCamera()->setTilt(tilt);

  irr::f32 rotationLR = ((mousePosition.X - ScreenSize.Width/2) * mouseSens);

#ifdef PHYSICS_NEWTON
  // This is what makes the player body turn left and right
  if(rotationLR != 0)
    body.PhysicsBody->setOmega(irr::core::vector3df(0, rotationLR * 0.55f, 0));
#endif

  device->getCursorControl()->setPosition(ScreenSize.Width/2, ScreenSize.Height/2);


  //
  // Firing/using button
  //

  CInventory *inventory = (game::CInventory*)Game->getCharacters()->getPlayer()->getInventory();
  SInventoryItem selectedItem = inventory->getItem(inventory->getSelectedItem());

  if(input->isKeyPressedOnce(irr::KEY_KEY_R)) {
    if(selectedItem.Type == INV_WEAPON) {
      ((CWeapon*)selectedItem.Data)->reload();
    }
  }

  if(input->isLeftButtonHeld() || input->isLeftButtonClicked())
  {
    if(selectedItem.Type == game::INV_WEAPON && Core->getCamera()->isFPSObjectReady())
    {
      game::CWeapon *weap = (CWeapon*)selectedItem.Data;

      if(weap->getClass() == EWC_TRACED)
        weap = (CTracedWeapon*)weap;

      if(weap->fire())
      {
        Core->getCamera()->setShaking(Game->weaponsParameters[weap->getType()]->recoilForce);
      }
    }
  }
  else
  {
    Core->getCamera()->setShaking(0.f);
  }


  //
  // Close-up aim with right mouse button
  //

  if(input->isRightButtonClicked())
  {
    Core->getCamera()->toggleWeaponCloseUpAim();
  }

}


void CPlayer::fire()
{

}

void CPlayer::refill()
{
  CBaseCharacter::refill();

  //
  // Refill ammo

  CInventory *inv = (CInventory*)inventory;

  for(irr::u16 i=0; i < inv->getSize(); ++i)
  {
    SInventoryItem selectedItem = inv->getItem(i);

    if(selectedItem.Type == INV_WEAPON)
    {
      ((CWeapon*)selectedItem.Data)->refill();
    }
  }

}
