#include "Core.h"
#include "BaseCharacter.h"
#include "Physics.h"
#include "Renderer.h" // For drawing 3d lines .. DEBUG

using namespace engine;

#ifdef ENGINE_DEVELOPMENT_MODE
  bool debug_jump = false;
#endif

CBaseCharacter::CBaseCharacter()
{
  runSpeedFactor = floorY = fallingTime = 0.f;

  body.Node = (irr::scene::IAnimatedMeshSceneNode*) NULL;

#ifdef PHYSICS_NEWTON
  body.PhysicsBody = (physics::CBody*)NULL;
#endif

  userID = -1;
}

CBaseCharacter::~CBaseCharacter()
{
  #ifdef PHYSICS_NEWTON
  //body.PhysicsBody->removeBody();
  #endif
}

void CBaseCharacter::remove()
{

}


// Character rotation is calculated from body and parented empty node
// in front of the character.
irr::core::vector3df CBaseCharacter::getRotation()
{
  rotationNode->updateAbsolutePosition();

  irr::core::vector3df heading =
    irr::core::vector3df(rotationNode->getAbsolutePosition() -
      body.Node->getAbsolutePosition()).getHorizontalAngle();

  return heading;
}


void CBaseCharacter::update()
{
/*#ifdef ENGINE_DEVELOPMENT_MODE
    if(Core->GetInput()->isKeyPressedOnce(irr::KEY_KEY_M)) debug_jump = !debug_jump;
#endif*/

#ifdef PHYSICS_NEWTON

  irr::core::vector3df currentPosition = body.PhysicsBody->getPosition();

  irr::core::vector3df gravity =
    irr::core::vector3df(0, -85*body.PhysicsBody->getMass() * gravityAccerelation, 0);

  if(!(parameters.States & ECS_FALLING))
  {
    if(parameters.States & ECS_JUMPING)
    {
      // Jump height achieved. Start falling
      if(currentPosition.Y >= jumpHeight
      // Or if the distance traveled in the last frame is very small,
      // we have hit a ceiling and can start faling again.
      || fabs(jumpHeightOnLastFrame - currentPosition.Y) <= 0.1f)
      {
        parameters.States & ECS_FALLING;
        parameters.States &= ~ECS_JUMPING;
      }
      else
      {
        // Jump force
        body.PhysicsBody->setForce(irr::core::vector3df(0, 47*body.PhysicsBody->getMass(), 0));
        jumpHeightOnLastFrame = currentPosition.Y;
      }
    }
    else
    {
      bool smoothStairStep = true;

      if(Core->time.delta >= 0.025f)
        smoothStairStep = false;

      if((currentPosition.Y < floorY - 0.20f) && smoothStairStep)
      {
        // Push up the stairs
        body.PhysicsBody->setForce(irr::core::vector3df(0, 25*body.PhysicsBody->getMass(), 0));

        parameters.States |= ECS_ON_STAIRS;
      }
      else
      {
        if(fabs(currentPosition.Y-floorY) > 0.20f)
        {
          parameters.States |= ECS_ON_STAIRS;
        }
        else
        {
          parameters.States &= ~ECS_ON_STAIRS;
        }

        currentPosition.Y = floorY;
        body.PhysicsBody->setPosition(currentPosition);
      }
    }
  }
  else
  {
    gravityAccerelation = fmin(gravityAccerelation + (1.3f * Core->time.delta), 1.3f);

    parameters.States &= ~ECS_ON_STAIRS;

    fallingTime += Core->time.delta;

    body.PhysicsBody->setForce(gravity);
  }

#endif

#ifdef PHYSICS_IRR_NEWT
  if((parameters.States & ECS_JUMPING) && !(parameters.States & ECS_FALLING))
  {
    // First create a line to use for tracing.
    irr::core::line3df toCeiling;
    toCeiling.start = nextPosition + irr::core::vector3df(0.f, 1.0f, 0.f);
    toCeiling.end = toCeiling.start + irr::core::vector3df(0.f, 3.30f, 0.f);

    // Character body is excluded from collision detection
    irr::core::array<NewtonBody*> excludedBodies;
    excludedBodies.push_back(body.PhysicsBody->getNewtonBody());

    // Check for collision
    irr::newton::SIntersectionPoint collisionOutput =
      Core->getPhysics()->GetCollisionFromLine(toCeiling, excludedBodies);

    // Maximum height achieved
    if(collisionOutput.body != NULL)
    {
      parameters.States |= ECS_FALLING;
    }
    // Maximum height achieved
    else if(nextPosition.Y >= jumpHeight)
    {
      parameters.States |= ECS_FALLING;
    }
    else
    {
      body.PhysicsBody->addForce(irr::core::vector3df(0, 38.f * body.PhysicsBody->getMass(), 0));
    }

  }
  else
  {
    irr::f32 moving_stamina_decrease = 0.f;

    if(parameters.States & ECS_MOVING)
      moving_stamina_decrease = 0.6f;

    if(parameters.States & ECS_STANDING) {
      parameters.Stamina = fmin(parameters.Stamina+((1.37f-moving_stamina_decrease)*Core->Time.Delta),parameters.StaminaMax);
    }
    else if(parameters.States & ECS_CROUCHING) {
      parameters.Stamina = fmin(parameters.Stamina+((1.50f-moving_stamina_decrease)*Core->Time.Delta),parameters.StaminaMax);
    }
    else if(parameters.States & ECS_LYING) {
      parameters.Stamina = fmin(parameters.Stamina+((1.85f-moving_stamina_decrease)*Core->Time.Delta),parameters.StaminaMax);
    }

    irr::u16 lines = 5;
    if(parameters.States & ECS_LYING) lines = 1;

    irr::core::line3df line[lines];

    irr::core::vector3df offset[lines];

    irr::newton::SIntersectionPoint collisionOutput[lines];

    bool collision = false;
    bool onSlope = false;
    bool tooSteep = false;

    irr::core::vector3df groundPosition;
    groundPosition.set(0, -9999, 0);

    irr::core::matrix4 mat;

    groundPosition.set(0,-1000,0);

    if(lines == 1)
    {
      offset[0].set(0.f, 0.f, 0.f);
    }
    else
    {
      offset[0].set(0.f, 0.f, 0.f);
      offset[1].set(-0.610f, 0.f, 0.580f);
      offset[2].set(0.610f, 0.f, 0.580f);
      offset[3].set(-0.610f, 0.f, -0.580f);
      offset[4].set(0.610f, 0.f, -0.580f);
    }

    // Character body is excluded from collision detection
    irr::core::array<NewtonBody*> excludedBodies;
    excludedBodies.push_back(body.PhysicsBody->getNewtonBody());

    mat.setRotationDegrees(getRotation());

    //irr::video::SMaterial lineMat;

    //Core->getRenderer()->getVideoDriver()->setMaterial(lineMat);
    //Core->getRenderer()->getVideoDriver()->setTransform(irr::video::ETS_WORLD, irr::core::matrix4());

    for(irr::s16 i = lines-1; i >= 0; --i)
    {
      mat.transformVect(offset[i]);

      line[i].start = nextPosition + offset[i];
      line[i].end = nextPosition + (offset[i]*0.90f);

      //line[i].start = nextPosition + offset[i];
      //line[i].end = line[i].start;

      line[i].end.Y -= 1;

      if(parameters.States & ECS_CROUCHING)
        line[i].start.Y += EYE_HEIGHT_CROUCHING.Y;
      else if(parameters.States & ECS_LYING)
        line[i].start.Y += EYE_HEIGHT_LYING.Y;
      else
        line[i].start.Y += EYE_HEIGHT_STANDING.Y;

      //Core->getRenderer()->getVideoDriver()->draw3DLine(line[i].start, line[i].end);

      collisionOutput[i] = Core->getPhysics()->GetCollisionFromLine(
        line[i],
        Core->getPhysics()->character_ignore);

      if(collisionOutput[i].body != NULL)
      {
        collision = true;

        irr::f32 s_dist = collisionOutput[i].point.Y - nextPosition.Y;

        //if(groundPosition.Y < collisionOutput[i].point.Y)
        if(collisionOutput[i].point.Y > groundPosition.Y && (s_dist <= MAX_STEP_HEIGHT+0.24f))
        {
          groundPosition = collisionOutput[i].point;

          irr::core::vector3df normal = collisionOutput[i].normals;

          irr::f32 dot_product = normal.dotProduct(irr::core::vector3df(0,1,0));

          if(dot_product <= 0.85)
            onSlope = true;

          if(dot_product <= 0.50)
            tooSteep = true;
        }

      }
    }




    // Collided!
    if(collision)
    {
        //if(nextPosition.Y <= groundPosition.Y + 0.45f)
        //{

          irr::f32 f_step = 4.5f * Core->Time.Delta;

          if(fabs(nextPosition.Y - groundPosition.Y) > 0.2f && onSlope == false)
          {
            if(nextPosition.Y > groundPosition.Y)
              nextPosition.Y -= f_step;
            else
              nextPosition.Y += f_step;

            parameters.States |= ECS_ON_STAIRS;

          }
          else
          {
            nextPosition.Y = groundPosition.Y;
            parameters.States &= ~ECS_ON_STAIRS;

            // You can jump again after landing
            if(parameters.States & ECS_FALLING)
            {
              parameters.States &= ~ECS_FALLING;
              parameters.States &= ~ECS_JUMPING;

              // Calculate falling distance and decrease health if needed
              irr::f32 fallingDistance = jumpHeight - nextPosition.Y;

              // TODO : Health reduction

              jumpHeight = 0.f;
              fallingSpeed = 0.f;
            }
          }

          body.PhysicsBody->setPosition(nextPosition);


        //}
        //else
        //    body.PhysicsBody->setForce(irr::core::vector3df(0, -32.f * body.PhysicsBody->getMass(), 0));

    }
    else
    {
      body.PhysicsBody->addForce(irr::core::vector3df(0, -(32.f + fallingSpeed) * body.PhysicsBody->getMass(), 0));

      fallingSpeed = fmin(fallingSpeed + (35 * Core->Time.Delta), MAX_CHARACTER_FALLING_SPEED);

      if(irr::s32(nextPosition.Y) < -50)
        Game->getCharacters()->spawn(this);
    }
  }
#endif
}

