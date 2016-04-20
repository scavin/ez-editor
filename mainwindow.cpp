#include "mainwindow.h"
#include "finddialog.h"

#include <QtGui>

#ifdef _NOTICE
#include "notice.h"
#include "SystemTray.h"
#endif  //_NOTICE

#include "fileassoc.h"

//show statusBar
//replace:
//QTextCursor  tc = textedit->textCursor();
//tc.insertText(str);
//QString QTextCursor::selectedText () const

#ifdef _NOTICE
#define APP_NAME "Notice"
#else
#define APP_NAME "Editor"
#endif  //_NOTICE

#define APP_VERSION "1.4"
#define APP_BUIDE_TIME "2011/3; 2012/5-2012/7"
#define WEIBO           "www.weibo.com/huangezhao"
#define PROJECT_SITE    "code.google.com/p/ez-editor/"

const QString SUPPORT_EXTENSION("txt");

//#define INI_FILE_NAME   "Notice.ini"
//#define INI_FILE_PATH   qApp->applicationDirPath() + "/" + INI_FILE_NAME

/* QPlainTextEdit是一个高级的支持纯文本的观察器／编辑器。
 * 它被优化为能够处理大的文件以及迅速地对用户的输入进行响应。
 * QPlainText使用与QTextEdit非常接近的技术和概念，但是是针对纯文本的处理进行了优化的。
 */


MainWindow::MainWindow()
{
//    textEdit = new QTextEdit(this);
    textEdit = new QPlainTextEdit(this);
//    textEdit->setAcceptRichText(false);
    setCentralWidget(textEdit);

//    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPalette pal = textEdit->palette();
    pal.setColor(QPalette::Inactive, QPalette::HighlightedText,
                 pal.color(QPalette::Active, QPalette::HighlightedText));
    pal.setColor(QPalette::Inactive, QPalette::Highlight,
                 pal.color(QPalette::Active, QPalette::Highlight));
    textEdit->setPalette(pal);

    createActions();
    createMenus();
    createStatusBar();

#ifdef _NOTICE
    isShowAbout = false;

    createSystemTray();
    createNotice();
#endif  //_NOTICE

    setMinimumWidth(statusBar()->sizeHint().width());
    setMinimumHeight(300);
    readSettings();

    findDialog = NULL;

    /* 在默认情况下，QTextEdit可以接受来自其他应用程序文本的拖动，
     * 如果用户在它上面放下文件，它将会把这个文件的名称插入到文本中。
     * 由于拖放事件是从子窗口部件传给父窗口部件的，所以通过禁用QTextEdit的放下操作
     * 以及启用主窗口的放下操作，就可以在整个MainWindow窗口中获得放下事件。
     */
//    textEdit->setAcceptDrops (false); ///否则textEdit无法接受拖放进的文本
    setAcceptDrops (true);

    setWindowIcon(QIcon(":/icon.ico"));
    setCurrentFile("");
}

void MainWindow::about()
{
#ifdef _NOTICE
    if(isShowAbout) return;

    isShowAbout = true;
#endif  //_NOTICE
    QMessageBox::about( this, tr("About %1").arg(APP_NAME),
                       tr("<h3>%1 - A quick plain text editor</h3>"
                          "<p>Version: v%2"
                          "<br>Buide by huangezhao"
                          "<br>%3"
                          "<br>CopyRight &copy; GuangZhou</p>"
                          "<p>Contact Author:  <a href='http://%4'>%4</a>"
                          "<br>Project Home:  <a href='http://%5'>%5</a></p>")
                       .arg(APP_NAME).arg(APP_VERSION)
                       .arg(APP_BUIDE_TIME).arg(WEIBO).arg(PROJECT_SITE));

#ifdef _NOTICE
    isShowAbout = false;
#endif  //_NOTICE
}

