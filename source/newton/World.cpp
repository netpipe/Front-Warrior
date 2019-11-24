#include "newton/World.h"

using namespace engine::physics;

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;

CPhysicsWorld::~CPhysicsWorld()
{
}

//
// Body callbacks
//

void body_DestroyCallback(const NewtonBody* body)
{
	// for now there is nothing to destroy
}

void body_SetTransformCallback(
  const NewtonBody* newtonBody,
  const float* matrix,
  int threadIndex)
{
	CBody* my_body = (CBody*)NewtonBodyGetUserData(newtonBody);

	irr::core::matrix4 irr_mat;
	memcpy(getMatrixPointer(irr_mat), matrix, sizeof(float)*16);

	//newt_mat.setTranslation(newt_mat.getTranslation()+p_node->getRealtimeOffset().Position);
	//newt_mat.setRotationDegrees(newt_mat.getRotationDegrees()+p_node->getRealtimeOffset().Rotation);
  //if(irr_mat.getTranslation().X != irr_mat.getTranslation().X) { printf("QNAN\n"); return; }

	NewtonBodySetMatrix(newtonBody, matrix);

	//adjust translation to set with irrlicht
	irr_mat.setTranslation(irr_mat.getTranslation() * NewtonToIrr);
	irr::core::vector3df new_pos = irr_mat.getTranslation();

	my_body->getNode()->setPosition(new_pos);
	my_body->getNode()->setRotation(irr_mat.getRotationDegrees());

  // set node rotation
	my_body->getNode()->updateAbsolutePosition();
}

float dt = 0.016f;
irr::f32 time_;

// callback to apply external forces to body
void body_ApplyForceAndTorqueCallback(
  const NewtonBody* newtonBody,
  float timestep,
  int threadIndex)
{
	CBody* my_body = (CBody*)NewtonBodyGetUserData(newtonBody);

  if(!my_body) return;

  irr::f32 force_array[3];
  irr::core::vector3df forceTotal = my_body->getForce();

  if(my_body->isGravityEnabled())
  {
    forceTotal += irr::core::vector3df(0.0f, -9.8f * my_body->getMass(), 0.0f);
  }

  dt = timestep;

  //forceTotal *= 0.8f;
  //forceTotal *= timestep * 60;

  forceTotal *= (60*time_);

  fillVec3(forceTotal, force_array);

  NewtonBodySetForce(newtonBody, force_array);
}

NewtonCollision * CPhysicsWorld::createCollisionFromBodyParameters(SBodyCreationParameters params)
{
  NewtonCollision * newtonCollision;

  switch(params.type)
  {
    case EBT_TREE_COLLISION:
      newtonCollision = collisionManager.createConvexTree(
        m_NewtonWorld,
        params.mesh,
        params.scale,
        params.bodyID);
    break;

    case EBT_CONVEX_HULL:
      newtonCollision = collisionManager.createConvexHull(
        m_NewtonWorld,
        params.mesh,
        params.scale,
        params.bodyID);
    break;

    case EBT_PRIMITIVE_BOX:
    case EBT_PRIMITIVE_SPHERE:
    case EBT_PRIMITIVE_CAPSULE:
      newtonCollision = collisionManager.createNewtonPrimitive(
        m_NewtonWorld,
        params.type,
        params.node,
        params.scale,
        params.offset,
        params.bodyID);
    break;

    case EBT_NULL:
      newtonCollision = NewtonCreateNull(m_NewtonWorld);
    break;

  }

  return newtonCollision;
}


