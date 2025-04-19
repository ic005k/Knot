/*https://github.com/agafon0ff/SimpleColorDialog*/

#include "src/Notes/ColorDialog.h"

#include <QPainter>

const int listSizeX = 13;
const int listSizeY = 15;

const QStringList colorsList = {
    "#fdd", "#ffd", "#ffd", "#ffd", "#dfd", "#dff", "#dff", "#dff", "#ddf",
    "#fdf", "#fdf", "#fdf", "#fff", "#fbb", "#feb", "#ffb", "#efb", "#bfb",
    "#bfe", "#bff", "#bef", "#bbf", "#ebf", "#fbf", "#fbe", "#eee", "#f99",
    "#fd9", "#ff9", "#df9", "#9f9", "#9fd", "#9ff", "#9df", "#99f", "#d9f",
    "#f9f", "#f9d", "#ddd", "#f77", "#fc7", "#ff7", "#cf7", "#7f7", "#7fc",
    "#7ff", "#7cf", "#77f", "#c7f", "#f7f", "#f7c", "#ccc", "#f55", "#fb5",
    "#ff5", "#bf5", "#5f5", "#5fb", "#5ff", "#5bf", "#55f", "#b5f", "#f5f",
    "#f5b", "#bbb", "#f33", "#fa3", "#ff3", "#af3", "#3f3", "#3fa", "#3ff",
    "#3af", "#33f", "#a3f", "#f3f", "#f3a", "#aaa", "#f11", "#f91", "#ff1",
    "#9f1", "#1f1", "#1f9", "#1ff", "#19f", "#11f", "#91f", "#f1f", "#f19",
    "#999", "#f00", "#f80", "#ff0", "#8f0", "#0f0", "#0f8", "#0ff", "#08f",
    "#00f", "#80f", "#f0f", "#f08", "#888", "#d00", "#d70", "#dd0", "#7d0",
    "#0d0", "#0d7", "#0dd", "#07d", "#00d", "#70d", "#d0d", "#d07", "#777",
    "#b00", "#b60", "#bb0", "#6b0", "#0b0", "#0b6", "#0bb", "#06b", "#00b",
    "#60b", "#b0b", "#b06", "#666", "#900", "#950", "#990", "#590", "#090",
    "#095", "#099", "#059", "#009", "#509", "#909", "#905", "#555", "#700",
    "#740", "#770", "#470", "#070", "#074", "#077", "#047", "#007", "#407",
    "#707", "#704", "#444", "#500", "#530", "#550", "#350", "#050", "#053",
    "#055", "#035", "#005", "#305", "#505", "#503", "#333", "#300", "#320",
    "#330", "#230", "#030", "#032", "#033", "#023", "#003", "#203", "#303",
    "#302", "#222", "#100", "#110", "#110", "#110", "#010", "#011", "#011",
    "#011", "#001", "#101", "#101", "#000", "#111"};

ColorDialog::ColorDialog(QWidget *parent)
    : QDialog(parent), m_hovered(-1), m_currentColor("#f00") {
  resize(360, 380);
  setWindowTitle(tr("Color Selection"));
  setWhatsThis(tr("Click on color to select"));

  int lSize = colorsList.size();
  for (int i = 0; i < lSize; ++i) {
    RectStruct rStruct;
    rStruct.color = colorsList.at(i);
    m_rectList.append(rStruct);
  }

  setMouseTracking(true);
  setStyleSheet("background: #777;");
}

void ColorDialog::paintEvent(QPaintEvent *) {
  QPainter p(this);

  p.setRenderHint(QPainter::Antialiasing);

  int lSize = m_rectList.size();

  if (m_hovered != -1) {
    p.setPen(QPen(QColor(255, 255, 255, 255), 2));
    p.setBrush(QBrush(QColor(255, 255, 255, 255)));
    p.drawRect(m_rectList.at(m_hovered).rect.x() - 2,
               m_rectList.at(m_hovered).rect.y() - 2,
               m_rectList.at(m_hovered).rect.width() + 4,
               m_rectList.at(m_hovered).rect.height() + 4);
  }

  p.setPen(QPen(QColor(0, 0, 0, 0), 1));

  for (int i = 0; i < lSize; ++i) {
    RectStruct sRect = m_rectList.at(i);
    p.setBrush(QBrush(QColor(sRect.color)));
    p.drawRect(sRect.rect);
  }

  p.end();
}

void ColorDialog::resizeEvent(QResizeEvent *) {
  qreal rWidth =
      static_cast<qreal>(width() - 3) / static_cast<qreal>(listSizeX);
  qreal rHeight =
      static_cast<qreal>(height() - 3) / static_cast<qreal>(listSizeY);
  int step = 0;

  for (int i = 0; i < listSizeY; ++i) {
    for (int j = 0; j < listSizeX; ++j) {
      QRect rect(3 + j * rWidth, 3 + i * rHeight, rWidth - 3, rHeight - 3);
      m_rectList[step].rect = rect;
      ++step;
    }
  }
}

void ColorDialog::mouseMoveEvent(QMouseEvent *e) {
  int rectNum = getHitedRect(e->pos());

  if (rectNum != -1) {
    if (rectNum != m_hovered) {
      m_hovered = rectNum;
      update();
    }
  }
}

void ColorDialog::mousePressEvent(QMouseEvent *e) {
  if (e->button() == Qt::LeftButton) {
    int rectNum = getHitedRect(e->pos());

    if (rectNum != -1) {
      m_currentColor = m_rectList.at(rectNum).color;
      emit currentColor(m_currentColor);
      accept();
    }
  }
}

int ColorDialog::getHitedRect(const QPoint &pos) {
  int result = -1;
  int listSize = m_rectList.size();

  for (int i = 0; i < listSize; ++i) {
    if (m_rectList.at(i).rect.contains(pos)) {
      result = i;
      break;
    }
  }

  return result;
}
