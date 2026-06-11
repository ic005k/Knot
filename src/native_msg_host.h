#ifndef NATIVE_MSG_HOST_H
#define NATIVE_MSG_HOST_H

#include <QIODevice>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QStandardPaths>
#include <QThread>

// 通信逻辑函数
QJsonArray getNotebookList();
void addWebNote(const QString& notebookId, const QString& tags,
                const QString& title, const QString& url,
                const QString& plainText, const QString& htmlText);
void replyMessage(const QJsonObject& resp);
void nativeMessageLoop();

// 专用子线程类
class NativeMsgThread : public QThread {
  Q_OBJECT
 protected:
  void run() override;
};

#endif  // NATIVE_MSG_HOST_H