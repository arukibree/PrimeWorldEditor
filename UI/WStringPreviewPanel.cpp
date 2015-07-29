#include "WStringPreviewPanel.h"
#include <QFontMetrics>
#include <QTextLayout>
#include <Resource/CStringTable.h>

WStringPreviewPanel::WStringPreviewPanel(QWidget *pParent) : IPreviewPanel(pParent)
{
    mpLayout = new QVBoxLayout(this);
    mpLayout->setAlignment(Qt::AlignTop);
    mpLayout->setSpacing(0);
    setLayout(mpLayout);
}

WStringPreviewPanel::~WStringPreviewPanel()
{
}

QSize WStringPreviewPanel::sizeHint() const
{
    return QSize(400, 0);
}

EResType WStringPreviewPanel::ResType()
{
    return eStringTable;
}

void WStringPreviewPanel::SetResource(CResource *pRes)
{
    foreach(const QLabel *pLabel, mLabels)
        delete pLabel;
    mLabels.clear();

    if (pRes && (pRes->Type() == eStringTable))
    {
        CStringTable *pString = static_cast<CStringTable*>(pRes);
        mLabels.reserve(pString->GetStringCount());

        for (u32 iStr = 0; iStr < pString->GetStringCount(); iStr++)
        {
            QString text = QString::fromStdWString(pString->GetString(0, iStr));
            QLabel *pLabel = new QLabel(text, this);
            pLabel->setWordWrap(true);
            pLabel->setFrameStyle(QFrame::Plain | QFrame::Box);
            pLabel->setMargin(3);
            mLabels.push_back(pLabel);
            mpLayout->addWidget(pLabel);
        }
    }
}