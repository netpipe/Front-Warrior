#include "newton/World.h"

using namespace engine::physics;

void CCollisionManager::addMeshToTreeCollisionStandard(
  irr::scene::IMeshBuffer* meshBuffer,
  NewtonCollision* treeCollision,
  irr::core::vector3df scale)

{
	irr::core::vector3df vArray[3];

	irr::video::S3DVertex* mb_vertices = (irr::video::S3DVertex*) meshBuffer->getVertices();

	irr::u16* mb_indices = meshBuffer->getIndices();

  irr::video::SMaterial mat = meshBuffer->getMaterial();

	for (unsigned int j = 0; j < meshBuffer->getIndexCount(); j += 3)
	{
		int v1i = mb_indices[j + 0];
		int v2i = mb_indices[j + 1];
		int v3i = mb_indices[j + 2];

		vArray[0] = mb_vertices[v1i].Pos * scale.X * IrrToNewton;
		vArray[1] = mb_vertices[v2i].Pos * scale.Y * IrrToNewton;
		vArray[2] = mb_vertices[v3i].Pos * scale.Z * IrrToNewton;

    NewtonTreeCollisionAddFace(treeCollision, 3, &vArray[0].X, sizeof(irr::core::vector3df), 1);

    if(mat.MaterialType == irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL
    || mat.MaterialType == irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF)
    {
      vArray[0] = mb_vertices[v3i].Pos * scale.X * IrrToNewton;
      vArray[1] = mb_vertices[v2i].Pos * scale.Y * IrrToNewton;
      vArray[2] = mb_vertices[v1i].Pos * scale.Z * IrrToNewton;

      NewtonTreeCollisionAddFace(treeCollision, 3, &vArray[0].X, sizeof(irr::core::vector3df), 1);
    }

	}
}

void CCollisionManager::addMeshToTreeCollision2TCoords(
  irr::scene::IMeshBuffer* meshBuffer,
  NewtonCollision* treeCollision,
  irr::core::vector3df scale)
{
	irr::core::vector3df vArray[3];

	irr::video::S3DVertex2TCoords* mb_vertices = (irr::video::S3DVertex2TCoords*) meshBuffer->getVertices();

	irr::u16* mb_indices  = meshBuffer->getIndices();

  irr::video::SMaterial mat = meshBuffer->getMaterial();

	for (unsigned int j = 0; j < meshBuffer->getIndexCount(); j += 3)
	{
		int v1i = mb_indices[j + 0];
		int v2i = mb_indices[j + 1];
		int v3i = mb_indices[j + 2];

		vArray[0] = mb_vertices[v1i].Pos * scale.X * IrrToNewton;
		vArray[1] = mb_vertices[v2i].Pos * scale.Y * IrrToNewton;
		vArray[2] = mb_vertices[v3i].Pos * scale.Z * IrrToNewton;

		NewtonTreeCollisionAddFace(treeCollision, 3, &vArray[0].X, sizeof(irr::core::vector3df), 1);

    if(mat.MaterialType == irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL
    || mat.MaterialType == irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF)
    {
      vArray[0] = mb_vertices[v3i].Pos * scale.X * IrrToNewton;
      vArray[1] = mb_vertices[v2i].Pos * scale.Y * IrrToNewton;
      vArray[2] = mb_vertices[v1i].Pos * scale.Z * IrrToNewton;

      NewtonTreeCollisionAddFace(treeCollision, 3, &vArray[0].X, sizeof(irr::core::vector3df), 1);
    }

	}
}

void CCollisionManager::addMeshToTreeCollisionTangents(
  irr::scene::IMeshBuffer* meshBuffer,
  NewtonCollision* treeCollision,
  irr::core::vector3df scale)
{
	irr::core::vector3df vArray[3];

	irr::video::S3DVertexTangents* mb_vertices = (irr::video::S3DVertexTangents*) meshBuffer->getVertices();

	irr::u16* mb_indices  = meshBuffer->getIndices();

  irr::video::SMaterial mat = meshBuffer->getMaterial();

	for (unsigned int j = 0; j < meshBuffer->getIndexCount(); j += 3)
	{
		int v1i = mb_indices[j + 0];
		int v2i = mb_indices[j + 1];
		int v3i = mb_indices[j + 2];

		vArray[0] = mb_vertices[v1i].Pos * scale.X * IrrToNewton;
		vArray[1] = mb_vertices[v2i].Pos * scale.Y * IrrToNewton;
		vArray[2] = mb_vertices[v3i].Pos * scale.Z * IrrToNewton;

		NewtonTreeCollisionAddFace(treeCollision, 3, &vArray[0].X, sizeof(irr::core::vector3df), 1);

    if(mat.MaterialType == irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL
    || mat.MaterialType == irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF)
    {
      vArray[0] = mb_vertices[v3i].Pos * scale.X * IrrToNewton;
      vArray[1] = mb_vertices[v2i].Pos * scale.Y * IrrToNewton;
      vArray[2] = mb_vertices[v1i].Pos * scale.Z * IrrToNewton;

      NewtonTreeCollisionAddFace(treeCollision, 3, &vArray[0].X, sizeof(irr::core::vector3df), 1);
    }

	}
}



