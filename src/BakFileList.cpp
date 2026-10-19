#include "BakFileList.h"

#include <QDir>
#include <QFileInfo>
#include <QListWidgetItem>
#include <algorithm>

#include "BakFileDelegate.h"
#include "MainWindow.h"
#include "defines.h"
#include "src/Comm/Method.h"
#include "ui_BakFileList.h"

BakFileList::BakFileList(QWidget* parent)
    : QDialog(parent), ui(new Ui::BakFileList) {
  ui->setupUi(this);
  setModal(true);

  // ✅ 1. 禁用水平滚动条，确保条目宽度严格跟随控件宽度
  ui->listBakFile->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // ✅ 2. 安装自定义的中间省略委托
  ui->listBakFile->setItemDelegate(new BakFileDelegate(this));
}

BakFileList::~BakFileList() { delete ui; }

void BakFileList ::closeEvent(QCloseEvent* event) { Q_UNUSED(event); }

QStringList BakFileList::getData() {
  QStringList result;  // 用于存储返回的数据
  QDir dir(bakfileDir);

  // 1. 目录不存在时返回空列表
  if (!dir.exists()) {
    return result;
  }

  // 2. 设置过滤条件
  dir.setNameFilters(QStringList() << QStringLiteral("*_Knot.zip"));
  dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
  dir.setSorting(QDir::NoSort);

  // 3. 获取文件信息列表
  QFileInfoList fileInfoList = dir.entryInfoList();

  // 4. 使用 std::sort 按最后修改时间精准降序排序（最新 -> 最旧）
  std::sort(fileInfoList.begin(), fileInfoList.end(),
            [](const QFileInfo& a, const QFileInfo& b) {
              return a.lastModified() > b.lastModified();
            });

  // 5. 遍历排序后的列表，提取文件名（或绝对路径）
  result.reserve(fileInfoList.size());  // 预分配内存，提升性能
  for (const QFileInfo& fileInfo : fileInfoList) {
    // 根据需要选择返回 fileName() 还是 absoluteFilePath()
    result << fileInfo.fileName();

    // 如果需要返回绝对路径，改为：
    // result << fileInfo.absoluteFilePath();
  }

  return result;  // 6. 返回数据
}

void BakFileList::refreshBakFile() {
  // 1. 清空现有列表
  ui->listBakFile->clear();

  // 2. 获取排序后的备份文件列表
  QStringList listData = getData();

  // 3. 批量填充数据（使用 addItems 比循环 addItem 性能更好）
  if (!listData.isEmpty()) {
    ui->listBakFile->addItems(listData);
    // 4. 默认选中第一项（最新备份）
    ui->listBakFile->setCurrentRow(0);
  } else {
    // 可选：无备份文件时给出提示，避免用户面对空白列表困惑
    // 注意：这个提示项不应参与后续的 restore 操作
    QListWidgetItem* emptyTip =
        new QListWidgetItem(tr("No backup files available"));
    emptyTip->setFlags(Qt::NoItemFlags);  // 禁用选中/点击
    emptyTip->setTextAlignment(Qt::AlignCenter);
    ui->listBakFile->addItem(emptyTip);
  }
}

void BakFileList::loadBakFile() {
  if (isAndroid) {
    int x, y, w, h;
    x = mw_one->geometry().x();
    y = mw_one->geometry().y();
    w = mw_one->geometry().width();
    h = mw_one->geometry().height();
    setGeometry(x, y, w, h);
  } else
    m_Method->setWinPos(this, 350, 3);

  show();

  refreshBakFile();
}

void BakFileList::on_btnRestore_clicked() {
  if (ui->listBakFile->currentRow() < 0) return;

  QString zipfile = bakfileDir + ui->listBakFile->currentItem()->text();
  mw_one->m_MainHelper->importBakFileList(zipfile);
}

void BakFileList::on_btnBackup_clicked() {
  mw_one->on_actionExport_Data_triggered();
}

void BakFileList::on_btnFromFile_clicked() {
  mw_one->on_actionImport_Data_triggered();
}
