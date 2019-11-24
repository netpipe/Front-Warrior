#include "Core.h"
#include "ObjectManager.h"
#include "VehicleManager.h"
#include "Renderer.h"
#include "Camera.h"
#include "Configuration.h"
#include "Maths.h"
#include "SoundManager.h"
#include "Door.h"
#include "TerrainNode.h"

#include <string>
#include <fstream>

using namespace engine;

using namespace irr;
using namespace irr::scene;
using namespace irr::video;
using namespace irr::core;

scene::IParticleSystemSceneNode *weatherParticleNode, *weatherParticleNode2;

irr::core::array<scene::ISceneNode *> rootNodes;
#ifdef PHYSICS_IRR_NEWT
newton::IBody* terrainBody = (newton::IBody*)NULL;
#endif
irr::scene::IMeshSceneNode *testNode;

bool snow_level = false;

bool disableLowDetail = false;
bool disableMediumDetail = false;

irr::f32 DISTANCE_BETWEEN_FOILAGE_GROUPS = 25.f;

const irr::u32 GRASS_MESH_BUFFER_COUNT = 10;

CObjectManager::CObjectManager(CCore * core) : Core(core)
{
  Vehicles = new engine::CVehicleManager();

  grassWave[0] = grassWave[1] = 0.f;

  grassMeshes.set_used(0);
  nameMaterials.set_used(0);
  f_groups.set_used(0);
  doorList.set_used(0);
  interactableList.set_used(0);

  loadNameMaterials();

  currentFoilageGroup = (SFoilageGroup*) NULL;
#ifdef SOUND_CAUDIO
  ambientSoundSource = NULL;
#endif

#ifdef GRASS_2
  gNode = (CGrassSceneNode*) NULL;
#endif

  foilageNode = (irr::scene::IMeshSceneNode*) NULL;
}

CObjectManager::~CObjectManager()
{
  clearAll(true);

  delete Vehicles;
}



void CObjectManager::findAndApplyShaderMaterials(irr::scene::IMeshSceneNode *node)
{
    irr::s32 lightmapShader = -1;

    // Replace certain materials
    for(irr::u32 i=0; i < node->getMaterialCount(); ++i)
    {
        if(node->getMaterial(i).isTransparent())
        {
          node->getMaterial(i).ZWriteEnable = true;
          node->getMaterial(i).ZBuffer = irr::video::ECFN_LESSEQUAL;

          if(node->getMaterial(i).MaterialType == irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL)
            node->getMaterial(i).MaterialTypeParam = 0.28f;
        }

        node->getMaterial(i).GouraudShading = false;

        irr::video::ITexture* MaterialTexture = node->getMaterial(i).getTexture(0);

        if(MaterialTexture)
        {
            irr::core::stringc texName = MaterialTexture->getName();

            char toFind = '\\';
            irr::u32 lastPos = texName.findLast( toFind ) + 1;

            irr::core::stringc textureFile = texName.subString(lastPos, texName.size() - lastPos-4);
            irr::core::stringc endStr = texName.subString(texName.size()-7, 3);

            // Multi-texture shader?
            if(endStr.equals_ignore_case("_mt") == true) // _mt
            {
                irr::s32 newmat = Core->getRenderer()->getShaders()->createMultiTextureShader();

                // R
                node->setMaterialTexture(2,
                  Core->getRenderer()->getVideoDriver()->getTexture("data/terrain/f_dirt.jpg"));  //road_dirty_gravel.tga"));//f_dirt.jpg"));
                // G
                node->setMaterialTexture(3,
                  Core->getRenderer()->getVideoDriver()->getTexture("data/terrain/f_stone.jpg"));
                // B
                node->setMaterialTexture(4,
                  Core->getRenderer()->getVideoDriver()->getTexture("data/terrain/grass2.jpg"));

                // Base texture
                if(snow_level)
                node->setMaterialTexture(1, Core->getRenderer()->getVideoDriver()->getTexture("data/terrain/snow1.jpg"));
                else
                node->setMaterialTexture(1, Core->getRenderer()->getVideoDriver()->getTexture("data/terrain/f_grass.jpg"));

                if(parameters.levelName == "industrial")
                node->setMaterialTexture(1,
                  Core->getRenderer()->getVideoDriver()->getTexture("data/terrain/groundplants.jpg"));

                node->getMaterial(i).setFlag(EMF_LIGHTING, false);
                node->getMaterial(i).MaterialType = (E_MATERIAL_TYPE)newmat;
            }

            // Special transparent textures in otherwise non-transparent objects
            /*if(textureFile.equals_ignore_case("mesh_c02") == true
            || textureFile.equals_ignore_case("bars_m01") == true
            || textureFile.equals_ignore_case("fence_c11") == true
            || textureFile.equals_ignore_case("truss_m06a") == true
            || textureFile.equals_ignore_case("mesh_c03_snow") == true)
            {
                node->getMaterial(i).MaterialType = EMT_TRANSPARENT_ALPHA_CHANNEL;
            }*/

        }

        if(node->getMaterial(i).MaterialType == irr::video::EMT_LIGHTMAP)
        {
          if(lightmapShader == -1)
          {
            //lightmapShader = Core->getRenderer()->getShaders()->createLightmapShader();
          }

          node->getMaterial(i).setFlag(EMF_LIGHTING, false);
          //node->getMaterial(i).MaterialType = (E_MATERIAL_TYPE)lightmapShader;
        }

    }
}

void CObjectManager::regenerateGrassMesh(irr::core::vector3df position)
{
#ifndef GRASS_2
  irr::scene::CBatchingMesh * foilageMesh = (irr::scene::CBatchingMesh*)foilageNode->getMesh();

  foilageMesh->clear();

  irr::f32 parentDist = currentFoilageGroup->position.getDistanceFrom(position);
  irr::s32 newMainFoilageGroup = -1;

  irr::core::aabbox3df cameraBBox = Core->getCamera()->getNode()->getViewFrustum()->boundingBox;

  for(irr::u16 i=0; i < currentFoilageGroup->neighbours.size(); ++i)
  {
    if(f_groups[currentFoilageGroup->neighbours[i]]->position.getDistanceFrom(position) < parentDist)
      newMainFoilageGroup = i;
  }

  // One of the neighbour patches is closer than the current "main"
  // Set that neighbour to be the new one.
  if(newMainFoilageGroup != -1)
    currentFoilageGroup = f_groups[currentFoilageGroup->neighbours[newMainFoilageGroup]];


  irr::core::array<irr::u16> groupsToCheck_HighDetail;
  irr::core::array<irr::u16> groupsToCheck_MedDetail;
  irr::core::array<irr::u16> groupsToCheck_LowDetail;

  irr::u16 grass_density = Core->getConfiguration()->getVideo()->grassDensity;
  irr::u16 grass_range = Core->getConfiguration()->getVideo()->grassRange;

  // High density

  groupsToCheck_HighDetail.push_back(currentFoilageGroup->ID);

  for(irr::u16 i=0; i < currentFoilageGroup->neighbours.size(); ++i)
    if(cameraBBox.intersectsWithBox(f_groups[currentFoilageGroup->neighbours[i]]->meshHigh->getBoundingBox()))
    {
      groupsToCheck_HighDetail.push_back(currentFoilageGroup->neighbours[i]);
      f_groups[currentFoilageGroup->neighbours[i]]->added = true;
    }


  // Med density
  if(!disableMediumDetail)
  for(irr::u16 gi=1; gi < groupsToCheck_HighDetail.size(); ++gi)
  {
    SFoilageGroup *g = f_groups[groupsToCheck_HighDetail[gi]];

    for(irr::u16 i=0; i < g->neighbours.size(); ++i)
    {
      if(f_groups[g->neighbours[i]]->added == false)
        if(cameraBBox.intersectsWithBox(f_groups[g->neighbours[i]]->meshMed->getBoundingBox()))
        {
          groupsToCheck_MedDetail.push_back(g->neighbours[i]);
          f_groups[g->neighbours[i]]->added = true;
        }
    }
  }

  // Low density

  if(!disableLowDetail)
  for(irr::u16 gi=1; gi < groupsToCheck_MedDetail.size(); ++gi)
  {
    SFoilageGroup *g = f_groups[groupsToCheck_MedDetail[gi]];

    for(irr::u16 i=0; i < g->neighbours.size(); ++i)
    {
      if(f_groups[g->neighbours[i]]->added == false)
        if(cameraBBox.intersectsWithBox(f_groups[g->neighbours[i]]->meshLow->getBoundingBox()))
        {
          groupsToCheck_LowDetail.push_back(g->neighbours[i]);
          f_groups[g->neighbours[i]]->added = true;
        }
    }
  }



  for(irr::u16 gi=0; gi < groupsToCheck_HighDetail.size(); ++gi)
  {
    foilageMesh->addMesh(f_groups[groupsToCheck_HighDetail[gi]]->meshHigh);
    f_groups[groupsToCheck_HighDetail[gi]]->added = false;
  }

  if(!disableMediumDetail)
  for(irr::u16 gi=0; gi < groupsToCheck_MedDetail.size(); ++gi)
  {
    foilageMesh->addMesh(f_groups[groupsToCheck_MedDetail[gi]]->meshMed);
    f_groups[groupsToCheck_MedDetail[gi]]->added = false;
  }

  if(!disableLowDetail)
  for(irr::u16 gi=0; gi < groupsToCheck_LowDetail.size(); ++gi)
  {
    foilageMesh->addMesh(f_groups[groupsToCheck_LowDetail[gi]]->meshLow);
    f_groups[groupsToCheck_LowDetail[gi]]->added = false;
  }


  foilageMesh->update(); //finalize();

  foilageNode->setMesh(foilageMesh);

  if(Core->commandLineParameters.hasParam("-disable_vbo") == false) {
    foilageNode->getMesh()->setHardwareMappingHint(scene::EHM_STATIC, EBT_VERTEX_AND_INDEX);
    foilageNode->getMesh()->setDirty(EBT_VERTEX_AND_INDEX);
  }


  static irr::s32 grassShaderMaterial = Core->getRenderer()->getShaders()->createGrassShader();

  if(Core->getConfiguration()->getVideo()->isShaderGrass == true)
    foilageNode->setMaterialType((E_MATERIAL_TYPE)grassShaderMaterial);
  else
    foilageNode->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF);
#endif

}



irr::s32 CObjectManager::findNearestFoilageGroup(irr::core::vector3df position, bool absolute)
{
  // Check all foilage groups

  irr::f32 closestDist = 999999.9f;
  irr::s32 closestId = -1;

  position.Y = 0.f;

  for(irr::u16 fgi=0; fgi < f_groups.size(); ++fgi)
  {
    irr::f32 dist = f_groups[fgi]->position.getDistanceFrom(position);

    if(dist < closestDist)
    {
      closestDist = dist;
      closestId = fgi;
    }
  }

  return closestId;
}

void CObjectManager::findFoilageGroupsNeighbours()
{
#ifndef GRASS_2
  for(irr::u16 i=0; i < f_groups.size(); ++i)
  {
    f_groups[i]->neighbours.set_used(0);

    f_groups[i]->meshHigh->finalize();
    f_groups[i]->meshMed->finalize();
    f_groups[i]->meshLow->finalize();

    for(irr::u16 j=0; j < f_groups.size(); ++j)
    {
      if(i == j) continue;

      if(f_groups[i]->position.getDistanceFrom(f_groups[j]->position) <= DISTANCE_BETWEEN_FOILAGE_GROUPS*1.5f)
        f_groups[i]->neighbours.push_back(j);
    }
  }
#endif
}

void setTangentsToPos(irr::scene::IMesh* mesh,irr::core::vector3df normal,irr::core::vector3df pos,float boundingSphere) {
    for(irr::u32 m=0; m < mesh->getMeshBufferCount(); ++m)
    {
        if (mesh->getMeshBuffer(m)->getVertexType()==irr::video::EVT_TANGENTS) {
            for (irr::u32 i=0; i<mesh->getMeshBuffer(m)->getVertexCount(); i++) {
                (((irr::video::S3DVertexTangents*)mesh->getMeshBuffer(m)->getVertices())+i)->Tangent=normal*boundingSphere;
                (((irr::video::S3DVertexTangents*)mesh->getMeshBuffer(m)->getVertices())+i)->Binormal=pos;
            }
        }
    }
}

void setGrassNormals(irr::scene::IMesh* mesh) {
    for(irr::u32 m=0; m < mesh->getMeshBufferCount(); ++m)
    {
        if (mesh->getMeshBuffer(m)->getVertexType()==irr::video::EVT_TANGENTS) {
            for (irr::u32 i=0; i<mesh->getMeshBuffer(m)->getVertexCount(); i++) {
                (((irr::video::S3DVertexTangents*)mesh->getMeshBuffer(m)->getVertices())+i)->Normal=irr::core::vector3df(0.0,1.0,0.0);
            }
        }
    }
}




#ifdef GRASS_2
void CObjectManager::loadGrassFromFile2(const irr::c8* gFile)
{
  gNode = new CGrassSceneNode(
    Core->getRenderer()->getSceneManager()->getRootSceneNode(),
    Core->getRenderer()->getSceneManager(),
    9999);
  gNode->setAutomaticCulling(EAC_OFF);

  gNode->addReferenceMesh("data/foilage/grass1_v2.ms3d");
  gNode->addReferenceMesh("data/foilage/grass2.ms3d");
  gNode->addReferenceMesh("data/foilage/grass3.ms3d");
  gNode->addReferenceMesh("data/foilage/grass5.ms3d");
  gNode->addReferenceMesh("data/foilage/grass6.ms3d");
  gNode->addReferenceMesh("data/foilage/weeds/fern.ms3d");
  gNode->addReferenceMesh("data/foilage/weeds/fern2.ms3d");
  gNode->addReferenceMesh("data/foilage/weeds/weed1.ms3d");
  gNode->addReferenceMesh("data/foilage/weeds/weed2.ms3d");
  gNode->addReferenceMesh("data/foilage/weeds/weed3.ms3d");



  std::ifstream ifs(gFile);
  std::string temp;

  irr::u32 numberOfGroups = 0;

  irr::u16 grass_density = Core->getConfiguration()->getVideo()->grassDensity;
  bool geomShaderEnabled = (Core->getConfiguration()->getVideo()->geomShaderGrass > 0);

  // Get number of groups
  getline(ifs, temp);
  sscanf(temp.c_str(), "%d", &numberOfGroups);

  /*
    Read all patches
  */

  for(irr::u32 gId=0; gId < numberOfGroups; ++gId)
  {
    irr::u32 groupID = 0, numberOfNeighbours = 0;
    irr::core::vector3df pos;
    irr::u16 neigh[10];

    getline(ifs, temp);

    sscanf(temp.c_str(), "%d %d %f %f %d %d %d %d %d %d %d %d %d %d",
      &groupID,
      &numberOfNeighbours,
      &pos.X,
      &pos.Z,
      &neigh[0], &neigh[1], &neigh[2], &neigh[3], &neigh[4],
      &neigh[5], &neigh[6], &neigh[7], &neigh[8], &neigh[9]);

    SFoilageGroup *f_group = new SFoilageGroup();

    f_group->ID = f_groups.size();

    // Center of each patch
    f_group->position = pos;

    for(irr::u32 n=0; n < numberOfNeighbours; ++n) {
      f_group->neighbours.push_back(neigh[n]);
    }

    // Add it to list
    //f_groups.push_back(f_group);

    gNode->addPatch(f_group);
  }

  /*
    Read all grass elements
  */

  while(getline(ifs, temp))
  {
    SFoilageGroupElement *element = new SFoilageGroupElement();

    irr::u32 groupID = 0;
    irr::core::vector3df normal;
    irr::core::matrix4 tempmat;

    sscanf(temp.c_str(), "%d %d %f %f %f %f %f %f %f %f",
      &groupID,
      &element->type,
      &element->position.X,
      &element->position.Y,
      &element->position.Z,
      &element->rotation.X,
      &element->rotation.Y,
      &element->rotation.Z,
      &element->scale.X,
      &element->scale.Y);

    // X and Z are the same so only X is stored in the file
    element->scale.Z = element->scale.X;

    tempmat.setRotationDegrees(element->rotation);
    tempmat.rotateVect(normal, irr::core::vector3df(0.f, 1.f, 0.f));



    gNode->getPatchByID(groupID)->elements.push_back(element);
  }

}
#endif