void MainWindow::changeAssociation(bool enable)
{
    if(FileAssoc::isSupportAssociation()){
        if(enable){
            FileAssoc::setAssociation(SUPPORT_EXTENSION, tr("Text Document"));
        }else{
            FileAssoc::clearAssociation(SUPPORT_EXTENSION);
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (sureWhenAskForSave()) {
        writeSettings();

#ifdef _NOTICE
        hideToTray();
        event->ignore();
#else
        event->accept();
#endif // _NOTICE

    } else {
        event->ignore();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent (QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString fName = urls.first().toLocalFile();
    if (!fName.isEmpty() && sureWhenAskForSave()) ///
        openFile(fName);
}

bool MainWindow::sureWhenAskForSave()
{
    if(isWindowModified()){ ///
        int ret = QMessageBox::warning(this, APP_NAME,
                            tr("The document has been modified.\n"
                            "Do you want to save your changes?"),
                            QMessageBox::Save|QMessageBox::Discard
                            |QMessageBox::Cancel);//tr("Save"), tr("NoSave"), tr("Cancel"));
        if(ret == QMessageBox::Save)
            return saveFile();
        else if (ret == QMessageBox::Cancel)
            return false;
    }

    return true;
}

void MainWindow::newFile()
{
    if( sureWhenAskForSave() ){
        textEdit->clear();

        setCurrentFile("");
        emit fileCleared();
    }
}

bool MainWindow::openFileHelper(const QString &filepath)
{
    QFile file(filepath);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){

        QTextStream stream(&file);

        if(isUtf8File(&file)){
            charset = tr("UTF-8");
            stream.setCodec(charset.toAscii());
        }else{
            charset = tr("ANSI");
        }

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        textEdit->setPlainText( stream.readAll() );
        QApplication::restoreOverrideCursor();

//        statusBar()->showMessage(tr("File loaded success!"), 2000);

        setCurrentFile(filepath);
        emit fileOpened(filepath);

        return true;
    } else {
        QMessageBox::critical( this, tr("Error"),
                              tr("Open file error: %1").arg(file.errorString()) );
        return false;
    }
}

void MainWindow::openFile()
{
    if( sureWhenAskForSave() ){
        QString filename = QFileDialog::getOpenFileName(this,
                                        tr("Open a text file"), ".",  ///
                                        tr("Text File (*.txt);;All Files (*)"));

        openFile(filename);
    }
}

void MainWindow::openFile(const QString &path)
{
    if( !path.isEmpty() )
        openFileHelper(path);
}

bool MainWindow::saveFileHelper(const QString &filepath)
{
    QFile file(filepath);
    //保存时使用QIODevice::Text则自动按windows平台的回车换行格式保存。
    if(file.open(QIODevice::ReadWrite|QIODevice::Text)){

        QTextStream stream(&file);

        if(charset == tr("UTF-8"))
            stream.setCodec(charset.toAscii());

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        stream << textEdit->toPlainText();
        QApplication::restoreOverrideCursor();

//        curFile = filepath; /////
        setCurrentFile(filepath);
        statusBar()->showMessage(tr("File saved"), 2000);
        file.close();/////写回文件，否则fileUpdated会出问题
        emit fileUpdated(filepath);
        return true;
    } else {
        QMessageBox::critical( this, tr("Error"),
                              tr("Save file error: %1").arg(file.errorString()) );
        return false;
    }
}

bool MainWindow::saveFile()
{
    if (curFile.isEmpty()) {
        return saveAsFile();
    } else {
        return saveFileHelper(curFile);
    }
}

bool MainWindow::saveAsFile()
{
    QString filepath = QFileDialog::getSaveFileName(this,
                                   tr("Save the text file"), ".", ///
                                   tr("Text File (*.txt);;All Files (*)"));
    if( filepath.isEmpty() )
        return false;
    else
        return saveFileHelper(filepath);
}

void MainWindow::openRecentFile()
{
    if (sureWhenAskForSave()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if (action)
            openFileHelper(action->data().toString());
    }
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowModified(false);

    QString shownName = tr("Untitled");
    if (!curFile.isEmpty()) {
        shownName = strippedName(curFile);
        recentFiles.removeAll(curFile);
        recentFiles.prepend(curFile);
        updateRecentFileActions();
    }else{
        charset = tr("ANSI");
    }

    setWindowTitle(tr("%1[*] - %2").arg(shownName)
                                   .arg(APP_NAME));

    updateEncode(); ///
}

void MainWindow::updateRecentFileActions()
{
    QMutableStringListIterator i(recentFiles);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }

    for (int j = 0; j < MaxRecentFiles; ++j) {
        if (j < recentFiles.count()) {
            QString text = tr("&%1 %2")
                           .arg(j + 1)
                           .arg(strippedName(recentFiles[j]));
            recentFileActions[j]->setText(text);
            recentFileActions[j]->setData(recentFiles[j]);///
            recentFileActions[j]->setVisible(true);
        } else {
            recentFileActions[j]->setVisible(false);
        }
    }
    separatorAction->setVisible(!recentFiles.isEmpty());
}

void MainWindow::updateEncode()
{
    encodelaLabel->setText(tr(" %1 ").arg(charset));
}

void MainWindow::textChanged()
{
    setWindowModified(true);
    wordCountLabel->setText(tr(" Characters Count: %1 ")
                   .arg(textEdit->document()->characterCount() - 1));////
}

void MainWindow::selectionChanged()
{
    QTextCursor cur = textEdit->textCursor();
    selectCountLabel->setText(tr(" Current Selected: %1 ")
                              .arg(cur.selectionEnd() - cur.selectionStart()));
                              //.arg(cur.selectedText().size()));
}

void MainWindow::find()
{
    if (!findDialog) {
        findDialog = new FindDialog(this);
        connect(findDialog, SIGNAL(findString(QString,QTextDocument::FindFlags)),
                SLOT(findString(QString,QTextDocument::FindFlags)));

        findDialog->setText(QApplication::clipboard()->text());
    }

    QTextCursor cur = textEdit->textCursor();
    QString text = cur.selectedText();
    if(!text.isEmpty())
        findDialog->setText(text);

    findDialog->focusOnLineEdit();
    findDialog->show();
    findDialog->raise();
    findDialog->activateWindow();
}

void MainWindow::findString(const QString &str, QTextDocument::FindFlags options)
{
    if(!textEdit->find(str, options)){
        QMessageBox::information(this, APP_NAME,
                                 tr("Cannot find \"%1\"").arg(str));
    }
}

void MainWindow::addWeek()
{
    textEdit->insertPlainText( QDate::currentDate().toString( tr("dddd: ") ) );
}

void MainWindow::addDate()
{
    textEdit->insertPlainText( QDate::currentDate().toString( tr("M/d: ") ) );
}

void MainWindow::addTime()
{
    textEdit->insertPlainText( QTime::currentTime().toString ( tr("hh:mm: ") ) );
}

void MainWindow::setAutoWrap(bool wrap)
{
    if(wrap){
        textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        wrapAct->setChecked(true);
    } else {
        textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
        wrapAct->setChecked(false);
    }
}

void MainWindow::selectFont()
{
    bool ok;
    QFont font = QFontDialog::getFont( &ok, textEdit->font(), this);
    if (ok)
        textEdit->setFont(font);
}



void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
#ifdef _NOTICE
    //触发后台图标执行相应事件
    switch (reason)
    {
        case QSystemTrayIcon::Trigger:          //鼠标单击
            if( systemTray->isNoticing() ){
                systemTray->stopNotice();
                showMainWidget();
                return;
            }

            isHidden() ? showMainWidget() : hideToTray();
            break;
        //case QSystemTrayIcon::DoubleClick:    //鼠标双击
        //case QSystemTrayIcon::MiddleClick:    //鼠标中键
        default:
            break;
    }
#endif // _NOTICE
}

void MainWindow::hideToTray()
{
#ifdef _NOTICE
    if(isShowAbout) return;

    hide();
#endif // _NOTICE
}

void MainWindow::showMainWidget()
{
#ifdef _NOTICE
    show();//showNormal();   //show();
    raise();
    activateWindow();
#endif // _NOTICE
}


void MainWindow::createActions()
{
    newAct = new QAction(tr("&New"),this);
    //newAct->setIcon(QIcon(":icon.png"));
    newAct->setShortcut(QKeySequence::New);  //newAct->setShortcut(tr("Ctrl+N"));
    newAct->setStatusTip(tr("Create a new text file"));
    connect(newAct,SIGNAL(triggered()),this,SLOT(newFile()));

    openAct = new QAction(tr("&Open"),this);
    openAct->setShortcut(QKeySequence::Open);       //"Ctrl+O"
    openAct->setStatusTip(tr("Open an existing text file"));
    connect(openAct,SIGNAL(triggered()),this,SLOT(openFile()));

    saveAct = new QAction(tr("&Save"),this);
    saveAct->setShortcut(QKeySequence::Save);       //"Ctrl+S"
    saveAct->setStatusTip(tr("Save the text to disk"));
    connect(saveAct,SIGNAL(triggered()),this,SLOT(saveFile()));

    saveAsAct = new QAction(tr("Save &As..."),this);
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the text under a new name"));
    connect(saveAsAct,SIGNAL(triggered()),this,SLOT(saveAsFile()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    exitAct = new QAction(tr("E&xit"),this);
    exitAct->setShortcut(tr("Ctrl+Q"));   //QKeySequence::Quit
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct,SIGNAL(triggered()),this,SLOT(close()));    ////

    undoAct = new QAction(tr("&Undo"),this);
    undoAct->setShortcut(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undoes the last operation"));
    connect(undoAct,SIGNAL(triggered()),textEdit,SLOT(undo()));
    undoAct->setEnabled(false);
    connect(textEdit, SIGNAL(undoAvailable(bool)),
            undoAct, SLOT(setEnabled(bool)));    ///

    redoAct = new QAction(tr("&Redo"),this);
    redoAct->setShortcut(QKeySequence::Redo);
    redoAct->setStatusTip(tr("Redoes the last operation"));
    connect(redoAct,SIGNAL(triggered()),textEdit,SLOT(redo()));
    redoAct->setEnabled(false);
    connect(textEdit, SIGNAL(redoAvailable(bool)),
            redoAct, SLOT(setEnabled(bool)));    ///

    cutAct = new QAction(tr("Cu&t"), this);
    cutAct->setShortcut(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents "
                               "to the clipboard"));
    connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));
    cutAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));    ///

    copyAct = new QAction(tr("&Copy"), this);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents "
                                "to the clipboard"));
    connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));

    pasteAct = new QAction(tr("&Paste"), this);
    pasteAct->setShortcut(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into "
                                 "the current selection"));
    connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));

    selectAllAct = new QAction(tr("Select &All"), this);
    selectAllAct->setShortcut(QKeySequence::SelectAll);
    selectAllAct->setStatusTip(tr("Select all the text"));
    connect(selectAllAct, SIGNAL(triggered()), textEdit, SLOT(selectAll()));

    findAct = new QAction(tr("&Find..."), this);
    findAct->setShortcut(QKeySequence::Find);
    findAct->setStatusTip(tr("Find a matching text"));
    connect(findAct, SIGNAL(triggered()), this, SLOT(find()));

    weekAct = new QAction(tr("&Week"),this);
    weekAct->setStatusTip(tr("Insert a sting of current week"));
    connect(weekAct,SIGNAL(triggered()),this,SLOT(addWeek()));

    dateAct = new QAction(tr("&Date"),this);
    dateAct->setStatusTip(tr("Insert a sting of current date"));
    connect(dateAct,SIGNAL(triggered()),this,SLOT(addDate()));

    timeAct = new QAction(tr("&Time"),this);
    timeAct->setStatusTip(tr("Insert a sting of current time"));
    connect(timeAct,SIGNAL(triggered()),this,SLOT(addTime()));

    wrapAct = new QAction(tr("&Wrap"),this);
    wrapAct->setStatusTip(tr("Setting wrap text by words or not"));
    wrapAct->setCheckable(true);
    wrapAct->setChecked(true);
    connect(wrapAct,SIGNAL(toggled(bool)),this,SLOT(setAutoWrap(bool)));

    fontAct = new QAction(tr("&Font..."),this);
    fontAct->setStatusTip(tr("Change the font of this editor"));
    connect(fontAct,SIGNAL(triggered()),this,SLOT(selectFont()));

    if(FileAssoc::isSupportAssociation()){
        txtAct = new QAction(tr("Associate txt extension"),this);
        txtAct->setStatusTip(tr("Associate txt extension with this editor"));
        txtAct->setCheckable(true);
        txtAct->setChecked(FileAssoc::checkAssociation(SUPPORT_EXTENSION));// before connect(). otherwise it will launch the function changeAssociation(bool).
        connect(txtAct,SIGNAL(toggled(bool)),SLOT(changeAssociation(bool)));
    }

    aboutAct = new QAction(tr("&About"),this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct,SIGNAL(triggered()),this,SLOT(about()));

