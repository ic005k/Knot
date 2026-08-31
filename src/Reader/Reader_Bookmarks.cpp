#include "Reader.h"
#include "src/MainWindow.h"

void Reader::showOrHideBookmark() {}

void Reader::showBookmarkList() {}

void Reader::clickBookmarkList(int i) {}

QStringList Reader::getCurrentBookmarkList() {
  QStringList list;

  QString file = iniDir + "bookini/" + currentBookName + ".ini";
  if (!QFile::exists(file))
    file = privateDir + "bookini/" + currentBookName + ".ini";

  QSettings Reg(file, QSettings::IniFormat);

  int count = Reg.value("/Bookmark/count", 0).toInt();
  for (int i = 0; i < count; i++) {
    QString txt = Reg.value("/Bookmark/Name" + QString::number(i)).toString();
    list.insert(0, txt);
  }
  return list;
}

QString Reader::getBookmarkTextFromQML() { return txt + "..."; }