void CObjectManager::loadGrassFromFile(const irr::c8* gFile)
{
  std::ifstream ifs(gFile);
  std::string temp;

  irr::u32 numberOfGroups=0;

  irr::u16 grass_skipped = 0;
  irr::u16 grass_density = Core->getConfiguration()->getVideo()->grassDensity;
  bool geomShaderEnabled = (Core->getConfiguration()->getVideo()->geomShaderGrass > 0);
  irr::video::ITexture* shadowMap = ((scene::IMeshSceneNode*)getSceneNodeFromName(getNodeFullName("Terrain").c_str()))->getMesh()->getMeshBuffer(0)->getMaterial().getTexture(0);

  disableLowDetail = false;
  disableMediumDetail = false;

  // Get number of groups
  getline(ifs, temp);
  sscanf(temp.c_str(), "%d", &numberOfGroups);

  for(irr::u32 gId=0; gId < numberOfGroups; ++gId)
  {
    irr::u32 groupID = 0, numberOfNeighbours = 0;
    irr::core::vector3df pos;
    irr::u16 neigh[10];

    getline(ifs, temp);

    sscanf(temp.c_str(), "%d %d %f %f %d %d %d %d %d %d %d %d %d %d",
      &groupID,
      &numberOfNeighbours,
      &pos.X,
      &pos.Z,
      &neigh[0], &neigh[1], &neigh[2], &neigh[3], &neigh[4],
      &neigh[5], &neigh[6], &neigh[7], &neigh[8], &neigh[9]);

    SFoilageGroup *f_group = new SFoilageGroup();

    f_group->ID = f_groups.size();

#ifndef GRASS_2
    // Create new meshes that will hold all our meshbuffers in the end
    f_group->meshHigh = new irr::scene::CBatchingMesh();
    f_group->meshMed = new irr::scene::CBatchingMesh();
    f_group->meshLow = new irr::scene::CBatchingMesh();
#endif

    // Center of each patch
    f_group->position = pos;

    for(irr::u32 n=0; n < numberOfNeighbours; ++n) {
      f_group->neighbours.push_back(neigh[n]);
    }

    // Add it to list
    f_groups.push_back(f_group);
  }


  grass_density = fmin(grass_density, 16);
  irr::u8 meshAdded = 0;

  /*if(grass_density <= 11)
    disableLowDetail = true;

  if(grass_density <= 6)
    disableMediumDetail = true;*/

  //grass_density /= 2;

  irr::scene::IMesh *meshes[GRASS_MESH_BUFFER_COUNT];

  meshes[0] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/grass1.ms3d");
  meshes[1] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/grass2.ms3d");
  meshes[2] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/grass3.ms3d");
  meshes[3] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/grass5.ms3d");
  meshes[4] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/grass6.ms3d");

  meshes[5] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/weeds/fern.ms3d");
  meshes[6] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/weeds/fern2.ms3d");
  meshes[7] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/weeds/weed1.ms3d");
  meshes[8] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/weeds/weed2.ms3d");
  meshes[9] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/weeds/weed3.ms3d");

  irr::scene::IMeshManipulator *meshManipulator =
      Core->getRenderer()->getSceneManager()->getMeshManipulator();

  // Meshes need to be converted to tangent mesh for grass geom shader
  if(geomShaderEnabled || Core->getConfiguration()->getVideo()->isShaderGrass)
  {
    irr::scene::IMesh *tmpmsh;
    for(irr::u16 mIdx=0; mIdx < GRASS_MESH_BUFFER_COUNT; ++mIdx)
    {
        tmpmsh = meshes[mIdx];
        meshes[mIdx] = meshManipulator->createMeshWithTangents(meshes[mIdx], false, false, false);
        Core->getRenderer()->getSceneManager()->getMeshCache()->removeMesh(tmpmsh);
        if (mIdx<5) {
            setGrassNormals(meshes[mIdx]);
        }
    }
  }

  irr::f32 shadowIntensity = parameters.lightValues.X;
  irr::f32 sunIntensity = parameters.lightValues.Y;

  while(getline(ifs, temp))
  {
    grass_skipped += 1;

    if(grass_skipped <= (16-grass_density))
      continue;

    grass_skipped = 0;



    irr::u32 groupID = 0;
    irr::u8 type = 0;
    irr::core::vector3df pos, rot, scale;
    irr::core::matrix4 tempmat;

    sscanf(temp.c_str(), "%d %d %f %f %f %f %f %f %f %f",
      &groupID,
      &type,
      &pos.X,
      &pos.Y,
      &pos.Z,
      &rot.X,
      &rot.Y,
      &rot.Z,
      &scale.X,
      &scale.Y);

    scale.Z = scale.X;

#ifndef GRASS_2

    bool addToHighDensityMesh = true;
    bool addToMediumDensityMesh = false;
    bool addToLowDensityMesh = false;

    /*if(meshAdded < 6) addToHighDensityMesh = true;
    else if(meshAdded >= 6 && meshAdded < 9) addToMediumDensityMesh = true;
    else if(meshAdded >= 9 && meshAdded < 11) addToLowDensityMesh = true;*/

    /*if(grass_density <= 4) {
      if(addToMediumDensityMesh) {
        addToMediumDensityMesh = false;
        addToHighDensityMesh = true;
        if(type==1)
          type=0;
      }
      if(addToLowDensityMesh) {
        addToLowDensityMesh = false;
        addToHighDensityMesh = true;
        if(type==2)
          type=1;
      }
    }
    else if(grass_density <= 7) {
      if(addToLowDensityMesh) {
        addToLowDensityMesh = false;
        addToMediumDensityMesh = true;
        if(type==2)
          type=1;
      }
    }*/

    if(meshAdded > 3) addToMediumDensityMesh = true;
    if(meshAdded > 5) addToLowDensityMesh = true;

    meshAdded ++;

    if(meshAdded == 7) meshAdded = 0;

    // Lower the mesh detail
    /*if(addToLowDensityMesh && type == 1)
      type = 2;
    if(addToLowDensityMesh && type == 0)
      type = 1;*/

    irr::scene::IMesh *g_mesh = meshes[type];

    /*if(Core->CommandLineParameters.hasParam("-disable_vbo") == false)
    {
      g_mesh->setHardwareMappingHint(scene::EHM_STATIC);
      g_mesh->setDirty();
    }*/

    /*if(geomShaderEnabled || Core->getConfiguration()->getVideo()->isShaderGrass)
    {
      g_mesh = meshManipulator->createMeshWithTangents(g_mesh,false,false,false);
      //printf("REFERENCES: %i",meshes[mIdx]->getReferenceCount());

      //Store normal and position in tangent
      setTangentsToPos(g_mesh, normal, pos, g_mesh->getBoundingBox().getExtent().getLength());
    }*/

    irr::core::vector3df tPos = pos;
    // DO NOT CHANGE
    irr::f32 tBoundingSphere = (g_mesh->getBoundingBox().MaxEdge*scale-g_mesh->getBoundingBox().MinEdge*scale).getLength()/2.f;

    for(irr::u32 mbIdx=0; mbIdx < g_mesh->getMeshBufferCount(); ++mbIdx)
    {
      irr::scene::IMeshBuffer *currentMeshBuffer = g_mesh->getMeshBuffer(mbIdx);

      SColor shadow_color; // = g_mesh->getMeshBuffer(m)->getMaterial().EmissiveColor;
      irr::f32 shade_color = 0; //(xml->getAttributeValueAsFloat(9)/255) * (80*shadowIntensity);

      if (!Core->getConfiguration()->getVideo()->isShaderGrass) {
        irr::u32 shade_color = u32(255*(sunIntensity*1.5));
        currentMeshBuffer->getMaterial().EmissiveColor = SColor(255,shade_color,shade_color,shade_color);
      }
      currentMeshBuffer->getMaterial().AmbientColor = SColor(255, 255, 255, 255); //28,32,17 = 27,30,19
      currentMeshBuffer->getMaterial().DiffuseColor = SColor(255, 142, 137, 139);
      currentMeshBuffer->getMaterial().SpecularColor = SColor(255, 0,0,0);
      currentMeshBuffer->getMaterial().Lighting = false;

      currentMeshBuffer->getMaterial().setFlag(video::EMF_BACK_FACE_CULLING, false);
      currentMeshBuffer->getMaterial().setFlag(video::EMF_ZWRITE_ENABLE, true);
      currentMeshBuffer->getMaterial().setFlag(video::EMF_LIGHTING, false);
      currentMeshBuffer->getMaterial().setFlag(video::EMF_FOG_ENABLE, parameters.fogEnabled);
      currentMeshBuffer->getMaterial().setFlag(video::EMF_TRILINEAR_FILTER, Core->getConfiguration()->getVideo()->isTrilinearFilter);
      currentMeshBuffer->getMaterial().setFlag(video::EMF_ANISOTROPIC_FILTER, Core->getConfiguration()->getVideo()->isAnistropicFilter);
      currentMeshBuffer->getMaterial().setTexture(3,shadowMap);

      // Set tangent here
      if(geomShaderEnabled || Core->getConfiguration()->getVideo()->isShaderGrass) {
        if(currentMeshBuffer->getVertexType() == irr::video::EVT_TANGENTS)
        {
            for(irr::u32 i=0; i<currentMeshBuffer->getVertexCount(); i++) {
                (((irr::video::S3DVertexTangents*)currentMeshBuffer->getVertices())+i)->Tangent.X=tBoundingSphere;
                (((irr::video::S3DVertexTangents*)currentMeshBuffer->getVertices())+i)->Binormal=tPos;

                if(type < 5)
                  (((irr::video::S3DVertexTangents*)currentMeshBuffer->getVertices())+i)->Normal = irr::core::vector3df(0.0, 1.0, 0.0);
            }
        }
      }
    }

    if(addToHighDensityMesh)
      f_groups[groupID]->meshHigh->addMesh(
        g_mesh,
        pos,
        rot,
        scale);

    if(addToMediumDensityMesh && !disableMediumDetail)
      f_groups[groupID]->meshMed->addMesh(
        g_mesh,
        pos,
        rot,
        scale);

    if(addToLowDensityMesh && !disableLowDetail)
      f_groups[groupID]->meshLow->addMesh(
        g_mesh,
        pos,
        rot,
        scale);

#endif

    //Core->getRenderer()->getSceneManager()->getMeshCache()->removeMesh(g_mesh);
    //g_mesh->drop();
  }

  for(irr::u32 gId=0; gId < f_groups.size(); ++gId)
  {

#ifndef GRASS_2
    f_groups[gId]->meshLow->finalize();
    f_groups[gId]->meshMed->finalize();
    f_groups[gId]->meshHigh->finalize();

    /*if(Core->commandLineParameters.hasParam("-disable_vbo") == false)
    {
      f_groups[gId]->meshHigh->setHardwareMappingHint(scene::EHM_STATIC, EBT_VERTEX_AND_INDEX);
      f_groups[gId]->meshHigh->setDirty(EBT_VERTEX_AND_INDEX);

      f_groups[gId]->meshMed->setHardwareMappingHint(scene::EHM_STATIC, EBT_VERTEX_AND_INDEX);
      f_groups[gId]->meshMed->setDirty(EBT_VERTEX_AND_INDEX);

      f_groups[gId]->meshLow->setHardwareMappingHint(scene::EHM_STATIC, EBT_VERTEX_AND_INDEX);
      f_groups[gId]->meshLow->setDirty(EBT_VERTEX_AND_INDEX);
    }*/
#else

#endif
  }

  ifs.close();


  //while(getline(ifs, temp))
  //{



    //sscanf(temp, "%s %*s %d",str,&i);
  //}

/*  irr::io::IXMLReader *xml;

  xml = Core->getRenderer()->getDevice()->getFileSystem()->createXMLReader(gFile);
>>>>>>> .r120

  irr::u16 grass_skipped = 0;
  irr::u16 grass_density = Core->GetConfiguration()->GetVideo()->GrassDensity;
  bool geomShaderEnabled = (Core->GetConfiguration()->GetVideo()->GeomShaderGrass > 0);

  // It wont go higher than 10 .. it doesnt work like that. Stop changing it to 20, devsh
  grass_density = fmin(grass_density, 10);

  irr::scene::IMesh *meshes[GRASS_MESH_BUFFER_COUNT];

#ifndef EXPERIMENTAL_GRASS
<<<<<<< .mine
  meshes[0] = Core->GetRenderer()->GetSceneManager()->getMesh("data/foilage/grass1.ms3d");
  meshes[1] = Core->GetRenderer()->GetSceneManager()->getMesh("data/foilage/grass2.ms3d");
  meshes[2] = Core->GetRenderer()->GetSceneManager()->getMesh("data/foilage/grass3.ms3d");
  meshes[3] = Core->GetRenderer()->GetSceneManager()->getMesh("data/foilage/grass5.ms3d");
  meshes[4] = Core->GetRenderer()->GetSceneManager()->getMesh("data/foilage/grass6.ms3d");
  // set right normals for grass illumination
  for (irr::u32 i=0; i<5; i++) {
      for (irr::u32 j=0; j<meshes[i]->getMeshBufferCount(); j++) {
          for (irr::u32 k=0; k<meshes[i]->getMeshBuffer(j)->getVertexCount(); k++) {
              ((irr::video::S3DVertex*)meshes[i]->getMeshBuffer(j)->getVertices())[k].Normal = irr::core::vector3df(0.f,1.f,0.f);
          }
      }
  }
=======
  meshes[0] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/grass1.ms3d");
  meshes[1] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/grass2.ms3d");
  meshes[2] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/grass3.ms3d");
  meshes[3] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/grass5.ms3d");
  meshes[4] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/grass6.ms3d");
>>>>>>> .r131

  meshes[5] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/weeds/fern.ms3d");
  meshes[6] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/weeds/fern2.ms3d");
  meshes[7] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/weeds/weed1.ms3d");
  meshes[8] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/weeds/weed2.ms3d");
  meshes[9] = Core->getRenderer()->getSceneManager()->getMesh("data/foilage/weeds/weed3.ms3d");
#endif

  irr::scene::IMeshManipulator *meshManipulator =
      Core->getRenderer()->getSceneManager()->getMeshManipulator();

#ifndef EXPERIMENTAL_GRASS
  // Meshes need to be converted to tangent mesh for grass geom shader
  if(geomShaderEnabled)
  {
    irr::scene::IMesh *tmpmsh;
    for(irr::u16 mIdx=0; mIdx < GRASS_MESH_BUFFER_COUNT; ++mIdx)
    {
        tmpmsh = meshes[mIdx];
        meshes[mIdx] = meshManipulator->createMeshWithTangents(meshes[mIdx], false, false, false);
        Core->getRenderer()->getSceneManager()->getMeshCache()->removeMesh(tmpmsh);
    }
  }
#endif

  irr::f32 shadowIntensity = parameters.lightValues.X;
  irr::f32 sunIntensity = parameters.lightValues.Y;

  //
  // Get the last element which is the size paramters

  while(xml && xml->read())
  {
    irr::core::stringc nodeName = irr::core::stringc(xml->getNodeName());

    if(xml->getNodeType() == irr::io::EXN_ELEMENT && nodeName == "size")
    {
      irr::f32 xmin = xml->getAttributeValueAsFloat(0);
      irr::f32 xmax = xml->getAttributeValueAsFloat(1);
      irr::f32 zmin = xml->getAttributeValueAsFloat(2);
      irr::f32 zmax = xml->getAttributeValueAsFloat(3);

      for(irr::f32 fg_x = xmin; fg_x <= xmax; fg_x += DISTANCE_BETWEEN_FOILAGE_GROUPS) {
        for(irr::f32 fg_z = zmin; fg_z <= zmax; fg_z += DISTANCE_BETWEEN_FOILAGE_GROUPS)
        {
          SFoilageGroup *f_group = new SFoilageGroup();

          f_group->ID = f_groups.size();

#ifndef EXPERIMENTAL_GRASS
          // Create a new mesh that will hold all our meshbuffers int the end
          f_group->meshHigh = new irr::scene::CBatchingMesh();
          f_group->meshMed = new irr::scene::CBatchingMesh();
          f_group->meshLow = new irr::scene::CBatchingMesh();

#else
          f_group->neighbours.set_used(0);
          f_group->elementsH.set_used(0);
          f_group->elementsM.set_used(0);
          f_group->elementsL.set_used(0);
#endif

          // Center of each patch
          f_group->position = irr::core::vector3df(
            fg_x + DISTANCE_BETWEEN_FOILAGE_GROUPS/2,
            0.f,
            fg_z + DISTANCE_BETWEEN_FOILAGE_GROUPS/2);

          // Add it to list
          f_groups.push_back(f_group);
        }
      }
    }
  }
  Core->GetRenderer()->GetDevice()->getOSOperator()->getSystemMemory(&total, &avail);
  printf("AVAILABLE SYSTEM MEMORY: %u \n",avail);
  xml->drop();


  // Re-open the xml reader
  xml = Core->getRenderer()->getDevice()->getFileSystem()->createXMLReader(gFile);

  irr::u8 meshAdded = 0;

  while(xml && xml->read()) {
    grass_skipped += 1;

    if(grass_skipped <= (10-grass_density))
      continue;

    grass_skipped = 0;

    irr::core::stringc nodeName = irr::core::stringc(xml->getNodeName());

    if(nodeName != "size")
    switch(xml->getNodeType())
    {
      case irr::io::EXN_ELEMENT:
      {
        irr::core::vector3df pos, rot, scale;

        pos.set(xml->getAttributeValueAsFloat(1), xml->getAttributeValueAsFloat(2), xml->getAttributeValueAsFloat(3));
        //pos -= vector3df(0.f, 0.25f, 0.f);
        rot.set(xml->getAttributeValueAsFloat(4), xml->getAttributeValueAsFloat(5), xml->getAttributeValueAsFloat(6));
        scale.set(xml->getAttributeValueAsFloat(7), xml->getAttributeValueAsFloat(8), xml->getAttributeValueAsFloat(7));

        irr::u16 grassType = xml->getAttributeValueAsInt(0);

        irr::core::matrix4 tempmat;
        irr::core::vector3df normal;
        tempmat.setRotationDegrees(rot);
        tempmat.rotateVect(normal,irr::core::vector3df(0.f,1.f,0.f));

        irr::s32 f_group_index = findNearestFoilageGroup(pos);

        bool addToHighDensityMesh = false;
        bool addToMediumDensityMesh = false;
        bool addToLowDensityMesh = false;

        if(meshAdded < 5) addToHighDensityMesh = true;
        else if(meshAdded >= 5 && meshAdded < 7) addToMediumDensityMesh = true;
        else if(meshAdded == 7) addToLowDensityMesh = true;

        if(grass_density <= 4) {
          if(addToMediumDensityMesh) {
            addToMediumDensityMesh = false;
            addToHighDensityMesh = true;
          }
          if(addToLowDensityMesh) {
            addToLowDensityMesh = false;
            addToHighDensityMesh = true;
          }
        }
        else if(grass_density <= 7) {
          if(addToLowDensityMesh) {
            addToLowDensityMesh = false;
            addToMediumDensityMesh = true;
          }
        }

        meshAdded ++;

        if(meshAdded > 7) meshAdded = 0;

        // Lower the mesh detail
        if(addToLowDensityMesh && grassType == 1)
          grassType = 2;
        if(addToLowDensityMesh && grassType == 0)
          grassType = 1;

        irr::scene::IMesh *g_mesh = meshes[grassType];

        if(Core->CommandLineParameters.hasParam("-disable_vbo") == false)
        {
          g_mesh->setHardwareMappingHint(scene::EHM_STATIC);
          g_mesh->setDirty();
        }

#ifndef EXPERIMENTAL_GRASS
        //
        // Transform the new mesh to be added

        matrix4 MeshBufferTransformation;
        MeshBufferTransformation.setRotationDegrees(rot);
        MeshBufferTransformation.setTranslation(pos);
        MeshBufferTransformation.setScale(scale);

        for(irr::u32 mbIdx=0; mbIdx < g_mesh->getMeshBufferCount(); ++mbIdx)
        {
          irr::scene::IMeshBuffer *currentMeshBuffer = g_mesh->getMeshBuffer(mbIdx);

          SColor shadow_color; // = g_mesh->getMeshBuffer(m)->getMaterial().EmissiveColor;
          irr::f32 shade_color = (xml->getAttributeValueAsFloat(9)/255) * (80*shadowIntensity);

          shadow_color.setAlpha(255);
          shadow_color.setRed( u32(255*(sunIntensity*1.5)) - irr::u32(shade_color));
          shadow_color.setGreen( u32(255*(sunIntensity*1.5)) - irr::u32(shade_color));
          shadow_color.setBlue( u32(255*(sunIntensity*1.5)) - irr::u32(shade_color));

          currentMeshBuffer->getMaterial().EmissiveColor = shadow_color;
          currentMeshBuffer->getMaterial().AmbientColor = SColor(255, 0,0,0);
          currentMeshBuffer->getMaterial().DiffuseColor = SColor(255, 200,200,200);
          currentMeshBuffer->getMaterial().SpecularColor = SColor(255, 0,0,0);
          currentMeshBuffer->getMaterial().Lighting = false;

          currentMeshBuffer->getMaterial().setFlag(video::EMF_BACK_FACE_CULLING, false);
          currentMeshBuffer->getMaterial().setFlag(video::EMF_ZWRITE_ENABLE, true);
          currentMeshBuffer->getMaterial().setFlag(video::EMF_LIGHTING, false);
          currentMeshBuffer->getMaterial().setFlag(video::EMF_FOG_ENABLE, parameters.fogEnabled);
          currentMeshBuffer->getMaterial().MaterialTypeParam = 0.30f;
        }

        if(geomShaderEnabled)
<<<<<<< .mine
        {
            //Store normal and position in tangent
            setTangentsToPos(tmpMesh, normal, pos,tmpMesh->getBoundingBox().getExtent().getLength()/2.f);
        }
=======
        {
          g_mesh = meshManipulator->createMeshCopy(g_mesh);

          //Store normal and position in tangent
          setTangentsToPos(g_mesh, normal, pos);
        }
>>>>>>> .r120

        if(addToHighDensityMesh)
          f_groups[f_group_index]->meshHigh->addMesh(
            g_mesh,
            pos,
            rot,
            scale);
        else if(addToMediumDensityMesh)
          f_groups[f_group_index]->meshMed->addMesh(
            g_mesh,
            pos,
            rot,
            scale);
        else if(addToLowDensityMesh)
          f_groups[f_group_index]->meshLow->addMesh(
            g_mesh,
            pos,
            rot,
            scale);


        if(geomShaderEnabled)
        {
          Core->getRenderer()->getSceneManager()->getMeshCache()->removeMesh(g_mesh);
          g_mesh->drop();
        }


        // Transform each mesh buffer of the new mesh
        meshManipulator->transformMesh(tmpMesh, MeshBufferTransformation);
        if (Core->GetConfiguration()->GetVideo()->GeomShaderGrass) {
            //Store normal and position in tangent scaled by the size of the bounding sphere
            setTangentsToPos(tmpMesh,grassType>4 ? normal:irr::core::vector3df(0.0,1.0,0.0),pos,tmpMesh->getBoundingBox().getExtent().getLength()/2.f);
        }

        // Add that meshbuffer to the target mesh-buffer
        for(irr::u32 m=0; m < tmpMesh->getMeshBufferCount(); ++m)
        {
          if(geomShaderEnabled)
          {
            f_groups[f_group_index]->meshBufferT[grassType]->append(
              tmpMesh->getMeshBuffer(m)->getVertices(),
              tmpMesh->getMeshBuffer(m)->getVertexCount(),
              tmpMesh->getMeshBuffer(m)->getIndices(),
              tmpMesh->getMeshBuffer(m)->getIndexCount());
          }
          else
          {
            f_groups[f_group_index]->meshBuffer[grassType]->append(
              tmpMesh->getMeshBuffer(m)->getVertices(),
              tmpMesh->getMeshBuffer(m)->getVertexCount(),
              tmpMesh->getMeshBuffer(m)->getIndices(),
              tmpMesh->getMeshBuffer(m)->getIndexCount());
          }
        }



#else
        // High detail
        f_groups[f_group_index]->meshHigh->addMesh(
          meshes[0],
          pos,
          rot,
          scale);

        // Medium detail
        f_groups[f_group_index]->meshMed->addMesh(
          meshes[1],
          pos,
          rot,
          scale);

        // Low detail
        f_groups[f_group_index]->meshLow->addMesh(
          meshes[2],
          pos,
          rot,
          scale);

        bool includeInHigh = true;
        bool includeInMed = true;
        bool includeInLow = true;

        for(irr::u32 eId=0; eId < f_groups[f_group_index]->elementsH.size(); ++eId) {
          if(f_groups[f_group_index]->elementsH[eId]->position.getDistanceFrom(pos) < 0.50f) {
            includeInHigh = false;
            break;
          }
        }

        for(irr::u32 eId=0; eId < f_groups[f_group_index]->elementsM.size(); ++eId) {
          if(f_groups[f_group_index]->elementsM[eId]->position.getDistanceFrom(pos) < 4.00f) {
            includeInMed = false;
            break;
          }
        }

        for(irr::u32 eId=0; eId < f_groups[f_group_index]->elementsL.size(); ++eId) {
          if(f_groups[f_group_index]->elementsL[eId]->position.getDistanceFrom(pos) < 8.30f) {
            includeInLow = false;
            break;
          }
        }

        if(includeInHigh || includeInMed || includeInLow)
        {
          SFoilageGroupElement *e = new SFoilageGroupElement();
          e->position = pos;
          e->rotation = rot;
          e->scale = scale * irr::core::vector3df(1.15f, 1.41f, 1.15f);

          if(includeInHigh)
            f_groups[f_group_index]->elementsH.push_back(e);

          if(includeInMed)
            f_groups[f_group_index]->elementsM.push_back(e);

          if(includeInLow)
            f_groups[f_group_index]->elementsL.push_back(e);
        }

#endif

        // TO BE DELETED!
        for(irr::u32 m=0; m < g_mesh->getMeshBufferCount(); ++m)
        {
          SColor shadow_color; // = g_mesh->getMeshBuffer(m)->getMaterial().EmissiveColor;
          irr::f32 shade_color = (xml->getAttributeValueAsFloat(9)/255) * (80*shadowIntensity);

          irr::u32 random_color_change = Core->getMath()->getRandomInt(0,10);

          shadow_color.setAlpha(255);
          shadow_color.setRed( u32(255*(sunIntensity*1.5)) - irr::u32(shade_color) - random_color_change);
          shadow_color.setGreen( u32(255*(sunIntensity*1.5)) - irr::u32(shade_color) - random_color_change);
          shadow_color.setBlue( u32(255*(sunIntensity*1.5)) - irr::u32(shade_color) - random_color_change);

          g_mesh->getMeshBuffer(m)->getMaterial().EmissiveColor = shadow_color;
          g_mesh->getMeshBuffer(m)->getMaterial().AmbientColor = SColor(255, 0,0,0);
          g_mesh->getMeshBuffer(m)->getMaterial().DiffuseColor = SColor(255, 200,200,200);
          g_mesh->getMeshBuffer(m)->getMaterial().SpecularColor = SColor(255, 0,0,0);
          g_mesh->getMeshBuffer(m)->getMaterial().Lighting = false;

          g_mesh->getMeshBuffer(m)->getMaterial().setFlag(video::EMF_BACK_FACE_CULLING, false);
          g_mesh->getMeshBuffer(m)->getMaterial().setFlag(video::EMF_ZWRITE_ENABLE, true);
          g_mesh->getMeshBuffer(m)->getMaterial().setFlag(video::EMF_LIGHTING, false);
          g_mesh->getMeshBuffer(m)->getMaterial().setFlag(video::EMF_FOG_ENABLE, parameters.fogEnabled);
          g_mesh->getMeshBuffer(m)->getMaterial().MaterialTypeParam = 0.38f;
          memory_fpt += g_mesh->getMeshBuffer(m)->getVertexCount()*18+g_mesh->getMeshBuffer(m)->getIndexCount();
        }

      }
      break;
    }

  }

#ifndef EXPERIMENTAL_GRASS

  // For each patch ..
  for(irr::u32 gPtchIdx=0; gPtchIdx < f_groups.size(); ++gPtchIdx)
  {
    irr::u32 mbCount = 0;

    if(geomShaderEnabled)
      mbCount = f_groups[gPtchIdx]->meshBufferT.size();
    else
      mbCount = f_groups[gPtchIdx]->meshBuffer.size();

    for(irr::u32 mbIdx=0; mbIdx < mbCount; ++mbIdx)
    {
      irr::scene::IMeshBuffer *currentMeshBuffer;

      if(geomShaderEnabled)
        currentMeshBuffer = f_groups[gPtchIdx]->meshBufferT[mbIdx];
      else
        currentMeshBuffer = f_groups[gPtchIdx]->meshBuffer[mbIdx];

      if(currentMeshBuffer->getVertexCount() > 32000)
        printf("MESH LOSS ALERT!\n");


      currentMeshBuffer->getMaterial() = meshes[mbIdx]->getMeshBuffer(0)->getMaterial();

      if (!Core->GetConfiguration()->GetVideo()->IsShaderGrass) {
        irr::u32 shade_color = u32(255*(sunIntensity*1.5)) - irr::u32((xml->getAttributeValueAsFloat(9)/255) * (80*shadowIntensity));
        currentMeshBuffer->getMaterial().EmissiveColor = SColor(255,shade_color,shade_color,shade_color);
      }
      currentMeshBuffer->getMaterial().AmbientColor = SColor(255, 140, 119, 165); //28,32,17 = 27,30,19
      currentMeshBuffer->getMaterial().DiffuseColor = SColor(255, 0,0,0);
      currentMeshBuffer->getMaterial().SpecularColor = SColor(255, 0,0,0);
      currentMeshBuffer->getMaterial().Lighting = false;

      currentMeshBuffer->getMaterial().setFlag(video::EMF_BACK_FACE_CULLING, false);
      currentMeshBuffer->getMaterial().setFlag(video::EMF_ZWRITE_ENABLE, true);
      currentMeshBuffer->getMaterial().setFlag(video::EMF_LIGHTING, false);
      currentMeshBuffer->getMaterial().setFlag(video::EMF_FOG_ENABLE, parameters.fogEnabled);
      currentMeshBuffer->getMaterial().MaterialTypeParam = 0.30f;

      if(Core->CommandLineParameters.hasParam("-disable_vbo") == false)
      {
          currentMeshBuffer->setHardwareMappingHint(scene::EHM_STATIC);
          currentMeshBuffer->setDirty();
      }

      currentMeshBuffer->recalculateBoundingBox();

      if(geomShaderEnabled)
        f_groups[gPtchIdx]->mesh_gen_internal->addMeshBuffer(f_groups[gPtchIdx]->meshBufferT[mbIdx]);
      else
        f_groups[gPtchIdx]->mesh_gen_internal->addMeshBuffer(f_groups[gPtchIdx]->meshBuffer[mbIdx]);

    }
  }
#else


#endif

  findFoilageGroupsNeighbours();*/

#ifndef EXPERIMENTAL_GRASS
  /*for(irr::u16 gi=0; gi < f_groups.size(); ++gi) {
    for(irr::u16 mb=0; mb < f_groups[gi]->meshBuffer.size(); ++mb)
    {
      delete f_groups[gi]->meshBuffer[mb];
    }

    for(irr::u16 mb=0; mb < f_groups[gi]->meshBufferT.size(); ++mb)
    {
      delete f_groups[gi]->meshBufferT[mb];
    }

    f_groups[gi]->meshBufferT.clear();
    f_groups[gi]->meshBuffer.clear();

    //Core->getRenderer()->getSceneManager()->getMeshCache()->removeMesh(f_groups[gi]->mesh_gen_internal);
  }*/
#endif


#ifndef GRASS_2

  GrassMesh = new irr::scene::CBatchingMesh();

  foilageNode = Core->getRenderer()->getSceneManager()->addMeshSceneNode(GrassMesh);
  foilageNode->setRotation( vector3df(0,0,0) );
  foilageNode->setName("FoilageNode");
  foilageNode->setAutomaticCulling(scene::EAC_OFF);

  if(geomShaderEnabled || Core->getConfiguration()->getVideo()->isShaderGrass)
  {
    for(irr::u16 i=0; i < 9; ++i) {
      Core->getRenderer()->getSceneManager()->getMeshCache()->removeMesh(meshes[i]);
      meshes[i]->drop();
    }
  }

  grassMeshes.push_back(GrassMesh);
#endif
}