//    aboutQtAction = new QAction(tr("About &Qt"), this);
//    aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
//    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    separatorAction = fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(recentFileActions[i]);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addSeparator();
    editMenu->addAction(findAct);
    editMenu->addSeparator();
    editMenu->addAction(selectAllAct);
    editMenu->addSeparator();
    editMenu->addAction(weekAct);
    editMenu->addAction(dateAct);
    editMenu->addAction(timeAct);

    QMenu *settingMenu = menuBar()->addMenu(tr("&Setting"));
    settingMenu->addAction(wrapAct);
    settingMenu->addAction(fontAct);
    if(FileAssoc::isSupportAssociation()){
        settingMenu->addSeparator();
        settingMenu->addAction(txtAct);
    }

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}

//void MainWindow::createContextMenu()
//{
//    textEdit->addAction(cutAct);
//    textEdit->addAction(copyAct);
//    textEdit->addAction(pasteAct);
//    textEdit->setContextMenuPolicy(Qt::ActionsContextMenu);
//}

//void MainWindow::createToolBars()
//{
//    fileToolBar = addToolBar(tr("&File"));
//    fileToolBar->addAction(newAct);
//    fileToolBar->addAction(openAct);
//    fileToolBar->addAction(saveAct);

//    editToolBar = addToolBar(tr("&Edit"));
//    editToolBar->addAction(cutAct);
//    editToolBar->addAction(copyAct);
//    editToolBar->addAction(pasteAct);
//    editToolBar->addSeparator();
//    editToolBar->addAction(findAct);
//    editToolBar->addAction(goToCellAct);
//}