CBody * CPhysicsWorld::createBody(SBodyCreationParameters params)
{
  CBody * body = new CBody(params.bodyID, m_NewtonWorld);
  NewtonCollision * newtonCollision;
  NewtonBody * newtonBody;

  // If no scale is specified, use the node scale
  if(params.scale == irr::core::vector3df(0,0,0))
    params.scale = params.node->getScale();

  bool isDynamicBody = true;

  if(params.type == EBT_TREE_COLLISION)
    isDynamicBody = false;

  newtonCollision = createCollisionFromBodyParameters(params);

	newtonBody = NewtonCreateBody(m_NewtonWorld, newtonCollision);

  // Dynamic body
  if(isDynamicBody)
  {
    /*irr::f32 inertia_array[3], origin_array[3];

    // bodies can have a destructor.
    // this is a function callback that can be used to destroy any local data stored
    // and that need to be destroyed before the body is destroyed.
    //NewtonBodySetDestructorCallback(body, body_DestroyBodyCallback);

    irr::f32 initial_force[3] = {0,0,0};
    NewtonBodySetForce(newtonBody, initial_force);

    // we need to set the proper center of mass and inertia matrix for this body
    // the inertia matrix calculated by this function does not include the mass.
    // therefore it needs to be multiplied by the mass of the body before it is used.
    NewtonConvexCollisionCalculateInertialMatrix(
      newtonCollision,
      inertia_array,
      origin_array);

    // set the body mass matrix
    NewtonBodySetMassMatrix(
      newtonBody,
      params.mass,
      inertia_array[0],
      inertia_array[1],
      inertia_array[2]);

    // set the body origin
    NewtonBodySetCentreOfMass(newtonBody, origin_array);*/
  }

  body->setNewtonBody(newtonBody);
  body->setNode(params.node);
  body->setMass(params.mass);
  body->addCollision(newtonCollision);

  if(isDynamicBody)
  {
    body->setAndCalculateMass();

    // set the function callback to apply the external forces and torque to the body
    // the most common force is Gravity
    NewtonBodySetForceAndTorqueCallback(newtonBody, body_ApplyForceAndTorqueCallback);

    // set the function callback to set the transformation state of the graphic entity associated with this body
    // each time the body change position and orientation in the physics world
    NewtonBodySetTransformCallback(newtonBody, body_SetTransformCallback);
  }

  body->setPosition(params.node->getPosition());
  //body->setRotation(params.node->getRotation());

	NewtonBodySetUserData(newtonBody, body);

  all_bodies.push_back(body);

  return body;
}

float t = 0.0f;


float currentTime = 0.0f;
float accumulator = 0.0f;

void CPhysicsWorld::advanceSimulation2()
{
  const float newTime = m_Timer->getRealTime()/1000.f;
  float deltaTime = newTime - currentTime;
  currentTime = newTime;

  // min 30 fps
  if (deltaTime>0.033f)
    deltaTime = 0.033f;

  accumulator += deltaTime;

  while(accumulator>=dt)
  {
    accumulator -= dt;
    //previous = current;

    irr::f32 dt_tmp = dt;

    NewtonUpdate(m_NewtonWorld, dt);

    //integrate(current, t, dt);
    t += dt_tmp;
  }

	/*//time releated
	static bool first_time=true;
	static irr::f32 last_time=0;

	if(first_time) {
		first_time=false;
		last_time=(irr::f32)m_Timer->getRealTime();
		return;
	}

	time_elapsed=((irr::f32)m_Timer->getRealTime())-last_time;
	irr::f32 milliseconds=time_elapsed;

	time_elapsed/=1000.0f; //convert to seconds

	last_time=m_Timer->getRealTime();
	//------------------------------------------------------------------
	//-------------end----------------------------------------------
	//----------------------------------------------------------------


	//update
	//for too slow machine fix
	if (milliseconds>100.0f) milliseconds = 100.0f;

	irr::f32 newton_time_step = (1000.0f / (irr::f32)70);

	 // add the number of millisconds passed to our accumlative total
	accumulated_time += milliseconds;

	while (accumulated_time > newton_time_step)
   {
      NewtonUpdate(m_NewtonWorld, (newton_time_step*1.0e-3f));   // convert to seconds
      accumulated_time -= newton_time_step;
   }*/
}

int pppp = 0;

void CPhysicsWorld::advanceSimulation3(irr::f32 time)
{
  time_ = time;

  NewtonUpdate(m_NewtonWorld, dt);
}

