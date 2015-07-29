#include "CFont.h"
#include <Core/CDrawUtil.h>
#include <Core/CRenderer.h>
#include <Core/CResCache.h>
#include <Common/AnimUtil.h>

CDynamicVertexBuffer CFont::smGlyphVertices;
        CIndexBuffer CFont::smGlyphIndices;
                bool CFont::smBuffersInitialized = false;

CFont::CFont() : CResource()
{
}

CFont::~CFont()
{
}

EResType CFont::Type()
{
    return eFont;
}

inline float PtsToFloat(s32 pt)
{
    // This is a bit of an arbitrary number but it works
    // 1 / (1280 / 1.333333f / 2)
    return 0.00208333f * pt;
}

CVector2f CFont::RenderString(std::string String, CRenderer *pRenderer, float,
                              CVector2f, CColor FillColor, CColor StrokeColor, u32 FontSize)
{
    // Not using parameter 3 (float - AspectRatio)
    // Not using parameter 4 (CVector2f - Position)
    // WIP
    if (!smBuffersInitialized) InitBuffers();

    // Shader setup
    CShader *pTextShader = CDrawUtil::GetTextShader();
    pTextShader->SetCurrent();

    GLuint ModelMtxLoc = pTextShader->GetUniformLocation("ModelMtx");
    GLuint ColorLoc = pTextShader->GetUniformLocation("FontColor");
    GLuint LayerLoc = pTextShader->GetUniformLocation("RGBALayer");
    CVector4f FillColor4f = FillColor.ToVector4f();
    CVector4f StrokeColor4f = StrokeColor.ToVector4f();
    mpFontTexture->Bind(0);
    smGlyphVertices.Bind();
    glDisable(GL_DEPTH_TEST);

    // Initialize some more stuff before we start the character loop
    CVector2f PrintHead(-1.f, 1.f);
    CTransform4f PtScale = CTransform4f::ScaleMatrix(PtsToFloat(1));
    SGlyph *pPrevGlyph = nullptr;

    float Scale;
    if (FontSize == CFONT_DEFAULT_SIZE) Scale = 1.f;
    else Scale = (float) FontSize / (mDefaultSize != 0 ? mDefaultSize : 18);

    for (u32 iChar = 0; iChar < String.length(); iChar++)
    {
        // Get character, check for newline
        char Char = String[iChar];

        if (Char == '\n')
        {
            pPrevGlyph = nullptr;
            PrintHead.x = -1;
            PrintHead.y -= (PtsToFloat(mLineHeight) + PtsToFloat(mLineMargin) + PtsToFloat(mUnknown)) * Scale;
            continue;
        }

        // Get glyph
        auto iGlyph = mGlyphs.find(Char);
        if (iGlyph == mGlyphs.end()) continue;
        SGlyph *pGlyph = &iGlyph->second;

        // Apply left padding and kerning
        PrintHead.x += PtsToFloat(pGlyph->LeftPadding) * Scale;

        if (pPrevGlyph)
        {
            if (pPrevGlyph->KerningIndex != -1)
            {
                for (u32 iKern = pPrevGlyph->KerningIndex; iKern < mKerningTable.size(); iKern++)
                {
                    if (mKerningTable[iKern].CharacterA != pPrevGlyph->Character) break;
                    if (mKerningTable[iKern].CharacterB == String[iChar])
                    {
                        PrintHead.x += PtsToFloat(mKerningTable[iKern].Adjust) * Scale;
                        break;
                    }
                }
            }
        }

        // Add a newline if this character goes over the right edge of the screen
        if (PrintHead.x + ((PtsToFloat(pGlyph->PrintAdvance) + PtsToFloat(pGlyph->RightPadding)) * Scale) > 1)
        {
            PrintHead.x = -1;
            PrintHead.y -= (PtsToFloat(mLineHeight) + PtsToFloat(mLineMargin) + PtsToFloat(mUnknown)) * Scale;

            if (Char == ' ') continue;
        }

        float XTrans = PrintHead.x;
        float YTrans = PrintHead.y + ((PtsToFloat(pGlyph->BaseOffset * 2) - PtsToFloat(mVerticalOffset * 2)) * Scale);

        CTransform4f GlyphTransform = PtScale;
        GlyphTransform.Scale(CVector3f((float) pGlyph->Width / 2, (float) pGlyph->Height, 1.f));
        GlyphTransform.Scale(Scale);
        GlyphTransform.Translate(CVector3f(XTrans, YTrans, 0.f));
        CMatrix4f Glyph4f = GlyphTransform.ToMatrix4f();

        // Get glyph layer
        u8 GlyphLayer = pGlyph->RGBAChannel;
        if (mTextureFormat == 3) GlyphLayer *= 2;
        else if (mTextureFormat == 8) GlyphLayer = 3;

        // Load shader uniforms, buffer texture
        glUniformMatrix4fv(ModelMtxLoc, 1, GL_FALSE, (GLfloat*) &Glyph4f);
        smGlyphVertices.BufferAttrib(eTex0, &pGlyph->TexCoords);

        // Draw fill
        glUniform1i(LayerLoc, GlyphLayer);
        glUniform4fv(ColorLoc, 1, &FillColor4f.x);
        smGlyphIndices.DrawElements();

        // Draw stroke
        if ((mTextureFormat == 1) || (mTextureFormat == 3) || (mTextureFormat == 8))
        {
            u8 StrokeLayer;
            if (mTextureFormat == 1) StrokeLayer = 1;
            else if (mTextureFormat == 3) StrokeLayer = GlyphLayer + 1;
            else if (mTextureFormat == 8) StrokeLayer = GlyphLayer - 2;

            glUniform1i(LayerLoc, StrokeLayer);
            glUniform4fv(ColorLoc, 1, &StrokeColor4f.x);
            smGlyphIndices.DrawElements();
        }

        // Update print head
        PrintHead.x += PtsToFloat(pGlyph->PrintAdvance) * Scale;
        PrintHead.x += PtsToFloat(pGlyph->RightPadding) * Scale;
        pPrevGlyph = pGlyph;
    }

    glEnable(GL_DEPTH_TEST);
    return PrintHead;
}

void CFont::InitBuffers()
{
    smGlyphVertices.SetActiveAttribs(ePosition | eTex0);
    smGlyphVertices.SetVertexCount(4);

    CVector3f Vertices[4] = {
        CVector3f( 0.f,  0.f, 0.f),
        CVector3f( 2.f,  0.f, 0.f),
        CVector3f( 0.f, -2.f, 0.f),
        CVector3f( 2.f, -2.f, 0.f)
    };
    smGlyphVertices.BufferAttrib(ePosition, Vertices);

    CVector2f TexCoords[4] = {
        CVector2f(0.f, 0.f),
        CVector2f(1.f, 0.f),
        CVector2f(0.f, 1.f),
        CVector2f(1.f, 1.f)
    };
    smGlyphVertices.BufferAttrib(eTex0, TexCoords);

    smGlyphIndices.Reserve(4);
    smGlyphIndices.AddIndex(0);
    smGlyphIndices.AddIndex(2);
    smGlyphIndices.AddIndex(1);
    smGlyphIndices.AddIndex(3);
    smGlyphIndices.SetPrimitiveType(GL_TRIANGLE_STRIP);

    smBuffersInitialized = true;
}