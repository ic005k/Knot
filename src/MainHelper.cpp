#include "MainHelper.h"

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"

MainHelper::MainHelper(QWidget* parent) : QDialog{parent} {}

bool MainHelper::mainEventFilter(QObject* watch, QEvent* evn) {
  QMouseEvent* event = static_cast<QMouseEvent*>(evn);  // 将之转换为鼠标事件

  if (evn->type() == QEvent::ToolTip) {
    QToolTip::hideText();
    evn->ignore();
    return true;
  }

  if (watch == mui->lblStats) {
    if (event->type() == QEvent::MouseButtonDblClick) {
      mw_one->on_btnSelTab_pressed();
      return true;
    }
  }

  if (watch == mui->lblTitleEditRecord) {
    if (event->type() == QEvent::MouseButtonPress) {
      QString title = mui->lblTitleEditRecord->text();
      title = title.mid(0, 4);
      if (!title.contains(QObject::tr("Add"))) return true;

      mui->btnTabMoveDown->hide();
      mui->btnTabMoveUp->hide();

      mw_one->m_EditRecord->saveCurrentValue();
      mw_one->on_btnBackEditRecord_pressed();
      mui->btnHome->click();
      isSelectTab = true;

      return true;
    }
  }

  m_Reader->eventFilterReader(watch, evn);

  m_Notes->eventFilterQwNote(watch, evn);

  if (evn->type() == QEvent::KeyPress) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);

    if (watch == mui->editSearchText && keyEvent->key() == Qt::Key_Return) {
      mw_one->on_btnStartSearch_clicked();
      return true;
    }

    if (keyEvent->key() == Qt::Key_Escape) {
      if (mui->frameReader->isVisible()) mw_one->on_btnBackReader_pressed();
      return true;
    }
  }

  return true;
}

void MainHelper::clickBtnChart() {
  if (mui->qwMainChart->isHidden()) {
    mui->qwMainDate->hide();
    mui->qwMainEvent->hide();
    mui->qwMainChart->show();

  } else {
    mui->qwMainChart->hide();
    mui->qwMainDate->show();
    mui->qwMainEvent->show();
  }
}

void MainHelper::clickBtnRestoreTab() {
  if (m_Method->getCountFromQW(mui->qwTabRecycle) == 0) return;

  int count = mui->tabWidget->tabBar()->count();
  QString twName = m_Notes->getDateTimeStr() + "_" + QString::number(count + 1);

  int c_year = QDate::currentDate().year();
  int iniFileCount = c_year - 2025 + 1 + 1;

  int index = m_Method->getCurrentIndexFromQW(mui->qwTabRecycle);
  QString recycle = m_Method->getText3(mui->qwTabRecycle, index);
  QStringList recycleList = recycle.split("\n");

  if (recycleList.at(0).contains(".json")) {
    for (int i = 0; i < recycleList.count(); i++) {
      QString file, newFile;
      file = recycleList.at(i);
      QFileInfo fi(file);
      QString fn = fi.fileName();
      QStringList list = fn.split("_");
      if (list.count() > 0) {
        QString a1 = list.at(0) + "_";
        QString a2 = list.at(1) + "_";
        fn = fn.replace(a1, "");
        fn = fn.replace(a2, "");

        newFile = iniDir + fn;
        QFile::rename(file, newFile);

        // Get tw name
        QStringList list1 = fn.split("-");
        if (fn.endsWith(".json") && list1.count() >= 2) {
          QString a3 = list1.at(0) + "-";
          QString tn = fn;
          tn = tn.replace(a3, "");
          tn = tn.replace(".json", "");
          twName = tn;
        }
      }
    }

  } else {  // ini files
    QString ini_file;
    for (int i = 0; i < iniFileCount; i++) {
      if (i == 0)
        ini_file = iniDir + twName + ".ini";
      else {
        ini_file =
            iniDir + QString::number(2025 + i - 1) + "-" + twName + ".ini";
      }

      if (QFile(ini_file).exists()) QFile(ini_file).remove();
      QString recFile;
      if (recycleList.count() > 1)
        recFile = recycleList.at(i);
      else
        recFile = recycle;
      QFile::copy(recFile, ini_file);
    }
  }

  QString tab_name = m_Method->getText0(mui->qwTabRecycle, index);
  QTreeWidget* tw = mw_one->init_TreeWidget(twName);
  mui->tabWidget->addTab(tw, tab_name);

  mw_one->addItem(tab_name, "", "", "", 0);
  mw_one->setCurrentIndex(count);

  mw_one->readData(tw);

  if (recycleList.count() > 1) {
    for (int i = 0; i < recycleList.count(); i++) {
      QFile recycle_file(recycle.split("\n").at(i));
      recycle_file.remove();
    }
  } else {
    QFile recycle_file(recycle);
    recycle_file.remove();
  }

  mw_one->on_btnBackTabRecycle_pressed();

  mw_one->saveTab();

  mw_one->reloadMain();
  mw_one->clickData();

  tabData->setCurrentIndex(count);

  mw_one->strLatestModify = tr("Restore Tab") + "(" + tab_name + ")";
}

