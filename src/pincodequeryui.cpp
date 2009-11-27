#include "pincodequeryui.h"

#include <DuiWidget>
#include <DuiApplication>
#include <DuiApplicationWindow>
#include <DuiLocale>
#include <DuiTextEdit>
#include <DuiTheme>
#include <DuiSceneManager>
#include <DuiGrid>
#include <DuiGridLayoutPolicy>
#include <DuiButton>
#include <DuiLabel>
#include <DuiTextEdit>

#include <QStringList>
#include <QSizePolicy>
#include <QTimer>
#include <QDebug>
#include <QIntValidator>


PinCodeQueryUI::PinCodeQueryUI()
{
    qDebug() << Q_FUNC_INFO;
    setVisible(false);
    setFullscreen(true);
    setDisplayMode(0);
    setBackButtonEnabled(false);
    setTitle("PIN code query:");
    createContent();
}

PinCodeQueryUI::~PinCodeQueryUI()
{
    qDebug() << Q_FUNC_INFO << (QObject*)this;

    if(backspaceTimer != NULL) {
        //we stop timing the press event
        backspaceTimer->stop();
    }
}

void PinCodeQueryUI::createContent()
{
    DuiApplicationPage::createContent();

    //create the widgets and numpad.
    createWidgetItems();

    //create mainLayout
    DuiLayout *layout = new DuiLayout(centralWidget());

    //create landscapePolicy and portraitPolicy for layout
    DuiGridLayoutPolicy* landscapePolicy = new DuiGridLayoutPolicy(layout);
    DuiGridLayoutPolicy* portraitPolicy = new DuiGridLayoutPolicy(layout);

    layout->setLandscapePolicy(landscapePolicy); // ownership transferred
    layout->setPortraitPolicy(portraitPolicy); // ownership transferred

    DuiLayout* nums = createNumpad();

    //attach widget items to landscape and portrait policies
    /*
    Note that the columns and rows are not equally wide or tall. If you want two
    columns to have the same width, you must set their minimum widths and stretch
    factors to be the same yourself. You do this using setColumnMinimumWidth() and
    setColumnStretch().
    */
    // addItemAtPosition (QGraphicsLayoutItem *item, int row, int column, int rowSpan, int columnSpan, Qt::Alignment alignment=0)
    landscapePolicy->addItemAtPosition(headerLabel, 0, 0, 1, 5);
    landscapePolicy->addItemAtPosition(emergencyCallButton, 0, 6);
    landscapePolicy->addItemAtPosition(entryTextEdit, 3, 0, 1, 3);
    landscapePolicy->addItemAtPosition(backspaceButton, 3, 3);
    landscapePolicy->addItemAtPosition(enterButton, 5, 0, 1, 2);
    landscapePolicy->addItemAtPosition(cancelButton, 5, 2, 1, 2);
    landscapePolicy->addItemAtPosition(nums, 1, 4, 5, 7);

    portraitPolicy->addItemAtPosition(headerLabel, 0, 0, 1, 4);
    portraitPolicy->addItemAtPosition(emergencyCallButton, 1, 3);
    portraitPolicy->addItemAtPosition(entryTextEdit, 2, 0, 1, 3);
    portraitPolicy->addItemAtPosition(backspaceButton, 2, 3);
    portraitPolicy->addItemAtPosition(nums, 3, 0, 1, 4);
    portraitPolicy->addItemAtPosition(enterButton, 4, 0, 1, 2);
    portraitPolicy->addItemAtPosition(cancelButton, 4, 2, 1, 2);

    //set column and row sizes
    qreal left,top,right,bottom;
    layout->getContentsMargins(&left,&top,&right,&bottom);
    qDebug() << Q_FUNC_INFO << "margins l:"<<left<<"r:"<<right<<"t:"<<top<<"b:"<<bottom;

    DuiApplicationWindow* win = DuiApplication::activeApplicationWindow();
    if(win){
        QSize size = win->sceneManager()->visibleSceneSize();
        qDebug() << Q_FUNC_INFO << "size w x h:"<<size.width()<<"x"<<size.height();
        int longSide = size.width();
        int shortSide = size.height();
        if(longSide < shortSide)
        {
            int tmp = longSide;
            longSide = shortSide;
            shortSide = tmp;
        }
        qDebug() << Q_FUNC_INFO << "size l x s:"<<longSide<<"x"<<shortSide;
        longSide -= left + right;
        shortSide -= top + bottom;

        int half = longSide/2;
        int quarter = shortSide/4;
        int eight = longSide/8;

        int sixth = shortSide/6;
        for(int i=0; i<6;++i) {
            landscapePolicy->setRowFixedHeight(i, sixth);
        }

        for(int i=0; i<8; ++i) {
            landscapePolicy->setColumnFixedWidth(i, eight);
        }
        for(int i=0; i<5; ++i) {
            if(i == 3 ) {
                portraitPolicy->setRowFixedHeight(i, half);
            }
            else {
                portraitPolicy->setRowFixedHeight(i, eight);
            }
        }
        for(int i=0; i<4;++i) {
            portraitPolicy->setColumnFixedWidth(i, quarter);
        }

        // button layouts
        int side = 36;
        QSizeF minSize(landscapePolicy->columnMinimumWidth(0), side);
        landscapePolicy->setRowAlignment(5, Qt::AlignBottom);
        portraitPolicy->setRowAlignment(4, Qt::AlignBottom);

        enterButton->setMinimumSize( minSize );
        cancelButton->setMinimumSize( minSize );
    }
    backspaceTimer = NULL;
}

