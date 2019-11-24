#include "Core.h"
#include "TerrainNode.h"

using namespace engine;

bool meshAdded = false;

CTerrainNode::CTerrainNode(
  irr::scene::ISceneNode* parent,
  irr::scene::ISceneManager* mgr,
  irr::s32 id) : irr::scene::ISceneNode(parent, mgr, id)
{
  m = k = 0;

  Box.addInternalPoint(irr::core::vector3df(4000,-100,4000));
  Box.addInternalPoint(irr::core::vector3df(-4000,-100,-4000));
  Box.addInternalPoint(irr::core::vector3df(4000,100,4000));
  Box.addInternalPoint(irr::core::vector3df(-4000,100,-4000));


}

CTerrainNode::~CTerrainNode()
{
  //dtor
  //m_Mesh->drop();

  free(indicesc);
  free(normals);
  free(vertpnt);
}

void CTerrainNode::setMesh(irr::scene::IMesh *mesh)
{
	m_Mesh = mesh;

	//m_Mesh->grab();

  meshAdded = true;

	buffer = (irr::scene::CMeshBuffer<irr::video::S3DVertex>*)m_Mesh->getMeshBuffer(0);

  m_Mesh->setHardwareMappingHint(irr::scene::EHM_STATIC, irr::scene::EBT_VERTEX_AND_INDEX);
  m_Mesh->setDirty();

	/*printf("Vertex Count: %u\n",buffer->getVertexCount());
	printf("Index Count: %u\n\n",buffer->getIndexCount());
	printf("Total Triangles Count: %u\n\n",buffer->getIndexCount()/3);*/

	indices = buffer->getIndices();
	vertices = buffer->getVertices();
	vertex = (irr::video::S3DVertex *) vertices;
	indexc = buffer->getIndexCount();
	indicesc = (irr::u16* )malloc(sizeof(short int)*buffer->getIndexCount());
	normals = (irr::core::vector3df *)malloc(sizeof(irr::core::vector3df)*buffer->getIndexCount());
	vertpnt = (irr::core::vector3df *)malloc(sizeof(irr::core::vector3df)*buffer->getIndexCount());

	irr::core::triangle3df poly;

	//precalculated faces normals and precalculated 1 point of intersection: normals/vertpnt
	memcpy(indicesc, indices, sizeof(short int)*buffer->getIndexCount());

	for(irr::s32 i = 0; i < indexc; i += 3)
	{
		poly.pointA = vertex[indicesc[i]].Pos;
		poly.pointB = vertex[indicesc[i+1]].Pos;
		poly.pointC = vertex[indicesc[i+2]].Pos;

    /*AbsoluteTransformation.transformVect(poly.pointA);
    AbsoluteTransformation.transformVect(poly.pointB);
    AbsoluteTransformation.transformVect(poly.pointC);*/

    /*irr::core::vector3df n =
      vertex[indicesc[i]].Normal + vertex[indicesc[i+1]].Normal + vertex[indicesc[i+2]].Normal;
    n /= 3;
    n.normalize();*/

		normals[i/3] = poly.getNormal().normalize();

		vertpnt[i/3] = vertex[indicesc[i]].Pos;

    //AbsoluteTransformation.transformVect(normals[i/3]);
		//AbsoluteTransformation.transformVect(vertpnt[i/3]);
	}
}

void CTerrainNode::render()
{
  if(meshAdded == false)
    return;

  //irr::core::vector3df camPos = SceneManager->getActiveCamera()->getPosition();

  //((irr::scene::SViewFrustum *)SceneManager->getActiveCamera()->getViewFrustum())->recalculateBoundingBox();

  /*irr::core::aabbox3df bBox = SceneManager->getActiveCamera()->getViewFrustum()->getBoundingBox();

  irr::u32 realIndiceCount = 0;

  for(irr::s32 i = 0; i < indexc; i += 3)
  {
    /*irr::core::vector3df p1 = vertpnt[indicesc[i]];
    irr::core::vector3df p2 = vertpnt[indicesc[i+1]];
    irr::core::vector3df p3 = vertpnt[indicesc[i+2]];

    AbsoluteTransformation.transformVect(p1);
    AbsoluteTransformation.transformVect(p2);
    AbsoluteTransformation.transformVect(p3);

    irr::core::vector3df pos = p1 + p2 + p3;
    pos /= 3;*/

    /*irr::core::line3df line1 = irr::core::line3df(p1, p2);
    irr::core::line3df line2 = irr::core::line3df(p2, p3);
    irr::core::line3df line3 = irr::core::line3df(p3, p1);*/

    /*if(SceneManager->getActiveCamera()->getViewFrustum()->clipLine(line1)
    || SceneManager->getActiveCamera()->getViewFrustum()->clipLine(line2)
    || SceneManager->getActiveCamera()->getViewFrustum()->clipLine(line3)
    || bBox.intersectsWithLine(line1)
    || bBox.intersectsWithLine(line2)
    || bBox.intersectsWithLine(line3))*/


    /*irr::core::vector3df p = vertpnt[i/3];
    irr::core::vector3df n = normals[i/3];

    AbsoluteTransformation.transformVect(p);
    AbsoluteTransformation.transformVect(n);*/

    //if(pos.getDistanceFrom(bBox.getCenter()) < 100)
    /*{
      indices[i] = indicesc[i];
      indices[i+1] = indicesc[i+1];
      indices[i+2] = indicesc[i+2];

      realIndiceCount += 1;
    }
  }

  SceneManager->getVideoDriver()->setMaterial(buffer->getMaterial());

  SceneManager->getVideoDriver()->setTransform(irr::video::ETS_WORLD, AbsoluteTransformation);

  SceneManager->getVideoDriver()->drawVertexPrimitiveList(
    buffer->getVertices(), buffer->getVertexCount(),
    indices, realIndiceCount,
    irr::video::EVT_STANDARD,
    irr::scene::EPT_TRIANGLES,
    irr::video::EIT_16BIT);*/

  SceneManager->getVideoDriver()->setTransform(irr::video::ETS_WORLD, AbsoluteTransformation);

  for(irr::u32 i = 0; i < m_Mesh->getMeshBufferCount(); ++i)
  {
    SceneManager->getVideoDriver()->setMaterial(m_Mesh->getMeshBuffer(i)->getMaterial());
    SceneManager->getVideoDriver()->drawMeshBuffer(m_Mesh->getMeshBuffer(i));
  }


}
