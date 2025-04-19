#include "Report.h"

#include <qdebug.h>

#include "MainWindow.h"
#include "ui_DateSelector.h"
#include "ui_MainWindow.h"

extern int fontSize;
extern MainWindow* mw_one;
extern Method* m_Method;
extern QString iniFile, iniDir, btnYText, btnMText, btnDText;
extern QTabWidget *tabData, *tabChart;
extern bool isEBook, isReport, isDark, isAndroid;

QString btnYearText, btnMonthText;
QStringList listCategory;

QTreeWidget* twOut2Img;
QLabel *lblTotal, *lblDetails;
QToolButton *btnCategory, *btnMonth, *btnYear;
int twTotalRow = 0;
bool isWholeMonth = true;
bool isDateSection = false;

int s_y1 = 0;
int s_m1 = 0;
int s_d1 = 0;
int s_y2 = 0;
int s_m2 = 0;
int s_d2 = 0;

void setTableNoItemFlags(QTableWidget* t, int row);

Report::Report(QWidget* parent) : QDialog(parent) {
  this->installEventFilter(this);

  mw_one->ui->btnYear->hide();
  mw_one->ui->btnMonth->hide();
  mw_one->ui->btnStartDate->hide();
  mw_one->ui->btnEndDate->hide();
  mw_one->ui->lblTo->hide();

  m_DateSelector = new DateSelector(this);

  twOut2Img = new QTreeWidget;
  twOut2Img->setColumnCount(3);

  twOut2Img->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  twOut2Img->header()->setDefaultAlignment(Qt::AlignCenter);
  twOut2Img->headerItem()->setTextAlignment(2, Qt::AlignRight);
  twOut2Img->setAlternatingRowColors(true);
  twOut2Img->setStyleSheet(mw_one->treeStyle);
  twOut2Img->setUniformRowHeights(true);

  lblTotal = mw_one->ui->lblTotal;
  lblDetails = mw_one->ui->lblDetails;
  btnCategory = mw_one->ui->btnCategory;
  btnMonth = mw_one->ui->btnMonth;
  btnYear = mw_one->ui->btnYear;

  QFont font1 = m_Method->getNewFont(18);
  mw_one->ui->lblTotal->setFont(font1);
  mw_one->ui->lblTo->setFont(font1);
  mw_one->ui->btnYear->setFont(font1);
  mw_one->ui->btnMonth->setFont(font1);
  mw_one->ui->btnStartDate->setFont(font1);
  mw_one->ui->btnEndDate->setFont(font1);
  mw_one->ui->btnBack_Report->setFont(font1);
  mw_one->ui->btnCategory->setFont(font1);
  twOut2Img->setFont(font1);

  QFont font = mw_one->ui->lblTotal->font();
  font.setBold(true);
  mw_one->ui->lblTotal->setFont(font);
  mw_one->ui->lblDetails->setFont(font);
  mw_one->ui->lblTitle_Report->setFont(font);
  mw_one->ui->lblDetails->setWordWrap(true);
  mw_one->ui->lblDetails->adjustSize();

  lblTotal->adjustSize();
  lblTotal->setWordWrap(true);
  lblDetails->adjustSize();
  lblDetails->setWordWrap(true);

  mw_one->ui->lblViewCate1->adjustSize();
  mw_one->ui->lblViewCate1->setWordWrap(true);

  mw_one->ui->lblViewCate2->adjustSize();
  mw_one->ui->lblViewCate2->setWordWrap(true);

  mw_one->ui->lblViewCate3->adjustSize();
  mw_one->ui->lblViewCate3->setWordWrap(true);
}

void Report::init() {
  mw_one->ui->frameReport->setGeometry(
      mw_one->geometry().x(), mw_one->geometry().y(),
      mw_one->geometry().width(), mw_one->geometry().height());
  mw_one->ui->qwReport->setFixedHeight(mw_one->height() / 3 - 15);
  mw_one->ui->frameMain->hide();
  mw_one->ui->frameReport->show();
  if (isWholeMonth)
    mw_one->ui->lblTitle_Report->setText(
        mw_one->ui->tabWidget->tabText(mw_one->ui->tabWidget->currentIndex()) +
        "(" + mw_one->ui->btnYear->text() + "-" + mw_one->ui->btnMonth->text() +
        ")");

  if (isDateSection) {
    QStringList listStart = mw_one->ui->btnStartDate->text().split("  ");
    QStringList listEnd = mw_one->ui->btnEndDate->text().split("  ");
    mw_one->ui->lblTitle_Report->setText(
        mw_one->ui->tabWidget->tabText(mw_one->ui->tabWidget->currentIndex()) +
        "(" + listStart.at(0) + "-" + listStart.at(1) + "-" + listStart.at(2) +
        "~" + listEnd.at(0) + "-" + listEnd.at(1) + "-" + listEnd.at(2) + ")");
  }
}

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

  mw_one->ui->frameReport->hide();
  mw_one->ui->frameMain->show();
}

