#include "Core.h"
#include "Physics.h"
#include "Renderer.h"

using namespace engine;

#ifdef PHYSICS_NEWTON
  using namespace engine::physics;
#endif

CPhysicsManager::~CPhysicsManager()
{
}

CPhysicsManager::CPhysicsManager(CCore* core) : Core(core)
{
#ifdef PHYSICS_IRR_NEWT
  PhysicsWorld = irr::newton::createPhysicsWorld(Device);
  PhysicsWorld->setSolverModel(irr::newton::EOM_PRECISE);
  PhysicsWorld->setFrictionModel(irr::newton::EOM_FAST);
  PhysicsWorld->setPlatformArchitecture(irr::newton::EUHM_MEDIUM);
  PhysicsWorld->setFrameRate(70);
  PhysicsWorld->setMinimumFrameRate(30);

  PhysicsCollisionManager = PhysicsWorld->getCollisionManager();

  gravity = irr::core::vector3df(0.f, -64.f, 0.f);
  upVector = irr::core::vector3df(0.f, 1.f, 0.f);
#endif

#ifdef PHYSICS_NEWTON

  PhysicsWorld = new CPhysicsWorld(Core->getRenderer()->getDevice()->getTimer());

  PhysicsWorld->createNewtonWorld();
#endif
}

void CPhysicsManager::clearMeshGroups()
{
  for(irr::u16 i=0; i < meshGroups.size(); ++i)
  {
    /*for(irr::u16 j=0; j < meshGroups[i].meshes.size(); ++j)
    {
      //Core->getRenderer()->getSceneManager()->getMeshCache()->removeMesh(meshGroups[i].meshes[j]);
      //meshGroups[i].meshes[j]->drop();
    }

    for(irr::u16 j=0; j < meshGroups[i].simple_collision_mesh.size(); ++j)
    {
      //Core->getRenderer()->getSceneManager()->getMeshCache()->removeMesh(meshGroups[i].simple_collision_mesh[j]);
      //meshGroups[i].simple_collision_mesh[j]->drop();
    }*/

    meshGroups[i].meshes.clear();
    meshGroups[i].meshes.set_used(0);

    meshGroups[i].simple_collision_mesh.clear();
    meshGroups[i].simple_collision_mesh.set_used(0);
  }

  meshGroups.clear();
  meshGroups.set_used(0);
}

#ifdef PHYSICS_NEWTON

void CPhysicsManager::initOnLevel()
{
  PhysicsWorld->clear();
}

void CPhysicsManager::update(
  irr::u32 time)
{
  PhysicsWorld->advanceSimulation(time*1000);
}

void CPhysicsManager::update2()
{
  //PhysicsWorld->advanceSimulation2();
  PhysicsWorld->advanceSimulation3(Core->time.delta);
}

void CPhysicsManager::close()
{
  PhysicsWorld->closeNewtonWorld();

  delete PhysicsWorld;
}

physics::CBody * CPhysicsManager::createCharacterBody(irr::scene::IAnimatedMeshSceneNode *node)
{
  SBodyCreationParameters bodyParameters;

  // Standing collision parameters
  bodyParameters.node = node;
  bodyParameters.type = EBT_PRIMITIVE_CAPSULE;
  bodyParameters.mass = 50.0f;
  bodyParameters.scale = node->getScale();
  bodyParameters.bodyID = PhysicsWorld->getUniqueBodyID();

  CBody * charBody = PhysicsWorld->createBody(bodyParameters);

  charBody->setGravityEnabled(false);
  charBody->createUpVectorConstraint(irr::core::vector3df(0,1,0));
  charBody->setContinuousCollisionMode(true);

  //
  // Now let's create two more collisions. One for crouching and one for lying down.

  // Crouching
  bodyParameters.scale = node->getScale();
  bodyParameters.scale.Y *= 0.5f;

  NewtonCollision * collision1 = PhysicsWorld->createCollisionFromBodyParameters(bodyParameters);

  // Lying down
  bodyParameters.scale = node->getScale();
  bodyParameters.scale.Z *= 7.7f;
  bodyParameters.scale.Y *= 0.35f;
  bodyParameters.offset.set(0,0, -0.086f);
  bodyParameters.type = EBT_PRIMITIVE_SPHERE;

  NewtonCollision * collision2 = PhysicsWorld->createCollisionFromBodyParameters(bodyParameters);

  charBody->addCollision(collision1);
  charBody->addCollision(collision2);

  return charBody;
}




