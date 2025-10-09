#include <QDebug>
#include <QFile>
#include <QObject>
#include <QSettings>

#include "MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"

#ifdef Q_OS_ANDROID
static void JavaNotify_0();
static void JavaNotify_1();
static void JavaNotify_2();
static void JavaNotify_3();
static void JavaNotify_4();
static void JavaNotify_5();
static void JavaNotify_6();
static void JavaNotify_7();
static void JavaNotify_8();
static void JavaNotify_9();
static void JavaNotify_10();
static void JavaNotify_11();
static void JavaNotify_12();
static void JavaNotify_13();
static void JavaNotify_14();
static void JavaNotify_15();
static void JavaNotify_16();
static void JavaNotify_17();
#endif

#ifdef Q_OS_ANDROID
static void JavaNotify_0() {
  // onResume

  if (mw_one->initMain) return;

  if (mw_one->m_Steps->isNeedRestoreUI) {
    mui->btnSteps->click();
  }

  qDebug() << "C++ JavaNotify_0";
}

static void JavaNotify_1() {
  // Lock screen or onPause

  if (mw_one->initMain) return;

  if (!mui->frameSteps->isHidden()) {
    mui->btnBackSteps->click();
    mw_one->m_Steps->isNeedRestoreUI = true;
  }

  qDebug() << "C++ JavaNotify_1";
}

static void JavaNotify_2() {
  // When the screen lights up.
  mw_one->m_Todo->refreshAlarm();
  mw_one->m_Steps->updateHardSensorSteps();

  qDebug() << "C++ JavaNotify_2";
}

static void JavaNotify_3() {
  mw_one->alertWindowsCount++;
  mw_one->m_Todo->refreshAlarm();

  qDebug() << "C++ JavaNotify_3";
}

static void JavaNotify_4() {
  mw_one->alertWindowsCount--;

  if (mw_one->alertWindowsCount == 0) {
    bool isBackMain = false;
    QJniObject activity =
        QJniObject(QNativeInterface::QAndroidApplication::context());
    if (activity.isValid()) {
      jboolean result = activity.callMethod<jboolean>("getIsBackMainUI", "()Z");
      activity.callMethod<void>("setIsBackMainUI", "(Z)V", false);
      isBackMain = result;
    }

    if (!isBackMain) {
      mw_one->setMini();
    }
  }

  qDebug() << "alertWindowsCount=" << mw_one->alertWindowsCount;
  qDebug() << "C++ JavaNotify_4";
}

static void JavaNotify_5() {
  mw_one->m_ReceiveShare->goReceiveShare();

  qDebug() << "C++ JavaNotify_5";
}

static void JavaNotify_6() {
  m_Notes->javaNoteToQMLNote();

  qDebug() << "C++ JavaNotify_6";
}

static void JavaNotify_7() {
  m_Notes->insertImage(privateDir + "receive_share_pic.png", true);

  qDebug() << "C++ JavaNotify_7";
}

static void JavaNotify_8() {
  if (isInitThemeEnd) {
    if (mw_one->m_Steps->isNeedRestoreUI) {
      QTimer::singleShot(1000, mw_one, []() { mw_one->execDeskShortcut(); });

    } else
      mw_one->execDeskShortcut();

  } else {
    isNeedExecDeskShortcut = true;
  }

  qDebug() << "C++ JavaNotify_8";
}

static void JavaNotify_9() {
  mw_one->m_ReceiveShare->callJavaNotify9();

  qDebug() << "C++ JavaNotify_9";
}

static void JavaNotify_10() {
  // Open Book
  mw_one->on_btnOpen_clicked();

  qDebug() << "C++ JavaNotify_10";
}

static void JavaNotify_11() {
  // Books List
  mui->btnReadList->click();

  qDebug() << "C++ JavaNotify_11";
}

static void JavaNotify_12() {
  if (isPDF && isAndroid) mw_one->m_Reader->openMyPDF(fileName);

  qDebug() << "C++ JavaNotify_12";
}

static void JavaNotify_13() {
  mw_one->m_Reader->openMyPDF(fileName);

  qDebug() << "C++ JavaNotify_13";
}