void Report::closeEvent(QCloseEvent* event) { Q_UNUSED(event); }

void Report::on_btnYear_clicked() {
  m_DateSelector->dateFlag = 1;

  // if (isAndroid) {
  //   int y, m;
  //   y = mw_one->ui->btnYear->text().toInt();
  //   m = mw_one->ui->btnMonth->text().toInt();
  //   m_Method->setDateTimePickerFlag("ym", y, m, 0, 0, 0, "");
  //   m_Method->openDateTimePicker();
  //   return;
  // }

  mw_one->ui->lblDetails->setText(tr("Details"));

  QDate date(mw_one->ui->btnYear->text().toInt(),
             mw_one->ui->btnMonth->text().toInt(), 1);
  m_DateSelector->m_datePickerYM->setDate(date);

  m_DateSelector->init();
}

void Report::on_btnMonth_clicked() {
  m_DateSelector->dateFlag = 2;
  mw_one->ui->lblDetails->setText(tr("Details"));

  m_DateSelector->init();
}

void Report::startReport1(QString year, QString month) {
  btnYearText = year;
  btnMonthText = month;

  isWholeMonth = true;
  isDateSection = false;
  mw_one->ui->lblTitle_Report->setText(
      mw_one->ui->tabWidget->tabText(mw_one->ui->tabWidget->currentIndex()) +
      "(" + mw_one->ui->btnYear->text() + "-" + mw_one->ui->btnMonth->text() +
      ")");
  listCategory.clear();
  mw_one->startInitReport();
}

void Report::startReport2() {
  isWholeMonth = false;
  isDateSection = true;
  listCategory.clear();

  QString start = mw_one->ui->btnStartDate->text();
  QString end = mw_one->ui->btnEndDate->text();
  QStringList listStart = start.split("  ");
  QStringList listEnd = end.split("  ");

  s_y1 = listStart.at(0).toInt();
  s_m1 = listStart.at(1).toInt();
  s_d1 = listStart.at(2).toInt();
  s_y2 = listEnd.at(0).toInt();
  s_m2 = listEnd.at(1).toInt();
  s_d2 = listEnd.at(2).toInt();

  mw_one->ui->lblTitle_Report->setText(
      mw_one->ui->tabWidget->tabText(mw_one->ui->tabWidget->currentIndex()) +
      "(" + listStart.at(0) + "-" + listStart.at(1) + "-" + listStart.at(2) +
      "~" + listEnd.at(0) + "-" + listEnd.at(1) + "-" + listEnd.at(2) + ")");

  mw_one->startInitReport();
}

void Report::updateTable() {
  freq = 0;
  t_amount = 0;

  clearAll();
  clearAll_xx();
  listTableSync.clear();

  mw_one->ui->lblTotal->setText(tr("Total") + " : " + tr("Freq") + " 0    " +
                                tr("Amount") + " 0");
  mw_one->ui->lblDetails->setText(tr("Details"));
  mw_one->ui->lblDetails->setStyleSheet(mw_one->ui->lblTitle->styleSheet());

  for (int i = 0; i < twOut2Img->topLevelItemCount(); i++) {
    QTreeWidgetItem* topItem = twOut2Img->topLevelItem(i);
    QString text0 = topItem->text(0);
    QString text1 = topItem->text(1);
    QString text2 = topItem->text(2);
    QString text3 = topItem->text(3);
    freq = freq + text1.toInt();
    if (text2.length() > 0) t_amount = t_amount + text2.toDouble();

    QStringList list = text0.split(" ");
    QString str_t0;
    if (list.count() == 3) {
      str_t0 = list.at(1) + " " + list.at(2);
    }

    appendTable(str_t0, text1, text2);
    listTableSync.append(text0 + "===" + text3);
  }

  mw_one->ui->lblTotal->setText(tr("Total") + " : " + tr("Freq") + " " +
                                QString::number(freq) + "    " + tr("Amount") +
                                " " + QString("%1").arg(t_amount, 0, 'f', 2));

  mw_one->ui->btnCategory->setText(tr("View Category"));

  setScrollBarPos(0);
  m_Method->setCurrentIndexFromQW(mw_one->ui->qwReport, 0);
  loadDetailsQml();

  mw_one->ui->lblMonthSum->setText("");
  mw_one->ui->lblMonthSum->setText(
      tr("Month Sum") + " : " + tr("Freq") + " " + QString::number(freq) +
      "    " + tr("Amount") + " " + QString("%1").arg(t_amount, 0, 'f', 2));
}

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