physics::CBody * CPhysicsManager::createBodyForNode(
  irr::scene::ISceneNode* node,
  bool dynamic,
  irr::f32 mass)
{
  physics::CBody * body;

  SBodyCreationParameters bodyParameters;

  bodyParameters.node = node;
  bodyParameters.mesh = ((irr::scene::IMeshSceneNode*)node)->getMesh();
  bodyParameters.bodyID = PhysicsWorld->getUniqueBodyID();

  if(dynamic)
    bodyParameters.type = engine::physics::EBT_CONVEX_HULL;
  else
    bodyParameters.type = engine::physics::EBT_TREE_COLLISION;

  bodyParameters.mass = mass;
  bodyParameters.scale = node->getScale();

  body = PhysicsWorld->createBody(bodyParameters);

  // Release collision
  PhysicsWorld->getCollisionManager()->releaseCollision(
    PhysicsWorld->getNewtonWorld(),
    body->getCurrentCollision());

  return body;
}




irr::core::array<CBody*> CPhysicsManager::createStaticPhysics(
  irr::scene::IMeshSceneNode* node)
{
  irr::core::array<physics::CBody*> bodies;
  bodies.set_used(0);

	if(node == NULL)
    return bodies;

  irr::core::vector3df rotation = node->getRotation();
  node->setRotation(NULLVECTOR);

  SBodyCreationParameters bodyParameters;

  bodyParameters.node = node;
  bodyParameters.type = engine::physics::EBT_TREE_COLLISION;
  bodyParameters.mass = 0.f;
  //bodyParameters.scale = node->getScale();

  // Single object, does not belong to any mesh group
  if(node->getParam(0) == 0)
  {
    irr::scene::IMesh *mesh = getPhysicsMesh(node);

    bodyParameters.mesh = mesh;
    bodyParameters.bodyID = PhysicsWorld->getUniqueBodyID();

    CBody * body = PhysicsWorld->createBody(bodyParameters);

    PhysicsWorld->getCollisionManager()->releaseCollision(
      PhysicsWorld->getNewtonWorld(),
      body->getCurrentCollision());

    bodies.push_back(body);
  }
  // Multi-body object (group), contains several nodes
  else
  {
    SPhysicsMesh meshGroup = meshGroups[node->getParam(3)];

    for(irr::u32 i=0; i < meshGroup.meshes.size(); ++i)
    {
      bodyParameters.mesh = meshGroup.meshes[i];
      bodyParameters.bodyID = PhysicsWorld->getUniqueBodyID();

      CBody * body = PhysicsWorld->createBody(bodyParameters);

      PhysicsWorld->getCollisionManager()->releaseCollision(
        PhysicsWorld->getNewtonWorld(),
        body->getCurrentCollision());

      bodies.push_back(body);
    }
  }

  for(irr::u32 i=0; i < bodies.size(); ++i)
  {
    bodies[i]->setRotation(rotation);
    //bodies[i]->setPosition(position);
  }

  return bodies;
}

void CPhysicsManager::createTestObject(irr::u32 type, irr::core::vector3df position)
{
#ifdef ENGINE_DEVELOPMENT_MODE
  // Create a cube
  if(type == 0)
  {
    irr::scene::ISceneNode *falling_cube_node =
      Core->getRenderer()->getSceneManager()->addCubeSceneNode(1.0f);
    falling_cube_node->setScale(irr::core::vector3df(4.f, 4.f, 4.f));
    falling_cube_node->setMaterialTexture(0,
      Core->getRenderer()->getVideoDriver()->getTexture("data/2d/terminal/buy_heavytank.png"));
    falling_cube_node->setMaterialFlag(irr::video::EMF_LIGHTING, false);

    SBodyCreationParameters box_params;

    box_params.node = falling_cube_node;
    box_params.type = engine::physics::EBT_PRIMITIVE_BOX;
    box_params.scale = falling_cube_node->getScale();
    box_params.mass = 10.f;
    box_params.bodyID = PhysicsWorld->getUniqueBodyID();

    CBody * cubeBody = PhysicsWorld->createBody(box_params);

    cubeBody->setPosition(position);
  }
#endif
}