static void JavaNotify_14() {
  if (m_Method->getDateTimeFlag() == "todo") {
    mw_one->m_TodoAlarm->setDateTime();
  } else if (m_Method->getDateTimeFlag() == "gpslist") {
    mui->btnGetGpsListData->click();
  } else {
    mw_one->m_DateSelector->ui->btnOk->click();
  }
  qDebug() << "C++ JavaNotify_14";
}

static void JavaNotify_15() {
  qDebug() << "C++ JavaNotify_15";

  if (colorDlg != nullptr) {
    if (colorDlg->isVisible()) {
      colorDlg->close();
      return;
    }
  }

  if (m_Method->m_EnColorPicker != nullptr) {
    if (m_Method->m_EnColorPicker->isVisible()) {
      m_Method->m_EnColorPicker->close();
      return;
    }
  }

  if (m_PrintPDF != nullptr) {
    if (m_PrintPDF->isVisible()) {
      m_PrintPDF->close();
      return;
    }
  }

  if (mw_one->mainMenu != nullptr) {
    if (mw_one->mainMenu->isVisible()) {
      mw_one->mainMenu->close();
      return;
    }
  }

  if (mw_one->m_Report->m_Menu != nullptr) {
    if (mw_one->m_Report->m_Menu->isVisible()) {
      mw_one->m_Report->m_Menu->close();
      return;
    }
  }

  if (m_Method->menuNoteBook != nullptr) {
    if (m_Method->menuNoteBook->isVisible()) {
      m_Method->menuNoteBook->close();
      return;
    }
  }

  if (m_Method->menuNoteList != nullptr) {
    if (m_Method->menuNoteList->isVisible()) {
      m_Method->menuNoteList->close();
      return;
    }
  }

  if (m_NotesList->menuRecentOpen != nullptr) {
    if (m_NotesList->menuRecentOpen->isVisible()) {
      m_NotesList->menuRecentOpen->close();
      return;
    }
  }

  if (m_NotesList->m_MoveTo != nullptr) {
    if (m_NotesList->m_MoveTo->isVisible()) {
      m_NotesList->m_MoveTo->ui->btnCancel->click();
      return;
    }
  }

  if (m_NotesList->m_NewNoteBook != nullptr) {
    if (m_NotesList->m_NewNoteBook->isVisible()) {
      m_NotesList->m_NewNoteBook->ui->btnCancel->click();
      return;
    }
  }

  if (mw_one->textToolbar != nullptr) {
    if (mw_one->textToolbar->isVisible()) {
      mw_one->textToolbar->hide();
      return;
    }
  }

  if (textToolbar != nullptr) {
    if (textToolbar->isVisible()) {
      textToolbar->hide();
      return;
    }
  }

  if (mw_one->m_Preferences->textToolbarPreferences != nullptr) {
    if (mw_one->m_Preferences->textToolbarPreferences->isVisible()) {
      mw_one->m_Preferences->textToolbarPreferences->hide();
      return;
    }
  }

  if (m_ShowMessage != nullptr) {
    if (m_ShowMessage->isVisible()) {
      m_ShowMessage->close();
      return;
    }
  }

  if (mw_one->m_RenameDlg != nullptr) {
    if (mw_one->m_RenameDlg->isVisible()) {
      mw_one->m_RenameDlg->close();
      return;
    }
  }

  if (mw_one->m_Todo->textToolbarReeditTodo != nullptr) {
    if (mw_one->m_Todo->textToolbarReeditTodo->isVisible()) {
      mw_one->m_Todo->textToolbarReeditTodo->hide();
      return;
    }
  }

  if (mw_one->m_Todo->m_ReeditTodo != nullptr) {
    if (mw_one->m_Todo->m_ReeditTodo->isVisible()) {
      mw_one->m_Todo->m_ReeditTodo->close();
      return;
    }
  }

  if (m_NotesList->textToolbarRenameNotes != nullptr) {
    if (m_NotesList->textToolbarRenameNotes->isVisible()) {
      m_NotesList->textToolbarRenameNotes->hide();
      return;
    }
  }

  if (m_NotesList->m_RenameNotes != nullptr) {
    if (m_NotesList->m_RenameNotes->isVisible()) {
      m_NotesList->m_RenameNotes->close();
      return;
    }
  }

  if (mw_one->m_Preferences->isVisible()) {
    mw_one->m_Preferences->ui->btnBack->click();
    return;
  }

  if (mw_one->m_AboutThis->isVisible()) {
    mw_one->m_AboutThis->ui->btnBack_About->click();
    return;
  }

  if (mw_one->m_StepsOptions->isVisible()) {
    mw_one->m_StepsOptions->ui->btnBack->click();
    return;
  }

  if (mw_one->m_Reader->dlgAddBookNote != nullptr) {
    if (mw_one->m_Reader->dlgAddBookNote->isVisible()) {
      mw_one->m_Reader->dlgAddBookNote->close();
      return;
    }
  }

  if (mw_one->m_Reader->dlgEditBookNote != nullptr) {
    if (mw_one->m_Reader->dlgEditBookNote->isVisible()) {
      mw_one->m_Reader->dlgEditBookNote->close();
      return;
    }
  }

  if (mui->qwViewBookNote->isVisible()) {
    QTimer::singleShot(100, mw_one,
                       []() { mw_one->m_Reader->closeViewBookNote(); });
    return;
  }

  if (mui->f_ReaderNote->isVisible()) {
    mui->btnCancelSel->click();
    return;
  }

  if (mui->f_ReaderSet->isVisible()) {
    mui->btnBackReaderSet->click();
    return;
  }

  if (!mui->frameReader->isHidden()) {
    if (mui->qwCata->isVisible()) {
      mui->btnCatalogue->click();
      return;

    } else if (mui->qwBookmark->isVisible()) {
      mui->btnShowBookmark->click();
      return;

    } else if (!mw_one->mydlgSetText->isHidden()) {
      mw_one->mydlgSetText->close();
      return;

    } else if (!mui->textBrowser->isHidden()) {
      QTimer::singleShot(100, mw_one,
                         []() { mw_one->on_btnSelText_clicked(); });
      return;
    }

    else {
      mui->btnBackReader->click();
      return;
    }
  }

  if (!mui->frameImgView->isHidden()) {
    mui->btnBackImg->click();
    return;
  }

  if (!mui->frameMain->isHidden()) {
    if (!mui->f_charts->isHidden()) {
      mui->btnChart->click();
      return;
    }

    mw_one->setMini();

    return;
  }

  if (!mui->frameOne->isHidden()) {
    mui->btnBack_One->click();
    return;
  }

  if (!mui->frameNoteRecycle->isHidden()) {
    mui->btnBackNoteRecycle->click();
    return;
  }

  if (!mui->frameNotesSearchResult->isHidden()) {
    mui->btnBack_NotesSearchResult->click();
    return;
  }

  if (mui->f_FindNotes->isVisible()) {
    mui->f_FindNotes->hide();
    return;
  }

  if (!mui->frameNoteList->isHidden()) {
    mui->btnBackNoteList->click();
    return;
  }

  if (!mui->frameDiff->isHidden()) {
    mui->btnBackNoteDiff->click();
    return;
  }

  if (!mui->frameNotesGraph->isHidden()) {
    mui->btnBackNotesGraph->click();
    return;
  }

  if (mw_one->m_TodoAlarm->isVisible()) {
    mw_one->m_TodoAlarm->ui->btnBack->click();
    return;
  }

  if (!mui->frameTodo->isHidden()) {
    mui->btnBackTodo->click();
    return;
  }

  if (!mui->frameTodoRecycle->isHidden()) {
    mui->btnReturnRecycle->click();
    return;
  }

  if (!mui->frameTabRecycle->isHidden()) {
    mui->btnBackTabRecycle->click();
    return;
  }

  if (!mui->frameSteps->isHidden()) {
    mui->btnBackSteps->click();
    return;
  }

  if (!mui->frameViewCate->isHidden()) {
    QTimer::singleShot(100, mw_one, []() {
      mui->frameViewCate->hide();
      mui->frameReport->show();
    });

    return;
  }

  if (!mui->frameReport->isHidden()) {
    mui->btnBack_Report->click();
    return;
  }

  if (!mui->frameSearch->isHidden()) {
    mui->btnBackSearch->click();
    return;
  }

  if (!mui->frameBakList->isHidden()) {
    mui->btnBackBakList->click();
    return;
  }

  if (!mui->frameCategory->isHidden()) {
    mui->btnCancelType->click();
    return;
  }

  if (!mui->frameSetTab->isHidden()) {
    mui->btnBackSetTab->click();
    return;
  }

  if (!mui->frameEditRecord->isHidden()) {
    mui->btnBackEditRecord->click();

    return;
  }

  if (!mui->frameBookList->isHidden()) {
    mui->btnBackBookList->click();
    return;
  }

  if (!mui->frameNotesTree->isHidden()) {
    mui->btnBack_Tree->click();
    return;
  }
}

