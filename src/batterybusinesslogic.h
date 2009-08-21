#ifndef BATTERYBUSINESSLOGIC_H
#define BATTERYBUSINESSLOGIC_H

#include "notifier.h"
#include <qmsystem/qmbattery.h>
#include <qmsystem/qmdevicemode.h>

#include <QObject>


using namespace Maemo;

class BatteryBusinessLogic : public QObject
{
    Q_OBJECT
public:
    BatteryBusinessLogic();
    virtual ~BatteryBusinessLogic();        

signals:
    void charging();

private: //attributes
    Notifier *uiNotif;
    QmBattery *battery;
    QmDeviceMode *deviceMode;
    bool powerSaveMode;    

private: //methos
    void checkRemainingTime(); //TEMP
    void checkChargingState();
    void checkBatteryLevel();

public slots:
    void checkBattery();

private slots:
    void batteryLevelChanged(Maemo::QmBattery::Level level);
    void batteryStateChanged(Maemo::QmBattery::State state);
    void remainingTalkTimeChanged(int secondsLeft);
    void activatePSM();

};


#endif // BATTERYBUSINESSLOGIC_H

