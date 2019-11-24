#include "newton/World.h"

using namespace engine::physics;

void CBody::setRotation(irr::core::vector3df rotation)
{
  m_Node->setRotation(rotation);

	irr::core::matrix4 temp_mat;
	NewtonBodyGetMatrix(m_NewtonBody, getMatrixPointer(temp_mat));
	temp_mat.setRotationDegrees(rotation);
	NewtonBodySetMatrix(m_NewtonBody, getMatrixPointer(temp_mat));
}

void CBody::setPosition(irr::core::vector3df position)
{
  m_Node->setPosition(position);

  irr::core::matrix4 body_matrix;
	NewtonBodyGetMatrix(m_NewtonBody, getMatrixPointer(body_matrix));

  body_matrix.setTranslation(position * IrrToNewton);

  NewtonBodySetMatrix(
    m_NewtonBody,
    getMatrixPointer(body_matrix));
}

irr::core::vector3df CBody::getPositionBody()
{
  irr::core::matrix4 body_matrix;
	NewtonBodyGetMatrix(m_NewtonBody, getMatrixPointer(body_matrix));

	return (body_matrix.getTranslation() * NewtonToIrr);
}

void CBody::setContinuousCollisionMode(bool value)
{
	if(value)
		NewtonBodySetContinuousCollisionMode(m_NewtonBody, 1);
	else
		NewtonBodySetContinuousCollisionMode(m_NewtonBody, 0);
}

void CBody::setVelocity(irr::core::vector3df velocity)
{
	irr::f32 velocity_array[3];

	fillVec3(velocity, velocity_array);

	NewtonBodySetVelocity(m_NewtonBody, velocity_array);
}

void CBody::setOmega(irr::core::vector3df omega)
{
	irr::f32 omega_array[3];

	fillVec3(omega, omega_array);

	NewtonBodySetOmega(m_NewtonBody, omega_array);
}

irr::core::vector3df CBody::getVelocity()
{
  irr::f32 velocity_array[3] = {0, 0, 0};

  NewtonBodyGetVelocity(m_NewtonBody, velocity_array);

  return irr::core::vector3df(velocity_array[0], velocity_array[1], velocity_array[2]);
}

irr::core::vector3df CBody::getOmega()
{
  irr::f32 omega_array[3] = {0, 0, 0};

  NewtonBodyGetOmega(m_NewtonBody, omega_array);

  return irr::core::vector3df(omega_array[0], omega_array[1], omega_array[2]);
}



void CBody::removeBody()
{
  for(irr::u16 jIdx=0; jIdx < a_NewtonJoints.size(); ++jIdx)
    NewtonDestroyJoint(m_NewtonWorld, a_NewtonJoints[jIdx]);

  if(m_NewtonBody)
  {
    //set callbacks to NULL
    NewtonBodySetTransformCallback(m_NewtonBody, NULL);
    NewtonBodySetForceAndTorqueCallback(m_NewtonBody, NULL);

    //destroy body
    NewtonDestroyBody(m_NewtonWorld, m_NewtonBody);

    m_NewtonBody = NULL;
  }

  a_NewtonJoints.clear();
  a_NewtonJoints.set_used(0);

  a_NewtonCollisions.clear();
  a_NewtonCollisions.set_used(0);
}

void CBody::setUsedCollision(irr::u32 id)
{
  if(id == m_CurrentCollisionIndex)
    return;

  if(id > a_NewtonCollisions.size()-1)
    id = a_NewtonCollisions.size()-1;

  m_CurrentCollisionIndex = id;

  printf("Collision %d used\n", id);

  NewtonBodySetCollision(m_NewtonBody, a_NewtonCollisions[id]);

  setAndCalculateMass();
}

void CBody::setAndCalculateMass()
{
  irr::f32 inertia_array[3], origin_array[3];

  // bodies can have a destructor.
  // this is a function callback that can be used to destroy any local data stored
  // and that need to be destroyed before the body is destroyed.
  //NewtonBodySetDestructorCallback(body, body_DestroyBodyCallback);

  irr::f32 initial_force[3] = {0,0,0};
  NewtonBodySetForce(m_NewtonBody, initial_force);

  // we need to set the proper center of mass and inertia matrix for this body
  // the inertia matrix calculated by this function does not include the mass.
  // therefore it needs to be multiplied by the mass of the body before it is used.
  NewtonConvexCollisionCalculateInertialMatrix(
    a_NewtonCollisions[m_CurrentCollisionIndex],
    inertia_array,
    origin_array);

  // set the body mass matrix
  NewtonBodySetMassMatrix(
    m_NewtonBody,
    m_Mass,
    inertia_array[0],
    inertia_array[1],
    inertia_array[2]);

  // set the body origin
  NewtonBodySetCentreOfMass(m_NewtonBody, origin_array);

}

void CBody::createUpVectorConstraint(irr::core::vector3df upv)
{
  irr::f32 pin[3] = {upv.X, upv.Y, upv.Z};

  a_NewtonJoints.push_back(
    NewtonConstraintCreateUpVector(
      m_NewtonWorld,
      &pin[0],
      m_NewtonBody));

  //void NewtonUpVectorSetPin( const NewtonJoint* upVector, const dFloat *pin)
}


#ifdef ENGINE_DEVELOPMENT_MODE
irr::video::IVideoDriver *vdriver;

void NewtonDebugCollision(
  void* body,
  int vertexCount,
  const float* faceVertec,
  int id)
{
	int i;
	irr::core::vector3df line;

	i = vertexCount - 1;
	irr::core::vector3df p0 (faceVertec[i * 3 + 0] * NewtonToIrr, faceVertec[i * 3 + 1] * NewtonToIrr, faceVertec[i * 3 + 2] * NewtonToIrr);

	for (i = 0; i < vertexCount; i ++) {
		irr::core::vector3df p1 (faceVertec[i * 3 + 0] * NewtonToIrr, faceVertec[i * 3 + 1] * NewtonToIrr, faceVertec[i * 3 + 2] * NewtonToIrr);
		vdriver->draw3DLine(p0, p1, irr::video::SColor(255, 255, 255, 0));
 		p0 = p1;
	}

}
#endif

void CBody::drawDebug(irr::video::IVideoDriver *driver)
{
#ifdef ENGINE_DEVELOPMENT_MODE
	vdriver = driver;

	irr::core::matrix4 mat;
	irr::video::SMaterial material;

	driver->setTransform(irr::video::ETS_WORLD, mat);
	driver->setMaterial(material);

  /*irr::f32 *mat2;
  NewtonBodyGetMatrix(m_NewtonBody, mat2);

	irr::core::matrix4 irr_mat;
	memcpy(getMatrixPointer(irr_mat), mat2, sizeof(float)*16);*/

  irr::core::matrix4 node_matrix;
	NewtonBodyGetMatrix(m_NewtonBody, getMatrixPointer(node_matrix));

	// draw the collision mesh in wireframe
	NewtonCollisionForEachPolygonDo(
    NewtonBodyGetCollision(m_NewtonBody),
    getMatrixPointer(node_matrix),
    NewtonDebugCollision,
    NULL);
#endif
}
