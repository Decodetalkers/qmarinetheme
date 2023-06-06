/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
                         const QString &text = QString(),
                         bool hasMenu = true,
                         bool hasIcon = true,
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
