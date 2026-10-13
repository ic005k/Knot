#include "BakFileList.h"

#include <QDir>
#include <QFileInfo>
#include <QListWidgetItem>
#include <algorithm>

#include "MainWindow.h"
#include "defines.h"
#include "src/Comm/Method.h"
#include "ui_BakFileList.h"

BakFileList::BakFileList(QWidget* parent)
    : QDialog(parent), ui(new Ui::BakFileList) {
  ui->setupUi(this);
  setModal(true);
}

BakFileList::~BakFileList() { delete ui; }

void BakFileList ::closeEvent(QCloseEvent* event) { Q_UNUSED(event); }

void BakFileList::loadBakFile() {
  // 1. 清空现有列表
  ui->listBakFile->clear();

  QDir dir(bakfileDir);
  if (!dir.exists()) {
    return;
  }

  // 2. 设置过滤条件
  dir.setNameFilters(QStringList() << QStringLiteral("*_Knot.zip"));
  dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
  // 不使用 QDir::Time，避免平台差异，改为 NoSort 或 QDir::Name
  dir.setSorting(QDir::NoSort);

  // 3. 获取文件信息列表
  QFileInfoList fileInfoList = dir.entryInfoList();

  // 4. 使用 std::sort 按最后修改时间精准降序排序（最新 -> 最旧）
  std::sort(fileInfoList.begin(), fileInfoList.end(),
            [](const QFileInfo& a, const QFileInfo& b) {
              // lastModified() 返回 QDateTime，直接比较即可
              // 如果需要极致精度（毫秒/微秒级），可改用
              // a.fileTime(QFileDevice::FileModificationTime)
              return a.lastModified() > b.lastModified();
            });

  // 5. 遍历排序后的列表，填充到 QListWidget
  for (const QFileInfo& fileInfo : fileInfoList) {
    QListWidgetItem* item = new QListWidgetItem(fileInfo.fileName());

    // 将完整路径存储到 UserRole 中，方便后续双击或读取时使用
    item->setData(Qt::UserRole, fileInfo.absoluteFilePath());

    // 可选：将精确的最后修改时间显示为 Tooltip，方便用户查看
    item->setToolTip(fileInfo.lastModified().toString(
        QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")));

    ui->listBakFile->addItem(item);
  }

  if (isAndroid) {
    int x, y, w, h;
    x = mw_one->geometry().x();
    y = mw_one->geometry().y();
    w = mw_one->geometry().width();
    h = mw_one->geometry().height();
    setGeometry(x, y, w, h);
  } else
    m_Method->setWinPos(this, 350, 3);

  if (ui->listBakFile->count() > 0) ui->listBakFile->setCurrentRow(0);

  show();
}

void BakFileList::on_btnRestore_clicked() {
  if (ui->listBakFile->currentRow() < 0) return;

  QString zipfile = bakfileDir + ui->listBakFile->currentItem()->text();
  mw_one->m_MainHelper->importBakFileList(zipfile);
}