void CObjectManager::generateGrassFile(SGrassObject gObj)
{
  scene::IMeshSceneNode *node =
    (scene::IMeshSceneNode*)getSceneNodeFromName(
      getNodeFullName(gObj.objectName.c_str()).c_str());

  if(node == NULL) return;

#ifdef PHYSICS_NEWTON

  // TODO : Check if image file exists before loading

  irr::core::stringc sourceFile = "data/levels/";
  sourceFile += parameters.levelName;
  sourceFile += "/";
  sourceFile += gObj.generationSourceFileName;

  irr::video::IImage* mapimage =
    Core->getRenderer()->getVideoDriver()->createImageFromFile(
      sourceFile.c_str());

  irr::u32 w = mapimage->getDimension().Width;

  irr::core::stringc grassFile = "data/levels/";
  grassFile += parameters.levelName;
  grassFile += "/";
  grassFile += gObj.objectName;
  grassFile += ".grs";

  irr::io::IWriteFile *writer =
    Core->getRenderer()->getDevice()->getFileSystem()->createAndWriteFile(
      grassFile.c_str());

  irr::core::stringc tmp_s = "<?xml version=\"1.0\"?>";
  writer->write(tmp_s.c_str(), (u32)tmp_s.size());

  irr::u16 *indices;
  void* vrt;
  S3DVertex* vertices;

  // TODO : All meshbuffers, not only 0

  irr::scene::IMeshBuffer *mb = ((IMeshSceneNode*)node)->getMesh()->getMeshBuffer(0);

  // Get indices and vertices of mesh buffer
  indices = mb->getIndices();
  vrt = mb->getVertices();
  vertices = (S3DVertex*)vrt;

  irr::core::array<triangle3df> tri1, tri2;

  tri1.set_used(0);
  tri2.set_used(0);

  irr::u16 vert[9999][3];

  for(irr::u32 t=0; t < mb->getIndexCount(); t+=3)
  {
    vector2df in1 = vertices[indices[t]].TCoords;
    vector2df in2 = vertices[indices[t+1]].TCoords;
    vector2df in3 = vertices[indices[t+2]].TCoords;

    triangle3df tri_1 = triangle3df(
     vector3df(in1.X, 0, in1.Y),
     vector3df(in2.X, 0, in2.Y),
     vector3df(in3.X, 0, in3.Y));

    triangle3df tri_2 = triangle3df(
      vertices[indices[t]].Pos,
      vertices[indices[t+1]].Pos,
      vertices[indices[t+2]].Pos);

    vert[tri2.size()][0] = indices[t];
    vert[tri2.size()][1] = indices[t+1];
    vert[tri2.size()][2] = indices[t+2];

    tri1.push_back(tri_1);
    tri2.push_back(tri_2);
  }



  irr::u32 grass_density = 0;
  irr::u32 grassCount = 0;
  irr::f32 xmin=0.f,zmin=0.f,xmax=0.f,zmax=0.f;
  bool first_g_element = true;

  for(irr::u32 i = 0; i < w; ++i) {
  for(irr::u32 j = 0; j < w; ++j)
  {
    irr::f32 TCoordX = irr::f32(i) / irr::f32(w-1);
    irr::f32 TCoordY = irr::f32(j) / irr::f32(w-1);

    irr::f32 lum = mapimage->getPixel(i,j).getLuminance();
    irr::f32 alpha = mapimage->getPixel(i,j).getAlpha();

    irr::core::line3df texCoordLine =
      line3df(vector3df(TCoordX, 1, TCoordY), vector3df(TCoordX, -1, TCoordY));

    if(lum <= 8) {

    //if(Core->getMath()->getRandomInt(0, irr::u32((lum*2))) == 0)
    {
      vector3df pos;
      bool triangleFound = false;
      irr::u16 coveringTriangleIndex = 0;
      irr::f32 weight1=0.f, weight2=0.f, weight3=0.f;

      // Find on which triangle is the texture coordinate
      for(irr::u32 t=0; t < tri1.size(); ++t)
      {
        vector3df outpos;

        if(tri1[t].getIntersectionWithLimitedLine(texCoordLine, outpos))
        {
          vector2df in1 = vector2df(tri1[t].pointA.X, tri1[t].pointA.Z);
          vector2df in2 = vector2df(tri1[t].pointB.X, tri1[t].pointB.Z);
          vector2df in3 = vector2df(tri1[t].pointC.X, tri1[t].pointC.Z);

          irr::f32 mat_1_1 = in1.X - in3.X;
          irr::f32 mat_1_2 = in2.X - in3.X;
          irr::f32 mat_2_1 = in1.Y - in3.Y;
          irr::f32 mat_2_2 = in2.Y - in3.Y;

          irr::f32 detT = (mat_1_1 * mat_2_2) - (mat_2_1 * mat_1_2);

          irr::f32 l1 = ((mat_2_2*(TCoordX-in3.X)) - (mat_1_2*(TCoordY-in3.Y))) / detT;
          irr::f32 l2 = (-(mat_2_1*(TCoordX-in3.X)) + (mat_1_1*(TCoordY-in3.Y))) / detT;
          irr::f32 l3 = 1 - l1 - l2;

          pos = (tri2[t].pointA * l1) + (tri2[t].pointB * l2) + (tri2[t].pointC * l3);
          pos *= node->getScale();

          triangleFound = true;

          weight1 = l1;
          weight2 = l2;
          weight3 = l3;

          coveringTriangleIndex = t;

          //Core->getMath()->lerp();

          /*printf("Normals:\n\t%.4f %.4f %.4f\n\t%.4f %.4f %.4f\n\t%.4f %.4f %.4f\n",
            vertices[vert[t][0]].Normal.X,
            vertices[vert[t][0]].Normal.Y,
            vertices[vert[t][0]].Normal.Z,
            vertices[vert[t][1]].Normal.X,
            vertices[vert[t][1]].Normal.Y,
            vertices[vert[t][1]].Normal.Z,
            vertices[vert[t][2]].Normal.X,
            vertices[vert[t][2]].Normal.Y,
            vertices[vert[t][2]].Normal.Z
          );*/



          break;
        }
      }

      if(triangleFound == false) continue;

      if(Core->getMath()->getRandomInt(0,1) == 0)
        pos.X += (irr::f32(Core->getMath()->getRandomInt(0,4)) / 2.1f);
      else
        pos.X -= (irr::f32(Core->getMath()->getRandomInt(0,4)) / 2.f);

      if(Core->getMath()->getRandomInt(0,1) == 0)
        pos.Z += (irr::f32(Core->getMath()->getRandomInt(0,4)) / 2.3f);
      else
        pos.Z -= (irr::f32(Core->getMath()->getRandomInt(0,4)) / 2.f);

      pos.Y += 1000;

      vector3df offset;

      matrix4 oldMat;
      //oldMat.setRotationDegrees(vector3df(0, Core->getMath()->getRandomInt(0, 360), 0));

      /*offset = vector3df(3,0,3);
      //oldMat.rotateVect(offset);
      irr::core::line3df traceLine1 = irr::core::line3df(pos+offset, pos+offset-vector3df(0,2000,0));

      offset = vector3df(-3,0,3);
      //oldMat.rotateVect(offset);
      irr::core::line3df traceLine2 = irr::core::line3df(pos+offset, pos+offset-vector3df(0,2000,0));

      offset = vector3df(3,0,-3);
      //oldMat.rotateVect(offset);
      irr::core::line3df traceLine3 = irr::core::line3df(pos+offset, pos+offset-vector3df(0,2000,0));

      offset = vector3df(-3,0,-3);
      //oldMat.rotateVect(offset);
      irr::core::line3df traceLine4 = irr::core::line3df(pos+offset, pos+offset-vector3df(0,2000,0));

      physics::SRayCastParameters line1params;
      physics::SRayCastParameters line2params;
      physics::SRayCastParameters line3params;
      physics::SRayCastParameters line4params;

      line1params.line = traceLine1;
      line2params.line = traceLine2;
      line3params.line = traceLine3;
      line4params.line = traceLine4;

      physics::SRayCastResult out1 = Core->getPhysics()->getRayCollision(line1params);
      physics::SRayCastResult out2 = Core->getPhysics()->getRayCollision(line2params);
      physics::SRayCastResult out3 = Core->getPhysics()->getRayCollision(line3params);
      physics::SRayCastResult out4 = Core->getPhysics()->getRayCollision(line4params);*/


      irr::core::line3df traceLine1 = irr::core::line3df(pos+offset, pos+offset-vector3df(0,2000,0));

      physics::SRayCastParameters line1params;
      line1params.line = traceLine1;

      physics::SRayCastResult out1 = Core->getPhysics()->getRayCollision(line1params);

      if(out1.body == NULL) continue;

      if(out1.body->getNode() == node)
      {
        //pos = out1.position + out2.position + out3.position + out4.position;
        //pos /= 4;

        pos = out1.position;

        if(first_g_element)
        {
          xmin = xmax = pos.X;
          zmin = zmax = pos.Z;

          first_g_element = false;
        }
        else
        {
          if(pos.X > xmax) xmax = pos.X;
          else if(pos.X < xmin) xmin = pos.X;

          if(pos.Z > zmax) zmax = pos.Z;
          else if(pos.Z < zmin) zmin = pos.Z;
        }

        //irr::core::vector3df norm = out1.normal + out2.normal + out3.normal + out4.normal;
        //norm /= 4;

        //irr::core::vector3df norm = out1.normal;

        //norm.normalize();

        irr::core::vector3df norm =
          (vertices[vert[coveringTriangleIndex][0]].Normal * weight1) + (vertices[vert[coveringTriangleIndex][1]].Normal * weight2) + (vertices[vert[coveringTriangleIndex][2]].Normal * weight3);

        matrix4 newMat;


        //pos -= norm * 0.30f;

        //oldMat.rotateVect(norm);

        Core->getMath()->alignToUpVector(newMat, oldMat, norm, 1.0f);

        irr::core::vector3df rot = newMat.getRotationDegrees();

        char dataToWrite[90];

        irr::u32 grassMeshID = 0;
        irr::core::vector3df gscale;

        if(Core->getMath()->getRandomInt(0,70) <= 67) {
          if(Core->getMath()->getRandomInt(0,30) != 1)
            grassMeshID = Core->getMath()->getRandomInt(0,2);
          else
            grassMeshID = Core->getMath()->getRandomInt(3,4);

          gscale.set(0.62f, f32(Core->getMath()->getRandomInt(60,70)/100.f), 0.62f);
        }
        else {
          grassMeshID = 5 + Core->getMath()->getRandomInt(0,4);
          gscale.set(0.72f, f32(Core->getMath()->getRandomInt(60,74)/100.f), 0.72f);
        }

        sprintf(dataToWrite, "\n<g t=\"%d\" x=\"%.3f\" y=\"%.3f\" z=\"%.3f\" i=\"%.3f\" j=\"%.3f\" k=\"%.3f\" n=\"%.2f\" m=\"%.2f\" a=\"%.4f\"/>",
          grassMeshID,
          pos.X,
          pos.Y,
          pos.Z,
          rot.X,
          rot.Y,
          rot.Z,
          gscale.X,
          gscale.Y,
          alpha);

        writer->write(dataToWrite, (u32)strlen(dataToWrite));

        grassCount ++;
      }
      }
    } // SColor

  }
  }

  /*char dataToWrite[90];

  sprintf(dataToWrite, "\n<size xmin=\"%.4f\" xmax=\"%.4f\" zmin=\"%.4f\" zmax=\"%.4f\"/>",
    xmin,
    xmax,
    zmin,
    zmax);
  writer->write(dataToWrite, (u32)strlen(dataToWrite));*/

  mapimage->drop();
  writer->drop();


  printf("\n\tXMIN=%.4f\n\tXMAX=%.4f\n\tZMIN=%.4f\n\tZMAX=%.4f\n", xmin,xmax,zmin,zmax);

  generateGrassGroupFile(gObj, grassFile.c_str(), xmin, xmax, zmin, zmax);






  /*for(irr::f32 fg_x = xmin; fg_x <= xmax; fg_x += DISTANCE_BETWEEN_FOILAGE_GROUPS) {
    for(irr::f32 fg_z = zmin; fg_z <= zmax; fg_z += DISTANCE_BETWEEN_FOILAGE_GROUPS)
    {
      SFoilageGroup *f_group = new SFoilageGroup();

      f_group->mesh = new irr::scene::CBatchingMesh();
      f_group->position = irr::core::vector3df(
        fg_x + DISTANCE_BETWEEN_FOILAGE_GROUPS/2,
        0.f,
        fg_z + DISTANCE_BETWEEN_FOILAGE_GROUPS/2);

      f_groups.push_back(f_group);
    }
  }*/

  /*irr::io::IReadFile *reader = createAndOpenFile(grassFile.c_str());

  irr::core::stringc grassFile = "data/levels/";
  grassFile += parameters.levelName;
  grassFile += "/";
  grassFile += gObj.objectName;
  grassFile += ".grs";

  irr::io::IWriteFile *writer =
    Core->getRenderer()->getDevice()->getFileSystem()->createAndWriteFile(
      grassFile.c_str());


  writer->drop();


  reader->drop();*/

#endif

  return;
}