static void JavaNotify_16() {
  mui->btnOpenNote->click();

  qDebug() << "C++ JavaNotify_16";
}

static void JavaNotify_17() {
  QTimer::singleShot(100, mw_one, []() { m_NotesList->clickNoteList(); });
  qDebug() << "C++ JavaNotify_17";
}

static const JNINativeMethod gMethods[] = {
    {"CallJavaNotify_0", "()V", (void *)JavaNotify_0},
    {"CallJavaNotify_1", "()V", (void *)JavaNotify_1},
    {"CallJavaNotify_2", "()V", (void *)JavaNotify_2},
    {"CallJavaNotify_3", "()V", (void *)JavaNotify_3},
    {"CallJavaNotify_4", "()V", (void *)JavaNotify_4},
    {"CallJavaNotify_5", "()V", (void *)JavaNotify_5},
    {"CallJavaNotify_6", "()V", (void *)JavaNotify_6},
    {"CallJavaNotify_7", "()V", (void *)JavaNotify_7},
    {"CallJavaNotify_8", "()V", (void *)JavaNotify_8},
    {"CallJavaNotify_9", "()V", (void *)JavaNotify_9},
    {"CallJavaNotify_10", "()V", (void *)JavaNotify_10},
    {"CallJavaNotify_11", "()V", (void *)JavaNotify_11},
    {"CallJavaNotify_12", "()V", (void *)JavaNotify_12},
    {"CallJavaNotify_13", "()V", (void *)JavaNotify_13},
    {"CallJavaNotify_14", "()V", (void *)JavaNotify_14}

};

