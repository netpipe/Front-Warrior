#include "Core.h"
#include "ShaderManager.h"
#include "Configuration.h"
#include "Renderer.h"
#include "ObjectManager.h"
#include "Camera.h"

using namespace engine;

using namespace irr;
using namespace irr::video;
using namespace irr::core;

void CShaderManager::clear()
{
  /*printf("Shader count: %d\n", shaderList.size());

  for(irr::s16 shader_idx = 0; shader_idx < shaderList.size(); ++shader_idx)
  {
    delete shaderList[shader_idx];
  }

  shaderList.clear();
  shaderList.set_used(0);*/
}

void CShaderManager::update()
{
  /*for(irr::s16 shader_idx = 0; shader_idx < shaderList.size(); ++shader_idx)
  {
    if(shaderList[shader_idx]->toBeDeleted)
    {
      delete shaderList[shader_idx];

      shaderList.erase(shader_idx);
      shader_idx -= 1;
    }
  }*/
}

void CBaseShader::OnSetConstants(
  irr::video::IMaterialRendererServices* services,
  irr::s32 userData)
{
  irr::video::IVideoDriver* driver = services->getVideoDriver();

  if(parameters & ESS_TEXTURES_OPENGL)
  {
    int texture0 = 0;
    services->setPixelShaderConstant("tex0", (f32*)(&texture0), 1);
    int texture1 = 1;
    services->setPixelShaderConstant("tex1", (f32*)(&texture1), 1);
    int texture2 = 2;
    services->setPixelShaderConstant("tex2", (f32*)(&texture2), 1);
    int texture3 = 3;
    services->setPixelShaderConstant("tex3", (f32*)(&texture3), 1);
    int texture4 = 4;
    services->setPixelShaderConstant("tex4", (f32*)(&texture4), 1);
  }

  if(parameters & ESS_SUN_POSITION)
  {
    f32 pos[3];
		driver->getTransform(video::ETS_VIEW).transformVect(pos,Core->getObjects()->parameters.sunPosition);
		services->setVertexShaderConstant("mSunPos", pos, 3);
  }

  if(parameters & ESS_AMBIENT_LIGHT)
  {
    irr::core::vector2df ambient = Core->getObjects()->parameters.lightValues;

    irr::core::vector3df ambientData;
    ambientData.X = ambient.X;
    ambientData.Y = ambient.Y;
    ambientData.Z = 0;

    services->setVertexShaderConstant("mAmbientData", reinterpret_cast<f32*>(&ambientData), 3);
  }

  if(parameters & ESS_TIME)
  {
    //SMoother like that!!! LEAVE
    float time = Core->getRenderer()->getDevice()->getTimer()->getTime();
    time /= 1000.f;
    services->setPixelShaderConstant("time", &time, 1);
  }

  if(parameters & ESS_FAR_VALUE)
  {
    float mFar = Core->getConfiguration()->getVideo()->drawRange;
    services->setPixelShaderConstant("mFar", &mFar,1);
  }
}

  /*int texture0 = 0;
  services->setPixelShaderConstant("tex0", (f32*)(&texture0), 1);
  int texture1 = 1;
  services->setPixelShaderConstant("tex1", (f32*)(&texture1), 1);
  int texture2 = 2;
  services->setPixelShaderConstant("tex2", (f32*)(&texture2), 1);
  int texture3 = 3;
  services->setPixelShaderConstant("tex3", (f32*)(&texture3), 1);
  int texture4 = 4;
  services->setPixelShaderConstant("tex4", (f32*)(&texture4), 1);*/


void CParticleFadeShader::OnSetConstants(
  irr::video::IMaterialRendererServices* services,
  irr::s32 userData)
{
  CBaseShader::OnSetConstants(services, userData);

  timeLeft -= Core->time.delta;

  irr::f32 alpha = timeLeft / timeStarted;

  if(timeLeft <= 0.06f)
  {
    toBeDeleted = true;
  }

  services->setVertexShaderConstant("alphaValue", reinterpret_cast<f32*>(&alpha), 1);
}


void CGrassShader::OnSetConstants(
  video::IMaterialRendererServices* services,
  s32 userData)
{
  CBaseShader::OnSetConstants(services, userData);
  float copies = Core->getConfiguration()->getVideo()->geomShaderGrass;
  services->setVertexShaderConstant("instances", &copies, 1);
  float decay = Core->getConfiguration()->getVideo()->geomShaderGrassDecay;
  services->setVertexShaderConstant("decay", &decay, 1);
  services->setVertexShaderConstant("windDir", &Core->GetAtmo()->getWindPacked().X,3);
  int i=3;
  services->setVertexShaderConstant("shadowMap", (float*)&i,1);
/*
  vector3df campos = Core->getRenderer()->getSceneManager()->getActiveCamera()->getPosition();
  services->setVertexShaderConstant("mCameraPos", reinterpret_cast<f32*>(&campos), 3);

  // Grass wave
  irr::core::vector3df gw;

  gw.X = sin(Core->getObjects()->GetGrassWave(0)) * 0.40f;
  gw.Y = sin(Core->getObjects()->GetGrassWave(1)) * 0.40f;
  gw.Z = 1.f - (UsedMaterial->EmissiveColor.getRed() / 255.f);

  services->setVertexShaderConstant("mGrassWave", reinterpret_cast<f32*>(&gw), 2);
  */
}



