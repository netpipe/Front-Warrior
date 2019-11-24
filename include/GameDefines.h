#ifndef GAME_DEFINES_HEADER_DEFINED
#define GAME_DEFINES_HEADER_DEFINED

#include <irrlicht.h>

#include "CompileConfig.h"

namespace game {

  class CInventoryBaseObject;

  enum E_TEAMS
  {
    E_TEAM1 = 0,
    E_TEAM2 = 1
  };

  enum E_GAME_TYPES
  {
    EGT_NOT_STARTED = 0,
    EGT_MISSION = 1,
    EGT_SKIRMISH_ANNIHILATION,
    EGT_SKIRMISH_CAPTURE_FLAG
  };

  enum E_CHARACTER_CLASSES
  {
    ECC_SOLDIER = 0,
    ECC_HEAVY_SOLDIER = 1,
    ECC_MEDIC = 2,
    ECC_ENGINEER,
    ECC_SPY,
    ECC_COUNT
  };

  struct SCharacterClassParameters
  {
    irr::f32 health;
    irr::f32 stamina;
    irr::f32 move_speed;
    irr::core::stringc class_name;
  };


  struct SGameParameters
  {
    irr::u16 Type;
    irr::u16 TeamID;
    irr::u16 Class;
    irr::core::stringc Level;

    struct
    {
     irr::u8 BotCount;
     irr::u8 TimeHours, TimeMinutes;
     irr::u16 StartCredit;
    } Skirmish;

    struct
    {
     irr::u8 ID;
    } Mission;
  };

  /*
    LEVEL OBJECTS IDENTIFICATION
  */

  enum E_OBJECT_TYPES
  {
    EOT_STATIC = 0,
    EOT_DYNAMIC = 1,
    EOT_BUILDING,
    EOT_DOOR,
    EOT_INTERACTABLE,
    EOT_LADDER,
    EOT_PLAYER,
    EOT_BOT,
    EOT_VEHICLE,
    EOT_TERRAIN
  };

  enum E_BODY_MATERIAL_TYPE
  {
    EBMT_GROUND = 0,
    EBMT_FLESH = 1,
    EBMT_METAL = 2,
    EBMT_STONE,
    EBMT_WOOD,
    EBMT_FOILAGE,
    EBMT_SANDBAG,
    EBMT_COUNT
  };

  struct SObjectData
  {
    irr::u16 container_id, element_id;

    E_OBJECT_TYPES type;

    E_TEAMS team;

    irr::core::stringc name;

    irr::u16 material;
  };

  /*
    INVENTORY
  */

  enum E_INVENTORY_TYPE
  {
    INV_WEAPON,
    INV_TOOL
  };

  struct SInventoryItem
  {
    //! What kind of object is it
    E_INVENTORY_TYPE Type;

    //! User-defined class for each specific kind of object. Eg. convert to CWeapon* for weapons or CTool* for tools.
    CInventoryBaseObject *Data;
  };


  /*
    WEAPONS
  */

  enum E_WEAPON_CLASS
  {
    EWC_TRACED = 0,
    EWC_PROJECTILE_LAUNCHER = 1
  };

  enum E_WEAPON_TYPE
  {
    INV_GOV_MACHINE_GUN = 0,
    INV_NOVA_MACHINE_GUN = 1,
    INV_GOV_SNIPER_RIFLE,
    INV_NOVA_SNIPER_RIFLE
  };

  struct SWeaponParameters
  {
    SWeaponParameters()
    {
      fireSounds.set_used(0);
      reloadSounds.set_used(0);
    }

    ~SWeaponParameters()
    {
      fireSounds.clear();
      reloadSounds.clear();
    }

    E_WEAPON_CLASS w_class;

    irr::f32 fireRate;

    irr::u32 clipSize;

    irr::f32 reloadTime;

    irr::u32 startClips;

    irr::f32 recoilForce;

    irr::f32 fireRange;

    // First person
    irr::core::stringc f_mesh;
    irr::core::stringc f_muzzle;
    irr::core::vector3df f_position;
    irr::core::vector3df f_scale;

    // Third person
    irr::core::stringc t_mesh;
    irr::core::stringc t_muzzle;
    irr::core::vector3df t_scale;

    irr::core::array<irr::core::stringc> fireSounds;
    irr::core::array<irr::core::stringc> reloadSounds;
  };

  struct SWeaponCreationParameters
  {
    E_WEAPON_TYPE type;

    E_WEAPON_CLASS w_class;

    irr::scene::ISceneNode *node;
  };

  struct SAmmoClip
  {
    //! Ammo remaining in clip
    irr::u16 ammo;

    //! Size of the ammo clip
    const irr::u16 size;

    bool empty;

    SAmmoClip(irr::u16 csize) : size(csize)
    {
      empty = false;
    }
  };

  /*
    GUI
  */

  enum E_LOADING_SCREEN
  {
    ELS_STARTUP = 0,
    ELS_LOAD_LEVEL = 1
  };

