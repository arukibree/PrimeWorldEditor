#include "CSceneManager.h"

#include <list>
#include <string>

#include <Common/CRay.h>
#include <Common/CRayCollisionTester.h>
#include <Common/StringUtil.h>
#include <Core/CGraphics.h>
#include <Core/CResCache.h>
#include <FileIO/CFileInStream.h>

/**
 * This class direly needs a rewrite
 * Future plan is to integrate a "scene layer" system, where nodes are grouped into layers
 * We would have terrain layer, lights layer, collision layer, multiple script layers, etc
 * Advantage of this is that I don't need to write separate functions for every single node type
 * They can all be tracked together and the code could be streamlined a lot.
 */
CSceneManager::CSceneManager()
{
    mShowTerrain = true;
    mShowCollision = false;
    mShowObjects = true;
    mShowLights = true;
    mSplitTerrain = true;
    mNodeCount = 0;
    mpSceneRootNode = new CRootNode(this, nullptr);
    mpArea = nullptr;
    mpWorld = nullptr;
    mpAreaRootNode = nullptr;
}

CSceneManager::~CSceneManager()
{
    ClearScene();
}

CModelNode* CSceneManager::AddModel(CModel *m)
{
    if (m == nullptr) return nullptr;

    CModelNode *node = new CModelNode(this, mpSceneRootNode, m);
    mModelNodes.push_back(node);
    mNodeCount++;
    return node;
}

CStaticNode* CSceneManager::AddStaticModel(CStaticModel *mdl)
{
    if (mdl == nullptr) return nullptr;

    CStaticNode *node = new CStaticNode(this, mpAreaRootNode, mdl);
    mStaticNodes.push_back(node);
    mNodeCount++;
    return node;
}

CCollisionNode* CSceneManager::AddCollision(CCollisionMesh *mesh)
{
    if (mesh == nullptr) return nullptr;

    CCollisionNode *node = new CCollisionNode(this, mpAreaRootNode, mesh);
    mCollisionNodes.push_back(node);
    mNodeCount++;
    return node;
}

CScriptNode* CSceneManager::AddScriptObject(CScriptObject *obj)
{
    if (obj == nullptr) return nullptr;

    CScriptNode *node = new CScriptNode(this, mpAreaRootNode, obj);
    mScriptNodes.push_back(node);
    mNodeCount++;
    return node;
}

CLightNode* CSceneManager::AddLight(CLight *Light)
{
    if (Light == nullptr) return nullptr;

    CLightNode *node = new CLightNode(this, mpAreaRootNode, Light);
    mLightNodes.push_back(node);
    mNodeCount++;
    return node;
}

void CSceneManager::SetActiveArea(CGameArea* _area)
{
    // Clear existing area
    delete mpAreaRootNode;
    mModelNodes.clear();
    mStaticNodes.clear();
    mCollisionNodes.clear();
    mScriptNodes.clear();
    mLightNodes.clear();
    mAreaAttributesObjects.clear();
    mpActiveAreaAttributes = nullptr;
    mScriptNodeMap.clear();

    // Create nodes for new area
    mpArea = _area;
    mAreaToken = CToken(mpArea);
    mpAreaRootNode = new CRootNode(this, mpSceneRootNode);

    if (mSplitTerrain)
    {
        u32 count = mpArea->GetStaticModelCount();
        for (u32 m = 0; m < count; m++)
            AddStaticModel(mpArea->GetStaticModel(m));
    }
    else
    {
        u32 count = mpArea->GetTerrainModelCount();
        for (u32 m = 0; m < count; m++)
        {
            CModel *mdl = mpArea->GetTerrainModel(m);
            CModelNode *node = AddModel(mdl);
            node->SetDynamicLighting(false);
        }
    }

    AddCollision(mpArea->GetCollision());

    u32 NumLayers = mpArea->GetScriptLayerCount();
    for (u32 l = 0; l < NumLayers; l++)
    {
        CScriptLayer *layer = mpArea->GetScriptLayer(l);
        u32 NumObjects = layer->GetNumObjects();
        mScriptNodes.reserve(mScriptNodes.size() + NumObjects);

        for (u32 o = 0; o < NumObjects; o++)
        {
            CScriptObject *pObj = layer->ObjectByIndex(o);
            CScriptNode *Node = AddScriptObject( pObj );
            Node->BuildLightList(mpArea);

            // Add to map
            mScriptNodeMap[pObj->InstanceID()] = Node;

            // AreaAttributes check
            switch (pObj->ObjectTypeID())
            {
            case 0x4E:       // MP1 AreaAttributes ID
            case 0x52454141: // MP2 AreaAttributes ID ("REAA")
                mAreaAttributesObjects.emplace_back( CAreaAttributes(pObj) );
                break;
            }
        }
    }

    CScriptLayer *pGenLayer = mpArea->GetGeneratorLayer();
    if (pGenLayer)
    {
        for (u32 o = 0; o < pGenLayer->GetNumObjects(); o++)
        {
            CScriptObject *pObj = pGenLayer->ObjectByIndex(o);
            CScriptNode *Node = AddScriptObject(pObj);
            Node->BuildLightList(mpArea);

            // Add to map
            mScriptNodeMap[pObj->InstanceID()] = Node;
        }
    }
    PickEnvironmentObjects();

    // Ensure script nodes have valid positions
    for (auto it = mScriptNodeMap.begin(); it != mScriptNodeMap.end(); it++)
        it->second->GeneratePosition();

    u32 NumLightLayers = mpArea->GetLightLayerCount();
    CGraphics::sAreaAmbientColor = CColor::skBlack;

    for (u32 ly = 0; ly < NumLightLayers; ly++)
    {
        u32 NumLights = mpArea->GetLightCount(ly);

        for (u32 l = 0; l < NumLights; l++)
        {
            CLight *Light = mpArea->GetLight(ly, l);

            if (Light->GetType() == eLocalAmbient)
                CGraphics::sAreaAmbientColor += Light->GetColor();

            AddLight(Light);
        }
    }

    std::cout << CSceneNode::NumNodes() << " nodes\n";
}

