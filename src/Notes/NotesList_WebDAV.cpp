#include "NotesList.h"

void NotesList::delRemoteWebDAVFiles() {
  if (mui->chkAutoSync->isChecked() && mui->chkWebDAV->isChecked()) {
    // 去重
    QSet<QString> uniqueSet(needDelWebDAVFiles.begin(),
                            needDelWebDAVFiles.end());
    needDelWebDAVFiles = uniqueSet.values();

    int count = needDelWebDAVFiles.count();
    if (count > 0) {
      m_Notes->delRemoteFile(needDelWebDAVFiles);
    } else
      qDebug() << "无须删除远程文件";

    m_Method->setAccessCount(needDelWebDAVFiles.count());
  }
}

void NotesList::needDelNotes() {
  QString jsonFile0 = iniDir + "mainnotes.json";
  int count = 0;

  if (QFile::exists(jsonFile0)) {
    QFile f(jsonFile0);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "打开 mainnotes.json 失败:" << f.errorString();
      return;  // 文件不存在或无法打开
    }

    QByteArray data = f.readAll();
    f.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
      qWarning() << "解析 mainnotes.json 失败（格式无效）";
      return;  // JSON 解析失败
    }

    QJsonObject rootObj = doc.object();

    if (!rootObj.contains("needDelNotes") ||
        !rootObj["needDelNotes"].isArray()) {
      qWarning() << "JSON中缺少needDelNotes数组";
      return;
    }

    // 获取待删除文件数组
    QJsonArray needDelArray = rootObj["needDelNotes"].toArray();

    needDelFiles.clear();
    count = needDelArray.size();
    for (int i = 0; i < count; ++i) {
      QString str1 = needDelArray[i].toString();
      if (str1.isEmpty()) continue;
      needDelFiles.append(str1);
    }

    qDebug() << "needDelFiles=" << needDelFiles.count();
  }

  // del remote file
  for (int i = 0; i < needDelFiles.count(); i++) {
    QString delFile = needDelFiles.at(i);
    QFileInfo fi(delFile);
    QString delFileName = fi.completeBaseName();
    if (i == 0)
      qDebug() << "delFile=" << delFile << "delFileName=" << delFileName;

    for (int j = 0; j < m_Notes->orgRemoteFiles.count(); j++) {
      QString remoteFile = m_Notes->orgRemoteFiles.at(j);

      if (j == 0 && i == 0) qDebug() << "remoteFile=" << remoteFile;

      if (remoteFile.contains(delFileName)) {
        if (!needDelWebDAVFiles.contains(remoteFile))
          needDelWebDAVFiles.append(remoteFile);
        break;
      }
    }
  }

  // del local files
  bool isDelMDOk, isDelJSONOk;
  for (int i = 0; i < needDelFiles.count(); i++) {
    QString mdFile = iniDir + needDelFiles.at(i);
    QString strFile = m_Notes->getCurrentJSON(mdFile);
    isDelMDOk = delFile(mdFile);
    isDelJSONOk = delFile(strFile);

    if (!isAndroid) {
      if (isDelMDOk) qDebug() << "Del Note: " << mdFile << isDelMDOk;
      if (isDelJSONOk) qDebug() << "Del Note: " << strFile << isDelJSONOk;
    }
  }
}

void NotesList::setDelNoteFlag(QString mdFile) {
  int maxCount = 1000;
  needDelFiles.insert(0, mdFile);
  if (needDelFiles.count() > maxCount) {
    needDelFiles.removeAt(maxCount);  // 移除老的
  }
}
