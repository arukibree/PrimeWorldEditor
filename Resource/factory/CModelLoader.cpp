#include "CModelLoader.h"
#include "CMaterialLoader.h"
#include <Core/Log.h>

CModelLoader::CModelLoader()
{
    mFlags = eNoFlags;
}

CModelLoader::~CModelLoader()
{
}

void CModelLoader::LoadWorldMeshHeader(CInputStream &Model)
{
    // I don't really have any need for most of this data, so
    Model.Seek(0x34, SEEK_CUR);
    mAABox = CAABox(Model);
    mpBlockMgr->ToNextBlock();
}

void CModelLoader::LoadAttribArrays(CInputStream& Model)
{
    // Positions
    if (mFlags & eShortPositions) // Shorts (DKCR only)
    {
        mPositions.resize(mpBlockMgr->CurrentBlockSize() / 0x6);
        float Divisor = 8192.f; // Might be incorrect! Needs verification via size comparison.

        for (u32 iVtx = 0; iVtx < mPositions.size(); iVtx++)
        {
            float x = Model.ReadShort() / Divisor;
            float y = Model.ReadShort() / Divisor;
            float z = Model.ReadShort() / Divisor;
            mPositions[iVtx] = CVector3f(x, y, z);
        }
    }

    else // Floats
    {
        mPositions.resize(mpBlockMgr->CurrentBlockSize() / 0xC);

        for (u32 iVtx = 0; iVtx < mPositions.size(); iVtx++)
            mPositions[iVtx] = CVector3f(Model);
    }

    mpBlockMgr->ToNextBlock();

    // Normals
    if (mFlags & eShortNormals) // Shorts
    {
        mNormals.resize(mpBlockMgr->CurrentBlockSize() / 0x6);
        float Divisor = (mVersion < eReturns) ? 32768.f : 16384.f;

        for (u32 iVtx = 0; iVtx < mNormals.size(); iVtx++)
        {
            float x = Model.ReadShort() / Divisor;
            float y = Model.ReadShort() / Divisor;
            float z = Model.ReadShort() / Divisor;
            mNormals[iVtx] = CVector3f(x, y, z);
        }
    }
    else // Floats
    {
        mNormals.resize(mpBlockMgr->CurrentBlockSize() / 0xC);

        for (u32 iVtx = 0; iVtx < mNormals.size(); iVtx++)
            mNormals[iVtx] = CVector3f(Model);
    }

    mpBlockMgr->ToNextBlock();

    // Colors
    mColors.resize(mpBlockMgr->CurrentBlockSize() / 4);

    for (u32 iVtx = 0; iVtx < mColors.size(); iVtx++)
        mColors[iVtx] = CColor(Model);

    mpBlockMgr->ToNextBlock();


    // Float UVs
    mTex0.resize(mpBlockMgr->CurrentBlockSize() / 0x8);

    for (u32 iVtx = 0; iVtx < mTex0.size(); iVtx++)
        mTex0[iVtx] = CVector2f(Model);

    mpBlockMgr->ToNextBlock();

    // Short UVs
    if (mFlags & eHasTex1)
    {
        mTex1.resize(mpBlockMgr->CurrentBlockSize() / 0x4);
        float Divisor = (mVersion < eReturns) ? 32768.f : 8192.f;

        for (u32 iVtx = 0; iVtx < mTex1.size(); iVtx++)
        {
            float x = Model.ReadShort() / Divisor;
            float y = Model.ReadShort() / Divisor;
            mTex1[iVtx] = CVector2f(x, y);
        }

        mpBlockMgr->ToNextBlock();
    }
}

void CModelLoader::LoadSurfaceOffsets(CInputStream& Model)
{
    mSurfaceCount = Model.ReadLong();
    mSurfaceOffsets.resize(mSurfaceCount);

    for (u32 iSurf = 0; iSurf < mSurfaceCount; iSurf++)
        mSurfaceOffsets[iSurf] = Model.ReadLong();

    mpBlockMgr->ToNextBlock();
}

