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
              closeNoteActivityLoadingDlg();
              m_Method->openActivity("openNoteGraphActivity", listNoteGraph);
            } else {
              m_NoteGraphView->showNoteGraph();
            }
          });
}

void NotesList::closeNoteActivityLoadingDlg() {
#ifdef Q_OS_ANDROID

  QString className = "NoteActivity";
  QString callJavaName = "dismissLoadingDialog";

  QString c1, c2;
  c1 = "com/x/" + className;
  c2 = "Lcom/x/" + className + ";";

  QJniObject instance = QJniObject::getStaticObjectField(
      c1.toUtf8().constData(), "mInstance", c2.toUtf8().constData());

  if (instance.isValid()) {
    instance.callMethod<void>(callJavaName.toUtf8().constData(), "()V");
  }

  qInfo() << callJavaName;

#endif
}
