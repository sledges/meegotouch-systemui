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
#include "mcompositornotificationsink.h"
#include "notificationwidgetparameterfactory.h"
#include <MSceneManager>
#include <MScene>
#include <QApplication>
#include <MGConfItem>
#include <QTimer>
#include <QX11Info>
#include <X11/extensions/shape.h>
#include "x11wrapper.h"

#undef Bool

static const QString NOTIFICATION_PREVIEW_ENABLED = "/desktop/meego/notifications/previews_enabled";

MCompositorNotificationSink::MCompositorNotificationSink() :
        sinkDisabled(false),
        allPreviewsDisabled(false),
        window(NULL),
        currentBanner(NULL),
        touchScreenLockActive(false)
{
    notificationPreviewMode = new MGConfItem(NOTIFICATION_PREVIEW_ENABLED, this);
    changeNotificationPreviewMode();
    connect(notificationPreviewMode, SIGNAL(valueChanged()), this, SLOT(changeNotificationPreviewMode()));
#ifdef HAVE_QMSYSTEM
    connect(&displayState, SIGNAL(displayStateChanged(MeeGo::QmDisplayState::DisplayState)), this, SLOT(changeNotificationPreviewMode()));
#endif
    // Setup the timer which makes the banner disappear
    connect(&bannerTimer, SIGNAL(timeout()), this, SLOT(disappearCurrentBanner()));
    bannerTimer.setSingleShot(true);

    currentAppWindowAtom = X11Wrapper::XInternAtom(QX11Info::display(), "_MEEGOTOUCH_CURRENT_APP_WINDOW", False);
    notificationPreviewsDisabledAtom = X11Wrapper::XInternAtom(QX11Info::display(), "_MEEGOTOUCH_NOTIFICATION_PREVIEWS_DISABLED", False);
}

MCompositorNotificationSink::~MCompositorNotificationSink()
{
    // Destroy the queued banners; the current banner (if any) will get destroyed with the window so don't destroy it here
    foreach(MBanner *banner, bannerQueue) {
        delete banner;
    }
    delete window;
}

void MCompositorNotificationSink::createWindowIfNecessary()
{
    if (window == NULL) {
        // Set up the window
        window = new MWindow;
        window->setSceneManager(new MSceneManager);
        window->setTranslucentBackground(true);
        window->setAttribute(Qt::WA_X11DoNotAcceptFocus);
        window->setAttribute(Qt::WA_X11NetWmWindowTypeNotification);
        window->setObjectName("MCompositorNotificationSinkWindow");
        window->setWindowTitle("Notification");
        window->setProperty("followsCurrentApplicationWindowOrientation", true);

        // This fixes bug #281758. For some reason the window is not transparent by default.
        XSetWindowBackground(QX11Info::display(), window->effectiveWinId(), 0);

        // Clear the mask for the duration of orientation change, because the mask is not rotated along with the notification
        connect(window->sceneManager(), SIGNAL(orientationAboutToChange(M::Orientation)), this, SLOT(clearWindowMask()));
        connect(window->sceneManager(), SIGNAL(orientationChangeFinished(M::Orientation)), this, SLOT(updateWindowMask()));
        connect(window, SIGNAL(displayEntered()), this, SLOT(addOldestBannerToWindow()));
    }
}

