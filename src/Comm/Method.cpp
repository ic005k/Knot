#include "Method.h"

#include <QKeyEvent>

#include "src/MainWindow.h"
#include "ui_MainWindow.h"

// 工具函数：将 Qt 路径转换为 minizip-ng 兼容的路径格式
std::string toNormalizedPath(const QString &qtPath) {
  // 直接保留原始路径格式（使用正斜杠）
  QString normalized = qtPath;

  // 调试输出验证路径
  qDebug() << "Raw path:" << normalized;
  qDebug() << "UTF-8 bytes:" << normalized.toUtf8().toHex();

  return normalized.toUtf8().constData();
}

extern MainWindow *mw_one;
extern QTabWidget *tabData;
extern QString iniDir, searchStr, currentMDFile, privateDir, encPassword,
    errorInfo;
extern CategoryList *m_CategoryList;
extern bool isEpub, isText, isPDF, loading, isDark, isAndroid, isEncrypt;
extern int iPage, sPos, totallines, baseLines, htmlIndex, s_y1, s_m1, s_d1,
    s_y2, s_m2, s_d2, fontSize;
extern QStringList readTextList, htmlFiles, listCategory;
extern TextSelector *m_TextSelector;

QStringList resultsList;

bool isPasswordError = false;

Method::Method(QWidget *parent) : QDialog(parent) {
  mw_one->set_ToolButtonStyle(this);

  this->installEventFilter(this);

  m_widget = new QWidget(mw_one);
  m_widget->close();
}

void Method::init() {
  int w = mw_one->geometry().width();
  int h = mw_one->geometry().height();
  setFixedWidth(w);
  setFixedHeight(h);
  setGeometry(mw_one->geometry().x(), mw_one->geometry().y(), w, h);

  show();
}

Method::~Method() {}

QString Method::getRealPathFile(QString strFile) {
  strFile = mw_one->m_Reader->getUriRealPath(strFile);
  QString realFile = strFile;

  int aver = mw_one->m_AboutThis->getAndroidVer();
  // Android7.0及以上
  if (aver >= 24) {
  } else {
    // Android6.0
    QStringList list = strFile.split(":");
    int count = list.count();
    QString r_file = list.at(count - 1);
    QString driver;
    for (int i = 0; i < 5; i++) {
      driver = "/storage/emulated/" + QString::number(i) + "/";
      realFile = driver + r_file;
      if (QFile(realFile).exists()) break;
    }
  }

  return realFile;
}

void Method::showGrayWindows() {
  closeGrayWindows();

  m_widget->resize(mw_one->width(), mw_one->height());
  m_widget->move(0, 0);
  m_widget->setStyleSheet("background-color:rgba(0, 0, 0,35%);");

  m_widget->show();
}

void Method::closeGrayWindows() { m_widget->close(); }

QInputDialog *Method::inputDialog(QString windowsTitle, QString lblEdit,
                                  QString defaultValue) {
  QInputDialog *idlg = new QInputDialog(this);
  idlg->hide();
  idlg->setWindowFlag(Qt::FramelessWindowHint);
  QString style1 = "QDialog{border-radius:px;border:0px solid darkred;}";
  idlg->setStyleSheet(style1);
  idlg->setOkButtonText(tr("Ok"));
  idlg->setCancelButtonText(tr("Cancel"));
  idlg->setContentsMargins(10, 10, 10, 10);
  idlg->setWindowTitle(windowsTitle);
  idlg->setTextValue(defaultValue);
  idlg->setLabelText(lblEdit);
  mw_one->set_PushButtonStyle(idlg);

  m_widget = new QWidget(mw_one);
  showGrayWindows();

  idlg->show();
  idlg->setFixedWidth(mw_one->width() - 2);
  idlg->setGeometry(
      mw_one->geometry().x() + (mw_one->geometry().width() - idlg->width()) / 2,
      mw_one->geometry().y(), idlg->width(), idlg->height());

  connect(idlg, &QDialog::rejected, [=]() mutable { closeGrayWindows(); });
  connect(idlg, &QDialog::accepted, [=]() mutable { closeGrayWindows(); });

  return idlg;
}

void Method::closeKeyboard() {
  if (mw_one->pAndroidKeyboard->isVisible()) {
    mw_one->pAndroidKeyboard->hide();
  }

  if (!m_TextSelector->isHidden()) {
    m_TextSelector->on_btnClose_clicked();
  }
}

int Method::getFontHeight() {
  QFontMetrics fontMetrics(font());
  int nFontHeight = fontMetrics.height();
  return nFontHeight;
}

int Method::getStrWidth(const QString str) {
  QFontMetrics fontMetrics(font());
  int nFontWidth = fontMetrics.horizontalAdvance(str, -1);
  return nFontWidth;
}

void Method::addItem(QString text_tab, QString text0, QString text1,
                     QString text2, QString text3, int itemH) {
  QQuickItem *root = mw_one->ui->qwSearch->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "addItem",
                            Q_ARG(QVariant, text_tab), Q_ARG(QVariant, text0),
                            Q_ARG(QVariant, text1), Q_ARG(QVariant, text2),
                            Q_ARG(QVariant, text3), Q_ARG(QVariant, itemH));
}

void Method::delItem(int index) {
  QQuickItem *root = mw_one->ui->qwSearch->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "delItem", Q_ARG(QVariant, index));
}

int Method::getCount() {
  QQuickItem *root = mw_one->ui->qwSearch->rootObject();
  QVariant itemCount;
  QMetaObject::invokeMethod((QObject *)root, "getItemCount",
                            Q_RETURN_ARG(QVariant, itemCount));
  return itemCount.toInt();
}

void Method::clearAll() {
  int count = getCount();
  for (int i = 0; i < count; i++) {
    delItem(0);
  }
}

void Method::setCurrentIndex(int index) {
  QQuickItem *root = mw_one->ui->qwSearch->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "setCurrentItem",
                            Q_ARG(QVariant, index));
}

void Method::addItemToQW(QQuickWidget *qw, QString text0, QString text1,
                         QString text2, QString text3, int itemH) {
  QQuickItem *root = qw->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "addItem", Q_ARG(QVariant, text0),
                            Q_ARG(QVariant, text1), Q_ARG(QVariant, text2),
                            Q_ARG(QVariant, text3), Q_ARG(QVariant, itemH));
}

void Method::insertItem(QQuickWidget *qw, QString text0, QString text1,
                        QString text2, QString text3, int curIndex) {
  QQuickItem *root = qw->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "insertItem",
                            Q_ARG(QVariant, text0), Q_ARG(QVariant, text1),
                            Q_ARG(QVariant, text2), Q_ARG(QVariant, text3),
                            Q_ARG(QVariant, curIndex));
}

void Method::delItemFromQW(QQuickWidget *qw, int index) {
  QQuickItem *root = qw->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "delItem", Q_ARG(QVariant, index));
}

int Method::getCountFromQW(QQuickWidget *qw) {
  QQuickItem *root = qw->rootObject();
  QVariant itemCount;
  QMetaObject::invokeMethod((QObject *)root, "getItemCount",
                            Q_RETURN_ARG(QVariant, itemCount));
  return itemCount.toInt();
}

void Method::clearAllBakList(QQuickWidget *qw) {
  int count = getCountFromQW(qw);
  for (int i = 0; i < count; i++) {
    delItemFromQW(qw, 0);
  }
}

void Method::gotoEnd(QQuickWidget *qw) {
  QQuickItem *root = qw->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "gotoEnd");
}

void Method::setScrollBarPos(QQuickWidget *qw, double pos) {
  QQuickItem *root = qw->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "setScrollBarPos",
                            Q_ARG(QVariant, pos));
}

void Method::setCurrentIndexFromQW(QQuickWidget *qw, int index) {
  QQuickItem *root = qw->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "setCurrentItem",
                            Q_ARG(QVariant, index));
}

int Method::getCurrentIndexFromQW(QQuickWidget *qw) {
  QQuickItem *root = qw->rootObject();
  QVariant itemIndex;
  QMetaObject::invokeMethod((QObject *)root, "getCurrentIndex",
                            Q_RETURN_ARG(QVariant, itemIndex));
  return itemIndex.toInt();
}

void Method::modifyItemText0(QQuickWidget *qw, int index, QString strText) {
  QQuickItem *root = qw->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "modifyItemText0",
                            Q_ARG(QVariant, index), Q_ARG(QVariant, strText));
}

void Method::modifyItemText2(QQuickWidget *qw, int index, QString strText) {
  QQuickItem *root = qw->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "modifyItemText2",
                            Q_ARG(QVariant, index), Q_ARG(QVariant, strText));
}

void Method::modifyItemText3(QQuickWidget *qw, int index, QString strText) {
  QQuickItem *root = qw->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "modifyItemText3",
                            Q_ARG(QVariant, index), Q_ARG(QVariant, strText));
}

QString Method::getText0(QQuickWidget *qw, int index) {
  QQuickItem *root = qw->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject *)root, "getText0",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

QString Method::getText1(QQuickWidget *qw, int index) {
  QQuickItem *root = qw->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject *)root, "getText1",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

QString Method::getText2(QQuickWidget *qw, int index) {
  QQuickItem *root = qw->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject *)root, "getText2",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

QString Method::getText3(QQuickWidget *qw, int index) {
  QQuickItem *root = qw->rootObject();
  QVariant item;
  QMetaObject::invokeMethod((QObject *)root, "getText3",
                            Q_RETURN_ARG(QVariant, item),
                            Q_ARG(QVariant, index));
  return item.toString();
}

bool Method::eventFilter(QObject *watchDlgSearch, QEvent *evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      close();
      return true;
    }
  }

  return QWidget::eventFilter(watchDlgSearch, evn);
}

