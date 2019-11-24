#include "Core.h"
#include "Renderer.h"
#include "Configuration.h"

using namespace engine;

irr::f32 DoFaceCull = 0.f;

//void frontwarrior::core::SFaceCullData::Update()
//{
  /*scene::ISceneCollisionManager *colman = Core->getRenderer()->getSceneManager()->getSceneCollisionManager();
  scene::ICameraSceneNode *camera = Core->getRenderer()->getSceneManager()->getActiveCamera();

  u32 mb = 0;

  //for(u32 mb=0; mb < Buffers.size(); ++mb)
  //{
    /*m[mb] = k[mb] = 0;

    //irr::scene::CMeshBuffer<irr::video::S3DVertex> *Buffer = new scene::CMeshBuffer<irr::video::S3DVertex>();

    //Buffers[mb]->Indices.clear();

    for(u32 i=0; i < IndexCount[mb]; i+=3)
    {
      m[mb] ++;

  		if(Normals[mb][i/3].dotProduct(VertPoint[mb][i/3] - camera->getPosition()) < 0){

        position2d<s32>pos1 = colman->getScreenCoordinatesFrom3DPosition(
          Vertex[mb][IndicesCount[mb][i]].Pos );
        position2d<s32>pos2 = colman->getScreenCoordinatesFrom3DPosition(
          Vertex[mb][IndicesCount[mb][i+1]].Pos );
        position2d<s32>pos3 = colman->getScreenCoordinatesFrom3DPosition(
          Vertex[mb][IndicesCount[mb][i+2]].Pos );

        // Are trinagle points inside view space
        if((pos1.X >= 0 || pos2.X >= 0 || pos3.X >= 0)
        && (pos1.Y >= 0 || pos2.Y >= 0 || pos3.Y >= 0)
        && (pos1.X <= 800 || pos2.X <= 800 || pos3.X <= 800)
        && (pos1.Y <= 600 || pos2.Y <= 600 || pos3.Y <= 600))
        {
  				//Buffers[mb]->Indices.push_back(IndicesCount[mb][i]);
  				//Buffers[mb]->Indices.push_back(IndicesCount[mb][i+1]);
  				//Buffers[mb]->Indices.push_back(IndicesCount[mb][i+2]);

          k[mb] ++;
        }
  		}*/
  	/*
      u32 c1=0, pos=0;

      //Buffers[mb]->Indices.erase(0, Buffers[mb]->getIndexCount());
      //buffer->Indices.set_used(indexc);

      for(u32 i=0; i<IndexCount[mb]; ++i) {
         //m++;
         if(Normals[mb][i].dotProduct(VertPoint[mb][i] - camera->getPosition() ) < 0 ) {
            //buffer->Indices.push_back(indicesc[i*3]);
            //buffer->Indices.push_back(indicesc[i*3+1]);
            //buffer->Indices.push_back(indicesc[i*3+2]);
            //k++;
            c1++;
         }
         else {
            if (c1!=0) {
               c1*=3;
               pos+=c1;
               Buffers[mb]->Indices.set_used(pos);
               memcpy(&Indices[mb][pos-c1], &IndicesCount[mb][i*3-c1], c1*2);
               c1=0;
            }
         }
      }
      Buffers[mb]->Indices.set_used(pos);
      pos=0;*/



  	//printf("size: %d\n", Buffer->Indices.size());
  	//faceCulledObjects[id].Buffers[mb]->Indices.set_used( Buffer->Indices.size() );

  	//faceCulledObjects[id].Buffers[mb]->Indices = Buffer->Indices;

  	//for(u32 i_id=0; i_id < Buffer->Indices.size(); ++i_id)
    	//faceCulledObjects[id].Buffers[mb]->Indices[i_id] = Buffer->Indices[i_id];

    //faceCulledObjects[id].Buffers[mb]->recalculateBoundingBox();
    //faceCulledObjects[id].Buffers[mb]->setDirty();

  //} // mb

  //FaceCullingObjects[id].Buffers[0]->setDirty();

  //return;
//}


