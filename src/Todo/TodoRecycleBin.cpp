#include "TodoRecycleBin.h"

#include "Todo/TodoRecycleBinDelegate.h"
#include "defines.h"
#include "src/Comm/Method.h"
#include "ui_TodoRecycleBin.h"

TodoRecycleBin::TodoRecycleBin(QWidget* parent)
    : QDialog(parent), ui(new Ui::TodoRecycleBin) {
  ui->setupUi(this);
  this->setModal(true);
}

TodoRecycleBin::~TodoRecycleBin() { delete ui; }

void TodoRecycleBin::on_btnClear_clicked() {}

void TodoRecycleBin::on_btnDel_clicked() {}

void TodoRecycleBin::on_btnRestore_clicked() {}

void TodoRecycleBin::showTodoRecycleBin(QStringList list) {
  qInfo() << "list=" << list;

  ui->listRecycle->clear();

  // 隐藏水平滚动条，Delegate 的 sizeHint 会自动适配 viewport 宽度
  ui->listRecycle->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui->listRecycle->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  // 设置自定义代理（替代之前的 setItemWidget + QSS 方案）
  ui->listRecycle->setItemDelegate(new TodoRecycleBinDelegate(this));

  for (const QString& item : list) {
    // 直接以原始字符串作为 DisplayRole 数据
    // Delegate 内部负责解析和渲染
    QListWidgetItem* listItem = new QListWidgetItem(item, ui->listRecycle);
    // sizeHint 由 Delegate 自动提供，无需手动设置
    ui->listRecycle->addItem(listItem);
  }

  m_Method->setWinPos(this, 350);
  show();
}