/*
  Moves the character in the given direction with the given speed.
  Speed is altered depending on the state of the character (crouching, lyring down)
  Stamina is decreased.

  dir - Direction vector for moving
  moveSpeed - Nasdaq' index .. duh.
  inAir - True when character is jumping or falling (it's not on the ground)
*/

void CBaseCharacter::move(irr::core::vector3df dir, irr::f32 moveSpeed, bool inAir, irr::f32 deceleration)
{
  irr::core::matrix4 mat;
  mat.setRotationDegrees(getRotation());
  mat.transformVect(dir);

  if(parameters.States & ECS_LYING) moveSpeed = 4.25f;
  else if(parameters.States & ECS_CROUCHING) moveSpeed *= 0.6f;

  //moveSpeed *= 2.0f;

#ifdef PHYSICS_NEWTON
  irr::core::vector3df currentForce = body.PhysicsBody->getForce();

  // Decelerate
  if(inAir)
  {
    if(currentForce.getLength() > 1.5f) {
      fallingVelocity *= 0.967f;
      currentForce.X = fallingVelocity.X;
      currentForce.Z = fallingVelocity.Z;
    }
    else
    {
      currentForce.X = 0.f;
      currentForce.Z = 0.f;
    }
  }
  else
  {
    currentForce += dir * moveSpeed * body.PhysicsBody->getMass()
      * (1 + (parameters.States & ECS_RUNNING)
      * (0.50f * runSpeedFactor));
  }

  body.PhysicsBody->setForce(currentForce);
#endif

  // Decrease stamina and limit it to 0
  /*if(parameters.States & ECS_RUNNING) {
    parameters.Stamina = fmax(parameters.Stamina-(runSpeedFactor*3.2f*Core->Time.Delta),0.f);
  }*/

  // Set MOVING flag
  parameters.States |= ECS_MOVING;
}

