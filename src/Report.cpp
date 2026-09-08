#include "Report.h"

#include <qdebug.h>

#include "MainWindow.h"
#include "defines.h"
#include "ui_DateSelector.h"

QTreeWidget* twOut2Img;
QLabel *lblTotal, *lblDetails;
QToolButton *btnCategory, *btnMonth, *btnYear;
int twTotalRow = 0;

Report::Report(QWidget* parent) : QDialog(parent) {
  twOut2Img = new QTreeWidget(nullptr);
}

void Report::init() {}

Report::~Report() { delete twOut2Img; }

void Report::keyReleaseEvent(QKeyEvent* event) { Q_UNUSED(event) }

bool Report::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      if (!mw_one->m_Report->isHidden()) {
        on_btnBack_clicked();
        return true;
      }
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void Report::on_btnBack_clicked() {
  saveYMD();

  listCategory.clear();
  indexCategory = 0;
}

void Report::closeEvent(QCloseEvent* event) { Q_UNUSED(event); }

void Report::on_btnSingleYear_clicked() {}

void Report::on_btnYear_clicked() {}

void Report::on_btnMonth_clicked() {}

void Report::startReport1(QString year, QString month) {}

void Report::startReport2() {}

void Report::readReportDone() {
  reportTitle2 = tr("Count:") + QString::number(totalFreq) + "  " +
                 tr("Amount:") + QString::number(totalAmount, 'f', 2);

  QStringList list;
  list.append(reportTitle1);
  list.append(reportTitle2);
  m_Method->refreshJavaData("refreshTitles", "DataReportActivity", list);

  m_Method->refreshJavaData("refreshReportList", "DataReportActivity", listTop);

  qInfo() << "listTop=" << listTop;
  qInfo() << "listCategory=" << listCategory;
  qInfo() << "title1 2=" << reportTitle1 << reportTitle2;
}

void Report::updateTable() {}

void Report::getData(QString y) {
  mw_one->showProgress();
  isReport = true;

  isWholeMonth = true;
  isDateSection = false;
  btnYearText = y;
  btnMonthText = "Year-Round";

  QString tabText = tabData->tabText(tabData->currentIndex());
  reportTitle1 = tabText + "(" + btnYearText + ")";

  mw_one->myReadEBookThread->start();
}

void Report::getData(QString y, QString m) {
  mw_one->showProgress();
  isReport = true;

  isWholeMonth = true;
  isDateSection = false;
  btnYearText = y;
  btnMonthText = m;

  QString tabText = tabData->tabText(tabData->currentIndex());
  reportTitle1 = tabText + "(" + btnYearText + "-" + btnMonthText + ")";

  mw_one->myReadEBookThread->start();
}

void Report::getData(int y1, int m1, int d1, int y2, int m2, int d2) {
  mw_one->showProgress();
  isReport = true;

  isWholeMonth = false;
  isDateSection = true;
  btnYearText = QString::number(QDate::currentDate().year());
  btnMonthText = QString::number(QDate::currentDate().month());
  s_y1 = y1;
  s_m1 = m1;
  s_d1 = d1;
  s_y2 = y2;
  s_m2 = m2;
  s_d2 = d2;

  mw_one->myReadEBookThread->start();
}

