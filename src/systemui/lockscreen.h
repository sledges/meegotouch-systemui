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
#ifndef LOCKSCREEN_H
#define LOCKSCREEN_H

#include <MSceneWindow>

class MWindow;

class LockScreen : public MSceneWindow
{
    Q_OBJECT

public:
    LockScreen(MWindow* parent);
    virtual ~LockScreen();
    void updateDateTime ();
    void reset ();
    MWindow* parent();

signals:
    void unlocked ();
    void dateTimeChanged();

protected:
    void createContent ();

private slots:
    void sliderUnlocked ();
};

#endif