void Report::saveYMD() {
  QSettings Reg(iniDir + "ymd.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif
  Reg.setValue("/YMD/btnYearText", btnYearText);
  Reg.setValue("/YMD/btnMonthText", btnMonthText);
  Reg.setValue("/YMD/btnYText", btnYText);
  Reg.setValue("/YMD/btnMText", btnMText);
  Reg.setValue("/YMD/btnDText", btnDText);

  QStringList list1, list2;
  list1 = mw_one->ui->btnStartDate->text().split("  ");
  list2 = mw_one->ui->btnEndDate->text().split("  ");
  Reg.setValue("/YMD/Y1", list1.at(0));
  Reg.setValue("/YMD/Y2", list2.at(0));
  Reg.setValue("/YMD/M1", list1.at(1));
  Reg.setValue("/YMD/M2", list2.at(1));
  Reg.setValue("/YMD/D1", list1.at(2));
  Reg.setValue("/YMD/D2", list2.at(2));

  Reg.setValue("/YMD/isWholeMonth", isWholeMonth);
  Reg.setValue("/YMD/isDateSection", isDateSection);
}

int Report::cmp(const void* a, const void* b) { return *(int*)a < *(int*)b; }

void Report::on_btnCategory_clicked() {
  mw_one->ui->frameReport->hide();
  mw_one->ui->frameViewCate->show();

  int count = getCount();
  if (count == 0) {
    btnCategory->setText(tr("View Category"));

    return;
  }

  m_Method->clearAllBakList(mw_one->ui->qwViewCate);
  mw_one->ui->lblViewCate1->setText(mw_one->ui->lblTitle_Report->text());
  mw_one->ui->lblViewCate2->setText(mw_one->ui->lblTotal->text());

  if (listCategory.count() > 0) {
    listCategorySort.clear();
    listD.clear();
    for (int i = 0; i < listCategory.count(); i++) {
      getCategoryData(listCategory.at(i), false);
    }

    QList<double> listE = listD;
    std::sort(listE.begin(), listE.end());

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

            m_Method->addItemToQW(mw_one->ui->qwViewCate,
                                  tr("Category") + " : " + item0,
                                  tr("Percent") + " : " + pre,
                                  tr("Amount") + " : " + item1, "", 0);

            listCategorySort.removeOne(str1);

            break;
          }
        }
      }
    }

    int cate_count = m_Method->getCountFromQW(mw_one->ui->qwViewCate);
    if (cate_count > 0) {
      mw_one->ui->lblViewCate3->setText(tr("View Category") + "  " +
                                        QString::number(cate_count));
      m_Method->setCurrentIndexFromQW(mw_one->ui->qwViewCate, indexCategory);
    }

    // qDebug() << "listCategorySort=" << listCategorySort.count()
    //        << listCategorySort;
    // qDebug() << "listE=" << listE.count() << listE;
  }
}

void Report::on_CateOk() {
  int index = m_Method->getCurrentIndexFromQW(mw_one->ui->qwViewCate);
  QString str0 = m_Method->getText0(mw_one->ui->qwViewCate, index);
  str0 = str0.replace(tr("Category") + " : ", "").trimmed();

  getCategoryData(str0, true);
  indexCategory = index;

  mw_one->ui->frameViewCate->hide();
  mw_one->ui->frameReport->show();
}

void Report::getCategoryData(QString strCategory, bool appendTable) {
  if (appendTable) {
    m_Method->clearAllBakList(mw_one->ui->qwReportSub);
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

          QString text0, text1, text2, text3;
          text0 = tr("Date") + " : " + date + "  " + time;
          text1 = tr("Amount") + " : " + amount;
          text2 = str;
          m_Method->addItemToQW(mw_one->ui->qwReportSub, text0, text1, text2,
                                text3, 0);
        }

        if (amount.length() > 0) {
          d_amount = d_amount + amount.toDouble();
        }
      }
    }
  }

  double bfb;
  if (t_amount > 0) bfb = d_amount / t_amount;

  QString ta = QString("%1").arg(d_amount, 0, 'f', 2);
  if (appendTable) {
    mw_one->ui->lblDetails->setText(strCategory + "\n" + tr("Freq") + " : " +
                                    QString::number(freq) + "  " +
                                    tr("Amount") + " : " + ta);

    setScrollBarPos_xx(0);
  } else {
    listCategorySort.append(strCategory + "|" + ta + "-=-" +
                            QString::number(bfb));
    listD.append(bfb);
  }

  if (m_Method->getCountFromQW(mw_one->ui->qwReportSub) > 0)
    m_Method->setCurrentIndexFromQW(mw_one->ui->qwReportSub, 0);
}

