#ifndef IRRLICHT_HEADER_DEFINED
#define IRRLICHT_HEADER_DEFINED

#include "Engine.h"
#include "ShaderManager.h"

namespace engine {

  /*struct SFaceCullData
  {
    irr::core::array<irr::scene::CMeshBuffer<irr::video::S3DVertex>*> Buffers;
    irr::core::array<irr::video::S3DVertex*> Vertex;
    irr::core::array<u16*> Indices;
    irr::core::array<irr::s32> IndexCount;
    irr::core::array<irr::u16*> IndicesCount;
    irr::core::array<irr::core::vector3df*> Normals;
    irr::core::array<irr::core::vector3df*> VertPoint;

    irr::core::array<irr::u32> m;
    irr::core::array<irr::u32> k;

    void Setup(irr::scene::IMesh* Mesh, bool batchedMesh);
    void Update();

    SFaceCullData()
    {
      m.set_used(0);
      k.set_used(0);
    }
  };*/

  class CCullingManager
  {
  public:

    CCullingManager(CCore * core) : Core(core)
    {
      //faceCulledObjects.set_used(0);
    }

    void update(irr::f32);

    void addFaceCulledMesh(irr::scene::IMesh* mesh);

  private:

    CCore * Core;

    //irr::core::array<SFaceCullData *> faceCulledObjects;

  };

  class CRenderer
  {
  public:

    CRenderer(CCore *core) : Core(core)
    {

    }

    ~CRenderer()
    {
      delete ShaderManager;
      delete CullingManager;
    }

    irr::u32 createDevice();

    bool updateOcclusionMap(irr::f32 time);

    irr::IrrlichtDevice *getDevice(){ return Device; }
    irr::video::IVideoDriver *getVideoDriver(){ return VideoDriver; }
    irr::scene::ISceneManager *getSceneManager(){ return SceneManager; }
    irr::gui::IGUIEnvironment *getGUI(){ return GUI; }
    irr::ITimer *getTimer() { return Timer; }
    CShaderManager *getShaders() { return ShaderManager; }
    CCullingManager *getCullingManager() { return CullingManager; }
    irr::scene::ICameraSceneNode *getCamera() { return SceneManager->getActiveCamera(); }

  private:

    CCore * Core;

    void renderToOcclusionMap();
    void registerNodes();
    void readRTT();
    void restoreNode(irr::scene::ISceneNode* node);

    bool needsOcclusionQuery();

    irr::IrrlichtDevice *Device;
    irr::video::IVideoDriver *VideoDriver;
    irr::scene::ISceneManager *SceneManager;
    irr::gui::IGUIEnvironment *GUI;
    irr::ITimer *Timer;

    CShaderManager *ShaderManager;

    CCullingManager *CullingManager;

    struct SOcclusionRTT
    {
       	irr::video::ITexture* Texture;
        irr::u32 FrameSkip, FramesToSkip;
       	irr::s32 Piece_Index, Piece_Count;
       	irr::f32 FrustFOV, FrustAspect;
       	unsigned int *Data;
        irr::u32 Size;
       	bool FinishedProcessing;

       	SOcclusionRTT()
       	{
            Texture = 0;
            Piece_Index = Piece_Count = 0;
            FrameSkip = FramesToSkip = 0;
            FinishedProcessing = true;
        }
    } occlusion;


  };


}

#endif