irr::u16 tmp_i = 0;

void CBaseCharacter::checkForStairs(irr::core::vector3df dir)
{
  if(parameters.States & ECS_JUMPING)
    return;

  irr::core::vector3df currentPosition = body.PhysicsBody->getPosition();
  irr::core::vector3df size = body.Node->getBoundingBox().getExtent();
  irr::core::matrix4 rotationMatrix;

  physics::SRayCastParameters stairRayCastParams;
  physics::SRayCastResult stairRayCastResult[5];

  rotationMatrix.setRotationDegrees(getRotation());
  stairRayCastParams.excluded.push_back(body.PhysicsBody->getShapeID());

  irr::core::vector3df lineOffsets[5];
  lineOffsets[0].set(0, 0.0f, 0);
  lineOffsets[1].set(0.30f, 0.0f, 0.30f);
  lineOffsets[2].set(-0.30f, 0.0f, 0.30f);
  lineOffsets[3].set(0.30f, 0.0f, -0.30f);
  lineOffsets[4].set(-0.30f, 0.0f, -0.30f);

  irr::core::line3df stairLine;

  irr::video::SMaterial lineMat;

  Core->getRenderer()->getVideoDriver()->setMaterial(lineMat);
  Core->getRenderer()->getVideoDriver()->setTransform(irr::video::ETS_WORLD, irr::core::matrix4());

  for(irr::u16 i=0; i<5; ++i)
  {
    rotationMatrix.rotateVect(lineOffsets[i]);

    stairLine.start =
      currentPosition +
      lineOffsets[i] -
      (parameters.States & ECS_STANDING) * irr::core::vector3df(0, 0.25f, 0);

    stairLine.end = stairLine.start - irr::core::vector3df(0, 100, 0);

    Core->getRenderer()->getVideoDriver()->draw3DLine(stairLine.start, stairLine.end);

    stairRayCastParams.line = stairLine;

    stairRayCastResult[i] = Core->getPhysics()->getRayCollision(stairRayCastParams);
  }

  irr::s32 contactPointIndex = -1;
  irr::f32 contactPointY = -99999.f;

  // Find the highest contact point

  for(irr::u16 i=0; i<5; ++i)
  {
    if(stairRayCastResult[i].body != NULL)
    {
      if(stairRayCastResult[i].position.Y > contactPointY
      && stairRayCastResult[i].position.Y <= currentPosition.Y)
      {
        contactPointY = stairRayCastResult[i].position.Y;
        contactPointIndex = i;
      }
    }
  }

  //irr::f32 offsetFromGround = 1.03f;
  irr::f32 heightOffset = 2.5f;
  irr::f32 floorOffset = 0.f;

  if(parameters.States & ECS_CROUCHING)
  {
    heightOffset = 1.60f;
  }
  else if(parameters.States & ECS_LYING)
  {
    heightOffset = 0.72f;
  }


  if(Core->time.delta >= 0.025f)
    floorOffset += 0.70f;

  //printf("%.5f\n", size.Y);

  // size.Y is 3.05



  // Any contact at all?
  if(contactPointIndex != -1)
  {
    if(contactPointY >= (currentPosition.Y - heightOffset - (0.38f + floorOffset)))
    {
      // Set the new floor height
      floorY = contactPointY + heightOffset;

      // Not falling anymore
      parameters.States &= ~ECS_FALLING;

      fallingTime = 0;

      if(parameters.States & ECS_MOVING)
        fallingVelocity = NULLVECTOR;
    }
    else
    {
      // Start falling
      if(!(parameters.States & ECS_FALLING))
      {
        gravityAccerelation = 0.35;
        parameters.States |= ECS_FALLING;
      }
    }
  }
  else
  {
    // No contact? FALLL!!1115##% AAAAAAAAAAaaaaaa
    if(!(parameters.States & ECS_FALLING))
    {
      gravityAccerelation = 0.35;
      parameters.States |= ECS_FALLING;
    }
  }


}

