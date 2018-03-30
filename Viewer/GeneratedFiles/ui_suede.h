/********************************************************************************
** Form generated from reading UI file 'suede.ui'
**
** Created by: Qt User Interface Compiler version 5.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SUEDE_H
#define UI_SUEDE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "windows/console/console.h"
#include "windows/controls/canvas.h"
#include "windows/game/game.h"
#include "windows/hierarchy/hierarchy.h"
#include "windows/inspector/inspector.h"

QT_BEGIN_NAMESPACE

class Ui_Suede
{
public:
    QAction *exit;
    QAction *screenCapture;
    QAction *about;
    QWidget *centralWidget;
    Game *game;
    QWidget *gameView;
    QVBoxLayout *verticalLayout;
    Canvas *canvas;
    Hierarchy *hierarchy;
    QWidget *hierarchyView;
    QVBoxLayout *verticalLayout_3;
    QTreeView *tree;
    Console *console;
    QWidget *consoleView;
    QVBoxLayout *verticalLayout_2;
    QTableWidget *table;
    Inspector *inspector;
    QWidget *inspectorView;
    QVBoxLayout *verticalLayout_4;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *content;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout;
    QCheckBox *active;
    QComboBox *tag;
    QLineEdit *name;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QLabel *label;
    QLineEdit *px;
    QLabel *label_7;
    QPushButton *p0;
    QLineEdit *pz;
    QLineEdit *py;
    QPushButton *r0;
    QLabel *label_5;
    QLabel *label_8;
    QLabel *label_4;
    QLineEdit *rx;
    QLineEdit *ry;
    QLineEdit *sx;
    QLabel *label_3;
    QLabel *label_2;
    QPushButton *s0;
    QLineEdit *rz;
    QLabel *label_9;
    QLineEdit *sz;
    QLineEdit *sy;
    QLabel *label_6;
    QSpacerItem *verticalSpacer;
    QMenuBar *menuBar;
    QMenu *file;
    QMenu *help;

    void setupUi(QMainWindow *Suede)
    {
        if (Suede->objectName().isEmpty())
            Suede->setObjectName(QStringLiteral("Suede"));
        Suede->resize(705, 1090);
        QFont font;
        font.setFamily(QStringLiteral("Tahoma"));
        Suede->setFont(font);
        Suede->setFocusPolicy(Qt::StrongFocus);
        Suede->setAnimated(true);
        Suede->setDocumentMode(false);
        Suede->setDockNestingEnabled(true);
        Suede->setDockOptions(QMainWindow::AllowNestedDocks|QMainWindow::AllowTabbedDocks|QMainWindow::AnimatedDocks);
        exit = new QAction(Suede);
        exit->setObjectName(QStringLiteral("exit"));
        screenCapture = new QAction(Suede);
        screenCapture->setObjectName(QStringLiteral("screenCapture"));
        about = new QAction(Suede);
        about->setObjectName(QStringLiteral("about"));
        centralWidget = new QWidget(Suede);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        Suede->setCentralWidget(centralWidget);
        game = new Game(Suede);
        game->setObjectName(QStringLiteral("game"));
        game->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(game->sizePolicy().hasHeightForWidth());
        game->setSizePolicy(sizePolicy);
        game->setMinimumSize(QSize(80, 100));
        game->setFocusPolicy(Qt::StrongFocus);
        gameView = new QWidget();
        gameView->setObjectName(QStringLiteral("gameView"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(gameView->sizePolicy().hasHeightForWidth());
        gameView->setSizePolicy(sizePolicy1);
        verticalLayout = new QVBoxLayout(gameView);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        canvas = new Canvas(gameView);
        canvas->setObjectName(QStringLiteral("canvas"));

        verticalLayout->addWidget(canvas);

        game->setWidget(gameView);
        Suede->addDockWidget(static_cast<Qt::DockWidgetArea>(1), game);
        hierarchy = new Hierarchy(Suede);
        hierarchy->setObjectName(QStringLiteral("hierarchy"));
        sizePolicy1.setHeightForWidth(hierarchy->sizePolicy().hasHeightForWidth());
        hierarchy->setSizePolicy(sizePolicy1);
        hierarchy->setMaximumSize(QSize(524287, 524287));
        hierarchy->setAllowedAreas(Qt::AllDockWidgetAreas);
        hierarchyView = new QWidget();
        hierarchyView->setObjectName(QStringLiteral("hierarchyView"));
        verticalLayout_3 = new QVBoxLayout(hierarchyView);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        tree = new QTreeView(hierarchyView);
        tree->setObjectName(QStringLiteral("tree"));
        tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tree->setSelectionMode(QAbstractItemView::ExtendedSelection);

        verticalLayout_3->addWidget(tree);

        hierarchy->setWidget(hierarchyView);
        Suede->addDockWidget(static_cast<Qt::DockWidgetArea>(1), hierarchy);
        console = new Console(Suede);
        console->setObjectName(QStringLiteral("console"));
        console->setEnabled(true);
        console->setMinimumSize(QSize(89, 111));
        console->setFloating(false);
        console->setAllowedAreas(Qt::AllDockWidgetAreas);
        consoleView = new QWidget();
        consoleView->setObjectName(QStringLiteral("consoleView"));
        verticalLayout_2 = new QVBoxLayout(consoleView);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        table = new QTableWidget(consoleView);
        table->setObjectName(QStringLiteral("table"));
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);

        verticalLayout_2->addWidget(table);

        console->setWidget(consoleView);
        Suede->addDockWidget(static_cast<Qt::DockWidgetArea>(1), console);
        inspector = new Inspector(Suede);
        inspector->setObjectName(QStringLiteral("inspector"));
        inspector->setEnabled(true);
        inspector->setMinimumSize(QSize(100, 200));
        inspector->setMaximumSize(QSize(524287, 524287));
        inspector->setFloating(false);
        inspector->setAllowedAreas(Qt::AllDockWidgetAreas);
        inspectorView = new QWidget();
        inspectorView->setObjectName(QStringLiteral("inspectorView"));
        sizePolicy1.setHeightForWidth(inspectorView->sizePolicy().hasHeightForWidth());
        inspectorView->setSizePolicy(sizePolicy1);
        verticalLayout_4 = new QVBoxLayout(inspectorView);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        scrollArea = new QScrollArea(inspectorView);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setFrameShadow(QFrame::Raised);
        scrollArea->setLineWidth(0);
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 497, 271));
        content = new QVBoxLayout(scrollAreaWidgetContents);
        content->setSpacing(6);
        content->setContentsMargins(11, 11, 11, 11);
        content->setObjectName(QStringLiteral("content"));
        content->setContentsMargins(0, 0, 0, 0);
        widget_2 = new QWidget(scrollAreaWidgetContents);
        widget_2->setObjectName(QStringLiteral("widget_2"));
        widget_2->setEnabled(true);
        QSizePolicy sizePolicy2(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(widget_2->sizePolicy().hasHeightForWidth());
        widget_2->setSizePolicy(sizePolicy2);
        widget_2->setMaximumSize(QSize(16777215, 40));
        horizontalLayout = new QHBoxLayout(widget_2);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(-1, 0, -1, 0);
        active = new QCheckBox(widget_2);
        active->setObjectName(QStringLiteral("active"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(active->sizePolicy().hasHeightForWidth());
        active->setSizePolicy(sizePolicy3);

        horizontalLayout->addWidget(active);

        tag = new QComboBox(widget_2);
        tag->setObjectName(QStringLiteral("tag"));
        sizePolicy3.setHeightForWidth(tag->sizePolicy().hasHeightForWidth());
        tag->setSizePolicy(sizePolicy3);

        horizontalLayout->addWidget(tag);

        name = new QLineEdit(widget_2);
        name->setObjectName(QStringLiteral("name"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(name->sizePolicy().hasHeightForWidth());
        name->setSizePolicy(sizePolicy4);
        name->setMinimumSize(QSize(0, 0));
        name->setMaximumSize(QSize(16777215, 16777215));

        horizontalLayout->addWidget(name);

        horizontalLayout->setStretch(0, 1);
        horizontalLayout->setStretch(1, 4);

        content->addWidget(widget_2);

        groupBox = new QGroupBox(scrollAreaWidgetContents);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));
        sizePolicy3.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label, 0, 1, 1, 1);

        px = new QLineEdit(groupBox);
        px->setObjectName(QStringLiteral("px"));
        QSizePolicy sizePolicy5(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(px->sizePolicy().hasHeightForWidth());
        px->setSizePolicy(sizePolicy5);

        gridLayout_2->addWidget(px, 0, 2, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));
        sizePolicy3.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label_7, 0, 5, 1, 1);

        p0 = new QPushButton(groupBox);
        p0->setObjectName(QStringLiteral("p0"));
        sizePolicy3.setHeightForWidth(p0->sizePolicy().hasHeightForWidth());
        p0->setSizePolicy(sizePolicy3);
        p0->setMaximumSize(QSize(20, 20));

        gridLayout_2->addWidget(p0, 0, 0, 1, 1);

        pz = new QLineEdit(groupBox);
        pz->setObjectName(QStringLiteral("pz"));
        sizePolicy5.setHeightForWidth(pz->sizePolicy().hasHeightForWidth());
        pz->setSizePolicy(sizePolicy5);

        gridLayout_2->addWidget(pz, 0, 6, 1, 1);

        py = new QLineEdit(groupBox);
        py->setObjectName(QStringLiteral("py"));
        sizePolicy5.setHeightForWidth(py->sizePolicy().hasHeightForWidth());
        py->setSizePolicy(sizePolicy5);

        gridLayout_2->addWidget(py, 0, 4, 1, 1);

        r0 = new QPushButton(groupBox);
        r0->setObjectName(QStringLiteral("r0"));
        sizePolicy3.setHeightForWidth(r0->sizePolicy().hasHeightForWidth());
        r0->setSizePolicy(sizePolicy3);
        r0->setMaximumSize(QSize(20, 20));

        gridLayout_2->addWidget(r0, 1, 0, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));
        sizePolicy3.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label_5, 1, 3, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QStringLiteral("label_8"));
        sizePolicy3.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label_8, 1, 5, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));
        sizePolicy3.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label_4, 0, 3, 1, 1);

        rx = new QLineEdit(groupBox);
        rx->setObjectName(QStringLiteral("rx"));
        sizePolicy5.setHeightForWidth(rx->sizePolicy().hasHeightForWidth());
        rx->setSizePolicy(sizePolicy5);

        gridLayout_2->addWidget(rx, 1, 2, 1, 1);

        ry = new QLineEdit(groupBox);
        ry->setObjectName(QStringLiteral("ry"));
        sizePolicy5.setHeightForWidth(ry->sizePolicy().hasHeightForWidth());
        ry->setSizePolicy(sizePolicy5);

        gridLayout_2->addWidget(ry, 1, 4, 1, 1);

        sx = new QLineEdit(groupBox);
        sx->setObjectName(QStringLiteral("sx"));
        sizePolicy5.setHeightForWidth(sx->sizePolicy().hasHeightForWidth());
        sx->setSizePolicy(sizePolicy5);

        gridLayout_2->addWidget(sx, 2, 2, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));
        sizePolicy3.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label_3, 2, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));
        sizePolicy3.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label_2, 1, 1, 1, 1);

        s0 = new QPushButton(groupBox);
        s0->setObjectName(QStringLiteral("s0"));
        sizePolicy3.setHeightForWidth(s0->sizePolicy().hasHeightForWidth());
        s0->setSizePolicy(sizePolicy3);
        s0->setMaximumSize(QSize(20, 20));

        gridLayout_2->addWidget(s0, 2, 0, 1, 1);

        rz = new QLineEdit(groupBox);
        rz->setObjectName(QStringLiteral("rz"));
        sizePolicy5.setHeightForWidth(rz->sizePolicy().hasHeightForWidth());
        rz->setSizePolicy(sizePolicy5);

        gridLayout_2->addWidget(rz, 1, 6, 1, 1);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QStringLiteral("label_9"));
        sizePolicy3.setHeightForWidth(label_9->sizePolicy().hasHeightForWidth());
        label_9->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label_9, 2, 5, 1, 1);

        sz = new QLineEdit(groupBox);
        sz->setObjectName(QStringLiteral("sz"));
        sizePolicy5.setHeightForWidth(sz->sizePolicy().hasHeightForWidth());
        sz->setSizePolicy(sizePolicy5);

        gridLayout_2->addWidget(sz, 2, 6, 1, 1);

        sy = new QLineEdit(groupBox);
        sy->setObjectName(QStringLiteral("sy"));
        sizePolicy5.setHeightForWidth(sy->sizePolicy().hasHeightForWidth());
        sy->setSizePolicy(sizePolicy5);

        gridLayout_2->addWidget(sy, 2, 4, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));
        sizePolicy3.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label_6, 2, 3, 1, 1);


        content->addWidget(groupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        content->addItem(verticalSpacer);

        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout_4->addWidget(scrollArea);

        inspector->setWidget(inspectorView);
        Suede->addDockWidget(static_cast<Qt::DockWidgetArea>(1), inspector);
        menuBar = new QMenuBar(Suede);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 705, 23));
        file = new QMenu(menuBar);
        file->setObjectName(QStringLiteral("file"));
        file->setFocusPolicy(Qt::StrongFocus);
        help = new QMenu(menuBar);
        help->setObjectName(QStringLiteral("help"));
        Suede->setMenuBar(menuBar);

        menuBar->addAction(file->menuAction());
        menuBar->addAction(help->menuAction());
        file->addAction(screenCapture);
        file->addAction(exit);
        help->addAction(about);

        retranslateUi(Suede);

        QMetaObject::connectSlotsByName(Suede);
    } // setupUi

    void retranslateUi(QMainWindow *Suede)
    {
        Suede->setWindowTitle(QApplication::translate("Suede", "Suede", 0));
        exit->setText(QApplication::translate("Suede", "&Exit", 0));
        screenCapture->setText(QApplication::translate("Suede", "ScreenCapture", 0));
        screenCapture->setShortcut(QApplication::translate("Suede", "Ctrl+P", 0));
        about->setText(QApplication::translate("Suede", "About", 0));
        game->setWindowTitle(QApplication::translate("Suede", "Game", 0));
        hierarchy->setWindowTitle(QApplication::translate("Suede", "Hierarchy", 0));
        console->setWindowTitle(QApplication::translate("Suede", "Console", 0));
        inspector->setWindowTitle(QApplication::translate("Suede", "Inspector", 0));
        active->setText(QString());
        groupBox->setTitle(QApplication::translate("Suede", "Transform", 0));
        label->setText(QApplication::translate("Suede", "X", 0));
        label_7->setText(QApplication::translate("Suede", "Z", 0));
        p0->setText(QApplication::translate("Suede", "P", 0));
        r0->setText(QApplication::translate("Suede", "R", 0));
        label_5->setText(QApplication::translate("Suede", "Y", 0));
        label_8->setText(QApplication::translate("Suede", "Z", 0));
        label_4->setText(QApplication::translate("Suede", "Y", 0));
        label_3->setText(QApplication::translate("Suede", "X", 0));
        label_2->setText(QApplication::translate("Suede", "X", 0));
        s0->setText(QApplication::translate("Suede", "S", 0));
        label_9->setText(QApplication::translate("Suede", "Z", 0));
        label_6->setText(QApplication::translate("Suede", "Y", 0));
        file->setTitle(QApplication::translate("Suede", "&File", 0));
        help->setTitle(QApplication::translate("Suede", "&Help", 0));
    } // retranslateUi

};

namespace Ui {
    class Suede: public Ui_Suede {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SUEDE_H
