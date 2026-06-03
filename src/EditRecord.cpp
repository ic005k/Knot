#include "EditRecord.h"

#include <QKeyEvent>

#include "MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"

QStringList c_list;

EditRecord::EditRecord() {
  m_CategoryList = new CategoryList(mw_one);

  mw_one->installEventFilter(mw_one);

  initSuggestList();

  mui->editCategory->setFocus();
  mui->editDetails->setAcceptRichText(false);

  nH = mui->editCategory->height();

  QFont font = mw_one->font();
  font.setPointSize(22);
  mui->editCategory->setFont(font);
  if (isAndroid)
    font.setPointSize(40);
  else {
    font.setPointSize(12);
    mui->btn0->setMinimumHeight(0);
    mui->btn1->setMinimumHeight(0);
    mui->btn2->setMinimumHeight(0);
    mui->btn3->setMinimumHeight(0);
    mui->btn4->setMinimumHeight(0);
    mui->btn5->setMinimumHeight(0);
    mui->btn6->setMinimumHeight(0);
    mui->btn7->setMinimumHeight(0);
    mui->btn8->setMinimumHeight(0);
    mui->btn9->setMinimumHeight(0);
    mui->btnDot->setMinimumHeight(0);
    mui->btnDel_Number->setMinimumHeight(0);
  }
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

  QValidator* validator =
      new QRegularExpressionValidator(regxNumber, mui->editAmount);
  mui->editAmount->setValidator(validator);
  mui->editAmount->setAttribute(Qt::WA_InputMethodEnabled, false);
  mui->editAmount->setReadOnly(true);

  mui->editCategory->setPlaceholderText(QObject::tr("Enter a category"));
  mui->editDetails->setPlaceholderText(QObject::tr("Enter notes"));

  lblStyle = mui->lblCategory->styleSheet();

  mui->hsM->setStyleSheet(mui->hsH->styleSheet());

  m_Method->qssSlider = mui->hsH->styleSheet();
}

EditRecord::~EditRecord() {
  delete m_CategoryList;

  if (m_suggestList) delete m_suggestList;
}

void EditRecord::on_btnOk_clicked() {
  mw_one->on_btnBackEditRecord_pressed();

  if (!isAdd) {
    mw_one->modify_Data();

    mw_one->strLatestModify =
        QObject::tr("Modify Item") + " ( " + mw_one->getTabText() + " ) ";

  } else {
    mw_one->add_Data(mw_one->get_tw(mui->tabWidget->currentIndex()),
                     mui->lblTime->text(), mui->editAmount->text().trimmed(),
                     mui->editCategory->text().trimmed());

    mw_one->strLatestModify =
        QObject::tr("Add Item") + " ( " + mw_one->getTabText() + " ) ";
  }

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
    QListWidgetItem* item = new QListWidgetItem(str);
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

  if (mui->qwMainDate->isHidden()) {
    mw_one->clickMainTab();
  }
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
    mui->qwCategory->rootContext()->setContextProperty("m_CategoryList",
                                                       m_CategoryList);
    mui->qwCategory->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/type.qml")));
  }

  mui->frameEditRecord->hide();
  mui->frameCategory->show();
  init_MyCategory();
  m_CategoryList->ui->listWidget->setCurrentRow(0);
  m_Method->setCurrentIndexFromQW(mui->qwCategory, 0);
  m_CategoryList->setTypeRenameText();

  int count = m_Method->getCountFromQW(mui->qwCategory);
  mui->lblTypeInfo->setText(QObject::tr("Total") + " : " +
                            QString::number(count));
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

  m_Method->clearAllBakList(mui->qwCategory);

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

void EditRecord::on_btnClearAmount_clicked() { mui->editAmount->clear(); }

void EditRecord::on_btnClearDesc_clicked() {
  mui->editCategory->clear();
  hideSuggestions();
}

