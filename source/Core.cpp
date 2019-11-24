#include <irrlicht.h>

#include "Core.h"

#include "Renderer.h"
#include "SoundManager.h"
#include "Configuration.h"
#include "Input.h"
#include "ObjectManager.h"
#include "Physics.h"
#include "Maths.h"
#include "Camera.h"
#include "Atmosphere.h"
#include "Clock.h"

#include <GL/gl.h>
#include <GL/glu.h>

using namespace engine;

CCore::~CCore()
{
  delete Configuration;
  delete Renderer;
  delete Objects;
  delete Math;
  delete PhysicsManager;
  delete Camera;
  delete SoundManager;
  delete AtmoManager;
  delete Timer;
}


irr::u16 CCore::init()
{
  // Load video, sound, controls settings from an external file
  Configuration = new CConfiguration();

  Configuration->deserialize();

  Renderer = new CRenderer(this);
  SoundManager = new CSoundManager(this);

  // Create device
  irr::u32 deviceCreationResult = Renderer->createDevice();

  // Error occured!
  if(deviceCreationResult != 0)
    return deviceCreationResult;

  AtmoManager = new CAtmosphereManager(this);
  Objects = new CObjectManager(this);
  PhysicsManager = new CPhysicsManager(this);
  Camera = new CCamera(this);
  Timer = new CTimer(this);
  Math = new CMaths();

  Camera->setMaterial(Renderer->getShaders()->createCameraViewObjectShader());

  bIsRunning = true;
  requestAppClose = false;

  anaglyphCam = Renderer->getSceneManager()->addCameraSceneNode(0,irr::core::vector3df(50,50,-60), irr::core::vector3df(-70,30,-60), -1, false);
  anaglyphdist = -1.f;

  return 0;
}

irr::u8 showFPS = 2;
irr::u32 debug_delta=0, debug_fps=0;
irr::f32 update_debug=0;


void CCore::update()
{
  // Calulate frame time

  irr::u32 timeThisFrame = Renderer->getTimer()->getTime();

  // Lock FPS at around 60
  /*if(Game->getState() == game::EGS_GAME)
  {
    while((timeThisFrame - time.total) <= 16)
    {
      Renderer->GetTimer()->tick();
      timeThisFrame = getRenderer()->GetTimer()->getTime();
    }
  }*/

  //if(((timeThisFrame - time.total)/ 1000.0f) < 0.0166666f)
  //  Renderer->getDevice()->sleep(16-(timeThisFrame - time.total));

  //Renderer->getTimer()->tick();
  //timeThisFrame = getRenderer()->getTimer()->getTime();

  irr::u32 oldtime = time.total;

  time.total = timeThisFrame;
  time.delta = (time.total - oldtime) / 1000.0f;

  /*if(getRenderer()->UpdateOcclusionMap(time.delta))
  {
    //return;
  }*/


  Renderer->getVideoDriver()->beginScene(true, true, Objects->parameters.backgroundSkyColor);

  if(b_Paused == false)
  {
    AtmoManager->update();
    Timer->update();
    Objects->update(time.delta);
  }
}

void CCore::render()
{
#ifdef PHYSICS_NEWTON
  if(b_Paused == false)
    PhysicsManager->update2();
#endif

  // Updates the camera
  if(b_Paused == false)
    Camera->update(time.delta);
  Renderer->getCullingManager()->update(time.delta);

#define SET_APART 0.27f
  if (Camera->getNode()&&Configuration->getVideo()->Anaglyph) {
    Renderer->getVideoDriver()->getOverrideMaterial().Material.ColorMask=irr::video::ECP_RED;
    Renderer->getVideoDriver()->getOverrideMaterial().EnableFlags=irr::video::EMF_COLOR_MASK;
    Renderer->getVideoDriver()->getOverrideMaterial().EnablePasses=irr::scene::ESNRP_SKY_BOX+irr::scene::ESNRP_SOLID+irr::scene::ESNRP_TRANSPARENT+irr::scene::ESNRP_TRANSPARENT_EFFECT+irr::scene::ESNRP_SHADOW;
    Renderer->getSceneManager()->drawAll();

    Renderer->getVideoDriver()->setTransform(irr::video::ETS_WORLD,irr::core::matrix4());
    GLfloat depthVal;
    glReadPixels(irr::s32(Configuration->getVideo()->windowSize.Width)/2,irr::s32(Configuration->getVideo()->windowSize.Height)/2,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,(GLvoid*)&depthVal);
    GLdouble modelMatrix[16];
    glGetDoublev(GL_MODELVIEW_MATRIX,modelMatrix);
    GLdouble projMatrix[16];
    glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    GLdouble targetX,targetY,targetZ;
    gluUnProject(irr::s32(Configuration->getVideo()->windowSize.Width)/2,irr::s32(Configuration->getVideo()->windowSize.Height)/2,depthVal,modelMatrix,projMatrix,viewport,&targetX,&targetY,&targetZ);
    if (anaglyphdist<0.f)
        anaglyphdist = (irr::core::vector3df(targetX,targetY,targetZ)-Camera->getNode()->getAbsolutePosition()).getLength();
    else
        anaglyphdist = anaglyphdist*0.85+0.15*(irr::core::vector3df(targetX,targetY,targetZ)-Camera->getNode()->getAbsolutePosition()).getLength();


    Renderer->getVideoDriver()->clearZBuffer();
    Renderer->getVideoDriver()->getOverrideMaterial().Material.ColorMask=irr::video::ECP_GREEN+irr::video::ECP_BLUE;
    Renderer->getVideoDriver()->getOverrideMaterial().EnableFlags=irr::video::EMF_COLOR_MASK;
    Renderer->getVideoDriver()->getOverrideMaterial().EnablePasses=irr::scene::ESNRP_SKY_BOX+irr::scene::ESNRP_SOLID+irr::scene::ESNRP_TRANSPARENT+irr::scene::ESNRP_TRANSPARENT_EFFECT+irr::scene::ESNRP_SHADOW;
    irr::core::vector3df viewVec = Camera->getNode()->getTarget()-Camera->getNode()->getAbsolutePosition();
    viewVec = viewVec.normalize();
    irr::core::vector3df tan = irr::core::vector3df(-viewVec.Z,0,viewVec.X);
    tan = tan.normalize();
    irr::core::vector3df oldPos = Camera->getNode()->getPosition();
    Camera->getNode()->setPosition(Camera->getNode()->getAbsolutePosition()-tan*SET_APART);
    Camera->getNode()->setTarget(Camera->getNode()->getAbsolutePosition()+anaglyphdist*viewVec);
    Renderer->getSceneManager()->drawAll();
    Renderer->getVideoDriver()->getOverrideMaterial().Material.ColorMask=irr::video::ECP_ALL;
    Renderer->getVideoDriver()->getOverrideMaterial().EnableFlags=irr::video::EMF_COLOR_MASK;
    Renderer->getVideoDriver()->getOverrideMaterial().EnablePasses=irr::scene::ESNRP_SKY_BOX+irr::scene::ESNRP_SOLID+irr::scene::ESNRP_TRANSPARENT+irr::scene::ESNRP_TRANSPARENT_EFFECT+irr::scene::ESNRP_SHADOW;
    Camera->getNode()->setPosition(oldPos);
  }
  else {
    // Renders Irrlicht scene
    Renderer->getSceneManager()->drawAll();
  }
}

