#ifndef OBJECT_MANAGER_HEADER_DEFINED
#define OBJECT_MANAGER_HEADER_DEFINED

#include "Engine.h"

#include "StaticObject.h"
#include "DynamicObject.h"
#include "BaseBuilding.h"

#ifdef GRASS_2
  #include "GrassSceneNode.h"
#endif

namespace engine {

  struct SAmbientSound
  {
#ifdef SOUND_IRRKLANG
    irrklang::ISound * sound;
#endif
    irr::core::vector3df position;
    irr::f32 range;
    irr::f32 attenuation;
    irr::f32 strength;
  };

  class CObjectManager
  {
  public:

    CObjectManager(CCore *);

    ~CObjectManager();

    void copyMaterialToMesh(irr::scene::IMesh *mesh, irr::scene::ISceneNode *node);

    void calculateLightmapForNode(irr::scene::IMeshSceneNode* node);

    bool addObject(irr::scene::ISceneNode *node);

    void loadLevel(irr::core::stringc levelFile);

    engine::CVehicleManager *getVehicles(){ return Vehicles; }

    irr::core::stringc getObjectSimpleName(const irr::c8*name);
    irr::core::stringc getNodeFullName(const irr::c8*name);
    irr::scene::ISceneNode *getSceneNodeFromName(const irr::c8*name);

    irr::scene::ISceneNode *getRootParent(irr::scene::ISceneNode* node);

    irr::u16 getMaterialFromName(irr::core::stringc);

    void update(irr::f32 &);

    void clearAll(bool app_close=false);

    irr::f32 GetGrassWave(irr::u32 id) { return grassWave[id]; }

    //irr::core::vector3df GetSunPosition() { return SunPosition; }

    bool isNodeParameterSet(irr::core::stringc nodename, const irr::c8 *paramname)
    {
        irr::core::stringc param_n = "-";
        param_n += paramname;

        if(nodename.find(param_n.c_str()) != -1)
          return true;
        else
          return false;
    }

    irr::scene::ICameraSceneNode *getCamera() { return camera; }

    void getSpawnpointsForTeam(irr::u16 &teamid, irr::core::array<SSpawnpoint> &out);

    void createPlaneOverflight(irr::core::vector3df);

    struct SLevelParameters
    {
      irr::core::array<SGrassObject> grassObjects;

      irr::core::vector3df sunPosition;
      irr::core::vector2df lightValues;
      irr::f32 fogStrenght;
      irr::video::SColor fogColor;
      irr::video::SColor ambientColor;
      irr::video::SColor backgroundSkyColor;
      irr::core::stringc skyTextureFile;
      irr::core::stringc levelName;
      irr::core::stringc ambientSound;
      bool fogEnabled;

      SLevelParameters()
      {
        fogEnabled = false;
        skyTextureFile = "";
        grassObjects.set_used(0);
      }

    } parameters;

    irr::u32 getStaticObjectCount() { return staticList.size(); }
    irr::u32 getDynamicObjectCount() { return dynamicList.size(); }
    irr::u32 getBuildingCount() { return buildingList.size(); }
    irr::u32 getInteractableCount() { return interactableList.size(); }

    game::CStaticObject* getStaticObjectById(irr::u32 i) { return staticList[i]; }
    game::CDynamicObject* getDynamicObjectById(irr::u32 i) { return dynamicList[i]; }
    game::CBaseBuilding* getBuildingById(irr::u32 i) { return buildingList[i]; }
    CDoor* getDoorById(irr::u32 i) { return doorList[i]; }
    game::CStaticObject* getInteractableById(irr::u32 i) { return interactableList[i]; }

    CBaseMapObject* getObject(irr::u16 type, irr::u32 i) {
      if(type == 0) return (CBaseMapObject*)getStaticObjectById(i);
      else if(type == 1) return (CBaseMapObject*)getDynamicObjectById(i);
      else if(type == 2) return (CBaseMapObject*)getBuildingById(i);
    }

