

#ifndef __IPhysicsBaseEntity_h
#define __IPhysicsBaseEntity_h

#include "PhysicsGlobal.h"
#include "EPhysicsEnums.h"

/// The base class for all physics entities.
//! All of the entities in IPhysics derive from this base class, so the functions detailed here will always be available.
//! This is useful if you want to have a list of different entity types, and you can use getEntityType() to determine
//! what derived class it is safe to cast to.
class IPhysicsBaseEntity
{
public:
	/// Set the position.
	virtual void setPosition(vector3df position) = 0; 
	/// Set the rotation.
	virtual void setRotation(vector3df rotation) = 0; 
	/// Get position.
	virtual vector3df getPosition() = 0; 
	/// Get rotation.
	virtual vector3df getRotation() = 0; 
	/// Get a pointer to the Newton body. You should only need this if you need to make calls directly to Newton.
	virtual NewtonBody* getBody() = 0; 
	/// Get the body type. You can use this to then make a safe cast to one of the derived classes of entity.
	virtual ENUMERATED_ENTITY_TYPE getEntityType() = 0;
};


#endif