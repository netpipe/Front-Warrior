#ifndef FW_DOOR_HEADER_DEFINED
#define FW_DOOR_HEADER_DEFINED

#include "Engine.h"

namespace engine {

  enum E_DOOR_STATE
  {
    EDS_CLOSED = 0,
    EDS_OPENED = 1,
    EDS_OPENING,
    EDS_CLOSING
  };

  class CDoor
  {
  public:

    CDoor(CCore * core, E_DOOR_TYPE type) : m_Core(core), m_Type(type)
    {
      a_Bodies.set_used(0);
      a_ActivationPoints.set_used(0);
      b_Automatic = false;
      b_Locked = false;
    }

    ~CDoor()
    {

    }

    void addBody(physics::CBody *body) { a_Bodies.push_back(body); }

    void addActivationPoint(irr::core::vector3df pos) { a_ActivationPoints.push_back(pos); }

    void update();

    void checkProximity(irr::core::vector3df pos);

    void setIsAutomatic(bool autom) { b_Automatic = autom; }

    void setDoorPositions(
      irr::u16 idx,
      irr::core::vector3df start,
      irr::core::vector3df end) {

      startPosition[idx] = start;
      endPosition[idx] = end;
    }

    void setIsLocked(bool lock) { b_Locked = true; }

    bool isLocked() { return b_Locked; }

  private:

    CCore * m_Core;

    E_DOOR_TYPE m_Type;

    E_DOOR_STATE m_State;

    irr::core::array<physics::CBody *> a_Bodies;

    irr::core::array<irr::core::vector3df> a_ActivationPoints;

    irr::core::vector3df startPosition[4];

    irr::core::vector3df endPosition[4];

    bool b_Automatic;

    bool b_Locked;
  };

}

#endif