SModelData* CModelLoader::LoadSurfaces(CInputStream& Model)
{
    // This function is meant to be called at the start of the first surface
    SModelData *pData = new SModelData;
    u32 Offset = Model.Tell();

    // Surfaces
    pData->mSurfaces.resize(mSurfaceCount);

    for (u32 iSurf = 0; iSurf < mSurfaceCount; iSurf++)
    {
        SSurface *pSurf = new SSurface;
        pData->mSurfaces[iSurf] = pSurf;
        u32 NextSurface = mpBlockMgr->NextOffset();

        // Surface header
        if (mVersion  < eReturns)
            LoadSurfaceHeaderPrime(Model, pSurf);
        else
            LoadSurfaceHeaderDKCR(Model, pSurf);

        bool HasAABB = (pSurf->AABox != CAABox::skInfinite);
        CMaterial *pMat = mMaterials[0]->materials[pSurf->MaterialID];

        // Primitive table
        u8 Flag = Model.ReadByte();

        while ((Flag != 0) && ((u32) Model.Tell() < NextSurface))
        {
            SSurface::SPrimitive Prim;
            Prim.Type = EGXPrimitiveType(Flag & 0xF8);
            u16 VertexCount = Model.ReadShort();

            for (u16 iVtx = 0; iVtx < VertexCount; iVtx++)
            {
                CVertex Vtx;
                EVertexDescription VtxDesc = pMat->VtxDesc();

                for (u32 iMtxAttr = 0; iMtxAttr < 8; iMtxAttr++)
                    if (VtxDesc & (ePosMtx << iMtxAttr)) Model.Seek(0x1, SEEK_CUR);

                // Only thing to do here is check whether each attribute is present, and if so, read it.
                // A couple attributes have special considerations; normals can be floats or shorts, as can tex0, depending on vtxfmt.
                // tex0 can also be read from either UV buffer; depends what the material says.

                // Position
                if (VtxDesc & ePosition)
                {
                    u16 PosIndex = Model.ReadShort() & 0xFFFF;
                    Vtx.Position = mPositions[PosIndex];
                    Vtx.ArrayPosition = PosIndex;

                    if (!HasAABB) pSurf->AABox.ExpandBounds(Vtx.Position);
                }

                // Normal
                if (VtxDesc & eNormal)
                    Vtx.Normal = mNormals[Model.ReadShort() & 0xFFFF];

                // Color
                for (u32 c = 0; c < 2; c++)
                    if (VtxDesc & (eColor0 << (c * 2)))
                        Vtx.Color[c] = mColors[Model.ReadShort() & 0xFFFF];

                // Tex Coords - these are done a bit differently in DKCR than in the Prime series
                if (mVersion < eReturns)
                {
                    // Tex0
                    if (VtxDesc & eTex0)
                    {
                        if ((mFlags & eHasTex1) && (pMat->Options() & CMaterial::eShortTexCoord))
                            Vtx.Tex[0] = mTex1[Model.ReadShort() & 0xFFFF];
                        else
                            Vtx.Tex[0] = mTex0[Model.ReadShort() & 0xFFFF];
                    }

                    // Tex1-7
                    for (u32 iTex = 1; iTex < 7; iTex++)
                        if (VtxDesc & (eTex0 << (iTex * 2)))
                            Vtx.Tex[iTex] = mTex0[Model.ReadShort() & 0xFFFF];
                }

                else
                {
                    // Tex0-7
                    for (u32 iTex = 0; iTex < 7; iTex++)
                    {
                        if (VtxDesc & (eTex0 << iTex * 2))
                        {
                            if (!mSurfaceUsingTex1)
                                Vtx.Tex[iTex] = mTex0[Model.ReadShort() & 0xFFFF];
                            else
                                Vtx.Tex[iTex] = mTex1[Model.ReadShort() & 0xFFFF];
                        }
                    }
                }

                Prim.Vertices.push_back(Vtx);
            } // Vertex array end

            // Update vertex/triangle count
            pSurf->VertexCount += VertexCount;

            switch (Prim.Type)
            {
                case eGX_Triangles:
                    pSurf->TriangleCount += VertexCount / 3;
                    break;
                case eGX_TriangleFan:
                case eGX_TriangleStrip:
                    pSurf->TriangleCount += VertexCount - 2;
                    break;
            }

            pSurf->Primitives.push_back(Prim);
            Flag = Model.ReadByte();
        } // Primitive table end

        mpBlockMgr->ToNextBlock();
    } // Submesh table end

    return pData;
}

