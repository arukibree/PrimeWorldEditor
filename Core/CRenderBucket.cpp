#include "CRenderBucket.h"
#include <algorithm>
#include "CDrawUtil.h"
#include "CGraphics.h"

CRenderBucket::CRenderBucket()
{
    mEstSize = 0;
    mSize = 0;
}

void CRenderBucket::SetSortType(ESortType Type)
{
    mSortType = Type;
}

void CRenderBucket::Add(const SMeshPointer& Mesh)
{
    if (mSize >= mEstSize)
        mNodes.push_back(Mesh);
    else
        mNodes[mSize] = Mesh;

    mSize++;
}

void CRenderBucket::Sort(CCamera& Camera)
{
    struct {
        CCamera *pCamera;
        bool operator()(SMeshPointer left, SMeshPointer right) {
            CVector3f cPos = pCamera->Position();
            CVector3f cDir = pCamera->GetDirection();

            CVector3f distL = left.AABox.ClosestPointAlongVector(cDir) - cPos;
            float dotL = distL.Dot(cDir);
            CVector3f distR = right.AABox.ClosestPointAlongVector(cDir) - cPos;
            float dotR = distR.Dot(cDir);
            return (dotL > dotR);
        }
    } backToFront;
    backToFront.pCamera = &Camera;

    if (mSortType == BackToFront)
        std::stable_sort(mNodes.begin(), mNodes.begin() + mSize, backToFront);

    // Test: draw node bounding boxes + vertices used for sorting
    /*for (u32 iNode = 0; iNode < mNodes.size(); iNode++)
    {
        SMeshPointer *pNode = &mNodes[iNode];
        CVector3f Vert = pNode->AABox.ClosestPointAlongVector(Camera.GetDirection());
        CDrawUtil::DrawWireCube(pNode->AABox, CColor::skWhite);

        CVector3f Dist = Vert - Camera.GetPosition();
        float Dot = Dist.Dot(Camera.GetDirection());
        if (Dot < 0.f) Dot = -Dot;
        if (Dot > 50.f) Dot = 50.f;
        float Intensity = 1.f - (Dot / 50.f);
        CColor CubeColor(Intensity, Intensity, Intensity, 1.f);

        CGraphics::sMVPBlock.ModelMatrix = CTransform4f::TranslationMatrix(Vert).ToMatrix4f();
        CGraphics::UpdateMVPBlock();
        CDrawUtil::DrawCube(CubeColor);
    }*/
}

void CRenderBucket::Clear()
{
    mEstSize = mSize;
    if (mNodes.size() > mSize) mNodes.resize(mSize);
    mSize = 0;
}

void CRenderBucket::Draw(ERenderOptions Options)
{
    for (u32 n = 0; n < mSize; n++)
    {
        if (mNodes[n].Command == eDrawMesh)
            mNodes[n].pNode->Draw(Options);

        else if (mNodes[n].Command == eDrawAsset)
            mNodes[n].pNode->DrawAsset(Options, mNodes[n].Asset);

        else if (mNodes[n].Command == eDrawSelection)
            mNodes[n].pNode->DrawSelection();

        // todo: implementation for eDrawExtras
    }
}