void MainHelper::openTabRecycle() {
  mw_one->showProgress();

  if (mui->qwTabRecycle->source().isEmpty()) {
    mui->qwTabRecycle->rootContext()->setContextProperty("m_Report",
                                                         mw_one->m_Report);
    mui->qwTabRecycle->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/tabrecycle.qml")));
  }

  // 切换UI
  mui->frameMain->hide();
  mui->frameTabRecycle->show();
  m_Method->clearAllBakList(mui->qwTabRecycle);

  // 使用QFutureWatcher监控后台任务完成
  QFutureWatcher<QStringList>* watcher = new QFutureWatcher<QStringList>(this);

  // 后台执行文件处理
  QFuture<QStringList> future = QtConcurrent::run([=]() -> QStringList {
    QString tab_name, tab_time;
    QStringList iniFiles;
    QStringList fmt;
    fmt << "ini" << "json";

    // 获取所有文件（耗时操作，在后台执行）
    m_NotesList->getAllFiles(iniDir, iniFiles, fmt);

    QString iniTotal;
    QStringList myList, nameList, iniList;

    // 处理文件列表
    for (int i = 0; i < iniFiles.count(); i++) {
      QString ini_file = iniFiles.at(i);
      if (ini_file.contains("recycle_name_") && ini_file.contains(".ini")) {
        QFileInfo fi(ini_file);
        QString ini_filename = fi.fileName();
        ini_filename = ini_filename.replace(".ini", "");
        tab_name = ini_filename.split("_").at(1);
        QString t1, t2;
        t1 = ini_filename.split("_").at(2);
        t2 = ini_filename.split("_").at(3);
        QStringList list = t2.split("-");
        if (list.count() == 2) {
          t2 = list.at(0);
        }
        tab_time = t1 + "  " + t2;

        tab_name = m_Method->getRecycleTabName(t1 + "_" + t2);

        myList.append(tab_name + "-=-" + tab_time + "-=-" + ini_file);
        nameList.append(tab_name + "-=-" + tab_time);
        iniList.append(ini_file);
      }

      if (ini_file.contains("recycle_") && ini_file.contains(".json")) {
        QStringList list = ini_file.split("_");
        if (list.count() > 0) {
          tab_name = list.at(1);
        }
        tab_time = "----------";
        myList.append(tab_name + "-=-" + tab_time + "-=-" + ini_file);
        nameList.append(tab_name + "-=-" + tab_time);
        iniList.append(ini_file);
      }
    }

    int count = myList.count();
    QStringList lastList;
    for (int i = 0; i < count; i++) {
      QString str1 = myList.at(i);
      iniTotal = "";
      QStringList list1 = str1.split("-=-");
      tab_name = list1.at(0);
      tab_time = list1.at(1);
      for (int j = 0; j < count; j++) {
        QString str2 = nameList.at(j);
        if (str1.contains(str2)) {
          iniTotal += iniList.at(j) + "\n";
        }
      }

      lastList.append(tab_name + "-=-" + tab_time + "-=-" + iniTotal);
    }

    // 去重处理
    QSet<QString> set(lastList.begin(), lastList.end());
    return QStringList(set.begin(), set.end());
  });

  // 将future与watcher关联
  watcher->setFuture(future);

  // 当后台任务完成时，在主线程更新UI
  connect(watcher, &QFutureWatcher<QStringList>::finished, this, [=]() {
    QStringList uniqueList = watcher->result();

    // 在主线程更新UI
    for (int i = 0; i < uniqueList.count(); i++) {
      QString str = uniqueList.at(i);
      QString tab_name = str.split("-=-").at(0);
      QString tab_time = str.split("-=-").at(1);
      QString iniTotal = str.split("-=-").at(2);
      iniTotal = iniTotal.trimmed();
      m_Method->addItemToQW(mui->qwTabRecycle, tab_name, tab_time, "", iniTotal,
                            0);
    }

    int t_count = m_Method->getCountFromQW(mui->qwTabRecycle);
    if (t_count > 0) {
      m_Method->setCurrentIndexFromQW(mui->qwTabRecycle, 0);
    }

    mui->lblTitleTabRecycle->setText(tr("Tab Recycle") + "    " + tr("Total") +
                                     " : " + QString::number(t_count));

    // 清理watcher
    watcher->deleteLater();

    mw_one->closeProgress();
  });
}

