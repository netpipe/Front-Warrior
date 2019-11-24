

#ifndef __CPhysicsEntity_h
#define __CPhysicsEntity_h


#include "PhysicsGlobal.h"
#include "IPhysicsEntity.h"


class CPhysicsEntity : public IPhysicsEntity
{
public:
	CPhysicsEntity();
	~CPhysicsEntity();
	void init(NewtonBody *body, ISceneNode *node);
	void setPosition(vector3df position);
	void setRotation(vector3df rotation);
	vector3df getPosition();
	vector3df getRotation();
	NewtonBody *getBody();
	ISceneNode *getNode();
	ENUMERATED_ENTITY_TYPE getEntityType();
protected:
	NewtonBody *m_body;
	ISceneNode *m_node;
};


#endif