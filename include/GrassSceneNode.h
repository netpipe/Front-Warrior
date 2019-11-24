#ifndef CGRASSSCENENODE_H
#define CGRASSSCENENODE_H

#include "Engine.h"

namespace engine {

  class CGrassSceneNode : public irr::scene::ISceneNode
  {
  public:

    CGrassSceneNode(
      irr::scene::ISceneNode* parent,
      irr::scene::ISceneManager* mgr,
      irr::s32 id);

    ~CGrassSceneNode();

    virtual void OnRegisterSceneNode()
    {
      if (IsVisible)
        SceneManager->registerNodeForRendering(this, irr::scene::ESNRP_TRANSPARENT);

      ISceneNode::OnRegisterSceneNode();
    }

    virtual void render();

    virtual const irr::core::aabbox3d<irr::f32>& getBoundingBox() const
    {
      return Box;
    }

    virtual irr::u32 getMaterialCount() const
    {
      return 1;
    }

    virtual irr::video::SMaterial& getMaterial(irr::u32 i)
    {
      return Material;
    }

    void addReferenceMesh(const irr::c8 * meshfile);

    void addPatch(SFoilageGroup *p) {
      m_Patches.push_back(p);
    }

    SFoilageGroup *getPatchByID(irr::u32 pId) { return m_Patches[pId]; }

    void findClosestPatch(irr::core::vector3df position)
    {
      // Check all foilage groups

      irr::f32 closestDist = 999999.9f;

      position.Y = 0.f;

      for(irr::u16 fgi=0; fgi < m_Patches.size(); ++fgi)
      {
        irr::f32 dist = m_Patches[fgi]->position.getDistanceFrom(position);

        if(dist < closestDist)
        {
          closestDist = dist;
          m_ClosestPatchIndex = fgi;
        }
      }
    }

    void setIsDestCheck(bool b) { b_DistCheckEnabled = b; }

  private:

    irr::core::aabbox3d<irr::f32> Box;

    irr::video::SMaterial Material;

    irr::core::array<irr::scene::IMesh*> m_ReferenceMeshes;

    irr::core::array<SFoilageGroup *> m_Patches;

    irr::video::S3DVertex Vertices[4];

    irr::u32 m_ClosestPatchIndex;

    bool b_DistCheckEnabled;
  };

}

#endif // CGRASSSCENENODE_H
