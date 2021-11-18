/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "DeviceAudio.h"
#include "EnableManager.h"

#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class DeviceAudio_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceAudio = new DeviceAudio;
    }
    void TearDown()
    {
        delete m_deviceAudio;
    }
    DeviceAudio *m_deviceAudio;
};

TEST_F(DeviceAudio_UT, DeviceAudio_UT_setInfoFromHwinfo)
{
    QMap<QString, QString> map;
    map.insert("SysFS BusID", "x86");
    m_deviceAudio->setInfoFromHwinfo(map);
}

TEST_F(DeviceAudio_UT, DeviceAudio_UT_setInfoFromLshw)
{
    QMap<QString, QString> map;
    map.insert("SysFS BusID", "x86");
    map.insert("bus info", "x86@");
    map.insert("bus info", "/n@");
    m_deviceAudio->setInfoFromLshw(map);
}

TEST_F(DeviceAudio_UT, DeviceAudio_UT_setInfoFromCatDevices)
{
    QMap<QString, QString> map;
    map.insert("Device", "x86");
    m_deviceAudio->setInfoFromCatDevices(map);
    m_deviceAudio->setInfoFromCatAudio(map);
    m_deviceAudio->setAudioChipFromDmesg("/@");
}

TEST_F(DeviceAudio_UT, DeviceAudio_UT_name)
{
    m_deviceAudio->name();
    m_deviceAudio->driver();
}

EnableDeviceStatus ut_audio_enableDeviceByDriver()
{
    return EnableDeviceStatus::EDS_Faild;
}

bool ut_audio_enable()
{
    return false;
}

TEST_F(DeviceAudio_UT, DeviceAudio_UT_setEnable)
{
    //    m_deviceAudio->enable();
    Stub stub;
    stub.set(ADDR(EnableManager, enableDeviceByDriver), ut_audio_enableDeviceByDriver);
    //    stub.set(ADDR(DeviceAudio,enable),ut_audio_enable);
    m_deviceAudio->setEnable(false);
    m_deviceAudio->subTitle();
    m_deviceAudio->getOverviewInfo();
}

TEST_F(DeviceAudio_UT, DeviceAudio_UT_initFilterKey)
{
    m_deviceAudio->initFilterKey();
    m_deviceAudio->loadBaseDeviceInfo();
    m_deviceAudio->loadOtherDeviceInfo();
    m_deviceAudio->loadTableHeader();
    m_deviceAudio->loadTableData();
}