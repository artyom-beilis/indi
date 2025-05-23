/*
    Rotator Interface
    Copyright (C) 2017 Jasem Mutlaq (mutlaqja@ikarustech.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "indirotatorinterface.h"
#include "defaultdevice.h"

#include "indilogger.h"

#include <cstring>

namespace INDI
{

RotatorInterface::RotatorInterface(DefaultDevice *defaultDevice) : m_defaultDevice(defaultDevice)
{
}

void RotatorInterface::initProperties(const char *groupName)
{
    // Rotator Angle
    // @INDI_STANDARD_PROPERTY@
    IUFillNumber(&GotoRotatorN[0], "ANGLE", "Angle", "%.2f", 0, 360., 10., 0.);
    IUFillNumberVector(&GotoRotatorNP, GotoRotatorN, 1, m_defaultDevice->getDeviceName(), "ABS_ROTATOR_ANGLE", "Goto",
                       groupName, IP_RW, 0, IPS_IDLE );

    // Abort Rotator
    // @INDI_STANDARD_PROPERTY@
    IUFillSwitch(&AbortRotatorS[0], "ABORT", "Abort", ISS_OFF);
    IUFillSwitchVector(&AbortRotatorSP, AbortRotatorS, 1, m_defaultDevice->getDeviceName(), "ROTATOR_ABORT_MOTION",
                       "Abort Motion", groupName, IP_RW, ISR_ATMOST1, 0, IPS_IDLE);

    // Rotator Sync
    // @INDI_STANDARD_PROPERTY@
    IUFillNumber(&SyncRotatorN[0], "ANGLE", "Angle", "%.2f", 0, 360., 10., 0.);
    IUFillNumberVector(&SyncRotatorNP, SyncRotatorN, 1, m_defaultDevice->getDeviceName(), "SYNC_ROTATOR_ANGLE", "Sync",
                       groupName, IP_RW, 0, IPS_IDLE );

    // Home Rotator
    // @INDI_STANDARD_PROPERTY@
    IUFillSwitch(&HomeRotatorS[0], "HOME", "Start", ISS_OFF);
    IUFillSwitchVector(&HomeRotatorSP, HomeRotatorS, 1, m_defaultDevice->getDeviceName(), "ROTATOR_HOME", "Homing", groupName,
                       IP_RW, ISR_ATMOST1, 0, IPS_IDLE);

    // Reverse Direction
    // @INDI_STANDARD_PROPERTY@
    IUFillSwitch(&ReverseRotatorS[DefaultDevice::INDI_ENABLED], "INDI_ENABLED", "Enabled", ISS_OFF);
    IUFillSwitch(&ReverseRotatorS[DefaultDevice::INDI_DISABLED], "INDI_DISABLED", "Disabled", ISS_ON);
    IUFillSwitchVector(&ReverseRotatorSP, ReverseRotatorS, 2, m_defaultDevice->getDeviceName(), "ROTATOR_REVERSE", "Reverse",
                       groupName, IP_RW, ISR_1OFMANY,
                       0, IPS_IDLE);

    // Backlash Compensation
    // @INDI_STANDARD_PROPERTY@
    IUFillSwitch(&RotatorBacklashS[DefaultDevice::INDI_ENABLED], "INDI_ENABLED", "Enabled", ISS_OFF);
    IUFillSwitch(&RotatorBacklashS[DefaultDevice::INDI_DISABLED], "INDI_DISABLED", "Disabled", ISS_ON);
    IUFillSwitchVector(&RotatorBacklashSP, RotatorBacklashS, 2, m_defaultDevice->getDeviceName(), "ROTATOR_BACKLASH_TOGGLE",
                       "Backlash", groupName, IP_RW,
                       ISR_1OFMANY, 60, IPS_IDLE);


    // Backlash Compensation Value
    // @INDI_STANDARD_PROPERTY@
    IUFillNumber(&RotatorBacklashN[0], "ROTATOR_BACKLASH_VALUE", "Steps", "%.f", 0, 1e6, 100, 0);
    IUFillNumberVector(&RotatorBacklashNP, RotatorBacklashN, 1, m_defaultDevice->getDeviceName(), "ROTATOR_BACKLASH_STEPS",
                       "Backlash",
                       groupName, IP_RW, 60, IPS_OK);


    // Rotator Limits
    // @INDI_STANDARD_PROPERTY@
    RotatorLimitsNP[0].fill("ROTATOR_LIMITS_VALUE", "Max Range (degrees)", "%.f", 0, 360, 30, 0);
    RotatorLimitsNP.fill(m_defaultDevice->getDeviceName(), "ROTATOR_LIMITS", "Limits", groupName, IP_RW, 60, IPS_IDLE);
}

bool RotatorInterface::processNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    INDI_UNUSED(names);
    INDI_UNUSED(n);

    if (dev != nullptr && strcmp(dev, m_defaultDevice->getDeviceName()) == 0)
    {
        ////////////////////////////////////////////
        // Move Absolute Angle
        ////////////////////////////////////////////
        if (strcmp(name, GotoRotatorNP.name) == 0)
        {
            if (values[0] == GotoRotatorN[0].value)
            {
                GotoRotatorNP.s = IPS_OK;
                IDSetNumber(&GotoRotatorNP, nullptr);
                return true;
            }

            // If value is outside safe zone, then prevent motion
            if (RotatorLimitsNP[0].getValue() > 0 && ((values[0] < 180
                    && (std::abs(values[0] - m_RotatorOffset)) > RotatorLimitsNP[0].getValue()) ||
                    (values[0] > 180 && (std::abs(values[0] - m_RotatorOffset)) < (360 - RotatorLimitsNP[0].getValue()))))
            {
                GotoRotatorNP.s = IPS_ALERT;
                DEBUGFDEVICE(m_defaultDevice->getDeviceName(), Logger::DBG_ERROR,
                             "Rotator target %.2f exceeds safe limits of %.2f degrees...", values[0], RotatorLimitsNP[0].getValue());
                IDSetNumber(&GotoRotatorNP, nullptr);
            }
            else
            {
                GotoRotatorNP.s = MoveRotator(values[0]);
                IDSetNumber(&GotoRotatorNP, nullptr);
                if (GotoRotatorNP.s == IPS_BUSY)
                    DEBUGFDEVICE(m_defaultDevice->getDeviceName(), Logger::DBG_SESSION, "Rotator moving to %.2f degrees...", values[0]);
            }
            return true;
        }
        ////////////////////////////////////////////
        // Sync
        ////////////////////////////////////////////
        else if (strcmp(name, SyncRotatorNP.name) == 0)
        {
            if (values[0] == GotoRotatorN[0].value)
            {
                SyncRotatorNP.s = IPS_OK;
                IDSetNumber(&SyncRotatorNP, nullptr);
                return true;
            }

            bool rc = SyncRotator(values[0]);
            SyncRotatorNP.s = rc ? IPS_OK : IPS_ALERT;
            if (rc)
            {
                SyncRotatorN[0].value = values[0];
                // Always reset offset after a sync
                m_RotatorOffset = values[0];
            }

            IDSetNumber(&SyncRotatorNP, nullptr);
            return true;
        }
        ////////////////////////////////////////////
        // Backlash value
        ////////////////////////////////////////////
        else if (!strcmp(name, RotatorBacklashNP.name))
        {
            if (RotatorBacklashS[DefaultDevice::INDI_ENABLED].s != ISS_ON)
            {
                RotatorBacklashNP.s = IPS_IDLE;
                DEBUGDEVICE(dev, Logger::DBG_WARNING, "Rotatorer backlash must be enabled first.");
            }
            else
            {
                uint32_t steps = static_cast<uint32_t>(values[0]);
                if (SetRotatorBacklash(steps))
                {
                    RotatorBacklashN[0].value = values[0];
                    RotatorBacklashNP.s = IPS_OK;
                }
                else
                    RotatorBacklashNP.s = IPS_ALERT;
            }
            IDSetNumber(&RotatorBacklashNP, nullptr);
            return true;
        }
        ////////////////////////////////////////////
        // Limits
        ////////////////////////////////////////////
        else if (RotatorLimitsNP.isNameMatch(name))
        {
            RotatorLimitsNP.update(values, names, n);
            RotatorLimitsNP.setState(IPS_OK);
            RotatorLimitsNP.apply();
            if (RotatorLimitsNP[0].getValue() == 0)
                DEBUGDEVICE(dev, Logger::DBG_SESSION, "Rotator limits are disabled.");
            m_RotatorOffset = GotoRotatorN[0].value;
            return true;
        }
    }

    return false;
}

bool RotatorInterface::processSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    INDI_UNUSED(states);
    INDI_UNUSED(names);
    INDI_UNUSED(n);

    if (dev != nullptr && strcmp(dev, m_defaultDevice->getDeviceName()) == 0)
    {
        ////////////////////////////////////////////
        // Abort
        ////////////////////////////////////////////
        if (strcmp(name, AbortRotatorSP.name) == 0)
        {
            AbortRotatorSP.s = AbortRotator() ? IPS_OK : IPS_ALERT;
            IDSetSwitch(&AbortRotatorSP, nullptr);
            if (AbortRotatorSP.s == IPS_OK)
            {
                if (GotoRotatorNP.s != IPS_OK)
                {
                    GotoRotatorNP.s = IPS_OK;
                    IDSetNumber(&GotoRotatorNP, nullptr);
                }
            }
            return true;
        }
        ////////////////////////////////////////////
        // Home
        ////////////////////////////////////////////
        else if (strcmp(name, HomeRotatorSP.name) == 0)
        {
            HomeRotatorSP.s = HomeRotator();
            IUResetSwitch(&HomeRotatorSP);
            if (HomeRotatorSP.s == IPS_BUSY)
                HomeRotatorS[0].s = ISS_ON;
            IDSetSwitch(&HomeRotatorSP, nullptr);
            return true;
        }
        ////////////////////////////////////////////
        // Reverse Rotator
        ////////////////////////////////////////////
        else if (strcmp(name, ReverseRotatorSP.name) == 0)
        {
            int prevIndex = IUFindOnSwitchIndex(&ReverseRotatorSP);
            IUUpdateSwitch(&ReverseRotatorSP, states, names, n);
            const bool enabled = IUFindOnSwitchIndex(&ReverseRotatorSP) == DefaultDevice::INDI_ENABLED;

            if (ReverseRotator(enabled))
            {
                IUUpdateSwitch(&ReverseRotatorSP, states, names, n);
                ReverseRotatorSP.s = IPS_OK;
                DEBUGFDEVICE(m_defaultDevice->getDeviceName(), Logger::DBG_SESSION, "Rotator direction is %s.",
                             (enabled ? "reversed" : "normal"));
            }
            else
            {
                IUResetSwitch(&ReverseRotatorSP);
                ReverseRotatorS[prevIndex].s = ISS_ON;
                ReverseRotatorSP.s = IPS_ALERT;
                DEBUGDEVICE(m_defaultDevice->getDeviceName(), Logger::DBG_SESSION, "Rotator reverse direction failed.");
            }

            IDSetSwitch(&ReverseRotatorSP, nullptr);
            return true;
        }
        ////////////////////////////////////////////
        // Backlash enable/disable
        ////////////////////////////////////////////
        else if (strcmp(name, RotatorBacklashSP.name) == 0)
        {
            int prevIndex = IUFindOnSwitchIndex(&RotatorBacklashSP);
            IUUpdateSwitch(&RotatorBacklashSP, states, names, n);
            const bool enabled = IUFindOnSwitchIndex(&RotatorBacklashSP) == DefaultDevice::INDI_ENABLED;

            if (SetRotatorBacklashEnabled(enabled))
            {
                RotatorBacklashSP.s = IPS_OK;
                DEBUGFDEVICE(m_defaultDevice->getDeviceName(), Logger::DBG_SESSION, "Rotator backlash is %s.",
                             (enabled ? "enabled" : "disabled"));
            }
            else
            {
                IUResetSwitch(&RotatorBacklashSP);
                RotatorBacklashS[prevIndex].s = ISS_ON;
                RotatorBacklashSP.s = IPS_ALERT;
                DEBUGDEVICE(m_defaultDevice->getDeviceName(), Logger::DBG_ERROR, "Failed to set trigger rotator backlash.");
            }

            IDSetSwitch(&RotatorBacklashSP, nullptr);
            return true;
        }
    }

    return false;
}

bool RotatorInterface::updateProperties()
{
    if (m_defaultDevice->isConnected())
    {
        m_defaultDevice->defineProperty(&GotoRotatorNP);

        if (CanAbort())
            m_defaultDevice->defineProperty(&AbortRotatorSP);
        if (CanSync())
            m_defaultDevice->defineProperty(&SyncRotatorNP);
        if (CanHome())
            m_defaultDevice->defineProperty(&HomeRotatorSP);
        if (CanReverse())
            m_defaultDevice->defineProperty(&ReverseRotatorSP);
        if (HasBacklash())
        {
            m_defaultDevice->defineProperty(&RotatorBacklashSP);
            m_defaultDevice->defineProperty(&RotatorBacklashNP);
        }
        m_defaultDevice->defineProperty(RotatorLimitsNP);
    }
    else
    {
        m_defaultDevice->deleteProperty(GotoRotatorNP.name);

        if (CanAbort())
            m_defaultDevice->deleteProperty(AbortRotatorSP.name);
        if (CanSync())
            m_defaultDevice->deleteProperty(SyncRotatorNP.name);
        if (CanHome())
            m_defaultDevice->deleteProperty(HomeRotatorSP.name);
        if (CanReverse())
            m_defaultDevice->deleteProperty(ReverseRotatorSP.name);
        if (HasBacklash())
        {
            m_defaultDevice->deleteProperty(RotatorBacklashSP.name);
            m_defaultDevice->deleteProperty(RotatorBacklashNP.name);
        }
        m_defaultDevice->deleteProperty(RotatorLimitsNP);
    }

    return true;
}

bool RotatorInterface::SyncRotator(double angle)
{
    INDI_UNUSED(angle);
    DEBUGDEVICE(m_defaultDevice->getDeviceName(), Logger::DBG_ERROR, "Rotator does not support syncing.");
    return false;
}

IPState RotatorInterface::HomeRotator()
{
    DEBUGDEVICE(m_defaultDevice->getDeviceName(), Logger::DBG_ERROR, "Rotator does not support homing.");
    return IPS_ALERT;
}

bool RotatorInterface::AbortRotator()
{
    DEBUGDEVICE(m_defaultDevice->getDeviceName(), Logger::DBG_ERROR, "Rotator does not support abort.");
    return false;
}

bool RotatorInterface::ReverseRotator(bool enabled)
{
    INDI_UNUSED(enabled);
    DEBUGDEVICE(m_defaultDevice->getDeviceName(), Logger::DBG_ERROR, "Rotator does not support reverse.");
    return false;
}

bool RotatorInterface::SetRotatorBacklash(int32_t steps)
{
    INDI_UNUSED(steps);
    DEBUGDEVICE(m_defaultDevice->getDeviceName(), Logger::DBG_ERROR, "Rotator does not support backlash compensation.");
    return false;
}

bool RotatorInterface::SetRotatorBacklashEnabled(bool enabled)
{
    // If disabled, set the Rotatorer backlash to zero.
    if (enabled)
        return SetRotatorBacklash(static_cast<int32_t>(RotatorBacklashN[0].value));
    else
        return SetRotatorBacklash(0);
}

bool RotatorInterface::saveConfigItems(FILE *fp)
{
    if (CanReverse())
    {
        IUSaveConfigSwitch(fp, &ReverseRotatorSP);
    }
    if (HasBacklash())
    {
        IUSaveConfigSwitch(fp, &RotatorBacklashSP);
        IUSaveConfigNumber(fp, &RotatorBacklashNP);
    }
    RotatorLimitsNP.save(fp);

    return true;
}

}