    void findCurrentFoilageGroup(irr::core::vector3df pos) {
#ifdef GRASS_2
      gNode->findClosestPatch(pos);
#else
      if(f_groups.size() == 0)
        return;

      currentFoilageGroup = f_groups[findNearestFoilageGroup(pos, true)];
#endif
    }

    SFoilageGroup* getCurrentFoilageGroup() {
      return currentFoilageGroup;
    }

#ifdef SOUND_CAUDIO
    cAudio::IAudioSource* getAmbientSound() { return ambientSoundSource; }
#endif

    void checkDoorProximity(irr::core::vector3df);

    void trigger(irr::u16 triggerIndex);

    void triggerSpawn(irr::u8 teamIndex);

    void setAmbientSoundsPaused(bool paused) {
      b_AmbientSoundsPaused = paused;
    }

  private:

    CCore * Core;

    irr::scene::ICameraSceneNode *camera;

    irr::f32 grassWave[2];

    void findAndApplyShaderMaterials(irr::scene::IMeshSceneNode*node);

    void generateGrassFile(SGrassObject);

    void loadGrassFromFile(const irr::c8 *);

#ifdef GRASS_2
    void loadGrassFromFile2(const irr::c8 *);
#endif

    void generateGrassGroupFile(
      SGrassObject,
      const irr::c8 *,
      irr::f32, irr::f32, irr::f32, irr::f32);

    void pregenerateGrassMeshes(
      const irr::c8* inFile,
      const irr::c8* objName,
      irr::f32, irr::f32, irr::f32, irr::f32) { }

    void groupNodes();

    void checkChildren(irr::scene::ISceneNode *parent);

    void groupChildrenWithParent(
      irr::scene::ISceneNode *parent,
      irr::scene::CBatchingMesh *mesh,
      irr::s32 p_mesh_index);

    void appendMesh(
      irr::scene::CBatchingMesh *mesh,
      irr::s32 p_mesh_index,
      irr::scene::ISceneNode *parent,
      irr::scene::ISceneNode *node);

    irr::scene::CBatchingMesh *GrassMesh;

    engine::CVehicleManager *Vehicles;

    // Lists
    irr::core::array<game::CStaticObject*> staticList;
    irr::core::array<game::CDynamicObject*> dynamicList;
    irr::core::array<game::CBaseBuilding*> buildingList;
    irr::core::array<game::CStaticObject*> interactableList;
    irr::core::array<CDoor*> doorList;

    // Triggers are object that .. well .. trigger events
    irr::core::array<STriggerObject> triggerObjects;

    irr::s16 getSoundIndexFromName(irr::core::stringc name)
    {
      for(irr::u16 i=0; i<a_Sounds.size();++i)
        if(a_Sounds[i].name == name)
          return i;

      return -1;
    }

#ifdef GRASS_2
    CGrassSceneNode *gNode;
#endif

    irr::core::array<S3DSound> a_Sounds;

    irr::core::array<SAmbientSound> a_AmbientSounds;

    irr::core::array<irr::u16> spawnTriggers[2];

    irr::core::array<SFoilageGroup *> f_groups;

    SFoilageGroup *currentFoilageGroup;

    irr::scene::IMeshSceneNode *foilageNode;

    void regenerateGrassMesh(irr::core::vector3df position);

    irr::s32 findNearestFoilageGroup(irr::core::vector3df position, bool absolute=false);

    void findFoilageGroupsNeighbours();

    void updateTriggers();

    void loadLevelSettings(const irr::c8* name);

    //CSoundResource *ambientSoundResource;
#ifdef SOUND_CAUDIO
    cAudio::IAudioSource* ambientSoundSource;
#endif

    irr::core::array<irr::scene::CBatchingMesh*> grassMeshes;

    irr::core::array<irr::scene::CBatchingMesh*> levelMeshes;

    irr::core::array<SMaterialPair *> nameMaterials;

    void loadNameMaterials();

    void createDoor(irr::scene::ISceneNode *);

    bool b_AmbientSoundsPaused;
  };

}

#endif
