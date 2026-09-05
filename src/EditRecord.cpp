#include "EditRecord.h"

#include <QCompleter>
#include <QKeyEvent>
#include <QWidget>

#include "MainWindow.h"
#include "defines.h"

QStringList c_list;

EditRecord::EditRecord(QWidget* parent) : QDialog(parent) {
  m_CategoryList = new CategoryList(this);
}

EditRecord::~EditRecord() { delete m_CategoryList; }

void EditRecord::setDataToUI() {
  QString str = m_Method->getTempSwapStr();
  QStringList list = str.split("|==|");

  timeLabel = list.at(3);
  strAmount = list.at(2).trimmed();
  strCate = list.at(0).trimmed();
  strDeta = list.at(1).trimmed();
}

void EditRecord::on_btnOk_clicked() {
  setDataToUI();

  if (!isAdd) {
    modify_Data(timeLabel, strAmount, strCate, strDeta);

    mw_one->strLatestModify =
        QObject::tr("Modify Item") + " ( " + mw_one->getTabText() + " ) ";

  } else {
    add_Data(mw_one->get_tw(mw_one->ui->tabWidget->currentIndex()), timeLabel,
             strAmount, strCate, strDeta);

    mw_one->strLatestModify =
        QObject::tr("Add Item") + " ( " + mw_one->getTabText() + " ) ";
  }

  // Save Category Text
  QString str = strCate.trimmed();
  int count = m_CategoryList->ui->listWidget->count();
  for (int i = 0; i < count; i++) {
    QString str1 = m_CategoryList->ui->listWidget->item(i)->text().trimmed();
    if (str == str1) {
      m_CategoryList->ui->listWidget->takeItem(i);
      break;
    }
  }

  if (str.length() > 0) {
    QListWidgetItem* item = new QListWidgetItem(str);
    m_CategoryList->ui->listWidget->insertItem(0, item);
  }

  if (tabData->currentIndex() != 0) {
    int curindex = tabData->currentIndex();
    tabData->tabBar()->moveTab(curindex, 0);
  }

  mw_one->startSave("tab");
}

void EditRecord::on_btn7_clicked() { set_Amount("7"); }

void EditRecord::on_btn8_clicked() { set_Amount("8"); }

void EditRecord::on_btn9_clicked() { set_Amount("9"); }

void EditRecord::on_btn4_clicked() { set_Amount("4"); }

void EditRecord::on_btn5_clicked() { set_Amount("5"); }

void EditRecord::on_btn6_clicked() { set_Amount("6"); }

void EditRecord::on_btn1_clicked() { set_Amount("1"); }

void EditRecord::on_btn2_clicked() { set_Amount("2"); }

void EditRecord::on_btn3_clicked() { set_Amount("3"); }

void EditRecord::on_btn0_clicked() { set_Amount("0"); }

void EditRecord::on_btnDot_clicked() { set_Amount("."); }

void EditRecord::on_btnDel_clicked() {}

void EditRecord::set_Amount(QString Number) {}

void EditRecord::on_btnType_clicked() {
  if (!isAndroid) {
    // mw_one->ui->frameEditRecord->hide();
  } else {
    mw_one->ui->frameMain->hide();
    setDataToUI();
  }

  init_MyCategory();
  m_CategoryList->ui->listWidget->setCurrentRow(0);
  // m_Method->setCurrentIndexFromQW(mw_one->ui->qwCategory, 0);
  m_CategoryList->setTypeRenameText();

  int count = 0;  // m_Method->getCountFromQW(mw_one->ui->qwCategory);
  // mw_one->ui->lblTypeInfo->setText(QObject::tr("Total") + " : " +
  //                                  QString::number(count));

  m_CategoryList->setGeometry(mw_one->geometry().x(), mw_one->geometry().y(),
                              mw_one->geometry().width(),
                              mw_one->geometry().height());
  m_CategoryList->show();
}

void EditRecord::saveMyClassification() {
  QSettings Reg(iniDir + "desc.ini", QSettings::IniFormat);

  int count = m_CategoryList->ui->listWidget->count();

  c_list.clear();
  for (int i = 0; i < count; i++) {
    if (isBreak) break;
    c_list.append(m_CategoryList->ui->listWidget->item(i)->text().trimmed());
  }

  removeDuplicates(&c_list);

  for (int i = 0; i < c_list.count(); i++) {
    if (isBreak) break;
    QString str = c_list.at(i);
    if (str.length() > 0)
      Reg.setValue("/CustomDesc/Item" + QString::number(i), str);
  }
  Reg.setValue("/CustomDesc/Count", c_list.count());
}

