#ifndef NEWTON_PHYSICS_WORLD_HEADER
#define NEWTON_PHYSICS_WORLD_HEADER

#include <Newton.h>

#include "CompileConfig.h"
#include "newton/Body.h"
#include "newton/Collision.h"

namespace engine {
namespace physics {

  const irr::u32 DEMO_PHYSICS_FPS = 70;
  const irr::u32 DEMO_FPS_IN_MICROSECUNDS = irr::u32(1000/DEMO_PHYSICS_FPS);
  const irr::u32 MAX_PHYSICS_LOOPS = 1;

  //!Convert a position from newton to irrlicht
  const irr::f32 NewtonToIrr = 32.0f;

  //!Convert a position from irrlicht to newton
  const irr::f32 IrrToNewton = (1.0f / NewtonToIrr);

  inline void fillVec3(
      irr::core::vector3df vector,
      irr::f32* array) {
    array[0]=vector.X;
    array[1]=vector.Y;
    array[2]=vector.Z;
  }

  inline irr::f32* getMatrixPointer(const irr::core::matrix4& mat) {
    return const_cast<irr::f32*>(mat.pointer());
  }

  /*void body_DestroyCallback(const NewtonBody* body);

  void body_SetTransformCallback(
    const NewtonBody* newtonBody,
    const float* matrix,
    int threadIndex);

  void body_ApplyForceAndTorqueCallback(
    const NewtonBody* newtonBody,
    float timestep,
    int threadIndex);*/

  struct SRayCastParameters
  {
    irr::core::line3df line;
    irr::core::array<irr::u32> excluded;

    SRayCastParameters()
    {
      excluded.set_used(0);
    }
  };

  struct SConvexCastParameters
  {
    CBody * body;

    irr::core::vector3df offset;

    irr::core::array<irr::u32> excluded;

    SConvexCastParameters()
    {
      excluded.set_used(0);
    }
  };

  struct SRayCastResult
  {
    CBody *body;
    irr::core::vector3df normal, position;
    irr::f32 distance;
  };

  struct SConvexCastResultSingle
  {
    CBody *body;
    irr::core::vector3df normal, normalWorld, position;
    irr::f32 distance;
    irr::u32 contactID;
    irr::f32 param;
  };

  struct SConvexCastResult
  {
    irr::core::array<SConvexCastResultSingle> results;
    irr::f32 param;
  };

  class CPhysicsWorld
  {
  public:

    CPhysicsWorld(irr::ITimer * timer)
    {
      m_Timer = timer;
      g_timeAccumulator = DEMO_FPS_IN_MICROSECUNDS;

      all_bodies.set_used(0);

      m_UniqueBodyID = 0;

      accumulated_time = 0;
      update_fps = 0;
      time_elapsed = 0;
    }

    ~CPhysicsWorld();

    void advanceSimulation(irr::u32 timeInMilisecunds);

    void advanceSimulation2();

    void createNewtonWorld();

    void advanceSimulation3(irr::f32 time);

    NewtonWorld* getNewtonWorld() { return m_NewtonWorld; }

    void closeNewtonWorld();

    void clear();

    CBody * createBody(SBodyCreationParameters params);

    NewtonCollision * createCollisionFromBodyParameters(SBodyCreationParameters params);

    SRayCastResult getRayCollision(SRayCastParameters);

    SConvexCastResult getConvexCollision(SConvexCastParameters);

    void drawDebug(irr::video::IVideoDriver *driver, NewtonBody *Body);

    irr::core::vector3df calculateToqueAtPoint(
      const NewtonBody* body,
      const irr::core::vector3df& point,
      const irr::core::vector3df& force);

    irr::u32 getUniqueBodyID() { return m_UniqueBodyID++; }

    CCollisionManager * getCollisionManager() { return &collisionManager; }

  protected:

    NewtonWorld* m_NewtonWorld;

    irr::ITimer * m_Timer;

    CCollisionManager collisionManager;

    int g_currentTime;

    int g_physicTime;

    int g_timeAccumulator;


    irr::f32 accumulated_time;
    irr::f32 update_fps;
    irr::f32 time_elapsed;

    irr::core::array<CBody*> all_bodies;

    irr::u32 m_UniqueBodyID;
  };

} // physics namespace
} // engine namespace

#endif
