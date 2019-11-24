#ifndef CONFIGURATION_HEADER_DEFINED
#define CONFIGURATION_HEADER_DEFINED

#include <irrlicht.h>

namespace engine {

  struct SVideo
  {
    irr::core::dimension2d<irr::u16> windowSize;
    irr::core::dimension2d<irr::f32> aspectRatio;
    irr::u16 drawRange;
    irr::u16 grassDensity;
    irr::u16 grassRange;
    irr::u32 geomShaderGrass;
    irr::f32 geomShaderGrassDecay;
    irr::f32 gamma;
    irr::u8 colorDepth;
    irr::u8 ZDepth;
    irr::u8 renderDeviceID;
    irr::u8 textureSettings;
    irr::u8 occlusionQuality;
    irr::u8 antiAlias;
    bool isFullscreen;
    bool isTrilinearFilter;
    bool isAnistropicFilter;
    bool isShaderGrass;
    bool isDoubleBuffer;
    bool Anaglyph;
  };

  struct SAudio
  {
    irr::f32 volume;
    bool useHardware3DBuffers;
  };

  struct SControls
  {
    irr::f32 mouseSensitivity;
  };

  class CConfiguration : public virtual irr::io::IAttributeExchangingObject
  {
  private:

    SVideo Video;

    SAudio Audio;

    SControls Controls;

  public:

    CConfiguration(){ }

    // Read config file
    void deserialize();

    // Write to config file
    void serialize();

    virtual void serializeAttributes(irr::io::IAttributes* out, irr::io::SAttributeReadWriteOptions* options=0);

    virtual void deserializeAttributes(irr::io::IAttributes* in, irr::io::SAttributeReadWriteOptions* options=0);

    SVideo *getVideo(){ return &Video; }
    SAudio *getAudio() { return &Audio; }
    SControls *getControls(){ return &Controls; }

  };

}

#endif