DuiButton *PinCodeQueryUI::getEmergencyBtn()
{
    return emergencyCallButton;
}
DuiButton *PinCodeQueryUI::getCancelBtn()
{
    return cancelButton;
}
DuiButton *PinCodeQueryUI::getEnterBtn()
{
    return enterButton;
}
DuiTextEdit *PinCodeQueryUI::getCodeEntry()
{
    return entryTextEdit;
}
void PinCodeQueryUI::setHeader(QString header)
{
    headerLabel->setText(header);
}


void PinCodeQueryUI::createWidgetItems()
{    
    emergencyCallButton = new DuiButton();
    emergencyCallButton->setObjectName("emergencyCallButton");
    emergencyCallButton->setIconID("icon-m-common-emergency-call");
    connect(emergencyCallButton, SIGNAL(released()), this, SLOT(buttonReleased()));

    entryTextEdit = new DuiTextEdit(DuiTextEditModel::SingleLine, "");
    entryTextEdit->setObjectName("codeEntry");
    entryTextEdit->setMaskedInput(true);

    enterButton = new DuiButton(QString(trid("qtn_cell_enter", "Enter")));
    enterButton->setObjectName("enterButton");
    connect(enterButton, SIGNAL(released()), this, SLOT(buttonReleased()));

    cancelButton = new DuiButton(QString(trid("qtn_comm_cancel", "Cancel")));
    cancelButton->setObjectName("cancelButton");
    connect(cancelButton, SIGNAL(released()), this, SLOT(buttonReleased()));

    backspaceButton = new DuiButton();
    backspaceButton->setObjectName("backspaceButton");
    backspaceButton->setIconID("icon-m-keyboard-backspace");
    connect(backspaceButton, SIGNAL(released()), this, SLOT(buttonReleased()));
    connect(backspaceButton, SIGNAL(pressed()), this, SLOT(buttonPressed()));

    headerLabel = new DuiLabel();
    headerLabel->setAlignment(Qt::AlignCenter);    
}

DuiLayout* PinCodeQueryUI::createNumpad()
{
    //create numpadLayout and policy for that
    DuiLayout *numpadLayout = new DuiLayout(this);
    DuiGridLayoutPolicy *numpadLayoutPolicy = new DuiGridLayoutPolicy(numpadLayout);

    int values[] = {
        1, 2, 3,
        4, 5, 6,
        7, 8, 9,
        0
    };

    for (unsigned int i=0; i<sizeof(values)/sizeof(values[0]); ++i) {	
        QString str1 = QString("qtn_cell_dialer_").append(QString::number(values[i]));
        QString str2 = QString::number(values[i]);
        QString str3 = QString("numpadButton" + str2);
        DuiButton *num = new DuiButton(QString(trid(str1.toLatin1(), str2.toLatin1())), this);
        num->setObjectName(str3);
        connect(num, SIGNAL(released()), this, SLOT(buttonReleased()));
        if(values[i] == 0) {
            numpadLayoutPolicy->addItemAtPosition(num, (i/3), 1, 1, 1);
        } else {
            numpadLayoutPolicy->addItemAtPosition(num, (i/3), i%3, 1, 1);
        }
    }
    return numpadLayout;
}

