#include "EditRecord.h"

#include <QCompleter>
#include <QKeyEvent>
#include <QWidget>

#include "MainWindow.h"
#include "defines.h"

QStringList c_list;

EditRecord::EditRecord(QWidget* parent) : QDialog(parent) {
  m_CategoryList = new CategoryList(this);

  initCategoryCompleter();

  mw_one->ui->editCategory->setFocus();
  mw_one->ui->editDetails->setAcceptRichText(false);

  mw_one->ui->btnBackEditRecord->setFixedHeight(45);
  mw_one->ui->btnType->setFixedHeight(45);
  mw_one->ui->btnOkEditRecord->setFixedHeight(45);

  nH = mw_one->ui->editCategory->height();

  QFont font = this->font();
  font.setPointSize(22);
  mw_one->ui->editCategory->setFont(font);
  if (isAndroid)
    font.setPointSize(40);
  else {
    font.setPointSize(12);
    mw_one->ui->btn0->setMinimumHeight(0);
    mw_one->ui->btn1->setMinimumHeight(0);
    mw_one->ui->btn2->setMinimumHeight(0);
    mw_one->ui->btn3->setMinimumHeight(0);
    mw_one->ui->btn4->setMinimumHeight(0);
    mw_one->ui->btn5->setMinimumHeight(0);
    mw_one->ui->btn6->setMinimumHeight(0);
    mw_one->ui->btn7->setMinimumHeight(0);
    mw_one->ui->btn8->setMinimumHeight(0);
    mw_one->ui->btn9->setMinimumHeight(0);
    mw_one->ui->btnDot->setMinimumHeight(0);
    mw_one->ui->btnDel_Number->setMinimumHeight(0);
  }
  font.setBold(true);

  mw_one->ui->editAmount->setFont(font);

  mw_one->ui->btn0->setFont(font);
  mw_one->ui->btn1->setFont(font);
  mw_one->ui->btn2->setFont(font);
  mw_one->ui->btn3->setFont(font);
  mw_one->ui->btn4->setFont(font);
  mw_one->ui->btn5->setFont(font);
  mw_one->ui->btn6->setFont(font);
  mw_one->ui->btn7->setFont(font);
  mw_one->ui->btn8->setFont(font);
  mw_one->ui->btn9->setFont(font);
  mw_one->ui->btn0->setFont(font);
  mw_one->ui->btnDot->setFont(font);
  mw_one->ui->btnDel_Number->setFont(font);

  font.setPointSize(fontSize);
  mw_one->ui->editDetails->setFont(font);
  font.setBold(true);
  mw_one->ui->lblTitleEditRecord->setFont(font);
  mw_one->ui->lblTitleEditRecord->setFixedHeight(50);

  QValidator* validator =
      new QRegularExpressionValidator(regxNumber, mw_one->ui->editAmount);
  mw_one->ui->editAmount->setValidator(validator);
  mw_one->ui->editAmount->setAttribute(Qt::WA_InputMethodEnabled, false);
  mw_one->ui->editAmount->setReadOnly(true);

  mw_one->ui->editCategory->setPlaceholderText(QObject::tr("Enter a category"));
  mw_one->ui->editDetails->setPlaceholderText(QObject::tr("Enter notes"));

  lblStyle = mw_one->ui->lblCategory->styleSheet();

  mw_one->ui->hsM->setStyleSheet(mw_one->ui->hsH->styleSheet());

  m_Method->qssSlider = mw_one->ui->hsH->styleSheet();
}

EditRecord::~EditRecord() { delete m_CategoryList; }

void EditRecord::setDataToUI() {
  QString str = m_Method->getTempSwapStr();
  QStringList list = str.split("|==|");
  mw_one->ui->editCategory->setText(list.at(0));
  mw_one->ui->editDetails->setText(list.at(1));
  mw_one->ui->editAmount->setText(list.at(2));
  mw_one->ui->lblTime->setText(list.at(3));
}

