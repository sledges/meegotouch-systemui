/****************************************************************************
 **
 ** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (directui@nokia.com)
 **
 ** This file is part of systemui.
 **
 ** If you have questions regarding the use of this file, please contact
 ** Nokia at directui@nokia.com.
 **
 ** This library is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public
 ** License version 2.1 as published by the Free Software Foundation
 ** and appearing in the file LICENSE.LGPL included in the packaging
 ** of this file.
 **
 ****************************************************************************/
#include "shutdownui.h"
#include "sysuid.h"

#include <MLabel>
#include <MFeedback>
#include <MApplication>
#include <QTimer>
#include <QGraphicsLinearLayout>
#include <MSceneManager>
#include <MStylableWidget>
#include <MLocale>
#include <QX11Info>
#include "x11wrapper.h"

#ifdef HAVE_QMSYSTEM
#include <qmdisplaystate.h>
#endif

#include "mwidgetcreator.h"
M_REGISTER_WIDGET_NO_CREATE(ShutdownUI)

ShutdownUI::ShutdownUI() :
    MWindow((QWidget *)NULL),
    realized(false),
    sceneWindow(0),
    timer(new QTimer(this)),
    label1(0),
    label2(0),
    logo(0),
    layout(0),
    feedback(0)
{
    setWindowTitle("ShutdownUI");
    setObjectName("ShutdownUIWindow");

    // Pre-create/load the shutdown UI content to show it quickly when requested
    QTimer::singleShot(5000, this, SLOT(realize()));

    connect(timer, SIGNAL(timeout()), this, SLOT(showLogo()));
}

ShutdownUI::~ShutdownUI()
{
    delete sceneWindow;
}

void ShutdownUI::realize()
{
    if(realized) {
        return;
    }

    // Initialize non-graphical feedback
    feedback = new MFeedback(this);
    feedback->setName("power-off");

    //% "Shutting down"
    label1 = new MLabel(qtTrId("qtn_shut_down"));
    label1->setAlignment(Qt::AlignCenter);
    label1->setObjectName("shutdownTextFirst");

    //% "Good bye!"
    label2 = new MLabel(qtTrId("qtn_shut_greeting"));
    label2->setAlignment(Qt::AlignCenter);
    label2->setObjectName("shutdownTextSecond");

    // A full screen logo shown when the labels are already gone
    logo = new MStylableWidget;
    logo->setObjectName("shutdownLogo");

    layout = new QGraphicsLinearLayout(Qt::Vertical);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addItem(label1);
    layout->addItem(label2);

    // Create a scene window and put everything into it
    sceneWindow = new MSceneWindow;
    sceneWindow->setObjectName("shutdownWindow");
    sceneWindow->setContentsMargins(0, 0, 0, 0);
    sceneWindow->setLayout(layout);
    sceneWindow->appear(this);

    realized = true;
}

void ShutdownUI::showWindow(const QString &text1, const QString &text2, int timeout)
{
    // If the widgets are not created we create them now
    realize();

    // Set the labels to show the text strings that we got
    if (!(text1.isEmpty() && text2.isEmpty())) {
        if (label1 != NULL) {
            if (text1.startsWith("qtn")) {
                label1->setText(qtTrId(text1.toLatin1().constData()));
            } else {
                label1->setText(text1);
            }
        }

        if (label2 != NULL) {
            if (text2.startsWith("qtn")) {
                label2->setText(qtTrId(text2.toLatin1().constData()));
            } else {
                label2->setText(text2);
            }
        }
    }

    timer->stop();

    show();

    feedback->play();

    // Set the interval and start the timer to the next phase: hiding the labels and showing the logo
    timer->setInterval(timeout);
    timer->start();
}

void ShutdownUI::showLogo()
{
    timer->stop();

    // Hide the labels and show the image
    for (int i = layout->count(); i > 0; i--) {
        layout->removeAt(0);
    }
    layout->addItem(logo);

#if 0
    /* Logo available only for portait ATM */
    sceneManager ()->setOrientationAngle (M::Angle0, MSceneManager::ImmediateTransition);
    lockOrientation ();
#endif

    delete label1;
    delete label2;
    label1 = NULL;
    label2 = NULL;

    QTimer::singleShot(2000, this, SLOT(turnOffScreen()));
}

void ShutdownUI::turnOffScreen()
{
    bool success = false;

    // No way dimming or turning off the screen inside scratchbox
#if defined(HAVE_QMSYSTEM)
    MeeGo::QmDisplayState display;

    // Try to dim
    success = display.set(MeeGo::QmDisplayState::Dimmed);

    // Try to turn off
    success &= display.set(MeeGo::QmDisplayState::Off);
#endif

    if (!success) {
        QPalette Palette(palette());

        Palette.setColor(QPalette::Background, QColor("black"));
        setPalette(Palette);

        setBackgroundRole(QPalette::Background);
        logo->hide();
    }
}

void ShutdownUI::showEvent(QShowEvent *event)
{
    MWindow::showEvent(event);
    
    // Set the stacking layer
    Display *display = QX11Info::display();
    Atom stackingLayerAtom = X11Wrapper::XInternAtom(display, "_MEEGO_STACKING_LAYER", False);
    if (stackingLayerAtom != None) {
        long layer = 6;
        X11Wrapper::XChangeProperty(display, internalWinId(), stackingLayerAtom, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&layer, 1);
    }
}
