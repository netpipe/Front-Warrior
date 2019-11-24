


#ifndef __IPhysicsEntity_h
#define __IPhysicsEntity_h


#include "PhysicsGlobal.h"
#include "IPhysicsBaseEntity.h"

/// Entity interface.
//! This is the interface for controlling all basic physics entities, from cubes to terrains.
class IPhysicsEntity : public IPhysicsBaseEntity
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
	/// Get the entity type, which in this case will be ::EET_BASIC_ENTITY.
	virtual ENUMERATED_ENTITY_TYPE getEntityType() = 0; 
};



#endif