#include "Camera.h"
#include "Core.h"
#include "Renderer.h"
#include "BaseCharacter.h"
#include "Maths.h"
#include "Physics.h"
#include "ObjectManager.h"
#include "Configuration.h"

using namespace engine;

irr::f32 objectChangingRotation;
irr::f32 rotChange;
irr::f32 currentDistanceToWall = 0.f;

irr::scene::IMesh *newMesh;
irr::core::vector3df newPosition, newScale;

// 0 not moving
// 1 object pushing forward
// 2 object pulling back
irr::s16 objectMovingDirection = 0;
bool changeObjectMesh = false;

void CCamera::toggleWeaponCloseUpAim()
{
  weaponCloseUpAim = !weaponCloseUpAim;
}

void CCamera::reset()
{
  type = ECT_UNDEFINED;

  node = (irr::scene::ICameraSceneNode*) NULL;

  FPSViewObject = (irr::scene::IMeshSceneNode*) NULL;

  headBobY = headBobX = objectBobY = objectBobX = shadowValue = shakeValue = shakeValueTarget = 0.f;

  tilt = roll = 0.f;

  weaponCloseUpAim = false;

  cameraCharacter = (CBaseCharacter*)NULL;
}

bool CCamera::isFPSObjectReady()
{
  if(objectMovingDirection == 0) return true;
  else return false;
}

CCamera::CCamera(CCore * core) : Core(core)
{
  // Make sure values are reset
  reset();
}


void CCamera::setFPSViewObjectMesh(irr::scene::IMesh *mesh)
{
  if(FPSViewObject == NULL) return;

  mesh->setHardwareMappingHint(irr::scene::EHM_STATIC);
  mesh->setDirty();

  FPSViewObject->setMesh(mesh);

  FPSViewObject->setRotation(irr::core::vector3df(0,0,0));

  // Set correct material params
  for(irr::u16 materialId = 0; materialId < FPSViewObject->getMaterialCount(); ++materialId)
  {
    FPSViewObject->getMaterial(materialId).setFlag(irr::video::EMF_LIGHTING, false);
    // NOTE: you better make it independent of the ambient -- devsh
    FPSViewObject->getMaterial(materialId).DiffuseColor =  irr::video::SColor(255,
                                                          Core->getObjects()->parameters.lightValues.Y*255.f,
                                                          Core->getObjects()->parameters.lightValues.Y*255.f,
                                                          Core->getObjects()->parameters.lightValues.Y*255.f);
    FPSViewObject->getMaterial(materialId).AmbientColor = irr::video::SColor(255, 205,205,205);
    FPSViewObject->getMaterial(materialId).EmissiveColor = irr::video::SColor(255, 0,0,0);
    FPSViewObject->getMaterial(materialId).SpecularColor = irr::video::SColor(255, 160,160,160);
    FPSViewObject->getMaterial(materialId).Shininess = 108.f;

    irr::core::stringc texName = FPSViewObject->getMaterial(materialId).TextureLayer[0].Texture->getName();

    if(texName.find("hand") == -1)
      FPSViewObject->getMaterial(materialId).MaterialType = (irr::video::E_MATERIAL_TYPE)Material;

    if(texName.find("sight") != -1)
      FPSViewObject->getMaterial(materialId).MaterialType = irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL;

  }

  FPSViewObject->setMaterialFlag(irr::video::EMF_BACK_FACE_CULLING, true);


  //FPSViewObject->setMaterialType((irr::video::E_MATERIAL_TYPE)Material);


}

irr::s16 CCamera::getObjectMovingDirection() { return objectMovingDirection; }

bool objectMovesFast = false;

void CCamera::pullObjectBack(bool change, bool fastmove)
{
  objectMovingDirection = 2;
  changeObjectMesh = change;
  objectCurrentPosition = objectFinalPosition;
  objectChangingRotation = 0;
  objectMovesFast = fastmove;
}

void CCamera::pushObjectForward(bool fastmove)
{
  objectMovingDirection = 1;
  objectMovesFast = fastmove;
}


/*
  Set the mesh, relative position to the camera and scale
  of the player weapon/item node.

  If setInstant is true, the postion is set directly,
  instead of smootly moving it away and back.
*/

