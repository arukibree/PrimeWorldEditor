#ifndef CMASTERTEMPLATE_H
#define CMASTERTEMPLATE_H

#include "CScriptTemplate.h"
#include "CTemplateCategory.h"
#include "../EFormatVersion.h"
#include <Common/types.h>
#include <unordered_map>
#include <tinyxml2.h>

class CMasterTemplate
{
    friend class CTemplateLoader;

    EGame mGame;
    std::string mGameName;
    u32 mVersion;
    bool mFullyLoaded;

    std::unordered_map<u32, CScriptTemplate*> mTemplates;
    std::unordered_map<u32, std::string> mStates;
    std::unordered_map<u32, std::string> mMessages;
    std::vector<CTemplateCategory> mCategories;

    bool mHasPropList;
    std::unordered_map<u32, CPropertyTemplate*> mPropertyList;

    static std::unordered_map<EGame, CMasterTemplate*> smMasterMap;
    static u32 smGameListVersion;

public:
    CMasterTemplate();
    ~CMasterTemplate();
    EGame GetGame();
    u32 NumScriptTemplates();
    u32 NumStates();
    u32 NumMessages();
    CScriptTemplate* TemplateByID(u32 ObjectID);
    CScriptTemplate* TemplateByID(const CFourCC& ObjectID);
    CScriptTemplate* TemplateByIndex(u32 Index);
    std::string StateByID(u32 StateID);
    std::string StateByID(const CFourCC& StateID);
    std::string StateByIndex(u32 Index);
    std::string MessageByID(u32 MessageID);
    std::string MessageByID(const CFourCC& MessageID);
    std::string MessageByIndex(u32 Index);
    CPropertyTemplate* GetProperty(u32 PropertyID);
    bool IsLoadedSuccessfully();

    static CMasterTemplate* GetMasterForGame(EGame Game);
};

// ************ INLINE ************
inline u32 CMasterTemplate::NumScriptTemplates() {
    return mTemplates.size();
}

inline u32 CMasterTemplate::NumStates() {
    return mStates.size();
}

inline u32 CMasterTemplate::NumMessages() {
    return mMessages.size();
}

#endif // CMASTERTEMPLATE_H