irr::scene::CMeshBuffer<irr::video::S3DVertex>* buffer;
irr::u16* indices;
void* vertices;
irr::video::S3DVertex* vertex;
irr::s32 indexc;
irr::u16* indicesc;
irr::core::vector3df* normals;
irr::core::vector3df* vertpnt;
int m=0, k=0;

void CCullingManager::update(irr::f32 time)
{
  return;

  irr::core::vector3df camPos = Core->getRenderer()->getCamera()->getPosition();

  //buffer->Indices.erase(0, buffer->getIndexCount());
  //buffer->Indices.clear();
  //buffer->Indices.set_used(indexc);

  irr::u32 realIndiceCount = 0;

  for(irr::s32 i = 0; i < indexc; i += 3)
  {
    m++;
    if (normals[i/3].dotProduct(vertpnt[i/3]-camPos) < 0)
    {
      //memcpy((void *)&indices[i],(void *)&indicesc[i], 3); <--doesn't seem to work i'm noob in pointers :D

      //printf("%d\n", indicesc[i]);

      //buffer->Indices[i] = indicesc[i];
      //buffer->Indices[i+1] = indicesc[i+1];
      //buffer->Indices[i+2] = indicesc[i+2];

      realIndiceCount += 3;

      k++;
    }
  }

  //buffer->Indices.set_used(realIndiceCount);
}

void CCullingManager::addFaceCulledMesh(irr::scene::IMesh* mesh)
{
	return;

	buffer = (irr::scene::CMeshBuffer<irr::video::S3DVertex>*)mesh->getMeshBuffer(0);

	/*printf("Vertex Count: %u\n",buffer->getVertexCount());
	printf("Index Count: %u\n\n",buffer->getIndexCount());
	printf("Total Triangles Count: %u\n\n",buffer->getIndexCount()/3);*/

	indices = buffer->getIndices();
	vertices = buffer->getVertices();
	vertex = (irr::video::S3DVertex *) vertices;
	indexc = buffer->getIndexCount();
	indicesc = (irr::u16* )malloc(sizeof(short int)*buffer->getIndexCount());
	normals = (irr::core::vector3df *)malloc(sizeof(irr::core::vector3df)*buffer->getIndexCount());
	vertpnt = (irr::core::vector3df *)malloc(sizeof(irr::core::vector3df)*buffer->getIndexCount());

	irr::core::triangle3df poly;

	//precalculated faces normals and precalculated 1 point of intersection: normals/vertpnt
	memcpy(indicesc, indices, sizeof(short int)*buffer->getIndexCount());

	for(irr::s32 i = 0; i < indexc; i += 3)
	{
		poly.pointA = vertex[indicesc[i]].Pos;
		poly.pointB = vertex[indicesc[i+1]].Pos;
		poly.pointC = vertex[indicesc[i+2]].Pos;

		normals[i/3] = poly.getNormal().normalize();

		vertpnt[i/3] = vertex[indicesc[i]].Pos;
	}

  return;
}

/*void frontwarrior::core::SFaceCullData::Setup(irr::scene::IMesh* Mesh, bool batchedMesh)
{
  u32 mbcount = Mesh->getMeshBufferCount();

  printf("\t\t# (Vertex type: %d)\n", Mesh->getMeshBuffer(0)->getVertexType());
  printf("\t\t# (Meshbuffers: %d)\n", mbcount);

  Buffers.set_used(mbcount);
  Indices.set_used(mbcount);
  Vertex.set_used(mbcount);
  IndexCount.set_used(mbcount);
  IndicesCount.set_used(mbcount);
  Normals.set_used(mbcount);
  VertPoint.set_used(mbcount);
  k.set_used(mbcount);
  m.set_used(mbcount);

  for(u32 mb=0; mb < mbcount; ++mb)
  {
    Buffers[mb] = (irr::scene::CMeshBuffer<irr::video::S3DVertex>*) Mesh->getMeshBuffer(mb);

	  Indices[mb] = Buffers[mb]->getIndices();
    void* Vertices = Buffers[mb]->getVertices();

  	Vertex[mb] = (S3DVertex*) Vertices;
  	IndexCount[mb] = Buffers[mb]->getIndexCount();
  	IndicesCount[mb] = (u16*)malloc(sizeof(short int)*Buffers[mb]->getIndexCount());
  	Normals[mb] = (vector3df*)malloc(sizeof(vector3df)*Buffers[mb]->getIndexCount());
  	VertPoint[mb] = (vector3df*)malloc(sizeof(vector3df)*Buffers[mb]->getIndexCount());

    //memcpy(IndicesCount[mb], Indices, sizeof(short int)*Buffers[mb]->getIndexCount());

  	IndicesCount[mb] = (u16* )malloc(sizeof(short int) * IndexCount[mb]);

  	triangle3df Triangle;

  	memcpy(IndicesCount[mb], Indices[mb], sizeof(short int) * IndexCount[mb]);
  	for(u32 i=0; i < IndexCount[mb]; i+=3)
    {
  		Triangle.pointA = Vertex[mb][IndicesCount[mb][i]].Pos;
  		Triangle.pointB = Vertex[mb][IndicesCount[mb][i+1]].Pos;
  		Triangle.pointC = Vertex[mb][IndicesCount[mb][i+2]].Pos;
  		Normals[mb][i/3] = Triangle.getNormal().normalize();
  		VertPoint[mb][i/3] = Vertex[mb][IndicesCount[mb][i]].Pos;
  	}
 }

  return;
}*/