void CPhysicsManager::clear()
{
  PhysicsWorld->clear();
}


#endif





#ifdef PHYSICS_IRR_ODE

bool CRayEventListener::onEvent(IIrrOdeEvent *pEvent)
{
  CIrrOdeEventRayHit *pEvtRay = (CIrrOdeEventRayHit*)pEvent;

  CIrrOdeGeomRay *pRay = pEvtRay->getRay();

  //pRay->setHit(pEvtRay->getBody(), pEvtRay->getGeom(), pEvtRay->getPosition());

  //if(Core->GetInput()->isKeyPressedOnce(irr::KEY_KEY_L))
  //{
    /*Core->getRenderer()->getSceneManager()->addCubeSceneNode(2.0f, 0,-1, pEvtRay->getPosition());
    printf("cube at %.3f %.3f %.3f\n",
    pEvtRay->getPosition().X,
    pEvtRay->getPosition().Y,
    pEvtRay->getPosition().Z);*/
  //}


  //Game->getGUI()->getCrosshairObject()->shown = true;
  Game->getGUI()->getCrosshairObject()->name = irr::core::stringc(pEvtRay->getDistance());
  //Game->getGUI()->getCrosshairObject()->hit_position = pEvtRay->getPosition();

  return true;
}

bool CRayEventListener::handlesEvent(IIrrOdeEvent *pEvent)
{
  if(pEvent->getType() == eIrrOdeEventRayHit)
  {
    CIrrOdeEventRayHit *pEvtRay = (CIrrOdeEventRayHit*)pEvent;

    /*if(pEvtRay->getBody())
    {
      if(pEvtRay->getBody()->getID() ==
        Game->getCharacters()->getPlayer()->getBody()->PhysicsBody->getID())
        {
          printf("pl ");
          return false;
        }
    }*/

    return true;
  }
  else
  {
    return false;
  }
}

irr::s32 getNextId()
{
  static irr::s32 id=0;
  return ++id;
}


void CPhysicsManager::createPhysicsGeometry(irr::scene::IMeshSceneNode* node)
{
  irr::scene::ISceneManager *SceneManager = Core->getRenderer()->getSceneManager();

  /*CIrrOdeBody *pBody = reinterpret_cast<CIrrOdeBody *>
    (SceneManager->addSceneNode(
      CIrrOdeSceneNode::nodeNameToC8(IRR_ODE_BODY_NAME),
      worldNode));

  pBody->setPosition(node->getPosition());
  node->setParent(pBody);
  node->setPosition(irr::core::vector3df(0,0,0));*/

  node->setParent(worldNode);

  CIrrOdeGeomTrimesh *pTri =
    reinterpret_cast<CIrrOdeGeomTrimesh *>
    (SceneManager->addSceneNode(
      CIrrOdeSceneNode::nodeNameToC8(IRR_ODE_GEOM_TRIMESH_NAME),
      node));

  pTri->setID(getNextId());

  /*if(node->getParam(0) != 0)
  {
    irr::scene::IMesh *mesh = meshGroups[node->getParam(3)].meshes[0];

    if(!mesh)
      printf("Not mesh!\n");

    irr::scene::ISceneNode *pChildNode = SceneManager->addMeshSceneNode(
      node->getMesh(),
      pTri);

    if (pChildNode) {
      pChildNode->setMaterialFlag(EMF_LIGHTING,false);
      pChildNode->setMaterialFlag(EMF_WIREFRAME,true);
    }
  }*/


  for (irr::u32 j=0; j < pTri->getSurfaceParametersCount(); j++) {
    pTri->getSurfaceParameters(j)->setBounce(0.9f);
    pTri->getSurfaceParameters(j)->setModeBounce(true);
  }

  //pTri->setMassTotal(1000.0f);
  pTri->drop();

  worldNode->addGeom(pTri);

  //if(pTri->getBody())
    //printf("BODY EXISTS!\n");

  //printf("GEOM ID: %d\n", pTri->getID());

  //pBody->addGeom(pTri);
  //pBody->initPhysics();





  /*pBody->setMaxAngularSpeed(0.f);
  pBody->setGravityMode(0);
  pBody->drop();

  static_bodies.push_back(pBody);*/
}

