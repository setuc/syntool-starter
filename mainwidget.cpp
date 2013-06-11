#include "mainwidget.h"

static QtMessageHandler oldMsgHandler = 0;

MainWindow *MainWindow::self = 0;

static void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    MainWindow::self->log(type, context, msg);
//    oldMsgHandler(type, context, msg);
}

//#include "PythonQt.h"
//#include "gui/PythonQtScriptingConsole.h"

MainWindow::MainWindow(ConfigData *configValue, QWidget *parent)
    : QMainWindow(parent)
{
    configData = configValue;

    self = this;

    //set logger
    oldMsgHandler = qInstallMessageHandler(logMessageHandler);

    // add translator (in future)
//    QTranslator *translator = new QTranslator(this);
//    if (translator->load(QString(":/translations/test_%2.qm").arg(QLocale::system().name())))
//        qApp->installTranslator(translator);
//    else delete translator;

    //////

    vlayout = new QVBoxLayout;
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(5,0,5,2);

    centralwgt = new QWidget;
    centralwgt->setLayout(vlayout);
    setCentralWidget(centralwgt);

    splitter = new QSplitter(parent);

    topMenu = new TopMenu(this);
    topMenu->setFixedHeight(28);

    glwgt = new WindowWidget(this);
    glwgt->setMinimumSize(200, 200);

//    create right toolBox
    rightSidebar = new RightSidebar;

    mapsWgt = new MapsWidget(centralwgt);
    productsWgt = new ProductsWidget(configData, centralwgt);
    datasetsWgt = new DatasetsWidget(configData, centralwgt);
    placeWgt = new PlaceWidget(centralwgt);
    layersWgt = new LayersWidget(centralwgt);

    rightSidebar->addItem(productsWgt, tr("Products"));
    rightSidebar->addItem(mapsWgt, tr("Maps"));
    rightSidebar->addItem(layersWgt, tr("Layers"));
    rightSidebar->addItem(datasetsWgt, tr("Datasets"));
    rightSidebar->addItem(placeWgt, tr("Places"));

//    add all main widget
    splitter->addWidget(glwgt);
    splitter->addWidget(rightSidebar);
    splitter->setStretchFactor(0, 4);
    splitter->setStretchFactor(1, 1);

    timeLine = new TimeLine(configData, this);
    timeLine->setFixedHeight(100);
    timeLine->hide();

    vlayout->addWidget(topMenu);
    vlayout->addWidget(splitter);
    vlayout->addWidget(timeLine);

    createMenuBar();
    createPythonConsole();
    setFocusPolicy(Qt::StrongFocus);

    selectedProducts = new QHash<QString, selectedProduct>;
    granulesHash = new QHash<QString, Granule>;

    productsWgt->setSelectedProducts(selectedProducts, granulesHash);
    timeLine->setSelectedProducts(selectedProducts, granulesHash);
    datasetsWgt->setGranules(granulesHash);

    connect(timeLine, &TimeLine::getNewAllGranules, productsWgt, &ProductsWidget::getNewGranules);

    connect(productsWgt, &ProductsWidget::productAdded, layersWgt, &LayersWidget::addProduct);
    connect(productsWgt, &ProductsWidget::productDeleted, layersWgt, &LayersWidget::deleteProduct);

    connect(layersWgt, &LayersWidget::removeLayer, productsWgt, &ProductsWidget::removeProduct);

    connect(timeLine, &TimeLine::changedDisplayGranules, datasetsWgt, &DatasetsWidget::addDatasets);

    productsWgt->setObjectsPointer(timeLine);

    rightSidebar->setMinimumWidth(120);

    setWindowTitle("Syntool");
}

void MainWindow::setHostedWindow(EarthView *window)
{
    glwgt->setHostedWindow(window);
    earthViewPointer = window;
    metaGranulesPointer = earthViewPointer->metaGranulesNode;

    connect(datasetsWgt, &DatasetsWidget::displayGranule, metaGranulesPointer, &MetaGranules::addGranuleNode);
    connect(datasetsWgt, &DatasetsWidget::hideGranule, metaGranulesPointer, &MetaGranules::removeSimpleGranuleNode);
    connect(datasetsWgt, &DatasetsWidget::changedGranuleTransparency,
            metaGranulesPointer, &MetaGranules::setGranuleTransparency);
    connect(datasetsWgt, &DatasetsWidget::changedGranuleHeight, metaGranulesPointer, &MetaGranules::changedGranuleHeight);
    connect(datasetsWgt, SIGNAL(changedGranulesHeight(qint32,qint32,qint32,qint32)),
            metaGranulesPointer, SLOT(changedGranulesHeight(qint32,qint32,qint32,qint32)));

    connect(productsWgt, &ProductsWidget::productsHashSignal, metaGranulesPointer, &MetaGranules::setProductsHashSlot);
}

void MainWindow::aboutProgram()
{
    aboutWgt = new AboutWidget(configData);
}

void MainWindow::openHandBook()
{
    QDesktopServices::openUrl(QUrl("http://wiki.solab.rshu.ru/Main_Page"));
}

void MainWindow::showTimeLine()
{
    if (timeLine->isVisible())
        timeLine->hide();
    else
        timeLine->show();
}

