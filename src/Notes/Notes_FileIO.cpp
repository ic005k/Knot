#include <QDateTime>
#include <QFileInfo>
#include <QSaveFile>

#include "src/Notes/Notes.h"

void Notes::saveMDFile() {
#ifndef Q_OS_ANDROID
  mw_one->strLatestModify = tr("Modi Notes");
  if (isTextChange) {
    newText = m_EditSource->text();
    newText = formatMDText(newText);

    QFileInfo targetInfo(currentMDFile);
    QString tempFile = targetInfo.dir().filePath(
        targetInfo.fileName() + "." +
        QString::number(QDateTime::currentMSecsSinceEpoch()) + ".tmp");

    bool isOk = StringToFile(newText, tempFile);
    if (isOk) {
      QFile::rename(currentMDFile, currentMDFile + ".bak");
      if (QFile::rename(tempFile, currentMDFile)) {
        qDebug() << "Save Note: " << currentMDFile;
        QFile::remove(currentMDFile + ".bak");
        updateDiff(oldText, newText);
        updateMDFileToSyncLists();
        startBackgroundTaskUpdateNoteIndex(currentMDFile);
      } else {
        qWarning() << "重命名失败，清理临时文件";
        QFile::remove(tempFile);
        QFile::rename(currentMDFile + ".bak", currentMDFile);
      }
    } else {
      qWarning() << "临时文件写入失败";
    }
  }
  isTextChange = false;
#endif
}

void Notes::MD2Html(QString mdFile) {
  QString strmd = loadText(mdFile);
  strmd = strmd.replace("images/", "file://" + iniDir + "memo/images/");
  QString htmlString = markdownToHtmlWithMath(strmd);
  addImagePathToHtml(htmlString);
}

QString Notes::addImagePathToHtml(QString strhtml) {
  QTextEdit* edit = new QTextEdit;
  QPlainTextEdit* edit1 = new QPlainTextEdit;
  strhtml = strhtml.replace("><", ">\n<");
  edit->setPlainText(strhtml);
  QString str, str_2, str_3;
  for (int i = 0; i < edit->document()->lineCount(); i++) {
    str = getTextEditLineText(edit, i);
    str = str.trimmed();
    if (str.mid(0, 4) == "<img" && str.contains("file://")) {
      QString str1 = str;
      QStringList list = str1.split(" ");
      QString strSrc;
      for (int k = 0; k < list.count(); k++) {
        QString s1 = list.at(k);
        if (s1.contains("src=")) {
          strSrc = s1;
          break;
        }
      }
      QStringList list1 = strSrc.split("/memo/");
      if (list1.count() > 1)
        strSrc = "\"file://" + iniDir + "memo/" + list1.at(1) + " ";
      QStringList list2 = str1.split("/memo/");
      if (list2.count() > 1) str_2 = list2.at(1);
      str = "<img src=\"file:///" + iniDir + "memo/" + str_2;
      str = "<a href=" + strSrc + ">" + str + "</a>";
      QStringList list3 = str_2.split("\"");
      if (list3.count() > 0) str_3 = list3.at(0);
      QString imgFile = iniDir + "memo/" + str_3;
      QImage img(imgFile);
      if (img.width() >= mw_one->width() - 25) {
        QString strW = QString::number(mw_one->width() - 25);
        QString a1 = "width = ";
        str = str.replace("/>", a1 + "\"" + strW + "\"" + " />");
      }
    }
    edit1->appendPlainText(str);
  }
  QString strEnd = edit1->toPlainText();
  edit1->clear();
  edit1->setPlainText(strEnd);
  m_Reader->PlainTextEditToFile(edit1, htmlFileName);
  delete edit;
  delete edit1;
  return strEnd;
}

QString Notes::formatMDText(QString text) {
  static const QRegularExpression re("\n{3,}");
  return text.replace(re, "\n\n");
}

void Notes::saveEditorState(const QString& filePath) {
  Q_UNUSED(filePath);
#ifndef Q_OS_ANDROID
  QSettings settings(privateDir + "editor_config.ini", QSettings::IniFormat);

  settings.setValue("x", this->x());
  settings.setValue("y", this->y());
  settings.setValue("w", this->width());
  settings.setValue("h", this->height());

  QString groupName = "Documents/" + QFileInfo(filePath).canonicalFilePath();
  groupName.replace("/", "_");
  settings.beginGroup(groupName);
  int line, index;
  m_EditSource->getCursorPosition(&line, &index);
  settings.setValue("cursorLine", line);
  settings.setValue("cursorIndex", index);
  settings.setValue("vsbar",
                    m_EditSource->verticalScrollBar()->sliderPosition());

  settings.endGroup();
#endif
}

void Notes::restoreEditorState(const QString& filePath) {
  Q_UNUSED(filePath);
#ifndef Q_OS_ANDROID
  QSettings settings(privateDir + "editor_config.ini", QSettings::IniFormat);

  int x, y, w, h;
  x = settings.value("x", 0).toInt();
  y = settings.value("y", 20).toInt();
  w = settings.value("w", this->width()).toInt();
  h = settings.value("h", this->height()).toInt();

  move(x, y);
  resize(w, h);

  QString groupName = "Documents/" + QFileInfo(filePath).canonicalFilePath();
  groupName.replace("/", "_");
  settings.beginGroup(groupName);

  int line = settings.value("cursorLine", 0).toInt();
  int index = settings.value("cursorIndex", 0).toInt();
  int vsbar = settings.value("vsbar", 0).toInt();
  m_EditSource->verticalScrollBar()->setSliderPosition(vsbar);
  m_EditSource->setCursorPosition(line, index);
  settings.endGroup();
#endif
}