#ifndef SYSTEM_HEADER_DEFINED
#define SYSTEM_HEADER_DEFINED

namespace engine {
    class CCore;
}

#include "Engine.h"
#include "Atmosphere.h"

namespace engine {

	typedef irr::core::rect<irr::s32> rectangle;

  //#define rectangle(x,y,w,h)

  const irr::core::vector3d<irr::f32> NULLVECTOR = irr::core::vector3d<irr::f32>(0,0,0);

  class CCore
  {
  private:

    // Class containing the Irrlicht renderer
    CRenderer *Renderer;

    // Manages input (using Irrlicht event receiver)
    //CInput *Input;

    // Graphics, sound and controls conf.
    CConfiguration *Configuration;

    // Manages & contrains all level objects
    // All vehicles, characters, level buildings, etc. are added trough this.
    CObjectManager* Objects;

    CPhysicsManager *PhysicsManager;

    CMaths *Math;

    CCamera *Camera;
    irr::scene::ICameraSceneNode* anaglyphCam;
    float anaglyphdist;

    CSoundManager *SoundManager;

    CAtmosphereManager *AtmoManager;

    CTimer * Timer;

    bool bIsRunning, requestAppClose;

    bool b_Paused;

  public:

    // Constructor
    CCore() { b_Paused = false; }

    // Destructor
    ~CCore();

    // Some basic functions
    void loadCommandLineParameters(int argc, char *argv[]){ commandLineParameters.init(argc, argv); }

    irr::u16 init();

    void close();

    void requestClose() { requestAppClose = true; }

    void update();
    void render();

    // Application is closed when the current frame is finished.
    void finalizeUpdate();

    // Access other members
    CRenderer *getRenderer(){ return Renderer; }
    //CInput *GetInput(){ return Input; }
    CConfiguration *getConfiguration(){ return Configuration; }
    CObjectManager *getObjects(){ return Objects; }
    CMaths *getMath() { return Math; }
    CPhysicsManager *getPhysics(){ return PhysicsManager; }
    CSoundManager *getSound() { return SoundManager; }
    CAtmosphereManager *GetAtmo() { return AtmoManager; }
    CCamera *getCamera() { return Camera; }
    CTimer *getTimer() { return Timer; }

    // Is the main cycle running?
    bool isRunning(){ return bIsRunning; }

    void setPaused(bool pause) { b_Paused = pause; }

    bool isPaused() { return b_Paused; }

    struct STime
    {
        irr::f32 delta;
        irr::u32 total;

        STime()
        {
            delta = 0.f;
            total = 0;
        }

    } time;



    struct SCommandLineParameters
    {
      public:

      void init(int argc, char *argv[])
      {
         for(irr::u8 i=1; i < argc; ++i) {
           values.push_back( irr::core::stringc(argv[i]) );
         }
      }

      bool hasParam(const irr::c8 *paramName)
      {
        for(irr::u8 i = 0; i < values.size(); ++i)
          if(values[i] == paramName) return true;

        return false;
      }

      irr::core::stringc getParamValue(const irr::c8 *paramName, irr::u8 valueIndex = 1)
      {
        for(irr::u32 i = 0; i < values.size(); ++i)
          if(values[i] == paramName) {
            return values[i + valueIndex];
          }

        return irr::core::stringc("");
      }

      private:
      irr::core::array<irr::core::stringc> values;

    } commandLineParameters;


  };

}

#endif
