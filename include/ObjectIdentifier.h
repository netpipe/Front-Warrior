#ifndef OBJECT_IDENTIFIER_HEADER_DEFINED
#define OBJECT_IDENTIFIER_HEADER_DEFINED

namespace game {

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
    irr::u16 material;
  };

}

#endif