void CCamera::setFPSViewObject(
  irr::scene::IMesh *mesh,
  irr::core::vector3df &position,
  irr::core::vector3df &scale,
  bool setInstant)
{
  if(!FPSViewObject)
    return;

  if(setInstant == true)
  {
    setFPSViewObjectMesh(mesh);
    setFPSViewObjectPosition(position);
    setFPSViewObjectScale(scale);
    FPSViewObject->setRotation(irr::core::vector3df(0,0,0));

    objectMovingDirection = 0;
  }
  else
  {
    // These parameters will be set when the object is pulled
    // out of the view.
    newMesh = mesh;
    newPosition = position;
    newScale = scale;

    // Object starts moving "back" towards the player
    pullObjectBack(true);

    objectFinalPosition = position;
  }
}

/*
  Set the camera type. Different types of cameras are handeled
  differently in the update method.
*/

void CCamera::setType(E_CAMERA_TYPE newType)
{
  // Type already set
  if(type == newType) return;

  // Undefined type
  if(newType == 0) return;

  type = newType;

  // First remove the current camera
  if(node != NULL)
  {
    node->remove();
    node = (irr::scene::ICameraSceneNode*) NULL;
  }

  // Create the object which is in front of the camera in FPS mode
  if(FPSViewObject == NULL)
  {
    FPSViewObject = Core->getRenderer()->getSceneManager()->addMeshSceneNode(
      Core->getRenderer()->getSceneManager()->getMesh("data/weap/empty.ms3d"));

    // And hide it for the time being
    FPSViewObject->setVisible(false);
  }

  // Create the correct camera based on the type
  switch(type)
  {
    //
    // This camera will be used in cutscenes (eventually)

    case ECT_CONTROLLED:
    {
      node = Core->getRenderer()->getSceneManager()->addCameraSceneNode();
      node->bindTargetAndRotation(false);
    }
    break;

    //
    // Do I need to explain?

    case ECT_SPECTATOR:
    {
      node = Core->getRenderer()->getSceneManager()->addCameraSceneNodeFPS(0, 100.0, 0.100f);
      node->bindTargetAndRotation(false);
    }
    break;

    //
    // This is the main type of camera. Standard first-person camera.

    case ECT_FIRST_PERSON:
    {
      node = Core->getRenderer()->getSceneManager()->addCameraSceneNode();

      // Set to true so we can pass values to setRotation. Otherwise setTarget()
      // has to be used.
      node->bindTargetAndRotation(true);

      // Only in FPS camera is the player weapon object visible
      if(FPSViewObject)
        FPSViewObject->setVisible(true);
    }
    break;

    //
    // Not used atm.

    case ECT_3RD_PERSON:
    {
      node = Core->getRenderer()->getSceneManager()->addCameraSceneNode();
      node->bindTargetAndRotation(true);
    }
    break;

    // Some unknown value ..
    default:
      return;
    break;
  }

  if(node)
  {
    // Set the aspect ratio.
    node->setAspectRatio(
      Core->getConfiguration()->getVideo()->aspectRatio.Width /
      Core->getConfiguration()->getVideo()->aspectRatio.Height );

    // Far and near values
    node->setFarValue(irr::f32(Core->getConfiguration()->getVideo()->drawRange));

    node->setNearValue(0.032f);

    // Field of view
    node->setFOV(1.277f);

    // The object is parented to the camera
    if(FPSViewObject)
      FPSViewObject->setParent(node);
  }

  return;
}

/*
  Specifies the character which is used to position and rotate the camera
*/

void CCamera::setFollowedCharacter(CBaseCharacter* follow_char)
{
  cameraCharacter = follow_char;

  // If the camera is in first person mode we make the character
  // model invisible.
  if(cameraCharacter && type == ECT_FIRST_PERSON)
  {
    cameraCharacter->getBody()->Node->setMaterialType(
      irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF);

    // This texture is fully transparent
    cameraCharacter->getBody()->Node->setMaterialTexture(0,
      Core->getRenderer()->getVideoDriver()->getTexture("data/chars/transparent.png"));
  }
}

irr::core::vector3df eyeHeight = EYE_HEIGHT_STANDING, targetEyeHeight;
irr::f32 runningObjectTurnAngle = 0.f;
irr::f32 checkForShadow = 0;
bool eyeHeightReady = true, fpsCameraInShadow = false;

irr::f32 closeUpTransition = 0.f;

//bool aniso = false;

irr::core::vector3df previousFPSCameraRotation;