void CSceneManager::SetActiveWorld(CWorld* _world)
{
    mpWorld = _world;
    mWorldToken = CToken(mpWorld);
}

void CSceneManager::ClearScene()
{
    mpAreaRootNode->Unparent();
    delete mpAreaRootNode;

    mModelNodes.clear();
    mStaticNodes.clear();
    mCollisionNodes.clear();
    mScriptNodes.clear();
    mLightNodes.clear();

    mpArea = nullptr;
    mAreaToken.Unlock();
    mpWorld = nullptr;
    mWorldToken.Unlock();
    mNodeCount = 0;
}

void CSceneManager::AddSceneToRenderer(CRenderer *pRenderer)
{
    if (mShowTerrain)
    {
        for (u32 n = 0; n < mModelNodes.size(); n++)
            if (mModelNodes[n]->IsVisible())
                mModelNodes[n]->AddToRenderer(pRenderer);

        for (u32 n = 0; n < mStaticNodes.size(); n++)
            if (mStaticNodes[n]->IsVisible())
                mStaticNodes[n]->AddToRenderer(pRenderer);
    }

    if (mShowCollision)
    {
        for (u32 n = 0; n < mCollisionNodes.size(); n++)
            if (mCollisionNodes[n]->IsVisible())
                mCollisionNodes[n]->AddToRenderer(pRenderer);
    }

    if (mShowObjects)
    {
        for (u32 n = 0; n < mScriptNodes.size(); n++)
            if (mScriptNodes[n]->IsVisible())
                mScriptNodes[n]->AddToRenderer(pRenderer);
    }

    if (mShowLights)
    {
        for (u32 n = 0; n < mLightNodes.size(); n++)
            if (mLightNodes[n]->IsVisible())
                mLightNodes[n]->AddToRenderer(pRenderer);
    }
}

SRayIntersection CSceneManager::SceneRayCast(const CRay& Ray)
{
    // Terribly hacky stuff to avoid having tons of redundant code
    // because I'm too lazy to rewrite CSceneManager right now and fix it
    // (I'm probably going to do it soon...)
    std::vector<CSceneNode*> *pNodeVectors[5] = {
        reinterpret_cast<std::vector<CSceneNode*>*>(&mModelNodes),
        reinterpret_cast<std::vector<CSceneNode*>*>(&mStaticNodes),
        reinterpret_cast<std::vector<CSceneNode*>*>(&mCollisionNodes),
        reinterpret_cast<std::vector<CSceneNode*>*>(&mScriptNodes),
        reinterpret_cast<std::vector<CSceneNode*>*>(&mLightNodes),
    };
    bool NodesVisible[5] = {
        true, mShowTerrain, mShowCollision, mShowObjects, mShowLights
    };

    // Less hacky stuff
    CRayCollisionTester Tester(Ray);

    for (u32 iVec = 0; iVec < 5; iVec++)
    {
        if (!NodesVisible[iVec]) continue;

        std::vector<CSceneNode*>& vec = *pNodeVectors[iVec];

        for (u32 iNode = 0; iNode < vec.size(); iNode++)
            if (vec[iNode]->IsVisible())
                vec[iNode]->RayAABoxIntersectTest(Tester);
    }

    return Tester.TestNodes();
}

void CSceneManager::PickEnvironmentObjects()
{
    // Pick AreaAttributes
    for (auto it = mAreaAttributesObjects.begin(); it != mAreaAttributesObjects.end(); it++)
    {
        if ((*it).IsLayerEnabled())
        {
            mpActiveAreaAttributes = &(*it);
            break;
        }
    }
}

CScriptNode* CSceneManager::ScriptNodeByID(u32 InstanceID)
{
    auto it = mScriptNodeMap.find(InstanceID);

    if (it != mScriptNodeMap.end()) return it->second;
    else return nullptr;
}

CScriptNode* CSceneManager::NodeForObject(CScriptObject *pObj)
{
    return ScriptNodeByID(pObj->InstanceID());
}

CLightNode* CSceneManager::NodeForLight(CLight *pLight)
{
    // Slow. Is there a better way to do this?
    for (auto it = mLightNodes.begin(); it != mLightNodes.end(); it++)
        if ((*it)->Light() == pLight) return *it;

    return nullptr;
}

CModel* CSceneManager::GetActiveSkybox()
{
    if (mpActiveAreaAttributes)
    {
        if (mpActiveAreaAttributes->IsSkyEnabled())
        {
            CModel *pSky = mpActiveAreaAttributes->SkyModel();
            if (pSky) return pSky;
            else return mpWorld->GetDefaultSkybox();
        }
        else
            return nullptr;
    }

    else return nullptr;
}

CGameArea* CSceneManager::GetActiveArea()
{
    return mpArea;
}

void CSceneManager::SetWorld(bool on)
{
    mShowTerrain = on;
}

void CSceneManager::SetCollision(bool on)
{
    mShowCollision = on;
}

void CSceneManager::SetLights(bool on)
{
    mShowLights = on;
}

void CSceneManager::SetObjects(bool on)
{
    mShowObjects = on;
}

bool CSceneManager::IsTerrainEnabled()
{
    return mShowTerrain;
}

bool CSceneManager::IsCollisionEnabled()
{
    return mShowCollision;
}

bool CSceneManager::AreLightsEnabled()
{
    return mShowLights;
}

bool CSceneManager::AreScriptObjectsEnabled()
{
    return mShowObjects;
}