void MainWindow::createStatusBar()
{
    wordCountLabel = new QLabel(tr(" Characters Count: 0 "));
    wordCountLabel->setAlignment(Qt::AlignHCenter);
    wordCountLabel->setMinimumSize(wordCountLabel->sizeHint());

    selectCountLabel = new QLabel(tr(" Current Selected: 0 "));
    selectCountLabel->setAlignment(Qt::AlignHCenter);
    selectCountLabel->setMinimumSize(selectCountLabel->sizeHint());
//    selectCountLabel->setIndent(3);

    encodelaLabel = new QLabel(tr(" ANSI "));
    encodelaLabel->setAlignment(Qt::AlignRight);
    encodelaLabel->setMinimumSize(encodelaLabel->sizeHint());
    encodelaLabel->setIndent(6);

    statusBar()->addWidget(new QLabel, 1);  //
    statusBar()->addWidget(wordCountLabel);
    statusBar()->addWidget(selectCountLabel);
    statusBar()->addWidget(encodelaLabel);

    connect(textEdit,SIGNAL(textChanged()), SLOT(textChanged()));
    connect(textEdit, SIGNAL(selectionChanged()), SLOT(selectionChanged()));
}


#ifdef _NOTICE
void MainWindow::createSystemTray()
{
    //创建系统托盘图标
    if( QSystemTrayIcon::isSystemTrayAvailable() ){
        systemTray = new SystemTray(tr("Event Notice by yuezhao"), 0);///

        showAct = new QAction(tr("&Show main window"), this);
        connect(showAct,SIGNAL(triggered()),this,SLOT(showMainWidget()));
        hideAct = new QAction(tr("&Hide main window"), this);
        connect(hideAct,SIGNAL(triggered()),this,SLOT(hideToTray()));
        QAction *exit2Act = new QAction(tr("E&xit"), this);
        connect(exit2Act,SIGNAL(triggered()),qApp,SLOT(quit()));

        QMenu *trayIconMenu = new QMenu(QApplication::desktop());
        trayIconMenu->addAction(showAct);
        trayIconMenu->addAction(hideAct);
        trayIconMenu->addSeparator();
//        trayIconMenu->addAction(aboutAct);//当主界面隐藏到托盘区的时候选中这项，确定后程序会退出。
//        trayIconMenu->addSeparator();
        trayIconMenu->addAction(exit2Act);
        systemTray->setContextMenu(trayIconMenu);

        connect(systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                    this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    }else
        systemTray = NULL;
}

void MainWindow::createNotice()
{
    notice = new Notice(this);
    connect(this, SIGNAL(fileOpened(const QString &)),
                notice, SLOT(analysisText(const QString &)));
    connect(this, SIGNAL(fileUpdated(const QString &)),
                notice, SLOT(analysisText(const QString &)));
    connect(this, SIGNAL(fileCleared()), notice, SLOT(stopTimer()));

    if(systemTray != NULL){
        connect(notice, SIGNAL(toNotice(const QString &)),
                systemTray, SLOT(notice(const QString &)));
        connect(notice, SIGNAL(stopNotice()),
                systemTray, SLOT(stopNotice()));  ////
    }
}
#endif  //_NOTICE


void MainWindow::readSettings()
{
    QSettings settings("Software Inc.", APP_NAME);
//    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
    restoreGeometry(settings.value("geometry").toByteArray());

//    recentFiles = settings.value("recentFiles").toStringList();
//    updateRecentFileActions();

    bool autoWrap = settings.value("autoWrap", true).toBool();
    setAutoWrap(autoWrap);

    QFont font;
    if(font.fromString(settings.value("font").toString()))
        textEdit->setFont( font );
}

void MainWindow::writeSettings()
{
    QSettings settings("Software Inc.", APP_NAME);
//    QSettings settings(INI_FILE_PATH, QSettings::IniFormat);
    settings.setValue("geometry", saveGeometry());
//    settings.setValue("recentFiles", recentFiles);
    settings.setValue("autoWrap", wrapAct->isChecked());
    settings.setValue("font", textEdit->font().toString());
}




bool MainWindow::isUtf8File(QIODevice *file)
{
    const int testSize = 1024;
    char str[testSize];///
    int size = file->peek(str, testSize);

//    char buf[3];
//    if (f->peek(buf, sizeof(buf)) == sizeof(buf))
//        return (buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF);

    int encodingBytesCount = 0;
    bool allTextsAreASCIIChars = true;

    for (int i = 0; i < size; ++i){
        char current = str[i];

        if ((current & 0x80) == 0x80)
            allTextsAreASCIIChars = false;

        // First byte
        if (encodingBytesCount == 0){
            if ((current & 0x80) == 0)
                continue;// ASCII chars, from 0x00-0x7F

            if ((current & 0xC0) == 0xC0){
                encodingBytesCount = 1;
                current <<= 2;
                // More than two bytes used to encoding a unicode char.
                // Calculate the real length.
                while ((current & 0x80) == 0x80){
                    current <<= 1;
                    ++encodingBytesCount;
                }
            }else{
                // Invalid bits structure for UTF8 encoding rule.
                return false;
            }
        }else{
            // Following bytes, must start with 10.
            if ((current & 0xC0) == 0x80)
                --encodingBytesCount;
            else
                return false;
        }
    }

//    if(encodingBytesCount != 0)
//    {
//        // Invalid bits structure for UTF8 encoding rule.
//        // Wrong following bytes count.
//        return false;
//    }

    // Although UTF8 supports encoding for ASCII chars, we regard as a input stream, whose contents are all ASCII as default encoding.
    return !allTextsAreASCIIChars;
}