void CObjectManager::generateGrassGroupFile(
  SGrassObject gObj,
  const irr::c8 *gFile,
  irr::f32 xmin,
  irr::f32 xmax,
  irr::f32 zmin,
  irr::f32 zmax)
{
  irr::core::stringc groupsFile = "data/levels/";
  groupsFile += parameters.levelName;
  groupsFile += "/";
  groupsFile += gObj.objectName;
  groupsFile += ".fgd"; //foilage-group-data

  for(irr::f32 fg_x = xmin; fg_x <= xmax; fg_x += DISTANCE_BETWEEN_FOILAGE_GROUPS) {
    for(irr::f32 fg_z = zmin; fg_z <= zmax; fg_z += DISTANCE_BETWEEN_FOILAGE_GROUPS)
    {
      SFoilageGroup *f_group = new SFoilageGroup();

      f_group->ID = f_groups.size();

      // Center of each patch
      f_group->position = irr::core::vector3df(
        fg_x + DISTANCE_BETWEEN_FOILAGE_GROUPS/2,
        0.f,
        fg_z + DISTANCE_BETWEEN_FOILAGE_GROUPS/2);

      // Add it to list
      f_groups.push_back(f_group);
    }
  }

  // Find neighbours

  for(irr::u16 i=0; i < f_groups.size(); ++i)
  {
    f_groups[i]->neighbours.set_used(0);

    for(irr::u16 j=0; j < f_groups.size(); ++j)
    {
      if(i == j) continue;

      if(f_groups[i]->position.getDistanceFrom(f_groups[j]->position) <= DISTANCE_BETWEEN_FOILAGE_GROUPS*1.5f)
        f_groups[i]->neighbours.push_back(j);
    }
  }

  // Start writing to groups file ..

  irr::io::IWriteFile *writer =
  Core->getRenderer()->getDevice()->getFileSystem()->createAndWriteFile(
    groupsFile.c_str());


  char dataToWrite[128];


  //
  // Number of groups
  sprintf(dataToWrite, "%d", f_groups.size());

  writer->write(dataToWrite, (u32)strlen(dataToWrite));

  //
  // All group data that's needed
  for(irr::u32 i = 0; i < f_groups.size(); ++i)
  {
    //
    // ID, how many neighbours, position X Y Z
    sprintf(dataToWrite, "\n%d %d %.5f %.5f",
      i,
      f_groups[i]->neighbours.size(),
      f_groups[i]->position.X,
      f_groups[i]->position.Z);

    writer->write(dataToWrite, (u32)strlen(dataToWrite));

    //
    // All neighbours ID's
    for(irr::u32 n = 0; n < f_groups[i]->neighbours.size(); ++n)
    {
      sprintf(dataToWrite, " %d",
        f_groups[i]->neighbours[n]);

      writer->write(dataToWrite, (u32)strlen(dataToWrite));
    }
  }

  irr::io::IXMLReader *xml = Core->getRenderer()->getDevice()->getFileSystem()->createXMLReader(gFile);

  while(xml && xml->read())
  {
    irr::core::stringc nodeName = irr::core::stringc(xml->getNodeName());

    if(xml->getNodeType() == irr::io::EXN_ELEMENT)
    {
      irr::core::vector3df pos, rot, scale;

      pos.set(
        xml->getAttributeValueAsFloat(1),
        xml->getAttributeValueAsFloat(2),
        xml->getAttributeValueAsFloat(3));

      rot.set(
        xml->getAttributeValueAsFloat(4),
        xml->getAttributeValueAsFloat(5),
        xml->getAttributeValueAsFloat(6));

      scale.set(
        xml->getAttributeValueAsFloat(7),
        xml->getAttributeValueAsFloat(8),
        xml->getAttributeValueAsFloat(7));

      irr::u8 grassType = xml->getAttributeValueAsInt(0);

      irr::s32 f_group_index = findNearestFoilageGroup(pos);

      sprintf(dataToWrite, "\n%d %d %.5f %.5f %.5f %.5f %.5f %.5f %.5f %.5f",
        f_group_index,
        grassType,
        pos.X,
        pos.Y,
        pos.Z,
        rot.X,
        rot.Y,
        rot.Z,
        scale.X,
        scale.Y);
      writer->write(dataToWrite, (u32)strlen(dataToWrite));

    }
  }

  writer->drop();

  xml->drop();
  //delete xml;

  for(irr::u16 i=0; i < f_groups.size(); ++i)
  {
    f_groups[i]->neighbours.clear();
    delete f_groups[i];
  }

  f_groups.clear();
  f_groups.set_used(0);
}


void CObjectManager::copyMaterialToMesh(scene::IMesh *mesh, scene::ISceneNode *node)
{
  for(irr::u32 m=0; m < mesh->getMeshBufferCount(); ++m)
    mesh->getMeshBuffer(m)->getMaterial() = node->getMaterial(m);
}

scene::ISceneNode* CObjectManager::getRootParent(scene::ISceneNode* node) {
  scene::ISceneNode *noParent = Core->getRenderer()->getSceneManager()->getRootSceneNode();
  scene::ISceneNode *result = node->getParent();

  while(node->getParent() != noParent) result = node = node->getParent();

  return result;
}

irr::scene::ISceneNode* CObjectManager::getSceneNodeFromName(const irr::c8*name)
{
  /*irr::core::array<scene::ISceneNode *> all_nodes;
  Core->getRenderer()->getSceneManager()->getSce getSceneNodesFromType(scene::ESNT_ANY, all_nodes);

  for(u32 i=0; i < all_nodes.size(); ++i)
  {
    stringc node_name = stringc(all_nodes[i]->getName());
    if(node_name == stringc(name)) return all_nodes[i];
  }

  return Core->getRenderer()->getSceneManager()->getRootSceneNode();*/

  return Core->getRenderer()->getSceneManager()->getSceneNodeFromName(name);
}

irr::u16 CObjectManager::getMaterialFromName(irr::core::stringc name)
{
  for(irr::u16 mni=0; mni<nameMaterials.size(); ++mni)
    if(nameMaterials[mni]->name == name) return nameMaterials[mni]->material;

  return 3;
}

irr::core::stringc CObjectManager::getObjectSimpleName(const irr::c8*name)
{
  irr::core::stringc result = "", find_name = stringc(name);

  irr::s32 name_end_id = find_name.find(" ");

  if(name_end_id == -1) name_end_id = find_name.size();

  result = find_name.subString(0, name_end_id);
  result.replace('_', ' ');

  return result;
}

irr::core::stringc CObjectManager::getNodeFullName(const irr::c8*name)
{
  irr::core::stringc result="", find_name = stringc(name);

  irr::core::array<scene::ISceneNode *> all_nodes;
  Core->getRenderer()->getSceneManager()->getSceneNodesFromType(scene::ESNT_ANY, all_nodes);

  for(u32 i=0; i < all_nodes.size(); ++i)
  {
    irr::core::stringc full_name = all_nodes[i]->getName();

    irr::s32 name_end_id = full_name.find(" ");

    if(name_end_id == -1) name_end_id = full_name.size();

    if(full_name.subString(0, name_end_id) == find_name)
    {
      result = full_name;
      break;
    }
  }

  return result;
}


void CObjectManager::appendMesh(irr::scene::CBatchingMesh *mesh,
                                irr::s32 p_mesh_index,
                                irr::scene::ISceneNode *parent,
                                irr::scene::ISceneNode *node)
{
  IMeshManipulator *meshManip = Core->getRenderer()->getSceneManager()->getMeshManipulator();

  scene::IMesh *ChildMesh = ((scene::IMeshSceneNode*)node)->getMesh();
  //meshManip->createMeshWith2TCoords( ((scene::IMeshSceneNode*)node)->getMesh() );

  if(Core->commandLineParameters.hasParam("-disable_vbo") == false) {
    ChildMesh->setHardwareMappingHint(irr::scene::EHM_STATIC);
    ChildMesh->setDirty();
  }

    scene::IMesh *PhysicsMesh ;

  PhysicsMesh = Core->getPhysics()->getPhysicsMesh(node);
  scene::IMesh *PSimpleMesh = Core->getPhysics()->getPhysicsMeshSimple(node);

  matrix4 matParent = parent->getAbsoluteTransformation();
  matrix4 matChild  = node->getAbsoluteTransformation();

  vector3df pos = matChild.getTranslation() - matParent.getTranslation();
  vector3df rot = node->getRotation();
  vector3df scale = matChild.getScale();

  scene::ISceneNode *par = node->getParent();
  while(par != Core->getRenderer()->getSceneManager()->getRootSceneNode()) {
    rot += par->getRotation();
    par = par->getParent();
  }

  //if(PhysicsMesh != NULL)
    //p_mesh->addMesh(PhysicsMesh, pos, rot, scale);

  if(PhysicsMesh != NULL) {

    irr::scene::IMeshManipulator *mesh_manip = Core->getRenderer()->getSceneManager()->getMeshManipulator();

    matrix4 PMeshTransformation;
    PMeshTransformation.setRotationDegrees(rot);
    PMeshTransformation.setTranslation(pos);

    mesh_manip->scaleMesh(PhysicsMesh, scale);
    mesh_manip->transformMesh(PhysicsMesh, PMeshTransformation);

    if(PSimpleMesh)
    {
      mesh_manip->scaleMesh(PSimpleMesh, scale);
      mesh_manip->transformMesh(PSimpleMesh, PMeshTransformation);
    }

    copyMaterialToMesh(PhysicsMesh, node);
    Core->getPhysics()->meshGroups[p_mesh_index].meshes.push_back(PhysicsMesh);
    Core->getPhysics()->meshGroups[p_mesh_index].simple_collision_mesh.push_back(PSimpleMesh);
    Core->getPhysics()->meshGroups[p_mesh_index].name.push_back(getObjectSimpleName(node->getName()));
    Core->getPhysics()->meshGroups[p_mesh_index].irrID.push_back(node->getID());
    Core->getPhysics()->meshGroups[p_mesh_index].position.push_back(node->getAbsolutePosition());
  }

  // Copy material and add mesh
  copyMaterialToMesh( ChildMesh, node );

  mesh->addMesh(ChildMesh, pos, rot, scale);

  return;
}

void CObjectManager::checkChildren(ISceneNode *parent)
{
  irr::core::array<scene::ISceneNode *> all_nodes;
  bool allBatchedToParents = false;
  u16  loopCounter = 0;

  scene::ISceneNode *noParent = Core->getRenderer()->getSceneManager()->getRootSceneNode();

  while(allBatchedToParents == false)
  {
  	loopCounter ++;

    // Up to 16 levels of children
    if(loopCounter > 16) { printf("ERROR: Loop count exceeded: (%d MAX 16) in function \"groupChildrenWithParent\"\n", loopCounter); break; }

    all_nodes.set_used(0);
    Core->getRenderer()->getSceneManager()->getSceneNodesFromType(scene::ESNT_MESH, all_nodes);

  	allBatchedToParents = true;

    for(u32 c=0; c < all_nodes.size(); ++c)
  	{
      scene::ISceneNode *node = all_nodes[c];

      node->updateAbsolutePosition();

      if(node->getParent() != noParent && getRootParent(node) == parent)
      {
        irr::core::stringc n_name = irr::core::stringc(node->getName());

        if(isNodeParameterSet(n_name, "n") || isNodeParameterSet(n_name, "d"))
        {
          matrix4 node_transformation = node->getAbsoluteTransformation();

          node->setParent(noParent);
          node->setScale( node_transformation.getScale() );
          node->setPosition( node_transformation.getTranslation() );
          node->setRotation( node_transformation.getRotationDegrees() );

          node->updateAbsolutePosition();

          node->setParam(1, parent->getParam(1));
          node->setParam(0, 0);

          rootNodes.push_back(node);
        }
      }
    } // all_nodes.size()
  }

}

