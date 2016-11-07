#include "CWorldCooker.h"
#include "Core/GameProject/DependencyListBuilders.h"

CWorldCooker::CWorldCooker()
{
}

// ************ STATIC ************
bool CWorldCooker::CookMLVL(CWorld *pWorld, IOutputStream& rMLVL)
{
    ASSERT(rMLVL.IsValid());
    EGame Game = pWorld->Game();

    // MLVL Header
    rMLVL.WriteLong(0xDEAFBABE);
    rMLVL.WriteLong( GetMLVLVersion(pWorld->Game()) );

    CAssetID WorldNameID = pWorld->mpWorldName ? pWorld->mpWorldName->ID() : CAssetID::InvalidID(Game);
    CAssetID DarkWorldNameID = pWorld->mpDarkWorldName ? pWorld->mpDarkWorldName->ID() : CAssetID::InvalidID(Game);
    CAssetID SaveWorldID = pWorld->mpSaveWorld ? pWorld->mpSaveWorld->ID() : CAssetID::InvalidID(Game);
    CAssetID DefaultSkyID = pWorld->mpDefaultSkybox ? pWorld->mpDefaultSkybox->ID() : CAssetID::InvalidID(Game);

    WorldNameID.Write(rMLVL);

    if (Game == eEchoesDemo || Game == eEchoes)
    {
        DarkWorldNameID.Write(rMLVL);
        rMLVL.WriteLong(0);
    }

    SaveWorldID.Write(rMLVL);
    DefaultSkyID.Write(rMLVL);

    // Memory Relays
    if (Game == ePrime)
    {
        rMLVL.WriteLong( pWorld->mMemoryRelays.size() );

        for (u32 iMem = 0; iMem < pWorld->mMemoryRelays.size(); iMem++)
        {
            CWorld::SMemoryRelay& rRelay = pWorld->mMemoryRelays[iMem];
            rMLVL.WriteLong(rRelay.InstanceID);
            rMLVL.WriteLong(rRelay.TargetID);
            rMLVL.WriteShort(rRelay.Message);
            rMLVL.WriteBool(rRelay.Active);
        }
    }

    // Areas
    rMLVL.WriteLong(pWorld->mAreas.size());
    if (Game <= ePrime) rMLVL.WriteLong(1); // Unknown
    std::set<CAssetID> AudioGroups;

    for (u32 iArea = 0; iArea < pWorld->mAreas.size(); iArea++)
    {
        // Area Header
        CWorld::SArea& rArea = pWorld->mAreas[iArea];
        CResourceEntry *pAreaEntry = gpResourceStore->FindEntry(rArea.AreaResID);
        ASSERT(pAreaEntry && pAreaEntry->ResourceType() == eArea);

        CAssetID AreaNameID = rArea.pAreaName ? rArea.pAreaName->ID() : CAssetID::InvalidID(Game);
        AreaNameID.Write(rMLVL);
        rArea.Transform.Write(rMLVL);
        rArea.AetherBox.Write(rMLVL);
        rArea.AreaResID.Write(rMLVL);
        rArea.AreaID.Write(rMLVL);

        // Attached Areas
        if (Game <= eCorruption)
        {
            rMLVL.WriteLong( rArea.AttachedAreaIDs.size() );

            for (u32 iAttach = 0; iAttach < rArea.AttachedAreaIDs.size(); iAttach++)
                rMLVL.WriteShort(rArea.AttachedAreaIDs[iAttach]);
        }

        // Dependencies
        if (Game <= eEchoes)
        {
            std::list<CAssetID> Dependencies;
            std::list<u32> LayerDependsOffsets;
            CAreaDependencyListBuilder Builder(pAreaEntry);
            Builder.BuildDependencyList(Dependencies, LayerDependsOffsets, &AudioGroups);

            rMLVL.WriteLong(0);
            rMLVL.WriteLong( Dependencies.size() );

            for (auto Iter = Dependencies.begin(); Iter != Dependencies.end(); Iter++)
            {
                CAssetID ID = *Iter;
                CResourceEntry *pEntry = gpResourceStore->FindEntry(ID);
                ID.Write(rMLVL);
                pEntry->CookedExtension().Write(rMLVL);
            }

            rMLVL.WriteLong(LayerDependsOffsets.size());

            for (auto Iter = LayerDependsOffsets.begin(); Iter != LayerDependsOffsets.end(); Iter++)
                rMLVL.WriteLong(*Iter);
        }

        // Docks
        if (Game <= eCorruption)
        {
            rMLVL.WriteLong( rArea.Docks.size() );

            for (u32 iDock = 0; iDock < rArea.Docks.size(); iDock++)
            {
                CWorld::SArea::SDock& rDock = rArea.Docks[iDock];
                rMLVL.WriteLong( rDock.ConnectingDocks.size() );

                for (u32 iCon = 0; iCon < rDock.ConnectingDocks.size(); iCon++)
                {
                    CWorld::SArea::SDock::SConnectingDock& rConDock = rDock.ConnectingDocks[iCon];
                    rMLVL.WriteLong(rConDock.AreaIndex);
                    rMLVL.WriteLong(rConDock.DockIndex);
                }

                rMLVL.WriteLong( rDock.DockCoordinates.size() );

                for (u32 iCoord = 0; iCoord < rDock.DockCoordinates.size(); iCoord++)
                    rDock.DockCoordinates[iCoord].Write(rMLVL);
            }
        }

        // Module Dependencies
        if (Game == eEchoesDemo || Game == eEchoes)
        {
            std::vector<TString> ModuleNames;
            std::vector<u32> ModuleLayerOffsets;
            CAreaDependencyTree *pAreaDeps = static_cast<CAreaDependencyTree*>(pAreaEntry->Dependencies());
            pAreaDeps->GetModuleDependencies(Game, ModuleNames, ModuleLayerOffsets);

            rMLVL.WriteLong(ModuleNames.size());

            for (u32 iMod = 0; iMod < ModuleNames.size(); iMod++)
                rMLVL.WriteString(ModuleNames[iMod].ToStdString());

            rMLVL.WriteLong(ModuleLayerOffsets.size());

            for (u32 iOff = 0; iOff < ModuleLayerOffsets.size(); iOff++)
                rMLVL.WriteLong(ModuleLayerOffsets[iOff]);
        }

        // Internal Name
        if (Game >= eEchoesDemo)
            rMLVL.WriteString(rArea.InternalName.ToStdString());
    }

    if (Game <= eCorruption)
    {
        // World Map
        CAssetID MapWorldID = pWorld->mpMapWorld ? pWorld->mpMapWorld->ID() : CAssetID::skInvalidID32;
        MapWorldID.Write(rMLVL);

        // Script Layer - unused in all retail builds but this will need to be supported eventually to properly support the MP1 demo
        rMLVL.WriteByte(0);
        rMLVL.WriteLong(0);
    }

    // Audio Groups
    if (Game <= ePrime)
    {
        // Debug: make sure our generated list matches the original, no missing or extra audio groups
        std::set<CAssetID> OriginalGroups;

        for (u32 iGrp = 0; iGrp < pWorld->mAudioGrps.size(); iGrp++)
        {
            CWorld::SAudioGrp& rAudioGroup = pWorld->mAudioGrps[iGrp];
            OriginalGroups.insert(rAudioGroup.ResID);

            if (AudioGroups.find(rAudioGroup.ResID) == AudioGroups.end())
            {
                CResourceEntry *pEntry = gpResourceStore->FindEntry(rAudioGroup.ResID);
                Log::Error("Missing audio group: " + pEntry->Name().ToUTF8());
            }
        }

        for (auto It = AudioGroups.begin(); It != AudioGroups.end(); It++)
        {
            if (OriginalGroups.find(*It) == OriginalGroups.end())
            {
                CResourceEntry *pEntry = gpResourceStore->FindEntry(*It);
                Log::Error("Extra audio group: " + pEntry->Name().ToUTF8());
            }
        }

        // Create sorted list of audio groups (sort by group ID)
        std::vector<CAudioGroup*> SortedAudioGroups;

        for (auto It = AudioGroups.begin(); It != AudioGroups.end(); It++)
        {
            CAudioGroup *pGroup = (CAudioGroup*) gpResourceStore->LoadResource(*It, "AGSC");
            ASSERT(pGroup);
            SortedAudioGroups.push_back(pGroup);
        }

        std::sort(SortedAudioGroups.begin(), SortedAudioGroups.end(), [](CAudioGroup *pLeft, CAudioGroup *pRight) -> bool {
            return pLeft->GroupID() < pRight->GroupID();
        });

        // Write sorted audio group list to file
        rMLVL.WriteLong(SortedAudioGroups.size());

        for (u32 iGrp = 0; iGrp < SortedAudioGroups.size(); iGrp++)
        {
            CAudioGroup *pGroup = SortedAudioGroups[iGrp];
            rMLVL.WriteLong(pGroup->GroupID());
            pGroup->ID().Write(rMLVL);
        }

#if 0
        rMLVL.WriteLong(pWorld->mAudioGrps.size());

        for (u32 iGrp = 0; iGrp < pWorld->mAudioGrps.size(); iGrp++)
        {
            CWorld::SAudioGrp& rAudioGroup = pWorld->mAudioGrps[iGrp];
            rMLVL.WriteLong(rAudioGroup.GroupID);
            rAudioGroup.ResID.Write(rMLVL);
        }
#endif

        rMLVL.WriteByte(0);
    }

    // Layers
    rMLVL.WriteLong(pWorld->mAreas.size());
    std::vector<TString> LayerNames;
    std::vector<u32> LayerNameOffsets;

    // Layer Flags
    for (u32 iArea = 0; iArea < pWorld->mAreas.size(); iArea++)
    {
        CWorld::SArea& rArea = pWorld->mAreas[iArea];
        LayerNameOffsets.push_back(LayerNames.size());
        rMLVL.WriteLong(rArea.Layers.size());

        u64 LayerActiveFlags = -1;

        for (u32 iLyr = 0; iLyr < rArea.Layers.size(); iLyr++)
        {
            CWorld::SArea::SLayer& rLayer = rArea.Layers[iLyr];
            if (!rLayer.Active)
                LayerActiveFlags &= ~(1 << iLyr);

            LayerNames.push_back(rLayer.LayerName);
        }

        rMLVL.WriteLongLong(LayerActiveFlags);
    }

    // Layer Names
    rMLVL.WriteLong(LayerNames.size());

    for (u32 iLyr = 0; iLyr < LayerNames.size(); iLyr++)
        rMLVL.WriteString(LayerNames[iLyr].ToStdString());

    // todo: Layer Saved State IDs go here for MP3/DKCR; need support for saved state IDs to implement
    if (Game == eCorruption || Game == eReturns)
    {
    }

    // Layer Name Offsets
    rMLVL.WriteLong(LayerNameOffsets.size());

    for (u32 iOff = 0; iOff < LayerNameOffsets.size(); iOff++)
        rMLVL.WriteLong(LayerNameOffsets[iOff]);

    return true;
}

u32 CWorldCooker::GetMLVLVersion(EGame Version)
{
    switch (Version)
    {
    case ePrimeDemo:  return 0xD;
    case ePrime:      return 0x11;
    case eEchoesDemo: return 0x14;
    case eEchoes:     return 0x17;
    case eCorruption: return 0x19;
    case eReturns:    return 0x1B;
    default:          return 0;
    }
}
