#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets>
#include <QTreeView>

#include "iostream"
#include <QStandardItemModel>
#include <QList>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <algorithm>
#include <stdio.h>
#include "WordCollection.h"
#include "Word.h"
#include "StemCollection.h"
#include "Stem.h"
#include "SuffixCollection.h"
#include "Suffix.h"
#include "SignatureCollection.h"
#include "Signature.h"
#include <QKeyEvent>
#include <QtWidgets>
#include <QString>
#include <QDebug>
#include <QPair>

#include <QDir>

#include <QSplitter>
#include <QTableView>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QMapIterator>

typedef  QMap<QString,CWord*> StringToWordPtr;

MainWindow::MainWindow()
    : m_tableView_upper(new UpperTableView),  m_tableView_lower(new LowerTableView)
{
//    Lexicon = new CLexicon();
    m_lexicon_list.append ( new CLexicon() );
    m_treeModel = new QStandardItemModel();
    m_tableView_lower->set_parent(this);

    Word_model= new QStandardItemModel(1,3);
    Stem_model= new QStandardItemModel;
    Signature_model= new QStandardItemModel;
    Affix_model= new QStandardItemModel();

    createSplitter();
    setCentralWidget(m_mainSplitter);
    createActions();
    createStatusBar();

    readSettings();


#ifndef QT_NO_SESSIONMANAGER
    QGuiApplication::setFallbackSessionManagementEnabled(false);
    connect(qApp, &QGuiApplication::commitDataRequest,
            this, &MainWindow::commitData);
#endif

    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);

    connect(m_tableView_upper,SIGNAL(clicked(const QModelIndex & )), m_tableView_lower,SLOT(display_this_signature(const QModelIndex &  )));

}

void MainWindow::createSplitter()
{
    m_mainSplitter = new QSplitter();

    m_rightSplitter = new QSplitter(Qt::Vertical);
    m_rightSplitter->addWidget(m_tableView_upper);
    m_rightSplitter->addWidget(m_tableView_lower);

    m_leftTreeView = new LeftSideTreeView(this);
    m_leftTreeView->setModel(m_treeModel);

    m_mainSplitter->addWidget(m_leftTreeView);
    m_mainSplitter->addWidget(m_rightSplitter);

    connect(m_leftTreeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(rowClicked(const QModelIndex&)));
}






// not used: delete...
void MainWindow::createHorizontalGroupBox()
{
    horizontalGroupBox = new QGroupBox(tr("Horizontal layout"));
    QHBoxLayout *layout = new QHBoxLayout;

    verticalGroupBox = new QGroupBox(tr("Vertical layout"));
    QVBoxLayout *vLayout = new QVBoxLayout;

    verticalGroupBox->setLayout(vLayout);

    QTreeView* tree = new QTreeView(horizontalGroupBox);
    tree->header()->resizeSection(0, 100);
    layout->addWidget(tree);
    layout->addWidget(verticalGroupBox);
    horizontalGroupBox->setLayout(layout);
}



void MainWindow::load_word_model()
{

    StringToWordPtr * WordMap = get_lexicon()->GetWordCollection()->GetMap();
    QMapIterator<QString,CWord*> word_iter(*WordMap);
    while (word_iter.hasNext())
    {   CWord* pWord = word_iter.next().value();
        QList<QStandardItem*> item_list;

        QStandardItem* pItem = new QStandardItem(pWord->GetWord());
        item_list.append(pItem);

        QStandardItem* pItem2 = new QStandardItem(QString::number(pWord->GetWordCount()));
        item_list.append(pItem2);

        QListIterator<CSignature*> sig_iter(*pWord->GetSignatures());
        while (sig_iter.hasNext()){
            QStandardItem* pItem3 = new QStandardItem(sig_iter.next()->GetSignature());
            item_list.append(pItem3);
        }

        Word_model->appendRow(item_list);
    }
}
void MainWindow::load_stem_model()
{
    CStem* stem;
    QMap<QString, CStem*>::iterator iter;

    for (iter = get_lexicon()->GetStemCollection()->GetBegin(); iter != get_lexicon()->GetStemCollection()->GetEnd(); ++iter)
    {
        stem = iter.value();
        QStandardItem *item = new QStandardItem(stem->get_key());
        QList<QStandardItem*> item_list;
        item_list.append(item);
        QListIterator<QString> sig_iter(*stem->GetSignatures());
        while (sig_iter.hasNext()){
           QString sig = sig_iter.next();
           QStandardItem *item = new QStandardItem(sig);
           item_list.append(item);
        }
        Stem_model->appendRow(item_list);
    }
}
void MainWindow::load_affix_model()
{
     QMapIterator<QString, CSuffix*> suffix_iter(get_lexicon()->GetSuffixCollection()->GetMap());
    while (suffix_iter.hasNext())
    {
        CSuffix* pSuffix = suffix_iter.next().value();
        QStandardItem *item = new QStandardItem(pSuffix->GetSuffix());
        QList<QStandardItem*> item_list;
        item_list.append(item);
        Affix_model->appendRow(item_list);
    }
}