void EditRecord::on_btnOk_clicked() {
  if (!isAndroid)
    mw_one->on_btnBackEditRecord_clicked();
  else {
    setDataToUI();
  }

  if (!isAdd) {
    modify_Data();

    mw_one->strLatestModify =
        QObject::tr("Modify Item") + " ( " + mw_one->getTabText() + " ) ";

  } else {
    mw_one->add_Data(mw_one->get_tw(mw_one->ui->tabWidget->currentIndex()),
                     mw_one->ui->lblTime->text(),
                     mw_one->ui->editAmount->text().trimmed(),
                     mw_one->ui->editCategory->text().trimmed());

    mw_one->strLatestModify =
        QObject::tr("Add Item") + " ( " + mw_one->getTabText() + " ) ";
  }

  // Save Category Text
  QString str = mw_one->ui->editCategory->text().trimmed();
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

    mw_one->clearAll();
    for (int i = 0; i < tabData->count(); i++) {
      QString text = tabData->tabText(i);
      // mw_one->addItem(text, "", "", "", 0);
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
  QString str = mw_one->ui->editAmount->text().trimmed();
  str = str.mid(0, str.length() - 1);
  mw_one->ui->editAmount->setText(str);
}

void EditRecord::set_Amount(QString Number) {
  QString str = mw_one->ui->editAmount->text().trimmed();
  if (str == "0.00") mw_one->ui->editAmount->setText("");
  if (str.split(".").count() == 2 && str != "0.00") {
    QString str0 = str.split(".").at(1);
    if (str0.length() == 2) return;
  }
  mw_one->ui->editAmount->setText(str + Number);
}

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
  mw_one->ui->lblTime->setText(strh + ":" + strm + ":" + strs);
}

void EditRecord::on_btnClearAmount_clicked() {
  mw_one->ui->editAmount->clear();
}

void EditRecord::on_btnClearDesc_clicked() {
  mw_one->ui->editCategory->clear();
}

void EditRecord::on_editAmount_textChanged(const QString& arg1) {
  int count = 0;
  for (int i = 0; i < arg1.length(); i++) {
    if (arg1.mid(i, 1) == ".") count++;
    if (count == 2) {
      QString str0 = arg1;
      QString str = str0.mid(0, str0.length() - 1);
      mw_one->ui->editAmount->setText(str);
      break;
    }
  }

  if (arg1.length() > 0) {
    mw_one->ui->lblAmount->setStyleSheet(lblStyleHighLight);
    if (!isDark) {
      m_Method->setQLabelImage(mw_one->ui->lblAmount, nH, nH, ":/res/je_l.svg");
    }
  } else {
    mw_one->ui->lblAmount->setStyleSheet(lblStyle);
    if (!isDark) {
      m_Method->setQLabelImage(mw_one->ui->lblAmount, nH, nH, ":/res/je.svg");
    }
  }
}

void EditRecord::on_hsH_valueChanged(int value) {
  getTime(value, mw_one->ui->hsM->value());
}

void EditRecord::on_hsM_valueChanged(int value) {
  getTime(mw_one->ui->hsH->value(), value);
}

void EditRecord::on_btnClearDetails_clicked() {
  mw_one->ui->editDetails->clear();
}

void EditRecord::on_editCategory_textChanged(const QString& arg1) {
  if (arg1.length() > 0) {
    mw_one->ui->lblCategory->setStyleSheet(lblStyleHighLight);
    if (!isDark) {
      m_Method->setQLabelImage(mw_one->ui->lblCategory, nH, nH,
                               ":/res/fl_l.svg");
    }
  } else {
    mw_one->ui->lblCategory->setStyleSheet(lblStyle);
    if (!isDark) {
      m_Method->setQLabelImage(mw_one->ui->lblCategory, nH, nH, ":/res/fl.svg");
    }
  }
}

void EditRecord::on_editDetails_textChanged() {
  QString arg1 = mw_one->ui->editDetails->toPlainText();
  if (arg1.length() > 0) {
    mw_one->ui->lblDetailsType->setStyleSheet(lblStyleHighLight);
    if (!isDark) {
      m_Method->setQLabelImage(mw_one->ui->lblDetailsType, nH, nH,
                               ":/res/xq_l.svg");
    }
  } else {
    mw_one->ui->lblDetailsType->setStyleSheet(lblStyle);
    if (!isDark) {
      m_Method->setQLabelImage(mw_one->ui->lblDetailsType, nH, nH,
                               ":/res/xq.svg");
    }
  }
}

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

  Reg.setValue("value1", mw_one->ui->editCategory->text());
  Reg.setValue("value2", mw_one->ui->editDetails->toPlainText());
  Reg.setValue("value3", mw_one->ui->editAmount->text());
}

