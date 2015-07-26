#ifndef WSTRINGPREVIEWPANEL_H
#define WSTRINGPREVIEWPANEL_H

#include "IPreviewPanel.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QSpacerItem>

class WStringPreviewPanel : public IPreviewPanel
{
    Q_OBJECT

    QLabel *mpTextLabel;
    QVBoxLayout *mpLayout;
    QSpacerItem *mpSpacer;

public:
    explicit WStringPreviewPanel(QWidget *pParent = 0);
    ~WStringPreviewPanel();
    EResType ResType();
    void SetResource(CResource *pRes);
};

#endif // WSTRINGPREVIEWPANEL_H
