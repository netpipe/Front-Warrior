#ifndef PATHFINDER_HEADER_DEFINED
#define PATHFINDER_HEADER_DEFINED

#ifdef MICROPATHER

#include "Micropather.h"

struct SPathNode
{
  irr::s32 id;
  irr::core::vector3df position;
  irr::core::array<irr::s32> adjacent;
};

class CPathfinder : public micropather::Graph
{
  public:

    CPathfinder()
    {
      points.set_used(0);
    }

    ~CPathfinder()
    {
      points.clear();
    }

  private:

    irr::core::array<SPathNode> points;

};

#endif
#endif