void setTableNoItemFlags(QTableWidget* t, int row) {
  for (int z = 0; z < t->columnCount(); z++) {
    t->item(row, z)->setFlags(Qt::NoItemFlags);
  }
}

QString Report::Out2Img(bool isShowMessage) {
  Q_UNUSED(isShowMessage);
  QString picFile = "";
  if (twOut2Img->topLevelItemCount() == 0) return picFile;

  if (twOut2Img->topLevelItem(0)->text(0) != mw_one->ui->btnYear->text()) {
    twOut2Img->expandAll();

    QTreeWidgetItem* item0 = new QTreeWidgetItem;
    QTreeWidgetItem* item1 = new QTreeWidgetItem;
    QString st = mw_one->ui->lblTitle_Report->text();
    QStringList st_list = st.split("(");
    QString st1 = st_list.at(0);
    QString st2 = "(" + st_list.at(1);
    item0->setText(0, st2);
    item1->setText(0, st1);

    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(0, tr("Total") + " : ");
    item->setText(1, tr("Freq") + " " + QString::number(freq));
    item->setText(2,
                  tr("Amount") + " " + QString("%1").arg(t_amount, 0, 'f', 2));

    qreal h = twTotalRow * 28 + 4 * 28;
    twOut2Img->setGeometry(0, 0, mw_one->width(), h);

    // The column merge is implemented
    QTreeWidget* m_t = new QTreeWidget();
    m_t->setStyleSheet(mw_one->treeStyle);
    m_t->verticalScrollBar()->hide();
    QFont f = twOut2Img->font();
    f.setPointSize(13);
    m_t->setFont(f);
    m_t->setColumnCount(3);
    m_t->headerItem()->setText(0, "  " + tr("Date") + "  ");
    m_t->headerItem()->setText(1, tr("Freq"));
    m_t->headerItem()->setText(2, tr("Amount"));
    m_t->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_t->header()->setDefaultAlignment(Qt::AlignCenter);
    m_t->headerItem()->setTextAlignment(0, Qt::AlignHCenter);
    m_t->headerItem()->setTextAlignment(1, Qt::AlignLeft);
    m_t->headerItem()->setTextAlignment(2, Qt::AlignRight);
    m_t->header()->hide();
    m_t->setAlternatingRowColors(true);
    m_t->setUniformRowHeights(true);
    for (int i = 0; i < twOut2Img->topLevelItemCount(); i++) {
      QTreeWidgetItem* top = twOut2Img->topLevelItem(i)->clone();
      if (isDark) {
        top->setForeground(0, Qt::red);
        top->setForeground(1, Qt::red);
        top->setForeground(2, Qt::red);
      }
      m_t->addTopLevelItem(top);
    }

    m_t->insertTopLevelItem(0, item0);
    m_t->insertTopLevelItem(0, item1);
    m_t->addTopLevelItem(item);

    for (int i = 0; i < m_t->topLevelItemCount(); i++) {
      QTreeWidgetItem* top = m_t->topLevelItem(i);
      for (int j = 0; j < top->childCount(); j++) {
        if (top->child(j)->text(0).contains(tr("Details")))
          top->child(j)->setFirstColumnSpanned(true);
      }
    }
    m_t->expandAll();
    m_t->setGeometry(0, 0, mw_one->width(), h + m_t->header()->height());

    // Method1
    QPixmap pixmap(m_t->size());
    m_t->render(&pixmap);

    // Method2
    // QPixmap pixmap = QPixmap::grabWidget(m_t);

    QString strFile;
    strFile = mw_one->ui->lblTitle_Report->text() + ".png";

#ifdef Q_OS_ANDROID
    QDir* folder = new QDir;
    QString path = "/storage/emulated/0/KnotBak/";
    folder->mkdir(path);
    picFile = path + strFile;
    pixmap.save(picFile, "PNG");
    m_Method->m_widget = new QWidget(mw_one);
    ShowMessage* m_ShowMsg = new ShowMessage(this);
    if (!QFile(picFile).exists()) {
      m_ShowMsg->showMsg(
          "Knot", tr("Please turn on the storage permission of the app."), 1);

    } else {
      if (isShowMessage)
        m_ShowMsg->showMsg(
            "Knot", tr("Picture output successful!") + "\n\n" + picFile, 1);
    }
#else

    picFile = QFileDialog::getSaveFileName(this, tr("Save Config"),
                                           QDir::homePath() + "/" + strFile,
                                           tr("PNG files(*.png)"));

    if (!picFile.isNull()) {
      pixmap.save(picFile, "PNG");
    }

#endif
  }

  return picFile;
}

