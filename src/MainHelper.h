#ifndef MAINHELPER_H
#define MAINHELPER_H

#include <QDialog>
#include <QMenu>
#include <QObject>
#include <QTreeWidget>
#include <QWidget>

class MainWindow;  // 前向声明，避免头文件循环包含

class MainHelper : public QDialog {
  Q_OBJECT
 public:
  explicit MainHelper(QWidget *parent = nullptr);

  void clickBtnChart();
  void clickBtnRestoreTab();
  bool mainEventFilter(QObject *watch, QEvent *evn);
  QTreeWidget *init_TreeWidget(QString name);
  void init_Menu(QMenu *mainMenu);
  void openTabRecycle();
  void initQW();
  void init_UIWidget();
  void startBackgroundTaskUpdateBakFileList();

  void init_ButtonStyle();
  void delBakFile();
  void delTabRecycleFile();
  void importBakFileList();
  void init_Theme();

  void sort_childItem(QTreeWidgetItem *item);
  private:
  QStringList bakFileList;

  QString clickableLabelButtonStyle = R"(
        /* 模拟按钮样式：中蓝底+白字，适配明暗主题 */
        QLabel {
            /* 中蓝色背景（明暗主题下保持一致，突出可点击） */
            background-color: #FF9933;  /* 中蓝色 #2196F3; 安卓常用强调色 */

            /* 文字白色，与蓝色背景高对比，确保清晰 */
            color: white;

            /* 按钮质感：圆角+内边距 */
            border-radius: 4px; /* 圆角，模拟安卓按钮 */
            padding: 6px 12px; /* 上下6px，左右12px内边距，避免文字贴边 */

            /* 文字居中（按钮类元素常用） */
            /* text-align: center; */

        }

        /* 点击反馈：按下时颜色加深，模拟按钮按压感 */
        QLabel:pressed {
            background-color: #1976D2; /* 深蓝，比原色调深一级 */
        }

        /* 禁用状态（可选，需要时启用） */
        QLabel:disabled {
            background-color: #BBDEFB; /* 浅蓝灰色，区分禁用状态 */
            color: #757575; /* 灰色文字 */
        }
        )";

 signals:
};

#endif  // MAINHELPER_H
