#include <QFileSystemModel>
#include <QTreeView>
#include <QtWidgets>

#include "norwegianwoodstyle.h"
#include "widgetgallery.h"

//! [0]
WidgetGallery::WidgetGallery(QWidget *parent)
  : QMainWindow(parent)
{
    m_originalPalette = QApplication::palette();

    m_styleComboBox = new QComboBox;
    m_styleComboBox->addItem("NorwegianWood");
    m_styleComboBox->addItems(QStyleFactory::keys());
    auto keys = QStyleFactory::keys();
    int i = 1;
    for (; i < keys.length(); i++) {
        if (keys[i-1].toLower() == qApp->style()->name()) {
            break;
        }
    }
    m_styleComboBox->setCurrentIndex(i);
    m_styleComboBox->setToolTip("Hello");

    m_styleLabel = new QLabel(tr("&Style:"));
    m_styleLabel->setBuddy(m_styleComboBox);

    m_useStylePaletteCheckBox = new QCheckBox(tr("&Use style's standard palette"));
    m_useStylePaletteCheckBox->setChecked(false);

    m_disableWidgetsCheckBox = new QCheckBox(tr("&Disable widgets"));

    // changeStyle("NorwegianWood");

    createTopLeftGroupBox();
    createTopRightGroupBox();
    createBottomLeftTabWidget();
    createBottomRightGroupBox();
    createProgressBar();
    //! [0]

    //! [1]
    connect(m_styleComboBox, &QComboBox::currentTextChanged, this, &WidgetGallery::changeStyle);
    connect(m_useStylePaletteCheckBox, &QCheckBox::toggled, this, &WidgetGallery::changePalette);
    connect(
      m_disableWidgetsCheckBox, &QCheckBox::toggled, m_topLeftGroupBox, &QCheckBox::setDisabled);
    connect(
      m_disableWidgetsCheckBox, &QCheckBox::toggled, m_topRightGroupBox, &QCheckBox::setDisabled);
    connect(m_disableWidgetsCheckBox,
            &QCheckBox::toggled,
            m_bottomLeftTabWidget,
            &QCheckBox::setDisabled);
    connect(m_disableWidgetsCheckBox,
            &QCheckBox::toggled,
            m_bottomRightGroupBox,
            &QCheckBox::setDisabled);
    //! [2]

    //! [3]
    QWidget *mainWidget    = new QWidget();
    QHBoxLayout *topLayout = new QHBoxLayout;
    //! [3] //! [4]
    topLayout->addWidget(m_styleLabel);
    topLayout->addWidget(m_styleComboBox);
    topLayout->addStretch(1);
    topLayout->addWidget(m_useStylePaletteCheckBox);
    topLayout->addWidget(m_disableWidgetsCheckBox);

    QGridLayout *mainLayout = new QGridLayout(mainWidget);
    mainLayout->addLayout(topLayout, 0, 0, 1, 2);
    mainLayout->addWidget(m_topLeftGroupBox, 1, 0);
    mainLayout->addWidget(m_topRightGroupBox, 1, 1);
    mainLayout->addWidget(m_bottomLeftTabWidget, 2, 0);
    mainLayout->addWidget(m_bottomRightGroupBox, 2, 1);
    mainLayout->addWidget(m_progressBar, 3, 0, 1, 2);
    mainLayout->setRowStretch(1, 1);
    mainLayout->setRowStretch(2, 1);
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(1, 1);

    setWindowTitle(tr("Styles"));

    setCentralWidget(mainWidget);

    QMenu *firstLayer = new QMenu("first");
    menuBar()->addAction("chameleon")->setMenu(firstLayer);
    firstLayer->addAction("menu 1")->setIcon(QIcon::fromTheme("document-open"));
    firstLayer->addAction("menu 2")->setEnabled(false);
    firstLayer->addSection("Section");
    firstLayer->addAction("menu 3")->setCheckable(true);
    firstLayer->addSeparator();

    QMenu *secondLayer = new QMenu("second");
    firstLayer->addMenu(secondLayer);
    QActionGroup *group = new QActionGroup(this);
    group->setExclusive(true);
    QAction *sa1 = new QAction("submenu 1");
    sa1->setCheckable(true);
    group->addAction(sa1);
    QAction *sa2 = new QAction("submenu 2");
    sa2->setCheckable(true);
    sa2->setIcon(QIcon::fromTheme("application-exit"));
    group->addAction(sa2);
    QAction *sa3 = new QAction("submenu 3");
    sa3->setCheckable(true);
    sa3->setShortcut(QKeySequence::New);
    group->addAction(sa3);
    secondLayer->addActions(group->actions());
    secondLayer->setLayoutDirection(Qt::RightToLeft);

    menuBar()->addAction("dlight");
    menuBar()->addAction("ddark");
    menuBar()->addAction("dsemidark");
    menuBar()->addAction("dsemilight");

    connect(menuBar(), &QMenuBar::triggered, this, [this](const QAction *action) {
        changeStyle(action->text());
    });
}
//! [4]

