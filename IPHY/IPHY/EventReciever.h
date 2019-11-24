


// boolean for changing the camera mode
bool staticCamera = true;



/**
* Function to drop a dynamic sphere just above the car  
*/
void dropDynSphere(scene::ISceneManager* smgr ,video::IVideoDriver* driver, CPhysics physics,IPhysicsCar* car)
{
	// get the active camera 
	ICameraSceneNode* camera = smgr->getActiveCamera();

	// make a sphere node 
	ISceneNode* dynamicShereNode = smgr->addSphereSceneNode(0.70f);
	// make the sphere node take the ball shap by adding a beach ball texture
	dynamicShereNode->setMaterialFlag(video::EMF_LIGHTING, false);
	ITexture * ballTexture = driver->getTexture("../media/Balls/BeachBallColor.jpg");
	dynamicShereNode->setMaterialTexture(0,ballTexture);
	
	// make a dynamic sphere 
	SPhysicsSphere dynamicSphere;
	dynamicSphere.bodyType = EBT_DYNAMIC;
	dynamicSphere.mass = 1.1;
	dynamicSphere.radius_x = .70f;
	dynamicSphere.radius_y = .70f;
	dynamicSphere.radius_z = .70f;
	dynamicSphere.node = dynamicShereNode;

	// add the ball to the physics world
	IPhysicsEntity* dynamicSphereEntity = physics.addEntity(&dynamicSphere);
	// set the ball position just above the car
	dynamicSphereEntity->setPosition(vector3df(car->getPosition().X, car->getPosition().Y+ 12, car->getPosition().Z));
}


/*
* CEventReciver class needed for handling the keyboard or mouse events 
* we can overide the OnEvent function to make our custome events 
*/

class CEventReceiver : public IEventReceiver
{
public:
	// This is the one method that we have to implement
	virtual bool OnEvent(const SEvent& event)
	{
		if(event.EventType == EET_KEY_INPUT_EVENT)
		{
			// if user press UP arrow
			if(event.KeyInput.Key == KEY_UP)
			{
				if(!m_keys[KEY_UP])
				{
					m_keys[KEY_UP] = true;
					// go forward
					m_car->setThrottlePercent(100.0f);
				}
				else if(event.KeyInput.PressedDown == false)
				{
					m_keys[KEY_UP] = false;
				}
			}
			// if user press Down arrow
			if(event.KeyInput.Key == KEY_DOWN)
			{
				if(!m_keys[KEY_DOWN])
				{
					m_keys[KEY_DOWN] = true;
					// go backword
					m_car->setThrottlePercent(-100.0f);
				}
				else if(event.KeyInput.PressedDown == false)
				{
					m_keys[KEY_DOWN] = false;
				}
			}
			// if user press Left arrow
			if(event.KeyInput.Key == KEY_LEFT)
			{
				if(!m_keys[KEY_LEFT])
				{
					m_keys[KEY_LEFT] = true;
					// go left
					m_car->setSteeringPercent(-100.0f);

				}
				else if(event.KeyInput.PressedDown == false)
				{
					m_keys[KEY_LEFT] = false;
				}
			}
			// if user press right arrow
			if(event.KeyInput.Key == KEY_RIGHT)
			{
				if(!m_keys[KEY_RIGHT])
				{
					m_keys[KEY_RIGHT] = true;
					// go right
					m_car->setSteeringPercent(100.0f);
				}
				else if(event.KeyInput.PressedDown == false)
				{
					m_keys[KEY_RIGHT] = false;
				}
			}
			if(!m_keys[KEY_LEFT] && !m_keys[KEY_RIGHT])
			{
				m_car->setSteeringPercent(0.0f);
			}

			if(!m_keys[KEY_UP] && !m_keys[KEY_DOWN])
			{
				m_car->setThrottlePercent(0.0f);

			}
			// if user press C button
			if(event.KeyInput.Key == KEY_KEY_C)
			{
				// drop the sphere 
				dropDynSphere(m_smgr,m_driver,m_physics,m_car);	
			} // if user press V button
			else if(event.KeyInput.Key == KEY_KEY_V)
			{
				// flib the car position
				m_car->setPosition(vector3df(m_car->getPosition().X , m_car->getPosition().Y + 3 ,m_car->getPosition().Z));
				m_car->setRotation(vector3df(m_car->getRotation().X +90,m_car->getRotation().Y,m_car->getRotation().Z));
			}
			else if(event.KeyInput.Key == KEY_F3)
			{
				// active the static camera
				staticCamera= true;
			}
			else if(event.KeyInput.Key == KEY_F2)
			{
				// active the dynamic camera
				staticCamera = false;
			}

		}
		return false;
	}
	// This is used to check whether a key is being held down
	virtual bool IsKeyDown(EKEY_CODE keyCode) const
	{
		return KeyIsDown[keyCode];
	}

	// the default class constructor
	CEventReceiver()
	{	
		for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
			KeyIsDown[i] = false;
	}
	// the custom class constructor
	CEventReceiver(IPhysicsCar* car , ICameraSceneNode* camera , scene::ISceneManager* smgr ,video::IVideoDriver* driver, CPhysics physics)
	{
		m_car = car;
		m_camera = camera;
		m_physics = physics;
		m_smgr = smgr;
		m_driver = driver;

		for(s32 i=0; i<KEY_KEY_CODES_COUNT; i++)
		{
			m_keys[i] = false;
		}
	}


private:
	// We use this array to store the current state of each key
	bool KeyIsDown[KEY_KEY_CODES_COUNT];
	IPhysicsCar* m_car;
	ICameraSceneNode* m_camera;
	CPhysics m_physics;
	scene::ISceneManager* m_smgr;
	video::IVideoDriver* m_driver;
	bool m_keys[KEY_KEY_CODES_COUNT];
};
