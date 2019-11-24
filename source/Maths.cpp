#include "Maths.h"

using namespace engine;

/*irr::core::vector3df CMaths::ConvertNormalToRotation(irr::core::vector3df normal, irr::core::vector3df rotation)
{
  normal.normalize();

  irr::core::vector3df Nx = irr::core::vector3df(normal.X, normal.Y, 0);
  irr::core::vector3df Nz = irr::core::vector3df(0, normal.Y, normal.Z);

  irr::core::vector3df upVec = irr::core::vector3df(0,1,0);

  irr::f32 X = acos(Nx.dotProduct(upVec)/Nz.getLength())*180/irr::core::PI;
  irr::f32 Z = acos(Nz.dotProduct(upVec)/Nx.getLength())*180/irr::core::PI;

  irr::core::matrix4 mat1;
  mat1.setRotationDegrees(irr::core::vector3df(X, 0, Z));
  irr::core::matrix4 mat2;
  mat2.setRotationDegrees(irr::core::vector3df(0, rotation.Y, 0));

  irr::core::matrix4 mat = mat1*mat2;

  return mat.getRotationDegrees();
}*/

void CMaths::alignToUpVector(
  irr::core::matrix4 &mat_,
  const irr::core::matrix4 &oldMat_,
  const irr::core::vector3df &newUp_,
  irr::f32 interpolate_)
{
    irr::core::vector3df up(0, 1, 0);
    oldMat_.rotateVect(up);
    irr::f32 dot = up.dotProduct(newUp_);
    irr::core::vector3df axis( up.crossProduct(newUp_) );
    irr::core::quaternion quatTarget(axis.X, axis.Y, axis.Z, 1+dot);
    quatTarget.normalize();
    irr::core::quaternion quatSource;
    irr::core::quaternion quatRot;
    quatRot.slerp(quatSource, quatTarget, interpolate_);
    irr::core::vector3df newRot;
    quatRot.toEuler(newRot);
    newRot *= irr::core::RADTODEG;
    mat_.setRotationDegrees(newRot);

    mat_ = mat_*oldMat_;
}