void CModelLoader::LoadSurfaceHeaderPrime(CInputStream& Model, SSurface *pSurf)
{
    pSurf->CenterPoint = CVector3f(Model);
    pSurf->MaterialID = Model.ReadLong();

    Model.Seek(0xC, SEEK_CUR);
    u32 ExtraSize = Model.ReadLong();
    pSurf->ReflectionDirection = CVector3f(Model);
    if (mVersion >= eEchoesDemo) Model.Seek(0x4, SEEK_CUR); // Extra values in Echoes. Not sure what they are.
    bool HasAABox = (ExtraSize >= 0x18); // MREAs have a set of bounding box coordinates here.

    // If this surface has a bounding box, we can just read it here. Otherwise we'll fill it in manually.
    if (HasAABox)
    {
        ExtraSize -= 0x18;
        pSurf->AABox = CAABox(Model);
    }
    else
        pSurf->AABox = CAABox::skInfinite;

    Model.Seek(ExtraSize, SEEK_CUR);
    Model.SeekToBoundary(32);
}

void CModelLoader::LoadSurfaceHeaderDKCR(CInputStream& Model, SSurface *pSurf)
{
    pSurf->CenterPoint = CVector3f(Model);
    Model.Seek(0xE, SEEK_CUR);
    pSurf->MaterialID = (u32) Model.ReadShort();
    Model.Seek(0x2, SEEK_CUR);
    mSurfaceUsingTex1 = (Model.ReadByte() == 1);
    u32 ExtraSize = Model.ReadByte();

    if (ExtraSize > 0)
    {
        ExtraSize -= 0x18;
        pSurf->AABox = CAABox(Model);
    }
    else
        pSurf->AABox = CAABox::skInfinite;

    Model.Seek(ExtraSize, SEEK_CUR);
    Model.SeekToBoundary(32);
}

// ************ STATIC ************
CModel* CModelLoader::LoadCMDL(CInputStream& CMDL)
{
    CModelLoader Loader;
    Log::Write("Loading " + CMDL.GetSourceString());

    // CMDL header - same across the three Primes, but different structure in DKCR
    u32 Magic = CMDL.ReadLong();

    u32 Version, BlockCount, MatSetCount;
    CAABox AABox;

    // 0xDEADBABE - Metroid Prime seres
    if (Magic == 0xDEADBABE)
    {
        Version = CMDL.ReadLong();
        u32 Flags = CMDL.ReadLong();
        AABox = CAABox(CMDL);
        BlockCount = CMDL.ReadLong();
        MatSetCount = CMDL.ReadLong();

        if (Flags & 0x2) Loader.mFlags |= eShortNormals;
        if (Flags & 0x4) Loader.mFlags |= eHasTex1;
    }

    // 0x9381000A - Donkey Kong Country Returns
    else if (Magic == 0x9381000A)
    {
        Version = Magic & 0xFFFF;
        u32 Flags = CMDL.ReadLong();
        AABox = CAABox(CMDL);
        BlockCount = CMDL.ReadLong();
        MatSetCount = CMDL.ReadLong();

        // todo: unknown flags
        Loader.mFlags = eShortNormals | eHasTex1;
        if (Flags & 0x10) Loader.mFlags |= eHasVisGroups;
        if (Flags & 0x20) Loader.mFlags |= eShortPositions;

        // Visibility group data
        // Skipping for now - should read in eventually
        if (Flags & 0x10)
        {
            CMDL.Seek(0x4, SEEK_CUR);
            u32 VisGroupCount = CMDL.ReadLong();

            for (u32 iVis = 0; iVis < VisGroupCount; iVis++)
            {
                u32 NameLength = CMDL.ReadLong();
                CMDL.Seek(NameLength, SEEK_CUR);
            }

            CMDL.Seek(0x14, SEEK_CUR); // no clue what any of this is!
        }
    }

    else
    {
        Log::FileError(CMDL.GetSourceString(), "Invalid CMDL magic: " + StringUtil::ToHexString(Magic));
        return nullptr;
    }

    // The rest is common to all CMDL versions
    Loader.mVersion = GetFormatVersion(Version);

    if (Loader.mVersion == eUnknownVersion)
    {
        Log::FileError(CMDL.GetSourceString(), "Unsupported CMDL version: " + StringUtil::ToHexString(Magic));
        return nullptr;
    }

    CModel *pModel = new CModel();
    Loader.mpModel = pModel;
    Loader.mpBlockMgr = new CBlockMgrIn(BlockCount, &CMDL);
    CMDL.SeekToBoundary(32);
    Loader.mpBlockMgr->Init();

    // Materials
    Loader.mMaterials.resize(MatSetCount);
    for (u32 iMat = 0; iMat < MatSetCount; iMat++)
    {
        Loader.mMaterials[iMat] = CMaterialLoader::LoadMaterialSet(CMDL, Loader.mVersion);

        if (Loader.mVersion < eCorruptionProto)
            Loader.mpBlockMgr->ToNextBlock();
    }

    pModel->mMaterialSets = Loader.mMaterials;
    pModel->mHasOwnMaterials = true;
    if (Loader.mVersion >= eCorruptionProto) Loader.mpBlockMgr->ToNextBlock();

    // Mesh
    Loader.LoadAttribArrays(CMDL);
    Loader.LoadSurfaceOffsets(CMDL);
    SModelData *pData = Loader.LoadSurfaces(CMDL);

    pModel->SetData(pData);
    pModel->mAABox = AABox;
    pModel->mHasOwnSurfaces = true;

    // Cleanup
    delete pData;
    delete Loader.mpBlockMgr;
    return pModel;
}

