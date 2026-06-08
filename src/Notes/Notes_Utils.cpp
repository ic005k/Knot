#include "src/Notes/Notes.h"

QString Notes::getDateTimeStr() {
  int y = QDate::currentDate().year();
  int m = QDate::currentDate().month();
  int d = QDate::currentDate().day();
  int hh = QTime::currentTime().hour();
  int mm = QTime::currentTime().minute();
  int s = QTime::currentTime().second();

  auto pad = [](int n) {
    return n < 10 ? "0" + QString::number(n) : QString::number(n);
  };
  return QString::number(y) + pad(m) + pad(d) + "_" + pad(hh) + pad(mm) +
         pad(s);
}

qreal Notes::getVPos() { return sliderPos; }
qreal Notes::getVHeight() { return textHeight; }
QColor Notes::StringToColor(QString mRgbStr) {
  return QColor(mRgbStr.toUInt(NULL, 16));
}

bool Notes::isSetNewNoteTitle() {
  QString title = m_NotesList->noteTitle;
  return (title.trimmed() == "无标题笔记" ||
          title.trimmed() == "Untitled Note");
}

void Notes::renameTitle(bool isOk) {
  if (m_NotesList->getNoteBookCurrentIndex() < 0) return;
  m_NotesList->setTWCurrentItem();
  if (isOk) {
    m_NotesList->renameCurrentItem(new_title);
    m_NotesList->saveNotesList();
  } else {
    m_NotesList->on_btnRename_clicked();
  }
}

void Notes::delLink(QString link) {
  QString mdBuffers = loadText(currentMDFile);
  mdBuffers.replace(link, "");
  mdBuffers.replace("[]()", "");
  // ... 原有清理逻辑
  StringToFile(mdBuffers, currentMDFile);
}

bool Notes::openUrl(const QString& url) {
#ifdef __linux__
  // Linux 打开浏览器
  QProcess::startDetached("xdg-open", {url});
#endif
  return true;
}

void Notes::openBrowserOnce(const QString& htmlPath) {
  if (isLinux)
    openUrl(htmlPath);
  else
    QDesktopServices::openUrl(QUrl::fromLocalFile(htmlPath));
}
