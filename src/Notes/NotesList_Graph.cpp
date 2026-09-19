#include "NotesList.h"
#include "src/MainWindow.h"

void NotesList::initNoteGraphView() {
  if (!m_graphController) {
    m_graphController = new NoteGraphController(this);

    // ★ 监听 JSON 数据就绪信号
    connect(m_graphController, &NoteGraphController::graphJsonChanged, this,
            [this]() {
              // 数据准备好了，关闭进度条
              // mw_one->safeCloseProgress();

              // 获取中性 JSON 字符串
              QString jsonData = m_graphController->graphJson();

              qInfo() << "图谱jsonData=" << jsonData;
              listNoteGraph.clear();
              int idx = getNoteListOrgIndex();
              QString mainTitle = listNoteEntry.at(idx);
              listNoteGraph.append(mainTitle);
              listNoteGraph.append(jsonData);

              if (isAndroid) {
              } else
                m_NoteGraphView->showNoteGraph();
            });
  }
}
