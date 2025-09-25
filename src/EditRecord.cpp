#include "EditRecord.h"

#include <QKeyEvent>

#include "MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"

QStringList c_list;

EditRecord::EditRecord(QWidget *parent) : QDialog(parent) {
  m_CategoryList = new CategoryList(this);

  this->installEventFilter(this);

  mui->editCategory->setFocus();
  mui->editDetails->setAcceptRichText(false);

  nH = mui->editCategory->height();

  QFont font = this->font();
  font.setPointSize(22);
  mui->editCategory->setFont(font);
  font.setPointSize(23);
  font.setBold(true);

  mui->editAmount->setFont(font);

  mui->btn0->setFont(font);
  mui->btn1->setFont(font);
  mui->btn2->setFont(font);
  mui->btn3->setFont(font);
  mui->btn4->setFont(font);
  mui->btn5->setFont(font);
  mui->btn6->setFont(font);
  mui->btn7->setFont(font);
  mui->btn8->setFont(font);
  mui->btn9->setFont(font);
  mui->btn0->setFont(font);
  mui->btnDot->setFont(font);
  mui->btnDel_Number->setFont(font);

  font.setPointSize(fontSize);
  mui->editDetails->setFont(font);
  font.setBold(true);
  mui->lblTitleEditRecord->setFont(font);

  QValidator *validator =
      new QRegularExpressionValidator(regxNumber, mui->editAmount);
  mui->editAmount->setValidator(validator);
  mui->editAmount->setAttribute(Qt::WA_InputMethodEnabled, false);
  mui->editAmount->setReadOnly(true);

  mui->editCategory->setPlaceholderText(tr("Please enter a category"));

  lblStyle = mui->lblCategory->styleSheet();

  mui->hsM->setStyleSheet(mui->hsH->styleSheet());

  m_Method->qssSlider = mui->hsH->styleSheet();

  QScroller::grabGesture(mui->editDetails, QScroller::LeftMouseButtonGesture);
  m_Method->setSCrollPro(mui->editDetails);
}

void EditRecord::init() {
  setModal(true);
  setGeometry(mw_one->geometry().x(), mw_one->geometry().y(), mw_one->width(),
              mw_one->height());

  if (isAdd) {
    mui->editCategory->setText("");
    mui->editAmount->setText("");
  }

  show();
}

EditRecord::~EditRecord() { delete m_CategoryList; }

void EditRecord::keyReleaseEvent(QKeyEvent *event) { Q_UNUSED(event); }

