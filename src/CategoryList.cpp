#include "CategoryList.h"

#include "MainWindow.h"
#include "src/defines.h"
#include "ui_CategoryList.h"
#include "ui_MainWindow.h"

CategoryList::CategoryList(QWidget* parent)
    : QDialog(parent), ui(new Ui::CategoryList) {
  ui->setupUi(this);

  setWindowFlag(Qt::FramelessWindowHint);
  // setAttribute(Qt::WA_TranslucentBackground);
  this->layout()->setContentsMargins(5, 5, 5, 5);

  ui->frame->setStyleSheet(
      "#frame{background-color: rgb(255, 255, 255);border-radius:10px; "
      "border:1px solid gray;}");

  setModal(true);
  this->installEventFilter(this);

  ui->listWidget->setStyleSheet("#listWidget{ border:None;}");
  ui->listWidget->verticalScrollBar()->setStyleSheet(m_Method->vsbarStyleSmall);
  ui->listWidget->setVerticalScrollMode(QListWidget::ScrollPerPixel);
  QScroller::grabGesture(ui->listWidget, QScroller::LeftMouseButtonGesture);
  ui->listWidget->horizontalScrollBar()->setHidden(true);
  ui->listWidget->setViewMode(QListView::IconMode);
  ui->listWidget->setMovement(QListView::Static);
  ui->listWidget->setStyleSheet(m_Method->listStyleMain);
  ui->listWidget->setSpacing(12);

  QFont font;
  font.setPointSize(fontSize + 3);
  ui->listWidget->setFont(font);

  ui->btnRename->setFixedHeight(ui->editRename->height() + 2);
}

CategoryList::~CategoryList() { delete ui; }

void CategoryList::keyReleaseEvent(QKeyEvent* event) { Q_UNUSED(event) }

void CategoryList::closeEvent(QCloseEvent* event) { Q_UNUSED(event); }

bool CategoryList::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      close();
      return true;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void CategoryList::on_listWidget_itemClicked(QListWidgetItem* item) {
  Q_UNUSED(item);
  QString txt = item->text();
  ui->editRename->setText(txt);
}

void CategoryList::on_btnDel_clicked() {
  int count = m_Method->getCountFromQW(mui->qwCategory);
  if (count == 0) return;

  int row = m_Method->getCurrentIndexFromQW(mui->qwCategory);

  if (row < 0) return;

  ui->listWidget->setCurrentRow(row);
  if (row >= 0) {
    auto m_ShowMsg = std::make_unique<ShowMessage>(this);
    if (!m_ShowMsg->showMsg("Kont",
                            tr("Delete this category?") + "\n\n" +
                                ui->listWidget->currentItem()->text(),
                            2))
      return;

    ui->listWidget->takeItem(row);

    m_Method->delItemFromQW(mui->qwCategory, row);
  }
  mw_one->m_EditRecord->saveMyClassification();
  if (ui->listWidget->count() > 0)
    on_listWidget_itemClicked(ui->listWidget->currentItem());
  else
    ui->editRename->clear();

  count = m_Method->getCountFromQW(mui->qwCategory);
  mui->lblTypeInfo->setText(tr("Total") + " : " + QString::number(count));
}

void CategoryList::on_btnOk_clicked() {
  int index = m_Method->getCurrentIndexFromQW(mui->qwCategory);
  ui->listWidget->setCurrentRow(index);

  setCategoryText();

  on_btnCancel_clicked();
}

void CategoryList::setCategoryText() {
  int row = ui->listWidget->currentRow();
  if (row >= 0) {
    mui->editCategory->setText(ui->listWidget->currentItem()->text());
  }

  close();
}

void CategoryList::on_listWidget_itemDoubleClicked(QListWidgetItem* item) {
  Q_UNUSED(item);
  setCategoryText();
}

void CategoryList::on_btnRename_clicked() {
  if (ui->listWidget->count() == 0) return;

  int row = m_Method->getCurrentIndexFromQW(mui->qwCategory);
  ui->listWidget->setCurrentRow(row);

  oldName = ui->listWidget->currentItem()->text().trimmed();

  QString text = ui->editRename->text().trimmed();
  QString str = ui->listWidget->currentItem()->text();
  if (!text.isEmpty() && text != str) {
    int index = ui->listWidget->currentRow();
    ui->listWidget->takeItem(index);
    QListWidgetItem* item = new QListWidgetItem(text);

    ui->listWidget->insertItem(index, item);

    m_Method->modifyItemText0(mui->qwCategory, row, text);

    QStringList list;
    for (int i = 0; i < ui->listWidget->count(); i++) {
      list.append(ui->listWidget->item(i)->text().trimmed());
    }
    mw_one->m_EditRecord->removeDuplicates(&list);
    ui->listWidget->clear();
    for (int i = 0; i < list.count(); i++) {
      QListWidgetItem* item = new QListWidgetItem(list.at(i));

      ui->listWidget->addItem(item);
    }
    if (index >= 0) ui->listWidget->setCurrentRow(index);
    mw_one->m_EditRecord->saveMyClassification();

    for (int i = 0; i < tabData->tabBar()->count(); i++) {
      QTreeWidget* tw = (QTreeWidget*)tabData->widget(i);
      for (int m = 0; m < tw->topLevelItemCount(); m++) {
        QTreeWidgetItem* topItem = tw->topLevelItem(m);
        for (int n = 0; n < topItem->childCount(); n++) {
          if (str == topItem->child(n)->text(2)) {
            topItem->child(n)->setText(2, text);
          }
        }
      }
    }

    renameAll();

    mui->editCategory->setText(ui->editRename->text().trimmed());

    mw_one->reloadMain();
  }
}

