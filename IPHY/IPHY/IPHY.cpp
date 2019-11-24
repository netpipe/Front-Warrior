//#include "stdafx.h"
#include <irrlicht.h>
#include <Newton.h>
#include <IPhysics.h>
#include "EventReciever.h"

#pragma comment (lib , "irrlicht.lib")
#pragma comment (lib , "newton.lib")

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


core::stringw MessageText = "Instructions";
core::stringw Caption = " Press UP , DOWN , LEFT , RIGHT buttons to drive the car \n Press F2 or F3 to change the camera mode \n Press V button to flib the car\n Press C to drop a ball";


/*
* Function to load the car
*/
IAnimatedMeshSceneNode* LoadCar(scene::ISceneManager* smgr )
{
	//
	char* carModel = "../media/Vehicle/impreza.3ds";
	IAnimatedMesh* carMesh = smgr->getMesh(carModel);
	IAnimatedMeshSceneNode* carSceneNode = smgr->addAnimatedMeshSceneNode(carMesh);
	carSceneNode->setMaterialFlag(video::EMF_LIGHTING, false);
	return carSceneNode;
}

/*
* Function to load the car wheels
*/
IAnimatedMeshSceneNode* LoadCarWheel(scene::ISceneManager* smgr , video::IVideoDriver* driver)
{
	char* carWheelModel = "../media/Model2/tire.3ds";
	IAnimatedMesh* carWheel = smgr->getMesh(carWheelModel);
	IAnimatedMeshSceneNode* carWheelNode = smgr->addAnimatedMeshSceneNode(carWheel);
	carWheelNode->setMaterialFlag(video::EMF_LIGHTING ,false);

	return carWheelNode;
}



// Main function

int main()
{
	// the map model
	char* pk3Map = "../media/map-20kdm2.pk3";
	char* mapName = "20kdm2.bsp";

	// make direct3d9 our device
	IrrlichtDevice* device = createDevice( EDT_OPENGL);
//	if (device == 0)
//		return ;

	// get the video driver
	video::IVideoDriver* driver = device->getVideoDriver();
	// get the scene manager
	scene::ISceneManager* smgr = device->getSceneManager();

	// make a camera scene node and set its position
	ICameraSceneNode* camera = smgr->addCameraSceneNode();
	camera->setPosition(vector3df(0, 30.0f, -30.0f));

	// tell the user a message
	device->getGUIEnvironment()->addMessageBox(MessageText.c_str(),Caption.c_str());

	//Load the game map
	device->getFileSystem()->addZipFileArchive(pk3Map);
	IAnimatedMesh* mapMesh = smgr->getMesh(mapName);
	IAnimatedMeshSceneNode* gameMap = smgr->addAnimatedMeshSceneNode(mapMesh);
	/////////////////////////////////////////////////////////////////////////////////////////

	// init the physics world
	CPhysics physics;
	physics.init(device->getTimer());

	// init the car parameter
	SPhysicsCar carData ;

	carData.carBodyOffset = vector3df(0, 0.0f, 0);
	carData.carBodySize = vector3df(1.2f, 01.85f, 0.2f);
	carData.carMass = 3000.0f;
	carData.frontAxleOffset = 01.5f;
	carData.rearAxleOffset = 01.1f;
	carData.axleWidth = 01.7f;
	carData.tireMass = 20.0f;
	carData.tireRadius = 0.98f;
	carData.tireWidth = 01.0f;
	carData.maxSteerAngle = 0.6f;
	carData.maxTorque = 2000.0f;
	carData.maxBrakes = 50.0f;


	carData.tireSuspensionLength = 0.20f;
	carData.tireSuspensionSpring = (carData.tireMass * 1.0f * 9.8f) / carData.tireSuspensionLength;
	carData.tireSuspensionShock = sqrt(carData.tireSuspensionSpring) * 1.0f;

	carData.carBodyNode = LoadCar(smgr);
	carData.carBodyNode->setScale(vector3df(.943,.943,.943));

	carData.tireNode_FL = LoadCarWheel(smgr ,driver);
	carData.tireNode_FL->setScale(vector3df(carData.tireRadius, carData.tireRadius, carData.tireWidth));

	carData.tireNode_FR = LoadCarWheel(smgr ,driver);
	carData.tireNode_FR->setScale(vector3df(carData.tireRadius, carData.tireRadius, carData.tireWidth));

	carData.tireNode_RL = LoadCarWheel(smgr ,driver);
	carData.tireNode_RL->setScale(vector3df(carData.tireRadius, carData.tireRadius, carData.tireWidth));

	carData.tireNode_RR = LoadCarWheel(smgr ,driver);
	carData.tireNode_RR->setScale(vector3df(carData.tireRadius, carData.tireRadius, carData.tireWidth));

	IPhysicsCar* car = physics.addCar(&carData);
	car->setPosition(vector3df(140, 10.0f, 130.0f));
	//car->setPosition(vector3df(0, 30.0f, 10.0f));

	// make a CEventReceiver object
	CEventReceiver receiver(car , camera , smgr , driver , physics);
	// add it to the device
	device->setEventReceiver(&receiver);

	// add the game map to the physics world
	SPhysicsStaticMesh level;
	level.mesh = mapMesh;
	level.meshnode = gameMap;
	level.meshScale = vector3df(0.1f, 0.1f, 0.1f);
	level.meshnode->setScale(level.meshScale);

	IPhysicsEntity* levelEntity = physics.addEntity(&level);
	// init the camera position
	camera->setPosition(vector3df(carData.carBodyNode->getPosition().X,carData.carBodyNode->getPosition().Y+3,carData.carBodyNode->getPosition().Z + 7));

	// the game main loop
 	while(device->run())
	{
		driver->beginScene(true, true, SColor(255,100,101,140));

		camera->setTarget(vector3df(carData.carBodyNode->getPosition().X,carData.carBodyNode->getPosition().Y+3,carData.carBodyNode->getPosition().Z  ));

		// switch between the two cameras
		if(staticCamera)
			camera->setPosition(vector3df(carData.carBodyNode->getPosition().X,carData.carBodyNode->getPosition().Y+3,carData.carBodyNode->getPosition().Z + 7));

		smgr->drawAll();

		device->getGUIEnvironment()->drawAll(); // draw the gui environment (the logo)

		driver->endScene();
		physics.update();
	}

	device->drop();

}

