

#ifndef __CPhysicsCarWheel_h
#define __CPhysicsCarWheel_h

#include "PhysicsGlobal.h"
#include "SPhysicsStructs.h"

//void _cdecl PhysicsCarWheelUpdate(const NewtonJoint *vehicle);


class CPhysicsCarWheel
{
public:
    CPhysicsCarWheel();
    ~CPhysicsCarWheel();

    void rigupPhysics(NewtonJoint *vehicle, vector3df wheelPosition, SPhysicsCar* car, CPhysicsCarWheel *wheelpointer, s32 tyreid);
    void setTirePhysics(const NewtonJoint *vehicle, void* id);

	// get
	ISceneNode* getNode();

	// set
    void setSteer(f32 steerAngle);
    void setTorque(f32 torque);
    void setBrakes(f32 brakes);
	void setNode(ISceneNode* node);
protected:
    f32 m_steerAngle;
    f32 m_radius;
    f32 m_torque;
    f32 m_brakes;
    ISceneNode *m_wheelNode;
};


//class CPhysicsCarSteerWheel : CPhysicsCarWheel
//{
//public:
//
//};
//
//class CPhysicsCarDriveWheel : CPhysicsCarWheel
//{
//public:
//};



#endif