void CObjectManager::groupChildrenWithParent(
  scene::ISceneNode *parent,
  scene::CBatchingMesh *mesh,
  irr::s32 p_mesh_index)
{
  irr::core::array<scene::ISceneNode *> all_nodes;
  bool allBatchedToParents = false;
  u16  loopCounter = 0;

  scene::ISceneNode *noParent = Core->getRenderer()->getSceneManager()->getRootSceneNode();

  while(allBatchedToParents == false)
  {
  	loopCounter ++;

    // Up to 16 levels of children
    if(loopCounter > 16) { printf("ERROR: Loop count exceeded: (%d MAX 16) in function \"groupChildrenWithParent\"\n", loopCounter); break; }

    all_nodes.set_used(0);
    Core->getRenderer()->getSceneManager()->getSceneNodesFromType(scene::ESNT_MESH, all_nodes);

  	allBatchedToParents = true;

    for(u32 c=0; c < all_nodes.size(); ++c)
  	{
      scene::ISceneNode *node = all_nodes[c];

      node->updateAbsolutePosition();

      if(node->getParent() != noParent && getRootParent(node) == parent)
      {
        irr::core::stringc n_name = irr::core::stringc(node->getName());

        if(node->getChildren().getSize() == 0)
        {
          appendMesh(mesh, p_mesh_index, parent, node);

          node->remove();

          allBatchedToParents = false;

          continue;
        }

      }
    } // all_nodes.size()
  }

  return;
}

bool isBaseBuilding(irr::core::stringc name)
{
  if(name == "Team1Powerplant"
  || name == "Team2Powerplant"
  || name == "Team1Command"
  || name == "Team2Command"
  || name == "Team1DefenceTower"
  || name == "Team2DefenceTower")
  return true;

  return false;
}

bool isAcceptableName(irr::core::stringc name)
{
  if(name == "Terrain" || isBaseBuilding(name))
  return false;

  return true;
}

void CObjectManager::clearAll(bool app_close)
{
  currentFoilageGroup = (SFoilageGroup*) NULL;

  printf("1\n");

  for(irr::u16 i=0; i< buildingList.size(); ++i)
    delete buildingList[i];

  for(irr::u16 i=0; i< staticList.size(); ++i)
    delete staticList[i];

  for(irr::u16 i=0; i< dynamicList.size(); ++i)
    delete dynamicList[i];

  for(irr::u16 i=0; i< doorList.size(); ++i)
    delete doorList[i];

  printf("2\n");

  buildingList.clear();
  staticList.clear();
  dynamicList.clear();
  doorList.clear();

#ifdef SOUND_IRRKLANG

  for(irr::u16 i=0; i< a_AmbientSounds.size(); ++i)
  {
    a_AmbientSounds[i].sound->stop();
  }

  Core->getSound()->clear();

#endif

#ifdef SOUND_CAUDIO
  if(ambientSoundSource)
  {
    ambientSoundSource->stop();
    ambientSoundSource = NULL;
  }
#endif

  printf("3\n");

  if(foilageNode && !app_close)
  {
    foilageNode->remove();
    foilageNode = (irr::scene::IMeshSceneNode*) NULL;
  }

#ifdef GRASS_2

  if(gNode && !app_close)
  {
    gNode->remove();
    gNode = (CGrassSceneNode*) NULL;
  }

#endif


  printf("3b\n");

  for(irr::u16 i=0; i< grassMeshes.size(); ++i)
  {
    grassMeshes[i]->finalize();
    grassMeshes[i]->clear();
    delete grassMeshes[i];
  }

  grassMeshes.clear();
  grassMeshes.set_used(0);

  if(!app_close)
  {
    Core->getRenderer()->getSceneManager()->clear();

    Core->getCamera()->reset();
  }
  else
  {
    for(irr::u16 i=0; i< nameMaterials.size(); ++i)
      delete nameMaterials[i];

    nameMaterials.clear();
  }


  printf("4\n");






  for(irr::u16 i=0; i < f_groups.size(); ++i)
  {
#ifndef GRASS_2
    f_groups[i]->meshHigh->clear();
    f_groups[i]->meshMed->clear();
    f_groups[i]->meshLow->clear();

    delete f_groups[i]->meshHigh;
    delete f_groups[i]->meshMed;
    delete f_groups[i]->meshLow;
#else
    for(irr::u16 e=0; e < f_groups[i]->elements.size(); ++e)
      delete f_groups[i]->elements[e];

    f_groups[i]->elements.clear();
#endif

    delete f_groups[i];
  }


  f_groups.clear();
  f_groups.set_used(0);

  for(irr::u16 i=0; i< levelMeshes.size(); ++i)
  {
    //Core->getRenderer()->getSceneManager()->getMeshCache()->removeMesh(levelMeshes[i]);

    levelMeshes[i]->finalize();
    levelMeshes[i]->clear();
    delete levelMeshes[i];
  }

  levelMeshes.clear();
  levelMeshes.set_used(0);

  parameters.grassObjects.clear();
  triggerObjects.clear();
  a_Sounds.clear();
  a_AmbientSounds.clear();

#ifdef PHYSICS_NEWTON
  Core->getPhysics()->clearMeshGroups();
#endif

  if(!app_close)
  {
    Core->getRenderer()->getSceneManager()->getMeshCache()->clearUnusedMeshes();
    Core->getRenderer()->getVideoDriver()->removeAllHardwareBuffers();
    Core->getRenderer()->getVideoDriver()->removeAllTextures();

    Core->getCamera()->setType(ECT_UNDEFINED);
  }

  printf("5\n");
}

void CObjectManager::groupNodes()
{
  printf("Grouping nodes\n");

  Core->getPhysics()->meshGroups.set_used(0);

	irr::core::array<scene::ISceneNode *> nodes;
  scene::IMeshSceneNode *node, *node2;
  scene::ISceneNode *noParent = Core->getRenderer()->getSceneManager()->getRootSceneNode();

  Core->getRenderer()->getSceneManager()->getSceneNodesFromType(scene::ESNT_ANY, nodes);

  irr::core::array<irr::scene::ISceneNode*> emptyNodes;
  emptyNodes.set_used(0);
  rootNodes.set_used(0);

  //
  // Get all root nodes
  // (Nodes that have children and their parent is scene root node)
  //
  for(u32 i=0; i < nodes.size(); ++i)
  {
    nodes[i]->setParam(0, 0);
    nodes[i]->setParam(1, 0);
    nodes[i]->setParam(2, 0);
    nodes[i]->setParam(3, -1);

    if(nodes[i]->getType() == scene::ESNT_MESH)
    {
      bool hasNoParent = false;

      if(nodes[i]->getParent() == noParent) hasNoParent = true;

      if(nodes[i]->getParent()->getType() == scene::ESNT_EMPTY
      && getObjectSimpleName(nodes[i]->getParent()->getName()) != "AutoDoor"
      && getObjectSimpleName(nodes[i]->getName()) != "Door")
      {
        nodes[i]->setParam(1, emptyNodes.size()+1);

        emptyNodes.push_back(nodes[i]->getParent());

        nodes[i]->setParent(noParent);

        hasNoParent = true;
      }

      if(hasNoParent && nodes[i]->getName() != "GrassNode")
      {
        rootNodes.push_back(nodes[i]);
      }
    }
  }







  //
  // 1 - Group children nodes with parent
  //

  printf("\tMerging children nodes with parent ... ");

  IMeshManipulator *meshManip = Core->getRenderer()->getSceneManager()->getMeshManipulator();

  for(u32 i=0; i < rootNodes.size(); ++i)
  {
    node = (IMeshSceneNode*)rootNodes[i];

    checkChildren(node);

    if(node->getChildren().getSize() == 0 && !isBaseBuilding(node->getName())) continue;
    //else if(isAcceptableName(node->getName()) == false) continue;

    node->updateAbsolutePosition();

    // Create a new batching mesh
    scene::CBatchingMesh *groupMesh = new scene::CBatchingMesh();

    SPhysicsMesh p_meshGroup;
    p_meshGroup.meshes.set_used(0);
    p_meshGroup.simple_collision_mesh.set_used(0);

    Core->getPhysics()->meshGroups.push_back( p_meshGroup );
    node->setParam(3, Core->getPhysics()->meshGroups.size()-1);

    // Get parent mesh
    scene::IMesh *ParentMesh = ((scene::IMeshSceneNode*)node)->getMesh();
    scene::IMesh *PMesh = Core->getPhysics()->getPhysicsMesh(node);

    if(Core->commandLineParameters.hasParam("-disable_vbo") == false) {
      ParentMesh->setHardwareMappingHint(irr::scene::EHM_STATIC);
      ParentMesh->setDirty();
    }

    scene::IMesh *PSimpleMesh = Core->getPhysics()->getPhysicsMeshSimple(node);

    if(PMesh != NULL) {
      // Transform mesh
      irr::scene::IMeshManipulator *mesh_manip = Core->getRenderer()->getSceneManager()->getMeshManipulator();

      matrix4 PMeshTransformation;
      PMeshTransformation.setRotationDegrees(node->getRotation());
      PMeshTransformation.setTranslation(irr::core::vector3df(0,0,0));

      mesh_manip->scaleMesh(PMesh, node->getScale());
      mesh_manip->transformMesh(PMesh, PMeshTransformation);

      if(PSimpleMesh)
      {
        mesh_manip->scaleMesh(PSimpleMesh, node->getScale());
        mesh_manip->transformMesh(PSimpleMesh, PMeshTransformation);
      }
      copyMaterialToMesh(PMesh, node);
      Core->getPhysics()->meshGroups[node->getParam(3)].meshes.push_back(PMesh);
      Core->getPhysics()->meshGroups[node->getParam(3)].simple_collision_mesh.push_back(PSimpleMesh);
      Core->getPhysics()->meshGroups[node->getParam(3)].name.push_back(getObjectSimpleName(node->getName()));
      Core->getPhysics()->meshGroups[node->getParam(3)].irrID.push_back(node->getID());
      Core->getPhysics()->meshGroups[node->getParam(3)].position.push_back(node->getAbsolutePosition());

    }

    // Copy material and add render mesh
    copyMaterialToMesh( ParentMesh, node );
    groupMesh->addMesh(
      ParentMesh,
      vector3df(0,0,0),
      node->getRotation(),
      node->getScale());

    groupChildrenWithParent(node, groupMesh, node->getParam(3));

    //groupMesh->setHardwareMappingHint(scene::EHM_STATIC, scene::EBT_VERTEX_AND_INDEX);
    //groupMesh->setDirty(scene::EBT_VERTEX_AND_INDEX);

    // Update meshes
    groupMesh->update();

    if(Core->commandLineParameters.hasParam("-disable_vbo") == false) {
      groupMesh->setHardwareMappingHint(scene::EHM_STATIC, EBT_VERTEX_AND_INDEX);
      groupMesh->setDirty(EBT_VERTEX_AND_INDEX);
    }

    // Use new batched mesh
    ((scene::IMeshSceneNode*)node)->setMesh( groupMesh );

    node->setParam(0, 1);
    node->setScale(vector3df(1,1,1));
    node->setRotation(vector3df(0,0,0));

    if(Core->commandLineParameters.hasParam("-disable_vbo") == false) {
      ((scene::IMeshSceneNode*)node)->getMesh()->setHardwareMappingHint(scene::EHM_STATIC, EBT_VERTEX_AND_INDEX);
      ((scene::IMeshSceneNode*)node)->getMesh()->setDirty(EBT_VERTEX_AND_INDEX);
    }

    levelMeshes.push_back(groupMesh);
  }

  printf("ok!\n");



  //
  // 2 - Combine similar nodes nearby
  //

  printf("\tMerging nodes with same name or mesh ... ");


  for(u32 i=0; i < rootNodes.size(); ++i)
  {
    node = (scene::IMeshSceneNode*)rootNodes[i];
    irr::core::stringc node_name = getObjectSimpleName(node->getName());
    bool skip_batching = false;

    if(node->getParam(0) != 0)
      continue;
    else if(isAcceptableName(node_name) == false)
      continue;
    else if(isNodeParameterSet(irr::core::stringc(node->getName()), "d")
    || isNodeParameterSet(irr::core::stringc(node->getName()), "n")) {
      skip_batching = true;
    }

    vector3df node_pos = node->getAbsolutePosition();

    // Create a new batching mesh
    scene::CBatchingMesh *groupMesh;

    SPhysicsMesh p_meshGroup;
    scene::IMesh *PMesh = (irr::scene::IMesh*) NULL;

    //if(!skip_batching || isNodeParameterSet(node, "n"))
    //{
    p_meshGroup.meshes.set_used(0);

    Core->getPhysics()->meshGroups.push_back(p_meshGroup);

    node->setParam(3, Core->getPhysics()->meshGroups.size()-1);

    PMesh = Core->getPhysics()->getPhysicsMesh(node);

    scene::IMesh *PSimpleMesh = Core->getPhysics()->getPhysicsMeshSimple(node);

    //}

    // Get parent mesh
    scene::IMesh *ParentMesh = ((scene::IMeshSceneNode*)node)->getMesh();

    if(Core->commandLineParameters.hasParam("-disable_vbo") == false) {
      ParentMesh->setHardwareMappingHint(irr::scene::EHM_STATIC);
      ParentMesh->setDirty();
    }

    if(PMesh != NULL) {

      irr::scene::IMeshManipulator *mesh_manip = Core->getRenderer()->getSceneManager()->getMeshManipulator();

      matrix4 PMeshTransformation;
      PMeshTransformation.setRotationDegrees(node->getRotation());
      PMeshTransformation.setTranslation(irr::core::vector3df(0,0,0));

      mesh_manip->scaleMesh(PMesh, node->getScale());
      mesh_manip->transformMesh(PMesh, PMeshTransformation);

      if(PSimpleMesh != NULL)
      {
        mesh_manip->scaleMesh(PSimpleMesh, node->getScale());
        mesh_manip->transformMesh(PSimpleMesh, PMeshTransformation);
      }

      copyMaterialToMesh(PMesh, node);
      Core->getPhysics()->meshGroups[node->getParam(3)].meshes.push_back(PMesh);
      Core->getPhysics()->meshGroups[node->getParam(3)].simple_collision_mesh.push_back(PSimpleMesh);
      Core->getPhysics()->meshGroups[node->getParam(3)].name.push_back(getObjectSimpleName(node->getName()));
      Core->getPhysics()->meshGroups[node->getParam(3)].irrID.push_back(node->getID());
      Core->getPhysics()->meshGroups[node->getParam(3)].position.push_back(node->getAbsolutePosition());
    }


    groupMesh = new scene::CBatchingMesh();

    // Copy material and add mesh
    copyMaterialToMesh( ParentMesh, node );

    groupMesh->addMesh(
      ParentMesh,
      vector3df(0,0,0),
      node->getRotation(),
      node->getScale());

    if(!skip_batching)
    for(u32 j=0; j < rootNodes.size(); ++j)
    {
      node2 = (scene::IMeshSceneNode*)rootNodes[j];

      if(node2 == node) continue;

      irr::core::stringc node_name2 = getObjectSimpleName(node2->getName());

      if(isAcceptableName(node_name2) == false)
        continue;
      else if(isNodeParameterSet(irr::core::stringc(node2->getName()), "d")
      || isNodeParameterSet(irr::core::stringc(node2->getName()), "n"))
        continue;
      else if(node2->getParam(0) != 0) continue;

      // Both objects belong to an LEVEL GROUP but not to the same one. Discard.
      if(node2->getParam(1) > 0 && node->getParam(1) > 0) {
        if(emptyNodes[node2->getParam(1)-1] != emptyNodes[node->getParam(1)-1]) {
          continue;
        }
      }
      // One node has a LEVEL GROUP, the other doesn't. Discard.
      else if((node2->getParam(1) > 0 && node->getParam(1) == 0) || (node->getParam(1) > 0 && node2->getParam(1) == 0))
      {
        continue;
      }

      bool batchTogether = false;

      ISceneNode *empty1 = noParent;
      ISceneNode *empty2 = noParent;

      scene::IMesh *mesh2 = ((scene::IMeshSceneNode*)node2)->getMesh();

      if(node2->getParam(1) > 0) empty1 = emptyNodes[node2->getParam(1)-1];
      if(node->getParam(1) > 0) empty2 = emptyNodes[node->getParam(1)-1];


      // Priority 1
      if(empty1 != noParent && empty2 != noParent) {
        if(empty1 == empty2) {
          batchTogether = true;
        }
      }
      // Priority 2
      else if((node_name2 == node_name || mesh2 == ParentMesh)
      && node_pos.getDistanceFrom(node2->getAbsolutePosition()) <= 300
      && (empty1 == noParent && empty2 == noParent)) {
        batchTogether = true;
      }
      // Priority 3
      else if(node_pos.getDistanceFrom(node2->getAbsolutePosition()) <= 300) {
        batchTogether = true;
      }

      /*if(((node_name2 == node_name || mesh2 == ParentMesh)
      && node_pos.getDistanceFrom(node2->getAbsolutePosition()) <= 300)
      || (node_pos.getDistanceFrom(node2->getAbsolutePosition()) <= 300))*/

      if(batchTogether)
      {
        //is_mesh_empty = false;

        // Append node's mesh to parent mesh
        appendMesh(groupMesh, node->getParam(3), node, node2);

        // Remove node
        node2->setParam(0, 1);
        rootNodes.erase(j);
        node2->remove();

        j -= 1;
        i = 0;

        continue;
      }

    }

    groupMesh->update();

    if(Core->commandLineParameters.hasParam("-disable_vbo") == false) {
      groupMesh->setHardwareMappingHint(scene::EHM_STATIC, EBT_VERTEX_AND_INDEX);
      groupMesh->setDirty(EBT_VERTEX_AND_INDEX);
    }

    // Use new batched mesh
    node->setMesh(groupMesh);

    node->setParam(0, 1);

    if(Core->commandLineParameters.hasParam("-disable_vbo") == false) {
      ((scene::IMeshSceneNode*)node)->getMesh()->setHardwareMappingHint(scene::EHM_STATIC, EBT_VERTEX_AND_INDEX);
      ((scene::IMeshSceneNode*)node)->getMesh()->setDirty(EBT_VERTEX_AND_INDEX);
    }

    node->setScale(vector3df(1,1,1));
    node->setRotation(vector3df(0,0,0));

    levelMeshes.push_back(groupMesh);
  }
  // step 2 is done!
  printf("ok!\n");




  nodes.set_used(0);
  Core->getRenderer()->getSceneManager()->getSceneNodesFromType(scene::ESNT_ANY, nodes);

  for(u32 n=0; n < nodes.size(); ++n)
  {
    if(nodes[n]->getParam(1) == 0) continue;

    nodes[n]->setPosition( nodes[n]->getPosition() + emptyNodes[ nodes[n]->getParam(1)-1 ]->getPosition() );
    emptyNodes[nodes[n]->getParam(1)-1]->setPosition(irr::core::vector3df(0,0,0));

    nodes[n]->setParent(emptyNodes[nodes[n]->getParam(1)-1]);
    nodes[n]->setParam(1, 0);

  }

  emptyNodes.clear();
  nodes.clear();
  rootNodes.clear();

  return;
}

