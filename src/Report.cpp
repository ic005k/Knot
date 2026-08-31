#include "Report.h"

#include <qdebug.h>

#include "MainWindow.h"
#include "defines.h"
#include "ui_DateSelector.h"

QTreeWidget* twOut2Img;
QLabel *lblTotal, *lblDetails;
QToolButton *btnCategory, *btnMonth, *btnYear;
int twTotalRow = 0;

Report::Report(QWidget* parent) : QDialog(parent) {}

void Report::init() {}

Report::~Report() {}

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

void Report::updateTable() {}

void Report::getMonthData() {
  QTreeWidget* tw = mw_one->get_tw(tabData->currentIndex());

  twOut2Img->clear();
  twTotalRow = 0;
  listCategory.clear();

  for (int i = 0; i < tw->topLevelItemCount(); i++) {
    QString strYear, strMonth;
    strYear = tw->topLevelItem(i)->text(3);
    strMonth = mw_one->get_Month(tw->topLevelItem(i)->text(0) + " " + strYear);
    int iDay = mw_one->get_Day(tw->topLevelItem(i)->text(0) + " " + strYear);

    if (isWholeMonth) {
      if (btnMonthText == tr("Year-Round")) {
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

void Report::on_btnCategory_clicked() {}

void Report::on_CateOk() {}

void Report::getCategoryData(QString strCategory, bool appendTable) {}

QString Report::Out2Img(bool isShowMessage) { return "picFile"; }

void Report::appendTable(QString date, QString freq, QString amount) {}

int Report::getCount() { return 0; }

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
