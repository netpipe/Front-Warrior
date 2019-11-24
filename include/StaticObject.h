#ifndef STATIC_OBJECT_HEADER_DEFINED
#define STATIC_OBJECT_HEADER_DEFINED

#include "BaseMapObject.h"

namespace game {

  class CStaticObject : public engine::CBaseMapObject
  {
  public:

    CStaticObject() {
      setType(EOT_STATIC);
    }
    ~CStaticObject() { }

  private:

  };

}

#endif