void CCore::finalizeUpdate()
{

  irr::scene::ICameraSceneNode *cam = Renderer->getSceneManager()->getActiveCamera();

  if(cam)
  {
    irr::core::vector3df cam_pos = cam->getAbsolutePosition();
    irr::core::vector3df cam_direction =
#ifdef SOUND_CAUDIO
    cam_pos - cam->getTarget();
#else
    cam->getTarget() - cam_pos;
#endif

    SoundManager->setListenerPositionAndDirection(cam_pos, cam_direction);

#ifdef ENGINE_DEVELOPMENT_MODE

    //if(Input->isKeyPressedOnce(irr::KEY_KEY_H)) showFPS ++;
    if(showFPS > 2) showFPS = 0;

    //if(cam && showFPS != 0)
    //{
      irr::core::vector3df campos = cam->getPosition();

      update_debug += time.delta;
      if(update_debug > 1) {
        update_debug = 0.f;
        debug_delta = irr::u32(time.delta*1000);
        debug_fps = irr::u32( 1/time.delta );
      }

      irr::core::stringw fpsStr = L"FPS: ";
      fpsStr += debug_fps;
      //fpsStr += Renderer->getVideoDriver()->getFPS();

      fpsStr += "\nDelta: ";
      fpsStr += debug_delta;
      fpsStr += "ms";

      fpsStr += "\nPOS: ";
      fpsStr += irr::s32(campos.X);
      fpsStr += " ";
      fpsStr += irr::s32(campos.Y);
      fpsStr += " ";
      fpsStr += irr::s32(campos.Z);

      if(showFPS == 2)
      {
        irr::u32 totalRAM=0,availRAM=0;

        Renderer->getDevice()->getOSOperator()->getSystemMemory(&totalRAM, &availRAM);

        fpsStr += "\nNodes: ";
        fpsStr += Renderer->getSceneManager()->getRootSceneNode()->getChildren().getSize();
        fpsStr += "\nPrims: ";
        fpsStr += Renderer->getVideoDriver()->getPrimitiveCountDrawn();
        fpsStr += "\nMem avail: ";
        fpsStr += availRAM;
        fpsStr += "\nWind Direction: ";
        if (AtmoManager->getWindPacked().Z>0.707)
            fpsStr += " NORTH";
        if (AtmoManager->getWindPacked().Z<-0.707)
            fpsStr += " SOUTH";
        if (AtmoManager->getWindPacked().X>0.707)
            fpsStr += " WEST";
        else if (AtmoManager->getWindPacked().X<-0.707)
            fpsStr += " EAST";
        fpsStr += "\nWind Speed M/s: ";
        fpsStr += AtmoManager->getWindPacked().Y/5.f;
      }

      Renderer->getGUI()->getBuiltInFont()->draw(fpsStr.c_str(), irr::core::rect<irr::s32>(10,10,300,200), irr::video::SColor(255,255,255,255));
    //}
#endif
  }

  Renderer->getVideoDriver()->endScene();

  if(requestAppClose)
    close();
}


void CCore::close()
{
  bIsRunning = false;

#ifdef PHYSICS_NEWTON
  PhysicsManager->close();
#endif
}