int EditRecord::removeDuplicates(QStringList* that) {
  int n = that->size();
  int j = 0;
  QSet<QString> seen;
  seen.reserve(n);
  int setSize = 0;
  for (int i = 0; i < n; ++i) {
    const QString& s = that->at(i);
    seen.insert(s);
    if (setSize == seen.size())  // unchanged size => was already seen
      continue;
    ++setSize;

    if (j != i) that->swapItemsAt(i, j);

    ++j;
  }
  if (n != j) that->erase(that->begin() + j, that->end());
  return n - j;
}

void EditRecord::init_MyCategory() {
  QString ini_file;

  ini_file = iniDir + "desc.ini";
  QSettings RegDesc(ini_file, QSettings::IniFormat);

  // m_Method->clearAllBakList(mw_one->ui->qwCategory);

  c_list.clear();
  m_CategoryList->ui->listWidget->clear();
  m_CategoryList->ui->listWidget->setViewMode(QListView::IconMode);
  int descCount = RegDesc.value("/CustomDesc/Count").toInt();
  for (int i = 0; i < descCount; i++) {
    QString str =
        RegDesc.value("/CustomDesc/Item" + QString::number(i)).toString();
    QListWidgetItem* item = new QListWidgetItem(str);

    m_CategoryList->ui->listWidget->addItem(item);
    c_list.append(str);

    // m_Method->addItemToQW(mw_one->ui->qwCategory, str, "", "", "", 0);
  }
}

QString EditRecord::getTime(int h, int m) {
  QString strh, strm, strs;
  if (h < 10)
    strh = "0" + QString::number(h);
  else
    strh = QString::number(h);
  if (m < 10)
    strm = "0" + QString::number(m);
  else
    strm = QString::number(m);
  int s = QTime::currentTime().second();
  if (s < 10)
    strs = "0" + QString::number(s);
  else
    strs = QString::number(s);

  return strh + ":" + strm + ":" + strs;
}

void EditRecord::on_btnClearAmount_clicked() {}

void EditRecord::on_btnClearDesc_clicked() {}

void EditRecord::on_editAmount_textChanged(const QString& arg1) {}

void EditRecord::on_hsH_valueChanged(int value) {}

void EditRecord::on_hsM_valueChanged(int value) {}

void EditRecord::on_btnClearDetails_clicked() {}

void EditRecord::on_editCategory_textChanged(const QString& arg1) {}

void EditRecord::on_editDetails_textChanged() {}

void EditRecord::saveCurrentYearData() {
  QTreeWidget* tw = (QTreeWidget*)tabData->currentWidget();
  if (!tw) {
    return;
  }

  DataManager* dataMgr = new DataManager(iniDir, nullptr);

  QString name = tw->objectName();
  QList<int> listAllYear = getExistingYears(tw);
  int count_year = listAllYear.count();
  int current_year = QDate::currentDate().year();
  bool currentYearSaved = false;
  for (int i = 0; i < count_year; i++) {
    int year = listAllYear.at(i);
    QString file = iniDir + QString::number(year) + "-" + name + ".json";
    if (!QFile::exists(file)) {
      dataMgr->saveData(tw, year);
      if (year == current_year) currentYearSaved = true;
    }
  }
  if (!currentYearSaved) dataMgr->saveData(tw);

  if (tw->topLevelItemCount() == 0) {
    QString file1 =
        iniDir + QString::number(current_year) + "-" + name + ".json";
    qDebug() << file1;
    QFile::remove(file1);
  }

  delete dataMgr;

  return;

  ///////////////////////////////////////////////////////////////////

  QString strCurrentYear = QString::number(QDate::currentDate().year());
  QString iniName = strCurrentYear + "-" + name;

  QString tempFile = iniDir + iniName + ".tmp";
  QString endFile = iniDir + iniName + ".ini";
  QSettings Reg(tempFile, QSettings::IniFormat);

  QString flag;
  flag = "/" + name + "/";

  int count = tw->topLevelItemCount();
  if (count == 0) return;

  Reg.setValue(flag + "TopCount", count);

  int Sn = 0;
  for (int i = 0; i < count; i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    Sn = i + 1;
    if (topItem->text(3) == strCurrentYear) {
      Reg.setValue(flag + QString::number(Sn) + "-topDate", topItem->text(0));
      Reg.setValue(flag + QString::number(Sn) + "-topYear", topItem->text(3));
      Reg.setValue(flag + QString::number(Sn) + "-topFreq", topItem->text(1));
      Reg.setValue(flag + QString::number(Sn) + "-topAmount", topItem->text(2));

      int childCount = topItem->childCount();
      Reg.setValue(flag + QString::number(Sn) + "-childCount", childCount);

      if (childCount > 0) {
        for (int j = 0; j < childCount; j++) {
          if (isBreak) return;
          Reg.setValue(
              flag + QString::number(Sn) + "-childTime" + QString::number(j),
              topItem->child(j)->text(0));
          Reg.setValue(
              flag + QString::number(Sn) + "-childAmount" + QString::number(j),
              topItem->child(j)->text(1));
          Reg.setValue(
              flag + QString::number(Sn) + "-childDesc" + QString::number(j),
              topItem->child(j)->text(2));
          Reg.setValue(
              flag + QString::number(Sn) + "-childDetails" + QString::number(j),
              topItem->child(j)->text(3));
        }
      }
    }
  }

  Reg.sync();
  m_Method->upIniFile(tempFile, endFile);
}

