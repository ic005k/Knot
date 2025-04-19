#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include <QPainter>
#include <QRectF>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextEdit>
#include <QWidget>

class QTextEditHighlighter : public QTextEdit {
  Q_OBJECT

 public:
  explicit QTextEditHighlighter(QWidget *parent = 0);

  int getFirstVisibleBlockId();
  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();

 signals:

 public slots:

  void resizeEvent(QResizeEvent *e);

 private slots:

  void updateLineNumberAreaWidth(int newBlockCount);
  void updateLineNumberArea(QRectF /*rect_f*/);
  void updateLineNumberArea(int /*slider_pos*/);
  void updateLineNumberArea();

 private:
  QWidget *lineNumberArea;
};
