#include "WheelWidget.h"

#include <QElapsedTimer>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <QtMath>

extern bool isDark;

WheelWidget::WheelWidget(QWidget* parent) : QWidget(parent) {
  m_itemHeight = 45;
  setFixedHeight(45 * 3);

  // 惯性动画定时器（60FPS）
  m_inertiaTimer.setInterval(16);

  connect(&m_inertiaTimer, &QTimer::timeout, this,
          &WheelWidget::animateInertia);
}

void WheelWidget::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);

  if (isDark) {
    m_textColor = QColor(0xFFE0E0E0);       // 浅灰白
    m_highlightColor = QColor(0xFF0A84FF);  // iOS 暗黑蓝
  } else {
    m_textColor = QColor(0xFF000000);       // 纯黑
    m_highlightColor = QColor(0xFF007AFF);  // iOS 默认蓝
  }

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  const int centerY = height() / 2;

  // 计算当前中心位置的浮点索引
  qreal centerIndex =
      m_currentIndex + static_cast<qreal>(m_offset) / m_itemHeight;

  // 动态计算需要绘制的项数（覆盖两倍屏幕高度）
  const int visibleItemCount = qCeil(height() / (qreal)m_itemHeight) * 2 + 3;

  // 计算绘制的起始和结束索引
  int startIndex = qFloor(centerIndex) - visibleItemCount;
  int endIndex = qCeil(centerIndex) + visibleItemCount;

  for (int k = startIndex; k <= endIndex; ++k) {
    // 循环核心算法：虚拟索引计算
    int virtualIndex = (k + m_items.size() * 10) % m_items.size();

    // 项位置计算：基于浮点中心索引
    qreal yPos = centerY + (k - centerIndex) * m_itemHeight;

    // 放宽可见性判断条件（确保部分可见的项不会被跳过）
    if (yPos < -m_itemHeight || yPos > height() + m_itemHeight) continue;

    // 动态样式计算（保持iOS风格）
    qreal distance = qAbs(yPos - centerY) / m_itemHeight;

    qreal opacity = qBound(0.2, 1.0 - distance * 0.4, 1.0);

    int fontSize = qBound(14, 24 - static_cast<int>(distance * 8), 24);

    painter.setOpacity(opacity);
    painter.setPen(m_textColor);
    painter.setFont(QFont("Helvetica Neue", fontSize));
    painter.drawText(QRect(0, yPos - m_itemHeight / 2, width(), m_itemHeight),
                     Qt::AlignCenter, m_items[virtualIndex]);
  }

  // iOS风格高亮区域
  painter.fillRect(QRect(0, centerY - m_itemHeight / 2, width(), m_itemHeight),
                   QColor(0, 122, 255, 38));
}

void WheelWidget::mousePressEvent(QMouseEvent* event) {
  m_dragStartY = event->pos().y();
  m_baseOffset = m_offset;  // 保存当前偏移量基准
  m_velocity = 0;
  m_dragTimer.start();
  m_inertiaTimer.stop();

  // 之前Qt5正常实现
  //  QWidget::mousePressEvent(event);

  event->accept();  // 明确接受事件
}

void WheelWidget::mouseReleaseEvent(QMouseEvent* event) {
  // 计算惯性速度（像素/毫秒） 之前Qt5正常实现
  /*
  qreal elapsed = m_dragTimer.elapsed();
    m_velocity = (elapsed > 0) ? (m_offset - m_baseOffset) / elapsed : 0;

    if (qAbs(m_velocity) > 0.5) {
      m_inertiaTimer.start();
    } else {
      updateIndex();
    }
    QWidget::mouseReleaseEvent(event);
  */

  // 计算实际拖拽距离（未经循环调整）
  const int actualDelta = m_offset - m_baseOffset;
  qreal elapsed = m_dragTimer.elapsed();
  m_velocity = (elapsed > 0) ? actualDelta / elapsed : 0;

  m_dragTimer.invalidate();  // 重置计时器

  if (qAbs(m_velocity) > 0.5) {
    m_inertiaTimer.start();
  } else {
    updateIndex();  // 直接同步状态
  }

  event->accept();  // 明确接受事件
}

void WheelWidget::mouseMoveEvent(QMouseEvent* event) {
  if (!m_dragTimer.isValid()) return;

  // 原始拖拽距离计算（保持1:1跟随）
  // const int deltaY = event->pos().y() - m_dragStartY; //相反方向
  const int deltaY = m_dragStartY - event->pos().y();  // 一般情况下的自然方向

  m_offset = m_baseOffset + deltaY;

  // 循环滚动核心：当偏移超过阈值时重置
  const int cycleThreshold = m_itemHeight * m_items.size();
  if (qAbs(m_offset) > cycleThreshold) {
    const int cycleThreshold = m_itemHeight * m_items.size();
    if (cycleThreshold != 0) {  // 避免除零
      m_offset = std::fmod(m_offset, static_cast<qreal>(cycleThreshold));

      // 同步调整到正区间（若需要）
      if (m_offset < 0) {
        m_offset += cycleThreshold;
      }
    }
  }

  update();
}