irr::ode::CIrrOdeBody *CPhysicsManager::createPlayerPhysics(irr::scene::ISceneNode *node)
{
  irr::scene::ISceneManager *SceneManager = Core->getRenderer()->getSceneManager();

  CIrrOdeBody *pBody = reinterpret_cast<CIrrOdeBody *>(
    SceneManager->addSceneNode(
      CIrrOdeSceneNode::nodeNameToC8(IRR_ODE_BODY_NAME),
      worldNode));

  pBody->setID(getNextId());

  node->setParent(pBody);

  printf("Player ID: %d\n", pBody->getID());

  CIrrOdeGeomSphere *pSphere = reinterpret_cast<CIrrOdeGeomSphere *>(
    SceneManager->addSceneNode(
      CIrrOdeSceneNode::nodeNameToC8(IRR_ODE_GEOM_SPHERE_NAME),
      node));
  pSphere->setMassTotal(50.0f);
  pSphere->setMassTranslation(irr::core::vector3df(0,-50,0));
  pSphere->drop();

  pBody->initPhysics();
  pBody->drop();

  return pBody;
}



CIrrOdeGeomRay *CPhysicsManager::createRay()
{
  irr::scene::ISceneManager *SceneManager = Core->getRenderer()->getSceneManager();

  irr::scene::ISceneNode *pNode = SceneManager->addSceneNode(
    CIrrOdeSceneNode::nodeNameToC8(IRR_ODE_GEOM_RAY_NAME),
    worldNode);

  CIrrOdeGeomRay *pRay = reinterpret_cast<CIrrOdeGeomRay*>(pNode);

  pRay->set(irr::core::vector3df(0,0,0), irr::core::vector3df(0,0,0), 0.f);
  pRay->setID(getNextId());
  pRay->initPhysics();
  pRay->setMassTotal(10.0f);

  //pRay->reset();

  pRay->drop();

  return pRay;
}

void CPhysicsManager::createTestSphere(irr::core::vector3df position)
{
  irr::scene::ISceneManager *SceneManager = Core->getRenderer()->getSceneManager();


  //first add a body as child of the worldNode
  CIrrOdeBody *pBody=(CIrrOdeBody *)
    SceneManager->addSceneNode(CIrrOdeSceneNode::nodeNameToC8(IRR_ODE_BODY_NAME),worldNode);
  pBody->setPosition(position);
  //next load a mesh and add an AnimatedMeshSceneNode
  //as child of the body
  irr::scene::IAnimatedMesh *Mesh=SceneManager->getMesh("data/sphere.3ds");
  IAnimatedMeshSceneNode *Node=SceneManager->addAnimatedMeshSceneNode(Mesh,pBody);
  Node->setMaterialTexture(0,Core->getRenderer()->getVideoDriver()->getTexture("data/sphere1.jpg"));
  Node->setMaterialFlag(EMF_LIGHTING,false);

  //as the last part we add a sphere geom as child of the
  //AnimatedMeshSceneNode
  CIrrOdeGeomSphere *pSphere=(CIrrOdeGeomSphere*)SceneManager->addSceneNode(CIrrOdeSceneNode::nodeNameToC8(IRR_ODE_GEOM_SPHERE_NAME),Node);
  pSphere->setMassTotal(10.5f);
  pSphere->getSurfaceParameters(0)->setBounce(0.8f);
  pSphere->getSurfaceParameters(0)->setModeBounce(true);
  pSphere->drop();

  pBody->initPhysics();
  pBody->drop();
}