void Method::startSearch() {
  resultsList.clear();
  int tabCount = tabData->count();

  // QString a0("<span style=\"color: white;background: red;\">");
  // QString a1("</span>");

  QString a0 = "<font color=\" red \"><b>";
  QString a1 = "</b></font>";

  for (int j = 0; j < tabCount; j++) {
    QTreeWidget *tw = mw_one->get_tw(j);
    QString tabStr = tabData->tabText(j);

    for (int i = 0; i < tw->topLevelItemCount(); i++) {
      QString strYear, strMonthDay;
      strYear = tw->topLevelItem(i)->text(3);
      strMonthDay = tw->topLevelItem(i)->text(0);
      QString weeks = strMonthDay.split(" ").at(0);
      QString day =
          strMonthDay.split(" ").at(1) + " " + strMonthDay.split(" ").at(2);

      QTreeWidgetItem *topItem;
      topItem = tw->topLevelItem(i);
      int childCount = topItem->childCount();
      for (int j = 0; j < childCount; j++) {
        QString txt0, txt1, txt2, txt3;
        QTreeWidgetItem *childItem = topItem->child(j);

        QString strTime = childItem->text(0);
        if (strTime.split(".").count() == 2) {
          txt0 = strYear + " " + day + " " + weeks + " " +
                 strTime.split(".").at(1).trimmed();
        }

        txt1 = childItem->text(1);
        txt2 = childItem->text(2);
        txt3 = childItem->text(3);

        QStringList list;
        bool isYes = false;
        if (searchStr.contains("&")) {
          list = searchStr.split("&");
          bool is0, is1, is2, is3;
          is0 = false;
          is1 = false;
          is2 = false;
          is3 = false;
          for (int n = 0; n < list.count(); n++) {
            QString str = list.at(n);
            str = str.trimmed();

            if (str.length() > 0) {
              if (strYear.contains(str) || day.contains(str) ||
                  weeks.contains(str)) {
                is0 = true;
                txt0 = txt0.replace(str, a0 + str + a1);
              }

              if (txt1.contains(str)) {
                is1 = true;
                txt1 = txt1.replace(str, a0 + str + a1);
              }
              if (txt2.contains(str)) {
                is2 = true;
                txt2 = txt2.replace(str, a0 + str + a1);
              }
              if (txt3.contains(str)) {
                is3 = true;
                txt3 = txt3.replace(str, a0 + str + a1);
              }
            }
          }

          if (list.count() == 2) {
            if (is0 && is1) isYes = true;
            if (is0 && is2) isYes = true;
            if (is0 && is3) isYes = true;
            if (is1 && is2) isYes = true;
            if (is1 && is3) isYes = true;
            if (is2 && is3) isYes = true;
          }

          if (list.count() == 3) {
            if (is0 && is1 && is2) isYes = true;
            if (is0 && is1 && is3) isYes = true;
            if (is0 && is2 && is3) isYes = true;
            if (is1 && is2 && is3) isYes = true;
          }

          if (list.count() >= 4) {
            if (is0 && is1 && is2 && is3) isYes = true;
          }

          QString s_total = txt0 + txt1 + txt2 + txt3;
          int n_count = 0;
          for (int x = 0; x < list.count(); x++) {
            QString str = list.at(x);
            if (str.length() > 0) {
              if (s_total.contains(str)) {
                n_count++;
              }
            }
          }

          if (isYes) {
            if (n_count < list.count()) isYes = false;
          }

        } else {
          if (txt1.contains(searchStr) || txt2.contains(searchStr) ||
              txt3.contains(searchStr)) {
            isYes = true;

            if (txt1.contains(searchStr)) {
              txt1 = txt1.replace(searchStr, a0 + searchStr + a1);
            }

            if (txt2.contains(searchStr)) {
              txt2 = txt2.replace(searchStr, a0 + searchStr + a1);
            }

            if (txt3.contains(searchStr)) {
              txt3 = txt3.replace(searchStr, a0 + searchStr + a1);
            }
          }
        }

        if (isYes) {
          resultsList.append(tabStr + "=|=" + txt0 + "=|=" + txt1 +
                             "=|=" + txt2 + "=|=" + txt3);
        }
      }
    }
  }
}

void Method::initSearchResults() {
  // qDebug() << resultsList;

  clearAll();
  int count = resultsList.count();

  mw_one->ui->lblSearchResult->setText(tr("Results") + " : " +
                                       QString::number(count));
  if (count == 0) return;

  generateData(count);

  setCurrentIndex(0);
}

void Method::generateData(int count) {
  QFontMetrics fontMetrics(font());
  int nFontHeight = fontMetrics.height();
  int line_count;

  for (int i = 0; i < count; i++) {
    QStringList list = resultsList.at(count - 1 - i).split("=|=");
    QString str_tab, str0, str1, str2, str3;
    str_tab = list.at(0);
    str0 = list.at(1);
    str1 = list.at(2);
    str2 = list.at(3);
    str3 = list.at(4);

    line_count = 4;
    QString text1, text2, text3;
    if (str1.trimmed().length() > 0) {
      text1 = tr("Amount") + " : " + str1;
      line_count++;
    }
    if (str2.trimmed().length() > 0) {
      text2 = tr("Category") + " : " + str2;
      line_count++;
    }
    if (str3.trimmed().length() > 0) {
      text3 = tr("Details") + " : " + str3;
      line_count++;
    }

    addItem(str_tab, str0, text1, text2, text3, nFontHeight * (0));
  }
}

void Method::setCellText(int row, int column, QString str,
                         QTableWidget *table) {
  QString a0("<span style=\"color: white;background: red;\">");
  QString a1("</span>");

  if (str.contains(searchStr)) {
    str = str.replace(searchStr, a0 + searchStr + a1);

    QLabel *lbl = new QLabel();
    lbl->adjustSize();
    lbl->setWordWrap(true);
    lbl->setText(str);
    table->setCellWidget(row, column, lbl);
  } else
    table->setItem(row, column, new QTableWidgetItem(str));
}

QString Method::getLastModified(QString file) {
  QFileInfo info(file);
  QDateTime lastModified = info.lastModified();
  QString item1 = lastModified.toString();
  return item1;
}

void Method::clickMainDate() {
  bool isAniEffects;
  if (mw_one->isDelItem || mw_one->isEditItem)
    isAniEffects = false;
  else
    isAniEffects = mw_one->m_Preferences->ui->chkAniEffects->isChecked();
  mw_one->isDelItem = false;
  mw_one->isEditItem = false;

  mw_one->ui->qwMainEvent->rootContext()->setContextProperty("isAniEffects",
                                                             isAniEffects);

  mw_one->ui->qwMainEvent->rootContext()->setContextProperty(
      "maineventWidth", mw_one->ui->qwMainEvent->width());

  QTreeWidget *tw = mw_one->get_tw(mw_one->ui->tabWidget->currentIndex());
  int maindateIndex = getCurrentIndexFromQW(mw_one->ui->qwMainDate);
  int maindateCount = getCountFromQW(mw_one->ui->qwMainDate);
  int topIndex = tw->topLevelItemCount() - maindateCount + maindateIndex;

  if (topIndex < 0) return;

  clearAllBakList(mw_one->ui->qwMainEvent);
  QTreeWidgetItem *topItem = tw->topLevelItem(topIndex);
  int childCount = topItem->childCount();
  QString text0, text1, text2, text3;
  int nullrows;
  for (int j = 0; j < childCount; j++) {
    QTreeWidgetItem *childItem = topItem->child(j);
    text0 = childItem->text(0);
    text1 = childItem->text(1);
    text2 = childItem->text(2);
    text3 = childItem->text(3);

    nullrows = 1;

    if (text1.length() > 0) {
      // text1 = tr("Amount") + " : " + text1;
      nullrows++;
    }

    if (text2.length() > 0) {
      // text2 = tr("Category") + " : " + text2;
      nullrows++;
    }

    if (text3.length() > 0) {
      // text3 = tr("Details") + " : " + text3;
      nullrows++;
    }

    addItemToQW(mw_one->ui->qwMainEvent, text0, text1, text2, text3, 0);
  }

  gotoEnd(mw_one->ui->qwMainEvent);
  int count = getCountFromQW(mw_one->ui->qwMainEvent);
  setCurrentIndexFromQW(mw_one->ui->qwMainEvent, count - 1);
  setScrollBarPos(mw_one->ui->qwMainEvent, 1.0);

  setMainTabCurrentIndex();
}

void Method::setMainTabCurrentIndex() {
  int tabIndex = tabData->currentIndex();
  if (tabIndex > 0) {
    mw_one->setCurrentIndex(tabIndex - 1);
    mw_one->setCurrentIndex(tabIndex);
  }
  if (tabIndex == 0 && mw_one->getCount() > 1) {
    mw_one->setCurrentIndex(tabIndex + 1);
    mw_one->setCurrentIndex(tabIndex);
  }
}

void Method::clickMainDateData() {
  QTreeWidget *tw = mw_one->get_tw(mw_one->ui->tabWidget->currentIndex());
  int maindateIndex = getCurrentIndexFromQW(mw_one->ui->qwMainDate);
  int maindateCount = getCountFromQW(mw_one->ui->qwMainDate);
  int topIndex = tw->topLevelItemCount() - maindateCount + maindateIndex;

  if (topIndex < 0) return;

  tw->setCurrentItem(tw->topLevelItem(topIndex));

  mw_one->on_twItemClicked();
}