irr::core::stringc getBuildingName(irr::core::stringc in)
{
  irr::core::stringc out;

  if(in == "Team1Powerplant" || in == "Team2Powerplant")
    out = "Power plant";
  else if(in == "Team1DefenceTower" || in == "Team2DefenceTower")
    out = "Defence tower";
  else if(in == "Team1Command" || in == "Team2Command")
    out = "Command center";

  return out;
}

bool CObjectManager::addObject(irr::scene::ISceneNode*node)
{
  bool createPhysicsBodies = !Core->commandLineParameters.hasParam("-disable_physics");

  /*switch(node->getType())
  {
    case scene::ESNT_MESH:
    {*/
      // Convert node to mesh node
      scene::IMeshSceneNode *meshNode = (scene::IMeshSceneNode*)node;

#ifdef PHYSICS_NEWTON
      // Create physics body/bodies for the node
      irr::core::array<physics::CBody*> bodies;
      bodies.set_used(0);

      if(createPhysicsBodies)
        bodies = Core->getPhysics()->createStaticPhysics(meshNode);

      if(bodies.size() > 0)
      {
        //
        // Check all bodies of the node

        for(irr::u32 bId=0; bId < bodies.size(); ++bId)
        {
          // New object
          CBaseMapObject * newObject;
          irr::u32 container_id = 0;
          irr::core::stringc objectName = "";
          irr::u32 objectID = 0;
          irr::core::vector3df originalPosition;

          if(node->getParam(3) == -1) {
            objectName = getObjectSimpleName(node->getName());
            objectID = node->getID();
            originalPosition = node->getAbsolutePosition();
          } else {
            objectName = Core->getPhysics()->meshGroups[node->getParam(3)].name[bId];
            objectID = Core->getPhysics()->meshGroups[node->getParam(3)].irrID[bId];
            originalPosition = Core->getPhysics()->meshGroups[node->getParam(3)].position[bId];
          }

          bodies[bId]->setOriginalPosition(originalPosition);

          // Check type

          game::E_OBJECT_TYPES o_type = game::EOT_STATIC;

          if(isNodeParameterSet(objectName, "d")) {
            o_type = game::EOT_DYNAMIC;
          }
          else if(isBaseBuilding(objectName.c_str())) {
            o_type = game::EOT_BUILDING;
          }
          else if(objectName == "Terrain") {
            o_type = game::EOT_TERRAIN;

            /*CTerrainNode *terrainNode = new CTerrainNode(
              Core->getRenderer()->getSceneManager()->getRootSceneNode(),
              Core->getRenderer()->getSceneManager(),
              9999);

            terrainNode->setPosition(meshNode->getPosition());
            terrainNode->setScale(meshNode->getScale());
            terrainNode->setRotation(meshNode->getRotation());
            terrainNode->updateAbsolutePosition();

            copyMaterialToMesh(meshNode->getMesh(), meshNode);

            terrainNode->setName("TerrainNew");
            terrainNode->setMesh(meshNode->getMesh());

            bodies[bId]->setNode(terrainNode);*/
          }
          else if(objectName == "Console"
          || objectName == "Radio") {
            o_type = game::EOT_INTERACTABLE;
          }

          //
          // Create new object container

          if(o_type == game::EOT_STATIC) {
            newObject = new game::CStaticObject();
            container_id = staticList.size();
            staticList.push_back((game::CStaticObject*)newObject);
          }
          else if(o_type == game::EOT_DYNAMIC) {
            newObject = new game::CDynamicObject();
            container_id = dynamicList.size();
            dynamicList.push_back((game::CDynamicObject*)newObject);
          }
          else if(o_type == game::EOT_BUILDING) {
            newObject = new game::CBaseBuilding();
            container_id = buildingList.size();
            buildingList.push_back((game::CBaseBuilding*)newObject);

            objectName = getBuildingName(objectName);
          }
          else if(o_type == game::EOT_TERRAIN) {
            newObject = new game::CStaticObject();
            container_id = staticList.size();
            staticList.push_back((game::CStaticObject*)newObject);
          }
          else if(o_type == game::EOT_INTERACTABLE) {
            newObject = new game::CStaticObject();
            container_id = interactableList.size();
            interactableList.push_back((game::CStaticObject*)newObject);
          }

          game::SObjectData *objectData = new game::SObjectData();
          objectData->container_id = container_id;
          objectData->element_id = bId;
          objectData->type = o_type;
          objectData->material = getMaterialFromName(objectName);
          objectData->name = objectName;
          objectData->team = game::E_TEAM1;

          newObject->setBody(bodies[bId]);

          newObject->setBodyData((void*)objectData);
        }
#endif
      }

      if(meshNode->getParam(3) != -1)
      {
        irr::u32 meshGrpIdx = meshNode->getParam(3);

        for(irr::u32 j=0; j < Core->getPhysics()->meshGroups[meshGrpIdx].meshes.size(); ++j)
        {
          if(Core->getPhysics()->meshGroups[meshGrpIdx].meshes[j])
          {
            Core->getPhysics()->meshGroups[meshGrpIdx].meshes[j]->drop();
            Core->getRenderer()->getSceneManager()->getMeshCache()->removeMesh(Core->getPhysics()->meshGroups[meshGrpIdx].meshes[j]);
          }
        }

        Core->getPhysics()->meshGroups[meshGrpIdx].meshes.clear();

        for(irr::u32 j=0; j < Core->getPhysics()->meshGroups[meshGrpIdx].simple_collision_mesh.size(); ++j)
        {
          if(Core->getPhysics()->meshGroups[meshGrpIdx].simple_collision_mesh[j])
          {
            Core->getPhysics()->meshGroups[meshGrpIdx].simple_collision_mesh[j]->drop();
            Core->getRenderer()->getSceneManager()->getMeshCache()->removeMesh(Core->getPhysics()->meshGroups[meshGrpIdx].simple_collision_mesh[j]);
          }
        }

        Core->getPhysics()->meshGroups[meshGrpIdx].simple_collision_mesh.clear();

      }

      if(Core->commandLineParameters.hasParam("-disable_vbo") == false)
      {
         meshNode->getMesh()->setHardwareMappingHint(scene::EHM_STATIC, scene::EBT_VERTEX_AND_INDEX);
         meshNode->getMesh()->setDirty();
      }

    /*}
    break;
  }*/

  return true;
}

void CObjectManager::calculateLightmapForNode(IMeshSceneNode* node)
{
  irr::s16 meshBufferIndex = -1;

  for(irr::u32 i=0; i < node->getMaterialCount(); ++i)
  {
    irr::video::ITexture* MaterialTexture = node->getMaterial(i).getTexture(0);

    if(MaterialTexture)
    {
        irr::core::stringc texName = MaterialTexture->getName();

        char toFind = '\\';
        irr::u32 lastPos = texName.findLast( toFind ) + 1;

        irr::core::stringc textureFile = texName.subString(lastPos, texName.size() - lastPos-4);
        irr::core::stringc endStr = texName.subString(texName.size()-7, 3);

        // Multi-texture shader
        if(endStr.equals_ignore_case("_mt") == true)
          meshBufferIndex = i;
    }
  }

  if(meshBufferIndex == -1)
    return;

  u16 *indices;
  void* vrt;
  S3DVertex* vertices;

  IMeshBuffer *mb = node->getMesh()->getMeshBuffer(meshBufferIndex);

  dimension2du texSize = dimension2du(512, 512);
  //texSize -= dimension2du(1,1);

  // Create a new blank texture
  IImage *output = Core->getRenderer()->getVideoDriver()->createImage(ECF_A8R8G8B8, texSize);

  // Black background
  output->fill(SColor(255, 0,0,0));

  // Get indices and vertices of mesh buffer
  indices = mb->getIndices();
  vrt = mb->getVertices();
  vertices = (S3DVertex*)vrt;

  irr::core::array<triangle3df> tri1, tri2;
  irr::core::array<rectf> boxes;
#ifdef PHYSICS_IRR_NEWT
  irr::core::array<NewtonBody*> excluded;
#endif
  tri1.set_used(0);
  tri2.set_used(0);
  boxes.set_used(0);

  vector3df SunPos = Core->getObjects()->parameters.sunPosition;

  printf("\tSun position: %.3f, %.3f, %.3f\n", SunPos.X, SunPos.Y, SunPos.Z);

  printf("\tSetting up collision triangles (Tri-count: %d) ... ", mb->getIndexCount()/3);

  for(irr::u32 t=0; t < mb->getIndexCount(); t+=3)
  {
    vector2df in1 = vertices[indices[t]].TCoords;
    vector2df in2 = vertices[indices[t+1]].TCoords;
    vector2df in3 = vertices[indices[t+2]].TCoords;

    triangle3df tri_1 = triangle3df(
     vector3df(in1.X, 0, in1.Y),
     vector3df(in2.X, 0, in2.Y),
     vector3df(in3.X, 0, in3.Y));

    irr::f32 x1=0, x2=0, y1=0, y2=0;

    x1 = fmin(fmin(in1.X, in2.X), in3.X);
    x2 = fmax(fmax(in1.X, in2.X), in3.X);
    y1 = fmin(fmin(in1.Y, in2.Y), in3.Y);
    y2 = fmax(fmax(in1.Y, in2.Y), in3.Y);

    boxes.push_back(rectf(x1, y1, x2, y2));

    triangle3df tri_2 = triangle3df(
      vertices[indices[t]].Pos,
      vertices[indices[t+1]].Pos,
      vertices[indices[t+2]].Pos);

    tri1.push_back(tri_1);
    tri2.push_back(tri_2);


    /*irr::core::line3df line1, line2, line3;
    irr::core::array<irr::newton::SIntersectionPoint> out;
    irr::core::array<irr::newton::SIntersectionPoint> result;

    result.set_used(0);

    line1.start = tri_2.pointA;
    line1.end = line1.start + SunPos;

    line2.start = tri_2.pointB;
    line2.end = line2.start + SunPos;

    line3.start = tri_2.pointC;
    line3.end = line3.start + SunPos;

    //
    // Get excluded collisions
    //

    out.set_used(0);
    Core->getPhysics()->GetWorld()->getCollisionManager()->getCollisionPoints(line1, &out);
    for(irr::u32 i=0; i<out.size(); ++i) result.push_back( out[i] );

    out.set_used(0);
    Core->getPhysics()->GetWorld()->getCollisionManager()->getCollisionPoints(line2, &out);
    for(irr::u32 i=0; i<out.size(); ++i) result.push_back( out[i] );

    out.set_used(0);
    Core->getPhysics()->GetWorld()->getCollisionManager()->getCollisionPoints(line3, &out);
    for(irr::u32 i=0; i<out.size(); ++i) result.push_back( out[i] );

    irr::core::array<irr::newton::IBody *> bodies2 = Core->getPhysics()->GetWorld()->getAllBodies();

    for(u32 b=0; b < result.size(); ++b)
    {
      irr::s32 res = bodies2.linear_search(result[b].body);

      if(res != -1) {
        bodies2.erase(res);
      }
    }

    irr::core::array<NewtonBody*> newtonbodies;
    newtonbodies.set_used(0);

    for(u32 b=0; b < bodies2.size(); ++b) newtonbodies.push_back(bodies2[b]->getNewtonBody());

    excluded.push_back(newtonbodies);*/
  }

  printf("ok!\n");

  matrix4 matObject;
  matObject.setRotationDegrees(node->getRotation());
  #ifdef PHYSICS_IRR_NEWT
  for(irr::u16 di=0; di < Core->getObjects()->getDynamicObjectCount(); ++di)
  {
    for(irr::u16 bi=0; bi < Core->getObjects()->getDynamicObjectById(di)->getBodyCount(); ++bi)
    {
      excluded.push_back(Core->getObjects()->getDynamicObjectById(di)->getBodyByID(bi)->getNewtonBody());
    }
  }
#endif
  //ITimer *timer = Core->getRenderer()->getDevice()->getTimer();

  //newton::IBody* node_body = Core->getPhysics()->GetWorld()->getUtils()->getBodyFromNode(node);

  f32 resolution = 0; // 0,1,2,3,4
  /*u32 time_start = 0;
  u32 time_end = 0;
  u32 time_longest = 0;
  u32 time_shortest = 999999;*/

  for(u32 w=0; w < texSize.Width; w += u32(resolution+1)) {
  for(u32 h=0; h < texSize.Height; h += u32(resolution+1))
  {
    f32 TCoordX = f32(w) / f32(texSize.Width-1);
    f32 TCoordY = f32(h) / f32(texSize.Height-1);

    if(output->getPixel(u32(TCoordX), u32(TCoordY)) == SColor(255, 255,255,255)) continue;

    //timer->tick();
    //time_start = timer->getTime();

    // Find on which triangle is the texture coordinate
    for(irr::u32 t=0; t < tri1.size(); ++t)
    {
      vector3df checkPos = vector3df(TCoordX, 0, TCoordY);

      if(!boxes[t].isPointInside(irr::core::position2df(TCoordX, TCoordY))) continue;

      if(tri1[t].isPointInside(checkPos))
      {
        f32 mat_1_1 = tri1[t].pointA.X - tri1[t].pointC.X;
        f32 mat_1_2 = tri1[t].pointB.X - tri1[t].pointC.X;
        f32 mat_2_1 = tri1[t].pointA.Z - tri1[t].pointC.Z;
        f32 mat_2_2 = tri1[t].pointB.Z - tri1[t].pointC.Z;

        f32 detT = (mat_1_1 * mat_2_2) - (mat_2_1 * mat_1_2);

        f32 l1 = ((mat_2_2*(TCoordX-tri1[t].pointC.X)) - (mat_1_2*(TCoordY-tri1[t].pointC.Z))) / detT;
        f32 l2 = (-(mat_2_1*(TCoordX-tri1[t].pointC.X)) + (mat_1_1*(TCoordY-tri1[t].pointC.Z))) / detT;
        f32 l3 = 1 - l1 - l2;

        vector3df resultPos = (tri2[t].pointA * l1) + (tri2[t].pointB * l2) + (tri2[t].pointC * l3);
        resultPos *= node->getScale();

        matObject.rotateVect(resultPos);

        resultPos += node->getPosition();

        irr::core::vector3df normal = tri2[t].getNormal().normalize();

        matObject.rotateVect(normal);

        resultPos += normal*0.25f;

        //Core->getRenderer()->getSceneManager()->addCubeSceneNode(0.25, 0, -1, resultPos);

        // Tracing FROM the sun to the pixel
        line3df sun_line = line3df(resultPos, resultPos+SunPos);

#ifdef PHYSICS_NEWTON

        physics::SRayCastParameters parameters;
        parameters.line = sun_line;

        physics::SRayCastResult col_out = Core->getPhysics()->getRayCollision(parameters);

        if(col_out.body != NULL)
        {
          output->setPixel(w, h, SColor(255, 255,255,255));

          for(s32 rx = -s32(resolution); rx < s32(resolution); ++rx)
          for(s32 ry = -s32(resolution); ry < s32(resolution); ++ry)
          {
            output->setPixel(w+rx, h+ry, SColor(255, 255,255,255));
          }
        }


#endif

#ifdef PHYSICS_IRR_NEWT

        newton::SIntersectionPoint col_out =
          Core->getPhysics()->GetCollisionFromLine(sun_line, excluded);

        if(col_out.body != NULL)
        {
          output->setPixel(w, h, SColor(255, 255,255,255));

          for(s32 rx = -s32(resolution); rx < s32(resolution); ++rx)
          for(s32 ry = -s32(resolution); ry < s32(resolution); ++ry)
          {
            output->setPixel(w+rx, h+ry, SColor(255, 255,255,255));
          }
        }

        // Test collision against all possible objects

        /*irr::core::array<irr::newton::SIntersectionPoint> p_objects = collisions[t];

        for(irr::u32 p_obj=0; p_obj<p_objects.size(); ++p_obj)
        {
          newton::SIntersectionPoint col_out =
            Core->getPhysics()->GetCollisionFromLine(p_objects[p_obj].body, sun_line);

          if(col_out.body != NULL)
          {
            output->setPixel(w, h, SColor(255, 255,255,255));

            for(s32 rx = -s32(resolution); rx < s32(resolution); ++rx)
            for(s32 ry = -s32(resolution); ry < s32(resolution); ++ry)
            {
              output->setPixel(w+rx, h+ry, SColor(255, 255,255,255));
            }

            break;
          }
        }*/

#endif

        break;
      }
    }

    /*timer->tick();
    time_end = timer->getTime();

    u32 time = time_end - time_start;

    if(time > time_longest) time_longest = time;
    else if(time < time_shortest) time_shortest = time;*/

    //printf("\tTime: %d ms\n", time);

  }

  printf("\tProgress %d/%d..\n", w+1, texSize.Width);


  /*IVideoDriver* drv = Core->getRenderer()->getVideoDriver();
  ITexture *tmp_tex = drv->addTexture("tmp_dbg", output);

  drv->beginScene(true, true, SColor(255, 30,30,30));

  drv->2DImage(
    tmp_tex,
    rect<s32>(0, 0, texSize.Width, texSize.Height),
    rect<s32>(0, 0, texSize.Width, texSize.Height), 0, 0, true);
  drv->endScene();*/


  }

  //printf("\tLongest time: %d ms\n", time_longest);
  //printf("\tShortest time: %d ms\n", time_shortest);


  // Write image to file
  stringc resultFileName = "data/levels/";
  resultFileName += parameters.levelName;
  resultFileName += "/shadow_";
  resultFileName += Core->getObjects()->getNodeFullName(node->getName());
  resultFileName += ".png";
  Core->getRenderer()->getVideoDriver()->writeImageToFile(output, resultFileName.c_str());

  output->drop();

  printf("\tCompleted!\n");
}