NewtonCollision* CCollisionManager::createConvexTree(
  NewtonWorld * world,
  irr::scene::IMesh * mesh,
  irr::core::vector3df scale,
  irr::u32 shapeId)
{
  NewtonCollision *treeCollision;

	treeCollision = NewtonCreateTreeCollision(world, shapeId);

	NewtonTreeCollisionBeginBuild(treeCollision);

	for (unsigned int i = 0; i < mesh->getMeshBufferCount(); i++)
	{
		irr::scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);

		switch(mb->getVertexType())
		{
			case irr::video::EVT_STANDARD:
				addMeshToTreeCollisionStandard(mb, treeCollision, scale);
			break;

			case irr::video::EVT_2TCOORDS:
				addMeshToTreeCollision2TCoords(mb, treeCollision, scale);
			break;

			case irr::video::EVT_TANGENTS:
				addMeshToTreeCollisionTangents(mb, treeCollision, scale);
			break;

			default:
				printf("Newton error: Unknown vertex type in static mesh: %d\n", mb->getVertexType());
			break;
		}
	}

	NewtonTreeCollisionEndBuild(treeCollision, 1);

	//NewtonReleaseCollision(m_NewtonWorld, treeCollision);

	return treeCollision;
}

NewtonCollision* CCollisionManager::createNewtonPrimitive(
  NewtonWorld *world,
  E_BODY_TYPE type,
  irr::scene::ISceneNode *node,
  irr::core::vector3df scale,
  irr::core::vector3df offsetPos,
  irr::u32 shapeId)
{
	NewtonCollision* collision;

  irr::core::aabbox3df bbox = node->getBoundingBox();


	//calculate the box size and dimensions of the physics collision shape
	irr::core::vector3df size((bbox.MaxEdge - bbox.MinEdge) * scale * IrrToNewton);
	irr::core::vector3df origin = bbox.getCenter() * IrrToNewton;

	irr::core::matrix4 offset;
	offset.setTranslation(offsetPos);

	if(type == EBT_PRIMITIVE_BOX)
	{
    collision = NewtonCreateBox(
      world,
      size.X,
      size.Y,
      size.Z,
      shapeId,
      getMatrixPointer(offset));
	}
	else if(type == EBT_PRIMITIVE_SPHERE)
	{
    collision = NewtonCreateSphere(
      world,
      size.X/2.0f,
      size.Y/2.0f,
      size.Z/2.0f,
      shapeId,
      getMatrixPointer(offset));
	}
	else if(type == EBT_PRIMITIVE_CAPSULE)
	{
		offset.setRotationDegrees(offset.getRotationDegrees() + irr::core::vector3df(-90,0,90));

		irr::f32 radius;
		if(size.Z >= size.X)
      radius = size.Z;
		else
      radius = size.X;

		radius /= 2.0f;

		collision = NewtonCreateCapsule(
			world,
			radius,
			size.Y,
			shapeId,
			getMatrixPointer(offset));
	}

	return collision;
}



NewtonCollision* CCollisionManager::createConvexHull(
  NewtonWorld * world,
  irr::scene::IMesh * mesh,
  irr::core::vector3df scale,
  irr::u32 shapeId)
{
  NewtonCollision* collision;

  /*irr::core::aabbox3df bbox = node->getBoundingBox();

	//calculate the box size and dimensions of the physics collision shape
	irr::core::vector3df size((bbox.MaxEdge - bbox.MinEdge) * IrrToNewton * node->getScale());
	irr::core::vector3df origin = bbox.getCenter();
  irr::core::vector3df scale = node->getScale();*/

  irr::core::array<irr::f32> polys;
  polys.set_used(0);

	for(unsigned int i = 0; i < mesh->getMeshBufferCount(); i++)
	{
		irr::scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);

		switch(mb->getVertexType())
		{
			case irr::video::EVT_STANDARD:
      {
        irr::video::S3DVertex* mb_vertices = (irr::video::S3DVertex*)mb->getVertices();

        for (irr::u32 i=0; i < mb->getVertexCount(); ++i)
        {
          irr::core::vector3df vPos;

          vPos = ((mb_vertices[i].Pos * scale)) * IrrToNewton;

          polys.push_back(vPos.X);
          polys.push_back(vPos.Y);
          polys.push_back(vPos.Z);
        }
      }
			break;

			case irr::video::EVT_2TCOORDS:
      {
        irr::video::S3DVertex2TCoords* mb_vertices = (irr::video::S3DVertex2TCoords*)mb->getVertices();

        for (irr::u32 i=0; i < mb->getVertexCount(); ++i)
        {
          irr::core::vector3df vPos;

          vPos = ((mb_vertices[i].Pos * scale)) * IrrToNewton;

          polys.push_back(vPos.X);
          polys.push_back(vPos.Y);
          polys.push_back(vPos.Z);
        }
      }
			break;

			case irr::video::EVT_TANGENTS:

			break;

			default:
				printf("createConvex: error: Unknown vertex type in static mesh: %d\n", mb->getVertexType());
			break;
		}
	}

	irr::core::matrix4 offset;
	//offset.setTranslation(origin*IrrToNewton);

	// now create a convex hull shape from the vertex geometry
	collision = NewtonCreateConvexHull(
	   world,
	   polys.size()/3,
	   &polys[0],
	   sizeof(irr::f32)*3,
	   0.002f,
	   shapeId,
	   getMatrixPointer(offset));

	return collision;
}

void CCollisionManager::releaseCollision(NewtonWorld* world, NewtonCollision *collision)
{
  NewtonReleaseCollision(world, collision);
}

