#include "ChatModel.h"

namespace glm {

ChatModel::ChatModel(QObject *parent) : QAbstractListModel(parent) {}

void ChatModel::appendMessage(const Message &m)
{
    const int row = m_messages.size();
    beginInsertRows(QModelIndex(), row, row);
    m_messages.append(m);
    endInsertRows();
}

void ChatModel::appendChunkToLast(const QString &text)
{
    if (m_messages.isEmpty()) return;
    m_messages.last().content += text;
    const QModelIndex idx = index(m_messages.size() - 1);
    emit dataChanged(idx, idx, {RoleText});
}

void ChatModel::clear()
{
    beginResetModel();
    m_messages.clear();
    endResetModel();
}

int ChatModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_messages.size();
}

QVariant ChatModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_messages.size()) return {};
    const Message &m = m_messages.at(index.row());
    switch (role) {
    case RoleText:      return m.content;
    case RoleIsUser:    return m.role == Role::User;
    case RoleTimestamp: return m.timestamp;
    }
    return {};
}

QHash<int, QByteArray> ChatModel::roleNames() const
{
    return { {RoleText, "text"}, {RoleIsUser, "isUser"}, {RoleTimestamp, "timestamp"} };
}

} // namespace glm