//! [5]
void
WidgetGallery::changeStyle(const QString &styleName)
//! [5] //! [6]
{
    if (styleName == "NorwegianWood") {
        QApplication::setStyle(new NorwegianWoodStyle);
    } else {
        QApplication::setStyle(QStyleFactory::create(styleName));
    }
    changePalette();
}
//! [6]

//! [7]
void
WidgetGallery::changePalette()
//! [7] //! [8]
{
    if (m_useStylePaletteCheckBox->isChecked()) {
        QApplication::setPalette(QApplication::style()->standardPalette());
    } else {
        QApplication::setPalette(m_originalPalette);
        QApplication::setAttribute(Qt::AA_SetPalette, false);
    }
}
//! [8]

//! [9]
void
WidgetGallery::advanceProgressBar()
//! [9] //! [10]
{
    int curVal = m_progressBar->value();
    int maxVal = m_progressBar->maximum();
    m_progressBar->setValue(curVal + (maxVal - curVal) / 100);
}
//! [10]

//! [11]
void
WidgetGallery::createTopLeftGroupBox()
//! [11] //! [12]
{
    m_topLeftGroupBox = new QGroupBox(tr("Group 1"));

    m_radioButton1 = new QRadioButton(tr("Radio button 1"));
    m_radioButton2 = new QRadioButton(tr("Radio button 2"));
    m_radioButton3 = new QRadioButton(tr("Radio button 3"));
    m_radioButton1->setChecked(true);

    m_checkBox = new QCheckBox(tr("Tri-state check box"));
    m_checkBox->setTristate(true);
    m_checkBox->setCheckState(Qt::PartiallyChecked);

    connect(m_radioButton1, &QRadioButton::clicked, this, [this] {
        m_bottomLeftTabWidget->setDocumentMode(!m_bottomLeftTabWidget->documentMode());
    });
    connect(m_radioButton2, &QRadioButton::clicked, this, [this] {
        m_bottomLeftTabWidget->setTabShape(QTabWidget::Rounded);
        m_bottomLeftTabWidget->update();
    });
    connect(m_radioButton3, &QRadioButton::clicked, this, [this] {
        m_bottomLeftTabWidget->setTabShape(QTabWidget::Triangular);
        m_bottomLeftTabWidget->update();
    });

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_radioButton1);
    layout->addWidget(m_radioButton2);
    layout->addWidget(m_radioButton3);
    layout->addWidget(m_checkBox);
    layout->addStretch(1);
    m_topLeftGroupBox->setLayout(layout);
}
//! [12]

void
WidgetGallery::createTopRightGroupBox()
{
    m_topRightGroupBox = new QGroupBox(tr("Group 2"));

    m_defaultPushButton = new QPushButton(tr("Default Push Button"));
    m_defaultPushButton->setDefault(true);

    m_togglePushButton = new QPushButton(tr("Toggle Push Button"));
    m_togglePushButton->setCheckable(true);
    m_togglePushButton->setChecked(true);

    m_flatPushButton = new QPushButton(tr("Flat Push Button"));
    m_flatPushButton->setFlat(true);

    m_xdgDialogPushButton = new QPushButton(tr("Clicked it to open Dialog"));
    connect(m_xdgDialogPushButton, &QPushButton::clicked, this, [=] {
        QFileDialog dialog;
        dialog.exec();
    });

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_defaultPushButton);
    layout->addWidget(m_togglePushButton);
    layout->addWidget(m_flatPushButton);
    layout->addWidget(m_xdgDialogPushButton);
    layout->addStretch(1);
    m_topRightGroupBox->setLayout(layout);
}