void CPhysicsWorld::advanceSimulation(irr::u32 timeInMilisecunds)
{
	// do the physics simulation here
	int deltaTime;
	int physicLoopsTimeAcc;
	dFloat physicTime;

	// get the time step
	deltaTime = (timeInMilisecunds - g_currentTime);

	g_currentTime = timeInMilisecunds;

  if(deltaTime < 0) return;

	g_timeAccumulator += deltaTime;

	physicTime = 0;
	// advance the simulation at a fix step
	int loops = 0;
	physicLoopsTimeAcc = 0;


	while ((loops < MAX_PHYSICS_LOOPS) && (g_timeAccumulator >= dt))
	{
		loops ++;

		// sample time before the Update
		g_physicTime = m_Timer->getTime();

		// run the newton update function
		NewtonUpdate (m_NewtonWorld, dt);

    // calculate the time spent in the physical Simulation
		g_physicTime = m_Timer->getTime() - g_physicTime;

		// call the visual debugger to show the physics scene
#ifdef USE_VISUAL_DEBUGGER
		//NewtonDebuggerServe (g_newtonDebugger, g_world);
#endif

		// subtract time from time accumulator
		g_timeAccumulator -= dt;
		physicTime ++;

		physicLoopsTimeAcc += g_physicTime/1000;
	}

	if (loops > MAX_PHYSICS_LOOPS) {
		g_physicTime = physicLoopsTimeAcc;
		g_timeAccumulator = dt;
	}


	// calculate the interpolation parameter for smooth rendering
	//g_sceneManager->SetIntepolationParam(dFloat (g_timeAccumulator) / dFloat(DEMO_FPS_IN_MICROSECUNDS));

}

void CPhysicsWorld::createNewtonWorld()
{
	m_NewtonWorld = NewtonCreate();

	// use the standard x87 floating point model
	NewtonSetPlatformArchitecture(m_NewtonWorld, 0);

	// set a fix world size
	irr::core::vector3df minSize(-5000.0f, -1000.0f, -5000.0f);
	irr::core::vector3df maxSize( 5000.0f,  2000.0f, 5000.0f);

	minSize *= IrrToNewton;
	maxSize *= IrrToNewton;

	irr::f32 min_size[3], max_size[3];

	fillVec3(minSize, min_size);
	fillVec3(maxSize, max_size);

	NewtonSetWorldSize(m_NewtonWorld, min_size, max_size);

	// configure the Newton world to use iterative solve mode 0
	// this is the most efficient but the less accurate mode
	NewtonSetSolverModel(m_NewtonWorld, 1);

	NewtonSetMinimumFrameRate(m_NewtonWorld, 30);

	g_timeAccumulator = DEMO_FPS_IN_MICROSECUNDS;
}

void CPhysicsWorld::clear()
{
  for(u32 i=0; i < all_bodies.size(); ++i)
  {
    all_bodies[i]->removeBody();

    delete all_bodies[i];
  }

	NewtonDestroyAllBodies(m_NewtonWorld);
	NewtonFreeMemory();

  all_bodies.clear();
  all_bodies.set_used(0);

  g_timeAccumulator = DEMO_FPS_IN_MICROSECUNDS;
}

void CPhysicsWorld::closeNewtonWorld()
{
	clear();

	NewtonDestroy(m_NewtonWorld);
  //NewtonMaterialDestroyAllGroupID(m_NewtonWorld);
}


irr::core::vector3df CPhysicsWorld::calculateToqueAtPoint(
  const NewtonBody* body,
  const irr::core::vector3df& point,
  const irr::core::vector3df& force)
{
  /*dVector com;
  dMatrix matrix;

  NewtonBodyGetMatrix (body, &matrix[0][0]);
  NewtonBodyGetCentreOfMass (body, &matrix[0][0]);
  com = matrix.TransformVector (com);*/

  return irr::core::vector3df(0,0,0); //(point - com) *  force;
}







NewtonBody * pickedBody;
irr::f32 pickedParam = 0.f;
irr::core::vector3df pickedPosition, pickedNormal;

SRayCastParameters currentRay;

static irr::f32 RayCastFilter (
  const NewtonBody* body,
  const irr::f32* normal,
  int collisionID,
  void* userData,
  irr::f32 intersetParam)
{
	/*dFloat mass;
	dFloat Ixx;
	dFloat Iyy;
	dFloat Izz;

	NewtonBodyGetMassMatrix (body, &mass, &Ixx, &Iyy, &Izz);*/

	if(intersetParam < pickedParam)
	{
		//isPickedBodyDynamics = (mass > 0.0f);
		pickedParam = intersetParam;
		pickedBody = (NewtonBody*)body;
		pickedNormal = irr::core::vector3df(normal[0], normal[1], normal[2]);
	}
	return intersetParam;
}