void WheelWidget::updateIndex0() {
  // 相反方向：offset正 → 项下移 → 索引递增
  const qreal position = m_offset / m_itemHeight;  // 移除负号
  int deltaIndex = static_cast<int>(std::round(position));

  // 循环模式计算
  if (m_circular) {
    m_currentIndex =
        (m_currentIndex + deltaIndex + m_items.size()) % m_items.size();
  } else {
    m_currentIndex = qBound(0, m_currentIndex + deltaIndex, m_items.size() - 1);
  }

  m_offset = 0;  // 强制对齐项中心
  emit currentIndexChanged(m_currentIndex);
  update();
}

void WheelWidget::updateIndex() {
  const qreal position = m_offset / m_itemHeight;
  int deltaIndex = static_cast<int>(std::round(position));

  // 循环模式增强补偿
  if (m_circular) {
    // 确保跨边界补偿（如1月→12月）
    m_currentIndex =
        (m_currentIndex + deltaIndex + 2 * m_items.size()) % m_items.size();
  } else {
    m_currentIndex = qBound(0, m_currentIndex + deltaIndex, m_items.size() - 1);
  }

  m_offset = 0;
  emit currentIndexChanged(m_currentIndex);
  update();
}

// 滚轮事件：逐项滚动
// 相反方向
void WheelWidget::wheelEvent0(QWheelEvent* event) {
  int delta = event->angleDelta().y();
  setCurrentIndex((m_currentIndex + (delta > 0 ? -1 : 1) + m_items.size()) %
                  m_items.size());
}

// 一般情况下的自然方向
void WheelWidget::wheelEvent(QWheelEvent* event) {
  // 自然方向：滚轮向上 → 数值递增，向下 → 递减
  int delta = event->angleDelta().y();
  int step = (delta > 0) ? 1 : -1;  // ✅ 方向反转
  setCurrentIndex((m_currentIndex + step + m_items.size()) % m_items.size());
}

// 公共接口实现
void WheelWidget::setItems(const QStringList& items) {
  if (m_items != items) {
    m_items = items;
    m_currentIndex = 0;
    update();
  }
}

void WheelWidget::setCurrentIndex(int index) {
  if (index >= 0 && index < m_items.size() && index != m_currentIndex) {
    m_currentIndex = index;
    emit currentIndexChanged(m_currentIndex);
    update();
  }
}

QString WheelWidget::currentText() const {
  // 安全访问当前项文本
  if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
    return m_items.at(m_currentIndex);
  }
  return QString();  // 越界时返回空字符串
}

void WheelWidget::animateInertia() {
  // 非线性阻尼（速度越快，阻尼越小）
  qreal damping = 0.92 + 0.04 * (1 - qAbs(m_velocity) / (m_itemHeight * 20));
  damping = qBound(0.85, damping, 0.98);

  m_velocity *= damping;
  m_offset += m_velocity * 16 / 1000;  // 转换为每帧位移

  // 边界弹性效果（非循环模式）
  if (!m_circular) {
    const qreal overshoot = calculateOvershoot();
    if (overshoot != 0) {
      m_velocity *= 0.6;            // 边界反弹阻尼
      m_offset += overshoot * 0.3;  // 弹性回弹
    }
  }

  // 精确停止条件增强
  const bool shouldStop =
      (qAbs(m_velocity) < 0.5) ||
      (qAbs(m_velocity) < 5.0 &&
       qAbs(m_offset - (m_currentIndex * m_itemHeight)) < 1.0);

  if (shouldStop) {
    m_inertiaTimer.stop();
    m_offset = m_currentIndex * m_itemHeight;
    // 精确对齐
    update();  // 额外调用确保重绘
    QMetaObject::invokeMethod(this, &WheelWidget::updateIndex,
                              Qt::QueuedConnection);
  } else {
    update();
  }
}

qreal WheelWidget::calculateOvershoot() {
  // 计算越界量（顶部/底部）
  const qreal minOffset = -m_currentIndex * m_itemHeight;
  const qreal maxOffset = (m_items.size() - 1 - m_currentIndex) * m_itemHeight;

  if (m_offset < minOffset) return minOffset - m_offset;
  if (m_offset > maxOffset) return maxOffset - m_offset;
  return 0;
}