void
WidgetGallery::createBottomLeftTabWidget()
{
    m_bottomLeftTabWidget = new QTabWidget;
    m_bottomLeftTabWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);

    m_bottomLeftTabWidget->setTabsClosable(true);
    m_bottomLeftTabWidget->setTabShape(QTabWidget::Triangular);

    QWidget *tab1 = new QWidget;
    m_tableWidget = new QTableWidget(10, 10);
    m_tableWidget->setAlternatingRowColors(true);

    QHBoxLayout *tab1hbox = new QHBoxLayout;
    // tab1hbox->setMargin(5);
    tab1hbox->addWidget(m_tableWidget);
    tab1->setLayout(tab1hbox);

    QWidget *tab2 = new QWidget;
    m_textEdit    = new QTextEdit;

    m_textEdit->setPlainText(tr("Twinkle, twinkle, little star,\n"
                                "How I wonder what you are.\n"
                                "Up above the world so high,\n"
                                "Like a diamond in the sky.\n"
                                "Twinkle, twinkle, little star,\n"
                                "How I wonder what you are!\n"));

    QHBoxLayout *tab2hbox = new QHBoxLayout;
    // tab2hbox->setMargin(5);
    tab2hbox->addWidget(m_textEdit);
    tab2->setLayout(tab2hbox);

    m_bottomLeftTabWidget->addTab(tab1, tr("&Table"));
    m_bottomLeftTabWidget->addTab(tab2, tr("Text &Edit"));

    QWidget *pTreeViewWidget = new QWidget;
    QFileSystemModel *model  = new QFileSystemModel(this);
    model->setRootPath(QDir::currentPath());
    QHBoxLayout *pTabLayout = new QHBoxLayout;
    // pTabLayout->setMargin(0);
    // pTabLayout->setMargin(0);
    pTreeViewWidget->setLayout(pTabLayout);

    QTreeView *tree = new QTreeView;
    tree->setModel(model);
    pTabLayout->addWidget(tree);

    QWidget *pListViewWidget      = new QWidget;
    QHBoxLayout *pListLayout      = new QHBoxLayout;
    QStandardItemModel *listModel = new QStandardItemModel(this);
    // pListLayout->setMargin(0);
    pListViewWidget->setLayout(pListLayout);

    QListView *lv = new QListView;
    lv->setDragEnabled(true);
    lv->setDragDropMode(QListView::DragDrop);
    lv->setDefaultDropAction(Qt::CopyAction);
    lv->setModel(listModel);
    pListLayout->addWidget(lv);

    for (uint8_t i = 0; i < 10; ++i) {
        QStandardItem *item = new QStandardItem;
        item->setIcon(this->style() ? this->style()->standardIcon(
                                        QStyle::StandardPixmap(QStyle::SP_DirIcon + i))
                                    : QIcon());
        item->setText(QStringLiteral("Row %1...............").arg(i + 1));
        item->setEnabled(i % 2);
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);
        item->setDragEnabled(true);
        listModel->appendRow(item);
    }

    QWidget *toolbtns      = new QWidget;
    QVBoxLayout *tbVLayout = new QVBoxLayout(toolbtns);
    tbVLayout->addWidget(createToolButtons(nullptr, false));
    tbVLayout->addWidget(createToolButtons(nullptr, true));
    QScrollArea *toolArea = new QScrollArea;
    toolArea->setWidget(toolbtns);
    m_bottomLeftTabWidget->addTab(pTreeViewWidget, "&TreeView");
    m_bottomLeftTabWidget->addTab(pListViewWidget, "&ListView");
    m_bottomLeftTabWidget->addTab(toolArea, "toolbuttons");
    m_bottomLeftTabWidget->addTab(new QWidget(), "tab 3");
    m_bottomLeftTabWidget->addTab(new QWidget(), "tab 4");
}