void Report::appendTable(QString date, QString freq, QString amount) {
  QQuickItem* root = mw_one->ui->qwReport->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "appendTableRow",
                            Q_ARG(QVariant, date), Q_ARG(QVariant, freq),
                            Q_ARG(QVariant, amount));
}

int Report::getCount() {
  QQuickItem* root = mw_one->ui->qwReport->rootObject();
  QVariant itemCount;
  QMetaObject::invokeMethod((QObject*)root, "getItemCount",
                            Q_RETURN_ARG(QVariant, itemCount));
  return itemCount.toInt();
}

void Report::delItem(int index) {
  QQuickItem* root = mw_one->ui->qwReport->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "delItem", Q_ARG(QVariant, index));
}

void Report::clearAll() {
  int count = getCount();
  for (int i = 0; i < count; i++) delItem(0);
}

void Report::appendSteps_xx(QString date, QString steps, QString km) {
  QQuickItem* root = mw_one->ui->qwReportSub->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "appendTableRow",
                            Q_ARG(QVariant, date), Q_ARG(QVariant, steps),
                            Q_ARG(QVariant, km));
}

int Report::getCount_xx() {
  QQuickItem* root = mw_one->ui->qwReportSub->rootObject();
  QVariant itemCount;
  QMetaObject::invokeMethod((QObject*)root, "getItemCount",
                            Q_RETURN_ARG(QVariant, itemCount));
  return itemCount.toInt();
}

void Report::delItem_xx(int index) {
  QQuickItem* root = mw_one->ui->qwReportSub->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "delItem", Q_ARG(QVariant, index));
}

void Report::clearAll_xx() {
  int count = getCount_xx();
  for (int i = 0; i < count; i++) delItem_xx(0);
}

int Report::getCurrentIndex() {
  QQuickItem* root = mw_one->ui->qwReport->rootObject();
  QVariant itemIndex;
  QMetaObject::invokeMethod((QObject*)root, "getCurrentIndex",
                            Q_RETURN_ARG(QVariant, itemIndex));
  return itemIndex.toInt();
}

QString Report::getDate(int row) {
  QQuickItem* root = mw_one->ui->qwReport->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject*)root, "getDate",
                            Q_RETURN_ARG(QVariant, item), Q_ARG(QVariant, row));
  return item.toString();
}

void Report::setCurrentHeader(int sn) {
  QQuickItem* root = mw_one->ui->qwReportSub->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "setHeader", Q_ARG(QVariant, sn));
}

void Report::setScrollBarPos(double pos) {
  QQuickItem* root = mw_one->ui->qwReport->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "setScrollBarPos",
                            Q_ARG(QVariant, pos));
}

void Report::setScrollBarPos_xx(double pos) {
  QQuickItem* root = mw_one->ui->qwReportSub->rootObject();
  QMetaObject::invokeMethod((QObject*)root, "setScrollBarPos",
                            Q_ARG(QVariant, pos));
}

void Report::loadDetailsQml() {
  if (getCount() == 0) return;

  btnCategory->setText(tr("View Category"));
  mw_one->ui->lblDetails->setText(tr("Details"));
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
      mw_one->ui->lblDetails->setText(tr("Details") + "    " + str_date +
                                      "    " + str_year);

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

        m_Method->addItemToQW(mw_one->ui->qwReportSub, text0, str1, str2, str3,
                              0);
      }
    }
  }
}

void Report::genReportMenu() {
  QMenu* m_Menu = new QMenu(this);
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

  QAction* actSetYear = new QAction(tr("Year Month"));
  m_Menu->addAction(actSetYear);
  connect(actSetYear, &QAction::triggered, this,
          [=]() { mw_one->on_btnYear_clicked(); });

  QAction* actSetMonth = new QAction(tr("Month"));
  m_Menu->addAction(actSetMonth);
  actSetMonth->setVisible(false);
  connect(actSetMonth, &QAction::triggered, this,
          [=]() { mw_one->on_btnMonth_clicked(); });

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
  int y = mw_one->geometry().y() + mw_one->ui->btnMenuReport->height() + 12;
  QPoint pos(x, y);
  m_Menu->exec(pos);
}