void EditRecord::saveCurrentValue() {
  QString ini_file = privateDir + "editrecord_value.ini";
  QSettings Reg(ini_file, QSettings::IniFormat);

  Reg.setValue("value1", strCate);
  Reg.setValue("value2", strDeta);
  Reg.setValue("value3", strAmount);
}

void EditRecord::setCurrentValue() {
  QString ini_file = privateDir + "editrecord_value.ini";
  QSettings Reg(ini_file, QSettings::IniFormat);

  strCate = Reg.value("value1").toString();
  strDeta = Reg.value("value2").toString();
  strAmount = Reg.value("value3").toString();
}

void EditRecord::monthSum() {
  QString str1, str2;
  str1 = btnYearText;
  str2 = btnMonthText;
  bool b1, b2;
  b1 = isWholeMonth;
  b2 = isDateSection;

  int month = QDate::currentDate().month();
  QString strMonth;
  if (month < 10)
    strMonth = "0" + QString::number(month);
  else
    strMonth = QString::number(month);
  QString strYear = QString::number(QDate::currentDate().year());
  mw_one->m_Report->startReport1(strYear, strMonth);

  while (isReport) QCoreApplication::processEvents();

  btnYearText = str1;
  btnMonthText = str2;
  isWholeMonth = b1;
  isDateSection = b2;
}

QList<int> EditRecord::getExistingYears(QTreeWidget* tw) {
  QSet<int> yearsSet;  // 先用QSet自动去重
  for (int i = 0; i < tw->topLevelItemCount(); ++i) {
    QTreeWidgetItem* item = tw->topLevelItem(i);
    bool isNumber;
    int year = item->text(3).toInt(&isNumber);
    if (isNumber && year >= DataManager::kDataStartYear) {
      yearsSet.insert(year);
    }
  }

  // 手动将QSet元素转换为QList（兼容所有Qt版本）
  QList<int> yearsList;
  // 遍历QSet，逐个插入QList
  for (int year : yearsSet) {
    yearsList.append(year);
  }

  // 排序（升序）
  std::sort(yearsList.begin(), yearsList.end());

  return yearsList;
}

void EditRecord::updateCategoryCompleterList() {
  if (!m_categoryModel) return;

  m_categoryModel->setStringList(c_list);
}

void EditRecord::on_AddRecord() {
  isAdd = true;

  titleAdd = tr("Add") + "  : " + tabData->tabText(tabData->currentIndex());
  timeH = QTime::currentTime().hour();
  timeM = QTime::currentTime().minute();

  timeLabel = getTime(timeH, timeM);

#ifdef Q_OS_ANDROID

  if (isSelectTab) {
    isSelectTab = false;
    setDataToUI();
    openAddEventRecord(titleAdd, strCate, strDeta, strAmount, timeLabel);
  } else {
    openAddEventRecord(titleAdd, "", "", "", timeLabel);
  }
#else
  mw_one->ui->lblTitleEditRecord->setText(titleAdd);
  mw_one->ui->hsH->setValue(timeH);
  mw_one->ui->hsM->setValue(timeM);
  mw_one->ui->lblTime->setText(timeLabel);
  mw_one->ui->editDetails->setText(strDeta);
  mw_one->ui->editCategory->setText(strCate);
  mw_one->ui->editAmount->setText(strAmount);

  mw_one->ui->frameMain->hide();
  mw_one->ui->frameEditRecord->show();
  updateCategoryCompleterList();
#endif
}

