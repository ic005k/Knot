#include "MyNoteDiff.h"

#include "Comm/Method.h"
#include "MainWindow.h"
#include "Notes/MyNoteDiffDelegate.h"
#include "Notes/NotesList.h"
#include "defines.h"
#include "ui_MyNoteDiff.h"

MyNoteDiff::MyNoteDiff(QWidget* parent)
    : QDialog(parent), ui(new Ui::MyNoteDiff) {
  ui->setupUi(this);
  setModal(true);

  // 设置自定义委托！
  MyNoteDiffDelegate* delegate = new MyNoteDiffDelegate(ui->listRevision);
  ui->listRevision->setItemDelegate(delegate);

  // ✅ 连接列表选中变化信号，实现点击条目显示差异详情
  connect(ui->listRevision, &QListWidget::currentRowChanged, this,
          [this](int row) {
            if (row >= 0 && row < noteDiffHtml.size()) {
              ui->textBrowser->setHtml(noteDiffHtml.at(row));
            } else {
              ui->textBrowser->clear();
            }
          });

  // 连接“旧文本”按钮点击信号（按需处理）
  connect(delegate, &MyNoteDiffDelegate::oldTextButtonClicked, this,
          [this](int row) {
            // TODO: 处理旧文本按钮点击，例如弹出 noteDiffPatch[row]
            m_NotesList->newtextToOldtextFromDiffStr(row);
          });
}

MyNoteDiff::~MyNoteDiff() { delete ui; }

void MyNoteDiff::showDiff() {
  ui->listRevision->clear();

  ui->listRevision->setUpdatesEnabled(false);  // 批量插入时关闭刷新，提升性能
  for (int i = 0; i < noteDiffTime.size(); ++i) {
    auto* item = new QListWidgetItem;

    // 存入修改时间（供委托绘制左侧文本）
    item->setData(Qt::UserRole, noteDiffTime.at(i));

    // 存入 HTML 差异（供单击时显示）
    // 注意：这里对应你在 Delegate 中定义的 HtmlRole (Qt::UserRole + 1)
    item->setData(Qt::UserRole + 1, noteDiffHtml.at(i));

    // ⚠️ 关键：必须设置一个非空的 DisplayRole，否则 QListWidget
    // 可能不会正确触发 delegate 的 paint/sizeHint
    item->setText(noteDiffTime.at(i));

    ui->listRevision->addItem(item);
  }
  ui->listRevision->setUpdatesEnabled(true);  // 恢复刷新

  // 4. 默认选中第一项（最新记录），触发详情显示
  if (ui->listRevision->count() > 0) {
    ui->listRevision->setCurrentRow(0);
  }

  m_Method->setWinPos(this, 350, 3);
  show();
}
