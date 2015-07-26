#ifndef CWORLDLOADER_H
#define CWORLDLOADER_H

#include "../CWorld.h"
#include "../EFormatVersion.h"
#include <FileIO/FileIO.h>
#include <Core/CResCache.h>

class CWorldLoader
{
    CWorld *mpWorld;
    EGame mVersion;

    CWorldLoader();
    void LoadPrimeMLVL(CInputStream& MLVL);
    void LoadReturnsMLVL(CInputStream& MLVL);

public:
    static CWorld* LoadMLVL(CInputStream& MLVL);
    static EGame GetFormatVersion(u32 Version);
};

#endif // CWORLDLOADER_H