void MainWindow::load_signature_model()
{   CSignature* sig;
    get_lexicon()->get_signatures()->sort();
    QList<CSignature*>* pSortedSignatures = get_lexicon()->get_signatures()->GetSortedSignatures();
    QListIterator<CSignature*> iter(*pSortedSignatures );
    while (iter.hasNext())
    {
        sig = iter.next();
        QList<QStandardItem*> items;
        QStandardItem * item2 = new QStandardItem(QString::number(sig->get_number_of_stems()));
        QStandardItem * item3 = new QStandardItem(QString::number(sig->get_robustness()));
        items.append(new QStandardItem(sig->GetSignature()));
        items.append(item2);
        items.append(item3);
        Signature_model->appendRow(items);
    }
}

void MainWindow::rowClicked(const QModelIndex &index)
{
    QStandardItem *item = m_treeModel->itemFromIndex(index);
    QString key = item->text();
    if (key == "Words"){
        m_tableView_upper->setModel(Word_model);
        m_tableView_upper->set_content_type( "words");
    }
    else if (key == "Stems"){
        m_tableView_upper->setModel(Stem_model);
        m_tableView_upper->set_content_type( "stems");
    }
    else if (key == "Suffixes"){
        m_tableView_upper->setModel(Affix_model);
        m_tableView_upper->set_content_type( "suffixes");
    }
    else if (key == "Signatures"){
        m_tableView_upper->setModel(Signature_model);
        m_tableView_upper->set_content_type(  "signatures");
    }
    else
        qDebug() << "Invalid selection: rowClicked";
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();

/*    if (ask_to_save()) {
        writeSettings();
        event->accept();
        qDebug() << " closing 1";
    } else {
        event->ignore();
        qDebug() << "   closing 2" ;
    }
*/
}
void MainWindow::newFile()
{
    if (ask_to_save()) {
        setCurrentFile(QString());
    }
}


void MainWindow::ask_for_filename()
{
    qDebug() << " ask for filename" ;
    m_name_of_data_file = QFileDialog::getOpenFileName(this);
    if (m_name_of_data_file.length() > 0){
        read_file_do_crab();
    }
}


void MainWindow::read_file_do_crab()
{
        read_dx1_file();
        get_lexicon()->Crab_1();
        qDebug()<<"Finished crab.";
        load_word_model();
        load_signature_model();
        load_affix_model();
        load_stem_model();
        createTreeModel();
}
bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}
bool MainWindow::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first());
}
void MainWindow::about()
{
   QMessageBox::about(this, tr("About Application"),
            tr("The <b>Application</b> example demonstrates how to "
               "write modern GUI applications using Qt, with a menu bar, "
               "toolbars, and a status bar."));
}
void MainWindow::documentWasModified()
{
    //setWindowModified(textEdit->document()->isModified());
}
void MainWindow::createActions()
{

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("File"));

    /*
    // No action associated with this.
    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon("../../../../QtLing/images/new.png"));
    QAction *newAct = new QAction(newIcon, tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAct);
    fileToolBar->addAction(newAct);
*/


    // Give a data file name, store the name, and read the file.
    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon("../../../../QtLing/images/open.png"));
    QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::ask_for_filename);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);
/*
    // No action associated with this yet.
    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon("../../../../QtLing/images/save.png"));
    QAction *saveAct = new QAction(saveIcon, tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(saveAct);
    fileToolBar->addAction(saveAct);
*/

    // No action associated with this yet.
    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    QAction *saveAsAct = fileMenu->addAction(saveAsIcon, tr("Save &As..."), this, &MainWindow::saveAs);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar *editToolBar = addToolBar(tr("Edit"));

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon("../../../../QtLing/images/cut.png"));
    QAction *cutAct = new QAction(cutIcon, tr("Cu&t"), this);

    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
//    connect(cutAct, &QAction::triggered, textEdit, &QPlainTextEdit::cut);
    editMenu->addAction(cutAct);
    editToolBar->addAction(cutAct);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon("../../../../QtLing/images/copy.png"));
    QAction *copyAct = new QAction(copyIcon, tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
//    connect(copyAct, &QAction::triggered, textEdit, &QPlainTextEdit::copy);
    editMenu->addAction(copyAct);
    editToolBar->addAction(copyAct);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon("../../../../QtLing/images/paste.png"));
    QAction *pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
