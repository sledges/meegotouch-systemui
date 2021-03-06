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
#include "shutdownwindowstyle.h"
#include "sysuid.h"

#include <MTheme>
#include <MLabel>
#include <MFeedback>
#include <MImageWidget>
#include <MSceneWindow>
#include <QTimer>
#include <QGraphicsLinearLayout>
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

    applyStyle();

    // Precreate the shutdown UI content to show it quickly when requested but only after the status indicator menu has been created
    QTimer::singleShot(12000, this, SLOT(realize()));

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

    setBackgroundBrush(Qt::black);

    // Initialize non-graphical feedback
    feedback = new MFeedback(this);
    feedback->setName("priority1_power_off");

    //% "Shutting down"
    label1 = new MLabel(qtTrId("qtn_shut_down"));
    label1->setAlignment(Qt::AlignCenter);
    label1->setObjectName("shutdownTextFirst");

    //% "Good bye!"
    label2 = new MLabel(qtTrId("qtn_shut_greeting"));
    label2->setAlignment(Qt::AlignCenter);
    label2->setObjectName("shutdownTextSecond");

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

void disableCompositorSwipeRect(QWidget *window, const QRectF &rect)
{
    if (window != NULL) {
        unsigned int customRegion[] = { rect.x(), rect.y(), rect.width(), rect.height() };
        Display *dpy = QX11Info::display();
        Atom customRegionAtom = X11Wrapper::XInternAtom(dpy, "_MEEGOTOUCH_CUSTOM_REGION", False);
        X11Wrapper::XChangeProperty(dpy, window->winId(), customRegionAtom, XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<unsigned char *>(&customRegion[0]), 4);
        X11Wrapper::XSync(dpy, False);
    }
}

void ShutdownUI::showWindow(const QString &text1, const QString &text2, int timeout)
{
    // If the widgets are not created create them now
    realize();

    // If the widgets have been created once but the labels are NULL the logo has already been shown so do nothing
    if (label1 == NULL || label2 == NULL) {
        return;
    }

    timer->stop();

    if (!(text1.isEmpty() && text2.isEmpty())) {
        // Set the labels to show the received text strings
        label1->setText(text1.startsWith("qtn") ? qtTrId(text1.toLatin1().constData()) : text1);
        label2->setText(text2.startsWith("qtn") ? qtTrId(text2.toLatin1().constData()) : text2);

        // Set the interval and start the timer to the next phase: hiding the labels and showing the logo
        timer->start(timeout);
    } else {
        // No text to show so show the logo immediately
        showLogo();
    }

    show();
    raise();
    disableCompositorSwipeRect(this, rect());

    feedback->play();
}

void ShutdownUI::showLogo()
{
    timer->stop();

    // Create the logo and make it invisible until it's in the correct position
    const ShutdownWindowStyle *style = static_cast<const ShutdownWindowStyle *>(MTheme::style("ShutdownWindowStyle"));
    logo = new MImageWidget(style->image());
    logo->setZoomFactor(1);
    logo->hide();
    MTheme::releaseStyle(style);

    // Destroy the labels (which will also remove them from the layout), lower one first
    delete label2;
    delete label1;
    label1 = NULL;
    label2 = NULL;

    // Make sure the logo is laid out to the correct position before showing it
    layout->addStretch();
    layout->addItem(logo);
    layout->addStretch();
    layout->activate();
    logo->show();

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

void ShutdownUI::applyStyle()
{
    const ShutdownWindowStyle *style = static_cast<const ShutdownWindowStyle *>(MTheme::style("ShutdownWindowStyle"));

    if (style->lockedOrientation() == "landscape") {
        setLandscapeOrientation();
        setOrientationAngle(M::Angle0);
        setOrientationLocked(true);
        setOrientationAngleLocked(true);
    } else if (style->lockedOrientation() == "portrait") {
        setPortraitOrientation();
        setOrientationAngle(M::Angle270);
        setOrientationLocked(true);
        setOrientationAngleLocked(true);
    } else {
        setOrientationLocked(false);
        setOrientationAngleLocked(false);
    }

    MTheme::releaseStyle(style);
}
