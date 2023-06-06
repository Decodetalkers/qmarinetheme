#ifndef WIDGETGALLERY_H
#define WIDGETGALLERY_H

#include <QMainWindow>
#include <QToolButton>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
class QDateTimeEdit;
class QDial;
class QGroupBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QRadioButton;
class QScrollBar;
class QSlider;
class QSpinBox;
class QTabWidget;
class QTableWidget;
class QTextEdit;
QT_END_NAMESPACE

//! [0]
class WidgetGallery : public QMainWindow
{
    Q_OBJECT

public:
    WidgetGallery(QWidget *parent = 0);

private slots:
    void changeStyle(const QString &styleName);
    void changePalette();
    void advanceProgressBar();

private:
    void createTopLeftGroupBox();
    void createTopRightGroupBox();
    void createBottomLeftTabWidget();
    void createBottomRightGroupBox();
    void createProgressBar();
    QToolButton *toolBtn(QToolButton::ToolButtonPopupMode mode,
                         const QString &text       = QString(),
                         bool hasMenu              = true,
                         bool hasIcon              = true,
                         Qt::ToolButtonStyle style = Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    QWidget *createToolButtons(QWidget *parent = nullptr, bool hasMenu = true);

    QPalette m_originalPalette;

    QLabel *m_styleLabel;
    QComboBox *m_styleComboBox;
    QCheckBox *m_useStylePaletteCheckBox;
    QCheckBox *m_disableWidgetsCheckBox;
    //! [0]

    QGroupBox *m_topLeftGroupBox;
    QRadioButton *m_radioButton1;
    QRadioButton *m_radioButton2;
    QRadioButton *m_radioButton3;
    QCheckBox *m_checkBox;

    QGroupBox *m_topRightGroupBox;
    QPushButton *m_defaultPushButton;
    QPushButton *m_togglePushButton;
    QPushButton *m_flatPushButton;
    QPushButton *m_xdgDialogPushButton;

    QTabWidget *m_bottomLeftTabWidget;
    QTableWidget *m_tableWidget;
    QTextEdit *m_textEdit;

    QGroupBox *m_bottomRightGroupBox;
    QLineEdit *m_lineEdit;
    QSpinBox *m_spinBox;
    QDateTimeEdit *m_dateTimeEdit;
    QComboBox *m_comboBoxEdit;
    QSlider *m_slider;
    QScrollBar *m_scrollBar;
    QDial *m_dial;

    QProgressBar *m_progressBar;
    //! [1]
};
//! [1]

#endif