//    connect(pasteAct, &QAction::triggered, textEdit, &QPlainTextEdit::paste);
    editMenu->addAction(pasteAct);
    editToolBar->addAction(pasteAct);

    menuBar()->addSeparator();

#endif // !QT_NO_CLIPBOARD

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
#ifndef QT_NO_CLIPBOARD
    cutAct->setEnabled(false);

    copyAct->setEnabled(false);
//    connect(textEdit, &QPlainTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
//    connect(textEdit, &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
#endif // !QT_NO_CLIPBOARD
}
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}
void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    m_name_of_data_file = settings.value("name_of_data_file").toString();
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}
void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("name_of_data_file", m_name_of_data_file );
}
bool MainWindow::ask_to_save()
{
    return false;

//    if (!textEdit->document()->isModified())
//        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}
void MainWindow::createTreeModel()
{
    QStandardItem *parent = m_treeModel->invisibleRootItem();
    QStandardItem *word_item = new QStandardItem(QString("Words"));
    QStandardItem * word_count_item = new QStandardItem(QString::number(get_lexicon()->GetWordCollection()->get_count()));
    QStandardItem *stem_item = new QStandardItem(QString("Stems"));
    QStandardItem * stem_count_item = new QStandardItem(QString::number(get_lexicon()->GetStemCollection()->get_count()));
    QStandardItem *suffix_item = new QStandardItem(QString("Suffixes"));
    QStandardItem * suffix_count_item = new QStandardItem(QString::number(get_lexicon()->GetSuffixCollection()->get_count()));

    QStandardItem *sig_item = new QStandardItem(QString("Signatures"));
    QStandardItem * sig_count_item = new QStandardItem(QString::number(get_lexicon()->GetSignatureCollection()->get_count()));

    QStandardItem *prStemItem = new QStandardItem(QString("Protostems"));

    QList<QStandardItem*> word_items;
    word_items.append(word_item);
    word_items.append(word_count_item);

    QList<QStandardItem*> stem_items;
    stem_items.append(stem_item);
    stem_items.append(stem_count_item);

    QList<QStandardItem*> suffix_items;
    suffix_items.append(suffix_item);
    suffix_items.append(suffix_count_item);

    QList<QStandardItem*> sig_items;
    sig_items.append(sig_item);
    sig_items.append(sig_count_item);


    parent->appendRow(word_items);
    parent->appendRow(stem_items);
    parent->appendRow(suffix_items);
    parent->appendRow(sig_items);
    parent->appendRow(prStemItem);
}

void MainWindow::read_dx1_file()
{
    QFile file(m_name_of_data_file);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(m_name_of_data_file), file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif

    CWordCollection * Words = get_lexicon()->GetWordCollection();

    while (!in.atEnd())
    {
        QString line = in.readLine();
        line.simplified(); // get rid of extra spaces
        QStringList words = line.split(" ");
        QString word = words[0];
        CWord* pWord = *Words << word;
        pWord->SetWordCount(words[1].toInt());
        qDebug() << word;
    }
    Words->sort_word_list();

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(m_name_of_data_file);
    statusBar()->showMessage(tr("File loaded"), 2000);
}
bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    //out << textEdit->toPlainText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}
void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    //textEdit->document()->setModified(false);
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
}
QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}
#ifndef QT_NO_SESSIONMANAGER

void MainWindow::commitData(QSessionManager &manager)
{
    if (manager.allowsInteraction()) {
        if (!ask_to_save())
            manager.cancel();
    } else {
        // Non-interactive: save without asking
//        if (textEdit->document()->isModified())
//            save();
    }
}
#endif

LowerTableView::LowerTableView()
{
   m_how_many_columns = 6;

}
 void LowerTableView::display_this_signature(QString signature)
 {
    CSignature*           pSig = m_parent_window->get_lexicon()->get_signatures()->get_signature(signature);
    CStem*                p_Stem;
    QList<CStem*>*        sig_stems = pSig->get_stems();
    QStandardItem*        p_item;
    QList<QStandardItem*> item_list;

 // Start by building a model fo this view.
    qDebug() << " building lower view" ;
    QStandardItemModel one_signature_model;
    
    foreach (p_Stem, *sig_stems)  {
        p_item = new QStandardItem(p_Stem->get_key() );
        item_list.append(p_item);
        if (item_list.length() >= m_number_of_columns){
            one_signature_model.appendRow(item_list);
            item_list.clear();
        }
    }
    setModel(& one_signature_model);

 }

 LeftSideTreeView::LeftSideTreeView(MainWindow* window)
 {
     m_parent_window = window;

 }
