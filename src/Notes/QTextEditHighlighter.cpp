#include "src/Notes/QTextEditHighlighter.h"

#include "src/Notes/LineNumberArea.h"

extern bool isDark;

QTextEditHighlighter::QTextEditHighlighter(QWidget *parent)
    : QTextEdit(parent) {
  // Line numbers
  lineNumberArea = new LineNumberArea(this);
  ///
  connect(this->document(), SIGNAL(blockCountChanged(int)), this,
          SLOT(updateLineNumberAreaWidth(int)));
  connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this,
          SLOT(updateLineNumberArea /*_2*/ (int)));
  connect(this, SIGNAL(textChanged()), this, SLOT(updateLineNumberArea()));
  connect(this, SIGNAL(cursorPositionChanged()), this,
          SLOT(updateLineNumberArea()));
  ///
  updateLineNumberAreaWidth(0);
}

int QTextEditHighlighter::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, this->document()->blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  int space = 0 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * (digits);

  return space;
}

void QTextEditHighlighter::updateLineNumberAreaWidth(int /* newBlockCount */) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void QTextEditHighlighter::updateLineNumberArea(QRectF /*rect_f*/) {
  QTextEditHighlighter::updateLineNumberArea();
}
void QTextEditHighlighter::updateLineNumberArea(int /*slider_pos*/) {
  QTextEditHighlighter::updateLineNumberArea();
}
void QTextEditHighlighter::updateLineNumberArea() {
  /*
   * When the signal is emitted, the sliderPosition has been adjusted according
   * to the action, but the value has not yet been propagated (meaning the
   * valueChanged() signal was not yet emitted), and the visual display has not
   * been updated. In slots connected to this signal you can thus safely adjust
   * any action by calling setSliderPosition() yourself, based on both the
   * action and the slider's value.
   */
  // Make sure the sliderPosition triggers one last time the valueChanged()
  // signal with the actual value !!!!

  // 2024-2-9
  // this->verticalScrollBar()->setSliderPosition(
  //   this->verticalScrollBar()->sliderPosition());

  // Since "QTextEdit" does not have an "updateRequest(...)" signal, we chose
  // to grab the imformations from "sliderPosition()" and "contentsRect()".
  // See the necessary connections used (Class constructor implementation part).

  QRect rect = this->contentsRect();
  lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
  updateLineNumberAreaWidth(0);
  //----------
  int dy = this->verticalScrollBar()->sliderPosition();
  if (dy > -1) {
    lineNumberArea->scroll(0, dy);
  }

  // Addjust slider to alway see the number of the currently being edited
  // line...
  int first_block_id = getFirstVisibleBlockId();
  if (first_block_id == 0 ||
      this->textCursor().block().blockNumber() == first_block_id - 1) {
    // this->verticalScrollBar()->setSliderPosition(
    //     dy - this->document()->documentMargin());

    //    // Snap to first line (TODO...)
    //    if (first_block_id > 0)
    //    {
    //        int slider_pos = this->verticalScrollBar()->sliderPosition();
    //        int prev_block_height = (int)
    //        this->document()->documentLayout()->blockBoundingRect(this->document()->findBlockByNumber(first_block_id-1)).height();
    //        if (dy <= this->document()->documentMargin() + prev_block_height)
    //            this->verticalScrollBar()->setSliderPosition(slider_pos -
    //            (this->document()->documentMargin() + prev_block_height));
    //    }
  }
}

void QTextEditHighlighter::resizeEvent(QResizeEvent *e) {
  QTextEdit::resizeEvent(e);

  QRect cr = this->contentsRect();
  lineNumberArea->setGeometry(
      QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

int QTextEditHighlighter::getFirstVisibleBlockId() {
  // Detect the first block for which bounding rect - once translated
  // in absolute coordinated - is contained by the editor's text area

  // Costly way of doing but since "blockBoundingGeometry(...)" doesn't
  // exists for "QTextEdit"...

  QTextCursor curs = QTextCursor(this->document());
  curs.movePosition(QTextCursor::Start);
  for (int i = 0; i < this->document()->blockCount(); ++i) {
    QTextBlock block = curs.block();

    QRect r1 = this->viewport()->geometry();
    QRect r2 =
        this->document()
            ->documentLayout()
            ->blockBoundingRect(block)
            .translated(this->viewport()->geometry().x(),
                        this->viewport()->geometry().y() -
                            (this->verticalScrollBar()->sliderPosition()))
            .toRect();

    if (r1.contains(r2, true)) {
      return i;
    }

    curs.movePosition(QTextCursor::NextBlock);
  }

  return 0;
}

void QTextEditHighlighter::lineNumberAreaPaintEvent(QPaintEvent *event) {
  // this->verticalScrollBar()->setSliderPosition(
  //     this->verticalScrollBar()->sliderPosition());

  QPainter painter(lineNumberArea);

  if (isDark)
    painter.fillRect(event->rect(), QColor(69, 83, 100));
  else
    painter.fillRect(event->rect(), Qt::lightGray);

#ifdef Q_OS_ANDROID
  QFont font = this->font();
  font.setPointSize(13);
  // painter.setFont(font);
#endif

  int blockNumber = this->getFirstVisibleBlockId();

  QTextBlock block = this->document()->findBlockByNumber(blockNumber);
  QTextBlock prev_block =
      (blockNumber > 0) ? this->document()->findBlockByNumber(blockNumber - 1)
                        : block;
  int translate_y =
      (blockNumber > 0) ? -this->verticalScrollBar()->sliderPosition() : 0;

  int top = this->viewport()->geometry().top();

  // Adjust text position according to the previous "non entirely visible" block
  // if applicable. Also takes in consideration the document's margin offset.
  int additional_margin;
  if (blockNumber == 0)
    // Simply adjust to document's margin
    additional_margin = (int)this->document()->documentMargin() - 1 -
                        this->verticalScrollBar()->sliderPosition();
  else
    // Getting the height of the visible part of the previous "non entirely
    // visible" block
    additional_margin = (int)this->document()
                            ->documentLayout()
                            ->blockBoundingRect(prev_block)
                            .translated(0, translate_y)
                            .intersected(this->viewport()->geometry())
                            .height();

  // Shift the starting point
  top += additional_margin;

  int bottom = top + (int)this->document()
                         ->documentLayout()
                         ->blockBoundingRect(block)
                         .height();

  QColor col_1;  // Current line (custom green)
  if (isDark)
    col_1.setRgb(50, 205, 50);
  else
    col_1.setRgb(255, 0, 0);

  QColor col_0;
  if (isDark)
    col_0.setRgb(255, 255, 255);  // Other lines  (custom darkgrey)
  else
    col_0.setRgb(0, 0, 0);

  // Draw the numbers (displaying the current line number in green)
  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(QColor(120, 120, 120));
      painter.setPen((this->textCursor().blockNumber() == blockNumber) ? col_1
                                                                       : col_0);
      painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                       Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + (int)this->document()
                       ->documentLayout()
                       ->blockBoundingRect(block)
                       .height();
    ++blockNumber;
  }
}