void CBaseCharacter::crouch()
{
  if(parameters.States & ECS_CROUCHING) return;

  parameters.States &= ~(ECS_STANDING);
  parameters.States |= ECS_CROUCHING;
  parameters.States &= ~(ECS_RUNNING);
  parameters.States &= ~(ECS_LYING);

  irr::core::vector3df rot = getRotation();

#ifdef PHYSICS_NEWTON

  irr::core::vector3df bodyPos = body.PhysicsBody->getPositionBody();

  body.PhysicsBody->setUsedCollision(1);

  body.PhysicsBody->setPosition(bodyPos);

#endif

#ifdef PHYSICS_IRR_NEWT
  if(body.PhysicsBody) body.PhysicsBody->removeBody();

  //body.PhysicsBody->setScale(UPRIGHT_SCALE);
  body.PhysicsBody = Core->getPhysics()->GetWorld()->createBody(body.Node, CrouchingCollision);

  body.PhysicsBody->setContinuousCollisionMode(true);
  body.PhysicsBody->setMass(100);

  // Create up-vector constraint. This keeps character up and only allows to rotate on Y axis.
  irr::newton::SJointUpVector joint;
  	joint.PinDir = Core->getPhysics()->getUpVector();
  	joint.ParentBody = body.PhysicsBody;

  irr::newton::IJointUpVector* upVector = Core->getPhysics()->GetWorld()->createJoint(joint);
  upVector->setCollisionState(false);

  body.PhysicsBody->setRotation(rot);
#endif
}