void Method::clickMainEventData() {
  QTreeWidget *tw = mw_one->get_tw(mw_one->ui->tabWidget->currentIndex());
  int maindateIndex = getCurrentIndexFromQW(mw_one->ui->qwMainDate);
  int maindateCount = getCountFromQW(mw_one->ui->qwMainDate);
  int topIndex = tw->topLevelItemCount() - maindateCount + maindateIndex;
  int childIndex = getCurrentIndexFromQW(mw_one->ui->qwMainEvent);
  tw->setCurrentItem(tw->topLevelItem(topIndex)->child(childIndex));

  if (topIndex < 0) return;
  if (childIndex < 0) return;

  mw_one->on_twItemClicked();

  setMainTabCurrentIndex();
}

void Method::reeditMainEventData() {
  QTreeWidget *tw = mw_one->get_tw(mw_one->ui->tabWidget->currentIndex());
  int maindateIndex = getCurrentIndexFromQW(mw_one->ui->qwMainDate);
  int maindateCount = getCountFromQW(mw_one->ui->qwMainDate);
  int topIndex = tw->topLevelItemCount() - maindateCount + maindateIndex;
  int childIndex = getCurrentIndexFromQW(mw_one->ui->qwMainEvent);

  if (topIndex < 0) return;
  if (childIndex < 0) return;

  tw->setCurrentItem(tw->topLevelItem(topIndex)->child(childIndex));
  mw_one->on_twItemDoubleClicked();
}

void Method::showNoteBookMenu(int x, int y) {
  QMenu *mainMenu = new QMenu(this);
  mw_one->m_NotesList->init_NoteBookMenu(mainMenu);

  QPoint pos(mw_one->geometry().x() + x, mw_one->geometry().y() + y);
  mainMenu->exec(pos);
}

void Method::showNotsListMenu(int x, int y) {
  QMenu *mainMenu = new QMenu(this);
  mw_one->m_NotesList->init_NotesListMenu(mainMenu);

  QPoint pos(mw_one->geometry().x() + x, mw_one->geometry().y() + y);
  mainMenu->exec(pos);
}

void Method::setTypeRenameText() {
  int index = getCurrentIndexFromQW(mw_one->ui->qwCategory);
  QString str = getText0(mw_one->ui->qwCategory, index);
  mw_one->ui->editRenameType->setText(str);
}

void Method::okType() {
  int index = getCurrentIndexFromQW(mw_one->ui->qwCategory);
  m_CategoryList->ui->listWidget->setCurrentRow(index);
  QListWidgetItem *item = m_CategoryList->ui->listWidget->currentItem();
  m_CategoryList->on_listWidget_itemDoubleClicked(item);
  m_CategoryList->on_btnCancel_clicked();
}

void Method::setSCrollPro(QObject *obj) {
  QScrollerProperties sp;
  sp.setScrollMetric(QScrollerProperties::DragStartDistance, 0.001);
  sp.setScrollMetric(QScrollerProperties::ScrollingCurve,
                     QEasingCurve::OutQuad);
  sp.setScrollMetric(QScrollerProperties::DragVelocitySmoothingFactor, 0.001);
  sp.setScrollMetric(QScrollerProperties::FrameRate,
                     QScrollerProperties::FrameRates::Fps60);
  QScroller *qs = QScroller::scroller(obj);
  qs->setScrollerProperties(sp);
}

QDialog *Method::getProgBar() {
  QDialog *dlg;
  dlg = new QDialog(this);
  dlg->setWindowFlag(Qt::FramelessWindowHint);
  dlg->setModal(true);
  dlg->setFixedHeight(200);
  if (isAndroid)
    dlg->setFixedWidth(mw_one->geometry().width() - 50);
  else
    dlg->setFixedWidth(320);
  QVBoxLayout *vbox = new QVBoxLayout;
  dlg->setLayout(vbox);

  QLabel *lbl = new QLabel();
  if (isDark)
    lbl->setStyleSheet("color:#ffffff;");
  else
    lbl->setStyleSheet("color:#000000;");
  QFont font = this->font();
  int size = font.pointSize();
  font.setBold(true);
  font.setPointSize(size + 2);
  lbl->setFont(font);
  lbl->setText(tr("Reading, please wait..."));
  lbl->setAlignment(Qt::AlignHCenter);  // | Qt::AlignVCenter);
  vbox->addWidget(lbl);

  if (nProgressBarType == 1) {
    QProgressBar *prog = new QProgressBar(this);
    prog->setStyleSheet(
        "QProgressBar{border:0px solid #FFFFFF;"
        "height:25;"
        "background:rgb(25,255,25);"
        "text-align:right;"
        "color:rgb(255,255,255);"
        "border-radius:0px;}"

        "QProgressBar:chunk{"
        "border-radius:0px;"
        "background-color:rgba(18,150,219,255);"
        "}");
    prog->setMaximum(0);
    prog->setMinimum(0);
    vbox->addWidget(prog);
  }

  if (nProgressBarType == 2) {
    IOSCircularProgress *progress = new IOSCircularProgress(this);
    progress->setProgress(0.00);  // 设置进度值0~1
    vbox->addWidget(progress, 0, Qt::AlignHCenter | Qt::AlignVCenter);
  }

  dlg->setGeometry(
      mw_one->geometry().x() + (mw_one->width() - dlg->width()) / 2,
      mw_one->geometry().y() + (mw_one->height() - dlg->height()) / 2,
      dlg->width(), dlg->height());

  return dlg;
}