void CObjectManager::loadNameMaterials()
{
  irr::core::stringc filename = "data/levels/materials.dat";

  //xml = irr::io::createIrrXMLReader(filename.c_str());
  irr::io::IXMLReader *xml =
    Core->getRenderer()->getDevice()->getFileSystem()->createXMLReader(filename.c_str());

  while(xml && xml->read())
  {
    if(xml->getNodeType() == irr::io::EXN_ELEMENT)
    {
      irr::core::stringc nodeName = xml->getNodeName();

      if(nodeName == "material")
      {
        SMaterialPair *matPair = new SMaterialPair();

        matPair->name = irr::core::stringc(xml->getAttributeValue(0));
        matPair->material = xml->getAttributeValueAsInt(1);

        nameMaterials.push_back(matPair);
      }
    }
  }

  xml->drop();
  //delete xml;

  return;
}


void CObjectManager::loadLevelSettings(const irr::c8* name)
{
  parameters.levelName = irr::core::stringc(name);

  irr::core::stringc filename = "data/levels/";
  filename += name;
  filename += "/level.dat";

  irr::io::IXMLReader *xml =
    Core->getRenderer()->getDevice()->getFileSystem()->createXMLReader(filename.c_str());

  printf("Loading level settings %s ... ", filename.c_str());

  while(xml && xml->read())
  {
    irr::core::stringc nodeElementName = irr::core::stringc(xml->getNodeName());

    if(xml->getNodeType() == irr::io::EXN_ELEMENT)
    {
      if(nodeElementName == "shadow-strength") {
        parameters.lightValues.X = xml->getAttributeValueAsFloat(0);
      }
      else if(nodeElementName == "sun-intensity") {
        parameters.lightValues.Y = xml->getAttributeValueAsFloat(0);
        Core->getRenderer()->getSceneManager()->setAmbientLight(irr::video::SColorf(parameters.lightValues.Y,parameters.lightValues.Y,parameters.lightValues.Y));
        Core->getRenderer()->getVideoDriver()->setAmbientLight(irr::video::SColorf(parameters.lightValues.Y,parameters.lightValues.Y,parameters.lightValues.Y));
      }
      else if(nodeElementName == "sky") {
        parameters.skyTextureFile = irr::core::stringc(xml->getAttributeValue(0));
        parameters.backgroundSkyColor.setRed(xml->getAttributeValueAsInt(0));
        parameters.backgroundSkyColor.setGreen(xml->getAttributeValueAsInt(1));
        parameters.backgroundSkyColor.setBlue(xml->getAttributeValueAsInt(2));
      }
      else if(nodeElementName == "fog") {
        parameters.fogEnabled = true;
        parameters.fogStrenght = xml->getAttributeValueAsFloat(4);
        parameters.fogColor.setRed(xml->getAttributeValueAsInt(1));
        parameters.fogColor.setGreen(xml->getAttributeValueAsInt(2));
        parameters.fogColor.setBlue(xml->getAttributeValueAsInt(3));
        parameters.fogColor.setAlpha(255);
      }
      else if(nodeElementName == "ambient-sound") {
        parameters.ambientSound = irr::core::stringc(xml->getAttributeValue(0));
      }
      else if(nodeElementName == "ambient") {
        parameters.ambientColor.setRed(xml->getAttributeValueAsInt(0));
        parameters.ambientColor.setGreen(xml->getAttributeValueAsInt(1));
        parameters.ambientColor.setBlue(xml->getAttributeValueAsInt(1));
        parameters.ambientColor.setAlpha(255);
      }
      else if(nodeElementName == "grass") {
        SGrassObject newGrassObject;
        newGrassObject.objectName = irr::core::stringc(xml->getAttributeValue(0));
        newGrassObject.generationSourceFileName = irr::core::stringc(xml->getAttributeValue(1));
        newGrassObject.patchSize = xml->getAttributeValueAsFloat(2);

        DISTANCE_BETWEEN_FOILAGE_GROUPS = newGrassObject.patchSize;

        parameters.grassObjects.push_back(newGrassObject);
      }
      else if(nodeElementName == "sound")
      {
        S3DSound newSnd;

        // default
        newSnd.attenuation = 3.6f;
        newSnd.strength = 0.9f;
        newSnd.range = 10.0f;

        for(irr::u16 eIdx=0; eIdx < xml->getAttributeCount(); ++eIdx)
        {
          irr::core::stringc attributeName = irr::core::stringc(xml->getAttributeName(eIdx));

          if(attributeName == "name")
            newSnd.name = irr::core::stringc(xml->getAttributeValue(eIdx));
          else if(attributeName == "file")
            newSnd.filename = irr::core::stringc(xml->getAttributeValue(eIdx));
          else if(attributeName == "range")
            newSnd.range = xml->getAttributeValueAsFloat(eIdx);
          else if(attributeName == "loop")
            newSnd.looped = xml->getAttributeValueAsInt(eIdx);
          else if(attributeName == "strength")
            newSnd.strength = xml->getAttributeValueAsFloat(eIdx);
          else if(attributeName == "attenuation")
            newSnd.attenuation = xml->getAttributeValueAsFloat(eIdx);
        }

        a_Sounds.push_back(newSnd);
      }
    }
  }

  printf("ok!\n");

  xml->drop();
  //delete xml;

  return;
}

void CObjectManager::loadLevel(irr::core::stringc levelFile)
{

#ifdef PHYSICS_IRR_ODE
  Core->getPhysics()->init(true);
#endif

  //
  // Set loading screen
  //

  //Game->getGUI()->enableLoadingScreen(game::ELS_LOAD_LEVEL, 0); // Loading level geom.

  stringc filePath = "data/levels/";
  filePath += levelFile;
  filePath += ".irr";

  a_AmbientSounds.set_used(0);
  a_Sounds.set_used(0);
  spawnTriggers[0].clear();
  spawnTriggers[1].clear();

  loadLevelSettings(levelFile.c_str());

  snow_level = false;

  if(levelFile == "g_m1") snow_level = true;

  printf("Loading level (%s) ... ", filePath.c_str());

  Core->getRenderer()->getVideoDriver()->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);
  Core->getRenderer()->getSceneManager()->loadScene(filePath.c_str());

  printf("ok!\n");

  #ifdef PHYSICS_IRR_NEWT
  Core->getPhysics()->init();
  #endif

  //Game->getGUI()->enableLoadingScreen(game::ELS_LOAD_LEVEL, 1); // Grouping

  scene::ISceneNode *noParent = Core->getRenderer()->getSceneManager()->getRootSceneNode();

	irr::core::array<scene::ISceneNode *> nodes;

  nodes.set_used(0);

	Core->getRenderer()->getSceneManager()->getSceneNodesFromType(scene::ESNT_ANY, nodes);

  for (u32 i=0; i < nodes.size(); ++i)
  {
    nodes[i]->setParam(0, 0);
    nodes[i]->setParam(3,-1);
    nodes[i]->setParam(4, 0);
  }

  levelMeshes.set_used(0);

  if(!Core->commandLineParameters.hasParam("-disable_groups"))
  {
    groupNodes();
  }

  //Game->getGUI()->enableLoadingScreen(game::ELS_LOAD_LEVEL, 2); // Creating physics bodies

  //
  // Get a list of all scene nodes
  //

  nodes.set_used(0);

	Core->getRenderer()->getSceneManager()->getSceneNodesFromType(scene::ESNT_ANY, nodes);

  printf("Creating physics bodies (%d) ... ", nodes.size());

	for (u32 i=0; i < nodes.size(); ++i)
	{
		scene::ISceneNode *node = nodes[i];

    if(node == Core->getRenderer()->getSceneManager()->getRootSceneNode())
      continue;

    if(node->getParam(4) == 1)
      continue;

    node->setParam(4, 1);

    //if(isAcceptableName(node->getName()) == false)
      //node->setParam(0, 0);

		switch(node->getType())
		{
  		case scene::ESNT_ANIMATED_MESH: break;

  		case scene::ESNT_MESH:
      {
        irr::core::stringc nodeName = getObjectSimpleName(node->getName());

        if(nodeName != "AutoDoor"
        && nodeName != "Door"
        && nodeName != "DoorMeshTop"
        && nodeName != "DoorMeshBottom")
        {
          findAndApplyShaderMaterials( (scene::IMeshSceneNode*)node );
          node->setAutomaticCulling(scene::EAC_FRUSTUM_BOX);

          addObject(node);

          if(Core->commandLineParameters.hasParam("-show_groups")) {
            if(strcmp(node->getName(), "Terrain") != 0)
              node->setDebugDataVisible(irr::scene::EDS_BBOX);
          }
        }
      }
      break;

  		case scene::ESNT_OCTREE:
    		findAndApplyShaderMaterials( (scene::IMeshSceneNode*)node );

        node->setAutomaticCulling(scene::EAC_FRUSTUM_BOX);

        addObject(node);
      break;

      case scene::ESNT_EMPTY:
      {
        irr::core::list<ISceneNode *> children = node->getChildren();
        irr::core::list<scene::ISceneNode*>::Iterator it;

        for (irr::core::list<ISceneNode*>::Iterator it = children.begin(); it != children.end(); ++it)
          nodes.push_back((*it));

        node->setAutomaticCulling(scene::EAC_FRUSTUM_BOX);

        // Get name
        stringc node_full_name = stringc(node->getName());
        stringc node_name;

        if(node_full_name.find(" ") == -1) node_name = node_full_name;
        else node_name = node_full_name.subString(0, node_full_name.find(" "));

        if(node_name == "AutoDoor")
          break;

        STriggerObject newTriggerObject;

        newTriggerObject.Position = node->getAbsolutePosition();

        s16 startIndex = node_full_name.find(" ");
        s16 endIndex = node_full_name.findNext(' ', startIndex+1);

        if(endIndex == -1)
          endIndex = node_full_name.size();

        stringc param1 = node_full_name.subString(startIndex+1, endIndex-startIndex-1);
        stringc param2 = node_full_name.subString(endIndex+1, node_full_name.size()-endIndex);

        if(node_name == "Sun") {
          parameters.sunPosition = newTriggerObject.Position;
          Core->getRenderer()->getSceneManager()->addToDeletionQueue(node);
          break;
        }
        else if(node_name == "Team1Spawnpoint") {
          newTriggerObject.Type = ETT_SPAWNPOINT;
          newTriggerObject.Team = game::E_TEAM1;
          Core->getRenderer()->getSceneManager()->addToDeletionQueue(node);
        }
        else if(node_name == "Team2Spawnpoint") {
          newTriggerObject.Type = ETT_SPAWNPOINT;
          newTriggerObject.Team = game::E_TEAM2;
          Core->getRenderer()->getSceneManager()->addToDeletionQueue(node);
        }
        else if(node_name == "Sound")
        {
          Core->getRenderer()->getSceneManager()->addToDeletionQueue(node);

          irr::s16 snd = getSoundIndexFromName(param1);

          if(snd != -1)
          {
            S3DSound newSnd = a_Sounds[snd];

            SAmbientSound amb;

//            amb.sound = Core->getSound()->getSound(newSnd.filename.c_str());
//            amb.sound->setIsPaused(true);
 //           amb.sound->setVolume(0.f);

  //          amb.range = newSnd.range;
  //          amb.position = newTriggerObject.Position;
 //           amb.attenuation = newSnd.attenuation;
 //           amb.strength = newSnd.strength;

            a_AmbientSounds.push_back(amb);

            /*Core->getSound()->playSound3D(
              newSnd.filename.c_str(),
              newTriggerObject.Position,
              newSnd.looped,
              newSnd.range,
              newSnd.strength,
              newSnd.attenuation,
              true);*/
          }
        }
        else
        {
          if(node_name == "Show")
          {
            newTriggerObject.Type = ETT_SHOW_NODE;

            if(param2 == "Spawn1" || param2 == "Spawn2")
              spawnTriggers[(param2 == "Spawn1")?0:1].push_back(triggerObjects.size());
            else
              newTriggerObject.Range = atof(param2.c_str());

            newTriggerObject.Node = Core->getRenderer()->getSceneManager()->getSceneNodeFromName(param1.c_str());
          }
          else if(node_name == "Hide")
          {
            newTriggerObject.Type = ETT_HIDE_NODE;

            if(param2 == "Spawn1" || param2 == "Spawn2")
              spawnTriggers[(param2 == "Spawn1")?0:1].push_back(triggerObjects.size());
            else
              newTriggerObject.Range = atof(param2.c_str());

            newTriggerObject.Node = Core->getRenderer()->getSceneManager()->getSceneNodeFromName(param1.c_str());
          }
          else
          {
            break;
          }
        }

        triggerObjects.push_back(newTriggerObject);
      }
      break;

  		default:
  		break;
		}

		node->setMaterialFlag(EMF_BILINEAR_FILTER, true);
		node->setMaterialFlag(EMF_TRILINEAR_FILTER, Core->getConfiguration()->getVideo()->isTrilinearFilter);
		node->setMaterialFlag(EMF_ANISOTROPIC_FILTER, Core->getConfiguration()->getVideo()->isAnistropicFilter);
    node->setMaterialFlag(EMF_WIREFRAME, false);

    if(node->getType() != ESNT_PARTICLE_SYSTEM)
      node->setMaterialFlag(EMF_FOG_ENABLE, parameters.fogEnabled);

    //
    // Is it a door?

    if(getObjectSimpleName(node->getName()) == "AutoDoor"
    || getObjectSimpleName(node->getName()) == "Door")
    {
      createDoor(node);
    }

    // Reset ..
    //node->setParam(0, 0);
	}

  printf("ok!\n");

  printf("\tStatic objects: %d\n", staticList.size());
  printf("\tDynamic objects: %d\n", dynamicList.size());



  if(Core->commandLineParameters.hasParam("-generate_lightmap"))
  {
    printf("Generating lightmap for terrain\n");

    calculateLightmapForNode((scene::IMeshSceneNode*)getSceneNodeFromName(getNodeFullName("Terrain").c_str()));
  }

  if(Core->commandLineParameters.hasParam("-generate_grass"))
  {
    printf("Generating grass ... \n");

    for(u32 go_id=0; go_id < parameters.grassObjects.size(); ++go_id)
    {
      irr::core::stringc gZipFile = "data/levels/";
      gZipFile += parameters.levelName;
      gZipFile += "/";
      gZipFile += parameters.grassObjects[go_id].objectName;
      gZipFile += ".grz";

      irr::core::stringc gFile = "data/levels/";
      gFile += parameters.levelName;
      gFile += "/";
      gFile += parameters.grassObjects[go_id].objectName;
      gFile += ".fgd";

      bool generate_g = true;

      if(Core->getRenderer()->getDevice()->getFileSystem()->existFile(gZipFile.c_str())
      || Core->getRenderer()->getDevice()->getFileSystem()->existFile(gFile.c_str()))
      {
        printf("\tGrass file for %s already exists\n\t\tOverwrite? (y/n): ",
          parameters.grassObjects[go_id].objectName.c_str());

        //char i[1];
        //gets(i);
        //if(strcmp(i, "y"))
          //generate_g = false;
      }

      if(generate_g)
      {
        printf("\tObject: %s, source: %s\n",
          parameters.grassObjects[go_id].objectName.c_str(),
          parameters.grassObjects[go_id].generationSourceFileName.c_str());

        printf("\t\tPlease wait ... ");
        generateGrassFile(parameters.grassObjects[go_id]);
        printf("ok!\n");
      }
    }

    printf("Grass generation finished!\n");
  }






  //Game->getGUI()->enableLoadingScreen(game::ELS_LOAD_LEVEL, 3); // Loading grass

  if(Core->commandLineParameters.hasParam("-disable_grass") == false)
  {
    printf("Loading grass ... \n");

    for(u32 go_id=0; go_id < parameters.grassObjects.size(); ++go_id)
    {
      irr::core::stringc gZipFile = "data/levels/";
      gZipFile += parameters.levelName;
      gZipFile += "/";
      gZipFile += parameters.grassObjects[go_id].objectName;
      gZipFile += ".grz";

      irr::core::stringc gFile = "data/levels/";
      gFile += parameters.levelName;
      gFile += "/";
      gFile += parameters.grassObjects[go_id].objectName;
      gFile += ".fgd";

      if(Core->getRenderer()->getDevice()->getFileSystem()->existFile(gZipFile.c_str()))
        Core->getRenderer()->getDevice()->getFileSystem()->addZipFileArchive(gZipFile.c_str());

      if(Core->getRenderer()->getDevice()->getFileSystem()->existFile(gFile.c_str()))
      {
        printf("\tObject: %s\n",
          parameters.grassObjects[go_id].objectName.c_str());

#ifdef GRASS_2
        loadGrassFromFile2(gFile.c_str());
#else
        loadGrassFromFile(gFile.c_str());
#endif
      }
      else
      {
        printf("\tERROR: Unable to open grass file %s\n", gFile.c_str());
      }

    }

    printf("Grass loaded!\n");
  }

  /*ISceneNode *oldTerrain = getSceneNodeFromName("Terrain");
  ISceneNode *newTerrain = getSceneNodeFromName("TerrainNew");

  if(newTerrain)
    newTerrain->setName("Terrain");

  if(oldTerrain)
  {
    Core->getRenderer()->getSceneManager()->addToDeletionQueue(oldTerrain);
  }*/

  //Game->getGUI()->enableLoadingScreen(game::ELS_LOAD_LEVEL, 4); // misc

  //
  // FOG

  if(parameters.fogEnabled)
  {
    Core->getRenderer()->getVideoDriver()->setFog(
      parameters.fogColor,             // Color
      EFT_FOG_EXP,                     // Type
      0, 0,                            // Start, end
      parameters.fogStrenght,          // Density
      true,                            // Pixel fog
      true                             // Range fog
    );
  }

  //
  // Ambient light

  Core->getRenderer()->getSceneManager()->setAmbientLight(parameters.ambientColor);

  if(Core->commandLineParameters.hasParam("-disable_sky") == false)
  {
    printf("Loading sky ... ");

    if(parameters.skyTextureFile != "")
    {
      irr::core::stringc skyTextureFilePath = "data/sky/";
      skyTextureFilePath += parameters.skyTextureFile;

      if(Core->getRenderer()->getDevice()->getFileSystem()->existFile(skyTextureFilePath.c_str()))
      {
        Core->getRenderer()->getSceneManager()->addSkyDomeSceneNode(
        Core->getRenderer()->getVideoDriver()->getTexture(skyTextureFilePath.c_str()),18,8,1.0f,1.17f,600.f);
      }
      else {
        printf("\tERROR: Unable to load sky texture %s\n", skyTextureFilePath.c_str());
      }

    }

    printf("ok!\n");
  }

    /*weatherParticleNode = Core->getRenderer()->getSceneManager()->addParticleSystemSceneNode(false);

    scene::IParticleEmitter* em = weatherParticleNode->createBoxEmitter(
    core::aabbox3d<f32>(-65,10,-90, 65,65,105),
    core::vector3df(0.001f, -0.0182f, 0.0f),
    390, 455,
    video::SColor(255,145,145,145), video::SColor(255,130,130,130),
    2600,2800, 25);

    weatherParticleNode->setEmitter(em);
    em->drop();

    weatherParticleNode->setScale(core::vector3df(1,1,1));
    weatherParticleNode->setParticleSize(core::dimension2d<f32>(1.1f, 1.1f));
    weatherParticleNode->setMaterialFlag(video::EMF_LIGHTING, true);
    weatherParticleNode->setMaterialFlag(video::EMF_FOG_ENABLE, true);
    weatherParticleNode->setMaterialTexture(0, Core->getRenderer()->getVideoDriver()->getTexture("data/particles/snowflake.png"));
    weatherParticleNode->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    weatherParticleNode->getMaterial(0).MaterialTypeParam = 0.25f;
    weatherParticleNode->getMaterial(0).DiffuseColor = SColor(255, 0, 0, 0);
    weatherParticleNode->getMaterial(0).EmissiveColor = SColor(255, 230, 230, 230);


    weatherParticleNode2 = Core->getRenderer()->getSceneManager()->addParticleSystemSceneNode(false);

    scene::IParticleEmitter* em2 = weatherParticleNode2->createBoxEmitter(
    core::aabbox3d<f32>(-65,10,-60, 65,65,65),
    core::vector3df(0.0f, -0.0175f, 0.026f),
    350, 380, video::SColor(255,145,145,145), video::SColor(255,130,130,130),
    2600,2800, 30);

    weatherParticleNode2->setEmitter(em2);
    em2->drop();

    weatherParticleNode2->setScale(core::vector3df(1,1,1));
    weatherParticleNode2->setParticleSize(core::dimension2d<f32>(1.15f, 1.15f));
    weatherParticleNode2->setMaterialFlag(video::EMF_LIGHTING, true);
    weatherParticleNode2->setMaterialFlag(video::EMF_FOG_ENABLE, true);
    weatherParticleNode2->setMaterialTexture(0, Core->getRenderer()->getVideoDriver()->getTexture("data/particles/snowflake2.png"));
    weatherParticleNode2->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    weatherParticleNode2->getMaterial(0).MaterialTypeParam = 0.25f;
    weatherParticleNode2->getMaterial(0).DiffuseColor = SColor(255, 0, 0, 0);
    weatherParticleNode2->getMaterial(0).EmissiveColor = SColor(255, 230, 230, 230);*/

  printf("Mesh-cache contains %d meshes .. ", Core->getRenderer()->getSceneManager()->getMeshCache()->getMeshCount());
  Core->getRenderer()->getSceneManager()->getMeshCache()->clearUnusedMeshes();
  printf("cleared!\n");

  printf("Texture count: %d\n",
    Core->getRenderer()->getVideoDriver()->getTextureCount());

  //
  // Done with loading
  //

  printf("All done!\n");