void MCompositorNotificationSink::addNotification(const Notification &notification)
{
    if (!canAddNotification(notification) || !containsText(notification)) {
        return;
    }

    // Check whether notification previews are disabled in current conditions
    PreviewMode previewMode = currentApplicationPreviewMode();
    if (allPreviewsDisabled ||
        (notification.type() == Notification::ApplicationEvent && (sinkDisabled || (previewMode & ApplicationEventsDisabled) != 0)) ||
        (notification.type() == Notification::SystemEvent && (previewMode & SystemEventsDisabled) != 0)) {
        // Notification previews are disabled
        if (notification.type() == Notification::ApplicationEvent) {
            // Transfer the notification onwards immediately
            emit notificationAdded(notification);
        } else {
            // System notifications need to be removed after they've been shown to avoid leaking and here they can be considered to be "shown"
            emit notificationRemovalRequested(notification.notificationId());
        }
        return;
    }

    if (notificationIds.contains(notification.notificationId())) {
        // The notification already exists so update it
        updateNotification(notification);
    } else {
#ifdef HAVE_QMSYSTEM
        if (window != NULL && displayState.get() == MeeGo::QmDisplayState::Off) {
            // When a notification comes in while the display is off, remove any other banners from the queue
            removeBannersFromQueue();
        }
#endif

        // Store the ID of the notification
        notificationIds.insert(notification.notificationId());

        // Create and set up info banner widget
        MBanner *banner = createInfoBanner(notification);
        banner->setStyleName(banner->objectName() == "EventBanner" ? "ShortEventBanner" : "SystemBanner");
        banner->setProperty("notificationId", notification.notificationId());
        banner->setProperty("timeout", notification.timeout());
        updateImage(banner, notification.parameters());

        // Connect slots to cleanup disappearing banner and handle next banner when disappeared
        // The banner sends disappear either by timeout or by user clicking on it
        connect(banner, SIGNAL(disappeared()), this, SLOT(currentBannerDone()));

        // Keep track of the mapping between IDs and banners
        idToBanner.insert(notification.notificationId(), banner);
        bannerQueue.append(banner);
        emit notificationAdded(notification);

        // Create the window if it does not yet exist
        createWindowIfNecessary();

        if (!window->isVisible()) {
            window->show();

            // This fixes bug #289583. For some reason the window is not transparent after opening it when MeeGo graphics system is forced.
            window->repaint();

            // Calling hide() causes the onDisplay property of the window to change with delay,
            // so if show() is called fast enough after hiding, onDisplay is never changed to false and we
            // never get a new displayEntered signal, so go to the slot immediately in that case.
            if(window->isOnDisplay()) {
                addOldestBannerToWindow();
            }
        }
    }
}

void MCompositorNotificationSink::updateNotification(const Notification &notification)
{
    MBanner *banner = idToBanner.value(notification.notificationId());

    if (banner != NULL) {
        // If the notification maps to a banner, update it
        banner->setTitle(infoBannerTitleText(notification.parameters()));
        banner->setSubtitle(infoBannerSubtitleText(notification.parameters()));
        banner->setProperty("notificationId", notification.notificationId());
        banner->setProperty("timeout", notification.timeout());

        // Update the info banner's image and actions
        updateImage(banner, notification.parameters());
        updateActions(banner, notification.parameters());
    }
}

void MCompositorNotificationSink::removeNotification(uint notificationId)
{
    notificationIds.remove(notificationId);

    MBanner *banner = idToBanner.take(notificationId);
    if (banner != NULL) {
        if(currentBanner == banner) {
            // The banner is on the screen, so make it disappear
            bannerTimer.stop();
            if (window != NULL) {
                window->sceneManager()->disappearSceneWindow(banner);
            }
        } else {
            // The banner is in the queue - remove it
            bannerQueue.removeAll(banner);
            delete banner;
        }
    }
}

void MCompositorNotificationSink::disappearCurrentBanner()
{
    if (currentBanner != NULL && window != NULL) {
        window->sceneManager()->disappearSceneWindow(currentBanner);
    }
}

void MCompositorNotificationSink::currentBannerDone()
{
    bannerDone(currentBanner);
    currentBanner = NULL;

    addOldestBannerToWindow();
}

void MCompositorNotificationSink::bannerDone(MBanner *banner)
{
    if (banner != NULL) {
        // Do not use the notification id associated with the banner
        // to remove the banner from "id to banner mapping" since the
        // original notification may have already been removed and a
        // new notification with the same id may have already been added.
        int id = idToBanner.key(banner, -1);
        if (id != -1) {
            idToBanner.remove(id);

            if (banner->styleName() == "SystemBanner") {
                // System notifications need to be removed after they've been shown to avoid leaking
                emit notificationRemovalRequested(id);
            }
        }
    }
}

void MCompositorNotificationSink::addOldestBannerToWindow()
{
    if (currentBanner == NULL) {
        // A banner can only be added if there is no current banner
        if (!bannerQueue.isEmpty()) {
            // The oldest banner should be shown
            currentBanner = bannerQueue.takeFirst();
            if (window != NULL) {
                window->sceneManager()->appearSceneWindow(currentBanner, MSceneWindow::DestroyWhenDone);
            }
            bannerTimer.start(currentBanner->property("timeout").toInt());
            updateWindowMask(currentBanner);
        } else {
            // No more banners exist to be shown -> hide the window
            if (window != NULL) {
                window->hide();
            }
        }
    }
}

void MCompositorNotificationSink::setApplicationEventsDisabled(bool disabled)
{
    sinkDisabled = disabled;
}

void MCompositorNotificationSink::updateWindowMask()
{
    if (currentBanner != NULL) {
        updateWindowMask(currentBanner);
    }
}