void MainWindow::createMenuBar()
{
    menuBar()->setContentsMargins(0,0,0,0);

    QMenu* fileMenu = menuBar()->addMenu("&"+tr("File"));
    fileMenu->setDisabled(true);
    QIcon::setThemeName("oxygen");

    QAction* openAction = new QAction(QIcon(":/icons/open.png"), "&"+tr("Open"), this);
    openAction->setShortcut(QKeySequence::Open);


    QAction* saveAction = new QAction(QIcon(":/icons/save.png"), "&"+tr("Save"), this);
    saveAction->setShortcut(QKeySequence::Save);


    QAction* exitAction = new QAction(QIcon(":/icons/exit.png"), tr("Exit"), this);
    exitAction->setShortcut(Qt::Key_Q+Qt::CTRL);
//    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setShortcutContext(Qt::ApplicationShortcut);

//    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
//    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);

    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

/////////////// Edit tab
    QMenu* editMenu = menuBar()->addMenu("&"+tr("Edit"));
    editMenu->setDisabled(true);

    QAction* findAction = new QAction(QIcon(":/icons/find.png"), tr("Find"), this);
    // properties current object
    QAction* propertiesAction = new QAction(QIcon(":/icons/properties.png"), tr("Properties"), this);

    editMenu->addAction(findAction);
    editMenu->addSeparator();
    editMenu->addAction(propertiesAction);

/////////////// View tab
    QMenu* viewMenu = menuBar()->addMenu("&"+tr("View"));
    viewMenu->setDisabled(true);

    QAction* showToolbarAction = new QAction(tr("Toolbar"), this);
    showToolbarAction->setCheckable(true);
    showToolbarAction->setChecked(true);
    connect(showToolbarAction, &QAction::triggered, topMenu, &TopMenu::setVisible);

    QAction* showSidebarAction = new QAction(tr("Sidebar"), this);
    showSidebarAction->setCheckable(true);
    showSidebarAction->setChecked(true);
    connect(showSidebarAction, &QAction::triggered, rightSidebar, &RightSidebar::setVisible);

    viewMenu->addAction(showToolbarAction);
    viewMenu->addAction(showSidebarAction);

/////////////// Settings tab
    QMenu* toolsMenu = menuBar()->addMenu("&"+tr("Tools"));

    QAction* settingAction = new QAction(QIcon(":/icons/settings.png"), tr("Settings"), this);
    connect (settingAction, &QAction::triggered, this, &MainWindow::showSettings);

    QAction* rulerAction = new QAction(QIcon(":/icons/ruler.png"), tr("Ruler"), this);

    toolsMenu->addAction(rulerAction);
    toolsMenu->addAction(settingAction);

/////////////// Help tab
    QMenu* helpMenu = menuBar()->addMenu("&"+tr("Help"));

    QAction* aboutAction = new QAction(QIcon(":/icons/help.png"), "&"+tr("About"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::aboutProgram);

    QAction* bugsAction = new QAction(QIcon(":/icons/e-mail.png"), tr("Send bugs"), this);
    bugsAction->setDisabled(true);
    bugsAction->setShortcut(Qt::Key_B);

    QAction* handbookAction = new QAction(QIcon(":/icons/help_book.png"), "&"+tr("Handbook"), this);
    connect(handbookAction, &QAction::triggered, this, &MainWindow::openHandBook);
    handbookAction->setShortcut(QKeySequence::HelpContents);

    QAction* aboutQtAction = new QAction(QIcon(":/icons/qt.png"), "&"+tr("About Qt"), this);
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    helpMenu->addAction(aboutAction);
    helpMenu->addAction(handbookAction);
    helpMenu->addAction(bugsAction);
    helpMenu->addAction(aboutQtAction);
}

void MainWindow::showSettings()
{
    settingsWidget = new SettingsWidget(configData);
    settingsWidget->show();
}

void MainWindow::createPythonConsole()
{
//    PythonQt::init(PythonQt::IgnoreSiteModule | PythonQt::RedirectStdOut);
//    PythonQt_QtAll::init();

//    PythonQtObjectPtr  mainContext = PythonQt::self()->getMainModule();
//    PythonQtScriptingConsole console(NULL, mainContext);

//    mainContext.evalFile(":example.py");

//    QWidget *window = new QWidget;
//    QPushButton *button1 = new QPushButton("One");
//    QPushButton *button2 = new QPushButton("Two");

//    QVBoxLayout *layout = new QVBoxLayout;
//    layout->addWidget(button1);
//    layout->addWidget(button2);
//    vlayout->addWidget(&console);

//    window->setLayout(layout);
//    window->show();
//    mainContext.addObject("window", window);
//    mainContext.addObject("button1", button1);
//    mainContext.addObject("button2", button2);
}

void MainWindow::keyPress(QKeyEvent *e)
{
    keyPressEvent(e);
}

void MainWindow::log(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
    {
        if (configData->logLevel & LogLevel::DebugOnly)
            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
    }
        break;
    case QtWarningMsg:
        if (configData->logLevel & LogLevel::WarningOnly)
            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        if (configData->logLevel & LogLevel::ErrorOnly)
            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        if (configData->logLevel & LogLevel::ErrorOnly)
            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}

MainWindow::~MainWindow()
{
//    qInstallMessageHandler(oldMsgHandler);
}