SModelData* CModelLoader::LoadWorldModel(CInputStream& MREA, CBlockMgrIn& BlockMgr, CMaterialSet& MatSet, EGame Version)
{
    CModelLoader Loader;
    Loader.mpBlockMgr = &BlockMgr;
    Loader.mVersion = Version;
    Loader.mFlags = eShortNormals;
    if (Version != eCorruptionProto) Loader.mFlags |= eHasTex1;
    Loader.mMaterials.resize(1);
    Loader.mMaterials[0] = &MatSet;

    Loader.LoadWorldMeshHeader(MREA);
    Loader.LoadAttribArrays(MREA);
    Loader.LoadSurfaceOffsets(MREA);
    SModelData *pData = Loader.LoadSurfaces(MREA);
    pData->mAABox = Loader.mAABox;

    return pData;
}

SModelData* CModelLoader::LoadCorruptionWorldModel(CInputStream &MREA, CBlockMgrIn &BlockMgr, CMaterialSet &MatSet, u32 HeaderSecNum, u32 GPUSecNum, EGame Version)
{
    CModelLoader Loader;
    Loader.mpBlockMgr = &BlockMgr;
    Loader.mVersion = Version;
    Loader.mFlags = eShortNormals;
    Loader.mMaterials.resize(1);
    Loader.mMaterials[0] = &MatSet;
    if (Version == eReturns) Loader.mFlags |= eHasTex1;

    // Corruption/DKCR MREAs split the mesh header and surface offsets away from the actual geometry data so I need two section numbers to read it
    BlockMgr.ToBlock(HeaderSecNum);
    Loader.LoadWorldMeshHeader(MREA);
    Loader.LoadSurfaceOffsets(MREA);

    BlockMgr.ToBlock(GPUSecNum);
    Loader.LoadAttribArrays(MREA);
    SModelData *pData = Loader.LoadSurfaces(MREA);
    pData->mAABox = Loader.mAABox;
    return pData;
}

EGame CModelLoader::GetFormatVersion(u32 Version)
{
    switch (Version)
    {
        case 0x2: return ePrime;
        case 0x3: return eEchoesDemo;
        case 0x4: return eEchoes;
        case 0x5: return eCorruption;
        case 0xA: return eReturns;
        default: return eUnknownVersion;
    }
}