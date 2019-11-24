

#ifndef __SPhysicsStructs_h
#define __SPhysicsStructs_h



#include "PhysicsGlobal.h"

#include "EPhysicsEnums.h"

/// Attributes struct.
//! These attributes are shared by all the primitive shapes. This struct just contains common data - the shape classes derive
//! from this and extend it; you can't create a physics entity from this alone.
struct SPhysicsAttributes
{
	/// Mass of the body.
	f32 mass;
	/// The type of body - either EBT_DYNAMIC (affected by collisions and gravity) or EBT_STATIC (solid but unaffected by collisions or gravity).
	ENUMERATED_BODY_TYPE bodyType;
	/// A pointer to the Irrlicht scene node. If there are several then this should be the root node, and the others added to it as child nodes.
	ISceneNode* node;
};

/// Cube struct.
//! Use this for creating a cube.
struct SPhysicsCube : SPhysicsAttributes
{
	/// Size in x axis.
	f32 size_x;
	/// Size in y axis.
	f32 size_y;
	/// Size in z axis.
	f32 size_z;
};

/// Sphere struct.
//! Use this for creating a sphere.
struct SPhysicsSphere : SPhysicsAttributes
{
	/// Radius in x axis.
	f32 radius_x;
	/// Radius in y axis.
	f32 radius_y;
	/// Radius in z axis.
	f32 radius_z;
};

/// Terrain struct.
//! Use this for creating terrain.
struct SPhysicsTerrain
{
	/// Pointer to Irrlicht ITerrainSceneNode.
	ITerrainSceneNode *terrainNode;
	/// Terrain scale. When IPhysics creates the terrain, it will apply this scale to the Irrlicht node as well as the Newton mesh, so you don't have to worry about that.
	/// However, you do need to ensure that the terrain scale is set to something, otherwise your terrain will be created with a scale of (0,0,0) and so will be invisible.
	/// If you want it to be the actual size of the mesh, set it to (1, 1, 1).
	vector3df terrainScale;
};

/// Static Mesh struct.
//! Use this for creating static meshes, for example rooms and levels.
struct SPhysicsStaticMesh
{
	/// Pointer to the Irrlicht IAnimatedMesh that the scene node was created from.
	IAnimatedMesh* mesh;
	/// Mesh scale. See notes for ::SPhysicsTerrain.
	vector3df meshScale;
	/// Pointer to the scene node.
	ISceneNode* meshnode;
};

/// Car struct.
//! Use this for creating a car. It is important to remember that cars in IPhysics <B>must</B> be constructed with the positive y-axis as up, and the vehicle pointing in the <B>negative</B> x-axis.
//! The reason for this is that otherwise a positive torque makes the car go backwards, which is far more confusing and messy than just having to construct the car this way.
//! It is also assumed that you are building the car around the origin.
//! This means you'll probably need to orient your car and wheel meshes in your modelling program to meet this criteria, although rest assured this system will become more flexible in future versions.
struct SPhysicsCar
{
	/// The car body's offset from the origin. For example, if you want the car body to be 5 units above the ground, this should be (0, 5.0f, 0).
	vector3df carBodyOffset;
	/// The car body is currently modelled as a box, with these dimensions. This will be changed soon!
	vector3df carBodySize;
	/// The car's mass. Note that this is the mass of the car body, not the tires.
	f32 carMass;

	/// The front axle's offset from the origin. This will determine how far forwards the front wheels are.
	f32 frontAxleOffset;
	/// The rear axle's offset from the origin. This should be a positive number. This will determine how far back the rear wheels are.
	f32 rearAxleOffset;

	/// The axle width. The same value will be used for both front and back axles. This will determine how far apart the left and right wheels are.
	f32 axleWidth;

	/// The mass of a tire.
	f32 tireMass;

	/// The width of a tire.
	f32 tireWidth;
	/// The radius of a tire.
	f32 tireRadius;

	/// The scene node for the car body.
	ISceneNode* carBodyNode;
	/// The scene node for the <B>front left</B> tire.
	ISceneNode* tireNode_FL;
	/// The scene node for the <B>front right</B> tire.
	ISceneNode* tireNode_FR;
	/// The scene node for the <B>rear left</B> tire.
	ISceneNode* tireNode_RL;
	/// The scene node for the <B>rear right</B> tire.
	ISceneNode* tireNode_RR;

	/// The tire suspension shock - this is passed straight to Newton. From the Newton docs: "parametrized damping constant for a spring, mass, damper system. A value of one corresponds to a critically damped system."
	f32 tireSuspensionShock;
	/// The tire suspension spring - this is passed straight to Newton. From the Newton docs: "parametrized spring constant for a spring, mass, damper system. A value of one corresponds to a critically damped system."
	f32 tireSuspensionSpring; 
	/// The tire suspension length - this is passed straight to Newton. From the Newton docs: "distance from the tire set position to the upper stop on the vehicle body frame. The total suspension length is twice that."
	f32 tireSuspensionLength;

	/// The max steering angle the car is able to make. A higher number means the car will be able to make tighter turns.
	f32 maxSteerAngle;
	/// The max torque that the car can produce. A higher number means that the car gan go faster, but too high and it will start to skid and lose control.
	f32 maxTorque;
	/// The max brakes that the car has. A higher number means stronger brakes.
	f32 maxBrakes;
};

#endif