void
WidgetGallery::createBottomRightGroupBox()
{
    m_bottomRightGroupBox = new QGroupBox(tr("Group 3"));
    m_bottomRightGroupBox->setCheckable(true);
    m_bottomRightGroupBox->setChecked(true);

    m_lineEdit = new QLineEdit("s3cRe7");
    m_lineEdit->setEchoMode(QLineEdit::Password);
    m_lineEdit->setClearButtonEnabled(true);
    m_lineEdit->setFrame(false);
    QMenu *menu = m_lineEdit->createStandardContextMenu();
    menu->setParent(m_lineEdit);
    QAction *testActoin = new QAction("b");
    testActoin->setCheckable(true);
    testActoin->setProperty("_d_menu_item_redpoint", true);

    QMenu *subMenu = new QMenu("just test red point sub menu", menu);
    subMenu->setProperty("_d_menu_item_redpoint", true);
    subMenu->setProperty("_d_menu_item_info", "new");
    subMenu->addAction("111");
    subMenu->addAction("222");
    QAction *a3 = new QAction("a3");
    a3->setProperty("_d_menu_item_redpoint", true);
    subMenu->addAction(a3);
    menu->addMenu(subMenu);
    testActoin->setProperty("_d_menu_item_info", "99+");
    QObject::connect(testActoin, &QAction::triggered, testActoin, [testActoin](bool checked) {
        testActoin->setProperty("_d_menu_item_redpoint", checked);
    });
    menu->addAction(testActoin);
    QObject::connect(
      m_lineEdit, &QLineEdit::textChanged, m_lineEdit, [menu]() { menu->popup(QCursor::pos()); });

    m_spinBox = new QSpinBox(m_bottomRightGroupBox);
    m_spinBox->setValue(50);
    m_spinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    m_spinBox->setPrefix(" Prefix ");
    m_spinBox->setSuffix(" Suffix ");
    m_spinBox->setAlignment(Qt::AlignCenter);
    m_spinBox->setFrame(false);

    m_dateTimeEdit = new QDateTimeEdit(m_bottomRightGroupBox);
    m_dateTimeEdit->setDateTime(QDateTime::currentDateTime());

    m_comboBoxEdit = new QComboBox(m_bottomRightGroupBox);
    m_comboBoxEdit->addItem(QIcon::fromTheme("dde-file-manager"), "dde-file-manager");
    m_comboBoxEdit->addItem(QIcon::fromTheme("dde-introduction"), "dde-introduction");
    m_comboBoxEdit->addItem(QIcon::fromTheme("deepin-deb-installer"), "deepin-deb-installer");
    m_comboBoxEdit->setEditable(true);

    m_slider = new QSlider(Qt::Horizontal, m_bottomRightGroupBox);
    m_slider->setRange(0, 100);
    m_slider->setTickInterval(10);
    m_slider->setTickPosition(QSlider::TicksBelow);
    m_slider->setValue(40);

    m_scrollBar = new QScrollBar(Qt::Horizontal, m_bottomRightGroupBox);
    m_scrollBar->setValue(60);
    m_scrollBar->setProperty("_d_dtk_slider_always_show", true);

    m_dial = new QDial(m_bottomRightGroupBox);
    m_dial->setValue(30);
    m_dial->setNotchesVisible(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(m_lineEdit, 0, 0, 1, 2);
    layout->addWidget(m_spinBox, 1, 0, 1, 2);
    layout->addWidget(m_dateTimeEdit, 2, 0, 1, 2);
    layout->addWidget(m_comboBoxEdit, 3, 0, 1, 2);
    layout->addWidget(m_slider, 4, 0);
    layout->addWidget(m_scrollBar, 5, 0);
    layout->addWidget(m_dial, 4, 1, 2, 1);
    layout->setRowStretch(6, 1);
    m_bottomRightGroupBox->setLayout(layout);
}

//! [13]
void
WidgetGallery::createProgressBar()
{
    m_progressBar = new QProgressBar;
    m_progressBar->setRange(0, 10000);
    m_progressBar->setValue(0);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &WidgetGallery::advanceProgressBar);
    timer->start(1000);
}
//! [13]