void MainHelper::startBackgroundTaskUpdateBakFileList() {
  mw_one->showProgress();

  // ==============================================
  // 子线程只做耗时操作，返回处理好的列表
  // ==============================================
  QFuture<QStringList> future = QtConcurrent::run([=]() -> QStringList {
    // 1. 获取原始列表
    QStringList list = mw_one->m_Preferences->getBakFilesList();

    // 2. 裁剪逻辑（只在子线程处理数据）
    int bakCount = list.count();
    if (bakCount > 15) {
      int count_a = bakCount - 15;
      for (int j = 0; j < count_a; j++) {
        QString str = list.at(0);
        QString fn = str.split("-===-").at(1);
        QFile file(fn);
        file.remove();
        list.removeAt(0);
      }
    }

    // 【关键 1】把处理完的列表 返回给主线程
    return list;
  });

  // ==============================================
  // 【关键 2】主线程接收结果，只在主线程操作 UI/QML
  // ==============================================
  QFutureWatcher<QStringList>* watcher =
      new QFutureWatcher<QStringList>(mw_one);
  connect(watcher, &QFutureWatcher<QStringList>::finished, mw_one, [=]() {
    // 获取子线程返回的安全数据
    QStringList finalList = watcher->result();

    // ======================
    // 所有 UI/QML 操作，全部放在主线程
    // ======================
    m_Method->clearAllBakList(mui->qwBakList);

    int bakCount = finalList.count();
    for (int i = 0; i < bakCount; i++) {
      QString action, bakfile;
      QString str = finalList.at(bakCount - 1 - i);
      action = str.split("-===-").at(0);
      bakfile = str.split("-===-").at(1);
      m_Method->addItemToQW(mui->qwBakList, action, "", "", bakfile, 0);
    }

    if (m_Method->getCountFromQW(mui->qwBakList) > 0) {
      m_Method->setCurrentIndexFromQW(mui->qwBakList, 0);
    }

    mui->lblBakListTitle->setText(
        tr("Backup File List") + "    " + tr("Total") + " : " +
        QString::number(m_Method->getCountFromQW(mui->qwBakList)));

    qDebug() << "BakFileList update completed";

    // 收尾
    mw_one->closeProgress();
    watcher->deleteLater();
  });

  watcher->setFuture(future);
}

