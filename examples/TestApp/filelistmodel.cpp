#include "filelistmodel.h"

FileListModel::FileListModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

bool FileListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)

    beginInsertRows(parent, row, row+count);

    while (count > 0) {
        files.insert(row, QString());
        count--;
    }

    endInsertRows();

    return true;
}

bool FileListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)

    beginRemoveRows(parent, row, row+count);

    while (count > 0) {
        files.removeAt(row);
        count--;
    }

    endRemoveRows();

    return true;
}

int FileListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return files.size();
}

QVariant FileListModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
        return files.value(index.row()).fileName();

    return QVariant();
}

bool FileListModel::setData(const QModelIndex &index, const QVariant &value,
                            int role)
{
    if (role == Qt::EditRole)
        files[index.row()] = QFileInfo(value.toString());

    emit dataChanged(index, index);

    return true;
}

QList<QFileInfo> FileListModel::fileList() const
{
    return files;
}
