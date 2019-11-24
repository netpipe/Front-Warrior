

#ifndef __EPhysicsEnums_h
#define __EPhysicsEnums_h


enum ENUMERATED_BODY_TYPE
{
	EBT_STATIC,
	EBT_DYNAMIC,
	EBT_MAX_TYPES
};

enum ENUMERATED_ENTITY_TYPE
{
	EET_BASIC_ENTITY,
	EET_CAR_ENTITY,
	EET_MAX_TYPES
};


/// PHYSICS_UPDATE_MODE
//! Use these flags with get/setUpdateMode().
enum PHYSICS_UPDATE_MODE
{
	/// Newton will be updated at a constant frame rate. The default is 60FPS; you can change this with ::setUpdatesPerSecond().
	UM_FIXED_FRAME_RATE, 
	/// Newton will be updated every frame, using the correct timestep. 
	UM_EVERY_FRAME, 
	UM_MAX_TYPES
};


#endif