void MainHelper::delBakFile() {
  if (m_Method->getCountFromQW(mui->qwBakList) == 0) return;

  int index = m_Method->getCurrentIndexFromQW(mui->qwBakList);
  QString bak_file = m_Method->getText3(mui->qwBakList, index);

  auto m_ShowMsg = std::make_unique<ShowMessage>(this);
  if (!m_ShowMsg->showMsg("Knot",
                          tr("Whether to remove") + "  " + bak_file + " ? ", 2))
    return;

  QFile file(bak_file);
  file.remove();
  m_Method->delItemFromQW(mui->qwBakList, index);

  int newIndex = index - 1;
  if (newIndex < 0) newIndex = 0;

  m_Method->setCurrentIndexFromQW(mui->qwBakList, newIndex);

  mui->lblBakListTitle->setText(
      tr("Backup File List") + "    " + tr("Total") + " : " +
      QString::number(m_Method->getCountFromQW(mui->qwBakList)));
}

void MainHelper::delTabRecycleFile() {
  if (m_Method->getCountFromQW(mui->qwTabRecycle) == 0) return;
  int index = m_Method->getCurrentIndexFromQW(mui->qwTabRecycle);
  QString tab_file = m_Method->getText3(mui->qwTabRecycle, index);

  auto m_ShowMsg = std::make_unique<ShowMessage>(this);
  if (!m_ShowMsg->showMsg("Knot",
                          tr("Whether to remove") + "  " + tab_file + " ? ", 2))
    return;

  QStringList list = tab_file.split("\n");
  QString rec_file;
  if (list.count() > 1) {
    for (int i = 0; i < list.count(); i++) {
      rec_file = list.at(i);
      QFile file(rec_file);
      file.remove();
    }
  } else {
    rec_file = tab_file;
    QFile file(rec_file);
    file.remove();
  }

  m_Method->delItemFromQW(mui->qwTabRecycle, index);

  mui->lblTitleTabRecycle->setText(
      tr("Tab Recycle") + "    " + tr("Total") + " : " +
      QString::number(m_Method->getCountFromQW(mui->qwTabRecycle)));
}

void MainHelper::importBakFileList() {
  if (m_Method->getCountFromQW(mui->qwBakList) == 0) return;

  int cur_index = m_Method->getCurrentIndexFromQW(mui->qwBakList);
  QString str = m_Method->getText3(mui->qwBakList, cur_index);
  zipfile = str.trimmed();

  if (!zipfile.isNull()) {
    auto m_ShowMsg = std::make_unique<ShowMessage>(this);
    if (!m_ShowMsg->showMsg(
            "Kont",
            tr("Import this data?") + "\n" + m_Reader->getUriRealPath(zipfile),
            2)) {
      isZipOK = false;
      return;
    }
  }

  isZipOK = true;
  mui->btnBackBakList->click();
  mw_one->showProgress();

  isMenuImport = true;
  isDownData = false;

  mw_one->myImportDataThread->start();
}

