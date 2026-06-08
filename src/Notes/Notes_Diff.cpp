#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "src/Notes/Notes.h"

void Notes::updateDiff(const QString& oldText, const QString& newText) {
  if (oldText.isEmpty()) return;
  QString strDiff = m_NoteDiffManager.createPatchFromTexts(oldText, newText);
  QString cleanedOldText = oldText;
  QString cleanedNewText = newText;
  cleanedOldText = cleanedOldText.replace("\r", "");
  cleanedNewText = cleanedNewText.replace("\r", "");
  QList<Diff> diffs =
      m_NoteDiffManager.computeDiffs(cleanedOldText, cleanedNewText);
  QList<Diff> filteredDiffs = m_NoteDiffManager.filterDiffsForDisplay(diffs, 3);
  QString diffHtml = m_NoteDiffManager.diffsToHtml(filteredDiffs);

  QFileInfo fi(currentMDFile);
  QString diffFilePath = iniDir + "/memo/" + fi.baseName() + ".json";
  appendDiffToFile(diffFilePath, currentMDFile, strDiff, diffHtml);
}

bool Notes::appendDiffToFile(const QString& diffFilePath,
                             const QString& noteFilePath,
                             const QString& strDiff, const QString& diffHtml) {
  QString version = getFileVersion(noteFilePath);
  QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
  QList<QJsonObject> diffList;

  QFile file(diffFilePath);
  if (file.exists()) {
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QByteArray data = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isArray()) {
      QJsonArray arr = doc.array();
      for (int i = 0; i < arr.size(); ++i) {
        if (arr[i].isObject()) diffList.append(arr[i].toObject());
      }
    }
  }

  QJsonObject newDiff;
  newDiff["version"] = version;
  newDiff["modifyTime"] = timestamp;
  newDiff["notePath"] = noteFilePath;
  newDiff["patch"] = strDiff;
  newDiff["htmlDiff"] = diffHtml;
  diffList.append(newDiff);

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
  QJsonArray newArr;
  foreach (const QJsonObject& obj, diffList) newArr.append(obj);
  QTextStream out(&file);
  out << QJsonDocument(newArr).toJson(QJsonDocument::Indented);
  file.close();
  return true;
}

QList<QJsonObject> Notes::loadAllDiffs(const QString& diffFilePath) {
  QList<QJsonObject> diffList;
  QFile file(diffFilePath);
  if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
    return diffList;
  QByteArray data = file.readAll();
  file.close();
  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isArray()) {
    QJsonArray arr = doc.array();
    for (int i = 0; i < arr.size(); ++i) {
      if (arr[i].isObject()) diffList.append(arr[i].toObject());
    }
  }
  return diffList;
}

QString Notes::getFileVersion(const QString& filePath) {
  QFileInfo fileInfo(filePath);
  if (fileInfo.exists()) {
    return fileInfo.lastModified().toString(Qt::ISODate);
  }
  return QDateTime::currentDateTime().toString(Qt::ISODate);
}