void Report::getMonthData() {
  int index = tabData->currentIndex();
  QTreeWidget* tw = mw_one->get_tw(index);
  QString tabText = tabData->tabText(index);

  twTotalRow = 0;
  totalFreq = 0;
  totalAmount = 0;
  listCategory.clear();
  listTop.clear();
  twOut2Img->clear();

  for (int i = 0; i < tw->topLevelItemCount(); i++) {
    QString strYear, strMonth;
    strYear = tw->topLevelItem(i)->text(3);
    strMonth = mw_one->get_Month(tw->topLevelItem(i)->text(0) + " " + strYear);
    int iDay = mw_one->get_Day(tw->topLevelItem(i)->text(0) + " " + strYear);

    if (isWholeMonth) {
      if (btnMonthText == "Year-Round") {
        if (strYear == btnYearText) {
          twTotalRow = twTotalRow + 1;
          QTreeWidgetItem* item;
          item = tw->topLevelItem(i)->clone();

          setTWImgData(item);
        }
      } else {
        if (strYear == btnYearText && strMonth == btnMonthText) {
          twTotalRow = twTotalRow + 1;
          QTreeWidgetItem* item;
          item = tw->topLevelItem(i)->clone();

          setTWImgData(item);
        }
      }
    }

    if (isDateSection) {
      int sy, sm, sd;
      sy = strYear.toInt();
      sm = strMonth.toInt();
      sd = iDay;
      QDateTime currentDateTime = QDateTime(QDate(sy, sm, sd), QTime(0, 0));
      QDateTime startDateTime = QDateTime(QDate(s_y1, s_m1, s_d1), QTime(0, 0));
      QDateTime endDateTime = QDateTime(QDate(s_y2, s_m2, s_d2), QTime(0, 0));
      int secondsDiff1 = startDateTime.secsTo(currentDateTime);
      int secondsDiff2 = currentDateTime.secsTo(endDateTime);

      reportTitle1 = tabText + "(" + startDateTime.toString("yyyy-MM-dd") +
                     "~" + endDateTime.toString("yyyy-MM-dd") + ")";

      if (secondsDiff1 >= 0 && secondsDiff2 >= 0) {
        twTotalRow = twTotalRow + 1;
        QTreeWidgetItem* item;
        item = tw->topLevelItem(i)->clone();

        setTWImgData(item);
      }
    }
  }
}

void Report::setTWImgData(QTreeWidgetItem* item) {
  QTreeWidgetItem* newtop = new QTreeWidgetItem;
  QFont f = newtop->font(0);
  f.setBold(true);
  newtop->setFont(0, f);
  newtop->setFont(1, f);
  newtop->setFont(2, f);
  newtop->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
  newtop->setText(0, item->text(0));
  newtop->setText(1, item->text(1));
  newtop->setText(2, item->text(2));
  newtop->setText(3, item->text(3));
  twOut2Img->addTopLevelItem(newtop);
  QBrush brush(Qt::lightGray);
  newtop->setBackground(0, brush);
  newtop->setBackground(1, brush);
  newtop->setBackground(2, brush);

  QString mstrDate = item->text(0);
  QString mstrFreq = item->text(1);
  QString mstrAmount = item->text(2);
  QString str_0 = mstrDate;
  QString str_1 = str_0.split(" ").at(0).trimmed();
  QString str_2 = str_0.replace(str_1, "").trimmed();
  listTop.append(mstrDate + " " + item->text(3) + "===" + tr("Count:") +
                 mstrFreq + "===" + tr("Amount:") + mstrAmount);

  int m_freq = mstrFreq.toInt();
  double m_amount = mstrAmount.toDouble();
  totalFreq = totalFreq + m_freq;
  totalAmount = totalAmount + m_amount;

  for (int z = 0; z < item->childCount(); z++) {
    QTreeWidgetItem* newchild = new QTreeWidgetItem(newtop);
    newchild->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
    newchild->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
    QString strClass = item->child(z)->text(2);
    newchild->setText(0, item->child(z)->text(0));
    newchild->setText(1, item->child(z)->text(1));
    newchild->setText(2, strClass);

    if (strClass.trimmed() != "") {
      listCategory.removeOne(strClass);
      listCategory.append(strClass);
    }

    QString strDes = item->child(z)->text(3);
    if (strDes.trimmed().length() > 0) {
      QTreeWidgetItem* des = new QTreeWidgetItem(newtop);
      des->setText(0, tr("Details") + " : " + strDes);
    }
  }

  twTotalRow = twTotalRow + newtop->childCount();
}

void Report::saveYMD() {}

