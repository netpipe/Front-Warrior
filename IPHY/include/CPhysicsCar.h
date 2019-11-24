





#ifndef __CPhysicsCar_h
#define __CPhysicsCar_h

#include "PhysicsGlobal.h"
#include "CPhysicsCarWheel.h"
#include "MPhysicsCallbacks.h"
#include "IPhysicsBaseEntity.h"
#include "SPhysicsStructs.h"
#include "IPhysicsCar.h"



class CPhysicsCar : public IPhysicsCar
{
public:
    CPhysicsCar();
    ~CPhysicsCar();

    CPhysicsCarWheel wheel_FL;
    CPhysicsCarWheel wheel_FR;
    CPhysicsCarWheel wheel_RR;
    CPhysicsCarWheel wheel_RL;

    void assembleCarPhysics(SPhysicsCar* car, NewtonWorld* world);

	// driving interface
	void setSteeringPercent(f32 steeringPercent);
	void setThrottlePercent(f32 throttlePercent);
	void setBrakesPercent(f32 brakesPercent);

	// set
    void setSteering(f32 steerangle);
    void setTorque(f32 torque);
    void setBrakes(f32 brakes);
	void setVehicleSpeed(f32 speed);

	// get
	ISceneNode* getNode();
	NewtonJoint* getVehicleJoint();

	// base class set
	void setPosition(vector3df position);
	void setRotation(vector3df rotation);

	// base class get
	vector3df getPosition();
	vector3df getRotation();
	NewtonBody* getBody();
	ENUMERATED_ENTITY_TYPE getEntityType();

protected:
	void cap(f32 limit, f32* target, bool positiveAndNegative);
    ISceneNode *m_carNode;
	f32 m_vehicleSpeed;
    NewtonBody *m_carBody;
    NewtonJoint *m_vehicleJoint;
	f32 m_maxTorque;
	f32 m_maxSteerAngle;
	f32 m_maxBrakes;
};

  


#endif