void CategoryList::renameAll_oldini() {
  QDir dir(iniDir);

  // 设置过滤器：只要.ini文件且排除目录
  QStringList filters;
  filters << "*.ini";
  dir.setNameFilters(filters);
  dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);  // 只获取文件，排除.和..

  // 获取文件信息列表
  QFileInfoList fileList = dir.entryInfoList();

  for (int i = 0; i < fileList.count(); i++) {
    QString file = fileList.at(i).absoluteFilePath();
    QString str = loadText(file);

    QString strNew = str;

    QString newName = ui->editRename->text().trimmed();
    if (strNew.contains("childDesc")) strNew = strNew.replace(oldName, newName);

    if (str != strNew) {
      StringToFile(strNew, file);
      qDebug() << oldName << " --> " << newName;
      qDebug() << "Rename: " + file;
    }
  }
}

void CategoryList::renameAll() {
  // 1. 初始化目录和过滤规则（简单直接）
  QDir dir(iniDir);
  QStringList filters;
  filters << "*.json";
  dir.setNameFilters(filters);
  dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

  // 2. 先把文件列表存到局部变量（避免临时对象引发的问题）
  QFileInfoList fileInfoList = dir.entryInfoList();

  // 3. 获取新名称（提前取，避免循环内重复访问UI）
  QString newName = ui->editRename->text().trimmed();
  if (newName.isEmpty()) {
    qWarning() << "新名称不能为空，取消修改";
    return;
  }

  // 4. 传统索引循环（最稳定，无任何兼容性问题）
  for (int i = 0; i < fileInfoList.count(); ++i) {
    QFileInfo fileInfo = fileInfoList.at(i);
    QString filePath = fileInfo.absoluteFilePath();

    // 5. 读取文件内容（基础文件操作，无技巧）
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "无法打开文件读取:" << filePath
                 << "错误:" << file.errorString();
      continue;  // 跳过当前文件，继续下一个
    }
    QByteArray fileData = file.readAll();
    file.close();  // 及时关闭文件，避免占用

    // 6. 解析JSON（结构化处理，避免暴力替换）
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
      qWarning() << "JSON解析失败:" << filePath
                 << "错误:" << parseError.errorString();
      continue;
    }
    if (!jsonDoc.isObject()) {
      qWarning() << "JSON根节点不是对象:" << filePath;
      continue;
    }

    // 7. 精准修改"childDesc"（按你的JSON结构逐层遍历）
    QJsonObject rootObj = jsonDoc.object();
    bool isModified = false;

    // 处理topItems数组
    if (rootObj.contains("topItems") && rootObj["topItems"].isArray()) {
      QJsonArray topItemsArr = rootObj["topItems"].toArray();
      for (int j = 0; j < topItemsArr.count(); ++j) {
        if (!topItemsArr[j].isObject()) continue;  // 跳过非对象元素
        QJsonObject topItemObj = topItemsArr[j].toObject();

        // 处理children子数组
        if (topItemObj.contains("children") &&
            topItemObj["children"].isArray()) {
          QJsonArray childrenArr = topItemObj["children"].toArray();
          for (int k = 0; k < childrenArr.count(); ++k) {
            if (!childrenArr[k].isObject()) continue;  // 跳过非对象元素
            QJsonObject childObj = childrenArr[k].toObject();

            // 只修改childDesc等于oldName的项
            if (childObj.contains("childDesc") &&
                childObj["childDesc"].isString()) {
              QString oldDesc = childObj["childDesc"].toString();
              if (oldDesc == oldName) {
                childObj["childDesc"] = newName;
                childrenArr[k] = childObj;
                isModified = true;  // 标记为已修改
              }
            }
          }
          topItemObj["children"] = childrenArr;
          topItemsArr[j] = topItemObj;
        }
      }
      rootObj["topItems"] = topItemsArr;
    }

    // 8. 有修改才写回文件（减少磁盘IO）
    if (isModified) {
      if (file.open(QIODevice::WriteOnly | QIODevice::Text |
                    QIODevice::Truncate)) {
        // 第一步：先给 jsonDoc 设置修改后的 rootObj（单独执行，因为返回 void）
        jsonDoc.setObject(rootObj);
        // 第二步：再调用 toJson() 获取 JSON 字符串（此时调用者是 QJsonDocument
        // 对象）
        QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Indented);
        // 第三步：写入文件
        file.write(jsonData);
        file.close();
        qDebug() << "修改成功:" << filePath << "(" << oldName << "→" << newName
                 << ")";
      } else {
        qWarning() << "无法打开文件写入:" << filePath
                   << "错误:" << file.errorString();
      }
    }
  }
}

void CategoryList::on_btnCancel_clicked() {
  mw_one->clearWidgetFocus();

  mui->frameCategory->hide();
  mui->frameEditRecord->show();
}
