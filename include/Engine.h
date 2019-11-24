#ifndef ENGINE_CLASSES_HEADER_DEFINED
#define ENGINE_CLASSES_HEADER_DEFINED

#include <irrlicht.h>
#include "CompileConfig.h"
#include "BatchingMesh.h"

#ifdef SOUND_CAUDIO
namespace cAudio {
  class IAudioSource;
}
#endif

#ifdef SOUND_IRRKLANG
namespace irrklang {
  class ISound;
}
#endif


namespace engine {

#ifdef PHYSICS_NEWTON
namespace physics {
  class CBody;
}
#endif

class CCore;
class CObjectManager;
class CBaseCharacter;
class CBaseMapObject;
class CRenderer;
class CMaths;
class CTimer;
class CConfiguration;
class CPhysicsManager;
class CCamera;
class CSoundManager;
class CVehicleManager;
class CDoor;
class CAtmosphereManager;
class CTerrainNode;

#ifdef GRASS_2
class CGrassSceneNode;
#endif

enum E_TIMERS
{
  TIMER_SKIRMISH = 0
};

enum ETriggerType
{
  ETT_SHOW_NODE = 0,
  ETT_HIDE_NODE = 1,
  ETT_SPAWNPOINT
};

struct SMaterialPair
{
  irr::core::stringc name;
  irr::u16 material;
};

struct SFoilageGroupElement
{
  irr::core::vector3df position, rotation, scale;
  irr::u8 type;
};

struct SFoilageGroup
{
  irr::u16 ID;
  irr::core::array<irr::u16> neighbours;
  irr::core::vector3df position;

#ifndef GRASS_2
  irr::scene::CBatchingMesh *meshHigh, *meshMed, *meshLow;
  bool added;
#else
  irr::core::array<SFoilageGroupElement *> elements;
#endif
};

struct S3DSound
{
  irr::core::stringc name;
  irr::core::stringc filename;
  irr::f32 range;
  irr::f32 strength;
  irr::f32 attenuation;
  bool looped;
};

struct STriggerObject
{
  irr::u16 Team;
  ETriggerType Type;
  irr::f32 Range;
  irr::core::vector3df Position;
  irr::scene::ISceneNode *Node;

  STriggerObject()
  {
     Node = (irr::scene::ISceneNode*)NULL;
  }
};

struct SGrassObject
{
  irr::core::stringc objectName;
  irr::core::stringc generationSourceFileName;
  irr::f32 patchSize;
};

struct SSpawnpoint
{
  irr::core::vector3df Position;
  irr::f32 Direction;
};

struct STempPhysics {
  irr::core::array<irr::scene::CBatchingMesh*> meshes;
};

struct SPhysicsMesh
{
  irr::core::array<irr::scene::IMesh*> meshes;
  irr::core::array<irr::scene::IMesh*> simple_collision_mesh;

  irr::core::array<irr::core::stringc> name;
  irr::core::array<irr::core::vector3df> position;
  irr::core::array<irr::s32> irrID;
};

enum E_DOOR_TYPE
{
  EDT_SINGLE = 0,
  EDT_DOUBLE_VERTICAL = 1,
  EFT_DOUBLE_HORIZONTAL
};

//! State enumeration for character
enum E_CHARACTER_STATES
{
  // Is the character in motion?
  ECS_MOVING           = 1 << 0,
  // Running?
  ECS_RUNNING          = 1 << 1,
  // Is the character standing up?
  ECS_STANDING         = 1 << 2,
  // Crouching or lyring?
  ECS_CROUCHING        = 1 << 3,
  ECS_LYING            = 1 << 4,
  ECS_JUMPING          = 1 << 5,
  ECS_FALLING          = 1 << 6,
  ECS_CLIMBING         = 1 << 7,
  // Character is on stair steps
  ECS_ON_STAIRS        = 1 << 8,
  ECS_DEAD             = 1 << 9
};

  // The maximum height a jump can have
  const irr::f32 MAX_JUMP_HEIGHT = 3.5f;

  // If the height of the jump is lower than this, don't jump
  const irr::f32 MIN_JUMP_HEIGHT = 1.0f;

  const irr::f32 MAX_STEP_HEIGHT = 0.910f;

  const irr::f32 MAX_CHARACTER_FALLING_SPEED = 30.0f;

  const irr::core::vector3df UPRIGHT_SCALE = irr::core::vector3df(1, 1, 1);

  const irr::core::vector3df CROUCH_SCALE = irr::core::vector3df(1, 0.50f, 1);

  const irr::core::vector3df LYING_SCALE = irr::core::vector3df(1, 0.25f, 1);

  //! Pass this struct in class constructor for characters
  struct SCharacterCreationParameters
  {
    irr::u16 Class;
    irr::u16 TeamID;
  };

  //! All the parameters for characters
  struct SCharacterParameters
  {
    irr::f32 Health, HealthMax;
    irr::f32 Stamina, StaminaMax;

    irr::u16 TeamID;

    irr::u16 Class;

    irr::s32 States;
  };

  //! Character body
  struct SCharacterRepresentation
  {
    //! Graphical node
    irr::scene::IAnimatedMeshSceneNode *Node;

    //! Selected weapon/inventory object
    irr::scene::IAnimatedMeshSceneNode *InventoryObjectNode;

#ifdef PHYSICS_NEWTON
    // Physics body
    engine::physics::CBody *PhysicsBody;
#endif
  };

  struct SCharacterStats
  {
    irr::u32 credit;
    irr::u16 kills;
    irr::u16 deaths;
    irr::u32 score;

    //! Returns the current Kill/Death value
    irr::f32 getKD();

    SCharacterStats()
    {
      credit = kills = deaths = score = 0;
    }
  };

}

#endif