irr::core::vector3df  LastPosition;
irr::f32              LastRotationY;
irr::f32              UpdateTime=0.f;
bool                  bForceOcclusionUpdate=false;

//void CSystem::ForceOcclusionUpdate() { bForceOcclusionUpdate = true; }

bool CRenderer::needsOcclusionQuery()
{
/*    bool result = false;
    irr::f32 CameraRotY = Core->getRenderer()->getCamera()->getRotation().Y;

    if(Occlusion.FinishedProcessing == false) {
        result = true;
    } else {
        if(UpdateTime > 2.0 || bForceOcclusionUpdate == true) {
            result = true;

            bForceOcclusionUpdate = false;
            UpdateTime = 0;
        } else {
            UpdateTime += Core->Time.Delta;

            if(fabs(CameraRotY - LastRotationY) > 25.0) {
                result = true;
                LastRotationY = CameraRotY;
                UpdateTime = 0;
            } else {
                irr::core::vector3df CameraPos = Core->getRenderer()->getCamera()->getAbsolutePosition();

                if((fabs(CameraPos.Y - LastPosition.Y) >= 45) || (CameraPos.getDistanceFrom(LastPosition) >= 150)) {
                    result = true;
                    LastPosition = CameraPos;
                    UpdateTime = 0;
                } // distance
            } // rotation
        } // time
    }
    return result;*/

    return false;
}



