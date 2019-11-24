#include "GrassSceneNode.h"

using namespace engine;

CGrassSceneNode::CGrassSceneNode(
  irr::scene::ISceneNode* parent,
  irr::scene::ISceneManager* mgr,
  irr::s32 id) : irr::scene::ISceneNode(parent, mgr, id)
{
  Material.Wireframe = false;
  Material.Lighting = true;
  Material.BackfaceCulling = false;
  Material.TextureLayer[0].Texture = SceneManager->getVideoDriver()->getTexture("data/foilage/grass.png");
  Material.MaterialType = irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
  Material.MaterialTypeParam = 0.32f;

  Box.addInternalPoint(irr::core::vector3df(1000,0,1000));
  Box.addInternalPoint(irr::core::vector3df(-1000,0,-1000));
  Box.addInternalPoint(irr::core::vector3df(1000,100,1000));
  Box.addInternalPoint(irr::core::vector3df(-1000,100,-1000));

  b_DistCheckEnabled = true;
}

CGrassSceneNode::~CGrassSceneNode()
{
  for(irr::u32 ref = 0; ref < m_ReferenceMeshes.size(); ++ref)
    m_ReferenceMeshes[ref]->drop();
}

void CGrassSceneNode::render()
{
  irr::video::IVideoDriver* driver = SceneManager->getVideoDriver();

  driver->setMaterial(Material);
  //driver->setTransform(irr::video::ETS_WORLD, AbsoluteTransformation);

  irr::core::vector3df cameraPos = SceneManager->getActiveCamera()->getPosition();
  irr::core::aabbox3df cameraBBox = SceneManager->getActiveCamera()->getViewFrustum()->boundingBox;

  irr::u16 gDen = 8;

  irr::core::array<SFoilageGroup* > patches;

  patches.push_back(m_Patches[m_ClosestPatchIndex]);

  irr::f32 currentDist = cameraPos.getDistanceFrom(m_Patches[m_ClosestPatchIndex]->position);

  // Check if any neighbour is closer than the current "main patch"

  for(irr::u32 n = 0; n < m_Patches[m_ClosestPatchIndex]->neighbours.size(); ++n) {
    patches.push_back(m_Patches[m_Patches[m_ClosestPatchIndex]->neighbours[n]]);

    irr::f32 nDist = cameraPos.getDistanceFrom(m_Patches[m_Patches[m_ClosestPatchIndex]->neighbours[n]]->position);

    // If it is, this will be our new "main patch"
    if(nDist < currentDist) {
      m_ClosestPatchIndex = m_Patches[m_ClosestPatchIndex]->neighbours[n];
      break;
    }
  }

#ifdef GRASS_2
  for(irr::u32 patchIdx = 0; patchIdx < patches.size(); ++patchIdx)
  for(irr::u32 elementIdx = 0; elementIdx < patches[patchIdx]->elements.size(); ++elementIdx)
  {
    irr::u8 ref_type = 0; //m_Patches[patchIdx]->elements[elementIdx]->type;

    irr::core::vector3df pos = patches[patchIdx]->elements[elementIdx]->position;
    irr::core::vector3df rot = patches[patchIdx]->elements[elementIdx]->rotation;
    irr::core::vector3df scale = patches[patchIdx]->elements[elementIdx]->scale;

    if((elementIdx % gDen) != 0)
      continue;

    if(cameraBBox.isPointInside(pos) == false)
      continue;

    if(b_DistCheckEnabled)
    if(pos.getDistanceFrom(cameraPos) > 70)
      continue;

    irr::core::matrix4 trans;
    //trans.setScale(scale);
    trans.setTranslation(pos);
    trans.setRotationDegrees(rot);

    driver->setTransform(irr::video::ETS_WORLD, trans);

    //driver->setMaterial(m_ReferenceMeshes[ref_type]->getMeshBuffer(0)->getMaterial());

    for(irr::u32 mb = 0; mb < m_ReferenceMeshes[ref_type]->getMeshBufferCount(); ++mb)
      driver->drawMeshBuffer(m_ReferenceMeshes[ref_type]->getMeshBuffer(mb));

    /*driver->drawVertexPrimitiveList(
      &Vertices[0], 4,
      &indices[0], 4,
      irr::video::EVT_STANDARD,
      irr::scene::EPT_TRIANGLES,
      irr::video::EIT_16BIT);*/

  }
#endif
  //}
}

void CGrassSceneNode::addReferenceMesh(const irr::c8 * meshfile)
{
  irr::scene::IMesh *mesh = getSceneManager()->getMesh(meshfile);

  if(mesh) {
    mesh->grab();
    m_ReferenceMeshes.push_back(mesh);
  }
}
