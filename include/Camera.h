#ifndef CAMERA_HEADER_DEFINED
#define CAMERA_HEADER_DEFINED

#include <irrlicht.h>

#include "Engine.h"

namespace engine {

  /*struct SCameraData
  {
    irr::scene::ISceneManager * SceneManager;
    irr::s32 Material;
  };*/

  enum E_CAMERA_TYPE
  {
    ECT_UNDEFINED = 0,
    ECT_CONTROLLED = 1,
    ECT_SPECTATOR,
    ECT_FIRST_PERSON,
    ECT_3RD_PERSON
  };

  const irr::core::vector3df EYE_HEIGHT_STANDING = irr::core::vector3df(0.f, 1.36f, 0.f);

  const irr::core::vector3df EYE_HEIGHT_CROUCHING = irr::core::vector3df(0.f, 0.67f, 0.f);

  const irr::core::vector3df EYE_HEIGHT_LYING = irr::core::vector3df(0.f, 0.30f, 0.f);

  const irr::f32 MAX_HEAD_ROLL = 18.0f;

  const irr::f32 CAMERA_PROXIMITY_TO_WALL = 1.75f;

  class CCamera
  {
  private:

    CCore * Core;

    E_CAMERA_TYPE type;

    irr::s32 Material;

    engine::CBaseCharacter *cameraCharacter;

    irr::scene::ICameraSceneNode *node;

    irr::scene::IMeshSceneNode *FPSViewObject;

    irr::f32 headBobY, headBobX, objectBobY, objectBobX;

    irr::f32 shadowValue, shakeValue, shakeValueTarget;

    irr::f32 tilt, roll, distanceToWall;

    // This is the position object has to animate into
    irr::core::vector3df objectFinalPosition;

    // When animating the pull/push object, this variable is used for storing the position
    irr::core::vector3df objectCurrentPosition;

    bool weaponCloseUpAim;

  public:

    CCamera(CCore * core);

    inline void setMaterial(irr::s32 material) { Material = material; }

    void setType(E_CAMERA_TYPE newType);

    void update(irr::f32 &);

    void reset();

    irr::scene::ICameraSceneNode *getNode() { return node; }

    irr::f32 getTilt() { return tilt; }

    irr::f32 getRoll() { return roll; }

    void setRoll(irr::f32 val) { roll = val; }

    void setTilt(irr::f32 val) { tilt = val; }

    void setDistanceToWall(irr::f32 val) { distanceToWall = val; }

    void setFollowedCharacter(CBaseCharacter *);

    CBaseCharacter* getFollowedCharacter() { return cameraCharacter; }

    void setFPSViewObjectMesh(irr::scene::IMesh *mesh);

    void setFPSViewObject(
      irr::scene::IMesh *mesh,
      irr::core::vector3df &position,
      irr::core::vector3df &scale,
      bool setInstant=false);

    void setShaking(irr::f32 strength) {
      shakeValueTarget = strength;
    }

    irr::scene::IMeshSceneNode *getFPSViewObjectNode() { return FPSViewObject; }

    irr::f32 getShadow() { return shadowValue; }

    bool isWeaponAimedCloseUp() { return weaponCloseUpAim; }

    void toggleWeaponCloseUpAim();

    bool isFPSObjectReady();

    void pullObjectBack(bool change = false, bool fastmove = false);
    void pushObjectForward(bool fastmove = false);
    irr::s16 getObjectMovingDirection();

    //
    // Position
    //

    irr::core::vector3df getFPSViewObjectPosition()
    {
      if(FPSViewObject != NULL)
        return FPSViewObject->getPosition();

      return irr::core::vector3df(0,0,0);
    }

    void setFPSViewObjectPosition(irr::core::vector3df &pos)
    {
      if(FPSViewObject != NULL)
        FPSViewObject->setPosition(pos);

      objectCurrentPosition = objectFinalPosition = pos;
    }

    //
    // Scale
    //

    irr::core::vector3df getFPSViewObjectScale()
    {
      if(FPSViewObject != NULL)
        return FPSViewObject->getScale();

      return irr::core::vector3df(0,0,0);
    }

    void setFPSViewObjectScale(irr::core::vector3df &scale)
    {
      if(FPSViewObject != NULL)
        FPSViewObject->setScale(scale);
    }

  };

}

#endif
