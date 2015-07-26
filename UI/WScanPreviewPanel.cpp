#include "WScanPreviewPanel.h"
#include "ui_WScanPreviewPanel.h"
#include "WStringPreviewPanel.h"
#include <Resource/CScan.h>

WScanPreviewPanel::WScanPreviewPanel(QWidget *parent) :
    IPreviewPanel(parent),
    ui(new Ui::WScanPreviewPanel)
{
    ui->setupUi(this);
    ui->ScanTextWidget->setFrameShape(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum  );
}

WScanPreviewPanel::~WScanPreviewPanel()
{
    delete ui;
}

EResType WScanPreviewPanel::ResType()
{
    return eScan;
}

void WScanPreviewPanel::SetResource(CResource *pRes)
{
    // Clear existing UI
    ui->ScanTypeLabel->clear();
    ui->ScanSpeedLabel->clear();
    ui->ScanCategoryLabel->clear();

    // Set up new UI
    if (pRes->Type() == eScan)
    {
        CScan *pScan = static_cast<CScan*>(pRes);

        // Scan type
        if (pScan->IsImportant())
            ui->ScanTypeLabel->setText("<b><font color=\"red\">Important</font></b>");
        else
            ui->ScanTypeLabel->setText("<b><font color=\"orange\">Normal</font></b>");

        // Scan speed
        if (pScan->IsSlow())
            ui->ScanSpeedLabel->setText("<b><font color=\"blue\">Slow</font></b>");
        else
            ui->ScanSpeedLabel->setText("<b><font color=\"green\">Fast</font></b>");

        // Scan category
        switch (pScan->LogbookCategory())
        {
        case CScan::eNone:
            ui->ScanCategoryLabel->setText("<b>None</b>");
            break;
        case CScan::eChozoLore:
            ui->ScanCategoryLabel->setText("<b>Chozo Lore</b>");
            break;
        case CScan::ePirateData:
            ui->ScanCategoryLabel->setText("<b>Pirate Data</b>");
            break;
        case CScan::eCreatures:
            ui->ScanCategoryLabel->setText("<b>Creatures</b>");
            break;
        case CScan::eResearch:
            ui->ScanCategoryLabel->setText("<b>Research</b>");
            break;
        }

        // Scan text
        ui->ScanTextWidget->SetResource(pScan->ScanText());
    }

    else
        ui->ScanTextWidget->SetResource(nullptr);
}
