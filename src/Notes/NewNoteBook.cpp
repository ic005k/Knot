#include "NewNoteBook.h"

#include "src/MainWindow.h"
#include "src/defines.h"
#include "ui_NewNoteBook.h"

NewNoteBook::NewNoteBook(QWidget* parent)
    : QDialog(parent), ui(new Ui::NewNoteBook) {
  ui->setupUi(this);
  setWindowFlag(Qt::FramelessWindowHint);
  ui->editName->installEventFilter(this);
  m_Method->set_ToolButtonStyle(this);

  QScroller::grabGesture(ui->listWidget, QScroller::LeftMouseButtonGesture);
  m_Method->setSCrollPro(ui->listWidget);

  QStringList list;
  list.append(tr("Main Root"));
  int count = m_NotesList->ui->treeWidget->topLevelItemCount();
  for (int i = 0; i < count; i++) {
    list.append(
        m_NotesList->ui->treeWidget->topLevelItem(i)->text(0).trimmed());
  }
  ui->listWidget->addItems(list);
  ui->listWidget->setCurrentRow(0);
}

NewNoteBook::~NewNoteBook() { delete ui; }

void NewNoteBook::showDialog() {
  int x, y, w, h;
  w = mw_one->width() - 20;
  h = mw_one->height() - 50;
  x = mw_one->geometry().x() + (mw_one->width() - w) / 2;
  y = mw_one->geometry().y() + (mw_one->height() - h) / 2;
  setGeometry(x, y, w, h);

  ui->editName->setFocus();

  // init edit toolbar
  initTextToolbarDynamic(this);
  if (editFilter != nullptr) {
    ui->editName->removeEventFilter(editFilter);
    delete editFilter;
    editFilter = nullptr;
  }
  editFilter = new EditEventFilter(textToolbarDynamic, this);
  ui->editName->installEventFilter(editFilter);

  m_Method->showGrayWindows();
  show();
  while (!isHidden()) QCoreApplication::processEvents();
}

bool NewNoteBook::eventFilter(QObject* watch, QEvent* evn) {
  if (evn->type() == QEvent::KeyPress) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnCancel_clicked();
      return true;
    }

    if (watch == ui->editName) {
      if (keyEvent->key() == Qt::Key_Return) {
        on_btnOk_clicked();
        return true;
      }
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void NewNoteBook::closeEvent(QCloseEvent* event) {
  Q_UNUSED(event)
  closeTextToolBar();
  m_Method->closeGrayWindows();
}

void NewNoteBook::on_btnCancel_clicked() {
  isOk = false;
  rootIndex = 0;
  close();
}

void NewNoteBook::on_btnOk_clicked() {
  isOk = true;
  rootIndex = 0;  // ui->listWidget->currentRow();
  notebookName = ui->editName->text().trimmed();
  notebookRoot = ui->listWidget->currentItem()->text().trimmed();
  close();
}
