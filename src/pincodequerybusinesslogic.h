#ifndef PINCODEQUERYBUSINESSLOGIC_H
#define PINCODEQUERYBUSINESSLOGIC_H

#include "pincodequeryui.h"
#include "notificationtype.h"

#include <QObject>
#include <QPointer>
#ifdef UNIT_TEST
    #include "../tests/ut_pincodequerybusinesslogic/pinstubs.h"
#else
    #include <SIM>
    #include <SIMLock>
#endif

class PinCodeQueryDBusAdaptor;
class CallHandler;

using namespace Cellular;

class PinCodeQueryBusinessLogic : public QObject
{
    Q_OBJECT
public:

    /** Sub states are mini states to keep user interface going while
     * SIM state does not change.
     * enum SIMStatus { UnknownStatus, Ok, NoSIM, PermanentlyBlocked,
     *                  NotReady, PINRequired, PUKRequired, Rejected, SIMLockRejected };
     */
    enum SubStates {
        SubNothing,
        // PIN change
        SubEnterOldPIN,
        // PIN change
        // PUK ok - change PIN
        SubEnterNewPIN,
        SubReEnterNewPIN,
        // PIN query
        SubFirstTry,
        SubFailedTry,
        // PIN query states
        SubPinQueryState,
        SubEnablePinQuery,
        SubDisablePinQuery,
    };

    explicit PinCodeQueryBusinessLogic(QObject* parent = 0);
    virtual ~PinCodeQueryBusinessLogic();

    void pinQueryState(SIMSecurity::PINType pinType);
    void enablePinQueryRequested(bool enabled);
    bool launchPinQuery(SIMSecurity::PINType pinType);
    void changePinCode();

public slots:
    void cancelQuery();
    void resendSimLockCode();

signals:
    void showNotification(const QString &text, NotificationType::Type type);
    void showConfirmation(const QString &text, const QString &buttonText);

private: // attributes
    QPointer<PinCodeQueryUI> uiPin;    
    PinCodeQueryDBusAdaptor* dbus;
    QString newPinCode;
    QString oldPinCode;
    QString simLockCode;

    SubStates subState;
    int previousSimState;
    bool queryLaunch;
    bool initialized;

    enum LaunchState {
        None,
        Requested,
        Ongoing,
        DoneWithError,
        DoneWithOk
    };
//    LaunchState queryState;

    SIM* sim;
    SIMSecurity* simSec;
    SIMLock *simLock;

    CallHandler *callUi;

private: // methods
    bool handleSIMError(SIMError error);
    bool handleSIMLockError(SIMLockError error);

    void init();
    void createUi(bool enableBack = false);
    void closeUi();
    void setUiHeader(QString headerText);
    bool stateOperation(int status, int relationState);
    void stateOperationIdle(int status);
    void startLaunch();
    void getCode(bool enableBack, QString header);
    void setPinHeader(int attemptsLeft);

    void doEmergencyCall();
    void informTechnicalProblem();
    void simLockRetry();

    //  states
    void ui2SIMLocked(bool showNote = true);
    void ui2PINQuery();
    void ui2PINFailed(int attemptsLeft);

    void ui2PUKQuery();
    void ui2enterNewPin();
    void ui2reenterPIN();    
    void ui2disappear(bool oerationOk);

private slots:
    void uiCodeChanged();
    void uiButtonReleased();
    void emitLaunchResponse(bool ok);

    void simStatusChanged(SIM::SIMStatus status);
    void simStatusComplete(SIM::SIMStatus status, SIMError error);
    void simPINCodeVerified(bool success, SIMError error);
    void simPUKCodeVerified(bool success, SIMError error);
    void simPINAttemptsLeft(int attempts, SIMError error);
    void simPUKAttemptsLeft(int attempts, SIMError error);
    void simPINCodeChanged(bool success, SIMError error);
    void simLockUnlockCodeVerified(SIMLockError error);
    void simEnablePINQueryComplete(SIMError error);
    void simPinQueryStateComplete(SIMSecurity::PINQuery state, SIMError error);

    void callStarted();
    void callDone();

#ifdef UNIT_TEST
   friend class Ut_PinCodeQueryBusinessLogic;
#endif // UNIT_TEST

};

#endif // PINCODEQUERYBUSINESSLOGIC_H
