include(../common_top.pri)
TARGET = ut_sysuid
INCLUDEPATH += $$SRCDIR/notifications $$SRCDIR/statusarea $$SRCDIR/unlocksliderwidget
INCLUDEPATH += /usr/include/contextsubscriber

# unit test and unit classes
SOURCES += \
    $$SRCDIR/debug.cpp \
    $$SRCDIR/shutdownui.cpp \
    $$SRCDIR/usbmodes.cpp \
    $$SRCDIR/usbui.cpp \
    $$SRCDIR/lockscreenui.cpp \
    $$SRCDIR/shutdownbusinesslogic.cpp \
    $$SRCDIR/ledbusinesslogic.cpp \
    $$SRCDIR/ledbusinesslogicadaptor.cpp \
    $$SRCDIR/batterybusinesslogic.cpp \
    $$SRCDIR/batterybusinesslogicadaptor.cpp \
    $$SRCDIR/usbbusinesslogic.cpp \
    $$SRCDIR/usbbusinesslogicadaptor.cpp \
    $$SRCDIR/lockscreenbusinesslogic.cpp \
    $$SRCDIR/lockscreenbusinesslogicadaptor.cpp \
    $$SRCDIR/systemuigconf.cpp \
    $$SRCDIR/sysuidrequest.cpp \
    $$SRCDIR/sysuid.cpp \
    ut_sysuid.cpp \

# service classes
SOURCES += \
    $$STUBSDIR/stubbase.cpp \
    $$STUBSDIR/ledstub.cpp \
    $$STUBSDIR/batterystub.cpp \
    $$STUBSDIR/devicemodestub.cpp \
    $$STUBSDIR/systemstatestub.cpp \
    $$STUBSDIR/displaystatestub.cpp \
    $$STUBSDIR/locksstub.cpp \
    $$STUBSDIR/testcontextitem.cpp \

# unit test and unit classes
HEADERS += \
    $$SRCDIR/debug.h \
    $$SRCDIR/shutdownui.h \
    $$SRCDIR/usbmodes.h \
    $$SRCDIR/usbui.h \
    $$SRCDIR/lockscreenui.h \
    $$SRCDIR/shutdownbusinesslogic.h \
    $$SRCDIR/ledbusinesslogic.h \
    $$SRCDIR/ledbusinesslogicadaptor.h \
    $$SRCDIR/batterybusinesslogic.h \
    $$SRCDIR/batterybusinesslogicadaptor.h \
    $$SRCDIR/usbbusinesslogic.h \
    $$SRCDIR/usbbusinesslogicadaptor.h \
    $$SRCDIR/lockscreenbusinesslogic.h \
    $$SRCDIR/lockscreenbusinesslogicadaptor.h \
    $$SRCDIR/systemuigconf.h \
    $$SRCDIR/sysuidrequest.h \
    $$SRCDIR/sysuid.h \
    $$SRCDIR/contextframeworkcontext.h \
    ut_sysuid.h \
    $$SRCDIR/notifications/eventtypestore.h \
    $$SRCDIR/notifications/notificationsink.h \
    $$SRCDIR/notifications/widgetnotificationsink.h \
    $$SRCDIR/notifications/duicompositornotificationsink.h \
    $$SRCDIR/notifications/duifeedbacknotificationsink.h \
    $$SRCDIR/notifications/notificationmanager.h \
    $$SRCDIR/unlocksliderwidget/unlockslider.h \
    $$SRCDIR/statusarea/statusareawindow.h \
    $$SRCDIR/applicationcontext.h \

# service classes
HEADERS += \
    $$STUBSDIR/stubbase.h \
    $$STUBSDIR/ledstub.h \
    $$STUBSDIR/batterystub.h \
    $$STUBSDIR/devicemodestub.h \
    $$STUBSDIR/systemstatestub.h \
    $$STUBSDIR/displaystatestub.h \
    $$STUBSDIR/locksstub.h \
    $$STUBSDIR/testcontextitem.h \
    $$STUBSDIR/widgetnotificationsink_stub.h \
    $$STUBSDIR/eventtypestore_stub.h \
    $$STUBSDIR/notificationmanager_stub.h \
    $$STUBSDIR/statusareawindow_stub.h \
    $$STUBSDIR/unlockslider_stub.h \

include(../common_bot.pri)