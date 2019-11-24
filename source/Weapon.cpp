#include "Core.h"
#include "Game.h"
#include "Weapon.h"
#include "Renderer.h"
#include "Maths.h"
#include "SoundManager.h"
#include "Physics.h"
#include "ObjectManager.h"
#include "CharacterManager.h"
#include "Player.h"

using namespace game;
using namespace engine;

irr::u32 CWeapon::getAverageAmmoInClips()
{
  irr::u32 avgAmmo = 0;
  irr::u8 clips = 0;

  for(irr::u16 i=0; i<ammoClips.size(); ++i)
    if(ammoClips[i]->empty == false) {
      avgAmmo += ammoClips[i]->ammo;
      clips ++;
    }

  avgAmmo /= clips;

  return avgAmmo;
}

irr::s16 CWeapon::findHighestAmmoClip()
{
  irr::u16 highestAmmo = 0;
  irr::s16 clip_id = -1;

  for(irr::u16 i=0; i<ammoClips.size(); ++i) {

    if(ammoClips[i]->empty == true)
      continue;

    if(ammoClips[i]->ammo > highestAmmo) {
      highestAmmo = ammoClips[i]->ammo;
      clip_id = i;
    }
  }

  return clip_id;
}

void CWeapon::createMuzzle()
{
  if(node != NULL)
  {
    SWeaponParameters *weap = Game->weaponsParameters[type];

    muzzle = Game->getCore()->getRenderer()->getSceneManager()->addMeshSceneNode(
      Game->getCore()->getRenderer()->getSceneManager()->getMesh(weap->f_muzzle.c_str()), node, -1);

    irr::f32 scaleChange = Game->getCore()->getMath()->getRandomInt(0,65) / 100.f;

    muzzle->setScale(irr::core::vector3df(3,3,3)-irr::core::vector3df(scaleChange,scaleChange,scaleChange));
    muzzle->setMaterialType(irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    muzzle->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    muzzle->setMaterialFlag(irr::video::EMF_ZWRITE_ENABLE, false);
    muzzle->setRotation(irr::core::vector3df(0,0,irr::f32(Game->getCore()->getMath()->getRandomInt(0,45))));

    irr::scene::ISceneNodeAnimator *anim = Game->getCore()->getRenderer()->getSceneManager()->createDeleteAnimator(120);
    muzzle->addAnimator(anim);
    anim->drop();

  }
}

void CWeapon::checkForHits()
{
  if(node != NULL)
  {
    SWeaponParameters *weap = Game->weaponsParameters[type];

    irr::core::vector3df firingTarget = irr::core::vector3df(0,0,weap->fireRange*16);

    irr::core::matrix4 weaponTransformationMatrix = node->getAbsoluteTransformation();
    irr::core::matrix4 targetRotationMatrix;
    targetRotationMatrix.setRotationDegrees(weaponTransformationMatrix.getRotationDegrees());

    targetRotationMatrix.rotateVect(firingTarget);

    irr::core::line3df wFireLine;
    bool bodyHit = false;
    irr::core::vector3df hitPosition, hitNormal;
    SObjectData* bodyUserData = (SObjectData*)NULL;
    irr::scene::IMeshSceneNode *hitNode = NULL;

#ifdef PHYSICS_NEWTON
    wFireLine.start = weaponTransformationMatrix.getTranslation();
    wFireLine.end = wFireLine.start + firingTarget;

    engine::physics::SRayCastParameters weaponTraceRay;

    weaponTraceRay.excluded.push_back(
      Game->getCharacters()->getPlayer()->getBody()->PhysicsBody->getShapeID());

    weaponTraceRay.line = wFireLine;

    engine::physics::SRayCastResult rayResult =
      Game->getCore()->getPhysics()->getRayCollision(weaponTraceRay);

    bodyHit = (rayResult.body == NULL) ? false : true;

    if(bodyHit)
    {
      bodyUserData = (SObjectData*)rayResult.body->getUserData();
      hitPosition = rayResult.position;
      hitNormal = rayResult.normal;
      hitNode = (irr::scene::IMeshSceneNode *)rayResult.body->getNode();
    }
#endif

    if(bodyHit && bodyUserData)
    {
      irr::core::vector3df impulse = irr::core::vector3df(0, 0, 0.25f);
      targetRotationMatrix.rotateVect(impulse);

      // What type of body was hit?
      engine::CBaseMapObject *obj_ =
        Game->getCore()->getObjects()->getObject(bodyUserData->type, bodyUserData->container_id);

      irr::core::matrix4 decal_matrix;

      bool create_bullet_hole_decal = false;
      irr::core::stringc bullet_debris_texture = "";
      irr::f32 bullet_debris_scale = 1.f;
      irr::f32 bullet_debris_velocity = 0.0010f;
      irr::u32 bullet_debris_random_angle = 30;
      irr::core::vector3df bullet_debris_position_offset;

      if(bodyUserData->type == EOT_DYNAMIC)
      {
        //colOut.body->addImpulse(impulse, colOut.point);

        create_bullet_hole_decal = true;
      }
      else if(bodyUserData->type == EOT_STATIC)
      {
        create_bullet_hole_decal = true;
      }
      else if(bodyUserData->type == EOT_BUILDING)
      {
        create_bullet_hole_decal = true;
      }
      else if(bodyUserData->type == EOT_TERRAIN)
      {
        bullet_debris_texture = "data/particles/debris/bh_dirt_piece.tga";
        bullet_debris_scale = 1.58f;
        bullet_debris_velocity = 0.0037f;
        bullet_debris_random_angle = 27;
        bullet_debris_position_offset.set(0.f, 0.30f, 0.f);
      }

      // Create bullet mark

      if(node->getParam(0) != 0 && create_bullet_hole_decal)
      {
        irr::core::vector3df bullet_hole_position = hitPosition - hitNode->getPosition();

        irr::core::stringc bullet_hole_texture = "";

        switch(bodyUserData->material)
        {
          case EBMT_METAL:
            bullet_hole_texture = "data/particles/decals/bullethole_metal.png";
            bullet_debris_texture = "data/particles/debris/bh_metal_fastpiece.tga";
          break;

          case EBMT_STONE:
            bullet_hole_texture = "data/particles/decals/bullethole_stone.png";
            bullet_debris_texture = "data/particles/debris/stonechip.tga";
          break;

          case EBMT_WOOD:
            bullet_hole_texture = "data/particles/decals/bullethole_wood.png";
            bullet_debris_texture = "data/particles/debris/woodsplinters.tga";
            bullet_debris_scale = 0.85f;
          break;

          case EBMT_SANDBAG:
            bullet_hole_texture = "data/particles/decals/bullethole_sandbag.png";
            bullet_debris_texture = "data/particles/debris/bh_wood_piece.tga";
            bullet_debris_scale = 0.5f;
          break;

          default:
            bullet_hole_texture = "data/particles/decals/bullethole_stone.png";
            bullet_debris_texture = "data/particles/debris/stonechip.tga";
          break;
        }

        if(obj_->isDecalAtPosition(bullet_hole_position, MINIMUM_DISTANCE_BETWEEN_BULLET_DECALS) == false)
        {
          irr::scene::IMeshBuffer* bullet_hole_decal =
            Game->getCore()->getRenderer()->getSceneManager()->getMesh("data/particles/decals/bullethole1.ms3d")->getMeshBuffer(0);

          bullet_hole_decal->getMaterial().setTexture(0,
            Game->getCore()->getRenderer()->getVideoDriver()->getTexture(bullet_hole_texture.c_str()));
          bullet_hole_decal->getMaterial().Lighting = false;
          bullet_hole_decal->getMaterial().MaterialType = irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF ;
          bullet_hole_decal->getMaterial().MaterialTypeParam = 0.20f;
          bullet_hole_decal->getMaterial().FogEnable = true;

          irr::core::matrix4 decal_rotation_matrix;
          decal_rotation_matrix.setRotationDegrees(
            irr::core::vector3df(0, irr::f32(Game->getCore()->getMath()->getRandomInt(0,360)),0));

          Game->getCore()->getMath()->alignToUpVector(decal_matrix, decal_rotation_matrix, hitNormal, 1.0f);

          irr::scene::CBatchingMesh* obj_mesh = (irr::scene::CBatchingMesh*)hitNode->getMesh();
          Game->getCore()->getObjects()->copyMaterialToMesh(obj_mesh, hitNode);

          obj_mesh->addMeshBuffer(
            bullet_hole_decal,
            bullet_hole_position,
            decal_matrix.getRotationDegrees(),
            irr::core::vector3df(0.0019f,0.0019f,0.0019f));

          obj_mesh->update();
          hitNode->setMesh(obj_mesh);
        }

      }

      if(bullet_debris_texture != "")
      {
        // Each particle has a slightly different size
        bullet_debris_scale *= irr::f32(Game->getCore()->getMath()->getRandomInt(750,1280)/1000.f);

        Game->createSurfaceHitParticles(
          bullet_debris_texture.c_str(),
          hitPosition + bullet_debris_position_offset,
          hitNormal.normalize(),
          bullet_debris_scale,
          bullet_debris_velocity,
          0.41f,
          15, 20, bullet_debris_random_angle);
      }

      // Play hit sound based on body material
      Game->playSurfaceHitSound((game::E_BODY_MATERIAL_TYPE)bodyUserData->material, hitPosition);

    }

  }
  else {
#ifdef ENGINE_DEVELOPMENT_MODE
    printf("checkForHits: Weapon node is NULL!\n");
#endif
  }

  return;
}

bool CWeapon::reload()
{
  if(bIsReloading || selectedClipIndex == -1) return false;
  if(ammoClips[selectedClipIndex]->ammo == ammoClips[selectedClipIndex]->size) return false;

  SWeaponParameters *weap = Game->weaponsParameters[type];

  nextSelectedClipIndex = findHighestAmmoClip();

  // This clip is already in use
  if(nextSelectedClipIndex == selectedClipIndex) return false;

  if(nextSelectedClipIndex != -1)
  {
    Game->getCore()->getSound()->playSound2D(
      weap->reloadSounds[Game->getCore()->getMath()->getRandomInt(0, weap->reloadSounds.size()-1)].c_str(),
      false, // no looping
      0.66f, // vol
      -0.02f); // pan .. very slightly to right OR +0.02 if the weapon-handndess is left

    timeReload = weap->reloadTime;

    bIsReloading = true;

    return true;
  }
  else
  {
    refreshActiveClip();
  }

  return false;
}

void CWeapon::init()
{
  for(irr::u16 i=0; i<ammoClips.size(); ++i)
    delete ammoClips[i];

  ammoClips.clear();

  SWeaponParameters *params = Game->weaponsParameters[type];

  for(irr::u32 clip_i=0; clip_i < params->startClips; ++clip_i)
  {
    SAmmoClip *clip = new SAmmoClip(params->clipSize);
    clip->ammo = params->clipSize;
    ammoClips.push_back(clip);
  }

  selectedClipIndex = 0;
}

void CWeapon::refill()
{
  for(irr::u16 clip_i=0; clip_i < ammoClips.size(); ++clip_i)
  {
    ammoClips[clip_i]->ammo = ammoClips[clip_i]->size;
    ammoClips[clip_i]->empty = false;
  }
}
