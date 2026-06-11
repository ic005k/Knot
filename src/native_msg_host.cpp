#include "native_msg_host.h"

#include <QByteArray>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QString>
#include <QtEndian>

// 全局静态文件对象，只打开一次标准输入/输出，避免反复开关句柄
static QFile g_stdinFile;
static QFile g_stdoutFile;

// 1. 获取笔记本列表
QJsonArray getNotebookList() {
  QJsonArray arr;
  arr.append(QJsonObject{{"id", "nb001"}, {"name", "默认笔记本"}});
  arr.append(QJsonObject{{"id", "nb002"}, {"name", "学习笔记"}});
  arr.append(QJsonObject{{"id", "nb003"}, {"name", "网页摘录"}});
  return arr;
}

// 2. 保存笔记
void addWebNote(const QString& notebookId, const QString& tags,
                const QString& title, const QString& url,
                const QString& plainText, const QString& htmlText) {
  Q_UNUSED(notebookId);
  Q_UNUSED(tags);
  Q_UNUSED(title);
  Q_UNUSED(url);
  Q_UNUSED(plainText);
  Q_UNUSED(htmlText);
}

// 3. 回复消息给浏览器扩展
void replyMessage(const QJsonObject& resp) {
  QByteArray json = QJsonDocument(resp).toJson(QJsonDocument::Compact);
  quint32 dataLen = static_cast<quint32>(json.size());

  QByteArray lenBuffer(4, 0);
  qToLittleEndian<quint32>(dataLen, reinterpret_cast<uchar*>(lenBuffer.data()));

  g_stdoutFile.write(lenBuffer);
  g_stdoutFile.write(json);
  // QFile 自带 flush
  g_stdoutFile.flush();
}

// 4. 核心监听循环
void nativeMessageLoop() {
  // 仅初始化一次标准输入、标准输出
  if (!g_stdinFile.isOpen()) {
    g_stdinFile.open(0, QIODevice::ReadOnly);
  }
  if (!g_stdoutFile.isOpen()) {
    g_stdoutFile.open(1, QIODevice::WriteOnly);
  }

  while (!g_stdinFile.atEnd()) {
    QByteArray lenBuf = g_stdinFile.read(4);
    if (lenBuf.size() != 4) break;

    quint32 msgLen = qFromLittleEndian<quint32>(
        reinterpret_cast<const uchar*>(lenBuf.data()));
    QByteArray jsonBuf = g_stdinFile.read(msgLen);
    if (jsonBuf.size() != msgLen) break;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(jsonBuf, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) continue;

    QJsonObject obj = doc.object();
    QString action = obj["action"].toString();
    QJsonObject response;
    response["code"] = 0;

    if (action == "getNotebooks") {
      response["notebooks"] = getNotebookList();
    } else if (action == "saveNote") {
      QString notebookId = obj["notebookId"].toString();
      QString tags = obj["tags"].toString();
      QString title = obj["title"].toString();
      QString url = obj["url"].toString();
      QString text = obj["text"].toString();
      QString html = obj["html"].toString();

      addWebNote(notebookId, tags, title, url, text, html);
      response["msg"] = "save success";
    } else {
      response["code"] = -1;
      response["msg"] = "unknown action";
    }

    replyMessage(response);
  }
}

// 子线程入口
void NativeMsgThread::run() { nativeMessageLoop(); }