void EditRecord::setCurrentValue() {
  QString ini_file = privateDir + "editrecord_value.ini";
  QSettings Reg(ini_file, QSettings::IniFormat);

  mw_one->ui->editCategory->setText(Reg.value("value1").toString());
  mw_one->ui->editDetails->setText(Reg.value("value2").toString());
  mw_one->ui->editAmount->setText(Reg.value("value3").toString());
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

void EditRecord::initCategoryCompleter() {
  // 1. Model
  m_categoryModel = new QStringListModel(this);
  m_categoryModel->setStringList(c_list);

  // 2. Completer（不传 c_list，显式绑定 model）
  completer = new QCompleter(this);
  completer->setFilterMode(Qt::MatchContains);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  completer->setModel(m_categoryModel);

  // 3. Popup 嵌入为子控件（Android 规避 RHI 崩溃，桌面端行为等价）
  QWidget* popupParent = mw_one->ui->frameEditRecord;
  QAbstractItemView* popup = completer->popup();
  popup->setParent(popupParent);
  popup->setWindowFlags(Qt::Widget);
  popup->setAttribute(Qt::WA_ShowWithoutActivating);
  popup->verticalScrollBar()->hide();
  popup->hide();

  // 4. 手动触发补全 + 定位
  connect(mw_one->ui->editCategory, &QLineEdit::textChanged, this,
          [this, popup, popupParent](const QString& text) {
            if (text.isEmpty()) {
              popup->hide();
              return;
            }

            completer->setCompletionPrefix(text);
            if (completer->completionCount() > 0) {
              QPoint pos = mw_one->ui->editCategory->mapTo(
                  popupParent, QPoint(0, mw_one->ui->editCategory->height()));
              int rowH = popup->sizeHintForRow(0);
              int visibleRows = qMin(completer->completionCount(), 8);
              popup->setGeometry(pos.x(), pos.y(),
                                 mw_one->ui->editCategory->width(),
                                 rowH * visibleRows);
              popup->raise();
              popup->show();
            } else {
              popup->hide();
            }
          });

  // 5. 选中项填入
  connect(completer, QOverload<const QString&>::of(&QCompleter::activated),
          this, [this, popup](const QString& text) {
            mw_one->ui->editCategory->setText(text);

            popup->hide();
          });

  // ✅ 安装事件过滤器监听父容器隐藏
  popupParent->installEventFilter(this);

  // ⚠ 不调用 setCompleter()，避免触发原生弹窗路径
}

void EditRecord::updateCategoryCompleterList() {
  if (!m_categoryModel) return;

  m_categoryModel->setStringList(c_list);
}

void EditRecord::on_AddRecord() {
  isAdd = true;

  mw_one->ui->lblTitleEditRecord->setText(
      tr("Add") + "  : " + tabData->tabText(tabData->currentIndex()));

  mw_one->ui->hsH->setValue(QTime::currentTime().hour());
  mw_one->ui->hsM->setValue(QTime::currentTime().minute());
  getTime(mw_one->ui->hsH->value(), mw_one->ui->hsM->value());

  mw_one->ui->editDetails->clear();

  mw_one->ui->editCategory->setText("");

  mw_one->ui->editAmount->setText("");

  if (isAndroid) {
    if (isSelectTab) {
      isSelectTab = false;
      setDataToUI();
      openAddEventRecord(
          tr("Add") + "  : " + tabData->tabText(tabData->currentIndex()),
          mw_one->ui->editCategory->text(),
          mw_one->ui->editDetails->toPlainText(),
          mw_one->ui->editAmount->text(), mw_one->ui->lblTime->text());
    } else {
      openAddEventRecord(
          tr("Add") + "  : " + tabData->tabText(tabData->currentIndex()), "",
          "", "", mw_one->ui->lblTime->text());
    }
  } else {
    mw_one->ui->frameMain->hide();
    mw_one->ui->frameEditRecord->show();
  }

  updateCategoryCompleterList();

  // tmeFlash->start(300);
}

bool EditRecord::eventFilter(QObject* watched, QEvent* event) {
  // 当 frameEditRecord 被隐藏时，强制关闭 popup
  if (watched == mw_one->ui->frameEditRecord && event->type() == QEvent::Hide) {
    if (completer) {
      completer->popup()->hide();
    }
  }

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

void EditRecord::modify_Data() {
  QTreeWidget* tw = (QTreeWidget*)mw_one->ui->tabWidget->currentWidget();
  QTreeWidgetItem* item = tw->currentItem();
  QTreeWidgetItem* topItem = item->parent();
  QString newtime = mw_one->ui->lblTime->text().trimmed();
  if (item->childCount() == 0 && item->parent()->childCount() > 0) {
    item->setText(0, newtime);
    QString sa = mw_one->ui->editAmount->text().trimmed();
    if (sa == "")
      item->setText(1, "");
    else
      item->setText(1, QString("%1").arg(sa.toDouble(), 0, 'f', 2));
    item->setText(2, mw_one->ui->editCategory->text().trimmed());
    item->setText(3, mw_one->ui->editDetails->toPlainText().trimmed());
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