void EditRecord::on_editAmount_textChanged(const QString& arg1) {
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

void EditRecord::on_editCategory_textChanged(const QString& arg1) {
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

  // 空内容就隐藏
  if (arg1.trimmed().isEmpty()) {
    hideSuggestions();
    return;
  }

  if (m_isUpdatingList) {
    m_isUpdatingList = false;
    return;
  }

  QTimer::singleShot(0, [this]() { showSuggestions(); });
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

void EditRecord::showSuggestions() {
  QString input = mui->editCategory->text().trimmed().toLower();
  if (input.isEmpty()) {
    hideSuggestions();
    return;
  }

  m_suggestList->clear();
  QStringList matches;

  for (const QString& item : std::as_const(c_list)) {
    if (item.toLower().contains(input)) {
      matches << item;
    }
  }

  if (matches.isEmpty()) {
    m_suggestList->hide();
    return;
  }

  for (int i = 0; i < qMin(8, matches.size()); ++i) {
    m_suggestList->addItem(matches[i]);
  }

  // 样式：系统级下拉，自动支持暗黑模式
  if (isDark) {
    m_suggestList->setStyleSheet(R"(
        QListWidget {
            background-color: #1E1E1E;
            border: 1px solid #444444;
            font-size: 18px;
            color: #EEEEEE;
        }
        QListWidget::item {
            padding: 10px 14px;
        }
        QListWidget::item:selected {
            background-color: #007bff;
            color: white;
        }
    )");
  } else {
    m_suggestList->setStyleSheet(R"(
        QListWidget {
            background-color: #ffffff;
            border: 1px solid #cccccc;
            font-size: 18px;
            color: #333333;
        }
        QListWidget::item {
            padding: 10px 14px;
        }
        QListWidget::item:selected {
            background-color: #007bff;
            color: white;
        }
    )");
  }

  // ==============================================
  // 绝对全局坐标，永远不飘，永远在输入框正下方
  // ==============================================
  QPoint globalPos =
      mui->editCategory->mapToGlobal(QPoint(0, mui->editCategory->height()));
  int w = mui->editCategory->width();
  int h = qMin(200, m_suggestList->sizeHint().height());

  m_suggestList->setGeometry(globalPos.x(), globalPos.y(), w, h);
  m_suggestList->show();
}

void EditRecord::delSuggestions() {
  if (!m_suggestList) return;

  m_suggestList->setParent(nullptr);
  m_suggestList->hide();
  delete m_suggestList;
  m_suggestList = nullptr;
}

// 点击条目 → 填入输入框
void EditRecord::onSuggestionClicked(QListWidgetItem* item) {
  m_isUpdatingList = true;

  mui->editCategory->setText(item->text());

  if (m_suggestList) {
    m_suggestList->close();
  }
}

void EditRecord::initSuggestList() {
  // 先销毁旧列表
  delSuggestions();

  // ========== 自定义下拉补全（安卓不崩溃） ==========
  m_suggestList = new QListWidget(mw_one);

  if (isAndroid)
    m_suggestList->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
  else {
    m_suggestList->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
  }

  m_suggestList->setFocusPolicy(Qt::NoFocus);
  m_suggestList->setHidden(true);
  m_suggestList->verticalScrollBar()->setStyleSheet(m_Method->vsbarStyleBig);

  m_suggestList->setAttribute(Qt::WA_NativeWindow, false);
  m_suggestList->setAttribute(Qt::WA_DontCreateNativeAncestors, true);
  m_suggestList->setAttribute(Qt::WA_AlwaysStackOnTop, false);

  QObject::connect(
      m_suggestList, &QListWidget::itemClicked, mw_one,
      [this](QListWidgetItem* item) { this->onSuggestionClicked(item); });

  //====================================================
}

void EditRecord::hideSuggestions() {
  if (m_suggestList) {
    m_suggestList->setGeometry(0, 0, 1, 1);
    m_suggestList->close();
  }
}