static const JNINativeMethod gMethods15[] = {
    {"CallJavaNotify_15", "()V", (void *)JavaNotify_15}};

static const JNINativeMethod gMethods16[] = {
    {"CallJavaNotify_16", "()V", (void *)JavaNotify_16}};

static const JNINativeMethod gMethods17[] = {
    {"CallJavaNotify_17", "()V", (void *)JavaNotify_17}};

void RegJni(const char *myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char *mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethods, sizeof(gMethods) / sizeof(gMethods[0]));
    if (mj != JNI_OK) {
      qDebug() << "register native method failed!";
      return;
    } else {
      qDebug() << "RegisterNatives success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

void RegJni15(const char *myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char *mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethods15, sizeof(gMethods15) / sizeof(gMethods15[0]));
    if (mj != JNI_OK) {
      qDebug() << "register native method failed!";
      return;
    } else {
      qDebug() << "RegisterNatives15 success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

void RegJni16(const char *myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char *mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethods16, sizeof(gMethods16) / sizeof(gMethods16[0]));
    if (mj != JNI_OK) {
      qDebug() << "register native method failed!";
      return;
    } else {
      qDebug() << "RegisterNatives16 success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

void RegJni17(const char *myClassName) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([=]() {
    QJniEnvironment Environment;
    const char *mClassName = myClassName;
    jclass j_class;
    j_class = Environment->FindClass(mClassName);
    if (j_class == nullptr) {
      qDebug() << "erro clazz";
      return;
    }
    jint mj = Environment->RegisterNatives(
        j_class, gMethods17, sizeof(gMethods17) / sizeof(gMethods17[0]));
    if (mj != JNI_OK) {
      qDebug() << "register native method failed!";
      return;
    } else {
      qDebug() << "RegisterNatives17 success!";
    }
  });
  qDebug() << "++++++++++++++++++++++++";
}

#endif
