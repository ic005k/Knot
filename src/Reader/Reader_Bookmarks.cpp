#include "Reader.h"

void Reader::showBookmarkList() {
  QStringList list = getCurrentBookmarkList();
  m_Method->clearAllBakList(mui->qwBookmark);
  for (int i = 0; i < list.count(); i++) {
    m_Method->addItemToQW(mui->qwBookmark, list.at(i), "", "", "", 0);
  }
}

void Reader::clickBookmarkList(int i) {
  int count = m_Method->getCountFromQW(mui->qwBookmark);
  int index = count - 1 - i;

  QString file = iniDir + "bookini/" + currentBookName + ".ini";
  if (!QFile::exists(file))
    file = privateDir + "bookini/" + currentBookName + ".ini";

  QSettings Reg(file, QSettings::IniFormat);

  if (isText) {
    currentPage =
        Reg.value("/Bookmark/currentPage" + QString::number(index)).toInt();
    textPos = Reg.value("/Bookmark/VPos" + QString::number(index)).toReal();
    isLandscape =
        Reg.value("/Bookmark/isLandscape" + QString::number(index)).toBool();

    if (currentPage <= totalPages) {
      QString txt1 = updateContent();
      setQMLText(txt1);
    }
  }

  if (isEpub) {
    htmlIndex =
        Reg.value("/Bookmark/htmlIndex" + QString::number(index)).toInt();
    textPos = Reg.value("/Bookmark/VPos" + QString::number(index)).toReal();
    isLandscape =
        Reg.value("/Bookmark/isLandscape" + QString::number(index)).toBool();
    if (htmlIndex >= htmlFiles.count()) {
      htmlIndex = 0;
    }

    currentHtmlFile = htmlFiles.at(htmlIndex);
    setQMLHtml(currentHtmlFile, "", "");
  }

  showInfo();

  setQmlLandscape(isLandscape);
  setVPos(textPos);

  mui->qwBookmark->hide();
  mui->qwReader->show();
  mui->btnCatalogue->setEnabled(true);
}

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

QString Reader::getBookmarkTextFromQML() {
  QVariant item;
  QQuickItem* root = mui->qwReader->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "getBookmarkText",
                            Q_RETURN_ARG(QVariant, item));
  QString txt = item.toString();
  if (isZH_CN) {
    txt = txt.left(50);
  }
  return txt + "...";
}