void MainHelper::sort_childItem(QTreeWidgetItem* item) {
  QStringList keys, list, keyTime, keysNew;
  int childCount = item->parent()->childCount();

  for (int i = 0; i < childCount; i++) {
    QString txt0 = item->parent()->child(i)->text(0);
    QStringList list0 = txt0.split(".");
    if (list0.count() == 2) {
      txt0 = list0.at(1);
      txt0 = txt0.trimmed();
    }
    QString txt1 = item->parent()->child(i)->text(1);
    QString txt2 = item->parent()->child(i)->text(2);
    QString txt3 = item->parent()->child(i)->text(3);
    keys.append(txt0 + "|===|" + txt1 + "|===|" + txt2 + "|===|" + txt3);
    keyTime.append(txt0);
  }

  std::sort(keyTime.begin(), keyTime.end(),
            [](const QString& s1, const QString& s2) { return s1 < s2; });

  for (int i = 0; i < keyTime.count(); i++) {
    QString time = keyTime.at(i);
    for (int n = 0; n < keys.count(); n++) {
      QString str1 = keys.at(n);
      QStringList l0 = str1.split("|===|");
      if (time == l0.at(0)) {
        keysNew.append(str1);
        break;
      }
    }
  }

  for (int i = 0; i < childCount; i++) {
    QTreeWidgetItem* childItem = item->parent()->child(i);
    QString str = keysNew.at(i);
    list.clear();
    list = str.split("|===|");
    if (list.count() == 4) {
      int number = i + 1;
      QString strChildCount = QString::number(childCount);
      QString strNum;
      strNum = QString("%1").arg(number, strChildCount.length(), 10,
                                 QLatin1Char('0'));
      childItem->setText(0, strNum + ". " + list.at(0).trimmed());
      childItem->setText(1, list.at(1).trimmed());
      childItem->setText(2, list.at(2).trimmed());
      childItem->setText(3, list.at(3).trimmed());
    }
  }
}

void MainHelper::on_AddRecord() {
  isAdd = true;

  mui->lblTitleEditRecord->setText(tr("Add") + "  : " +
                                   tabData->tabText(tabData->currentIndex()));

  mui->hsH->setValue(QTime::currentTime().hour());
  mui->hsM->setValue(QTime::currentTime().minute());
  mw_one->m_EditRecord->getTime(mui->hsH->value(), mui->hsM->value());

  mui->editDetails->clear();
  mw_one->m_EditRecord->isNoShowSuggestions = true;
  mui->editCategory->setText("");
  mw_one->m_EditRecord->isNoShowSuggestions = false;
  mui->editAmount->setText("");

  mui->frameMain->hide();
  mui->frameEditRecord->show();

  // tmeFlash->start(300);
}

////////////////////////////////////////////////////////////////////////////////

bool MainWindow::importBakData(QString fileName) {
  if (fileName.isNull()) return false;

  deleteDirfile(privateDir + "gps");
  m_Reader->copyDirectoryFiles(iniDir + "memo/gps", privateDir + "gps", true);

  QString zipPath = bakfileDir + "memo.zip";
  if (fileName != zipPath) {
    QFile::remove(zipPath);
    QFile::copy(fileName, zipPath);
  }

  QString dec_file;
  if (isEncrypt) {
    dec_file = zipPath + ".dec";
    bool result = false;
    result = m_Method->decryptFile(zipPath, dec_file, encPassword);

    while (result == false && isPasswordError == false) {
      QEventLoop loop;
      QTimer::singleShot(100, &loop, &QEventLoop::quit);  // 非阻塞延时
      loop.exec();
    }
  } else
    dec_file = zipPath;

  deleteDirfile(bakfileDir + "KnotData");
  bool unzipResult = false;
  unzipResult =
      m_Method->decompressWithPassword(dec_file, bakfileDir, encPassword);

  while (unzipResult == false && isPasswordError == false) {
    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);  // 非阻塞延时
    loop.exec();
  }

  if (isPasswordError) {
    return false;
  }

  isZipOK = true;

  QString bakFileFrom;
  QSettings Reg(bakfileDir + "KnotData/osflag.ini", QSettings::IniFormat);
  bakFileFrom = Reg.value("os", "mobile").toString();
  qDebug() << "bakFileFrom=" << bakFileFrom;

  if (bakFileFrom == "desktop") {
    deleteDirfile(iniDir + "memo");
    QFile::remove(iniDir + "todo.json");
    QFile::remove(iniDir + "mainnotes.json");

    m_Reader->copyDirectoryFiles(bakfileDir + "KnotData/memo", iniDir + "memo",
                                 true);
    QFile::copy(bakfileDir + "KnotData/todo.json", iniDir + "todo.json");
    QFile::copy(bakfileDir + "KnotData/mainnotes.json",
                iniDir + "mainnotes.json");

    deleteDirfile(iniDir + "memo/gps");
    m_Reader->copyDirectoryFiles(bakfileDir + "KnotData/memo/gps",
                                 iniDir + "memo/gps", true);
  }

  if (bakFileFrom == "mobile") {
    deleteDirfile(iniDir);
    m_Reader->copyDirectoryFiles(bakfileDir + "KnotData", iniDir, true);
  }

  return true;
}

