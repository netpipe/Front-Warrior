#ifndef PHYSICS_HEADER_DEFINED
#define PHYSICS_HEADER_DEFINED

#include "Engine.h"

#ifdef PHYSICS_NEWTON
  #include "newton/World.h"
#endif

namespace engine {

#ifdef PHYSICS_IRR_NEWT
  struct SCharacterPhysicsResult
  {
    irr::newton::IBody * Body;
    irr::newton::ICollision * StandingCollision;
    irr::newton::ICollision * CrouchingCollision;
    irr::newton::ICollision * LyingCollision;
    irr::newton::ICollision * EmptyCollision, *collision_restore;
  };

  struct SCharacterCollisionObject
  {
    irr::newton::IBody *body;
    irr::newton::ICollision *collision, *empty_collision;
  };
#endif

  class CPhysicsManager
  {
  public:

    CPhysicsManager(CCore *);

    ~CPhysicsManager();

    irr::core::array<SPhysicsMesh> meshGroups;

    irr::scene::IMesh *getPhysicsMesh(irr::scene::ISceneNode* node);

    irr::scene::IMesh *getPhysicsMeshSimple(irr::scene::ISceneNode* node);

    void clearMeshGroups();

#ifdef PHYSICS_NEWTON

    void update(irr::u32);

    void update2();

    void close();

    void clear();

    void initOnLevel();

    physics::CBody * createCharacterBody(irr::scene::IAnimatedMeshSceneNode *);

    physics::CBody * createBodyForNode(
      irr::scene::ISceneNode* node,
      bool dynamic = true,
      irr::f32 mass = 50.0f);

    irr::core::array<physics::CBody*> createStaticPhysics(irr::scene::IMeshSceneNode*);

    void createTestObject(irr::u32, irr::core::vector3df);

    physics::CPhysicsWorld *getPhysicsWorld() { return PhysicsWorld; }

    physics::SRayCastResult getRayCollision(physics::SRayCastParameters params)
    {
      return PhysicsWorld->getRayCollision(params);
    }

    physics::SConvexCastResult getConvexCollision(physics::SConvexCastParameters params)
    {
      return PhysicsWorld->getConvexCollision(params);
    }

  private:

    physics::CPhysicsWorld *PhysicsWorld;

#endif

    CCore * Core;

#ifdef PHYSICS_IRR_NEWT
    void update(irr::f32 &time)
    {
      PhysicsWorld->update(time);
    }

    void update()
    {
      PhysicsWorld->update();
    }

    void init();

    void clear();

    irr::core::array<irr::newton::IBody*> CreateStaticPhysics(irr::scene::IMeshSceneNode*);

    SCharacterPhysicsResult createPlayerPhysics(irr::scene::IAnimatedMeshSceneNode *);

    irr::newton::IBody* createBotPhysics(irr::scene::IAnimatedMeshSceneNode *);

    irr::newton::SIntersectionPoint GetCollisionFromLine(irr::core::line3df &);

    irr::newton::SIntersectionPoint GetCollisionFromLineEx(irr::core::line3df &);

    irr::newton::SIntersectionPoint GetCollisionFromLine(irr::core::line3df &, irr::core::array<NewtonBody*> &);

    irr::newton::SIntersectionPoint GetCollisionFromLine(irr::newton::IBody*, irr::core::line3df &);

    void ThrowTestCube() {
      PhysicsWorld->getUtils()->launchCube(2.4f);
    }

    irr::newton::IWorld* GetWorld() { return PhysicsWorld; }


    void setGravity(irr::core::vector3df &newGravity) { gravity = newGravity; }

    void setUpVector(irr::core::vector3df &up) { upVector = up; }

    irr::core::vector3df getGravity() { return gravity; }

    irr::core::vector3df getUpVector() { return upVector; }

    irr::newton::IMaterial* playerMaterial, *worldGeomMaterial;


    irr::core::array<NewtonBody*> character_ignore;

    irr::core::array<SCharacterCollisionObject> characterCollisionObjects;

    void removeCharacterCollisionObjectsFromWorld();
    void restoreCharacterCollisionObjects();

  private:

    irr::core::vector3df gravity, upVector;
    irr::newton::IWorld* PhysicsWorld;
    irr::newton::ICollisionManager *PhysicsCollisionManager;
#endif
  };

}

#endif
