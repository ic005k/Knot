#ifndef MAINHELPER_H
#define MAINHELPER_H

#include <QDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSettings>
#include <QTreeWidget>
#include <QWidget>

class MainWindow;  // 前向声明，避免头文件循环包含
class SliderButton;

class MainHelper : public QDialog {
  Q_OBJECT
 public:
  explicit MainHelper(QWidget* parent = nullptr);

  SliderButton* sliderButton;

  QString lightPCScrollbarStyle = R"(
        /* Light Vertical Scrollbar */
        QScrollBar:vertical {
            background: #F5F5F5;
            width: 22px;
            margin: 2px;
        }
        QScrollBar::handle:vertical {
            background: #C0C0C0;
            border-radius: 4px;
            border: 1px solid #D0D0D0;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #A8A8A8;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            background: transparent;
            border: none;
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: transparent;
        })";

  QString darkPCScrollbarStyle = R"(
        /* Dark Vertical Scrollbar */
        QScrollBar:vertical {
            background: #2D2D2D;
            width: 22px;
            margin: 2px;
        }
        QScrollBar::handle:vertical {
            background: #606060;
            border-radius: 4px;
            border: 1px solid #404040;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #707070;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            background: transparent;
            border: none;
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: transparent;
        })";

  void clickBtnChart();
  void clickBtnRestoreTab();
  bool mainEventFilter(QObject* watch, QEvent* evn);
  QTreeWidget* init_TreeWidget(QString name);
  void init_Menu(QMenu* mainMenu);
  void openTabRecycle();
  void initNotesQW();
  void init_UIWidget();
  void startBackgroundTaskUpdateBakFileList();

  void init_ButtonStyle();
  void delBakFile();
  void delTabRecycleFile();
  void importBakFileList();
  void init_Theme();

  void sort_childItem(QTreeWidgetItem* item);
  void on_AddRecord();

  void initMainQW();

 private:
  QStringList bakFileList;

  QString clickableLabelButtonStyle = R"(
        /* 模拟按钮样式：中蓝底+白字，适配明暗主题 */
        QLabel {
            /* 中蓝色背景（明暗主题下保持一致，突出可点击） */
            background-color: #FF9933;  /* 中蓝色 #2196F3; 安卓常用强调色 */

            /* 文字白色，与蓝色背景高对比，确保清晰 */
            color: white;

            /* 按钮质感：圆角+内边距 */
            border-radius: 4px; /* 圆角，模拟安卓按钮 */
            padding: 6px 12px; /* 上下6px，左右12px内边距，避免文字贴边 */

            /* 文字居中（按钮类元素常用） */
            /* text-align: center; */

        }

        /* 点击反馈：按下时颜色加深，模拟按钮按压感 */
        QLabel:pressed {
            background-color: #1976D2; /* 深蓝，比原色调深一级 */
        }

        /* 禁用状态（可选，需要时启用） */
        QLabel:disabled {
            background-color: #BBDEFB; /* 浅蓝灰色，区分禁用状态 */
            color: #757575; /* 灰色文字 */
        }
        )";

 signals:
};

class SliderButton : public QWidget {
  Q_OBJECT
  Q_PROPERTY(int sliderPosition READ getSliderPosition WRITE setSliderPosition)
 public:
  explicit SliderButton(QWidget* parent = nullptr) : QWidget(parent) {
    m_sliderPosition = 0;
    m_isDragging = false;
    m_animation = new QPropertyAnimation(this, "sliderPosition");
    m_animation->setDuration(200);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    m_tipText = tr("Slide Right to Start.");
  }

  void setTipText(const QString& text) {
    m_tipText = text;
    update();
  }

 signals:
  void sliderMovedToEnd();

 protected:
  void paintEvent(QPaintEvent* event) override {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    QRect backgroundRect(0, 0, width(), height());
    QBrush backgroundBrush(QColor(200, 200, 200));
    painter.setBrush(backgroundBrush);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(backgroundRect, height() / 2, height() / 2);

    // 绘制滑块
    int sliderWidth = height();
    QRect sliderRect(m_sliderPosition, 0, sliderWidth, height());
    QBrush sliderBrush(QColor(0, 128, 255));
    painter.setBrush(sliderBrush);
    painter.drawRoundedRect(sliderRect, height() / 2, height() / 2);

    // 绘制提示文本
    QFont font;
    font.setPixelSize(height() / 3);
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(backgroundRect, Qt::AlignCenter, m_tipText);
  }

  void mousePressEvent(QMouseEvent* event) override {
    if (event->button() == Qt::LeftButton) {
      int sliderWidth = height();
      QRect sliderRect(m_sliderPosition, 0, sliderWidth, height());
      if (sliderRect.contains(event->pos())) {
        m_isDragging = true;
        m_dragStartX = static_cast<int>(event->position().x());
      }
    }
  }

  void mouseMoveEvent(QMouseEvent* event) override {
    if (m_isDragging) {
      int deltaX = static_cast<int>(event->position().x()) - m_dragStartX;
      int newPosition = m_sliderPosition + deltaX;
      int maxPosition = width() - height();
      if (newPosition < 0) {
        newPosition = 0;
      } else if (newPosition > maxPosition) {
        newPosition = maxPosition;
      }
      setSliderPosition(newPosition);
      m_dragStartX = static_cast<int>(event->position().x());

      if (m_sliderPosition == maxPosition) {
        if (!isOne) {
          isOne = true;
          emit sliderMovedToEnd();
        }
      }
    }
  }

  void mouseReleaseEvent(QMouseEvent* event) override {
    Q_UNUSED(event);
    if (m_isDragging) {
      isOne = false;
      m_isDragging = false;
      int maxPosition = width() - height();
      if (m_sliderPosition == maxPosition) {
        m_animation->setStartValue(m_sliderPosition);
        m_animation->setEndValue(0);
        m_animation->start();
      } else {
        m_animation->setStartValue(m_sliderPosition);
        m_animation->setEndValue(0);
        m_animation->start();
      }
    }
  }

 public slots:
  void setSliderPosition(int position) {
    m_sliderPosition = position;
    update();
  }

  int getSliderPosition() const { return m_sliderPosition; }

 private:
  int m_sliderPosition;
  bool m_isDragging;
  int m_dragStartX;
  QPropertyAnimation* m_animation;
  QString m_tipText;
  bool isOne = false;
};

#endif  // MAINHELPER_H