int Report::cmp(const void* a, const void* b) { return *(int*)a < *(int*)b; }

void Report::on_btnCategory_clicked() {
  if (listCategory.count() == 0) return;

  int count = getCount();
  if (count == 0) {
    return;
  }

  if (listCategory.count() > 0) {
    listCategorySort.clear();
    listD.clear();
    for (int i = 0; i < listCategory.count(); i++) {
      getCategoryData(listCategory.at(i), false);
    }

    QList<double> listE = listD;
    std::sort(listE.begin(), listE.end());

    catetext = "";
    listCateSortDisplay.clear();

    int nListCateSort = listCategorySort.count();
    int nListECount = listE.count();
    for (int j = 0; j < nListECount; j++) {
      for (int i = 0; i < nListCateSort; i++) {
        QString str1 = listCategorySort.at(i);
        QStringList l1 = str1.split("-=-");
        if (l1.count() == 2 && l1.at(0).split("|").at(0).trimmed() != "") {
          if (QString::number(listE.at(nListECount - 1 - j)) == l1.at(1)) {
            QString str2 = l1.at(0) + "===" +
                           QString("%1").arg(
                               listE.at(nListECount - 1 - j) * 100, 0, 'f', 2) +
                           " %";

            QString item0 = str2.split("|").at(0);

            QString pre = str2.split("===").at(1);

            QString item1 = str2.split("===").at(0).split("|").at(1);

            listCateSortDisplay.append(tr("Category") + " : " + item0 +
                                       "===" + tr("Percent") + " : " + pre +
                                       "===" + tr("Amount") + " : " + item1);

            catetext = catetext + "\n\n" + tr("Category") + " : " + item0 +
                       "  " + tr("Amount") + " : " + item1;

            listCategorySort.removeOne(str1);

            break;
          }
        }
      }
    }

    // qDebug() << "listCategorySort=" << listCategorySort.count()
    //        << listCategorySort;
    // qDebug() << "listE=" << listE.count() << listE;
  }

  m_Method->refreshJavaData("showCategoryDialog", "DataReportActivity",
                            listCateSortDisplay);

  // qInfo() << "catetext=" << catetext;
  // qInfo() << "listCateSortDisplay=" << listCateSortDisplay;
}

void Report::on_CateOk() {}

void Report::getCategoryData(QString strCategory, bool appendTable) {
  if (appendTable) {
    listCateDetail.clear();
  }

  int freq = 0;
  double d_amount = 0;
  QTreeWidget* tw = twOut2Img;
  for (int i = 0; i < tw->topLevelItemCount(); i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);

    for (int j = 0; j < topItem->childCount(); j++) {
      QTreeWidgetItem* childItem = topItem->child(j);
      QString strClass = childItem->text(2);
      if (strClass == strCategory && strClass.trimmed() != "") {
        freq++;
        QString date, time, details;
        if (appendTable) {
          date = topItem->text(3) + "-" + topItem->text(0).split(" ").at(1) +
                 "-" + topItem->text(0).split(" ").at(2);
          time = childItem->text(0).split(".").at(1);

          if (j + 1 < topItem->childCount()) {
            QTreeWidgetItem* nextChild = topItem->child(j + 1);

            if (nextChild->text(0).contains(tr("Details"))) {
              details = nextChild->text(0);
            }
          }
        }
        QString amount = childItem->text(1);
        if (appendTable) {
          QString str;
          if (details.trimmed().length() > 0)
            str = details;
          else
            str = "";

          QString text0, text1, text2;
          text0 = tr("Date") + " : " + date + "  " + time;
          text1 = tr("Amount") + " : " + amount;
          text2 = str;
          listCateDetail.append(text0 + "===" + text1 + "===" + text2);
        }

        if (amount.length() > 0) {
          d_amount = d_amount + amount.toDouble();
        }
      }
    }
  }

  double bfb;
  if (totalAmount > 0) bfb = d_amount / totalAmount;

  QString ta = QString("%1").arg(d_amount, 0, 'f', 2);
  if (appendTable) {
    cateDetailTitle = strCategory + "\n" + tr("Freq") + " : " +
                      QString::number(freq) + "  " + tr("Amount") + " : " + ta;

  } else {
    listCategorySort.append(strCategory + "|" + ta + "-=-" +
                            QString::number(bfb));
    listD.append(bfb);
  }
}

