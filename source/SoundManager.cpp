#include "SoundManager.h"
#include "Core.h"
#include "Configuration.h"

using namespace engine;

CSoundManager::~CSoundManager()
{
#ifdef SOUND_IRRKLANG

  for(irr::u16 i=0; i<sounds3d.size(); ++i)
    sounds3d[i]->drop();

  engine->removeAllSoundSources();
  engine->drop();
#endif

#ifdef SOUND_CAUDIO
  sourcesToBeRemoved.clear();
  manager->releaseAllSources();
  manager->shutDown();
  cAudio::destroyAudioManager(manager);
#endif

}

CSoundManager::CSoundManager(CCore * core) : Core(core)
{
#ifdef SOUND_IRRKLANG

  irr::s32 parameters = irrklang::ESEO_MULTI_THREADED | irrklang::ESEO_LOAD_PLUGINS;

  if(Core->getConfiguration()->getAudio()->useHardware3DBuffers == true)
    parameters |= irrklang::ESEO_USE_3D_BUFFERS;

  engine = irrklang::createIrrKlangDevice(
    irrklang::ESOD_AUTO_DETECT,
    parameters);

  if(engine)
  {
    printf("VOL %.6f\n", Core->getConfiguration()->getAudio()->volume);
    engine->setSoundVolume(Core->getConfiguration()->getAudio()->volume);
    engine->setRolloffFactor(1.65f);
  }

  sounds3d.set_used(0);
#endif

#ifdef SOUND_CAUDIO
  manager = cAudio::createAudioManager(false);
  manager->initialize(0x0, 8000, 1);


  cAudio::ILogger *log = cAudio::getLogger();
  log->setLogLevel(cAudio::ELL_CRITICAL);

  manager->getListener()->setMasterVolume(0.75f);
//  manager->getListener()->setMetersPerUnit(0.2f);

  sourcesToBeRemoved.set_used(0);
#endif

  m_ListenerPosition = NULLVECTOR;
}

#ifdef SOUND_IRRKLANG
irrklang::ISound* CSoundManager::getSound(const irr::c8* filename)
{
  irrklang::ISound * snd;

  snd = engine->play2D(
    filename,
    false,
    false,
    true,
    irrklang::ESM_AUTO_DETECT,
    false);

  return snd;
}
#endif

void CSoundManager::preloadSounds(irr::core::array<const irr::c8*> &snd_files)
{
  for(irr::u32 i=0; i<snd_files.size(); ++i)
    preloadSound(snd_files[i]);
}

void CSoundManager::preloadSounds(irr::core::array<irr::core::stringc> &snd_files)
{
  for(irr::u32 i=0; i<snd_files.size(); ++i)
    preloadSound(snd_files[i].c_str());
}

void CSoundManager::preloadSound(const irr::c8* snd_file)
{
#ifdef SOUND_IRRKLANG
  engine->addSoundSourceFromFile(
    snd_file,
    irrklang::ESM_AUTO_DETECT,
    true); // preload
#endif

#ifdef SOUND_CAUDIO
  manager->create(snd_file, snd_file);
#endif
}

irr::u16 updateCounter = 0;

void CSoundManager::update()
{
#ifdef SOUND_IRRKLANG

  /*if(updateCounter == 0)
  for(irr::u16 i=0; i < sounds3d.size(); ++i)
  {
    irr::f32 min_dist = sounds3d[i]->getMinDistance() * 4.5f;
    irr::f32 dist = sounds3d[i]->getPosition().getDistanceFrom(position);

    if(dist > min_dist)
      sounds3d[i]->setIsPaused(true);
    else
      sounds3d[i]->setIsPaused(false);
  }

  updateCounter ++;

  if(updateCounter == 50)
    updateCounter = 0;*/

#endif

#ifdef SOUND_CAUDIO
//  manager->getListener()->move(cAudio::cVector3(position.X, position.Y, position.Z));
//  manager->getListener()->setDirection(cAudio::cVector3(direction.X, direction.Y, direction.Z));
#endif

}

void CSoundManager::clear()
{
#ifdef SOUND_IRRKLANG
  engine->stopAllSounds();
#endif
}

void CSoundManager::setListenerPositionAndDirection(irr::core::vector3df pos, irr::core::vector3df dir)
{
  m_ListenerPosition = pos;
  m_ListenerDirection = dir;

#ifdef SOUND_IRRKLANG
  engine->setListenerPosition(pos, dir);
#endif

#ifdef SOUND_CAUDIO
  manager->getListener()->move(cAudio::cVector3(pos.X, pos.Y, pos.Z));
  manager->getListener()->setDirection(cAudio::cVector3(dir.X, dir.Y, dir.Z));
#endif
}


void CSoundManager::playSound2D(
  const irr::c8*sound_file,
  bool looped,
  irr::f32 vol,
  irr::f32 pan,
  bool createNewSource)
{
#ifdef SOUND_IRRKLANG
  irrklang::ISound * tmpSnd = engine->play2D(
    sound_file,
    looped,
    false,
    true,
    irrklang::ESM_STREAMING,
    true);

  tmpSnd->setVolume(vol);
  tmpSnd->setPan(pan);
  tmpSnd->drop();
#endif

#ifdef SOUND_CAUDIO
  cAudio::IAudioSource* sound = getSoundResource(
    sound_file,
    createNewSource);

  sound->setVolume(vol);
  sound->play2d(looped);
#endif

  return;
}

#ifdef SOUND_CAUDIO
cAudio::IAudioSource* CSoundManager::getSoundResource(
  const irr::c8*sound_file,
  bool createNewSource)
{
  cAudio::IAudioSource* soundSource = NULL;

  if(!createNewSource)
    soundSource = manager->getSoundByName(sound_file);

  if(soundSource == NULL || createNewSource)
  {
    soundSource = manager->create(
      sound_file,
      sound_file);

    /*if(createNewSource) {
      sourcesToBeRemoved.push_back(soundSource);
    }*/
  }

  //soundSource->loop(looped);
  //soundSource->setMaxVolume(vol);

  return soundSource;
}
#endif

void CSoundManager::playSound3D(
  const irr::c8*sound_file,
  irr::core::vector3df pos,
  bool looped,
  irr::f32 range,
  irr::f32 strength,
  irr::f32 attenuation,
  bool createNewSource)
{
#ifdef SOUND_IRRKLANG
  irrklang::ISound * tmpSnd = engine->play3D(
    sound_file,
    pos,
    looped,
    true,
    true,
    irrklang::ESM_AUTO_DETECT,
    false);

  tmpSnd->setMinDistance(range);
  tmpSnd->setIsPaused(false);

  if(Core->getConfiguration()->getAudio()->useHardware3DBuffers == false)
    tmpSnd->setVolume(tmpSnd->getVolume() * 0.75f);

  if(!createNewSource)
    tmpSnd->drop();
  else
    sounds3d.push_back(tmpSnd);

#endif

#ifdef SOUND_CAUDIO

  cAudio::IAudioSource* sound = getSoundResource(
    sound_file,
    createNewSource);

  sound->play3d(cAudio::cVector3(pos.X,pos.Y,pos.Z),0.9f,looped);
  sound->setPosition(cAudio::cVector3(pos.X,pos.Y,pos.Z));
  sound->setMinVolume(0.000f);
  sound->setMinDistance(range);
  sound->setRolloffFactor(4.0f);
  sound->setMaxDistance(range*1000);
  //sound->setVolume(1.0f);
  //sound->setMinDistance(1.0f);
  //sound->setMaxDistance(vol);

#endif

  return;
}