void CPhysicsManager::update()
{
  CIrrOdeManager::getSharedInstance()->step();

  for(irr::u16 sbi=0; sbi < static_bodies.size(); ++sbi)
  {
    //static_bodies[sbi]->setRotation(NULLVECTOR);
    //static_bodies[sbi]->setPosition(NULLVECTOR);
  }

}

void CPhysicsManager::quickStep()
{
  CIrrOdeManager::getSharedInstance()->quickStep();
}

void CPhysicsManager::clear()
{
  CIrrOdeManager::getSharedInstance()->clearODE();
  static_bodies.clear();
}

void CPhysicsManager::close()
{
  clear();

  CIrrOdeManager::getSharedInstance()->closeODE();

  delete cFactory;
}

void CPhysicsManager::init(bool create_world)
{
  if(create_world)
  {
    ISceneNode *pNode = Core->getRenderer()->getSceneManager()->addSceneNode(
      CIrrOdeSceneNode::nodeNameToC8(IRR_ODE_WORLD_NAME),
      Core->getRenderer()->getSceneManager()->getRootSceneNode());

    worldNode = (CIrrOdeWorld*)pNode;

    worldNode->setGravity(vector3df(0.0f,-16.0f,0.0f));
  }
  else
  {
    worldNode->initPhysics();
  }
}

#endif




#ifdef PHYSICS_IRR_NEWT

void CPhysicsManager::init()
{
  playerMaterial = PhysicsWorld->createMaterial();
  worldGeomMaterial = PhysicsWorld->createMaterial();

  playerMaterial->setFriction(worldGeomMaterial, 0.052f, 0.015f);

  characterCollisionObjects.clear();

  character_ignore.clear();
  character_ignore.set_used(0);
}


//
// Create physics for PLAYER
//

SCharacterPhysicsResult CPhysicsManager::createPlayerPhysics(
  irr::scene::IAnimatedMeshSceneNode *node)
{
  SCharacterPhysicsResult out;

  irr::newton::SBodyFromNode objectData;

  objectData.Node = node;
  objectData.Type = irr::newton::EBT_CONVEX_HULL;

  objectData.Mesh = Core->getRenderer()->getSceneManager()->getMesh("data/chars/player_collision.b3d");
  out.StandingCollision = PhysicsWorld->createCollision(objectData);

  objectData.Mesh = Core->getRenderer()->getSceneManager()->getMesh("data/chars/player_collision_crouch.b3d");
  out.CrouchingCollision = PhysicsWorld->createCollision(objectData);

  objectData.Mesh = Core->getRenderer()->getSceneManager()->getMesh("data/chars/player_collision_lying.b3d");
  out.LyingCollision = PhysicsWorld->createCollision(objectData);

  objectData.Mesh = Core->getRenderer()->getSceneManager()->getMesh("data/assets/empty.ms3d");
  out.EmptyCollision = PhysicsWorld->createCollision(objectData);

  return out;
}


irr::newton::IBody* CPhysicsManager::createBotPhysics(
  irr::scene::IAnimatedMeshSceneNode *node)
{
  irr::newton::IBody* resultBody;

  irr::newton::SBodyFromNode objectData;
    objectData.Node = node;
    objectData.Type = irr::newton::EBT_PRIMITIVE_CAPSULE;

  resultBody = PhysicsWorld->createBody(objectData);

  return resultBody;
}