  enum E_GUI_TEXTURES
  {
    TEX_CROSSHAIR_FRIENDLY = 0,
    TEX_CROSSHAIR_ENEMY,
    TEX_TIME_ICON,
    TEX_CREDIT_ICON,
    TEX_OVERLAY_BINOCULARS,
    TEX_OVERLAY_SNIPER,
    TEX_AMMO_ICON,
    TEX_CLIP_ICON,
    TEX_HEALTH_ICON,
    TEX_STAMINA_ICON,
    TEX_HUD_BG,
    TEX_HUD_BG_VERTICAL,
    TEX_HUD_BG_2,
    TEX_HEALTHBAR_BG,
    TEX_HEALTHBAR,
    TEX_HAND_ICON,
    TEX_TERMINAL_BACKGROUND,
    TEX_TERMINAL_EXIT_BUTTON,
    TEX_TERMINAL_REFILL_ICON,
    TEX_TERMINAL_CHANGE_TEAM_ICON,
    TEX_TERMINAL_BUY_ICON,
    TEX_TERMINAL_CHANGE_CHARACTER_ICON,
    TEX_COUNT
  };

  enum E_CROSSHAIR_ICONS
  {
    ECI_HAND                = 1 << 0
  };

  struct SCrosshairObjectData
  {
    irr::core::stringw name;

    irr::f32 healthLevel; /// 0.f ... 1.0f

    E_TEAMS team;

    irr::core::vector3df hit_position;
    irr::f32 distance;

    E_OBJECT_TYPES type;
    irr::u32 elementNumber;

    irr::u32 icons;

    bool visible;
    bool showCrosshair;
    bool showHealthbar;
    bool interactable;
  };

  /*
    MENU
  */

  enum EMENU_IDENTIFIERS
  {
     EMI_UNDEFINED = 0,
     EMI_MAIN_MENU = 1,
     EMI_SINGLE_PLAYER = 2,
     EMI_MULTIPLAYER,
     EMI_LOADGAME,
     EMI_OPTIONS,
     EMI_EXIT,
     EMI_SINGLE_PLAYER_CAMPAIGN,
     EMI_SINGLE_PLAYER_SKIRMISH,
     EMI_IN_GAME_MENU,
     EMI_COUNT
  };

  enum EGAME_GUI_ID
  {
    MM_SINGLE_PL_START_GAME_BTN = 5000,
    MM_SINGLE_PL_NAME_TB = 5001,
    MM_SINGLE_PL_TEAM_CB,
    MM_SINGLE_PL_CLASS_CB,
    MM_SINGLE_PL_PRIMARY_WPN_CB,
    MM_SINGLE_PL_SECONDARY_WPN_CB,
    MM_SINGLE_PL_TEAM_IMAGE,
    MM_SINGLE_PL_LEVEL_SELECT,
    MM_SINGLE_PL_CLASS_IMAGE,
    MM_SINGLE_PL_SIDE_GOVERMENT_BTN,
    MM_SINGLE_PL_SIDE_NOVA_BTN,
    MM_SINGLE_PL_SKIRMISH_START_GAME_BTN,
    MM_SINGLE_PL_MISSION_SELECT,
    MM_SINGLE_PL_MISSION_BRIEFING_TEXT,
    MM_SINGLE_PL_SKIRMISH_GAME_TYPE_SELECT,
    MM_SINGLE_PL_SKIRMISH_TIME_SELECT,
    MM_SINGLE_PL_SKIRMISH_VICTORY_CONDITION_SELECT,
    MM_SINGLE_PL_SKIRMISH_LEVEL_THUMBNAIL_IMAGE,
    MM_SINGLE_PL_SKIRMISH_LEVEL_NEXT_BUTTON,
    MM_SINGLE_PL_SKIRMISH_LEVEL_PREV_BUTTON,
    MM_SINGLE_PL_SKIRMISH_LEVEL_NAME_TEXT,
    MM_SINGLE_PL_SKIRMISH_CREDIT_SELECT,
    MM_MULTIPLAYER_SERVER_SOURCE_SELECT,
    MM_MULTIPLAYER_SERVERS_TABLE,
    TERMINAL_EXIT_BUTTON,
    TERMINAL_REFILL_BUTTON,
    TERMINAL_CHANGE_TEAM_BUTTON,
    TERMINAL_BUY_ITEMS_BUTTON,
    TERMINAL_CHANGE_CHARACTER_TYPE_BUTTON
  };

  enum E_MAIN_MENU_TYPE
  {
    EMMT_UNDEFINED = 0,
    EMMT_DEFAULT = 1,
    EMMT_IN_GAME
  };

  struct SMainMenuData
  {
    irr::core::array<irr::video::ITexture*> texture;

    SMainMenuData(){
      texture.set_used(0);
      skirmish_level_index = 0;
    }

    // Skirmish mode
    E_TEAMS skirmish_team;
    irr::s32 skirmish_level_index;
  };

  enum E_GUI_FADE_ID
  {
    EGFI_TERMINAL_ACTIVATE = 0,
    EGFI_TERMINAL_DEACTIVATE
  };

}

#endif
