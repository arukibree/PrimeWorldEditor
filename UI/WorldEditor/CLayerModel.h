#ifndef CLAYERMODEL_H
#define CLAYERMODEL_H

#include <QAbstractListModel>
#include <Resource/CGameArea.h>

class CLayerModel : public QAbstractListModel
{
    CGameArea *mpArea;
    bool mHasGenerateLayer;

public:
    explicit CLayerModel(QObject *pParent = 0);
    ~CLayerModel();
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    void SetArea(CGameArea *pArea);
    CScriptLayer* Layer(const QModelIndex& index) const;
};

#endif // CLAYERMODEL_H