irr::core::array<irr::newton::IBody*> CPhysicsManager::CreateStaticPhysics(
  irr::scene::IMeshSceneNode* node)
{
	irr::core::array<irr::newton::ICollision*> collisions;
  irr::core::array<irr::newton::IBody*> bodies;
  bodies.set_used(0);

  // 0 bodies
	if(node == NULL) return bodies;

  irr::core::vector3df rotation = node->getRotation();

  node->setRotation(irr::core::vector3df(0,0,0));

  irr::newton::SBodyFromNode objectData;
    objectData.Node = node;

  bool dynamicObject = Core->getObjects()->isNodeParameterSet(node, "d");

  if(dynamicObject == true)
  {
    objectData.Type = irr::newton::EBT_CONVEX_HULL;
    objectData.Mass = 40;
    //objectData.CalculateOffsetMode = irr::newton::ECO_MESH;
  }
  else
    objectData.Type = irr::newton::EBT_TREE;

  irr::scene::IMeshManipulator *meshManipulator = Core->getRenderer()->getSceneManager()->getMeshManipulator();

  if(node->getParam(0) == 0)
  {
    irr::scene::IMesh *mesh = getPhysicsMesh(node);
    //meshManipulator->recalculateNormals(mesh, false);

    objectData.Mesh = mesh;

    bodies.push_back( PhysicsWorld->createBody(objectData) );

    //
    // Check if simple collision mesh for characters and such exists.
    //

    irr::scene::IMesh *simple_collision_mesh = (irr::scene::IMesh*) NULL;

    /*if(getPhysicsMeshSimple(node, simple_collision_mesh))
    {
      objectData.Mesh = simple_collision_mesh;
      irr::newton::IBody *simple_body = PhysicsWorld->createBody(objectData);
      simple_body->setRotation(rotation);
      character_collisions.push_back(simple_body->getNewtonBody());
      character_ignore.push_back(bodies.getLast()->getNewtonBody());
    }*/

    mesh->drop();
  }
  else
  {
    SPhysicsMesh meshGroup = meshGroups[node->getParam(3)];

    for(irr::u32 i=0; i < meshGroup.meshes.size(); ++i)
    {
      objectData.Mesh = meshGroup.meshes[i];

      collisions.push_back(PhysicsWorld->createCollision(objectData));

      if(meshGroup.simple_collision_mesh[i] != NULL)
      {
        objectData.Mesh = meshGroup.simple_collision_mesh[i];

        SCharacterCollisionObject cco;

        cco.collision = PhysicsWorld->createCollision(objectData);
        objectData.Mesh = Core->getRenderer()->getSceneManager()->getMesh("data/assets/empty.ms3d");
        cco.empty_collision = PhysicsWorld->createCollision(objectData);
        cco.body = PhysicsWorld->createBody(node, cco.collision);

        //cco.body->setRotation(rotation);

        characterCollisionObjects.push_back(cco);
      }
    }

    for(irr::u16 c = 0; c < collisions.size(); ++c) {
      bodies.push_back(PhysicsWorld->createBody(node, collisions[c]));
      collisions[c]->release();
    }
  }



  if(dynamicObject == true) {
    for(irr::u16 b = 0; b < bodies.size(); ++b) {
      bodies[b]->addForceContinuous(getGravity());
    }
  }

  for(irr::u16 b = 0; b < bodies.size(); ++b) {
      bodies[b]->setRotation(rotation);
      bodies[b]->setMaterial(worldGeomMaterial);
  }

  return bodies;
}

irr::newton::SIntersectionPoint CPhysicsManager::GetCollisionFromLine(
  irr::newton::IBody* body,
  irr::core::line3df &line)
{
  irr::newton::SIntersectionPoint output;

  PhysicsCollisionManager->getCollisionPoint(body, line, output);

  return output;
}

irr::newton::SIntersectionPoint CPhysicsManager::GetCollisionFromLine(irr::core::line3df &line)
{
  irr::newton::SIntersectionPoint output;

  output = PhysicsCollisionManager->getCollisionFirstPoint(line);

  return output;
}

irr::newton::SIntersectionPoint CPhysicsManager::GetCollisionFromLineEx(irr::core::line3df &line)
{
  irr::newton::SIntersectionPoint output;

  output = PhysicsCollisionManager->getCollisionFirstPointEx(line);

  return output;
}

irr::newton::SIntersectionPoint CPhysicsManager::GetCollisionFromLine(
  irr::core::line3df &line,
  irr::core::array<NewtonBody*> &exclude)
{
  irr::newton::SIntersectionPoint output;

  output = PhysicsCollisionManager->getCollisionFirstPointEx(line, exclude);

  return output;
}

void CPhysicsManager::removeCharacterCollisionObjectsFromWorld()
{
  for(irr::u16 char_collision_idx=0; char_collision_idx < characterCollisionObjects.size(); ++char_collision_idx)
    characterCollisionObjects[char_collision_idx].body->setCollision(
      characterCollisionObjects[char_collision_idx].empty_collision);
}