bool EditRecord::eventFilter(QObject* watched, QEvent* event) {
  return QDialog::eventFilter(watched, event);
}

void EditRecord::openAddEventRecord(const QString& titleText,
                                    const QString& categoryText,
                                    const QString& noteText,
                                    const QString& amountText,
                                    const QString& timeTagText) {
#ifdef Q_OS_ANDROID
  QString result = c_list.join(u"|==|"_qs);
  m_Method->setTempSwapStr(result);

  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  if (!activity.isValid()) return;

  QJniObject jTitle = QJniObject::fromString(titleText);
  QJniObject jCategory = QJniObject::fromString(categoryText);
  QJniObject jNote = QJniObject::fromString(noteText);
  QJniObject jAmount = QJniObject::fromString(amountText);
  QJniObject jTimeTag = QJniObject::fromString(timeTagText);

  activity.callMethod<void>("openAddEventRecord",
                            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/"
                            "String;Ljava/lang/String;Ljava/lang/String;)V",
                            jTitle.object(), jCategory.object(), jNote.object(),
                            jAmount.object(), jTimeTag.object());
#endif
}

void EditRecord::reeditMainEventData(int index0, int index1) {
  idxMainDate = index0;
  idxMainDateDetail = index1;

  QTreeWidget* tw = mw_one->get_tw(mw_one->ui->tabWidget->currentIndex());
  int maindateIndex = index0;  // getCurrentIndexFromQW(mw_one->ui->qwMainDate);
  int maineventIndex =
      index1;  // getCurrentIndexFromQW(mw_one->ui->qwMainEvent);

  if (maindateIndex < 0) return;
  if (maineventIndex < 0) return;

  // int maindateCount = getCountFromQW(mw_one->ui->qwMainDate);
  int topIndex = tw->topLevelItemCount() - 1 - maindateIndex;
  int childIndex = index1;  // getCurrentIndexFromQW(mw_one->ui->qwMainEvent);

  if (topIndex < 0) return;
  if (childIndex < 0) return;

  tw->setCurrentItem(tw->topLevelItem(topIndex)->child(childIndex));
  mw_one->on_twItemDoubleClicked();
}

void EditRecord::add_Data(QTreeWidget* tw, QString strTime, QString strAmount,
                          QString strCate, QString strDeta) {
  bool isYes = false;

  strDate = m_Method->setCurrentDateValue();

  int topc = tw->topLevelItemCount();
  for (int i = 0; i < topc; i++) {
    QString str = tw->topLevelItem(topc - 1 - i)->text(0) + " " +
                  tw->topLevelItem(topc - 1 - i)->text(3);
    if (mw_one->getYMD(str) == mw_one->getYMD(strDate)) {
      isYes = true;

      QTreeWidgetItem* topItem = tw->topLevelItem(topc - 1 - i);
      QTreeWidgetItem* item11 = new QTreeWidgetItem(topItem);
      item11->setText(0, strTime);
      if (strAmount == "")
        item11->setText(1, "");
      else
        item11->setText(1, QString("%1").arg(strAmount.toDouble(), 0, 'f', 2));

      item11->setText(2, strCate);
      item11->setText(3, strDeta.trimmed());

      int childCount = topItem->childCount();

      topItem->setTextAlignment(1, Qt::AlignHCenter | Qt::AlignVCenter);
      topItem->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
      item11->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);

      // Amount
      double amount = 0;
      for (int m = 0; m < childCount; m++) {
        QString str = topItem->child(m)->text(1);
        amount = amount + str.toDouble();
      }
      QString strAmount = QString("%1").arg(amount, 0, 'f', 2);
      topItem->setText(1, QString::number(childCount));
      if (strAmount == "0.00")
        topItem->setText(2, "");
      else
        topItem->setText(2, strAmount);

      break;
    } else
      break;
  }

  if (!isYes) {
    QTreeWidgetItem* topItem = new QTreeWidgetItem;

    QStringList lista = strDate.split(" ");
    if (lista.count() == 4) {
      QString a = lista.at(0) + " " + lista.at(1) + " " + lista.at(2);
      topItem->setText(0, a);
      topItem->setText(3, lista.at(3));
    }

    tw->addTopLevelItem(topItem);
    QTreeWidgetItem* item11 = new QTreeWidgetItem(topItem);
    item11->setText(0, strTime);
    if (strAmount == "")
      item11->setText(1, "");
    else
      item11->setText(1, QString("%1").arg(strAmount.toDouble(), 0, 'f', 2));
    item11->setText(2, strCate);
    item11->setText(3, strDeta);

    topItem->setTextAlignment(1, Qt::AlignHCenter | Qt::AlignVCenter);
    topItem->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
    item11->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);

    //  Amount
    int child = topItem->childCount();
    double amount = 0;
    for (int m = 0; m < child; m++) {
      QString str = topItem->child(m)->text(1);
      amount = amount + str.toDouble();
    }

    QString strAmount = QString("%1").arg(amount, 0, 'f', 2);
    topItem->setText(1, QString::number(child));
    if (strAmount == "0.00")
      topItem->setText(2, "");
    else
      topItem->setText(2, strAmount);
  }

  int topCount = tw->topLevelItemCount();
  QTreeWidgetItem* topItem = tw->topLevelItem(topCount - 1);
  tw->setCurrentItem(topItem);
  mw_one->m_MainHelper->sort_childItem(topItem->child(0));
  tw->setCurrentItem(topItem->child(topItem->childCount() - 1));

  mw_one->reloadMain();

  m_Method->openMyEventWindow();
}

