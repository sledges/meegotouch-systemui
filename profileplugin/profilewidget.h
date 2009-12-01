#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <DuiWidget>

class DuiApplicationPage;
class DuiButton;
class DuiButtonGroup;
class DuiStatusIndicatorMenuInterface;
class ProfileDataInterface;

/*!
 * The Profile widget makes it possible to select the currently
 * active profile.
 */
class ProfileWidget : public DuiWidget
{
    Q_OBJECT

public:
    ProfileWidget(DuiStatusIndicatorMenuInterface &statusIndicatorMenu, QGraphicsItem *parent = NULL);
    virtual ~ProfileWidget();

private slots:
    /*!
     * \brief A slot for showing the profile duicontrolpanel plugin
     */
    void showProfileModificationPage();

    /*!
     * \brief A slot for receiving information about profile button clicks.
     */
    void buttonClicked(int index);

private:
    //! Interface for controlling the status indicator menu
    DuiStatusIndicatorMenuInterface &statusIndicatorMenu;

    //! Profile data If to receive information about possible profiles
    ProfileDataInterface* dataIf;

    QStringList profileNames;
    DuiButtonGroup *buttonGroup;

};

#endif // PROFILEWIDGET_H