void Method::saveRecycleTabName(QString keyStr, QString tabName) {
  QSettings Reg(iniDir + "del_tabname.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif

  Reg.setValue(keyStr, tabName);
}

QString Method::getRecycleTabName(QString keyStr) {
  QSettings Reg(iniDir + "del_tabname.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif

  QString tabName = Reg.value(keyStr).toString();
  if (tabName.trimmed().length() == 0) tabName = "None";
  return tabName;
}

void Method::showDelMsgBox(QString title, QString info) {
  bool isOK;

  QDialog *dlg = new QDialog(this);
  QVBoxLayout *vbox0 = new QVBoxLayout;
  dlg->setLayout(vbox0);
  dlg->setModal(true);
  dlg->setWindowFlag(Qt::FramelessWindowHint);
  dlg->setAttribute(Qt::WA_TranslucentBackground);

  QFrame *frame = new QFrame(this);
  vbox0->addWidget(frame);
  frame->setStyleSheet(
      "QFrame{background-color: rgb(255, 255, 255);border-radius:10px; "
      "border:0px solid gray;}");

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->setContentsMargins(12, 12, 12, 12);
  vbox->setSpacing(12);
  frame->setLayout(vbox);

  QLabel *lblTitle = new QLabel(this);
  lblTitle->adjustSize();
  lblTitle->setWordWrap(true);
  lblTitle->setText(title);
  vbox->addWidget(lblTitle);

  QFrame *hframe = new QFrame(this);
  hframe->setFrameShape(QFrame::HLine);
  hframe->setStyleSheet("QFrame{background:red;min-height:2px}");
  vbox->addWidget(hframe);

  QLabel *lbl = new QLabel(this);
  lbl->adjustSize();
  lbl->setWordWrap(true);
  lbl->setText(info);
  vbox->addWidget(lbl);

  QToolButton *btnCancel = new QToolButton(this);
  QToolButton *btnOk = new QToolButton(this);
  btnCancel->setText(tr("Cancel"));
  btnOk->setText(tr("Delete"));
  btnOk->setStyleSheet(
      "QToolButton {background-color: rgb(255, 0, 0);color: rgb(255, "
      "255, 255);border-radius:10px;border:0px solid gray;} "
      "QToolButton:pressed "
      "{ background-color: "
      "rgb(220,220,230);color: black}");

  btnCancel->setStyleSheet(btnStyle);
  btnOk->setFixedHeight(35);
  btnCancel->setFixedHeight(35);

  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget(btnCancel);
  hbox->addWidget(btnOk);
  btnCancel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  btnOk->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  QSpacerItem *sparcer_item =
      new QSpacerItem(0, 160, QSizePolicy::Fixed, QSizePolicy::Expanding);
  vbox->addItem(sparcer_item);

  vbox->addLayout(hbox, 0);

  isOK = false;
  connect(btnCancel, &QToolButton::clicked, [=]() mutable {
    isOK = false;
    dlg->close();
  });
  connect(dlg, &QDialog::rejected, [=]() mutable { closeGrayWindows(); });
  connect(btnOk, &QToolButton::clicked, [=]() mutable {
    isOK = true;
    dlg->close();
  });

  int x, y, w, h;
  w = mw_one->width() - 40;
  x = mw_one->geometry().x() + (mw_one->width() - w) / 2;
  h = mw_one->calcStringPixelHeight(this->font(), fontSize) * 15;
  y = geometry().y() + (height() - h) / 2;
  dlg->setGeometry(x, y, w, h);

  showGrayWindows();

  dlg->exec();
}

QFont Method::getNewFont(int maxSize) {
  QFont font0 = this->font();
  if (fontSize > maxSize)
    font0.setPointSize(maxSize);
  else
    font0.setPointSize(fontSize);

  return font0;
}

void Method::setDark(QString strDark) {
  Q_UNUSED(strDark);
#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject javaDark = QAndroidJniObject::fromString(strDark);
  QAndroidJniObject jo = QAndroidJniObject::fromString("dark");
  jo.callMethod<void>("com.x/MyActivity", "setDark", "(Ljava/lang/String;)V",
                      javaDark.object<jstring>());

#else
  QJniObject javaDark = QJniObject::fromString(strDark);
  QJniObject jo = QJniObject::fromString("dark");
  jo.callMethod<void>("com.x/MyActivity", "setDark", "(Ljava/lang/String;)V",
                      javaDark.object<jstring>());

#endif

#endif
}

void Method::set_ToolButtonStyle2(QObject *parent) {
  QObjectList btnList =
      mw_one->getAllToolButton(mw_one->getAllUIControls(parent));
  for (int i = 0; i < btnList.count(); i++) {
    QToolButton *btn = (QToolButton *)btnList.at(i);
    setToolButtonQss(btn, 5, 3, "#009999", "#FFFFFF", "#009999", "#FFFFFF",
                     "#009090", "#EEEEEE");
  }
}

QString Method::setToolButtonQss(QToolButton *btn, int radius, int padding,
                                 const QString &normalColor,
                                 const QString &normalTextColor,
                                 const QString &hoverColor,
                                 const QString &hoverTextColor,
                                 const QString &pressedColor,
                                 const QString &pressedTextColor) {
  QStringList list;
  list.append(QString("QToolButton{border-style:none;padding:%1px;border-"
                      "radius:%2px;color:%3;background:%4;}")
                  .arg(padding)
                  .arg(radius)
                  .arg(normalTextColor)
                  .arg(normalColor));
  list.append(QString("QToolButton:hover{color:%1;background:%2;}")
                  .arg(hoverTextColor)
                  .arg(hoverColor));
  list.append(QString("QToolButton:pressed{color:%1;background:%2;}")
                  .arg(pressedTextColor)
                  .arg(pressedColor));

  QString qss = list.join("");
  btn->setStyleSheet(qss);
  return qss;
}

QString Method::setPushButtonQss(QPushButton *btn, int radius, int padding,
                                 const QString &normalColor,
                                 const QString &normalTextColor,
                                 const QString &hoverColor,
                                 const QString &hoverTextColor,
                                 const QString &pressedColor,
                                 const QString &pressedTextColor) {
  QStringList list;
  list.append(QString("QPushButton{border-style:none;padding:%1px;border-"
                      "radius:%2px;color:%3;background:%4;}")
                  .arg(padding)
                  .arg(radius)
                  .arg(normalTextColor)
                  .arg(normalColor));
  list.append(QString("QPushButton:hover{color:%1;background:%2;}")
                  .arg(hoverTextColor)
                  .arg(hoverColor));
  list.append(QString("QPushButton:pressed{color:%1;background:%2;}")
                  .arg(pressedTextColor)
                  .arg(pressedColor));

  QString qss = list.join("");
  btn->setStyleSheet(qss);
  return qss;
}

void Method::setVPosForQW(QQuickWidget *qw, qreal pos) {
  QQuickItem *root = qw->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "setVPos", Q_ARG(QVariant, pos));
}

qreal Method::getVPosForQW(QQuickWidget *qw) {
  QVariant itemCount;
  QQuickItem *root = qw->rootObject();
  QMetaObject::invokeMethod((QObject *)root, "getVPos",
                            Q_RETURN_ARG(QVariant, itemCount));
  qreal textPos = itemCount.toDouble();
  return textPos;
}

QString Method::getCustomColor() {
  QString strColor;

#ifdef Q_OS_ANDROID
  m_widget = new QWidget(this);
  showGrayWindows();

  ColorDialog *colorDlg = new ColorDialog(this);
  connect(colorDlg, &QDialog::rejected, [=]() mutable { closeGrayWindows(); });
  int x, y, w, h;
  x = mw_one->geometry().x();
  y = mw_one->geometry().y();
  w = mw_one->width();
  h = mw_one->ui->frameMain->height() - 50;
  colorDlg->setFixedWidth(w);
  colorDlg->setFixedHeight(h);
  colorDlg->setGeometry(x + (mw_one->width() - w) / 2, y, w, h);
  if (colorDlg->exec() == QDialog::Accepted) {
    strColor = ColorToString(colorDlg->getColor());
    closeGrayWindows();
  }
#else

  QColorDialog *colorDlg = new QColorDialog(this);
  QFont f = getNewFont(17);

  colorDlg->setFont(f);
  if (colorDlg->exec()) {
    QColor color = colorDlg->selectedColor();
    strColor = ColorToString(color);
    qDebug() << color << strColor;
  }
#endif

  return strColor.toUpper();
}

QString Method::ColorToString(QColor v_color) {
  QRgb mRgb = qRgb(v_color.red(), v_color.green(), v_color.blue());
  QString mRgbStr = QString::number(mRgb, 16);
  mRgbStr = mRgbStr.replace(0, 2, "#");
  return mRgbStr;
}

QString Method::getKeyType() {
  QSettings Reg("/storage/emulated/0/.Knot/shortcut.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif

  return Reg.value("/desk/keyType", "todo").toString();
}

QString Method::getExecDone() {
  QSettings Reg("/storage/emulated/0/.Knot/shortcut.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif

  return Reg.value("/desk/execDone", "true").toString();
}

void Method::setExecDone(QString execDone) {
  QSettings Reg("/storage/emulated/0/.Knot/shortcut.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif

  Reg.setValue("/desk/execDone", execDone);
}

void Method::Delay_MSec(unsigned int msec) {
  QEventLoop loop;
  QTimer::singleShot(msec, &loop, SLOT(quit()));
  loop.exec();
}

void Method::Sleep(int msec) {
  QTime dieTime = QTime::currentTime().addMSecs(msec);
  while (QTime::currentTime() < dieTime)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void Method::showToastMessage(QString msg) {
  Q_UNUSED(msg);
#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject msgObject = QAndroidJniObject::fromString(msg);
  QAndroidJniObject m_activity = QtAndroid::androidActivity();
  m_activity.callMethod<void>("showToastMessage", "(Ljava/lang/String;)V",
                              msgObject.object<jstring>());
#else
  QJniObject msgObject = QJniObject::fromString(msg);
  QJniObject m_activity = QJniObject::fromString("showToastMessage");
  m_activity.callMethod<void>("showToastMessage", "(Ljava/lang/String;)V",
                              msgObject.object<jstring>());
#endif

#endif
}

void Method::openFilePicker() {
#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("openFilePicker", "()V");
#else
  QJniObject activity = QJniObject::fromString("openNoteEditor");
  activity.callMethod<void>("openFilePicker", "()V");
#endif

#endif
}

void Method::closeFilePicker() {
#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("closeFilePicker", "()V");
#else
  QJniObject activity = QJniObject::fromString("openNoteEditor");
  activity.callMethod<void>("closeFilePicker", "()V");
#endif

#endif
}

void Method::setAndroidProgressInfo(QString info) {
  Q_UNUSED(info);

  if (mw_one->initMain) return;

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject strInfo = QAndroidJniObject::fromString(info);
  QAndroidJniObject m_activity =
      QAndroidJniObject::fromString("setProgressInfo");
  m_activity.callStaticMethod<void>("com.x/MyProgBar", "setProgressInfo",
                                    "(Ljava/lang/String;)V",
                                    strInfo.object<jstring>());
#else
  QJniObject strInfo = QJniObject::fromString(info);
  QJniObject m_activity = QJniObject::fromString("setProgressInfo");
  m_activity.callStaticMethod<void>("com.x/MyProgBar", "setProgressInfo",
                                    "(Ljava/lang/String;)V",
                                    strInfo.object<jstring>());
#endif

#endif
}

void Method::showTempActivity() {
#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("showTempActivity", "()V");
#else
  QJniObject activity = QJniObject::fromString("showTempActivity");
  activity.callMethod<void>("showTempActivity", "()V");
#endif

#endif
}

void Method::showAndroidProgressBar() {
  if (mw_one->initMain) return;

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("showAndroidProgressBar", "()V");
#else
  QJniObject activity = QJniObject::fromString("showAndroidProgressBar");
  activity.callMethod<void>("showAndroidProgressBar", "()V");
#endif

#endif
}

void Method::closeAndroidProgressBar() {
  if (mw_one->initMain) return;

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject activity =
      QAndroidJniObject::fromString("closeAndroidProgressBar");
  activity.callStaticMethod<void>("com.x/MyProgBar", "closeAndroidProgressBar",
                                  "()V");
#else
  QJniObject activity = QJniObject::fromString("closeAndroidProgressBar");
  activity.callStaticMethod<void>("com.x/MyProgBar", "closeAndroidProgressBar",
                                  "()V");
#endif

#endif
}

void Method::setQLabelImage(QLabel *lbl, int w, int h, QString imgFile) {
  lbl->setFixedHeight(h);
  lbl->setFixedWidth(w);
  QPixmap *pixmap = new QPixmap(imgFile);
  pixmap->scaled(lbl->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
  lbl->setScaledContents(true);
  lbl->setPixmap(*pixmap);
}

void Method::playMyText(QString text) {
  Q_UNUSED(text);

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jText = QAndroidJniObject::fromString(text);
  QAndroidJniObject activity = QAndroidJniObject::fromString("playMyText");
  activity.callStaticMethod<void>("com.x/MyActivity", "playMyText",
                                  "(Ljava/lang/String;)V",
                                  jText.object<jstring>());

#else
  QJniObject jText = QJniObject::fromString(text);
  QJniObject activity = QJniObject::fromString("playMyText");
  activity.callStaticMethod<void>("com.x/MyActivity", "playMyText",
                                  "(Ljava/lang/String;)V",
                                  jText.object<jstring>());

#endif

#endif
}

void Method::stopPlayMyText() {
#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

  QAndroidJniObject activity = QAndroidJniObject::fromString("stopPlayMyText");
  activity.callStaticMethod<void>("com.x/MyActivity", "stopPlayMyText", "()V");

#else

  QJniObject activity = QJniObject::fromString("stopPlayMyText");
  activity.callStaticMethod<void>("com.x/MyActivity", "stopPlayMyText", "()V");

#endif

#endif
}

int Method::checkRecordAudio() {
#ifdef Q_OS_ANDROID
  bool isOk;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

  QAndroidJniObject activity =
      QAndroidJniObject::fromString("checkRecordAudio");
  isOk = activity.callStaticMethod<int>("com.x/MyActivity", "checkRecordAudio",
                                        "()I");

#else

  QJniObject activity = QJniObject::fromString("checkRecordAudio");
  isOk = activity.callStaticMethod<int>("com.x/MyActivity", "checkRecordAudio",
                                        "()I");

#endif
  return isOk;

#else
  return false;

#endif
}

void Method::startRecord(QString file) {
  Q_UNUSED(file);

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jFile = QAndroidJniObject::fromString(file);
  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("startRecord", "(Ljava/lang/String;)V",
                            jFile.object<jstring>());

#else
  QJniObject jFile = QJniObject::fromString(file);
  QJniObject activity =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  activity.callMethod<void>("startRecord", "(Ljava/lang/String;)V",
                            jFile.object<jstring>());

#endif

#endif
}

void Method::stopRecord() {
#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("stopRecord", "()V");

#else

  QJniObject activity =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  activity.callMethod<void>("stopRecord", "()V");

#endif

#endif
}

void Method::playRecord(QString file) {
  Q_UNUSED(file);

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jFile = QAndroidJniObject::fromString(file);
  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("playRecord", "(Ljava/lang/String;)V",
                            jFile.object<jstring>());

#else
  QJniObject jFile = QJniObject::fromString(file);
  QJniObject activity =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  activity.callMethod<void>("playRecord", "(Ljava/lang/String;)V",
                            jFile.object<jstring>());

#endif

#endif
}

void Method::startPlay() {
#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("startPlay", "()V");

#else

  QJniObject activity =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  activity.callMethod<void>("startPlay", "()V");

#endif

#endif
}

void Method::pausePlay() {
#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("pausePlay", "()V");

#else

  QJniObject activity =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  activity.callMethod<void>("pausePlay", "()V");

#endif

#endif
}

void Method::stopPlayRecord() {
#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("stopPlayRecord", "()V");

#else

  QJniObject activity =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  activity.callMethod<void>("stopPlayRecord", "()V");

#endif

#endif
}

QString Method::FormatHHMMSS(qint32 total) {
  int hh = total / (60 * 60);
  int mm = (total - (hh * 60 * 60)) / 60;
  int ss = (total - (hh * 60 * 60)) - mm * 60;

  QString hour = QString::number(hh, 10);
  QString min = QString::number(mm, 10);
  QString sec = QString::number(ss, 10);

  if (hour.length() == 1) hour = "0" + hour;
  if (min.length() == 1) min = "0" + min;
  if (sec.length() == 1) sec = "0" + sec;

  QString strTime = hour + ":" + min + ":" + sec;
  return strTime;
}

void Method::openDateTimePicker() {
#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("openDateTimePicker", "()V");
#else
  QJniObject activity = QJniObject::fromString("openNoteEditor");
  activity.callMethod<void>("openDateTimePicker", "()V");
#endif

#endif
}

void Method::setDateTimePickerFlag(QString flag, int y, int m, int d, int h,
                                   int mm, QString dateFlag) {
  QSettings Reg(privateDir + "datetime.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif
  if (flag == "ymd") {
    Reg.setValue("/DateTime/flag", "ymd");
  }

  if (flag == "ym") {
    Reg.setValue("/DateTime/flag", "ym");
  }

  if (flag == "hm") {
    Reg.setValue("/DateTime/flag", "hm");
  }

  if (y == 0) y = 2022;
  if (m == 0) m = 1;
  if (d == 0) d = 1;
  if (h == 0) h = 0;
  if (mm == 0) mm = 0;

  Reg.setValue("/DateTime/y", y);
  Reg.setValue("/DateTime/m", m);
  Reg.setValue("/DateTime/d", d);
  Reg.setValue("/DateTime/h", h);
  Reg.setValue("/DateTime/mm", mm);

  Reg.setValue("/DateTime/dateFlag", dateFlag);
}

QStringList Method::getDateTimePickerValue() {
  QStringList list;
  QSettings Reg(privateDir + "datetime.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif
  list.append(Reg.value("/DateTime/y", 2022).toString());
  list.append(Reg.value("/DateTime/m", 3).toString());
  list.append(Reg.value("/DateTime/d", 3).toString());
  list.append(Reg.value("/DateTime/h", 10).toString());
  list.append(Reg.value("/DateTime/mm", 0).toString());
  return list;
}

QString Method::getDateTimeFlag() {
  QSettings Reg(privateDir + "datetime.ini", QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif
  return Reg.value("/DateTime/dateFlag", "").toString();
}

double Method::updateMicStatus() {
  double a;

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jo = QtAndroid::androidActivity();
  a = jo.callMethod<double>("updateMicStatus", "()D");
#else
  QJniObject jo =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  a = jo.callMethod<double>("updateMicStatus", "()D");
#endif

#endif
  return a;
}

int Method::getPlayDuration() {
  int a;

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jo = QtAndroid::androidActivity();
  a = jo.callMethod<int>("getPlayDuration", "()I");
#else
  QJniObject jo =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  a = jo.callMethod<int>("getPlayDuration", "()I");
#endif

#endif
  return a;
}

int Method::getPlayPosition() {
  int a;

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jo = QtAndroid::androidActivity();
  a = jo.callMethod<int>("getPlayPosition", "()I");
#else
  QJniObject jo =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  a = jo.callMethod<int>("getPlayPosition", "()I");
#endif

#endif
  return a;
}

bool Method::getPlaying() {
  bool a;

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jo = QtAndroid::androidActivity();
  a = jo.callMethod<int>("isPlaying", "()I");
#else
  QJniObject jo =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  a = jo.callMethod<int>("isPlaying", "()I");
#endif

#endif
  return a;
}

void Method::seekTo(QString strPos) {
  Q_UNUSED(strPos);

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jFile = QAndroidJniObject::fromString(strPos);
  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("seekTo", "(Ljava/lang/String;)V",
                            jFile.object<jstring>());

#else
  QJniObject jFile = QJniObject::fromString(strPos);
  QJniObject activity =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  activity.callMethod<void>("seekTo", "(Ljava/lang/String;)V",
                            jFile.object<jstring>());

#endif

#endif
}

void Method::setMDTitle(QString strTitle) {
  Q_UNUSED(strTitle);

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jFile = QAndroidJniObject::fromString(strTitle);
  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("setMDTitle", "(Ljava/lang/String;)V",
                            jFile.object<jstring>());

#else
  QJniObject jFile = QJniObject::fromString(strTitle);
  QJniObject activity =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  activity.callMethod<void>("setMDTitle", "(Ljava/lang/String;)V",
                            jFile.object<jstring>());

#endif

#endif
}

void Method::setMDFile(QString strMDFile) {
  Q_UNUSED(strMDFile);

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject jFile = QAndroidJniObject::fromString(strMDFile);
  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<void>("setMDFile", "(Ljava/lang/String;)V",
                            jFile.object<jstring>());

#else
  QJniObject jFile = QJniObject::fromString(strMDFile);
  QJniObject activity =
      QJniObject(QCoreApplication::instance()
                     ->nativeInterface<QNativeInterface::QAndroidApplication>()
                     ->context());
  activity.callMethod<void>("setMDFile", "(Ljava/lang/String;)V",
                            jFile.object<jstring>());

#endif

#endif
}

void Method::delay_MSec(unsigned int msec) {
  QEventLoop loop;
  QTimer::singleShot(msec, &loop, SLOT(quit()));
  loop.exec();
}

void Method::setAndroidFontSize(int nSize) {
#ifdef Q_OS_ANDROID
  QAndroidJniObject activity = QtAndroid::androidActivity();
  if (activity.isValid()) {
    // 调用 setFontSize 方法
    activity.callMethod<void>("setFontSize", "(I)V", nSize);
  }
#endif
}

bool Method::decompressWithPassword(const QString &zipPath,
                                    const QString &extractDir,
                                    const QString &password) {
  QuaZip zip(zipPath);
  if (!zip.open(QuaZip::mdUnzip)) {
    qWarning() << "[ERROR] Failed to open zip:" << zip.getZipError();
    isPasswordError = true;
    return false;
  }

  ///////

  // 预检查密码：找到第一个加密文件并验证密码有效性
  isPasswordError = false;
  bool hasEncrypted = false;
  bool passwordValid = true;

  for (bool f = zip.goToFirstFile(); f; f = zip.goToNextFile()) {
    QuaZipFileInfo64 info;
    if (!zip.getCurrentFileInfo(&info)) {
      qWarning() << "[WARN] Failed to get file info during password check";
      continue;
    }

    if (info.isEncrypted()) {
      hasEncrypted = true;
      QuaZipFile testFile(&zip);
      QByteArray passBytes = password.toUtf8();

      // 尝试打开加密文件
      if (!testFile.open(QIODevice::ReadOnly, passBytes.constData())) {
        passwordValid = false;
        qWarning() << "[ERROR] Password open failed for"
                   << zip.getCurrentFileName();

        break;
      }

      // 尝试读取数据以验证密码
      char buffer[1024];
      qint64 bytesRead = testFile.read(buffer, sizeof(buffer));
      testFile.close();  // 无论读取是否成功都需关闭文件

      if (bytesRead == -1) {  // 读取失败表明密码错误
        passwordValid = false;
        qWarning() << "[ERROR] Password read failed for"
                   << zip.getCurrentFileName();

        break;
      }

      // 找到第一个加密文件并验证通过后即可退出检查
      break;
    }
  }

  // 处理加密文件逻辑
  if (hasEncrypted) {
    if (password.isEmpty()) {
      qWarning() << "[ERROR] Password required but empty";
      zip.close();
      isPasswordError = true;
      return false;
    }
    if (!passwordValid) {
      qWarning() << "[ERROR] Incorrect password";
      zip.close();
      isPasswordError = true;
      return false;
    }
  }

  // 重置到第一个文件以开始解压
  zip.goToFirstFile();

  ///////

  QDir outputDir(extractDir);
  if (!outputDir.mkpath(".")) {
    qWarning() << "[ERROR] Cannot create output directory";
    return false;
  }

  QuaZipFile file(&zip);
  bool success = true;

  // 遍历所有文件
  for (bool fileExists = zip.goToFirstFile(); fileExists && success;
       fileExists = zip.goToNextFile()) {
    const QString fileName = zip.getCurrentFileName();
    const QString absPath = outputDir.absoluteFilePath(fileName);

    // 处理目录项
    if (fileName.endsWith('/')) {
      QDir().mkpath(absPath);
      qDebug() << "[INFO] Created directory:" << absPath;
      continue;
    }

    // 创建父目录
    QFileInfo fileInfo(absPath);
    if (!fileInfo.dir().mkpath(".")) {
      qWarning() << "[ERROR] Cannot create parent directory for" << absPath;
      success = false;
      break;
    }

    // 获取文件信息
    QuaZipFileInfo64 fileInfo1;
    if (!zip.getCurrentFileInfo(&fileInfo1)) {
      qWarning() << "[ERROR] Failed to get file info for"
                 << zip.getCurrentFileName();
      success = false;
      break;
    }

    // 根据加密状态打开文件
    isPasswordError = false;
    bool openSuccess;
    if (fileInfo1.isEncrypted()) {
      if (password.isEmpty()) {
        qWarning() << "[ERROR] File is encrypted but password is empty";

        success = false;
        break;
      }

      // 确保密码使用 UTF-8 编码
      QByteArray passwordBytes = password.toUtf8();
      const char *passData = passwordBytes.constData();

      openSuccess = file.open(QIODevice::ReadOnly, passData);
    } else {
      openSuccess = file.open(QIODevice::ReadOnly);
    }

    if (!openSuccess) {
      const int errorCode = file.getZipError();
      qWarning() << "[ERROR] Failed to open file" << fileName
                 << "Error code:" << errorCode
                 << "Description:" << quazipErrorString(errorCode);

      success = false;
      break;
    }

    QFile outFile(absPath);
    if (!outFile.open(QIODevice::WriteOnly)) {
      qWarning() << "[ERROR] Cannot open output file" << absPath;
      file.close();
      success = false;
      break;
    }

    // 分块读写（优化缓冲区管理）
    constexpr qint64 BUFFER_SIZE = 4 * 1024 * 1024;  // 4MB
    QByteArray buffer;
    buffer.resize(BUFFER_SIZE);

    qint64 totalBytes = 0;
    bool readSuccess = true;

    do {
      const qint64 bytesRead = file.read(buffer.data(), BUFFER_SIZE);

      if (bytesRead == -1) {
        qWarning() << "[ERROR] Read failed for" << fileName;
        readSuccess = false;
        break;
      } else if (bytesRead == 0) {
        break;  // 正常结束
      }

      const qint64 bytesWritten = outFile.write(buffer.constData(), bytesRead);
      if (bytesWritten != bytesRead) {
        qWarning() << "[ERROR] Write mismatch for" << fileName
                   << "Read:" << bytesRead << "Written:" << bytesWritten;
        readSuccess = false;
        break;
      }

      totalBytes += bytesWritten;
    } while (true);

    file.close();
    outFile.close();

    if (!readSuccess) {  //|| totalBytes == 0) {
      qWarning() << "[WARN] Removed incomplete file:" << absPath;

      // QFile::remove(absPath);
      // success = false;
      // break;
    }

    qDebug() << "[SUCCESS] Extracted" << fileName << "Size:" << totalBytes
             << "bytes";
  }

  zip.close();

  qDebug() << "unzip success=" << success;

  return success;
}

// QuaZip错误码转字符串
QString Method::quazipErrorString(int code) {
  switch (code) {
    case UNZ_OK:
      return "No error";
    case UNZ_END_OF_LIST_OF_FILE:
      return "End of file list";
    case UNZ_ERRNO:
      return "File IO error";
    case UNZ_PARAMERROR:
      return "Invalid parameter";
    default:
      return QString("Unknown error (%1)").arg(code);
  }
}

bool Method::compressDirectory(const QString &zipPath, const QString &sourceDir,
                               const QString &password) {
  QuaZip zip(zipPath);
  zip.setFileNameCodec("UTF-8");
  zip.setZip64Enabled(false);  // 禁用 ZIP64（除非必要）

  if (!zip.open(QuaZip::mdCreate)) {
    qWarning() << "Failed to create zip:" << zip.getZipError();
    return false;
  }

  QDir srcDir(sourceDir);
  QString parentPath = QFileInfo(srcDir.absolutePath()).path();

  QDirIterator it(sourceDir, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,
                  QDirIterator::Subdirectories);

  while (it.hasNext()) {
    QString filePath = it.next();
    QFileInfo fileInfo(filePath);

    // 计算相对路径（包含顶层目录名）
    QString relativePath = QDir(parentPath).relativeFilePath(filePath);
    relativePath = QDir::toNativeSeparators(relativePath).replace("\\", "/");

    // 如果是目录，手动添加目录条目
    if (fileInfo.isDir()) {
      relativePath += "/";
      QuaZipFile dirFile(&zip);
      QuaZipNewInfo dirInfo(relativePath);

      // 设置目录的外部属性，使其更具兼容性
      dirInfo.externalAttr = (0755 << 16);

      // 仅设置DOS目录属性,避免混合UNIX属性导致的兼容性问题
      // dirInfo.externalAttr = (0 << 16) | 0x10;

      if (!dirFile.open(QIODevice::WriteOnly, dirInfo, nullptr, 0, 8)) {
        qWarning() << "Failed to add directory:" << relativePath;
        return false;
      }
      dirFile.close();
      continue;
    }

    // 添加文件
    QuaZipFile zipFile(&zip);
    QuaZipNewInfo newInfo(relativePath, filePath);
    newInfo.setFileDateTime(filePath);

    // 设置加密参数
    const bool useEncryption = !password.isEmpty();

    // 确保密码使用 UTF-8 编码
    QByteArray passwordBytes = password.toUtf8();
    const char *passData = useEncryption ? passwordBytes.constData() : nullptr;

    // 重要：与7zip的压缩参数完全一直，高度兼容，特别是加密的时候
    if (!zipFile.open(QIODevice::WriteOnly, newInfo, passData, 0, Z_DEFLATED,
                      Z_DEFAULT_COMPRESSION, false, 15, 9,
                      Z_DEFAULT_STRATEGY)) {
      qWarning() << "Failed to add file:" << relativePath;
      return false;
    }

    QFile inFile(filePath);
    if (!inFile.open(QIODevice::ReadOnly)) return false;

    // 分块写入
    constexpr qint64 BUFFER_SIZE = 4 * 1024 * 1024;  // 4MB
    QByteArray buffer;
    buffer.resize(BUFFER_SIZE);

    qint64 bytesRead;
    while ((bytesRead = inFile.read(buffer.data(), BUFFER_SIZE)) > 0) {
      if (zipFile.write(buffer.constData(), bytesRead) != bytesRead) {
        zipFile.close();
        inFile.close();
        return false;
      }
    }

    zipFile.close();
    inFile.close();
  }

  zip.close();
  return true;
}

bool Method::compressFile(const QString &zipPath, const QString &filePath,
                          const QString &password) {
  QFileInfo fi(zipPath);
  QString strDir = fi.absolutePath();
  QDir dir;
  dir.mkpath(strDir);
  QFile::remove(zipPath);

  QuaZip zip(zipPath);
  zip.setFileNameCodec("UTF-8");
  zip.setZip64Enabled(false);  // 禁用 ZIP64（除非必要）

  if (!zip.open(QuaZip::mdCreate)) {
    qWarning() << "Failed to create zip:" << zip.getZipError();
    return false;
  }

  QString relativePath = QFileInfo(filePath).fileName();

  // 添加文件
  QuaZipFile zipFile(&zip);
  QuaZipNewInfo newInfo(relativePath, filePath);
  newInfo.setFileDateTime(filePath);

  // 设置加密参数
  const bool useEncryption = !password.isEmpty();

  // 确保密码使用 UTF-8 编码
  QByteArray passwordBytes = password.toUtf8();
  const char *passData = useEncryption ? passwordBytes.constData() : nullptr;

  // 重要：与7zip的压缩参数完全一直，高度兼容，特别是加密的时候
  if (!zipFile.open(QIODevice::WriteOnly, newInfo, passData, 0, Z_DEFLATED,
                    Z_DEFAULT_COMPRESSION, false, 15, 9, Z_DEFAULT_STRATEGY)) {
    qWarning() << "Failed to add file:" << relativePath;
    return false;
  }

  QFile inFile(filePath);
  if (!inFile.open(QIODevice::ReadOnly)) return false;

  // 分块写入
  constexpr qint64 BUFFER_SIZE = 4 * 1024 * 1024;  // 4MB
  QByteArray buffer;
  buffer.resize(BUFFER_SIZE);

  qint64 bytesRead;
  while ((bytesRead = inFile.read(buffer.data(), BUFFER_SIZE)) > 0) {
    if (zipFile.write(buffer.constData(), bytesRead) != bytesRead) {
      zipFile.close();
      inFile.close();
      return false;
    }
  }

  zipFile.close();
  inFile.close();

  zip.close();

  qDebug() << "zipPath=" << zipPath << "filePath=" << filePath;

  return true;
}

QByteArray Method::generateRandomBytes(int length) {
  QByteArray bytes(length, 0);
  if (RAND_bytes(reinterpret_cast<unsigned char *>(bytes.data()), length) !=
      1) {
    return QByteArray();  // 返回空表示生成失败
  }
  return bytes;
}

QByteArray Method::deriveKey(const QString &password, const QByteArray &salt,
                             int keyLength) {
  QByteArray key(keyLength, 0);
  const int iterations = 10000;  // 迭代次数
  const EVP_MD *digest = EVP_sha256();

  if (PKCS5_PBKDF2_HMAC(
          password.toUtf8().constData(),  // 密码明文
          password.length(),              // 密码长度
          reinterpret_cast<const unsigned char *>(salt.constData()),  // 盐值
          salt.size(),                                                // 盐长度
          iterations,                                    // 迭代次数
          digest,                                        // 哈希算法
          keyLength,                                     // 输出密钥长度
          reinterpret_cast<unsigned char *>(key.data())  // 输出缓冲区
          ) != 1) {
    return QByteArray();  // 返回空表示失败
  }
  return key;
}

bool Method::encryptFile(const QString &inputPath, const QString &outputPath,
                         const QString &password) {
  // 生成随机盐和IV（带错误日志）
  QByteArray salt = generateRandomBytes(16);
  QByteArray iv = generateRandomBytes(16);
  if (salt.isEmpty() || iv.isEmpty()) {
    qDebug() << "Salt/IV生成失败: "
             << ERR_error_string(ERR_get_error(), nullptr);
    return false;
  }

  // 打开文件（处理Android权限）
  QFile inFile(inputPath);
  QFile outFile(outputPath);
  if (!inFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
    qDebug() << "输入文件打开失败: " << inFile.errorString();
    return false;
  }

  qint64 dataSize = inFile.size();

  if (!outFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
    qDebug() << "输出文件打开失败: " << outFile.errorString();
    inFile.close();
    return false;
  }

  // 写入盐和IV
  if (outFile.write(salt) != 16 || outFile.write(iv) != 16) {
    qDebug() << "盐/IV写入失败";
    inFile.close();
    outFile.close();
    return false;
  }

  // 密钥派生（高迭代次数）
  QByteArray key = deriveKey(password, salt, 32);
  if (key.isEmpty()) {
    qDebug() << "密钥派生失败";
    inFile.close();
    outFile.close();
    return false;
  }

  // 初始化加密上下文（显式设置填充）
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (!ctx) {
    qDebug() << "EVP上下文创建失败";
    inFile.close();
    outFile.close();
    return false;
  }
  if (EVP_EncryptInit_ex(
          ctx, EVP_aes_256_cbc(), nullptr,
          reinterpret_cast<const unsigned char *>(key.constData()),
          reinterpret_cast<const unsigned char *>(iv.constData())) != 1) {
    qDebug() << "加密初始化失败: "
             << ERR_error_string(ERR_get_error(), nullptr);
    EVP_CIPHER_CTX_free(ctx);
    inFile.close();
    outFile.close();
    return false;
  }
  EVP_CIPHER_CTX_set_padding(ctx, 1);  // 显式启用PKCS#7填充

  // 动态分配缓冲区（避免栈溢出）
  const int bufferSize = 4096;
  unsigned char *inBuf = new unsigned char[bufferSize];
  unsigned char *outBuf = new unsigned char[bufferSize + EVP_MAX_BLOCK_LENGTH];
  bool success = true;
  int outLen = 0;

  // 分块加密
  while (true) {
    qint64 bytesRead = inFile.read(reinterpret_cast<char *>(inBuf), bufferSize);
    if (bytesRead < 0) {
      qDebug() << "文件读取错误: " << inFile.errorString();
      success = false;
      break;
    } else if (bytesRead == 0) {
      break;  // 文件结束
    }

    if (EVP_EncryptUpdate(ctx, outBuf, &outLen, inBuf, bytesRead) != 1) {
      qDebug() << "加密分块失败: "
               << ERR_error_string(ERR_get_error(), nullptr);
      success = false;
      break;
    }

    qint64 written = outFile.write(reinterpret_cast<char *>(outBuf), outLen);
    if (written != outLen) {
      qDebug() << "加密数据写入不完整: " << outFile.errorString();
      success = false;
      break;
    }
  }

  // 处理最终块
  if (success) {
    if (EVP_EncryptFinal_ex(ctx, outBuf, &outLen) != 1) {
      qDebug() << "最终块加密失败: "
               << ERR_error_string(ERR_get_error(), nullptr);
      success = false;
    } else {
      qint64 written = outFile.write(reinterpret_cast<char *>(outBuf), outLen);
      if (written != outLen) success = false;
    }
  }

  // 清理资源
  delete[] inBuf;
  delete[] outBuf;
  EVP_CIPHER_CTX_free(ctx);
  inFile.close();
  outFile.flush();
  outFile.close();

  // 验证文件大小（可选）
  // 在验证阶段计算预期大小
  if (success) {
    const int blockSize = 16;
    qint64 encryptedDataSize = dataSize + (blockSize - (dataSize % blockSize));
    if (dataSize % blockSize == 0) {
      encryptedDataSize += blockSize;
    }
    qint64 expectedSize = 32 + encryptedDataSize;

    if (outFile.size() != expectedSize) {
      qDebug() << "文件大小验证失败，预期：" << expectedSize << "实际："
               << outFile.size();
      // success = false;
    } else {
      qDebug() << "文件大小验证成功，预期：" << expectedSize << "实际："
               << outFile.size();
    }
  }

  return success;
}

bool Method::encryptFile_Old(const QString &inputPath,
                             const QString &outputPath,
                             const QString &password) {
  // 生成随机盐和IV
  QByteArray salt = generateRandomBytes(16);
  QByteArray iv = generateRandomBytes(16);
  if (salt.isEmpty() || iv.isEmpty()) return false;

  // 打开文件
  QFile inFile(inputPath);
  QFile outFile(outputPath);
  if (!inFile.open(QIODevice::ReadOnly) ||
      !outFile.open(QIODevice::WriteOnly)) {
    return false;
  }

  // 写入盐和IV到文件头部
  outFile.write(salt);
  outFile.write(iv);

  // 密钥派生
  QByteArray key = deriveKey(password, salt, 32);
  if (key.isEmpty()) return false;

  // 初始化加密上下文
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (!ctx ||
      EVP_EncryptInit_ex(
          ctx, EVP_aes_256_cbc(), nullptr,
          reinterpret_cast<const unsigned char *>(key.constData()),
          reinterpret_cast<const unsigned char *>(iv.constData())) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }

  // 分块加密（每次处理4KB）
  const int bufferSize = 4096;
  unsigned char inBuf[bufferSize], outBuf[bufferSize + EVP_MAX_BLOCK_LENGTH];
  int bytesRead = 0, outLen = 0;

  while ((bytesRead = inFile.read((char *)inBuf, bufferSize)) > 0) {
    if (EVP_EncryptUpdate(ctx, outBuf, &outLen, inBuf, bytesRead) != 1) {
      EVP_CIPHER_CTX_free(ctx);
      return false;
    }
    outFile.write((char *)outBuf, outLen);
  }

  // 处理最后的数据块
  if (EVP_EncryptFinal_ex(ctx, outBuf, &outLen) != 1) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  outFile.write((char *)outBuf, outLen);

  EVP_CIPHER_CTX_free(ctx);

  inFile.close();
  outFile.flush();
  outFile.close();

  return true;
}

bool Method::decryptFile(const QString &inputPath, const QString &outputPath,
                         const QString &password) {
  QFile inFile(inputPath);
  QFile outFile(outputPath);

  // 1. 检查文件打开状态（添加详细错误日志）
  if (!inFile.open(QIODevice::ReadOnly)) {
    qDebug() << "无法打开输入文件：" << inFile.errorString();
    return false;
  }

  if (!outFile.open(QIODevice::WriteOnly)) {
    qDebug() << "无法打开输出文件：" << outFile.errorString();
    inFile.close();
    return false;
  }

  // 2. 读取盐和 IV（检查实际读取长度）
  QByteArray salt = inFile.read(16);
  QByteArray iv = inFile.read(16);
  if (salt.size() != 16 || iv.size() != 16) {
    qDebug() << "盐或 IV 长度错误（salt:" << salt.size() << "iv:" << iv.size()
             << ")";
    inFile.close();
    outFile.close();
    return false;
  }

  // 3. 密钥派生（添加错误检查）
  QByteArray key = deriveKey(password, salt, 32);
  if (key.isEmpty()) {
    qDebug() << "密钥派生失败";
    inFile.close();
    outFile.close();
    return false;
  }

  // 4. 初始化 OpenSSL 上下文（启用填充）
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (!ctx) {
    qDebug() << "无法创建 EVP 上下文";
    inFile.close();
    outFile.close();
    return false;
  }
  if (EVP_DecryptInit_ex(
          ctx, EVP_aes_256_cbc(), nullptr,
          reinterpret_cast<const unsigned char *>(key.constData()),
          reinterpret_cast<const unsigned char *>(iv.constData())) != 1) {
    qDebug() << "解密初始化失败："
             << ERR_error_string(ERR_get_error(), nullptr);
    EVP_CIPHER_CTX_free(ctx);
    inFile.close();
    outFile.close();
    return false;
  }
  EVP_CIPHER_CTX_set_padding(ctx, 1);  // 显式启用 PKCS#7 填充

  // 5. 动态分配缓冲区（避免栈溢出）
  const int bufferSize = 4096 + EVP_MAX_BLOCK_LENGTH;
  unsigned char *inBuf = new unsigned char[bufferSize];
  unsigned char *outBuf = new unsigned char[bufferSize];
  int bytesRead = 0, outLen = 0;
  bool success = true;

  // 6. 分块解密（修复读取循环）
  while ((bytesRead = inFile.read((char *)inBuf, bufferSize)) > 0) {
    if (EVP_DecryptUpdate(ctx, outBuf, &outLen, inBuf, bytesRead) != 1) {
      qDebug() << "解密分块失败："
               << ERR_error_string(ERR_get_error(), nullptr);
      success = false;
      break;
    }
    if (outFile.write((char *)outBuf, outLen) != outLen) {
      qDebug() << "写入输出文件失败：" << outFile.errorString();
      success = false;
      break;
    }
  }

  // 7. 处理最终块（检查是否还有数据）
  if (success) {
    if (EVP_DecryptFinal_ex(ctx, outBuf, &outLen) != 1) {
      qDebug() << "最终块处理失败："
               << ERR_error_string(ERR_get_error(), nullptr);
      success = false;
      isPasswordError = true;
    } else {
      if (outFile.write((char *)outBuf, outLen) != outLen) {
        qDebug() << "写入最终块失败：" << outFile.errorString();
        success = false;
      }
    }
  }

  // 8. 清理资源
  delete[] inBuf;
  delete[] outBuf;
  EVP_CIPHER_CTX_free(ctx);
  inFile.close();
  outFile.flush();
  outFile.close();

  // 9. 验证输出文件完整性
  if (success) {
    qint64 expectedSize = inFile.size() - 32;  // 加密文件去头（salt + iv）
    qint64 actualSize = outFile.size();
    if (actualSize != expectedSize) {
      qDebug() << "文件大小不匹配，预期：" << expectedSize << "实际："
               << actualSize;
      // success = false;
    }
  }

  return success;
}

QString Method::useDec(QString enc_file) {
  if (isEncrypt) {
    QString dec_file = enc_file + ".dec";
    if (decryptFile(enc_file, dec_file, encPassword)) {
      QFile::remove(enc_file);
      QFile::rename(dec_file, enc_file);
      return enc_file;
    } else {
      QFile::remove(dec_file);
      return "";
    }
  }
  return "";
}

QString Method::useEnc(QString m_file) {
  if (isEncrypt) {
    QString enc_file = m_file + ".enc";
    if (encryptFile(m_file, enc_file, encPassword)) {
      QFile::remove(m_file);
      QFile::rename(enc_file, m_file);
      return m_file;
    } else {
      QFile::remove(enc_file);
      return "";
    }
  }

  return "";
}

/////////////////////////////////////////////////////////////////////////////////////

bool Method::compressDirectoryNG(const QString &zipPath,
                                 const QString &sourceDir,
                                 const QString &password) {
  return true;
}

//////////////////////////////////////////////////////////////////////////////////

bool Method::compressFileNG(const QString &zipPath, const QString &filePath,
                            const QString &password) {
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////

bool Method::decompressWithPasswordNG(const QString &zipPath,
                                      const QString &extractDir,
                                      const QString &password) {
  return 0;
}

bool Method::compressFileWithZlib(const QString &sourcePath,
                                  const QString &destPath, int level) {
  // compressFile(..., Z_BEST_SPEED);    // 最快速度
  // compressFile(..., Z_DEFAULT_COMPRESSION); // 平衡模式
  // compressFile(..., Z_BEST_COMPRESSION);    // 最高压缩率

  QFile srcFile(sourcePath);
  if (!srcFile.open(QIODevice::ReadOnly)) {
    qWarning() << "无法打开源文件:" << sourcePath;
    return false;
  }

  QFile destFile(destPath);
  if (!destFile.open(QIODevice::WriteOnly)) {
    qWarning() << "无法创建目标文件:" << destPath;
    srcFile.close();
    return false;
  }

  z_stream zs;
  memset(&zs, 0, sizeof(zs));

  // 初始化压缩流
  if (deflateInit(&zs, level) != Z_OK) {
    qWarning() << "zlib压缩初始化失败";
    srcFile.close();
    destFile.close();
    return false;
  }

  constexpr int CHUNK_SIZE = 128 * 1024;  // 128KB分块
  char inBuffer[CHUNK_SIZE];
  char outBuffer[CHUNK_SIZE];

  bool success = true;
  qint64 totalRead = 0;

  do {
    // 读取源文件块
    qint64 bytesRead = srcFile.read(inBuffer, CHUNK_SIZE);
    if (bytesRead < 0) {
      qWarning() << "文件读取错误";
      success = false;
      break;
    }

    zs.avail_in = bytesRead;
    zs.next_in = reinterpret_cast<Bytef *>(inBuffer);

    // 压缩并写入目标文件
    do {
      zs.avail_out = CHUNK_SIZE;
      zs.next_out = reinterpret_cast<Bytef *>(outBuffer);

      int ret = deflate(&zs, (srcFile.atEnd() ? Z_FINISH : Z_NO_FLUSH));
      if (ret == Z_STREAM_ERROR) {
        qWarning() << "压缩过程中发生错误";
        success = false;
        break;
      }

      qint64 compressedSize = CHUNK_SIZE - zs.avail_out;
      if (destFile.write(outBuffer, compressedSize) != compressedSize) {
        qWarning() << "文件写入错误";
        success = false;
        break;
      }

    } while (zs.avail_out == 0);

    totalRead += bytesRead;
  } while (!srcFile.atEnd() && success);

  deflateEnd(&zs);
  srcFile.close();
  destFile.close();

  if (success) {
    qDebug() << "压缩完成，原始大小:" << totalRead
             << "压缩后大小:" << QFileInfo(destPath).size();
  }
  return success;
}

bool Method::decompressFileWithZlib(const QString &sourcePath,
                                    const QString &destPath) {
  QFile srcFile(sourcePath);
  if (!srcFile.open(QIODevice::ReadOnly)) {
    qWarning() << "无法打开压缩文件:" << sourcePath;
    return false;
  }

  QFile destFile(destPath);
  if (!destFile.open(QIODevice::WriteOnly)) {
    qWarning() << "无法创建解压文件:" << destPath;
    srcFile.close();
    return false;
  }

  z_stream zs;
  memset(&zs, 0, sizeof(zs));

  if (inflateInit(&zs) != Z_OK) {
    qWarning() << "zlib解压初始化失败";
    srcFile.close();
    destFile.close();
    return false;
  }

  constexpr int CHUNK_SIZE = 128 * 1024;
  char inBuffer[CHUNK_SIZE];
  char outBuffer[CHUNK_SIZE];

  bool success = true;
  qint64 totalWritten = 0;
  qint64 bytesRead = 0;

  do {
    // 读取压缩数据块
    bytesRead = srcFile.read(inBuffer, CHUNK_SIZE);
    if (bytesRead < 0) {
      qWarning() << "文件读取错误";
      success = false;
      break;
    }

    zs.avail_in = bytesRead;
    zs.next_in = reinterpret_cast<Bytef *>(inBuffer);

    // 解压并写入目标文件
    do {
      zs.avail_out = CHUNK_SIZE;
      zs.next_out = reinterpret_cast<Bytef *>(outBuffer);

      int ret = inflate(&zs, Z_NO_FLUSH);
      if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
        qWarning() << "解压过程中发生错误:" << ret;
        success = false;
        break;
      }

      qint64 decompressedSize = CHUNK_SIZE - zs.avail_out;
      if (destFile.write(outBuffer, decompressedSize) != decompressedSize) {
        qWarning() << "文件写入错误";
        success = false;
        break;
      }

      totalWritten += decompressedSize;
    } while (zs.avail_out == 0 && success);

  } while (bytesRead > 0 && success);

  inflateEnd(&zs);
  srcFile.close();
  destFile.close();

  if (success) {
    qDebug() << "解压完成，解压后大小:" << totalWritten;
  }
  return success;
}

QString Method::getFileSize(const qint64 &size, int precision) {
  double sizeAsDouble = size;
  static QStringList measures;
  if (measures.isEmpty())
    measures << QCoreApplication::translate("QInstaller", "bytes")
             << QCoreApplication::translate("QInstaller", "KiB")
             << QCoreApplication::translate("QInstaller", "MiB")
             << QCoreApplication::translate("QInstaller", "GiB")
             << QCoreApplication::translate("QInstaller", "TiB")
             << QCoreApplication::translate("QInstaller", "PiB")
             << QCoreApplication::translate("QInstaller", "EiB")
             << QCoreApplication::translate("QInstaller", "ZiB")
             << QCoreApplication::translate("QInstaller", "YiB");
  QStringListIterator it(measures);
  QString measure(it.next());
  while (sizeAsDouble >= 1024.0 && it.hasNext()) {
    measure = it.next();
    sizeAsDouble /= 1024.0;
  }
  return QString::fromLatin1("%1 %2")
      .arg(sizeAsDouble, 0, 'f', precision)
      .arg(measure);
}

bool Method::androidCopyFile(QString src, QString des) {
  bool result = false;

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject srcObj = QAndroidJniObject::fromString(src);
  QAndroidJniObject desObj = QAndroidJniObject::fromString(des);
  QAndroidJniObject m_activity = QAndroidJniObject::fromString("copyFile");
  result = m_activity.callStaticMethod<int>(
      "com.x/MyActivity", "copyFile", "(Ljava/lang/String;Ljava/lang/String;)I",
      srcObj.object<jstring>(), desObj.object<jstring>());
#else
  QJniObject srcObj = QJniObject::fromString(src);
  QJniObject desObj = QJniObject::fromString(des);
  QJniObject m_activity = QJniObject::fromString("copyFile");
  result = m_activity.callStaticMethod<int>(
      "com.x/MyActivity", "copyFile", "(Ljava/lang/String;Ljava/lang/String;)I",
      srcObj.object<jstring>(), desObj.object<jstring>());
#endif

#endif
  qDebug() << "copyFile " << src << des;
  return result;
}

void Method::setOSFlag() {
  QSettings Reg(iniDir + "osflag.ini", QSettings::IniFormat);
  if (isAndroid)
    Reg.setValue("os", "mobile");
  else
    Reg.setValue("os", "desktop");
}
