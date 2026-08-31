#include "Reader.h"
#include "src/MainWindow.h"

void Reader::showCatalogue() {
  closeSelText();
  savePageVPos();

  setPageVPos();
  showInfo();
}

void Reader::gotoCataList(QString htmlFile) {
  for (int i = 0; i < ncxList.count(); i++) {
    QString item = ncxList.at(i);
    QString str1 = item.split("===").at(1);
    // qDebug() << "gotoCataList:" << str1 << htmlFile ;
    if (str1.contains(htmlFile)) {
      currentCataIndex = i;
      break;
    }
  }
}

void Reader::openCataList(QString htmlFile) {
  savePageVPos();

  initLink(htmlFile);
}
