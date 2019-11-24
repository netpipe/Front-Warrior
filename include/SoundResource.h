/*#ifndef SOUND_RESOURCE_HEADER_DEFINED
#define SOUND_RESOURCE_HEADER_DEFINED

namespace engine
{
  enum E_SOUND_RESOURCE_TYPE
  {
    ESRT_2D = 0,
    ESRT_3D = 1
  };

  class CSoundResource
  {
  friend class CSoundManager;

  private:

    // Irrklang sound object contains all the parameters:
    // volume, panning, 3d position, etc.
#ifdef SOUND_IRRKLANG
    irrklang::ISound *m_sound;
#endif

#ifdef SOUND_CAUDIO
    cAudio::IAudioSource* m_sound;
#endif

    E_SOUND_RESOURCE_TYPE m_type;

    bool m_notifyDeletion;

  public:

    CSoundResource(E_SOUND_RESOURCE_TYPE type) {
      m_type = type;
      m_notifyDeletion = false;
    }

    void remove()
    {
      m_notifyDeletion = true;
    }

    // Overwrite all these methods for another sound engine
#ifdef SOUND_IRRKLANG

    ~CSoundResource() {
      m_sound->drop();
    }

    void stop() {
      m_sound->stop();
      remove();
    }

    void setPaused(bool pause) {
      m_sound->setIsPaused(pause);
    }

    void setLooped(bool loop) {
      m_sound->setIsLooped(loop);
    }

    void setPosition(irr::core::vector3df &pos) {
      m_sound->setPosition(pos);
    }

    void setVolume(irr::f32 vol) {
      m_sound->setVolume(vol);
    }

    void setPan(irr::f32 pan) {
      m_sound->setPan(pan);
    }



    irr::f32 getVolume() {
      return m_sound->getVolume();
    }

    irr::f32 getPan() {
      return m_sound->getPan();
    }

    irrklang::vec3df getPosition() {
      return m_sound->getPosition();
    }

    bool isFinished() {
      return m_sound->isFinished();
    }

    bool isPaused() {
      return m_sound->getIsPaused();
    }

    bool isLooped() {
      return m_sound->isLooped();
    }

    irrklang::ISound *getSound() {
      return m_sound;
    }

    void setSound(irrklang::ISound* sound) {
      m_sound = sound;
    }
#endif

    E_SOUND_RESOURCE_TYPE getType() {
      return m_type;
    }

  };
}

#endif
*/
