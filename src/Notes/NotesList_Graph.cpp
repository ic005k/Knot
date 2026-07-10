#include "NotesList.h"

void NotesList::initNoteGraphView() {
  // 1. 先初始化图谱组件（注册QML类型和单例）
  registerNoteGraphTypes();  // 注册 NoteGraphModel 和 NoteRelationParser
  initializeNoteGraph();     // 注册 NoteGraphController 单例

  // 2. 确保 QQuickWidget 已初始化
  if (!mui->qwNoteGraphView) {
    qWarning() << "QQuickWidget 未初始化";
    return;
  }

  // 在加载 QML 前，先设置一个占位符属性 ***
  // 这可以避免 QML 在引擎初始化早期阶段因访问未定义属性而报错
  // 使用 nullptr 作为初始值
  mui->qwNoteGraphView->rootContext()->setContextProperty(
      "graphController", QVariant::fromValue<QObject*>(nullptr));
  mui->qwNoteGraphView->rootContext()->setContextProperty("isDark", isDark);

  // 3. 加载 QML 源文件（这一步会触发 QML 引擎初始化）
  // 此时，QML 引擎已经知道 graphController 这个属性的存在（虽然是 nullptr）
  mui->qwNoteGraphView->setSource(
      QUrl(QStringLiteral("qrc:/src/qmlsrc/NoteGraphView.qml")));

  // 4. 获取 QML 引擎（此时引擎已初始化）
  QQmlEngine* engine = mui->qwNoteGraphView->engine();
  if (!engine) {
    qWarning() << "无法获取 QML 引擎";
    // 即使引擎获取失败，我们也已经设置了占位符，避免 QML 报错
    return;
  }

  // 5. 尝试获取 NoteGraphController 单例（Qt 6 正确写法）
  NoteGraphController* controller =
      engine->singletonInstance<NoteGraphController*>("NoteGraph",
                                                      "NoteGraphController");
  if (!controller) {
    qWarning() << "无法从引擎获取 NoteGraphController 单例";
    // 注意：这里不再 return，而是继续将 nullptr 设置给属性
    // 这样 QML 中的检查 (graphController && ...) 可以处理控制器不可用的情况
  } else {
    qDebug() << "成功获取 NoteGraphController 单例指针:" << controller;
  }

  // 6. 无论 controller 是否为空，都更新上下文属性 ***
  // 如果 controller 有效，QML 现在可以访问到真正的控制器实例。
  // 如果 controller 是 nullptr，QML 中的空值检查可以防止崩溃。
  m_graphController = controller;  // 更新成员变量
  mui->qwNoteGraphView->rootContext()->setContextProperty("graphController",
                                                          m_graphController);
  qDebug() << "已将 graphController 上下文属性设置为:" << m_graphController;

  // 7. 如果 controller 有效，则连接节点双击信号（打开对应笔记）
  if (m_graphController) {
    connect(m_graphController, &NoteGraphController::nodeDoubleClicked, this,
            &NotesList::onNoteNodeDoubleClicked);
    qDebug() << "已连接 NoteGraphController 的 nodeDoubleClicked 信号";
  } else {
    qWarning() << "NoteGraphController 为空，未连接 nodeDoubleClicked 信号";
  }
}
