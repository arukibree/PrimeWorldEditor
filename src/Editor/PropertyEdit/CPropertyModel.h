#ifndef CPROPERTYMODEL_H
#define CPROPERTYMODEL_H

#include <QAbstractItemModel>
#include <Core/Resource/Script/IProperty.h>

class CPropertyModel : public QAbstractItemModel
{
    Q_OBJECT

    CPropertyStruct *mpBaseStruct;

public:
    CPropertyModel(QObject *pParent = 0);
    void SetBaseStruct(CPropertyStruct *pBaseStruct);
    IProperty* PropertyForIndex(const QModelIndex& rkIndex, bool HandleFlaggedPointers) const;
    QModelIndex IndexForProperty(IProperty *pProp) const;

    int columnCount(const QModelIndex& rkParent) const;
    int rowCount(const QModelIndex& rkParent) const;
    QVariant headerData(int Section, Qt::Orientation Orientation, int Role) const;
    QVariant data(const QModelIndex& rkIndex, int Role) const;
    QModelIndex index(int Row, int Column, const QModelIndex& rkParent) const;
    QModelIndex parent(const QModelIndex& rkChild) const;
    Qt::ItemFlags flags(const QModelIndex& rkIndex) const;

    void ArrayAboutToBeResized(const QModelIndex& rkIndex, u32 NewSize);
    void ArrayResized(const QModelIndex& rkIndex, u32 OldSize);
    void ResizeArray(const QModelIndex& rkIndex, u32 NewSize);

public slots:
    void NotifyPropertyModified(IProperty *pProp);
    void NotifyPropertyModified(const QModelIndex& rkIndex);

signals:
    void PropertyModified(const QModelIndex& rkIndex);
};

#endif // CPROPERTYMODEL_H