void MCompositorNotificationSink::updateWindowMask(MBanner* banner)
{
    if (window != NULL) {
        QSize size = banner->preferredSize().toSize();
        QPoint origin;

        switch(window->sceneManager()->orientationAngle()) {
        case M::Angle90:
            size.transpose();
            origin.setX(window->width() - size.width() - banner->pos().y());
            origin.setY(banner->pos().x());
            break;
        case M::Angle270:
            size.transpose();
            origin.setX(banner->pos().y());
            origin.setY(-banner->pos().x());
            break;
        case M::Angle180:
            origin.setY(window->height() - size.height() - banner->pos().y());
            origin.setX(-banner->pos().x());
            break;
        default:
            origin = banner->pos().toPoint();
            break;
        }

        Display *dpy = QX11Info::display();
        XRectangle rect;
        rect.x = origin.x();
        rect.y = origin.y();
        rect.width = size.width();
        rect.height = size.height();
        XserverRegion shapeRegion = X11Wrapper::XFixesCreateRegion(dpy, &rect, 1);
        X11Wrapper::XFixesSetWindowShapeRegion(dpy, window->winId(), ShapeInput, 0, 0, shapeRegion);
        X11Wrapper::XFixesDestroyRegion(dpy, shapeRegion);
        X11Wrapper::XSync(dpy, False);
    }
}

void MCompositorNotificationSink::clearWindowMask()
{
    if (window != NULL) {
        Display *dpy = QX11Info::display();
        X11Wrapper::XFixesSetWindowShapeRegion(dpy, window->winId(), ShapeInput, 0, 0, 0);
        X11Wrapper::XSync(dpy, False);
    }
}

void MCompositorNotificationSink::changeNotificationPreviewMode()
{
    QVariant gconfValue = notificationPreviewMode->value();
    if (gconfValue.isValid() && (gconfValue.type() == QVariant::Bool)) {
        allPreviewsDisabled = !gconfValue.toBool();
    }

#ifdef HAVE_QMSYSTEM
    // Always disable all previews when the display is off and the touch screen is locked
    allPreviewsDisabled |= (touchScreenLockActive && displayState.get() == MeeGo::QmDisplayState::Off);
#endif
}

void MCompositorNotificationSink::updateImage(MBanner *infoBanner, const NotificationParameters &parameters)
{
    QString imageId = parameters.value(NotificationWidgetParameterFactory::imageIdKey()).toString();
    QString previewIconId = parameters.value(NotificationWidgetParameterFactory::previewIconIdKey()).toString();
    if (imageId.isEmpty() && !previewIconId.isEmpty()) {
        // Icon has not been overridden by an image but there is a preview icon to be used
        infoBanner->setIconID(previewIconId);
    } else {
        // Default behavior: use either image or icon
        WidgetNotificationSink::updateImage(infoBanner, parameters);
    }
}

MCompositorNotificationSink::PreviewMode MCompositorNotificationSink::currentApplicationPreviewMode()
{
    Atom actualType;
    int actualFormat;
    unsigned long numItemsReturn, bytesLeft;
    unsigned char *data = NULL;
    PreviewMode previewMode = AllEventsEnabled;

    Display *display = QX11Info::display();

    Status result = X11Wrapper::XGetWindowProperty(display, QX11Info::appRootWindow(),
                                                   currentAppWindowAtom, 0L, 1L, False, XA_WINDOW,
                                                   &actualType, &actualFormat, &numItemsReturn, &bytesLeft, &data);
    if (result == Success && numItemsReturn) {
        Window currentApp = *(Window *)data;
        X11Wrapper::XFree(data);

        result = X11Wrapper::XGetWindowProperty(display, currentApp,
                                                notificationPreviewsDisabledAtom, 0L, 1L, False, XA_INTEGER,
                                                &actualType, &actualFormat, &numItemsReturn, &bytesLeft, &data);
        if (result == Success && numItemsReturn) {
            previewMode = (PreviewMode)(*(int *)data);
            X11Wrapper::XFree(data);
        }
    }

    return previewMode;
}

void MCompositorNotificationSink::setTouchScreenLockActive(bool active)
{
    touchScreenLockActive = active;
    changeNotificationPreviewMode();
}

void MCompositorNotificationSink::removeBannersFromQueue()
{
    // Remove references to all banners
    foreach (MBanner *banner, bannerQueue) {
        bannerDone(banner);
        delete banner;
    }
    bannerQueue.clear();

    if (currentBanner != NULL) {
        // Disappear any banner currently being displayed
        window->sceneManager()->disappearSceneWindowNow(currentBanner);
    }
}