void CCamera::update(irr::f32 &time)
{
  if(node == NULL) return;

  switch(type)
  {
    case ECT_FIRST_PERSON:
    {
      // Camera is not following any character
      if(cameraCharacter == NULL)
      {
#ifdef ENGINE_DEVELOPMENT_MODE
        static bool error_shown = false;
        if(!error_shown)
        {
          printf("CCamera::update : cameraCharacter is NULL");
          error_shown = true;
        }
#endif
        return;
      }

      // Shorter to write
      bool pl_crouching = cameraCharacter->getParameters()->States & ECS_CROUCHING;
      bool pl_lying = cameraCharacter->getParameters()->States & ECS_LYING;

      irr::core::vector3df cameraBobVector;
      irr::core::vector3df viewObjectBobVector;

      // Camera rotation is the same as the character
      irr::core::vector3df cameraRotation = cameraCharacter->getRotation();

      // Position of the character
      irr::core::vector3df characterPosition = cameraCharacter->getBody()->PhysicsBody->getPosition();

      // Will eventually contain the new position of the camera node
      irr::core::vector3df cameraPosition;

      /*
      When player has the weapon close-up aim enabled, and the weapon is not being
      pulled back/pushed forward, we can modify the variable closeUpTransition that
      shows the level of transition. Values 0 .. 0.5 + 0.12 if player is lying down.
      0 is normal and 0.5+ is fully enabled.
      This code smootly animates between those two values.
      */
      if(weaponCloseUpAim && objectMovingDirection == 0)
      {
        // If the value can be higher than 0.5 (the player is lying down)
        // we increase it a bit more.
        if(closeUpTransition > 0.5f + pl_lying*0.12f) {
          closeUpTransition = fmax(closeUpTransition - time, 0.5f + pl_lying*0.12f);
        }
        // But if it's higher than 0.5 (like 0.62) and the player is not lying down,
        // we have to bring back down to 0.5.
        else if(closeUpTransition < 0.5f + pl_lying*0.12f) {
          closeUpTransition = fmin(closeUpTransition + time, 0.5f + pl_lying*0.12f);
        }
      }
      // When close-up aim is disabled, we return to 0.
      else
      {
        closeUpTransition = fmax(closeUpTransition - (time*1.4f), 0.0f);
      }

      // Set the field of view
      node->setFOV(1.277f - (closeUpTransition * 0.87f));


      /*
        Camera and view object bobbing
      */

      // When moving the bob speed depends on the speed of moving
      if(cameraCharacter->getParameters()->States & ECS_MOVING)
      {
        irr::f32 bobStrenght = 0.5f + cameraCharacter->getRunSpeedFactor();

        headBobY += 12 * time * bobStrenght;
        headBobX += 13 * time * bobStrenght;
        objectBobY += 5 * time * bobStrenght;
        objectBobX += 6.8f * time * bobStrenght;
      }
      else // When standing still the head moves slowly
      {
        if(!pl_lying)
        {
          headBobY += 3.0f * time;
          headBobX += 4.4f * time;
          objectBobY += 0.21f * time;
          objectBobX += 1.27f * time;
        }
        // When lying down, it's even slower
        else
        {
          headBobY += 0.9f * time;
          headBobX += 1.24f * time;
          objectBobY += 0.08f * time;
          objectBobX += 0.34f * time;
        }
      }

      // Keeps in range of 360
      if(headBobY > 360) headBobY -= 360.f;
      else if(headBobX > 360) headBobX -= 360.f;

      if(objectBobY > 360) objectBobY -= 360.f;
      else if(objectBobX > 360) objectBobX -= 360.f;

      // Set head bobbing vector values
      cameraBobVector.set(
        closeUpTransition * 0.2f - (roll / 15),
        (sin(headBobY) / (6 + (pl_crouching + pl_lying)*3)) - closeUpTransition * 0.27f,
        closeUpTransition * 0.26f);

      // Transform bob vector to character rotation
      irr::core::matrix4 mat;
      mat.setRotationDegrees(cameraRotation);

      mat.transformVect(cameraBobVector);

      //
      // Set the correct eye height (camera Y position relative to player position)
      // It's different when standing up, croucing or lying down
      //

      if((cameraCharacter->getParameters()->States & ECS_CROUCHING))
      {
        targetEyeHeight = EYE_HEIGHT_CROUCHING;

        if(eyeHeightReady) eyeHeightReady = false;
      }
      else if((cameraCharacter->getParameters()->States & ECS_LYING))
      {
        targetEyeHeight = EYE_HEIGHT_LYING;

        if(eyeHeightReady) eyeHeightReady = false;
      }
      else
      {
        targetEyeHeight = EYE_HEIGHT_STANDING;

        if(eyeHeightReady) eyeHeightReady = false;
      }

      // Eye height is being transitioned
      if(!eyeHeightReady)
      {
        // Eye height is lower than it needs to be .. increase it!
        if(eyeHeight.Y < targetEyeHeight.Y)
        {
          eyeHeight.Y += 4 * time;

          // Target height reached
          if(eyeHeight.Y >= targetEyeHeight.Y)
          {
            eyeHeight.Y = targetEyeHeight.Y;
            eyeHeightReady = true;
          }
        }
        // Eye height is higher than it needs to be .. lower it!
        else if(eyeHeight.Y > targetEyeHeight.Y)
        {
          eyeHeight.Y -= 4 * time;

          // Target height reached
          if(eyeHeight.Y <= targetEyeHeight.Y)
          {
            eyeHeight.Y = targetEyeHeight.Y;
            eyeHeightReady = true;
          }
        }
      }


      /*
        Camera shaking. Shake strenght can be set trough setShaking() method.
        Shake value is slowly decreased from the given to 0
      */

      if(shakeValue < shakeValueTarget)
      {
        shakeValue = fmin(shakeValue + 40 * time, shakeValueTarget);
      }
      else if(shakeValue > shakeValueTarget)
      {
        shakeValue = fmax(shakeValue - 40 * time, shakeValueTarget);
      }

      // Camera starts looking up a little
      tilt -= shakeValue/7;

      shakeValueTarget = fmax(shakeValueTarget - (5 * time), 0.0f);




      rotChange = sin(headBobX) / 7;

      // Randomly subtract or add. That's what creates that shaky feeling
      if(Core->getMath()->getRandomInt(0,1) == 0)
        rotChange += shakeValue / 6;
      else
        rotChange -= shakeValue / 6;

      cameraRotation.set(rotChange, cameraRotation.Y + rotChange, 0);

      // Put it all together
      cameraPosition = characterPosition + eyeHeight + cameraBobVector;

      // Set the new position of the camera node
      node->setPosition(characterPosition + eyeHeight + cameraBobVector);

      // Update it
      node->updateAbsolutePosition();

      // Run the check 10 times a second. Should be often enough ..
      if(checkForShadow < 0.90f)
      {
        irr::core::vector3df sun = Core->getObjects()->parameters.sunPosition;

        irr::core::line3df shadow_line = irr::core::line3df(cameraPosition, cameraPosition + sun);

 #ifdef PHYSICS_NEWTON
        // Check if anything blocks the line between the camera position and the sun position
        physics::SRayCastParameters params;
        params.line = shadow_line;
        params.excluded.push_back(cameraCharacter->getBody()->PhysicsBody->getShapeID());

        physics::SRayCastResult result = Core->getPhysics()->getRayCollision(params);

        fpsCameraInShadow = (result.body == NULL) ? false : true;
 #endif

        checkForShadow = 1.0f;
      }
      else
      {
        checkForShadow -= 0.01f; //Core->Time.Delta;
      }

      if(fpsCameraInShadow)
        shadowValue = fmin(shadowValue + time * 10, 1.0f);
      else
        shadowValue = fmax(shadowValue - time * 10, 0.0f);

      //
      // The view objects also moves
      //

      if(FPSViewObject)
      {
        viewObjectBobVector.set(
          sin(objectBobX) / 3.5f + (roll / 13) - closeUpTransition * 4.25f,
          sin(objectBobY) / 4 + closeUpTransition * 2.15,
          -sin(objectBobX) / 12 - shakeValue*0.9f);

        // Hmm .. what was this for? :O
        /*if(cameraCharacter->getParameters()->States & ECS_LYING
        || cameraCharacter->getParameters()->States & ECS_CROUCHING) {

          irr::f32 cr_offset = (EYE_HEIGHT_CROUCHING.Y - eyeHeight.Y) * 2;
          if(cr_offset < 0) cr_offset = 0.f;

          viewObjectBobVector.Z -= cr_offset * 1.15f;
          viewObjectBobVector.X -= cr_offset / 1.8f;
          viewObjectBobVector.Y -= cr_offset / 4;
        }*/


        /*
          When camera is close to a wall, the player weapon/object
          is pulled closer to the player.

          distanceToWall - calculated each frame. See setDistanceToWall() in
            CGUI::checkCrosshairObject()
          currentDistanceToWall - this value "fades" into distanceToWall
        */

        // Wall is far away
        if(distanceToWall > CAMERA_PROXIMITY_TO_WALL) {
          currentDistanceToWall = fmin(currentDistanceToWall + time*10, CAMERA_PROXIMITY_TO_WALL);
        }
        // Wall is within range
        else if(distanceToWall < CAMERA_PROXIMITY_TO_WALL) {
          currentDistanceToWall = fmax(currentDistanceToWall - time*10, distanceToWall);
        }

        // wallOffset variable is used to pull player weapon closer to the camera,
        // so it's inverted distance of the wall. The closer the wall, the more
        // it needs to move back.

        irr::f32 wallOffset = 0;

        if(currentDistanceToWall < CAMERA_PROXIMITY_TO_WALL) {
          wallOffset = (CAMERA_PROXIMITY_TO_WALL-currentDistanceToWall) / 20;
        }

        if(closeUpTransition <= 0.25)
        {
          irr::f32 movespeed = 0.68f + (objectMovesFast * 0.45f);

          if(objectMovingDirection == 1)
          {
            objectChangingRotation -= 75 * time;
            if(objectChangingRotation < 0) objectChangingRotation = 0;

            objectCurrentPosition.Z += movespeed * time;
            objectCurrentPosition.Y = objectFinalPosition.Y - (objectChangingRotation*0.00065f);

            FPSViewObject->setPosition(
              (objectCurrentPosition - irr::core::vector3df(0,0,wallOffset)) + (viewObjectBobVector*0.0105f));

            if(objectCurrentPosition.Z > objectFinalPosition.Z) {
              objectCurrentPosition.Z = objectFinalPosition.Z;
              objectMovingDirection = 0;
            }

            FPSViewObject->setRotation(irr::core::vector3df(0,0,0));
          }
          else if(objectMovingDirection == 2)
          {
            objectChangingRotation += 75 * time;

            objectCurrentPosition.Z -= movespeed * time;
            objectCurrentPosition.Y = objectFinalPosition.Y + (objectChangingRotation*0.00069f);
            //objectCurrentPosition.X -= 0.11*time;

            FPSViewObject->setPosition(objectCurrentPosition + (viewObjectBobVector*0.0105f));

            if(objectCurrentPosition.Z < -0.035)
            {
              if(changeObjectMesh)
              {
                pushObjectForward();

                objectCurrentPosition =
                  irr::core::vector3df(objectFinalPosition.X, objectFinalPosition.Y, -0.035);

                //setFPSViewObjectPosition(newPosition);
                setFPSViewObjectMesh(newMesh);
                setFPSViewObjectScale(newScale);

                objectChangingRotation = 0;
              }
              else {
                objectMovingDirection = -1;
              }
            }


            FPSViewObject->setRotation(irr::core::vector3df(-objectChangingRotation*0.74f,-objectChangingRotation,0));
          }
        }


        if(objectMovingDirection == 0)
        {
          FPSViewObject->setPosition(
            objectFinalPosition - irr::core::vector3df(0,0,wallOffset) +
            viewObjectBobVector*0.0105f);
          FPSViewObject->setRotation(irr::core::vector3df(0,0,0));
        }


      }

      //
      // Set tilt - looking up and down

      cameraRotation += irr::core::vector3df(tilt, 0, 0);

      //
      // Set camera roll (for looking around corners)
      // It means we have to transform the camera up vector.

      irr::core::vector3df upv = irr::core::vector3df(0, 1, 0);

      irr::f32 ang = roll - closeUpTransition*19.0f;

      if(ang > engine::MAX_HEAD_ROLL)
        ang = engine::MAX_HEAD_ROLL;
      else if(ang < -engine::MAX_HEAD_ROLL)
        ang = -engine::MAX_HEAD_ROLL;

      upv.rotateXYBy(ang, irr::core::vector3df(0,0,0));
      upv.rotateXZBy(-cameraRotation.Y, irr::core::vector3df(0,0,0));
      upv.rotateXZBy(-cameraRotation.X, irr::core::vector3df(0,0,0));

      node->setUpVector(upv);

      // Set new rotation for the camera
      node->setRotation(cameraRotation);
    }
    break;
  }
}


