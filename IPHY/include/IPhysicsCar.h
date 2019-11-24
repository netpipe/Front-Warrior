

#ifndef __IPhysicsCar_h
#define __IPhysicsCar_h


#include "PhysicsGlobal.h"


#include "IPhysicsBaseEntity.h"

/// Car interface.
//! This interface provides functionality for controlling the car.
class IPhysicsCar : public IPhysicsBaseEntity
{
public:
	/// Set position.
	virtual void setPosition(vector3df position) = 0;
	/// Set rotation.
	virtual void setRotation(vector3df rotation) = 0;
	/// Get position.
	virtual vector3df getPosition() = 0;
	/// Get rotation.
	virtual vector3df getRotation() = 0;
	/// Get a pointer to the Newton body. This will be the body of the vehicle hull, not of the wheels. You should only need this if you want to make direct calls to Newton.
	virtual NewtonBody* getBody() = 0;
	/// Get the entity type, which in this case will be ::EET_CAR_ENTITY.
	virtual ENUMERATED_ENTITY_TYPE getEntityType() = 0;
	/// Set the steering as a percentage of the maximum steering value that was specified when the car was created. Use a negative value for turning left, positive for turning right. If you set this to 0, the wheels will be straight. This will be clamped at -100 and 100.
	virtual void setSteeringPercent(f32 steeringPercent) = 0;
	/// Set the throttle as a percentage of the maximum throttle value that was specified when the car was created. Use a positive value for going forwards, negative for reversing. If you set this to 0, the car will come to a halt. This will be clamped at -100 and 100.
	virtual void setThrottlePercent(f32 throttlePercent) = 0;
	/// Set the brakes as a percentage of the maximum brakes value that was specified when the car was created. This will be clamped at 0 and 100.
	virtual void setBrakesPercent(f32 brakesPercent) = 0;
};


#endif