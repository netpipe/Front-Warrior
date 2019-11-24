#ifndef H_ATMOSPHERE
#define H_ATMOSPHERE

#include "Engine.h"

namespace engine {

  struct SAtmosphereFront
  {
    irr::core::vector2df pos;
    irr::core::vector2df direction;
    irr::f32 pressure;
    irr::f32 temperature;
  };

  class CAtmosphereManager
  {
  private:

    engine::CCore* Core;

    irr::f64 lat,lon;

    irr::f32 terrainRoughness,speed,tempKelvins,pressure;

    SAtmosphereFront fronts[128];

    irr::f32 getMsecAtAltitude(irr::f32 altitude);

    irr::core::vector3df windDirAndSpeedPckd;

    irr::u32 lastTime;

    void moveFront(irr::u32 index, irr::u32 otherIndex, irr::u32 dTime);

  public:

    CAtmosphereManager(engine::CCore* core);

    void setRoughness(irr::f32 roughClass);

    void update();

    irr::core::vector3df getWindPacked() { return windDirAndSpeedPckd; }
  };

}

#endif