void CRenderer::registerNodes()
{
  /*SColor RenderColor = SColor(255,0,0,0);

  u32 registredNodeCount = 0;

  core::list<ISceneNode*>::Iterator it = GameWorldNodes.begin();
	for (; it != GameWorldNodes.end(); ++it)
	{
		if((*it)->isRenderInRTT() == false) continue;

    // Get object type
    u32 NodeType = identifyId( (*it)->getParam(0) );

    if(NodeType == STATIC_OBJECT
    || NodeType == BUILDING)
    {
      bool needsRendering = true;
      u32 ElementID;

      if(NodeType == STATIC_OBJECT){
        ElementID = (*it)->getParam(2);

        // If parent is invisible, dont render
        if(ObjectList[ElementID]->BaseParameters.ParentName != ""
        && ObjectList[ElementID]->BaseParameters.ParentObject->BaseParameters.Node->isVisible() == false)
        {
          needsRendering = false;
        }
        else if(ObjectList[ElementID]->BaseParameters.Node->getPosition().getDistanceFrom(
          Camera->getPosition()) >= Camera->getFarValue()*0.90)
        {
          needsRendering = false;
        }


      }

      if(needsRendering == false)
      {
         ObjectList[ElementID]->SetVisible(false);

      } else {
        (*it)->NodeParameters.Visible = (*it)->isVisible();

        // Force visibility
        (*it)->setVisible(false);

        (*it)->setAutomaticCulling(EAC_FRUSTUM_BOX );

        u32 res = SceneManager->registerNodeForRendering((*it));

        if(res == 1) (*it)->setVisible(true);

        (*it)->setAutomaticCulling(EAC_OFF);



        if((*it)->isVisible() == false){
          if(NodeType == STATIC_OBJECT)
            ObjectList[ElementID]->SetVisible(false);

        } else {
          registredNodeCount ++;

          RenderedNodes.push_back( (*it) );
          RenderedNodesVisibility.push_back( false );

          if(NodeType == STATIC_OBJECT)
          {
            if(ObjectList[ElementID]->Group.BatchMeshUsed == true)
            if(ObjectList[ElementID]->Parameters.Culling == CULLING_OCCLUSION)
              ObjectList[ElementID]->BaseParameters.Node->setMesh(
                ObjectList[ElementID]->Group.BatchOcclusionMesh);

          } else if(NodeType == BUILDING)
          {
            core::list<ISceneNode*> BuildingChildren = (*it)->getChildren();
            core::list<ISceneNode*>::Iterator it2 = BuildingChildren.begin();

            // Make child nodes invisible
            for (; it2 != BuildingChildren.end(); ++it2) (*it2)->setVisible(false);
          }

          for(u32 m=(*it)->getMaterialCount(); m > 0; --m)
          {
              (*it)->getMaterial(m-1).setTexture(0, Local.SolidColorTexture);
              (*it)->getMaterial(m-1).EmissiveColor = RenderColor;
              (*it)->getMaterial(m-1).MaterialType = EMT_SOLID;
          }

          (*it)->setMaterialFlag(video::EMF_LIGHTING, true);
          (*it)->setMaterialFlag(video::EMF_FOG_ENABLE, false);

          //
          // Change node color so that each node has a unique color
          //

          RenderColor.setBlue( RenderColor.getBlue() + 1 );

          // Increase blue color until it reaches 255, then reset it and increase green by 1
          if(RenderColor.getBlue() == 256){
              RenderColor.setBlue(0);
              RenderColor.setGreen( RenderColor.getGreen() + 1 );
          }
          // If green reaches 255 then increase red by 1
          else if(RenderColor.getGreen() == 256){
              RenderColor.setGreen(0);
              RenderColor.setRed( RenderColor.getRed() + 1 );
          }

        }
      }
    } // end: suitable object type

  }

  //printf("Registred nodes: %d\n", registredNodeCount);
  */
  return;
}



void CRenderer::restoreNode(irr::scene::ISceneNode* node)
{
  /*if(node->isVisible() == false) return;

  u32 NodeType = identifyId(node->getParam(0));

  if(NodeType != STATIC_OBJECT
  && NodeType != BUILDING)
  return;

  node->setVisible(node->NodeParameters.Visible);

  switch(identifyId(node->getParam(0)))
  {
    case STATIC_OBJECT:
    {
      u32 oID = node->getParam(2);

      if(ObjectList[oID]->Group.BatchMeshUsed == true)
      if(ObjectList[oID]->Parameters.Culling == CULLING_OCCLUSION)
        ObjectList[oID]->BaseParameters.Node->setMesh(
          ObjectList[oID]->Group.BatchMesh);
    }
    break;

    case BUILDING:
    {
    	if(node->NodeParameters.Visible == true)
        {
            core::list<ISceneNode*> BuildingChildren = node->getChildren();
            core::list<ISceneNode*>::Iterator it2 = BuildingChildren.begin();

            for (; it2 != BuildingChildren.end(); ++it2)
            (*it2)->setVisible(true);
        }
    }
    break;

    default:
      break;
  }

  for(u32 m=node->getMaterialCount(); m > 0; --m)
  {
      // Restore material type
      node->getMaterial(m-1).MaterialType = node->NodeParameters.MaterialType[m-1];

      // Set solid texture
      node->getMaterial(m-1).setTexture(0, node->NodeParameters.Texture[m-1]);

      // Set emissive color
      node->getMaterial(m-1).EmissiveColor = node->NodeParameters.Color[m-1];

      // Set light and fog states
      node->getMaterial(m-1).Lighting = node->NodeParameters.Light[m-1];
      node->getMaterial(m-1).FogEnable = node->NodeParameters.Fog[m-1];

  } // End of material cycle  */

}