void MainWindow::reloadMain() {
  bool isAniEffects;
  if (isDelItem || isEditItem)
    isAniEffects = false;
  else
    isAniEffects = true;

  mui->qwMainDate->rootContext()->setContextProperty("isAniEffects",
                                                     isAniEffects);
  mui->qwMainDate->rootContext()->setContextProperty("maindateWidth",
                                                     mui->qwMainDate->width());
  m_Method->clearAllBakList(mui->qwMainDate);

  QTreeWidget* tw = get_tw(tabData->currentIndex());

  int total = tw->topLevelItemCount();

  if (total == 0) {
    m_Method->clearAllBakList(mui->qwMainEvent);
    return;
  }

  int a;

  if (total - days > 0)
    a = total - days;
  else
    a = 0;

  QString text0, text1, text2, text3, topitem;
  for (int i = a; i < total; i++) {
    QTreeWidgetItem* topItem = tw->topLevelItem(i);

    text0 = topItem->text(0) + "  " + topItem->text(3);
    text1 = topItem->text(1);
    text2 = topItem->text(2);

    topitem = text0;

    m_Method->insertItem(mui->qwMainDate, text0, text1, text2, text3, 0);
  }

  m_Method->setCurrentIndexFromQW(mui->qwMainDate, 0);

  /*m_Method->gotoEnd(mui->qwMainDate);
  int count = m_Method->getCountFromQW(mui->qwMainDate);
  m_Method->setCurrentIndexFromQW(mui->qwMainDate, count - 1);
  m_Method->setScrollBarPos(mui->qwMainDate, 1.0);*/

  m_Method->clickMainDate();
}

QStringList MainWindow::get_MonthList(QString strY, QString strM) {
  QStringList listMonth;
  if (loading) return listMonth;
  // 格式：记录第一个子项的时间
  freqPointList.clear();
  amountList.clear();
  doubleList.clear();

  QTreeWidget* tw = (QTreeWidget*)tabData->currentWidget();
  for (int i = 0; i < tw->topLevelItemCount(); i++) {
    if (isBreak) break;
    QTreeWidgetItem* topItem = tw->topLevelItem(i);
    QString str0 = topItem->text(0) + " " + topItem->text(3);
    QString y, m, d;
    y = get_Year(str0);
    m = get_Month(str0);
    d = QString::number(get_Day(str0));
    double x0 = d.toDouble();
    if (y == strY) {
      if (m == strM) {
        if (isrbFreq) {
          if (topItem->childCount() > 0)
            listMonth.append(
                topItem->child(0)->text(0));  // 记录第一个子项的时间

          double y0 = topItem->text(1).toDouble();
          doubleList.append(y0);
          freqPointList.append(QPointF(x0, y0));

          double y1 = topItem->text(2).toDouble();
          amountList.append(QPointF(x0, y1));
        } else {
          double y0 = topItem->text(2).toDouble();
          doubleList.append(y0);

          freqPointList.append(QPointF(x0, y0));
        }
      }
    }
  }

  return listMonth;
}

