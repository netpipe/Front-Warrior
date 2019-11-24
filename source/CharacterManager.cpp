#include "Game.h"
#include "BaseCharacter.h"
#include "CharacterManager.h"
#include "Player.h"
#include "Bot.h"

#include "Renderer.h"
#include "ObjectManager.h"
#include "Maths.h"

using namespace game;

// Called when app's closing
void CCharacterManager::clearAll()
{
  // Delete all pointers created with "new" previously
  for(irr::u32 i=0; i < bots.size(); ++i)
  {
    //bots[i]->remove();
    delete bots[i];
  }

  if(m_Player)
  {
    //m_Player->remove();
    delete m_Player;
  }

  // NULL pointer
  m_Player = (CPlayer*)NULL;

  // Clear the array
  bots.clear();

  for(irr::u16 i=0; i< cClassParameters.size(); ++i)
    delete cClassParameters[i];

  cClassParameters.clear();
}

void CCharacterManager::loadExternalCharacterClassData()
{
  irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader("data/classes.dat");
  bool elementOpen = false;
  cClassParameters.set_used(0);
  SCharacterClassParameters* c_class;

  while(xml && xml->read())
  {
    switch(xml->getNodeType())
    {
      case irr::io::EXN_ELEMENT:
      {
        if(!strcmp("character-class", xml->getNodeName()) && !elementOpen) {
          elementOpen = true;
          c_class = new SCharacterClassParameters();

          c_class->class_name = irr::core::stringc(xml->getAttributeValue("name"));
        }
        else if(!strcmp("health", xml->getNodeName()))
          c_class->health = xml->getAttributeValueAsFloat("value");
        else if(!strcmp("stamina", xml->getNodeName()))
          c_class->stamina = xml->getAttributeValueAsFloat("value");
        else if(!strcmp("move-speed", xml->getNodeName()))
          c_class->move_speed = xml->getAttributeValueAsFloat("value");
      }

      case irr::io::EXN_ELEMENT_END:
      {
        if(!strcmp("character-class", xml->getNodeName()) && elementOpen) {
          elementOpen = false;
          cClassParameters.push_back(c_class);
        }
      }
    }
  }

  delete xml;
}

CPlayer * CCharacterManager::createPlayer(engine::SCharacterCreationParameters parameters)
{
  CPlayer * Player = new CPlayer(Game, parameters);

  irr::scene::ISceneManager *SceneManager = Game->getCore()->getRenderer()->getSceneManager();

  //
  // Create graphical node
  //

  /*if(!playerCharacter)
  {
    body.Node = SceneManager->addAnimatedMeshSceneNode(
      SceneManager->getMesh("data/chars/male/soldier.ms3d"),
      0, -1);
  }
  else
  {*/

  Player->getBody()->Node = SceneManager->addAnimatedMeshSceneNode(
    SceneManager->getMesh("data/chars/empty.b3d"),
    0, -1);


  Player->getBody()->Node->setScale(irr::core::vector3df(1,1,1));
  Player->getBody()->Node->setFrameLoop(0, 60);
  Player->getBody()->Node->setMaterialFlag(irr::video::EMF_BACK_FACE_CULLING, true);

  //game::SObjectData characterIdentifier;

  /*if(playerCharacter)
  {
    characterIdentifier.type = game::EOT_PLAYER;
  }
  else
  {
    characterIdentifier.type = game::EOT_BOT;
    characterIdentifier.container_id = Core->getObjects()->getCharacters()->getBotCount()+1;
  }*/

  //
  // Create physical body
  //

#ifdef PHYSICS_NEWTON
  Player->getBody()->PhysicsBody = Game->getCore()->getPhysics()->createCharacterBody(
    Player->getBody()->Node);
#endif

  Player->rotationNode = SceneManager->addEmptySceneNode(Player->getBody()->Node, 0);
  Player->rotationNode->setPosition(irr::core::vector3df(0,0,10));

  m_Player = Player;

  return Player;
}

CBot * CCharacterManager::createBot(engine::SCharacterCreationParameters parameters)
{
  CBot *bot = new CBot(parameters);


  bots.push_back(bot);

  return bot;
}

// Call this method to spawn a character at a random spawn point
void CCharacterManager::spawn(engine::CBaseCharacter* spawned)
{
  // Get a list of all spawnpoints for TeamID
  irr::core::array<engine::SSpawnpoint> spawnpoints;
  Game->getCore()->getObjects()->getSpawnpointsForTeam(spawned->getParameters()->TeamID, spawnpoints);

  // Pick a random point
  irr::u16 randomSpawnpoint = Game->getCore()->getMath()->getRandomInt(0, spawnpoints.size()-1);

  // Get a struct of all parameters for given character class
  SCharacterClassParameters *c_class_params = cClassParameters[spawned->getParameters()->Class];

  // Set values
  spawned->getParameters()->HealthMax = c_class_params->health;
  spawned->getParameters()->StaminaMax = c_class_params->stamina;

  // Default values are maxed out
  spawned->getParameters()->Health = spawned->getParameters()->HealthMax;
  spawned->getParameters()->Stamina = spawned->getParameters()->StaminaMax;

  spawned->init();

  // Set the body position
#ifdef PHYSICS_IRR_NEWT
  spawned->getBody()->PhysicsBody->setPosition(spawnpoints[randomSpawnpoint].Position);
  spawned->getBody()->PhysicsBody->addForceContinuous(irr::core::vector3df(0,0,0));
  spawned->getBody()->PhysicsBody->setForce(irr::core::vector3df(0,0,0));
#endif

#ifdef PHYSICS_NEWTON
  spawned->getBody()->PhysicsBody->setPosition(spawnpoints[randomSpawnpoint].Position+irr::core::vector3df(0,2,0));
#endif

  // If we're spawning the player, find the closest foilage group,
  // so we can start rendering the class.
  if(spawned == m_Player)
  {
    Game->getCore()->getObjects()->triggerSpawn(spawned->getParameters()->TeamID);
    Game->getCore()->getObjects()->findCurrentFoilageGroup(spawnpoints[randomSpawnpoint].Position);
    printf("# Player spawned\n");
  }
}
