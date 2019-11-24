#ifndef NEWTON_PHYSICS_BODY
#define NEWTON_PHYSICS_BODY

#include <irrlicht.h>
#include <Newton.h>

namespace engine {
namespace physics {

  /*void body_DestroyCallback(const NewtonBody* body);

  void body_SetTransformCallback(
    const NewtonBody* body,
    const float* matrix,
    int threadIndex);

  void body_ApplyForceAndTorqueCallback(
    const NewtonBody* newtonBody,
    float timestep,
    int threadIndex);*/

  class CPhysicsWorld;

  enum E_BODY_TYPE
  {
    EBT_TREE_COLLISION = 0,
    EBT_CONVEX_HULL = 1,
    EBT_CONVEX_HULL_MODIFIER,
    EBT_PRIMITIVE_BOX,
    EBT_PRIMITIVE_SPHERE,
    EBT_PRIMITIVE_CAPSULE,
    EBT_NULL
  };

  struct SBodyCreationParameters
  {
    irr::scene::ISceneNode * node;
    irr::scene::IMesh * mesh;
    E_BODY_TYPE type;
    irr::u32 bodyID;
    irr::f32 mass;
    irr::core::vector3df scale;
    irr::core::vector3df offset;

    SBodyCreationParameters()
    {
      scale.set(0,0,0);
      offset.set(0,0,0);
    }
  };

  class CBody
  {
  public:

    CBody(irr::u32 shapeId, NewtonWorld * world)
    {
      m_ShapeID = shapeId;
      m_NewtonBody = NULL;
      m_CurrentCollisionIndex = 0;
      a_NewtonCollisions.set_used(0);
      a_NewtonJoints.set_used(0);
      b_GravityEnabled = true;
      m_NewtonWorld = world;
    }

    /*
      SET methods
    */

    void setPosition(irr::core::vector3df position);

    void setRotation(irr::core::vector3df rotation);

    void setScale(irr::core::vector3df scale) { }

    inline void setMass(irr::f32 mass) { m_Mass = mass; }

    inline void setNewtonBody(NewtonBody * newtonBody) { m_NewtonBody = newtonBody; }

    inline void setNode(irr::scene::ISceneNode *node) { m_Node = node; }

    void setContinuousCollisionMode(bool value);

		inline void setUserData(void* userData) { m_UserData = userData; }

		void setUsedCollision(irr::u32 id);

    void setVelocity(irr::core::vector3df);

    inline void setForce(irr::core::vector3df force) { m_Force = force; }

    inline void setOriginalPosition(irr::core::vector3df pos) { m_OriginalPosition = pos; }

    void setOmega(irr::core::vector3df);

    inline void setGravityEnabled(bool grav) { b_GravityEnabled = grav; }

    /*
      GET methods
    */

		void* getUserData() { return m_UserData; }

    inline irr::scene::ISceneNode *getNode() { return m_Node; }

    inline NewtonBody * getNewtonBody() { return m_NewtonBody; }

    inline NewtonCollision* getCurrentCollision() { return a_NewtonCollisions[m_CurrentCollisionIndex]; }

    inline irr::f32 getMass() { return m_Mass; }

    inline irr::u32 getShapeID() { return m_ShapeID; }

    irr::core::vector3df getVelocity();

    irr::core::vector3df getOmega();

    inline irr::core::vector3df getForce() { return m_Force; }

    inline irr::core::vector3df getPosition() { return m_Node->getPosition(); }

    inline irr::core::vector3df getOriginalPosition() { return m_OriginalPosition; }

    irr::core::vector3df getPositionBody();

    bool isGravityEnabled() { return b_GravityEnabled; }



    void drawDebug(irr::video::IVideoDriver *driver);

		void addCollision(NewtonCollision *col) { a_NewtonCollisions.push_back(col); }

    void removeBody();

    void setAndCalculateMass();

    void createUpVectorConstraint(irr::core::vector3df upv);

  private:

    irr::f32 m_Mass;

    irr::u32 m_ShapeID;

    irr::u32 m_CurrentCollisionIndex;

    irr::scene::ISceneNode * m_Node;

    irr::core::array<NewtonCollision*> a_NewtonCollisions;

    irr::core::array<NewtonJoint*> a_NewtonJoints;

    NewtonBody * m_NewtonBody;

    NewtonWorld * m_NewtonWorld;

    irr::core::vector3df m_Force;

    irr::core::vector3df m_OriginalPosition;

    bool b_GravityEnabled;

    void *m_UserData;
  };

}
}

#endif
