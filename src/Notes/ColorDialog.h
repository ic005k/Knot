/* https://github.com/agafon0ff/SimpleColorDialog */

#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QMouseEvent>

class ColorDialog : public QDialog {
  Q_OBJECT
 public:
  explicit ColorDialog(QWidget *parent = Q_NULLPTR);

 private:
  struct RectStruct {
    QRect rect;
    QString color;
  };

  QList<RectStruct> m_rectList;
  int m_hovered;
  QString m_currentColor;

  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);
  void mouseMoveEvent(QMouseEvent *e);
  void mousePressEvent(QMouseEvent *e);

  int getHitedRect(const QPoint &pos);

 signals:
  void currentColor(const QString &color);

 public slots:
  QString getColor() { return m_currentColor; }
};

#endif  // COLORDIALOG_H