void CPhysicsManager::restoreCharacterCollisionObjects()
{
  for(irr::u16 char_collision_idx=0; char_collision_idx < characterCollisionObjects.size(); ++char_collision_idx)
    characterCollisionObjects[char_collision_idx].body->setCollision(
      characterCollisionObjects[char_collision_idx].collision);
}
#endif


irr::scene::IMesh *CPhysicsManager::getPhysicsMeshSimple(irr::scene::ISceneNode* node)
{
  // Get mesh name
  irr::core::stringc meshFile =
    Core->getRenderer()->getSceneManager()->getMeshCache()->getMeshName(
      ((irr::scene::IMeshSceneNode*)node)->getMesh()).getInternalName();

  irr::s32 fileExtensionStartIndex = meshFile.findLast('.');
  irr::core::stringc fileExtension = meshFile.subString(fileExtensionStartIndex, meshFile.size()-fileExtensionStartIndex);

  irr::core::stringc physcisSimpleMeshFile = meshFile.subString(0, fileExtensionStartIndex);
  physcisSimpleMeshFile += "_w";
  physcisSimpleMeshFile += fileExtension;

  if(Core->getRenderer()->getDevice()->getFileSystem()->existFile(physcisSimpleMeshFile))
  {
    irr::scene::IMesh*mesh = Core->getRenderer()->getDevice()->getSceneManager()->getMesh(physcisSimpleMeshFile.c_str());

    mesh = Core->getRenderer()->getSceneManager()->getMeshManipulator()->createMeshCopy(mesh);

    return mesh;
  }

  return NULL;
}

irr::scene::IMesh* CPhysicsManager::getPhysicsMesh(irr::scene::ISceneNode* node)
{
  irr::scene::IMesh *physicsMesh = ((irr::scene::IMeshSceneNode*)node)->getMesh();

  irr::core::stringc physcisMeshFile, physcisNoMeshFile;

  // Irrlict 1.7
  irr::core::stringc meshFile =
    Core->getRenderer()->getSceneManager()->getMeshCache()->getMeshName(physicsMesh).getInternalName();

  // For earlier versions ..
  /*irr::core::stringc meshFile =
    Core->getRenderer()->getSceneManager()->getMeshCache()->getMeshFilename(physicsMesh);*/

  irr::s32 fileExtensionStartIndex = meshFile.findLast('.');
  irr::core::stringc fileExtension = meshFile.subString(fileExtensionStartIndex, meshFile.size()-fileExtensionStartIndex);

  physcisMeshFile = meshFile.subString(0, fileExtensionStartIndex);
  physcisMeshFile += "_p";
  physcisMeshFile += fileExtension;

  physcisNoMeshFile = meshFile.subString(0, fileExtensionStartIndex);
  physcisNoMeshFile += "_x";
  physcisNoMeshFile += fileExtension;

  if(Core->getRenderer()->getDevice()->getFileSystem()->existFile(physcisNoMeshFile))
  {
    return (irr::scene::IMesh*)NULL;
  }


  if(fileExtensionStartIndex == -1)
  {
    printf("no-mesh for %s / Mesh file: %s\n", node->getName(), meshFile.c_str());
    if(node->getType() == irr::scene::ESNT_MESH)
      printf("is SNT_MESH\n");
    else
      printf("Error: is not ESNT_MESH !!!!\n");
  }

  if(Core->getRenderer()->getDevice()->getFileSystem()->existFile(physcisMeshFile))
  {
    physicsMesh = Core->getRenderer()->getDevice()->getSceneManager()->getMesh(physcisMeshFile.c_str());
  }
  else
  {
    //CBatchingMesh *msh = (CBatchingMesh*)physicsMesh;
    //printf("Size: %d\n", msh->getSourceBufferCount());
  }

  physicsMesh = Core->getRenderer()->getSceneManager()->getMeshManipulator()->createMeshCopy(physicsMesh);

  return physicsMesh;
}