void CRenderer::renderToOcclusionMap()
{
	/*if (!VideoDriver)
		return;

  SceneManager->setupRender();

	registerNodes();

  #ifdef RENDER_WITH_LIGHT_MANAGER
	if(LightManager)
		LightManager->OnPreRender(LightList);
	#endif

	u32 i; // new ISO for scoping problem in some compilers

  irr::core::array<irr::scene::ISceneNode*> CameraList = SceneManager->getCameraList();
  irr::core::array<irr::scene::ISceneNode*> SolidNodeList = SceneManager->getSolidNodeList();
  irr::core::array<irr::scene::ISceneNode*> TransparentNodeList = SceneManager->getTransparentNodeList();

  //printf("Game: Solid nodes: %d\n", SolidNodeList.size());

	//render camera scenes
	{
		SceneManager->setCurrentRendertime(scene::ESNRP_CAMERA);

    #ifdef RENDER_WITH_LIGHT_MANAGER
		if(LightManager)
			LightManager->OnRenderPassPreRender(CurrentRendertime);
    #endif

		for (i=0; i<CameraList.size(); ++i)
			CameraList[i]->render();

		//CameraList.set_used(0);

    #ifdef RENDER_WITH_LIGHT_MANAGER
		if(LightManager)
			LightManager->OnRenderPassPostRender(CurrentRendertime);
		#endif
	}


  // render default objects
	{
		SceneManager->setCurrentRendertime(scene::ESNRP_SOLID);

    #ifdef RENDER_WITH_LIGHT_MANAGER
		if(LightManager)
		{
			LightManager->OnRenderPassPreRender(CurrentRendertime);
			for (i=0; i<SolidNodeList.size(); ++i)
			{
				ISceneNode* node = SolidNodeList[i].Node;

        LightManager->OnNodePreRender(node);
				node->render();
				LightManager->OnNodePostRender(node);
			}
		}
		else
    #endif

    for (i=0; i<SolidNodeList.size(); ++i)
    {
      SolidNodeList[i]->render();

      //restoreNode(SolidNodeList[i]);
    }

		//Parameters.setAttribute ( "drawn_solid", (s32) SolidNodeList.size() );
		//SolidNodeList.set_used(0);

    #ifdef RENDER_WITH_LIGHT_MANAGER
		if(LightManager)
			LightManager->OnRenderPassPostRender(CurrentRendertime);
		#endif
	}


	// render transparent objects.
	{
		SceneManager->setCurrentRendertime(scene::ESNRP_TRANSPARENT);

    #ifdef RENDER_WITH_LIGHT_MANAGER
    if(LightManager)
		{
			LightManager->OnRenderPassPreRender(CurrentRendertime);

			for (i=0; i<TransparentNodeList.size(); ++i)
			{
				ISceneNode* node = TransparentNodeList[i].Node;

				LightManager->OnNodePreRender(node);
				node->render();
				LightManager->OnNodePostRender(node);
			}
		}
		else
		#endif
		for (i=0; i<TransparentNodeList.size(); ++i)
    {
      TransparentNodeList[i]->render();

      //restoreNode(TransparentNodeList[i]);
    }

		//Parameters.setAttribute ( "drawn_transparent", (s32) TransparentNodeList.size() );
		//TransparentNodeList.set_used(0);

    #ifdef RENDER_WITH_LIGHT_MANAGER
		if(LightManager)
			LightManager->OnRenderPassPostRender(CurrentRendertime);
    #endif
	}

  #ifdef RENDER_WITH_LIGHT_MANAGER
	if(LightManager)
		LightManager->OnPostRender();
	#endif


  for (i=0; i<SolidNodeList.size(); ++i) restoreNode(SolidNodeList[i]);
	for (i=0; i<TransparentNodeList.size(); ++i) restoreNode(TransparentNodeList[i]);


	SceneManager->setCurrentRendertime(scene::ESNRP_NONE);*/
}




