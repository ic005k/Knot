#include <QSize>
#include <QTextEdit>
#include <QWidget>

class LineNumberArea : public QWidget {
  Q_OBJECT

 public:
  LineNumberArea(QTextEdit *editor);

  QSize sizeHint() const;

 protected:
  void paintEvent(QPaintEvent *event);

 private:
  QTextEdit *codeEditor;
};