QString Report::Out2Img(bool isShowMessage) { return "picFile"; }

void Report::appendTable(QString date, QString freq, QString amount) {}

int Report::getCount() { return twOut2Img->topLevelItemCount(); }

void Report::delItem(int index) {}

void Report::clearAll() {
  int count = getCount();
  for (int i = 0; i < count; i++) delItem(0);
}

void Report::appendSteps_xx(QString date, QString steps, QString km) {}

int Report::getCount_xx() { return 0; }

void Report::delItem_xx(int index) {}

void Report::clearAll_xx() {
  int count = getCount_xx();
  for (int i = 0; i < count; i++) delItem_xx(0);
}

int Report::getCurrentIndex() { return 0; }

QString Report::getDate(int row) { return ""; }

void Report::setCurrentHeader(int sn) {}

void Report::setScrollBarPos(double pos) {}

void Report::setScrollBarPos_xx(double pos) {}

void Report::loadDetailsQml() {
  if (getCount() == 0) return;

  btnCategory->setText(tr("View Category"));

  clearAll_xx();

  int row = getCurrentIndex();
  QString date = getDate(row);
  date.replace("*", "");
  date = date.trimmed();
  QString year;
  QStringList list = listTableSync.at(row).split("===");
  if (list.count() == 2) {
    year = list.at(1).trimmed();
  }

  QTreeWidget* tw = mw_one->get_tw(tabData->currentIndex());
  for (int i = 0; i < tw->topLevelItemCount(); i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    QString str_year = topItem->text(3);
    QString str_date = topItem->text(0);

    if (str_date.contains(date) && str_year == year) {
      // mui->lblDetails->setText(tr("Details") + "    " + str_date + "    " +
      //                          str_year);

      int childCount = topItem->childCount();

      for (int j = 0; j < childCount; j++) {
        QTreeWidgetItem* childItem = topItem->child(j);

        QString text0 = childItem->text(0);
        QStringList list = text0.split(".");
        if (list.count() == 2) text0 = list.at(1).trimmed();
        text0 = tr("Time") + " : " + text0;

        QString text1 = childItem->text(1);
        QString text2 = childItem->text(2);
        QString text3 = childItem->text(3);

        QString str1, str2, str3;
        if (text1.trimmed().length() > 0) str1 = tr("Amount") + " : " + text1;
        if (text2.trimmed().length() > 0) str2 = tr("Category") + " : " + text2;
        if (text3.trimmed().length() > 0) str3 = tr("Details") + " : " + text3;

        // m_Method->addItemToQW(mui->qwReportSub, text0, str1, str2, str3, 0);
      }
    }
  }
}

void Report::getDetail(int index) {
  QTreeWidgetItem* topitem = twOut2Img->topLevelItem(index);
  int count = topitem->childCount();
  QStringList list;
  for (int i = 0; i < count; i++) {
    list.append(topitem->child(i)->text(0));
    list.append(topitem->child(i)->text(1));
    list.append(topitem->child(i)->text(2));
    list.append(topitem->child(i)->text(3));
  }

  m_Method->refreshJavaData("showDetailDialog", "DataReportActivity", list);

  // qInfo() << "listDetail=" << list;
}