void CRenderer::readRTT()
{
    //unsigned int * rtData = (unsigned int*) rt->lock();
    /*if(Occlusion.Data)
    {
        //video::SColor lastColor = SColor(255,255,255,255);
        video::SColor nextColor;
        video::SColor nextColor2;

        if(Occlusion.Piece_Index == 0)
        UsedColors.set_used(0);

        s32 step=2;
        s32 pieceSize = Occlusion.Size / Occlusion.Piece_Count;
        s32 start, end;

        if(Occlusion.Piece_Index == 0 || Occlusion.Piece_Index % 2 == 0)
        {
            start = Occlusion.Piece_Index * pieceSize, end = (Occlusion.Piece_Index + 1) * pieceSize;
        }
        else
        {
            start = (Occlusion.Piece_Count - Occlusion.Piece_Index)* pieceSize, end = (Occlusion.Piece_Count - Occlusion.Piece_Index + 1) * pieceSize;
        }

        for(int i=start; i < end; i += step)
        {
            video::SColor c;

            if(i+12 < end)
            {
                bool Color_1_3 = (SColor(Occlusion.Data[i+3]) == c && SColor(Occlusion.Data[i+1]) == c);

                if(SColor(Occlusion.Data[i+7]) == c && SColor(Occlusion.Data[i+5]) == c && Color_1_3 == true)
                {
                    if(SColor(Occlusion.Data[i+9]) == c && SColor(Occlusion.Data[i+11]) == c)
                    {
                        i += 12;
                        c = video::SColor(Occlusion.Data[i]);
                    }
                    else
                    {
                        i += 8;
                        c = video::SColor(Occlusion.Data[i]);
                    }
                }
                else
                {
                    bool Color_2 = (SColor(Occlusion.Data[i+2]) == c);

                    if(SColor(Occlusion.Data[i+4]) == c
                    && Color_2 == true && Color_1_3 == true)
                    {
                        i += 5;
                        c = video::SColor(Occlusion.Data[i]);
                    }
                    else
                    if(Color_1_3 == true && Color_2 == true)
                    {
                        i += 4;
                        c = video::SColor(Occlusion.Data[i]);
                    }
                    else
                    {
                        bool Color_1 = (SColor(Occlusion.Data[i+1]) == c);

                        if(Color_2 == true && Color_1 == true)
                        {
                            i += 3;
                            c = video::SColor(Occlusion.Data[i]);
                        }
                        else
                        if(Color_1 == true)
                        {
                            i += 2;
                            c = video::SColor(Occlusion.Data[i]);
                        }
                        else
                        {
                            c = video::SColor(Occlusion.Data[i]);
                        }
                    }
                }
            }

            // Ignore background color and last used color
            if(c == video::SColor(255,128,255,128) || UsedColors.binary_search(c) != -1)
            //IsColorUsed(c)
            continue;

            UsedColors.push_back(c);

            const s32 nodeId = c.getRed()*65536 + c.getGreen()*256 + c.getBlue();
            if(nodeId > RenderedNodes.size()) continue;

            if(RenderedNodes[ nodeId ])
            {
                RenderedNodesVisibility[nodeId] = true;
            }

        }
    }
    */

    if(occlusion.Piece_Index < occlusion.Piece_Count)
    occlusion.Piece_Index += 1;

    return;
}





bool CRenderer::updateOcclusionMap(irr::f32 time)
{
  if(needsOcclusionQuery() == true)
  {
    //if(Occlusion.FinishedProcessing == true)
    //{
      occlusion.FinishedProcessing = false;

      VideoDriver->setRenderTarget(occlusion.Texture, true, true, irr::video::SColor(255,128,255,128));

      //RenderedNodes.set_used(0);
      //RenderedNodesVisibility.set_used(0);

      //f32 CurrentFOV = Camera->getFOV();

      //Camera->setFOV( Game->GetFOV(EFT_NORMAL) + Occlusion.FrustFOV);
      //Camera->setAspectRatio(Occlusion.FrustAspect);

      renderToOcclusionMap();

      // Restore FOV to normal
      //Camera->setFOV( CurrentFOV );
      //Camera->setAspectRatio( GameSettings.aspectRatio.Width / GameSettings.aspectRatio.Height );

      VideoDriver->setRenderTarget(0, true, true);

    //}


    // Do the check
    /*if(Occlusion.Piece_Index == 0)
    {
      Occlusion.Size = Occlusion.Texture->getSize().getArea();
      Occlusion.Data = (unsigned int*) Occlusion.Texture->lock();
    }

    if(Occlusion.FrameSkip == 0) readRTT();
    else Occlusion.FrameSkip += 1;

    //Occlusion.Piece_Index++;

    if(Occlusion.FrameSkip > Occlusion.FramesToSkip) Occlusion.FrameSkip = 0;

    if(Occlusion.Piece_Index == Occlusion.Piece_Count)
    {
      Occlusion.Piece_Index = 0;
      Occlusion.FinishedProcessing = true;

      for(s32 j=RenderedNodes.size()-1; j>=0; --j){
          RenderedNodes[j]->setVisible( RenderedNodesVisibility[j] );
      }

      Occlusion.Texture->unlock();
    }*/

    return true;
  }

  return false;
}


