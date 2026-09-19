#include "MyNoteDiffDelegate.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>

// 定义数据角色，与 QListWidgetItem::setData 对应
namespace DiffRoles {
constexpr int TimeRole = Qt::UserRole;       // 修改时间
constexpr int HtmlRole = Qt::UserRole + 1;   // HTML 差异
constexpr int PatchRole = Qt::UserRole + 2;  // 旧文本/Patch
}  // namespace DiffRoles

MyNoteDiffDelegate::MyNoteDiffDelegate(QObject* parent)
    : QStyledItemDelegate(parent),
      m_oldTextBtnLabel(tr("Old Text"))  // ✅ 在构造函数中调用 tr()，上下文明确
{}

// ================= 1. 绘制 =================
void MyNoteDiffDelegate::paint(QPainter* painter,
                               const QStyleOptionViewItem& option,
                               const QModelIndex& index) const {
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  // ✅ 【关键】清空 text，防止 CE_ItemViewItem 重复绘制文本导致重影
  opt.text = QString();

  painter->save();

  // 现在只会绘制背景、选中高亮、焦点框等，不会画文字
  QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

  // 2. 绘制左侧文本（修改时间）
  QRect textRect = opt.rect.adjusted(15, 0, -100, 0);
  QString timeText = index.data(DiffRoles::TimeRole).toString();

  QColor textColor = (opt.state & QStyle::State_Selected)
                         ? opt.palette.highlightedText().color()
                         : opt.palette.text().color();
  painter->setPen(textColor);

  QFont font = painter->font();
  font.setPointSize(10);
  painter->setFont(font);
  painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, timeText);

  // 3. 绘制右侧按钮（保持不变）
  QStyleOptionButton buttonOpt;
  buttonOpt.rect = getButtonRect(opt);
  buttonOpt.text = m_oldTextBtnLabel;
  buttonOpt.state = QStyle::State_Enabled | QStyle::State_Raised;
  QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonOpt,
                                     painter);

  painter->restore();
}

// ================= 2. 尺寸 =================
QSize MyNoteDiffDelegate::sizeHint(const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
  Q_UNUSED(index)
  // 宽度跟随视图，高度固定为 45 像素，让界面更宽松
  return QSize(option.rect.width(), 45);
}

// ================= 3. 事件拦截 =================
bool MyNoteDiffDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                     const QStyleOptionViewItem& option,
                                     const QModelIndex& index) {
  // 只处理鼠标释放事件
  if (event->type() == QEvent::MouseButtonRelease) {
    auto* mouseEvent = static_cast<QMouseEvent*>(event);

    // 判断点击位置是否在按钮区域内
    QRect btnRect = getButtonRect(option);
    if (btnRect.contains(mouseEvent->pos())) {
      // 发出信号，通知外部“旧文本”被点击
      emit oldTextButtonClicked(index.row());
      return true;  // 消费事件，阻止列表项的默认选中行为
    }
  }

  // 其他事件交由基类处理（如点击空白处选中该行）
  return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// ================= 辅助：计算按钮矩形 =================
QRect MyNoteDiffDelegate::getButtonRect(
    const QStyleOptionViewItem& option) const {
  int btnWidth = 70;
  int btnHeight = 28;
  int marginRight = 15;

  int x = option.rect.right() - btnWidth - marginRight;
  int y = option.rect.center().y() - (btnHeight / 2);

  return QRect(x, y, btnWidth, btnHeight);
}