#ifndef GLM_CHAT_MODEL_H
#define GLM_CHAT_MODEL_H

#include <QAbstractListModel>
#include <QList>
#include "../core/LlmTypes.h"

namespace glm {

// 消息列表 Model(QAbstractListModel)—— Model/View,UI 不碰裸数据(扩展点 4)。
// 视图(ChatWidget/DebugView)接此 model,数据变自动刷新。
class ChatModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        RoleText = Qt::UserRole + 1,   // 消息文本
        RoleIsUser,                     // 是否用户(气泡方向/颜色)
        RoleTimestamp,                  // 时间戳
    };

    explicit ChatModel(QObject *parent = nullptr);

    void appendMessage(const Message &m);          // 加完整消息(用户/完成态)
    void appendChunkToLast(const QString &text);   // 流式增量追加到末条 assistant
    void clear();
    QList<Message> messages() const { return m_messages; }
    int count() const { return m_messages.size(); }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<Message> m_messages;
};

} // namespace glm

#endif // GLM_CHAT_MODEL_H
