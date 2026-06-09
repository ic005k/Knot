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