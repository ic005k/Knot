#include "Reader.h"

void Reader::showCatalogue() {
  closeSelText();
  savePageVPos();

  if (mui->qwCata->isVisible()) {
    mui->lblCataInfo->hide();
    mui->qwCata->hide();
    mui->qwReader->show();
    mui->btnShowBookmark->setEnabled(true);

  } else {
    mui->qwReader->hide();
    mui->lblCataInfo->show();
    mui->qwCata->show();
    mui->btnShowBookmark->setEnabled(false);

    m_Method->clearAllBakList(mui->qwCata);
    for (int i = 0; i < ncxList.count(); i++) {
      QString item = ncxList.at(i);
      QString str0, str1;
      str0 = item.split("===").at(0);
      str1 = item.split("===").at(1);
      m_Method->addItemToQW(mui->qwCata, str0, str1, "", "", 0);
    }
  }

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
  mui->lblCataInfo->hide();
  mui->qwCata->hide();
  mui->qwReader->show();
  mui->btnShowBookmark->setEnabled(true);

  initLink(htmlFile);
  m_Method->clearAllBakList(mui->qwCata);
}
