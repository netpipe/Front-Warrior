#ifndef INVENTORY_HEADER_DEFINED
#define INVENTORY_HEADER_DEFINED

#include "GameDefines.h"

namespace game {

  class CInventoryBaseObject
  {

  };

  class CInventory
  {
  public:

    CInventory()
    {
      #ifdef ENGINE_DEVELOPMENT_MODE
             printf("Inventory created\n");
      #endif

      // Make sure inventory is empty at start
      objects.set_used(0);

      // No item selected
      selectedItem = -1;
    }

    ~CInventory()
    {
      for(irr::u32 i=0; i < objects.size(); ++i)
        delete objects[i].Data;

      objects.clear();
    }

    irr::s32 getSelectedItem() { return selectedItem; }

    void setSelectedItem(irr::s32 selected) { selectedItem = selected; }

    void addItem(SInventoryItem newItem) { objects.push_back(newItem); }

    void removeItem(irr::u32 i)
    {
      if(i >= objects.size()) return;

      delete objects[i].Data;
      objects.erase(i);
    }

    void clear() { objects.clear(); }

    irr::u32 getSize() { return objects.size(); }

    SInventoryItem getItem(irr::u32 i) { return objects[i]; }



    void setItem(irr::u32 i, SInventoryItem item)
    {
      if(i >= objects.size()) return;

      objects[i] = item;
    }

  private:

    // List of inventory objects by IDs
    irr::core::array<SInventoryItem> objects;

    irr::s32 selectedItem;
  };
}

#endif
