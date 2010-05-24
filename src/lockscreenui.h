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
/* -*- Mode: C; indent-tabs-mode: s; c-basic-offset: 4; tab-width: 4 -*- */
/* vim:set et ai sw=4 ts=4 sts=4: tw=80 cino="(0,W2s,i2s,t0,l1,:0" */
#ifndef LOCKSCREENUI_H
#define LOCKSCREENUI_H

#include <MWindow>
#include <QBasicTimer>

class QPixmap;
class MWidget;
class MLinearLayoutPolicy;
class MGConfItem;

class LockScreenUI : public MWindow
{
    Q_OBJECT

public:
    LockScreenUI ();
    virtual ~LockScreenUI();

    virtual void paint (QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget *widget = 0);

signals:
    void unlocked ();

public slots:
    void updateDateTime ();

protected:
    void createContent ();

private slots:
    void realize ();
    void sliderUnlocked ();
    void reloadLandscapeBackground ();
    void reloadPortraitBackground ();
    void updateMissedEvents (int emails,
                             int messages,
                             int calls,
                             int im);

private:
    bool             m_Realized;
    MLinearLayoutPolicy *m_policy;

    MWidget         *m_notificationArea;
    MWidget         *m_LockLiftArea;
    MWidget         *m_LockLandArea;

    QPixmap          m_bgLandscape;
    QPixmap          m_bgPortrait;
    MGConfItem      *m_confBgLandscape;
    MGConfItem      *m_confBgPortrait;
};

class EventEaterUI : public MWindow
{
    Q_OBJECT

public:
    EventEaterUI ();

protected:
    void createContent ();

protected:
    void mousePressEvent (QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent (QGraphicsSceneMouseEvent *event);

signals:
    void OneInput ();
};

#endif
