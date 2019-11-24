#ifndef TERRAINNODE_H
#define TERRAINNODE_H

#include "Engine.h"

namespace engine {

class CTerrainNode : public irr::scene::ISceneNode
{
  public:

    CTerrainNode(
      irr::scene::ISceneNode* parent,
      irr::scene::ISceneManager* mgr,
      irr::s32 id);

    ~CTerrainNode();

    virtual void OnRegisterSceneNode()
    {
      if(IsVisible)
        SceneManager->registerNodeForRendering(this);

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

    void setMesh(irr::scene::IMesh *mesh);

  private:

    irr::core::aabbox3d<irr::f32> Box;

    irr::video::SMaterial Material;

    irr::scene::IMesh * m_Mesh;

    irr::scene::CMeshBuffer<irr::video::S3DVertex>* buffer;
    irr::u16* indices;
    void* vertices;
    irr::video::S3DVertex* vertex;
    irr::s32 indexc;
    irr::u16* indicesc;
    irr::core::vector3df* normals;
    irr::core::vector3df* vertpnt;
    int m, k;

};

}

#endif // TERRAINNODE_H