void EditRecord::on_btnOk_clicked() {
  mw_one->on_btnBackEditRecord_clicked();

  if (!isAdd) {
    mw_one->modify_Data();

    mw_one->strLatestModify =
        tr("Modify Item") + " ( " + mw_one->getTabText() + " ) ";

  } else {
    mw_one->add_Data(mw_one->get_tw(mui->tabWidget->currentIndex()),
                     mui->lblTime->text(), mui->editAmount->text().trimmed(),
                     mui->editCategory->text().trimmed());

    mw_one->strLatestModify =
        tr("Add Item") + " ( " + mw_one->getTabText() + " ) ";
  }

  mw_one->clickData();

  // Save Category Text
  QString str = mui->editCategory->text().trimmed();
  int count = m_CategoryList->ui->listWidget->count();
  for (int i = 0; i < count; i++) {
    QString str1 = m_CategoryList->ui->listWidget->item(i)->text().trimmed();
    if (str == str1) {
      m_CategoryList->ui->listWidget->takeItem(i);
      break;
    }
  }

  if (str.length() > 0) {
    QListWidgetItem *item = new QListWidgetItem(str);
    m_CategoryList->ui->listWidget->insertItem(0, item);
  }

  if (tabData->currentIndex() != 0) {
    int curindex = tabData->currentIndex();
    tabData->tabBar()->moveTab(curindex, 0);

    mw_one->clearAll();
    for (int i = 0; i < tabData->count(); i++) {
      QString text = tabData->tabText(i);
      mw_one->addItem(text, "", "", "", 0);
    }
  }

  mw_one->updateMainTab();
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

void EditRecord::on_btnDel_clicked() {
  QString str = mui->editAmount->text().trimmed();
  str = str.mid(0, str.length() - 1);
  mui->editAmount->setText(str);
}

void EditRecord::set_Amount(QString Number) {
  QString str = mui->editAmount->text().trimmed();
  if (str == "0.00") mui->editAmount->setText("");
  if (str.split(".").count() == 2 && str != "0.00") {
    QString str0 = str.split(".").at(1);
    if (str0.length() == 2) return;
  }
  mui->editAmount->setText(str + Number);
}

void EditRecord::on_btnCustom_clicked() {
  if (mui->qwCategory->source().isEmpty()) {
    mui->qwCategory->rootContext()->setContextProperty("m_Method", m_Method);
    mui->qwCategory->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/type.qml")));
  }

  this->hide();
  mui->frameEditRecord->hide();
  mui->frameCategory->show();
  init_MyCategory();
  m_CategoryList->ui->listWidget->setCurrentRow(0);
  m_Method->setCurrentIndexFromQW(mui->qwCategory, 0);
  m_Method->setTypeRenameText();

  int count = m_Method->getCountFromQW(mui->qwCategory);
  mui->lblTypeInfo->setText(tr("Total") + " : " + QString::number(count));

  return;

  m_CategoryList->close();
  m_CategoryList = new CategoryList(mw_one->m_EditRecord);

  int h = mw_one->height();
  int w = mw_one->width();
  m_CategoryList->setGeometry(mw_one->geometry().x(), mw_one->geometry().y(), w,
                              h);

  init_MyCategory();
  m_CategoryList->ui->listWidget->setFocus();
  if (m_CategoryList->ui->listWidget->count() > 0)
    m_CategoryList->ui->listWidget->setCurrentRow(0);
  m_CategoryList->ui->editRename->clear();

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

int EditRecord::removeDuplicates(QStringList *that) {
  int n = that->size();
  int j = 0;
  QSet<QString> seen;
  seen.reserve(n);
  int setSize = 0;
  for (int i = 0; i < n; ++i) {
    const QString &s = that->at(i);
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

  m_Method->clearAllBakList(mui->qwCategory);

  c_list.clear();
  m_CategoryList->ui->listWidget->clear();
  m_CategoryList->ui->listWidget->setViewMode(QListView::IconMode);
  int descCount = RegDesc.value("/CustomDesc/Count").toInt();
  for (int i = 0; i < descCount; i++) {
    QString str =
        RegDesc.value("/CustomDesc/Item" + QString::number(i)).toString();
    QListWidgetItem *item = new QListWidgetItem(str);

    m_CategoryList->ui->listWidget->addItem(item);
    c_list.append(str);

    m_Method->addItemToQW(mui->qwCategory, str, "", "", "", 0);
  }
}

void EditRecord::getTime(int h, int m) {
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
  mui->lblTime->setText(strh + ":" + strm + ":" + strs);
}

bool EditRecord::eventFilter(QObject *watch, QEvent *evn) {
  return QWidget::eventFilter(watch, evn);
}

void EditRecord::on_btnClearAmount_clicked() { mui->editAmount->clear(); }

void EditRecord::on_btnClearDesc_clicked() { mui->editCategory->clear(); }

void EditRecord::on_editAmount_textChanged(const QString &arg1) {
  int count = 0;
  for (int i = 0; i < arg1.length(); i++) {
    if (arg1.mid(i, 1) == ".") count++;
    if (count == 2) {
      QString str0 = arg1;
      QString str = str0.mid(0, str0.length() - 1);
      mui->editAmount->setText(str);
      break;
    }
  }

  if (arg1.length() > 0) {
    mui->lblAmount->setStyleSheet(lblStyleHighLight);
    if (!isDark) {
      m_Method->setQLabelImage(mui->lblAmount, nH, nH, ":/res/je_l.svg");
    }
  } else {
    mui->lblAmount->setStyleSheet(lblStyle);
    if (!isDark) {
      m_Method->setQLabelImage(mui->lblAmount, nH, nH, ":/res/je.svg");
    }
  }
}

void EditRecord::on_hsH_valueChanged(int value) {
  getTime(value, mui->hsM->value());
}

void EditRecord::on_hsM_valueChanged(int value) {
  getTime(mui->hsH->value(), value);
}

void EditRecord::on_btnClearDetails_clicked() { mui->editDetails->clear(); }

void EditRecord::on_editCategory_textChanged(const QString &arg1) {
  if (arg1.length() > 0) {
    mui->lblCategory->setStyleSheet(lblStyleHighLight);
    if (!isDark) {
      m_Method->setQLabelImage(mui->lblCategory, nH, nH, ":/res/fl_l.svg");
    }
  } else {
    mui->lblCategory->setStyleSheet(lblStyle);
    if (!isDark) {
      m_Method->setQLabelImage(mui->lblCategory, nH, nH, ":/res/fl.svg");
    }
  }

  QCompleter *completer = new QCompleter(c_list);
  completer->setFilterMode(Qt::MatchContains);
  mui->editCategory->setCompleter(completer);
}

void EditRecord::on_editDetails_textChanged() {
  QString arg1 = mui->editDetails->toPlainText();
  if (arg1.length() > 0) {
    mui->lblDetailsType->setStyleSheet(lblStyleHighLight);
    if (!isDark) {
      m_Method->setQLabelImage(mui->lblDetailsType, nH, nH, ":/res/xq_l.svg");
    }
  } else {
    mui->lblDetailsType->setStyleSheet(lblStyle);
    if (!isDark) {
      m_Method->setQLabelImage(mui->lblDetailsType, nH, nH, ":/res/xq.svg");
    }
  }
}

void EditRecord::saveAdded() {
  QTreeWidget *tw = (QTreeWidget *)tabData->currentWidget();

  QString name = tw->objectName();
  QString iniName = QString::number(QDate::currentDate().year()) + "-" + name;

  QString ini_file = iniDir + iniName + ".ini";
  QSettings Reg(ini_file, QSettings::IniFormat);

  qDebug() << "save added: ini_file=" << ini_file;

  QString flag;
  if (QFile::exists(ini_file)) {
    QString group = Reg.childGroups().at(0);
    if (group.trimmed().length() == 0)
      flag = "/" + name + "/";
    else
      flag = "/" + group + "/";
  } else
    flag = "/" + name + "/";

  int count = tw->topLevelItemCount();
  if (count == 0) return;

  bool isExist = false;

  QTreeWidgetItem *item = tw->currentItem();
  int index = 0;
  if (item->parent() == NULL)
    index = tw->indexOfTopLevelItem(item);
  else
    index = tw->indexOfTopLevelItem(item->parent());

  count = Reg.value(flag + "TopCount", 0).toInt();

  QTreeWidgetItem *topItem = tw->topLevelItem(index);
  QString twText0, twText3, text0, text3;
  twText0 = topItem->text(0);
  twText3 = topItem->text(3);
  text0 = Reg.value(flag + QString::number(count) + "-topDate").toString();
  text3 = Reg.value(flag + QString::number(count) + "-topYear").toString();

  static const QRegularExpression regexRemovePrefix("^\\S+\\s*");
  twText0 = twText0.remove(regexRemovePrefix);  // 移除星期
  text0 = text0.remove(regexRemovePrefix);

  if ((twText0 == text0) && (twText3 == text3)) {
    isExist = true;
  }

  int Sn = count;

  if (!isExist) {
    Reg.setValue(flag + "TopCount", count + 1);
    Sn = count + 1;
  }

  Reg.setValue(flag + QString::number(Sn) + "-topDate", topItem->text(0));
  Reg.setValue(flag + QString::number(Sn) + "-topYear", topItem->text(3));
  Reg.setValue(flag + QString::number(Sn) + "-topFreq", topItem->text(1));
  Reg.setValue(flag + QString::number(Sn) + "-topAmount", topItem->text(2));

  int childCount = topItem->childCount();

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

  Reg.setValue(flag + QString::number(Sn) + "-childCount", childCount);
}

void EditRecord::saveDeleted() {
  QTreeWidget *tw = (QTreeWidget *)tabData->currentWidget();

  QString name = tw->objectName();
  QString iniName = QString::number(QDate::currentDate().year()) + "-" + name;

  QString ini_file = iniDir + iniName + ".ini";
  QSettings Reg(ini_file, QSettings::IniFormat);

  qDebug() << "save deleted: ini_file=" << ini_file;

  QString flag;
  if (QFile::exists(ini_file)) {
    QString group = Reg.childGroups().at(0);
    if (group.trimmed().length() == 0)
      flag = "/" + name + "/";
    else
      flag = "/" + group + "/";
  } else
    flag = "/" + name + "/";

  int count = tw->topLevelItemCount();
  if (count == 0) return;

  int m_count = Reg.value(flag + "TopCount", 0).toInt();
  int Sn = m_count;

  if (isRemovedTopItem) {
    Reg.setValue(flag + "TopCount", m_count - 1);
    return;
  }

  QTreeWidgetItem *topItem = tw->topLevelItem(count - 1);
  int childCount = topItem->childCount();
  if (childCount > 0) {
    Reg.setValue(flag + QString::number(Sn) + "-topDate", topItem->text(0));
    Reg.setValue(flag + QString::number(Sn) + "-topYear", topItem->text(3));
    Reg.setValue(flag + QString::number(Sn) + "-topFreq", topItem->text(1));
    Reg.setValue(flag + QString::number(Sn) + "-topAmount", topItem->text(2));

    Reg.setValue(flag + QString::number(Sn) + "-childCount", childCount);
  }
}

void EditRecord::saveModified() {
  QTreeWidget *tw = (QTreeWidget *)tabData->currentWidget();

  QString name = tw->objectName();
  QString iniName = QString::number(QDate::currentDate().year()) + "-" + name;

  QString ini_file = iniDir + iniName + ".ini";
  QSettings Reg(ini_file, QSettings::IniFormat);

  qDebug() << "save modified: ini_file=" << ini_file;

  QString flag;
  if (QFile::exists(ini_file)) {
    QString group = Reg.childGroups().at(0);
    if (group.trimmed().length() == 0)
      flag = "/" + name + "/";
    else
      flag = "/" + group + "/";
  } else
    flag = "/" + name + "/";

  int top_count = tw->topLevelItemCount();
  if (top_count == 0) return;

  int count = Reg.value(flag + "TopCount", 0).toInt();
  // Reg.setValue(flag + "TopCount", count + 1);

  QTreeWidgetItem *item = tw->currentItem();
  int i = 0;
  if (item->parent() == NULL)
    i = tw->indexOfTopLevelItem(item);
  else
    i = tw->indexOfTopLevelItem(item->parent());
  int Sn = count - (top_count - i) + 1;

  Reg.setValue(flag + QString::number(Sn) + "-topDate",
               tw->topLevelItem(i)->text(0));
  Reg.setValue(flag + QString::number(Sn) + "-topYear",
               tw->topLevelItem(i)->text(3));
  Reg.setValue(flag + QString::number(Sn) + "-topFreq",
               tw->topLevelItem(i)->text(1));
  Reg.setValue(flag + QString::number(Sn) + "-topAmount",
               tw->topLevelItem(i)->text(2));

  qDebug() << "i=" << i << Sn << tw->topLevelItem(i)->text(0);

  int childCount = tw->topLevelItem(i)->childCount();

  if (childCount > 0) {
    for (int j = 0; j < childCount; j++) {
      if (isBreak) return;
      Reg.setValue(
          flag + QString::number(Sn) + "-childTime" + QString::number(j),
          tw->topLevelItem(i)->child(j)->text(0));
      Reg.setValue(
          flag + QString::number(Sn) + "-childAmount" + QString::number(j),
          tw->topLevelItem(i)->child(j)->text(1));
      Reg.setValue(
          flag + QString::number(Sn) + "-childDesc" + QString::number(j),
          tw->topLevelItem(i)->child(j)->text(2));
      Reg.setValue(
          flag + QString::number(Sn) + "-childDetails" + QString::number(j),
          tw->topLevelItem(i)->child(j)->text(3));
    }
  }

  Reg.setValue(flag + QString::number(Sn) + "-childCount", childCount);
}

void EditRecord::saveCurrentYearData() {
  QTreeWidget *tw = (QTreeWidget *)tabData->currentWidget();
  if (!tw) {
    return;
  }

  DataManager *dataMgr = new DataManager(iniDir, nullptr);

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
    QTreeWidgetItem *topItem = tw->topLevelItem(i);
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

  Reg.setValue("value1", mui->editCategory->text());
  Reg.setValue("value2", mui->editDetails->toPlainText());
  Reg.setValue("value3", mui->editAmount->text());
}

void EditRecord::setCurrentValue() {
  QString ini_file = privateDir + "editrecord_value.ini";
  QSettings Reg(ini_file, QSettings::IniFormat);

  mui->editCategory->setText(Reg.value("value1").toString());
  mui->editDetails->setText(Reg.value("value2").toString());
  mui->editAmount->setText(Reg.value("value3").toString());
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

QList<int> EditRecord::getExistingYears(QTreeWidget *tw) {
  QSet<int> yearsSet;  // 先用QSet自动去重
  for (int i = 0; i < tw->topLevelItemCount(); ++i) {
    QTreeWidgetItem *item = tw->topLevelItem(i);
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
