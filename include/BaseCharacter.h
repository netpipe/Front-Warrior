#ifndef BASECHARACTER_HEADER_DEFINED
#define BASECHARACTER_HEADER_DEFINED

#include <irrlicht.h>

#include "Physics.h"

namespace engine {

  /*
    This is the base character class. All other types use this. This class handles all movement.

  */

  class CBaseCharacter
  {
  public:

    CBaseCharacter();

    ~CBaseCharacter();

    /*
      GET methods
    */

    SCharacterRepresentation *getBody() { return &body; }

    SCharacterParameters *getParameters() { return &parameters; }

    SCharacterStats *getStats() { return &stats; }

    irr::core::vector3df getRotation();

    irr::f32 getRunSpeedFactor() { return runSpeedFactor; }

    void *getInventory() { return inventory; }

    //! Get the user-defined ID of this character
    irr::s32 getUserID() { return userID; }

    /*
      SET methods
    */

    //! Set the user ID for this characters. Useful for indentifying class type for casting.
    void setUserID(irr::s32 uid) { userID = uid; }

    void setInventory(void* inv) { inventory = inv; }

    /*
      Other public methods
    */

    virtual void fire()
    {
      // Implement in derived classes
    }

    void init()
    {
      parameters.States = 0;
      runSpeedFactor = 0.f;

      stand();
    }

    virtual void refill()
    {
      parameters.Health = parameters.HealthMax;

      // Rest should be implemented in derived classes
    }

    void hideCollision() {
/*#ifdef PHYSICS_IRR_NEWT
      if(parameters.States & ECS_STANDING)
        collision_restore = StandingCollision;
      else if(parameters.States & ECS_CROUCHING)
        collision_restore = CrouchingCollision;
      else
        collision_restore = LyingCollision;

      body.PhysicsBody->setCollision(EmptyCollision);
#endif*/

    }


    void restoreCollision() {
/*#ifdef PHYSICS_IRR_NEWT
      body.PhysicsBody->setCollision(collision_restore);
#endif*/
    }

    void remove();

    irr::scene::ISceneNode * rotationNode;

  protected:

    CCore * Core;

    //! Main update .. gravity, movement and such
    void update();

    //! Move the character with the given direction vector
    void move(irr::core::vector3df dir, irr::f32 moveSpeed, bool inAir=false, irr::f32 deceleration=.0f);

    void checkForStairs(irr::core::vector3df dir);

    //! Makes the character jump
    void jump();

    //! Make the character crouch
    void crouch();

    //! Character lies down
    void lie();

    //! Stand up
    void stand();

    void *inventory;

    /// Rendered node and physics body
    SCharacterRepresentation body;

    /// Contains all the character parameters
    SCharacterParameters parameters;

    SCharacterStats stats;

    irr::f32 jumpHeight, jumpHeightOnLastFrame;

    irr::core::vector3df fallingVelocity;

    irr::f32 runSpeedFactor;

    irr::f32 gravityAccerelation;

    irr::f32 fallingTime;

    irr::s32 userID;

    irr::f32 floorY;
  };

}

#endif
