#include "Configuration.h"

using namespace engine;

void CConfiguration::deserialize()
{
  irr::IrrlichtDevice *device = irr::createDevice(irr::video::EDT_NULL);
  device->sleep(600);

  irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader("data/settings.dat");

  while(xml && xml->read()){
    switch(xml->getNodeType()){
    case irr::io::EXN_ELEMENT:
    {
      if (!strcmp("graphics", xml->getNodeName()))
      {
          Video.renderDeviceID = xml->getAttributeValueAsInt("device");
          Video.windowSize.Width = xml->getAttributeValueAsInt("width");
          Video.windowSize.Height = xml->getAttributeValueAsInt("height");
          Video.colorDepth = xml->getAttributeValueAsInt("depth");
          Video.ZDepth = xml->getAttributeValueAsInt("zdepth");
          Video.isFullscreen = xml->getAttributeValueAsInt("fullscreen");
          Video.isDoubleBuffer = xml->getAttributeValueAsInt("doublebuffer");
          Video.aspectRatio.Width = xml->getAttributeValueAsFloat("aspectw");
          Video.aspectRatio.Height = xml->getAttributeValueAsFloat("aspecth");
          Video.drawRange = xml->getAttributeValueAsInt("drawrange");
          Video.isTrilinearFilter = xml->getAttributeValueAsInt("trilinear");
          Video.isAnistropicFilter = xml->getAttributeValueAsInt("anistropic");
          Video.antiAlias = xml->getAttributeValueAsInt("antialias");
          Video.isShaderGrass = xml->getAttributeValueAsInt("shadergrass");
          Video.grassDensity = xml->getAttributeValueAsInt("grassdensity");
          Video.grassRange = xml->getAttributeValueAsInt("grassrange");
          Video.geomShaderGrass = xml->getAttributeValueAsInt("instancedgrass");
          Video.geomShaderGrassDecay = xml->getAttributeValueAsFloat("instancedgrassdecay");
          Video.Anaglyph = xml->getAttributeValueAsInt("anaglyph");
          Video.gamma = xml->getAttributeValueAsFloat("gamma");
      }
      else if (!strcmp("sound", xml->getNodeName()))
      {
          Audio.volume = xml->getAttributeValueAsFloat("volume");
          Audio.useHardware3DBuffers = xml->getAttributeValueAsInt("hardwarebuffers");
      }
      else if (!strcmp("controls", xml->getNodeName()))
      {
          Controls.mouseSensitivity = xml->getAttributeValueAsFloat("mousesensitivity");
      }
      /*else if (!strcmp("sound", xml->getNodeName())){
          GameSettings.sndDevice = xml->getAttributeValueAsInt("sndDevice");
          GameSettings.masterVolume = xml->getAttributeValueAsFloat("masterVolume");
      }
      else if (!strcmp("game", xml->getNodeName())){
          GameSettings.PlayerProfileName = stringc( xml->getAttributeValue("profile") );
      }*/
    }
    break;
    }
  }

  delete xml;

  device->closeDevice();
  device->drop();

  return;
}

void CConfiguration::serializeAttributes(
  irr::io::IAttributes* out,
  irr::io::SAttributeReadWriteOptions* options)
{

}

void CConfiguration::deserializeAttributes(
  irr::io::IAttributes* in,
  irr::io::SAttributeReadWriteOptions* options)
{

}