void MainWindow::readData(QTreeWidget* tw) {
  tw->clear();
  QString name = tw->objectName();

  int cu_year = QDate::currentDate().year();
  bool isFileExists = false;
  for (int i = 2022; i <= cu_year; i++) {
    QString file = iniDir + QString::number(i) + "-" + name + ".json";
    if (QFile::exists(file)) {
      isFileExists = true;
    }
  }

  if (isFileExists) {
    DataManager* dataMgr = new DataManager(iniDir, nullptr);
    dataMgr->loadData(tw);
    delete dataMgr;
    dataMgr = nullptr;

    return;
  }

  /////////////////////////////////////////////////////

  QStringList myTopStrList;

  int iniFileCount = QDate::currentDate().year() - 2025 + 1 + 1;

  QString ini_file;
  for (int p = 0; p < iniFileCount; p++) {
    if (p == 0) {
      ini_file = iniDir + name + ".ini";
    } else {
      QString strYear = QString::number(2025 + p - 1);
      ini_file = iniDir + strYear + "-" + name + ".ini";
    }

    qDebug() << "read ini_file=" << ini_file;

    if (QFile::exists(ini_file)) {
      QSettings Reg(ini_file, QSettings::IniFormat);

      QString group = Reg.childGroups().at(0);

      int rowCount = Reg.value("/" + group + "/TopCount").toInt();
      for (int i = 0; i < rowCount; i++) {
        int childCount = Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childCount")
                             .toInt();

        // 不显示子项为0的数据
        if (childCount > 0) {
          QTreeWidgetItem* topItem = new QTreeWidgetItem;
          QString strD0 =
              Reg.value("/" + group + "/" + QString::number(i + 1) + "-topDate")
                  .toString();

          QStringList lista = strD0.split(" ");
          if (lista.count() == 4) {
            QString a0 = lista.at(0) + " " + lista.at(1) + " " + lista.at(2);
            topItem->setText(0, a0);
            topItem->setText(3, lista.at(3));
          } else {
            topItem->setText(0, strD0);
            QString year = Reg.value("/" + group + "/" +
                                     QString::number(i + 1) + "-topYear")
                               .toString();
            topItem->setText(3, year);
          }

          topItem->setTextAlignment(1, Qt::AlignHCenter | Qt::AlignVCenter);
          topItem->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);

          topItem->setText(1, Reg.value("/" + group + "/" +
                                        QString::number(i + 1) + "-topFreq")
                                  .toString());
          topItem->setText(2, Reg.value("/" + group + "/" +
                                        QString::number(i + 1) + "-topAmount")
                                  .toString());

          // 移除异项（时间相同，但频次处于累加的异常情况）
          int lastTopIndex = tw->topLevelItemCount() - 1;
          if (lastTopIndex >= 0) {
            QTreeWidgetItem* lastTopItem = tw->topLevelItem(lastTopIndex);
            // 时间相同但频次或金额不同
            if ((lastTopItem->text(0) == topItem->text(0)) &&
                (lastTopItem->text(2) != topItem->text(2) ||
                 lastTopItem->text(1) != topItem->text(1))) {
              tw->takeTopLevelItem(lastTopIndex);
            }
          }

          QString topStr = QString("%1|%2|%3|%4")
                               .arg(topItem->text(0), topItem->text(1),
                                    topItem->text(2), topItem->text(3));

          if (myTopStrList.contains(topStr)) {
            for (int x = 0; x < tw->topLevelItemCount(); x++) {
              QString top, top0, top1, top2, top3;
              top0 = tw->topLevelItem(x)->text(0);
              top1 = tw->topLevelItem(x)->text(1);
              top2 = tw->topLevelItem(x)->text(2);
              top3 = tw->topLevelItem(x)->text(3);
              top = QString("%1|%2|%3|%4").arg(top0, top1, top2, top3);
              if (top == topStr) {
                tw->takeTopLevelItem(x);
                tw->insertTopLevelItem(x, topItem);
                for (int j = 0; j < childCount; j++) {
                  QTreeWidgetItem* item11 = new QTreeWidgetItem(topItem);
                  item11->setText(
                      0, Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childTime" + QString::number(j))
                             .toString());
                  item11->setText(
                      1, Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childAmount" + QString::number(j))
                             .toString());
                  item11->setText(
                      2, Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childDesc" + QString::number(j))
                             .toString());
                  item11->setText(
                      3, Reg.value("/" + group + "/" + QString::number(i + 1) +
                                   "-childDetails" + QString::number(j))
                             .toString());

                  item11->setTextAlignment(1,
                                           Qt::AlignRight | Qt::AlignVCenter);
                }
                break;
              }
            }
          }

          if (!myTopStrList.contains(topStr)) {
            tw->addTopLevelItem(topItem);
            myTopStrList.append(topStr);

            for (int j = 0; j < childCount; j++) {
              QTreeWidgetItem* item11 = new QTreeWidgetItem(topItem);
              item11->setText(
                  0, Reg.value("/" + group + "/" + QString::number(i + 1) +
                               "-childTime" + QString::number(j))
                         .toString());
              item11->setText(
                  1, Reg.value("/" + group + "/" + QString::number(i + 1) +
                               "-childAmount" + QString::number(j))
                         .toString());
              item11->setText(
                  2, Reg.value("/" + group + "/" + QString::number(i + 1) +
                               "-childDesc" + QString::number(j))
                         .toString());
              item11->setText(
                  3, Reg.value("/" + group + "/" + QString::number(i + 1) +
                               "-childDetails" + QString::number(j))
                         .toString());

              item11->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
            }
          }
        }
      }
    }
  }
}

