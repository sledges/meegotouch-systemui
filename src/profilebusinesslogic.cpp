#include "profilebusinesslogic.h"

#include <Profile>
#include <QDebug>
#include <QStringList>
#include <QList>

using namespace ProfileName;

namespace ProfileId{
    // must be identical with enum defined in ../profileapplet/profilebuttons.h
    enum ProfileId {
        ringing = 0,
        silent,
        beep,
        loud,
        none // none must be last to teel how many profiles there are available
    };
}

ProfileBusinessLogic::ProfileBusinessLogic()
{
    api = new Profile(this);
    // TODO: what is the possibility the profiles can change without
    // user action when UI is open?
}

ProfileBusinessLogic::~ProfileBusinessLogic()
{
}

void ProfileBusinessLogic::getCurrentProfile()
{
    qDebug() << "ProfileBusinessLogic::getCurrentProfile()";
    QString prof = api->activeProfile();
    emit currentProfile(name2Id(prof));
}

void ProfileBusinessLogic::getVolumeLevels()
{
    qDebug() << "ProfileBusinessLogic::getVolumeLevels()";

    QStringList names = api->profileNames();
    for(int i = 0; i < names.count(); ++i)
    {
        QString name = names.at(i);
        int value = api->volumeLevel(name);
        emit volumeLevel(name2Id(name), value);
    }
}

void ProfileBusinessLogic::getVibrationValues()
{
    qDebug() << "ProfileBusinessLogic::getVibrationValues()";

    // TODO: get correct functionality
    QStringList names = api->profileNames();
    for(int i = 0; i < names.count(); ++i)
    {
        QString name = names.at(i);
        bool value = api->isVibrationEnabled(name);
        emit vibrationValue(name2Id(name), value);
    }
}

void ProfileBusinessLogic::setProfile(int value)
{
    qDebug() << "ProfileBusinessLogic::setProfile(" << value << ")";
    bool success = api->setActiveProfile(id2Name(value));
    if(!success){
        // TODO: what??
    }
}

void ProfileBusinessLogic::setVibration(int id, bool value)
{
    qDebug() << "ProfileBusinessLogic::setVibration(" << id << "," << value <<  ")";
    bool success = api->setVibration(id2Name(id), value);
    if(!success){
        // TODO: what??
    }
}

void ProfileBusinessLogic::setVolumeLevel(int id, int value)
{
    qDebug() << "ProfileBusinessLogic::setVolumeLevel(" << id << "," << value <<  ")";
    bool success = api->setVolumeLevel(id2Name(id), value);
    if(!success){
        // TODO: what??
    }
}

int ProfileBusinessLogic::name2Id(QString profileName)
{
    int id = ProfileId::none;
    if(ProfileName::ringing == profileName){
        id = ProfileId::ringing;
    }else if(ProfileName::silent == profileName){
        id = ProfileId::silent;
    }else if(ProfileName::beep == profileName){
        id = ProfileId::beep;
    }else if(ProfileName::loud == profileName){
        id = ProfileId::loud;
    }
    return id;
}

QString ProfileBusinessLogic::id2Name(int profileId)
{
    QString name = "";
    switch(profileId){
        case ProfileId::ringing:
            name = ProfileName::ringing;
            break;
        case ProfileId::silent:
            name = ProfileName::silent;
            break;
        case ProfileId::beep:
            name = ProfileName::beep;
            break;
        case ProfileId::loud:
            name = ProfileName::loud;
            break;
    }
    return name;
}


