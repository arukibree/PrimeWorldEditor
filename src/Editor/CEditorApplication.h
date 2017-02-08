#ifndef CEDITORAPPLICATION_H
#define CEDITORAPPLICATION_H

#include <QApplication>
#include <QTimer>
#include <QVector>

class CBasicViewport;
class CProjectOverviewDialog;
class CResourceBrowser;
class CResourceEntry;
class CWorldEditor;
class IEditor;

const int gkTickFrequencyMS = 8;

class CEditorApplication : public QApplication
{
    Q_OBJECT

    QTimer mRefreshTimer;
    CWorldEditor *mpWorldEditor;
    CResourceBrowser *mpResourceBrowser;
    CProjectOverviewDialog *mpProjectDialog;
    QVector<IEditor*> mEditorWindows;
    QMap<CResourceEntry*,IEditor*> mEditingMap;
    double mLastUpdate;

public:
    CEditorApplication(int& rArgc, char **ppArgv);
    ~CEditorApplication();
    void InitEditor();
    void EditResource(CResourceEntry *pEntry);
    void NotifyAssetsModified();
    void CookAllDirtyPackages();

    // Accessors
    inline CWorldEditor* WorldEditor() const                { return mpWorldEditor; }
    inline CResourceBrowser* ResourceBrowser() const        { return mpResourceBrowser; }
    inline CProjectOverviewDialog* ProjectDialog() const    { return mpProjectDialog; }

    inline void SetEditorTicksEnabled(bool Enabled)         { Enabled ? mRefreshTimer.start(gkTickFrequencyMS) : mRefreshTimer.stop(); }
    inline bool AreEditorTicksEnabled() const               { return mRefreshTimer.isActive(); }

public slots:
    void AddEditor(IEditor *pEditor);
    void TickEditors();
    void OnEditorClose();

signals:
    void AssetsModified();
};

#define gpEdApp static_cast<CEditorApplication*>(qApp)

#endif // CEDITORAPPLICATION_H
