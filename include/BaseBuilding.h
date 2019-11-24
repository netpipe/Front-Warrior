#ifndef BASE_BUILDING_HEADER_DEFINED
#define BASE_BUILDING_HEADER_DEFINED

#include "BaseMapObject.h"

namespace game {

  class CBaseBuilding : public engine::CBaseMapObject
  {
  public:

    CBaseBuilding() {
      setType(EOT_BUILDING);
    }
    ~CBaseBuilding() { }

  private:

  };

}

#endif