#ifdef SOUND_IRRKLANG
  // Start playing the ambient sound
  /*if(parameters.ambientSound != "")
   ambientSoundResource = Core->getSound()->getSoundResource2D(
      parameters.ambientSound.c_str(),
      true,
      0.59f);*/
#endif

#ifdef SOUND_CAUDIO
  // Start playing the ambient sound
  /*if(parameters.ambientSound != "")
  {
    ambientSoundSource = Core->getSound()->getSoundResource(
      parameters.ambientSound.c_str());
    //ambientSoundSource->play();
  }
  else
  {
    printf("!!! No ambient sound for level \"%s\"\n", parameters.levelName.c_str());
  }*/
#endif

  return;
}

void CObjectManager::checkDoorProximity(irr::core::vector3df position)
{
  for(irr::u16 dIdx=0; dIdx < doorList.size(); ++dIdx)
  {
    doorList[dIdx]->checkProximity(position);
  }
}

void CObjectManager::createDoor(irr::scene::ISceneNode *node)
{
  irr::scene::ISceneManager *SceneManager = Core->getRenderer()->getSceneManager();

  // Automatically opened door
  if(getObjectSimpleName(node->getName()) == "AutoDoor"
  && node->getType() == irr::scene::ESNT_EMPTY)
  {
    if(node->getChildren().size() == 2)
    {
      irr::scene::IMeshSceneNode *nodeTop =
        (irr::scene::IMeshSceneNode *)SceneManager->getSceneNodeFromName("DoorMeshTop", node);
      irr::scene::IMeshSceneNode *nodeBottom =
        (irr::scene::IMeshSceneNode *)SceneManager->getSceneNodeFromName("DoorMeshBottom", node);

      if(!nodeTop && !nodeBottom)
        return;

      matrix4 nodeTop_Transformation = nodeTop->getAbsoluteTransformation();
      matrix4 nodeBottom_Transformation = nodeBottom->getAbsoluteTransformation();

      nodeTop->setParent(SceneManager->getRootSceneNode());
      nodeBottom->setParent(SceneManager->getRootSceneNode());

      nodeTop->setScale( nodeTop_Transformation.getScale() );
      nodeTop->setPosition( nodeTop_Transformation.getTranslation() );
      nodeTop->setRotation( nodeTop_Transformation.getRotationDegrees() );

      nodeBottom->setScale( nodeBottom_Transformation.getScale() );
      nodeBottom->setPosition( nodeBottom_Transformation.getTranslation() );
      nodeBottom->setRotation( nodeBottom_Transformation.getRotationDegrees() );

      nodeTop->updateAbsolutePosition();
      nodeBottom->updateAbsolutePosition();


      CDoor * door = new CDoor(Core, EDT_DOUBLE_VERTICAL);

      physics::CBody *topBody = Core->getPhysics()->createBodyForNode(nodeTop, false);
      physics::CBody *bottomBody = Core->getPhysics()->createBodyForNode(nodeBottom, false);

      game::SObjectData *objectData = new game::SObjectData();
      objectData->container_id = doorList.size();
      objectData->element_id = 0;
      objectData->type = game::EOT_DOOR;

      topBody->setUserData(objectData);
      bottomBody->setUserData(objectData);

      // Top part of the door is added first
      door->addBody(topBody);
      door->addBody(bottomBody);

      // Calulate activation points on both side of the door
      irr::core::matrix4 matrixRot;
      matrixRot.setRotationDegrees(node->getRotation());

      irr::core::vector3df pos1 = irr::core::vector3df(0.f, 1.5f, 5.f);
      irr::core::vector3df pos2 = irr::core::vector3df(0.f, 1.5f,-5.f);

      matrixRot.rotateVect(pos1);
      matrixRot.rotateVect(pos2);

      pos1 += node->getPosition();
      pos2 += node->getPosition();

      door->addActivationPoint(pos1);
      door->addActivationPoint(pos2);

      //SceneManager->addCubeSceneNode(3.f, 0, -1, pos1);
      //SceneManager->addCubeSceneNode(3.f, 0, -1, pos2);

      // This door opens automatically
      door->setIsAutomatic(true);

      door->setDoorPositions(0, nodeTop->getPosition(), nodeTop->getPosition()+irr::core::vector3df(0, 2.6f, 0));
      door->setDoorPositions(1, nodeTop->getPosition(), nodeTop->getPosition()-irr::core::vector3df(0, 2.7f, 0));

      doorList.push_back(door);
    }
  }
  // Door with a single mesh
  else if(getObjectSimpleName(node->getName()) == "Door")
  {

  }
}

void CObjectManager::getSpawnpointsForTeam(irr::u16 &teamid, irr::core::array<SSpawnpoint> &out)
{
  for (u16 i=0; i < triggerObjects.size(); ++i)
  {
      if(triggerObjects[i].Type == ETT_SPAWNPOINT && triggerObjects[i].Team == teamid) {
        SSpawnpoint sp;
          sp.Position = triggerObjects[i].Position;
          sp.Direction = triggerObjects[i].Range; // We use it as direction value

          out.push_back(sp);
      }
  }
}




f32 hiddenObjectsUpdateTime = 99.f;
irr::core::vector3df grassUpdateLastPosition = NULLVECTOR, grassUpdateLastRotation = NULLVECTOR;

void CObjectManager::updateTriggers()
{
	if(!Core->getRenderer()->getSceneManager()->getActiveCamera())
    return;

	f32 closestDist=99999.f;

  irr::core::array<u32> closestObjects;
  closestObjects.set_used(0);

  vector3df campos = Core->getRenderer()->getSceneManager()->getActiveCamera()->getAbsolutePosition();

  for (u16 i=0; i < triggerObjects.size(); ++i) {
    if(triggerObjects[i].Type == ETT_SPAWNPOINT) continue;

    f32 dist = campos.getDistanceFrom(triggerObjects[i].Position);

    if(s32(dist) <= s32(closestDist) && s32(dist) <= s32(triggerObjects[i].Range)) {
      closestDist = dist;
      closestObjects.push_back(i);
    }
  }

  for (u16 i=0; i < closestObjects.size(); ++i)
  {
    // Make sure node pointer is set
    if(triggerObjects[closestObjects[i]].Node)
    {
      trigger(closestObjects[i]);
    }
  }

  closestObjects.clear();
}

void CObjectManager::triggerSpawn(irr::u8 teamIndex)
{
  for (u16 i=0; i < spawnTriggers[teamIndex].size(); ++i)
    trigger(spawnTriggers[teamIndex][i]);
}

void CObjectManager::trigger(irr::u16 triggerIndex)
{
  if(triggerObjects[triggerIndex].Type == ETT_HIDE_NODE) {
    triggerObjects[triggerIndex].Node->setVisible(false);
  }
  else if(triggerObjects[triggerIndex].Type == ETT_SHOW_NODE) {
    triggerObjects[triggerIndex].Node->setVisible(true);
  }
}

irr::scene::IMeshSceneNode * Plane = (IMeshSceneNode*) NULL;
vector3df planeTargetPos;

void CObjectManager::createPlaneOverflight(irr::core::vector3df targetPos)
{
  ISceneNode *planeStart1 = getSceneNodeFromName("Plane1");

  if(!planeStart1) return;

  targetPos.Y = planeStart1->getPosition().Y;

  vector3df heading = (targetPos - planeStart1->getPosition()).getHorizontalAngle();

  Plane = Core->getRenderer()->getSceneManager()->addMeshSceneNode(
    Core->getRenderer()->getSceneManager()->getMesh("data/vehicles/plane/plane.b3d"));
  Plane->setPosition( planeStart1->getPosition() );
  Plane->setRotation( heading );
  Plane->setMaterialFlag(EMF_LIGHTING, false);
  Plane->setMaterialFlag(video::EMF_FOG_ENABLE, parameters.fogEnabled);

  matrix4 mat;
  mat.setRotationDegrees(heading);
  vector3df targ = vector3df(0,0,1000);
  mat.transformVect(targ);

  planeTargetPos = targetPos + targ;

  printf("Plane created\n");
}

void CObjectManager::update(irr::f32 &time)
{

  /*
    Update door animations and similar
  */

  for(irr::u32 dIdx=0; dIdx < doorList.size(); ++dIdx)
    doorList[dIdx]->update();

  /*
    Does grass need regenerating?
  */

  if(currentFoilageGroup)
  {
    bool updateGrassMesh = false;

    irr::core::vector3df cameraPosition = Core->getCamera()->getNode()->getAbsolutePosition();
    irr::core::vector3df cameraRotation = Core->getCamera()->getNode()->getRotation();

    if(fabs(cameraRotation.Y - grassUpdateLastRotation.Y) >= 18)
      updateGrassMesh = true;
    else if(cameraPosition.getDistanceFrom(grassUpdateLastPosition) >= DISTANCE_BETWEEN_FOILAGE_GROUPS/4)
      updateGrassMesh = true;

    if(updateGrassMesh)
    {
      grassUpdateLastPosition = cameraPosition;
      grassUpdateLastRotation = cameraRotation;

#ifndef GRASS_2
      regenerateGrassMesh(cameraPosition);
#else

#endif
    }

  }


  /*
    Update ambient sounds
  */

  irr::core::vector3df listenerPosition = Core->getSound()->getListenerPosition();

  if(listenerPosition != NULLVECTOR)
  for(irr::u16 ambSnd=0; ambSnd < a_AmbientSounds.size(); ++ambSnd)
  {
    SAmbientSound ambientSnd = a_AmbientSounds[ambSnd];

    irr::f32 listenerDistFromSound = listenerPosition.getDistanceFrom(ambientSnd.position);
    irr::f32 maxRange = ((ambientSnd.range*4) / ambientSnd.attenuation);

    if(listenerDistFromSound > (ambientSnd.range + maxRange))
    {
//      ambientSnd.sound->setIsPaused(true);

      continue;
    }
    else
    {
      irr::f32 vol = 0.f;

      if(listenerDistFromSound <= ambientSnd.range)
      {
        vol = 1.0f;
      }
      else
      {
        listenerDistFromSound -= ambientSnd.range;
        vol = 1.f - (listenerDistFromSound / maxRange);
      }

//      ambientSnd.sound->setVolume(vol * ambientSnd.strength);

//      if(ambientSnd.sound->getIsPaused())
      {
//        ambientSnd.sound->setIsPaused(false);
//        ambientSnd.sound->setIsLooped(true);
      }
    }
  }






  /*
  Core->getRenderer()->getVideoDriver()->setTransform(video::ETS_WORLD, core::matrix4());
  for(irr::u32 i=0; i<f_groups.size(); ++i)
    Core->getRenderer()->getVideoDriver()->draw3DBox(f_groups[i]->mesh->getBoundingBox());
  */


  if(hiddenObjectsUpdateTime >= 0.5f)
  {
    hiddenObjectsUpdateTime = 0.f;
    updateTriggers();
  }
  else
  {
    hiddenObjectsUpdateTime += time;
  }

  if(Plane != NULL)
  {
    matrix4 mat;
    mat.setRotationDegrees(Plane->getRotation());

    vector3df forward = vector3df(0,0,1);

    mat.transformVect(forward);

    if(Plane->getPosition().getDistanceFrom(planeTargetPos) < 30)
    {
      Plane->remove();
      Plane = (IMeshSceneNode*) NULL;
      printf("Plane removed\n");
    }
    else
      Plane->setPosition( Plane->getPosition() + forward * 47 * time);
  }


#ifdef PHYSICS_NEWTON
#ifdef ENGINE_DEVELOPMENT_MODE
  /*for (u16 i=0; i < staticList.size(); ++i)
  {
    u32 bodycount = staticList[i]->getBodyCount();

    for (u16 j=0; j < bodycount; ++j)
      staticList[i]->getBodyByID(j)->drawDebug(Core->getRenderer()->getVideoDriver());
  }*/
#endif
#endif



  /*for (u16 i=0; i < Core->getPhysics()->characterCollisionObjects.size(); ++i) {
    Core->getPhysics()->characterCollisionObjects[i].body->drawDebugInfo();
  }*/



  /*if(terrainBody != NULL)
    terrainBody->drawDebugInfo();*/

  /*grassWave[0] += f32(Core->getMath()->getRandomInt(3,9)) * time * 0.42f;
  grassWave[1] += f32(Core->getMath()->getRandomInt(2,4)) * time * 0.33f;
  if(grassWave[0] > 360) grassWave[0] = 0.f;
  if(grassWave[1] > 360) grassWave[1] = 0.f;*/


  /*if(Core->GetInput()->isKeyPressedOnce(irr::KEY_KEY_B)) {
    CBatchingMesh* meshtest = (CBatchingMesh*)testNode->getMesh();

    irr::core::matrix4 m;
    m.setTranslation(irr::core::vector3df(1,0,0));

    meshtest->moveMesh(meshtest->getSourceMesh(0), m);
  }*/




  /*if(weatherParticleNode && weatherParticleNode2)
  {
    core::matrix4 matrix;

    vector3df cameraPos = Core->getRenderer()->getSceneManager()->getActiveCamera()->getPosition();
    vector3df heading = vector3df(0,10,50);

    vector3df cam_rot = vector3df(
      Core->getRenderer()->getSceneManager()->getActiveCamera()->getTarget() - cameraPos).getHorizontalAngle();
    cam_rot.X = cam_rot.Z = 0.f;

    matrix.setRotationDegrees(cam_rot);
    matrix.rotateVect(heading);

    weatherParticleNode->setRotation(matrix.getRotationDegrees());
    weatherParticleNode->setPosition(cameraPos+heading);

    weatherParticleNode2->setRotation(matrix.getRotationDegrees());
    weatherParticleNode2->setPosition(cameraPos+heading);
  }*/



  return;
}