unsigned RayCastPrefilter(
	const NewtonBody* body,
	const NewtonCollision* collision,
	void* userData)
{
  irr::u32 shapeId = NewtonCollisionGetUserID(collision);

  //printf("%d\n", shapeId);

  if(currentRay.excluded.linear_search(shapeId) == -1)
    return 1;
  else
    return 0;
}


SRayCastResult CPhysicsWorld::getRayCollision(SRayCastParameters ray)
{
  currentRay = ray;

  SRayCastResult result;

  irr::f32 lineStart[3], lineEnd[3];

  irr::core::vector3df lineStartPos = currentRay.line.start;

  currentRay.line.start *= IrrToNewton;
  currentRay.line.end *= IrrToNewton;

  fillVec3(currentRay.line.start, lineStart);
  fillVec3(currentRay.line.end, lineEnd);

  result.body = (CBody*) NULL;

  pickedBody = (NewtonBody*) NULL;
  pickedParam = 1.0f;
  pickedPosition.set(0,0,0);

  NewtonWorldRayCast(
    m_NewtonWorld,
    lineStart,
    lineEnd,
    RayCastFilter,
    NULL,
    RayCastPrefilter);

  if(pickedBody)
  {
    result.body = (CBody*)NewtonBodyGetUserData(pickedBody);
    pickedPosition = currentRay.line.start + pickedParam * (currentRay.line.end - currentRay.line.start);
    pickedPosition *= NewtonToIrr;
  }

  result.position = pickedPosition;
  result.normal = pickedNormal;
  result.distance = irr::core::line3df(lineStartPos, pickedPosition).getLength();

  return result;
}

static unsigned ConvexCastCallback(
  const NewtonBody* body,
  const NewtonCollision* collision,
  void* userData)
{
	// this convex cast have to skip the casting body
	NewtonBody* me = (NewtonBody*)userData;
	return (me == body) ? 0 : 1;
}

SConvexCastResult CPhysicsWorld::getConvexCollision(SConvexCastParameters convex)
{
	irr::f32 param;
	irr::core::matrix4 matrix;
	NewtonWorldConvexCastReturnInfo info[16];

  SConvexCastResult result;

	NewtonBodyGetMatrix(convex.body->getNewtonBody(), getMatrixPointer(matrix));

  irr::core::vector3df targetPos = matrix.getTranslation() + convex.offset;
  targetPos *= IrrToNewton;

  irr::f32 target[3];
  fillVec3(targetPos, target);

	irr::u32 hits = NewtonWorldConvexCast(
    m_NewtonWorld,
    getMatrixPointer(matrix),
    &target[0],
    convex.body->getCurrentCollision(),
    &param,
    convex.body->getNewtonBody(),
    ConvexCastCallback,
    &info[0],
    8,
    0);

  // Copy results
  for(irr::u16 i=0; i<hits; ++i)
  {
    SConvexCastResultSingle hitResult;

    hitResult.position.set(
      info[i].m_point[0],
      info[i].m_point[1],
      info[i].m_point[2]);

    hitResult.position *= NewtonToIrr;

    hitResult.normal.set(
      info[i].m_normal[0],
      info[i].m_normal[1],
      info[i].m_normal[2]);

    hitResult.normalWorld.set(
      info[i].m_normalOnHitPoint[0],
      info[i].m_normalOnHitPoint[1],
      info[i].m_normalOnHitPoint[2]);

    hitResult.param = info[i].m_penetration;
    hitResult.body = (CBody*)NewtonBodyGetUserData(info[i].m_hitBody);
    hitResult.contactID = info[i].m_contactID;

    result.results.push_back(hitResult);
  }

  result.param = param;

	//_ASSERTE (param < 1.0f);

	// the point at the intersection param is the floor
	//matrix.m_posit.m_y += (p.m_y - matrix.m_posit.m_y) * param;

	// Set the Body matrix to the new placement Matrix adjusted by the cast proccess.
	//NewtonBodySetMatrix(body, &matrix[0][0]);

	return result;
}