irr::u32 CRenderer::createDevice()
{
  irr::SIrrlichtCreationParameters param;

  irr::video::E_DRIVER_TYPE deviceToUse;
  irr::u32 renderDeviceID = Core->getConfiguration()->getVideo()->renderDeviceID;

  if(renderDeviceID == 0) deviceToUse = irr::video::EDT_DIRECT3D9;
  else if(renderDeviceID == 1) deviceToUse = irr::video::EDT_OPENGL;
  else if(renderDeviceID == 2) deviceToUse = irr::video::EDT_BURNINGSVIDEO;

  param.AntiAlias = Core->getConfiguration()->getVideo()->antiAlias;
  param.HighPrecisionFPU = false;
  param.Vsync = false; // Don't need this. FPS is capped in code
  param.DriverType = deviceToUse;
  param.WindowSize = Core->getConfiguration()->getVideo()->windowSize;
  param.Bits = Core->getConfiguration()->getVideo()->colorDepth;
  param.Doublebuffer = Core->getConfiguration()->getVideo()->isDoubleBuffer;
  param.ZBufferBits = Core->getConfiguration()->getVideo()->ZDepth;
  param.Stencilbuffer = false;
  param.Fullscreen = Core->getConfiguration()->getVideo()->isFullscreen;
  //param.EventReceiver = &Core->GetInput()->GetEventReceiver();

  Device = createDeviceEx( param );

  Device->getLogger()->setLogLevel(irr::ELL_ERROR);

  {
    irr::u32 TotalRAM, AvailableRAM, ProcessorSpeed;

    if(!Device->getOSOperator()->getSystemMemory(&TotalRAM, &AvailableRAM))
        TotalRAM = AvailableRAM = 0;

    if(!Device->getOSOperator()->getProcessorSpeedMHz(&ProcessorSpeed))
        ProcessorSpeed = 0;

    printf("***\nFront Warrior\n\nOperating system: %S\nCPU: %d MHz\nRAM: %d (Avail: %d)\n***\n",
        Device->getOSOperator()->getOperationSystemVersion(),
        ProcessorSpeed, TotalRAM, AvailableRAM);
  }

  Device->getCursorControl()->setVisible(false);
  Device->setGammaRamp(Core->getConfiguration()->getVideo()->gamma,Core->getConfiguration()->getVideo()->gamma,Core->getConfiguration()->getVideo()->gamma,1.f,1.f);

  // Get pointers from device
  SceneManager = Device->getSceneManager();
  VideoDriver = Device->getVideoDriver();
  GUI = Device->getGUIEnvironment();
  Timer = Device->getTimer();

  ShaderManager = new CShaderManager(Core);
  CullingManager = new CCullingManager(Core);

  // Set window caption
  Device->setWindowCaption(L"Front Warrior");

  SceneManager->getParameters()->setAttribute(irr::scene::B3D_LOADER_IGNORE_MIPMAP_FLAG, true);
  SceneManager->getParameters()->setAttribute(irr::scene::ALLOW_ZWRITE_ON_TRANSPARENT, true);

  //core::dimension2d<u32> RTT_Size;
  occlusion.Piece_Count = 8;
  occlusion.FramesToSkip = 2;
  //Occlusion.Texture = VideoDriver->addRenderTargetTexture(irr::core::dimension2d<u32>(256,128), "OcclusionMap");

  return 0;
}