void EditRecord::modify_Data(QString strTime, QString strAmount,
                             QString strCate, QString strDeta) {
  QTreeWidget* tw = (QTreeWidget*)mw_one->ui->tabWidget->currentWidget();
  QTreeWidgetItem* item = tw->currentItem();
  QTreeWidgetItem* topItem = item->parent();
  QString newtime = strTime;
  if (item->childCount() == 0 && item->parent()->childCount() > 0) {
    item->setText(0, newtime);
    QString sa = strAmount;
    if (sa == "")
      item->setText(1, "");
    else
      item->setText(1, QString("%1").arg(sa.toDouble(), 0, 'f', 2));
    item->setText(2, strCate);
    item->setText(3, strDeta);
    // Amount
    int child = item->parent()->childCount();
    double amount = 0;
    for (int m = 0; m < child; m++) {
      QString str = item->parent()->child(m)->text(1);
      amount = amount + str.toDouble();
    }
    QString strAmount = QString("%1").arg(amount, 0, 'f', 2);
    item->parent()->setTextAlignment(1, Qt::AlignHCenter | Qt::AlignVCenter);
    item->parent()->setText(1, QString::number(child));
    if (strAmount == "0.00")
      item->parent()->setText(2, "");
    else
      item->parent()->setText(2, strAmount);

    int childRow0 = tw->currentIndex().row();
    mw_one->m_MainHelper->sort_childItem(item);

    int childRow1 = 0;
    for (int i = 0; i < topItem->childCount(); i++) {
      QTreeWidgetItem* childItem = topItem->child(i);

      QString time = childItem->text(0).split(".").at(1);
      time = time.trimmed();

      if (time == newtime) {
        childRow1 = i;
        break;
      }
    }

    int newrow;
    int row =
        idxMainDateDetail;  // m_Method->getCurrentIndexFromQW(mw_one->ui->qwMainEvent);
    if (childRow0 - childRow1 == 0) newrow = row;
    if (childRow0 - childRow1 < 0) newrow = row + childRow1 - childRow0;
    if (childRow0 - childRow1 > 0) newrow = row - (childRow0 - childRow1);

    int maindateIndex =
        idxMainDate;  // m_Method->getCurrentIndexFromQW(mw_one->ui->qwMainDate);

    mw_one->reloadMain();

    QTimer::singleShot(100, mw_one, [this, maindateIndex, newrow]() {
      // m_Method->setCurrentIndexFromQW(mw_one->ui->qwMainDate, maindateIndex);

      mw_one->clickMainDate(maindateIndex);
      // m_Method->setCurrentIndexFromQW(mw_one->ui->qwMainEvent, newrow);
    });
  }
}

void EditRecord::showCategorySelectDialog() {
#ifdef Q_OS_ANDROID

  QStringList list1 = c_list;

  QJniObject jArrayList("java/util/ArrayList", "()V");
  for (const QString& item : list1) {
    QJniObject jItem = QJniObject::fromString(item);
    jArrayList.callMethod<bool>("add", "(Ljava/lang/Object;)Z", jItem.object());
  }

  QJniObject instance = QJniObject::getStaticObjectField(
      "com/x/AddEventRecord", "mInstance", "Lcom/x/AddEventRecord;");

  if (instance.isValid()) {
    instance.callMethod<void>("showCategorySelectDialog",
                              "(Ljava/util/ArrayList;)V", jArrayList.object());
  }

#endif
}