void MainWindow::drawMonthChart() {
  listM.clear();
  listM = get_MonthList(strY, strM);
  CurrentYearMonth = strY + strM;
}

void MainWindow::drawDayChart() {
  QTreeWidget* tw = (QTreeWidget*)tabData->currentWidget();
  if (loading) return;
  freqPointList.clear();

  int topCount = tw->topLevelItemCount();
  if (topCount == 0) {
    return;
  }

  if (topCount > 0) {
    if (!tw->currentIndex().isValid()) {
      QTreeWidgetItem* topItem = tw->topLevelItem(topCount - 1);
      tw->setCurrentItem(topItem);
    }
  }

  QTreeWidgetItem* item = tw->currentItem();
  bool child;
  int childCount;
  if (item->parent() == NULL)
    child = false;
  else
    child = true;

  QString month;

  if (child) {
    childCount = item->parent()->childCount();
    parentItem = item->parent();
    month = get_Month(item->parent()->text(0));
  } else {
    childCount = item->childCount();
    parentItem = item;
    month = get_Month(item->text(0));
  }

  QList<double> dList;
  double x, y;
  QString str, str1;
  int step = 1;
  if (childCount > 500) step = 100;

  for (int i = 0; i < childCount; i = i + step) {
    if (isBreak) break;

    if (child) {
      str = item->parent()->child(i)->text(0);
      str1 = item->parent()->child(i)->text(1);
    } else {
      str = item->child(i)->text(0);
      str1 = item->child(i)->text(1);
    }
    QStringList l0 = str.split(".");
    if (l0.count() == 2) str = l0.at(1);
    QStringList list = str.split(":");
    int t = 0;
    if (list.count() == 3) {
      QString a, b, c;
      a = list.at(0);
      b = list.at(1);
      c = list.at(2);
      int a1, b1;
      a1 = a.toInt();
      b1 = b.toInt();
      t = a1 * 3600 + b1 * 60 + c.toInt();
    }
    x = (double)t / 3600;
    if (isrbFreq)
      y = i + 1;
    else {
      y = str1.toDouble();
      dList.append(y);
    }

    freqPointList.append(QPointF(x, y));
  }

  if (isrbFreq) {
    int a = chartMax;
    if (childCount > a)
      yMaxDay = childCount;
    else
      yMaxDay = a;
  } else {
    yMaxDay = *std::max_element(dList.begin(), dList.end());
  }
}
