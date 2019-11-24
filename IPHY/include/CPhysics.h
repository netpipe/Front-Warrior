




#ifndef __CPhysics_h
#define __CPhysics_h

#include "PhysicsGlobal.h"
#include "SPhysicsStructs.h"
#include "CPhysicsEntity.h"
#include "MPhysicsCallbacks.h"
#include "IPhysicsEntity.h"
#include "EPhysicsEnums.h"


#include "CPhysicsCar.h"




/// Root class.
/// All functionality for IPhysics is accessed through this class.
class CPhysics
{
public:
	CPhysics(); 
	~CPhysics();
	void init(ITimer* timer);
	void update();
	void update(f32 timestep_ms);
	void close();
	NewtonWorld *getWorld();
	IPhysicsEntity* addEntity(SPhysicsCube* cube);
	IPhysicsEntity* addEntity(SPhysicsSphere* sphere);
	IPhysicsEntity* addEntity(SPhysicsTerrain* terrain);
	IPhysicsEntity* addEntity(SPhysicsStaticMesh* staticMesh);
	IPhysicsCar* addCar(SPhysicsCar *car);
	IPhysicsEntity* dropTestCube(ISceneManager* smgr, vector3df fromPosition);
	IPhysicsEntity* dropTestCube(ISceneManager* smgr, vector3df fromPosition, f32 size, f32 mass);
	IPhysicsEntity* dropTestSphere(ISceneManager* smgr, vector3df fromPosition);
	IPhysicsEntity* dropTestSphere(ISceneManager* smgr, vector3df fromPosition, f32 size, f32 mass);
	void setUpdateMode(PHYSICS_UPDATE_MODE updateMode);
	void setUpdatesPerSecond(s32 updatesPerSecond);
	void setWorldSize(vector3df sizeMin, vector3df sizeMax);
private:
	IPhysicsEntity* setUpRigidBody(NewtonBody *body, SPhysicsAttributes* attributes);
	void addMeshToTreeCollisionStandard(IMeshBuffer* meshBuffer, NewtonCollision* treeCollision, SPhysicsStaticMesh* staticMesh);
	void addMeshToTreeCollision2TCoords(IMeshBuffer* meshBuffer, NewtonCollision* treeCollision, SPhysicsStaticMesh* staticMesh);
	void addMeshToTreeCollisionTangents(IMeshBuffer* meshBuffer, NewtonCollision* treeCollision, SPhysicsStaticMesh* staticMesh);
	NewtonWorld *m_world;
	list<IPhysicsBaseEntity*> m_entityList;
	// timestep stuff
	f32 m_timestepAccumulator;
	ITimer* m_timer;
	f32 m_oldTime;
	PHYSICS_UPDATE_MODE m_updateMode;
	s32 m_updatesPerSecond;
	
};


#endif