void CCameraViewObjectShader::OnSetConstants(
  video::IMaterialRendererServices* services,
  s32 userData)
{
  CBaseShader::OnSetConstants(services, userData);

  float shadow = 1.f-Core->getCamera()->getShadow();

  services->setVertexShaderConstant("shadow", &shadow, 1);
}







irr::s32 CShaderManager::createMultiTextureShader()
{
  irr::s32 result = 0;

  const c8* vsFileName = 0;
  const c8* psFileName = 0;
  const c8* vsFunc = 0;
  const c8* psFunc = 0;
  video::E_VERTEX_SHADER_TYPE vsType = video::EVST_VS_1_1;
  video::E_PIXEL_SHADER_TYPE psType = video::EPST_PS_2_0;

  irr::s32 flags =
    ESS_SUN_POSITION |
    ESS_AMBIENT_LIGHT;

  /*if(Core->getConfiguration()->GetVideo()->RenderDeviceID == 0)
  {
    if(psType == video::EPST_PS_2_0)
    {
      psFileName = "data/shaders/hlsl/SplatShader_ps20.hlsl";
      vsFileName = "data/shaders/hlsl/SplatShader_ps20.hlsl";
    }
    else if(psType == video::EPST_PS_1_4)
    {
      psFileName = "data/shaders/hlsl/SplatShader_ps14.hlsl";
      vsFileName = "data/shaders/hlsl/SplatShader_ps14.hlsl";
    }
    else
    {
      psFileName = "data/shaders/hlsl/SplatShader_ps20.hlsl";
      vsFileName = "data/shaders/hlsl/SplatShader_ps20.hlsl";
    }

    psFunc = "pixelMain";
    vsFunc = "vertexMain";
  }
  else */if(Core->getConfiguration()->getVideo()->renderDeviceID == 1)
  {
    psFileName = "data/shaders/glsl/SplatShader_ps20.frag";
    vsFileName = "data/shaders/glsl/SplatShader_ps20.vert";

    psFunc = "main";
    vsFunc = "main";

    flags |= ESS_TEXTURES_OPENGL;
  }

  video::IGPUProgrammingServices* gpu = Core->getRenderer()->getVideoDriver()->getGPUProgrammingServices();

  if(gpu)
  {
    CBaseShader *pShader = new CBaseShader(Core, flags);

    result = gpu->addHighLevelShaderMaterialFromFiles(
      vsFileName, vsFunc, vsType,
      psFileName, psFunc, psType,
      pShader);

    pShader->drop();

    shaderList.push_back(pShader);
  }

  return result;
}


irr::s32 CShaderManager::createGrassShader()
{
  irr::s32 result = 0;

  const c8* vsFileName = 0;
  const c8* gsFileName = 0;
  const c8* psFileName = 0;
  const c8* vsFunc = 0;
  const c8* psFunc = 0;
  video::E_VERTEX_SHADER_TYPE vsType = video::EVST_VS_2_0;
  video::E_GEOMETRY_SHADER_TYPE gsType = video::EGST_GS_4_0;
  video::E_PIXEL_SHADER_TYPE psType = video::EPST_PS_2_0;

  psFileName = "data/shaders/glsl/Grass.frag";
  gsFileName = "data/shaders/glsl/Grass.geom";
  vsFileName = "data/shaders/glsl/Grass.vert";
  psFunc     = "main";
  vsFunc     = "main";

  video::IGPUProgrammingServices* gpu = Core->getRenderer()->getVideoDriver()->getGPUProgrammingServices();

  if(gpu)
  {
    CBaseShader *pShader = new CGrassShader(Core, ESS_SUN_POSITION|ESS_TIME|ESS_FAR_VALUE|ESS_AMBIENT_LIGHT);

    if (Core->getRenderer()->getVideoDriver()->queryFeature(EVDF_GEOMETRY_SHADER)
    && Core->getConfiguration()->getVideo()->geomShaderGrass) {
        result = gpu->addHighLevelShaderMaterialFromFiles(
                            vsFileName, vsFunc, vsType,
                            psFileName, psFunc, psType,
                            gsFileName, psFunc, gsType,
                            scene::EPT_TRIANGLES, scene::EPT_TRIANGLE_STRIP, 0,
                            pShader, video::EMT_SOLID);
    }
    else {
#ifdef EXPERIMENTAL_GRASS
        result = gpu->addHighLevelShaderMaterialFromFiles(
                            "data/shaders/glsl/Grass2.vert", vsFunc, vsType,
                            "data/shaders/glsl/Grass2.frag", psFunc, psType,
                            pShader, video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF);
#else
        result = gpu->addHighLevelShaderMaterialFromFiles(
                            "data/shaders/glsl/Grass2.vert", vsFunc, vsType,
                            "data/shaders/glsl/Grass2.frag", psFunc, psType,
                            pShader, video::EMT_SOLID);
#endif
    }

    pShader->drop();

    shaderList.push_back(pShader);
  }

  return result;
}



