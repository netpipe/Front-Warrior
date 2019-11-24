#ifndef SOUND_MANAGER_HEADER_DEFINED
#define SOUND_MANAGER_HEADER_DEFINED

#include <Engine.h>

#ifdef SOUND_CAUDIO
  #include <cAudio.h>
#endif

#ifdef SOUND_IRRKLANG
  #include <irrKlang.h>
#endif

//#include "SoundResource.h"

namespace engine {

  class CSoundManager
  {
  public:
    CSoundManager(CCore * core);

    ~CSoundManager();

    void playSound2D(
      const irr::c8*sound_file,
      bool looped = false,
      irr::f32 vol = 0.75f,
      irr::f32 pan = 0.f,
      bool createNewSource = false);

    void playSound3D(
      const irr::c8*sound_file,
      irr::core::vector3df pos,
      bool looped,
      irr::f32 range=10.0f,
      irr::f32 strength=0.9f,
      irr::f32 attenuation=3.6f,
      bool createNewSource = false);

#ifdef SOUND_IRRKLANG
    irrklang::ISound* getSound(const irr::c8* filename);
#endif

#ifdef SOUND_CAUDIO
    cAudio::IAudioSource* getSoundResource(
      const irr::c8*sound_file,
      bool createNewSource = false);
#endif

    void update();

    void clear();

    void setListenerPositionAndDirection(irr::core::vector3df, irr::core::vector3df);

    void preloadSounds(irr::core::array<const irr::c8*> &snd_files);

    void preloadSounds(irr::core::array<irr::core::stringc> &snd_files);

    void preloadSound(const irr::c8* snd_file);

    irr::core::vector3df getListenerPosition() { return m_ListenerPosition; }

    irr::core::vector3df getListenerDirection() { return m_ListenerDirection; }

  private:

    CCore * Core;

    irr::core::vector3df m_ListenerPosition;
    irr::core::vector3df m_ListenerDirection;

#ifdef SOUND_IRRKLANG
    irrklang::ISoundEngine* engine;

    irr::core::array<irrklang::ISound*> sounds3d;
#endif

#ifdef SOUND_CAUDIO
    cAudio::IAudioManager* manager;

    irr::core::array<cAudio::IAudioSource*> sourcesToBeRemoved;
#endif
  };
}

#endif
