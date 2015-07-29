#include "CAreaAttributes.h"
#include <Resource/script/CMasterTemplate.h>
#include <Resource/script/CScriptLayer.h>

CAreaAttributes::CAreaAttributes(CScriptObject *pObj)
{
    SetObject(pObj);
}

CAreaAttributes::~CAreaAttributes()
{
}

void CAreaAttributes::SetObject(CScriptObject *pObj)
{
    mpObj = pObj;
    mGame = pObj->Template()->MasterTemplate()->GetGame();
}

bool CAreaAttributes::IsLayerEnabled()
{
    return mpObj->Layer()->IsActive();
}

bool CAreaAttributes::IsSkyEnabled()
{
    CPropertyStruct *pBaseStruct = mpObj->Properties();

    switch (mGame)
    {
    case ePrime:
    case eEchoes:
        return static_cast<CBoolProperty*>(pBaseStruct->PropertyByIndex(1))->Get();
    case eCorruption:
        return static_cast<CBoolProperty*>(pBaseStruct->PropertyByIndex(9))->Get();
    default:
        return false;
    }
}

CModel* CAreaAttributes::SkyModel()
{
    CPropertyStruct *pBaseStruct = mpObj->Properties();

    switch (mGame)
    {
    case ePrime:
    case eEchoes:
        return (CModel*) static_cast<CFileProperty*>(pBaseStruct->PropertyByIndex(7))->Get();
    case eCorruption:
        return (CModel*) static_cast<CFileProperty*>(pBaseStruct->PropertyByIndex(8))->Get();
    default:
        return nullptr;
    }
}