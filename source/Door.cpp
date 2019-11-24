#include "Core.h"
#include "Door.h"
#include "Renderer.h"
#include "SoundManager.h"
#include "newton/Body.h"

using namespace engine;
using namespace engine::physics;

using namespace irr;
using namespace irr::core;
using namespace irr::scene;
using namespace irr::video;

void CDoor::checkProximity(irr::core::vector3df pos)
{
  if(b_Automatic)
  {
    bool proximity = false;

    for(irr::u16 i=0; i < a_ActivationPoints.size(); ++i)
    {
      if(a_ActivationPoints[i].getDistanceFrom(pos) <= 6) {
        proximity = true;
        break;
      }
    }

    // Not near
    if(!proximity)
    {
      if((m_State == EDS_OPENED || m_State == EDS_OPENING) && m_State != EDS_CLOSING)
      {
        m_State = EDS_CLOSING;
        m_Core->getSound()->playSound3D(
          "data/sounds/doors/door1_close.ogg",
          a_Bodies[0]->getPosition(),
          false,
          9);
      }
    }
    else
    {
      if((m_State == EDS_CLOSED || m_State == EDS_CLOSING) && m_State != EDS_OPENING)
      {
        m_State = EDS_OPENING;
        m_Core->getSound()->playSound3D(
          "data/sounds/doors/door1_open.ogg",
          a_Bodies[0]->getPosition(),
          false,
          9);

      }
    }
  }
}

void CDoor::update()
{
  if(b_Automatic)
  {
    irr::f32 time = m_Core->time.delta;

    if(m_State == EDS_OPENING)
    {
      irr::core::vector3df pos1 = a_Bodies[0]->getPosition();
      irr::core::vector3df pos2 = a_Bodies[1]->getPosition();

      if(pos1.Y < endPosition[0].Y)
      {
        pos1.Y += 5.f * time;

        if(pos1.Y > endPosition[0].Y) {
          pos1.Y = endPosition[0].Y;
        }

        a_Bodies[0]->setPosition(pos1);
      }

      if(pos2.Y > endPosition[1].Y)
      {
        pos2.Y -= 5.f * time;

        if(pos2.Y < endPosition[1].Y) {
          pos2.Y = endPosition[1].Y;
        }

        a_Bodies[1]->setPosition(pos2);
      }
    }
    else if(m_State == EDS_CLOSING)
    {
      irr::core::vector3df pos1 = a_Bodies[0]->getPosition();
      irr::core::vector3df pos2 = a_Bodies[1]->getPosition();

      if(pos1.Y > startPosition[0].Y)
      {
        pos1.Y -= 5.f * time;

        if(pos1.Y < startPosition[0].Y) {
          pos1.Y = startPosition[0].Y;
        }

        a_Bodies[0]->setPosition(pos1);
      }

      if(pos2.Y < startPosition[1].Y)
      {
        pos2.Y += 5.f * time;

        if(pos2.Y > startPosition[1].Y) {
          pos2.Y = startPosition[1].Y;
        }

        a_Bodies[1]->setPosition(pos2);
      }
    }


  }
}