void PinCodeQueryUI::buttonReleased()
{    
    DuiButton* button = static_cast<DuiButton*>(this->sender());

    //Check if the button was a numpad button       
    if(button->objectName().left(button->objectName().length()-1) == "numpadButton") {
        entryTextEdit->insert(button->objectName().right(1));
        qDebug() << Q_FUNC_INFO << "text now: " << entryTextEdit->text();
    }

    //Check if the button was backspace
    else if(button->objectName() == QString("backspaceButton")) {
        if(backspaceTimer != NULL) {
            //we stop timing the press event
            backspaceTimer->stop();
            delete backspaceTimer;
            backspaceTimer = NULL;
        }
        entryTextEdit->setText(entryTextEdit->text().left(entryTextEdit->text().length()-1));
    }
}

void PinCodeQueryUI::buttonPressed()
{    
    DuiButton* button = static_cast<DuiButton*>(this->sender());
    //Check if the button was backspace
    if(button->objectName() == QString("backspaceButton")) {
        //we check if the user holds the button down for 1 second or longer
        backspaceTimer = new QTimer(this);
        connect(backspaceTimer, SIGNAL(timeout()), this, SLOT(removeText()));
        backspaceTimer->start(1000);
    }
}

void PinCodeQueryUI::removeText()
{
    if(backspaceTimer != NULL) {
        backspaceTimer->stop();
        delete backspaceTimer;
        backspaceTimer = NULL;
    }
    entryTextEdit->setText("");
}

void PinCodeQueryUI::hideWindow()
{
    DuiApplicationWindow *win = DuiApplication::activeApplicationWindow();
    if(win && !win->isHidden()){
        win->hide();
    }
    qDebug() << Q_FUNC_INFO << "win->isHidden()" << (win ? win->isHidden() : true);
}

void PinCodeQueryUI::showWindow()
{
    DuiApplicationWindow *win = DuiApplication::activeApplicationWindow();
    if(win && win->isHidden()){
        win->show();
    }
    qDebug() << Q_FUNC_INFO << "win->isHidden()" << (win ? win->isHidden() : true);
}

void PinCodeQueryUI::setWindowOnTop(bool onTop)
{
#ifdef UNIT_TEST
    // window cannot be on top at unit tests or it crashes:
    // "The WM_COMMAND property of a window needs correct values for argv and this is not present."
    onTop = false;
#endif

    DuiApplicationWindow *win = DuiApplication::activeApplicationWindow();
    if(!win){
        return;
    }

    Qt::WindowFlags flags = win->windowFlags();
    if(onTop && !(flags & Qt::WindowStaysOnTopHint) ){
        flags |= Qt::WindowStaysOnTopHint;
        win->setWindowFlags(flags);
    } else if (flags & Qt::WindowStaysOnTopHint){
        flags &= ~Qt::WindowStaysOnTopHint;
        win->setWindowFlags(flags);
    }
    qDebug() << Q_FUNC_INFO << "win onTop" << onTop;
}

void PinCodeQueryUI::appear(DuiSceneWindow::DeletionPolicy policy)
{
    if(!isVisible()){
        DuiApplicationPage::appear(policy);
    }
    qDebug() << Q_FUNC_INFO << "isVisible()" << isVisible();
}

void PinCodeQueryUI::appearNow(DuiSceneWindow::DeletionPolicy policy)
{
    if(!isVisible()){
        DuiApplicationPage::appearNow(policy);
    }
    qDebug() << Q_FUNC_INFO << "isVisible()" << isVisible();
}

void PinCodeQueryUI::disappear()
{
    if(isVisible()){
        DuiApplicationPage::disappear();
    }
    qDebug() << Q_FUNC_INFO << "isVisible()" << isVisible();
}

void PinCodeQueryUI::disappearNow()
{
    if(isVisible()){
        DuiApplicationPage::disappearNow();
    }
    qDebug() << Q_FUNC_INFO << "isVisible()" << isVisible();
}

// eof
