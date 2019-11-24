#ifndef SHADERS_HEADER_DEFINED
#define SHADERS_HEADER_DEFINED

#include <irrlicht.h>

namespace engine {

  enum E_SHADER_STATES
  {
    ESS_WORLD_VIEW_PROJECTION = 1 << 0,
    ESS_TRANSFORMED_WORLD_MATRIX = 1 << 1,
    ESS_INVERTED_WORLD_MATRIX = 1 << 2,
    ESS_SUN_POSITION = 1 << 3,
    ESS_AMBIENT_LIGHT = 1 << 4,
    ESS_TEXTURES_OPENGL = 1 << 5,
    ESS_TIME = 1 << 6,
    ESS_FAR_VALUE = 1 << 7
  };


  //
  // The base shader
  //

  class CBaseShader : public irr::video::IShaderConstantSetCallBack
  {
  public:

    CBaseShader(CCore * core, irr::u32 params) : Core(core){
      toBeDeleted = false;
      parameters = params;
    }

    virtual void OnSetConstants(
      irr::video::IMaterialRendererServices* services,
      irr::s32 userData);

    virtual void OnSetMaterial(const irr::video::SMaterial& material)
    {
      UsedMaterial = &material;
    }

    friend class CShaderManager;

  protected:

    irr::u32 parameters;

    const irr::video::SMaterial *UsedMaterial;

    CCore * Core;

    bool toBeDeleted;
  };



  class CGrassShader : public CBaseShader
  {
    public:

      CGrassShader(CCore *core, irr::u32 params) : CBaseShader(core, params) {
        parameters = params;
      }

      virtual void OnSetConstants(
        irr::video::IMaterialRendererServices* services,
        irr::s32 userData);
  };


  class CCameraViewObjectShader : public CBaseShader
  {
    public:

      CCameraViewObjectShader(CCore *core, irr::u32 params) : CBaseShader(core, params) {
        parameters = params;
      }

      virtual void OnSetConstants(
        irr::video::IMaterialRendererServices* services,
        irr::s32 userData);
  };



  class CParticleFadeShader : public CBaseShader
  {
    private:

      irr::f32 timeStarted;

      irr::f32 timeLeft;

    public:

      CParticleFadeShader(CCore* core, irr::u32 params) : CBaseShader(core, params) {
        parameters = params;
      }

      virtual void OnSetConstants(
        irr::video::IMaterialRendererServices* services,
        irr::s32 userData);

      void setTime(irr::f32 time) {
        timeStarted = timeLeft = time;
      }
  };


  //
  // Shader manager
  // You can create new shaders trough this
  //

  class CShaderManager
  {
  private:
    irr::core::array<CBaseShader*> shaderList;

  public:

    CShaderManager(CCore *core) : Core(core)
    {
      shaderList.set_used(0);
    }

    ~CShaderManager()
    {
    }

    void update();

    void clear();

    irr::s32 createMultiTextureShader();
    irr::s32 createGrassShader();
    irr::s32 createCameraViewObjectShader();
    irr::s32 createLightmapShader();
    irr::s32 createParticleFadeOutShader(irr::f32 fade_time);

  private:
    CCore * Core;
  };

}

#endif