void Report::genReportMenu() {
  m_Menu = new QMenu(this);
  m_Menu->setStyleSheet(m_Method->qssMenu);

  QAction* actOuttoPic = new QAction(tr("Output to Image"));
  m_Menu->addAction(actOuttoPic);
  connect(actOuttoPic, &QAction::triggered, this, [=]() { Out2Img(true); });

#ifdef Q_OS_ANDROID
  QAction* actSharePic = new QAction(tr("Create Image and Share"));
  m_Menu->addAction(actSharePic);
  connect(actSharePic, &QAction::triggered, this, [=]() {
    QString picFile = Out2Img(false);
    if (QFile::exists(picFile))
      mw_one->m_ReceiveShare->shareImage(tr("Share to"), picFile, "image/png");
  });
#endif

  QAction* actSetYear = new QAction(tr("Year-Round"));
  m_Menu->addAction(actSetYear);
  actSetYear->setVisible(true);
  connect(actSetYear, &QAction::triggered, this,
          [=]() { on_btnSingleYear_clicked(); });

  QAction* actSetYearMonth = new QAction(tr("Year Month"));
  m_Menu->addAction(actSetYearMonth);
  connect(actSetYearMonth, &QAction::triggered, this,
          [=]() { mw_one->on_btnYear_clicked(); });

  QAction* actStartDate = new QAction(tr("Start Date"));
  m_Menu->addAction(actStartDate);
  connect(actStartDate, &QAction::triggered, this,
          [=]() { mw_one->on_btnStartDate_clicked(); });

  QAction* actEndDate = new QAction(tr("End Date"));
  m_Menu->addAction(actEndDate);
  connect(actEndDate, &QAction::triggered, this,
          [=]() { mw_one->on_btnEndDate_clicked(); });

  int x = 0;
  x = mw_one->geometry().x() + 2;
  int y = 0;  // mw_one->geometry().y() + mui->btnMenuReport->height() + 12;
  QPoint pos(x, y);
  m_Menu->exec(pos);
}

void Report::aiAnalysis() {
  isAiAnalysis = true;
  QString text = mainDataString;
  QString newEvent = mw_one->listMyEventTitle.at(0);
  int pos = text.indexOf('\n');
  QString trimText =
      QString("Event: %1").arg(newEvent) + (pos >= 0 ? text.mid(pos) : "");

  trimText = trimText.trimmed();
  if (trimText.isEmpty()) {
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg(tr("Tip"), tr("No data available"), 0);
    return;
  }

  // qDebug() << trimText;

  // 获取当前程序生效的语言标识
  QLocale loc = QLocale::system();
  QString langCode = loc.name();  // 格式 zh_CN / en_US / ja_JP

  // 标准化英文指令，明确指定输出语言，精准可控
  QString promptTemplate = R"(
Analyze the data records below, judge whether the occurrence rule of the target Event is scientific and reasonable combined with Event type, daily Frequency, specific occurrence time and corresponding date, then give mild adjustment suggestions and aggressive adjustment plans for reference.
Strict rules you must follow:
1. All analysis and suggestions must be written in language code: %1
2. First complete data summary and rationality analysis: sort out the overall distribution cycle of this Event, the daily occurrence frequency range, concentrated time period of the Event every day; judge unreasonable points such as excessive daily frequency, excessively scattered or overly concentrated occurrence time, long-term continuous occurrence, irregular occurrence rhythm.
3. Targeted suggestions need to be divided into two plans for each optimization direction:
   - Mild adjustment plan: low transformation cost, small change to original living habits, easy long-term adherence
   - Aggressive improvement plan: stronger constraint effect, more obvious optimization of the occurrence rhythm of this Event
4. Do not output redundant irrelevant content, only retain data summary rationality analysis + two sets of classified adjustment suggestions.
5. Focus on analyzing unreasonable risk points of the Event: frequent repeated daily occurrences, abnormal occurrence time (too early/too late), continuous multi-day occurrence without interval, unstable daily frequency fluctuation.

Event records:

%2
)";

  QString fullPrompt = promptTemplate.arg(langCode, trimText);
  mw_one->aiChatQuery(fullPrompt);
}
