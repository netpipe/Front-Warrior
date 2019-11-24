#ifndef BASE_MAP_OBJECT_HEADER_DEFINED
#define BASE_MAP_OBJECT_HEADER_DEFINED

#include "GameDefines.h"
#include "Physics.h"

namespace engine {

  struct SMergedItem {
    irr::s32 irrID;
    irr::core::stringc name;
  };

  class CBaseMapObject
  {
  public:

    // Constructor and deconstructor
    CBaseMapObject() {
      decalPositions.set_used(0);
    }

    ~CBaseMapObject() {
#ifdef PHYSICS_IRR_NEWT
      for(irr::u16 i = 0; i < mBodies.size(); ++i)
      {
        delete (game::SObjectData*)mBodies[i]->getUserData();
        mBodies[i]->removeBody();
      }

      mBodies.clear();
#endif

#ifdef PHYSICS_NEWTON
      /*for(irr::u16 i = 0; i < mBodies.size(); ++i)
      {
        delete (game::SObjectData*)mBodies[i]->getUserData();
        //mBodies[i]->removeBody();
      }

      mBodies.clear();*/

      delete (game::SObjectData*)m_Body->getUserData();
#endif

    }

#ifdef PHYSICS_NEWTON
    //irr::core::array<physics::CBody *> getBodies() { return mBodies; }

    physics::CBody * getBody() { return m_Body; }

    /*void setBodies(irr::core::array<physics::CBody *> bodies)
    {
      mBodies = bodies;
    }*/

    void setBody(physics::CBody * body) {
      m_Body = body;
    }

    /*void addBody(physics::CBody * body)
    {
      mBodies.push_back(body);
    }*/

    //irr::u32 getBodyCount() { return mBodies.size(); }

    void setBodyData(void* data)
    {
      /*SMergedItem mergeItem;
        mergeItem.name = name;
        mergeItem.irrID = irr_id;
      mergedItems.push_back(mergeItem);*/

      //mBodies[body_id]->setUserData(data);
      m_Body->setUserData(data);
    }
#endif



#ifdef PHYSICS_IRR_NEWT
    irr::core::array<irr::newton::IBody *> getBodies() { return mBodies; }

    irr::newton::IBody * getBodyByID(irr::u32 id) { return mBodies[id]; }

    void setBodies(irr::core::array<irr::newton::IBody *> bodies)
    {
      mBodies = bodies;
    }

    irr::u32 getBodyCount() { return mBodies.size(); }

    void setBodyData(irr::u16 body_id, void* data, irr::core::stringc name, irr::s32 irr_id)
    {
      SMergedItem mergeItem;
        mergeItem.name = name;
        mergeItem.irrID = irr_id;
      mergedItems.push_back(mergeItem);

      mBodies[body_id]->setUserData(data);
    }
#endif

    irr::scene::IMeshSceneNode * getNode() { return mNode; }

    //SMergedItem *getMergedItem(irr::u16 i) { return &mergedItems[i]; }

    void setNode(irr::scene::IMeshSceneNode * node) { mNode = node; }

    void setType(irr::u32 t) { mType = t; }

    bool isDecalAtPosition(irr::core::vector3df &pos, irr::f32 distance, bool add_new_decal_position=true) {
      for(irr::u32 i=0; i<decalPositions.size(); ++i)
        if(decalPositions[i].getDistanceFromSQ(pos) < distance) return true;

      if(add_new_decal_position) decalPositions.push_back(pos);

      return false;
    }

  private:

#ifdef PHYSICS_IRR_NEWT
  irr::core::array<irr::newton::IBody *> mBodies;
#endif

#ifdef PHYSICS_NEWTON
  //irr::core::array<physics::CBody *> mBodies;
  physics::CBody * m_Body;
#endif

  irr::scene::IMeshSceneNode *mNode;

  //irr::core::array<SMergedItem> mergedItems;

  irr::u32 mType;

  irr::core::array<irr::core::vector3df> decalPositions;
  };

}


#endif
