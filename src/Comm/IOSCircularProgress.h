#pragma once

#include <QBrush>
#include <QConicalGradient>
#include <QFont>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPropertyAnimation>
#include <QRadialGradient>
#include <QTimer>
#include <QWidget>

class IOSCircularProgress : public QWidget {
  Q_OBJECT
  Q_PROPERTY(int rotationAngle READ rotationAngle WRITE setRotationAngle)
  Q_PROPERTY(qreal progress READ progress WRITE setProgress)

 public:
  explicit IOSCircularProgress(QWidget* parent = nullptr)
      : QWidget(parent),
        m_rotation(0),
        m_progress(0),
        m_penWidth(5),
        m_seconds(0) {
    setFixedSize(60, 60);

    // 旋转动画
    m_rotateAnimation = new QPropertyAnimation(this, "rotationAngle", this);
    m_rotateAnimation->setDuration(1500);
    m_rotateAnimation->setLoopCount(-1);
    m_rotateAnimation->setStartValue(0);
    m_rotateAnimation->setEndValue(360);
    m_rotateAnimation->start();

    // 进度动画（示例用）
    auto* progressAnim = new QPropertyAnimation(this, "progress", this);
    progressAnim->setDuration(3000);
    progressAnim->setLoopCount(-1);
    progressAnim->setStartValue(0);
    progressAnim->setEndValue(1);
    progressAnim->start();

    // 初始化计时器，每秒更新一次
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this,
            &IOSCircularProgress::updateSeconds);
    m_timer->start(1000);
  }

  int rotationAngle() const { return m_rotation; }
  void setRotationAngle(int angle) {
    m_rotation = angle;
    update();
  }

  qreal progress() const { return m_progress; }
  void setProgress(qreal p) {
    m_progress = qBound<qreal>(0, p, 1);
    update();
  }

 protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const qreal outerRadius = qMin(width(), height()) * 0.5 - m_penWidth;
    const QPointF center = rect().center();

    // 绘制背景环
    QPen bgPen(QColor(0xE5, 0xE5, 0xE5));
    bgPen.setWidth(m_penWidth);
    bgPen.setCapStyle(Qt::RoundCap);
    p.setPen(bgPen);
    p.drawEllipse(center, outerRadius, outerRadius);

    // 绘制进度环
    if (m_progress > 0) {
      QConicalGradient gradient(center, -m_rotation);
      gradient.setColorAt(0.0, QColor(0x00, 0x7A, 0xFF));
      gradient.setColorAt(0.5, QColor(0x34, 0xC7, 0x59));
      gradient.setColorAt(1.0, QColor(0x00, 0x7A, 0xFF));

      QPen progressPen(QBrush(gradient), m_penWidth);
      progressPen.setCapStyle(Qt::RoundCap);
      p.setPen(progressPen);

      const int startAngle = (-m_rotation + 90) * 16;
      const int spanAngle = static_cast<int>(-m_progress * 360 * 16);

      p.drawArc(QRectF(center.x() - outerRadius, center.y() - outerRadius,
                       outerRadius * 2, outerRadius * 2),
                startAngle, spanAngle);
    }

    // 绘制高光
    QRadialGradient highlight(center, outerRadius * 2);
    highlight.setColorAt(0.0, QColor(255, 255, 255, 150));
    highlight.setColorAt(0.3, QColor(255, 255, 255, 50));
    highlight.setColorAt(1.0, Qt::transparent);
    p.setBrush(highlight);
    p.setPen(Qt::NoPen);
    p.drawEllipse(center, outerRadius + m_penWidth / 2.0,
                  outerRadius + m_penWidth / 2.0);

    // 绘制中央秒数
    p.setPen(QColor(0x33, 0x33, 0x33));
    QFont font = p.font();
    font.setPointSize(12);
    p.setFont(font);
    p.drawText(rect(), Qt::AlignCenter, QString::number(m_seconds));
  }

 private slots:
  void updateSeconds() {
    m_seconds++;
    update();
  }

 private:
  QPropertyAnimation* m_rotateAnimation;
  QTimer* m_timer;
  int m_rotation;
  qreal m_progress;
  int m_penWidth;
  int m_seconds;
};