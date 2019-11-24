#ifndef NEWTON_PHYSICS_COLLISION_HEADER
#define NEWTON_PHYSICS_COLLISION_HEADER

#include "irrlicht.h"
#include "Newton.h"

namespace engine {
namespace physics {

class CCollisionManager
{
public:

  NewtonCollision* createConvexTree(
    NewtonWorld *world,
    irr::scene::IMesh * mesh,
    irr::core::vector3df scale,
    irr::u32 shapeId);

  NewtonCollision* createConvexHull(
    NewtonWorld * world,
    irr::scene::IMesh * mesh,
    irr::core::vector3df scale,
    irr::u32 shapeId);

  NewtonCollision* createNewtonPrimitive(
    NewtonWorld *world,
    engine::physics::E_BODY_TYPE type,
    irr::scene::ISceneNode *node,
    irr::core::vector3df scale,
    irr::core::vector3df offsetPos,
    irr::u32 shapeId);

  void releaseCollision(NewtonWorld *world, NewtonCollision *collision);

  protected:

    void addMeshToTreeCollisionTangents(
      irr::scene::IMeshBuffer* meshBuffer,
      NewtonCollision* treeCollision,
      irr::core::vector3df scale);

    void addMeshToTreeCollisionStandard(
      irr::scene::IMeshBuffer* meshBuffer,
      NewtonCollision* treeCollision,
      irr::core::vector3df scale);

    void addMeshToTreeCollision2TCoords(
      irr::scene::IMeshBuffer* meshBuffer,
      NewtonCollision* treeCollision,
      irr::core::vector3df scale);

};

}
}

#endif
