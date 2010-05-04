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

#ifndef STATUSAREARENDERER_H_
#define STATUSAREARENDERER_H_

#include <MNamespace>
#include "qmdisplaystate.h"

class QGraphicsScene;
class StatusArea;

/*!
 *  StatusAreaRenderer renders the contents of the scene to a shared pixmap which is then shown by libmeegotouch.
 */
class StatusAreaRenderer : public QObject
{
    Q_OBJECT

public:
    /*!
     * Creates a new status area renderer
     *
     * \param statusArea the status area to be displayed in this window
     * \param parent the parent widget
     */
    StatusAreaRenderer(QWidget *parent = NULL);

    /*!
     * Destroys the status area renderer
     */
    virtual ~StatusAreaRenderer();

private slots:
   /*!
    * \brief A slot for notifying that the scene has changed and needs to be painted
    */
    virtual void sceneChanged(const QList<QRectF> &region);

    /*!
     * \brief A slot for setting if the scene renders to pixmap or not.
     * \param Maemo::QmDisplayState::DisplayState. When Maemo::QmDisplayState::Off / Maemo::QmDisplayState::Dimmed the scene does not render. When Maemo::QmDisplayState::On scene renders to pixmap
     */
    void setSceneRender(Maemo::QmDisplayState::DisplayState state);

signals:
    /*!
     * Signal that status indicator menu is visible
     */
    void statusIndicatorMenuVisibilityChanged(bool visible);

private:
    //! Scene which is rendererd
    QGraphicsScene *scene;

    //! The status area to be displayed in this window
    StatusArea *statusArea_;

    //! Shared Pixmap between libmeegotouch and systemui for the status area.
    QPixmap* statusAreaPixmap;

    //! creates a pixmap and writes the handle to a temp file
    bool createSharedPixmapHandle();

    //! set the status bar size with information from style
    void setSizeFromStyle();

    //! Status Area dimensions.
    uint statusAreaHeight;
    uint statusAreaWidth;
 
    //! Keep track of device display state
    Maemo::QmDisplayState* displayState;
    //! Keep track whether scene should render or not
    bool renderScene;

#ifdef UNIT_TEST
    friend class Ut_StatusAreaRenderer;
#endif

};

#endif /* STATUSAREARENDERER_H_ */