void CBaseCharacter::lie()
{
  if(parameters.States & ECS_LYING) return;

  parameters.States &= ~(ECS_CROUCHING);
  parameters.States |= ECS_LYING;
  parameters.States &= ~(ECS_RUNNING);

  irr::core::vector3df rot = getRotation();

#ifdef PHYSICS_NEWTON

  irr::core::vector3df bodyPos = body.PhysicsBody->getPositionBody();

  body.PhysicsBody->setUsedCollision(2);

  body.PhysicsBody->setPosition(bodyPos);

#endif

#ifdef PHYSICS_IRR_NEWT
  if(body.PhysicsBody) body.PhysicsBody->removeBody();

  //body.PhysicsBody->setScale(UPRIGHT_SCALE);
  body.PhysicsBody = Core->getPhysics()->GetWorld()->createBody(body.Node, LyingCollision);

  body.PhysicsBody->setContinuousCollisionMode(true);
  body.PhysicsBody->setMass(100);
  body.PhysicsBody->setMaterial(Core->getPhysics()->playerMaterial);

  // Create up-vector constraint. This keeps character up and only allows to rotate on Y axis.
  irr::newton::SJointUpVector joint;
  	joint.PinDir = Core->getPhysics()->getUpVector();
  	joint.ParentBody = body.PhysicsBody;

  irr::newton::IJointUpVector* upVector = Core->getPhysics()->GetWorld()->createJoint(joint);
  upVector->setCollisionState(false);

  body.PhysicsBody->setRotation(rot);
#endif
}


void CBaseCharacter::stand()
{
  if(parameters.States & ECS_STANDING) { return; }

  parameters.States &= ~(ECS_CROUCHING);
  parameters.States &= ~(ECS_LYING);
  parameters.States |= ECS_STANDING;

  irr::core::vector3df rot = getRotation();

#ifdef PHYSICS_NEWTON

  body.PhysicsBody->setUsedCollision(0);

#endif

#ifdef PHYSICS_IRR_NEWT
  if(body.PhysicsBody) body.PhysicsBody->removeBody();

  //body.PhysicsBody->setScale(UPRIGHT_SCALE);
  body.PhysicsBody = Core->getPhysics()->GetWorld()->createBody(body.Node, StandingCollision);

  body.PhysicsBody->setContinuousCollisionMode(true);
  body.PhysicsBody->setMass(100);

  // Create up-vector constraint. This keeps character up and only allows to rotate on Y axis.
  irr::newton::SJointUpVector joint;
  	joint.PinDir = Core->getPhysics()->getUpVector();
  	joint.ParentBody = body.PhysicsBody;

  irr::newton::IJointUpVector* upVector = Core->getPhysics()->GetWorld()->createJoint(joint);
  upVector->setCollisionState(false);

  body.PhysicsBody->setRotation(rot);

#endif
}

void CBaseCharacter::jump()
{
  // Not falling nor jumping currently
  if(!(parameters.States & ECS_JUMPING) && !(parameters.States & ECS_FALLING))
  {
    parameters.States |= ECS_JUMPING;

    irr::core::vector3df currentPosition = body.PhysicsBody->getPosition();

    // Get the position of the top of the character
    irr::core::vector3df topOfCharacter = currentPosition;
    topOfCharacter.Y = body.Node->getTransformedBoundingBox().MaxEdge.Y;

    // First create a line to use for tracing.
    irr::core::line3df jumpLine;
            jumpLine.start = topOfCharacter;
            jumpLine.end = topOfCharacter + irr::core::vector3df(0, MAX_JUMP_HEIGHT, 0);

    engine::physics::SRayCastParameters crosshairRay;

    crosshairRay.excluded.push_back(body.PhysicsBody->getShapeID());

    crosshairRay.line = jumpLine;

    engine::physics::SRayCastResult rayResult =
      Core->getPhysics()->getRayCollision(crosshairRay);

    // There's something above characters head
    if(rayResult.body != NULL)
    {
      irr::f32 distToSolid = rayResult.position.Y - topOfCharacter.Y;

      // Too little space for jumping
      if(distToSolid < MIN_JUMP_HEIGHT)
      {
        parameters.States &= ~ECS_JUMPING;
        return;
      }

      // Limit jump height
      if(distToSolid > MAX_JUMP_HEIGHT) distToSolid = MAX_JUMP_HEIGHT;

      // Current position Y + jumping height
      jumpHeight = currentPosition.Y + distToSolid;
    }
    else // No obstacles
    {
      jumpHeight = currentPosition.Y + MAX_JUMP_HEIGHT;
    }

    jumpHeight -= 0.15f;
    jumpHeightOnLastFrame = -9999.f;
    fallingTime = 0;

    parameters.States &= ~ECS_FALLING;

    printf("Jumping: Y=%.4f\n", jumpHeight);

#ifdef ENGINE_DEVELOPMENT_MODE
    if(debug_jump)
      jumpHeight += MAX_JUMP_HEIGHT*16;
#endif

  }
}

irr::f32 SCharacterStats::getKD()
{
  irr::u32 k = kills;
  irr::u32 d = deaths;

  if(k == 0)
    k = 1;

  if(d == 0)
    d = 1;

  irr::f32 KDValue = irr::f32(k/d);

  return KDValue;
}