irr::s32 CShaderManager::createCameraViewObjectShader()
{
  irr::s32 result = 0;

  const c8* vsFileName = 0;
  const c8* psFileName = 0;
  const c8* vsFunc = 0;
  const c8* psFunc = 0;
  video::E_VERTEX_SHADER_TYPE vsType = video::EVST_VS_1_1;
  video::E_PIXEL_SHADER_TYPE psType = video::EPST_PS_1_4;

  if(Core->getConfiguration()->getVideo()->renderDeviceID == 0)
  {
    psFileName = "data/shaders/hlsl/FPSWeaponShader.hlsl";
    vsFileName = "data/shaders/hlsl/FPSWeaponShader.hlsl";

    psFunc = "pixelMain";
    vsFunc = "vertexMain";
  }
  else if(Core->getConfiguration()->getVideo()->renderDeviceID == 1)
  {
    psFileName = "data/shaders/glsl/FPSWeaponShader.frag";
    vsFileName = "data/shaders/glsl/FPSWeaponShader.vert";

    psFunc = "main";
    vsFunc = "main";
  }

  video::IGPUProgrammingServices* gpu = Core->getRenderer()->getVideoDriver()->getGPUProgrammingServices();

  if(gpu)
  {
    CCameraViewObjectShader *pShader = new CCameraViewObjectShader(
      Core,
      ESS_SUN_POSITION);

    result = gpu->addHighLevelShaderMaterialFromFiles(
      vsFileName, vsFunc, vsType,
      psFileName, psFunc, psType,
      pShader);

    pShader->drop();

    shaderList.push_back(pShader);
  }

  return result;
}



irr::s32 CShaderManager::createLightmapShader()
{
  irr::s32 result = 0;

  const c8* vsFileName = 0;
  const c8* psFileName = 0;
  const c8* vsFunc = 0;
  const c8* psFunc = 0;
  video::E_VERTEX_SHADER_TYPE vsType = video::EVST_VS_1_1;
  video::E_PIXEL_SHADER_TYPE psType = video::EPST_PS_2_0;

  if(Core->getConfiguration()->getVideo()->renderDeviceID == 0)
  {
    psFileName = "data/shaders/hlsl/LightmapShader.hlsl";
    vsFileName = "data/shaders/hlsl/LightmapShader.hlsl";

    psFunc     = "pixelMain";
    vsFunc     = "vertexMain";
  }

  video::IGPUProgrammingServices* gpu = Core->getRenderer()->getVideoDriver()->getGPUProgrammingServices();

  if(gpu)
  {
    CBaseShader *pShader = new CBaseShader(
      Core,
      ESS_WORLD_VIEW_PROJECTION |
      ESS_AMBIENT_LIGHT);

    result = gpu->addHighLevelShaderMaterialFromFiles(
      vsFileName, vsFunc, vsType,
      psFileName, psFunc, psType,
      pShader);

    pShader->drop();

    shaderList.push_back(pShader);
  }

  return result;
}


irr::s32 CShaderManager::createParticleFadeOutShader(irr::f32 fade_time)
{
  irr::s32 result = 0;

  const c8* vsFileName = 0;
  const c8* psFileName = 0;
  const c8* vsFunc = 0;
  const c8* psFunc = 0;
  video::E_VERTEX_SHADER_TYPE vsType = video::EVST_VS_1_1;
  video::E_PIXEL_SHADER_TYPE psType = video::EPST_PS_2_0;

  if(Core->getConfiguration()->getVideo()->renderDeviceID == 0)
  {
    psFileName = "data/shaders/hlsl/ParticleFadeOutShader.hlsl";
    vsFileName = "data/shaders/hlsl/ParticleFadeOutShader.hlsl";

    psFunc     = "pixelMain";
    vsFunc     = "vertexMain";
  }
  else if(Core->getConfiguration()->getVideo()->renderDeviceID == 1)
  {
    psFileName = "data/shaders/glsl/ParticleFadeOutShader.frag";
    vsFileName = "data/shaders/glsl/ParticleFadeOutShader.vert";

    psFunc = "main";
    vsFunc = "main";

  }

  video::IGPUProgrammingServices* gpu = Core->getRenderer()->getVideoDriver()->getGPUProgrammingServices();

  if(gpu)
  {
    CParticleFadeShader *pShader = new CParticleFadeShader(
      Core,
      ESS_WORLD_VIEW_PROJECTION);

    result = gpu->addHighLevelShaderMaterialFromFiles(
      vsFileName, vsFunc, vsType,
      psFileName, psFunc, psType,
      pShader,
      irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);

    pShader->setTime(fade_time);

    pShader->drop();

    shaderList.push_back(pShader);
  }

  return result;
}


