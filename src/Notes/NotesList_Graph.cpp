#include "NotesList.h"
#include "src/MainWindow.h"

void NotesList::initNoteGraphView() {
  m_graphController = new NoteGraphController(this);

  // ★ 监听 JSON 数据就绪信号
  connect(m_graphController, &NoteGraphController::graphJsonChanged, this,
          [this]() {
            if (!isAndroid) mw_one->safeCloseProgress();

            // 获取中性 JSON 字符串
            QString jsonData = m_graphController->graphJson();

            qInfo() << "图谱jsonData=" << jsonData;

            listNoteGraph.clear();
            listNoteGraph.append(noteTitle);
            listNoteGraph.append(jsonData);

            if (isAndroid) {
              m_Method->execJavaFunc("mInstance", "dismissAiLoadingDialog",
                                     "NoteActivity");
              m_Method->openActivity("openNoteGraphActivity", listNoteGraph);
            } else {
              m_NoteGraphView->showNoteGraph();
            }
          });
}