QToolButton *
WidgetGallery::toolBtn(QToolButton::ToolButtonPopupMode mode,
                       const QString &text,
                       bool hasMenu,
                       bool hasIcon,
                       Qt::ToolButtonStyle style)
{
    QToolButton *btn = new QToolButton;
    QMenu *menu      = new QMenu;
    menu->addAction("action1");
    menu->addAction("action2");
    if (hasMenu)
        btn->setMenu(menu);
    if (hasIcon)
        btn->setIcon(QIcon::fromTheme("edit"));
    btn->setIconSize({16, 16});
    btn->setPopupMode(mode);
    if (!text.isEmpty()) {
        btn->setText(text);
        btn->setToolButtonStyle(style);
    }

    return btn;
}

QWidget *
WidgetGallery::createToolButtons(QWidget *parent, bool hasMenu)
{
    QWidget *holder = new QWidget(parent);
    holder->resize(300, 500);
    QGridLayout *gridLayout = new QGridLayout(holder);
    gridLayout->addWidget(new QLabel(QString("ToolButtonPopupMode")), 0, 0);
    gridLayout->addWidget(new QLabel("IconOnly"), 0, 1);
    gridLayout->addWidget(new QLabel("    TextOnly"), 0, 2);
    gridLayout->addWidget(new QLabel("TextBesideIcon"), 0, 3);
    gridLayout->addWidget(new QLabel("TextUnderIcon"), 0, 4);
    gridLayout->addWidget(new QLabel("FollowStyle"), 0, 5);
    QString tmp = +hasMenu ? QString("(hasMenu)") : QString("(NoMenu)");
    for (int i = 0; i < 3; ++i) {
        auto mode          = static_cast<QToolButton::ToolButtonPopupMode>(i);
        QMetaEnum metaEnum = QMetaEnum::fromType<QToolButton::ToolButtonPopupMode>();

        gridLayout->addWidget(new QLabel(metaEnum.valueToKey(mode) + tmp), i + 1, 0);

        if (hasMenu) {
            QToolButton *menuTextIconBtnUnderIconOnly =
              toolBtn(mode, "ToolButton", true, true, Qt::ToolButtonStyle::ToolButtonIconOnly);
            gridLayout->addWidget(menuTextIconBtnUnderIconOnly, i + 1, 1);

            QToolButton *menuTextIconBtnUnderTextOnly =
              toolBtn(mode, "ToolButton", true, true, Qt::ToolButtonStyle::ToolButtonTextOnly);
            gridLayout->addWidget(menuTextIconBtnUnderTextOnly, i + 1, 2);

            QToolButton *menuTextIconBtn = toolBtn(
              mode, "ToolButton", true, true, Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
            gridLayout->addWidget(menuTextIconBtn, i + 1, 3);

            QToolButton *menuTextIconBtnUnder =
              toolBtn(mode, "ToolButton", true, true, Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
            gridLayout->addWidget(menuTextIconBtnUnder, i + 1, 4);

            QToolButton *menuTextIconBtnUnderFollow =
              toolBtn(mode, "ToolButton", true, true, Qt::ToolButtonStyle::ToolButtonFollowStyle);
            gridLayout->addWidget(menuTextIconBtnUnderFollow, i + 1, 5);
        } else {
            QToolButton *noMenuTextIconBtnIconOnly =
              toolBtn(mode, "ToolButton", false, true, Qt::ToolButtonStyle::ToolButtonIconOnly);
            gridLayout->addWidget(noMenuTextIconBtnIconOnly, i + 1, 1);

            QToolButton *noMenuTextIconBtnTextOnly =
              toolBtn(mode, "ToolButton", false, true, Qt::ToolButtonStyle::ToolButtonTextOnly);
            gridLayout->addWidget(noMenuTextIconBtnTextOnly, i + 1, 2);

            QToolButton *noMenuTextIconBtn = toolBtn(
              mode, "ToolButton", false, true, Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
            gridLayout->addWidget(noMenuTextIconBtn, i + 1, 3);

            QToolButton *noMenuTextIconBtnUnder = toolBtn(
              mode, "ToolButton", false, true, Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
            gridLayout->addWidget(noMenuTextIconBtnUnder, i + 1, 4);

            QToolButton *noMenuTextIconBtnFollow =
              toolBtn(mode, "ToolButton", false, true, Qt::ToolButtonStyle::ToolButtonFollowStyle);
            gridLayout->addWidget(noMenuTextIconBtnFollow, i + 1, 5);
        }
    }
    return holder;
}
