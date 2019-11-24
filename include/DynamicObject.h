#ifndef DYNAMIC_OBJECT_HEADER_DEFINED
#define DYNAMIC_OBJECT_HEADER_DEFINED

#include "BaseMapObject.h"

namespace game {

  class CDynamicObject : public engine::CBaseMapObject
  {
  public:

    CDynamicObject() {
      setType(EOT_DYNAMIC);
    }

    ~CDynamicObject() { }

  private:

  };

}

#endif
