#include "NotesList.h"
#include "src/MainWindow.h"

void NotesList::initNoteGraphView() {
  // 纯 C++ 初始化：直接 new 即可，无需任何 QML 注册
  if (!m_graphController) {
    m_graphController = new NoteGraphController(mw_one);

    // 重新绑定原本在 QML 阶段绑定的信号
    connect(m_graphController, &NoteGraphController::nodeDoubleClicked, this,
            &NotesList::onNoteNodeDoubleClicked);

    qDebug() << "[Graph] 纯C++模式初始化完成:" << m_graphController;
  }
}
