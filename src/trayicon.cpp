/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2011 Razor team
 * Authors:
 *   Christian Surlykke <christian@surlykke.dk>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include <QDebug>
#include <QApplication>
#include <QProcess>
#include <QMessageBox>
#include <QToolTip>
#include <QHelpEvent>
#include <Solid/Battery>
#include <Solid/Device>
#include <XdgIcon>

#include "trayicon.h"
#include "batteryhelper.h"
#include "../config/powermanagementsettings.h"

TrayIcon::TrayIcon(Solid::Battery *battery, QObject *parent)
    : QSystemTrayIcon(parent),
    mBattery(battery),
    mIconProducer(battery),
    mContextMenu()
{
    connect(mBattery, &Solid::Battery::chargePercentChanged, this, &TrayIcon::updateTooltip);
    connect(mBattery, &Solid::Battery::chargeStateChanged, this, &TrayIcon::updateTooltip);
    updateTooltip();

    connect(&mIconProducer, SIGNAL(iconChanged()), this, SLOT(iconChanged()));
    iconChanged();

    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            SLOT(onActivated(QSystemTrayIcon::ActivationReason)));

    mContextMenu.addAction(XdgIcon::fromTheme(QStringLiteral("configure")), tr("Configure"),
                           this, SLOT(onConfigureTriggered()));
    mContextMenu.addAction(XdgIcon::fromTheme(QStringLiteral("help-about")), tr("About"),
                           this, SLOT(onAboutTriggered()));
    mContextMenu.addAction(XdgIcon::fromTheme(QStringLiteral("edit-delete")), tr("Disable icon"),
                           this, SLOT(onDisableIconTriggered()));
    setContextMenu(&mContextMenu);
}

TrayIcon::~TrayIcon()
{
}

void TrayIcon::iconChanged()
{
    setIcon(mIconProducer.mIcon);
}

void TrayIcon::updateTooltip()
{
    QString tooltip = BatteryHelper::stateToString(mBattery->chargeState());
    tooltip += QString(" (%1 %)").arg(mBattery->chargePercent());
    setToolTip(tooltip);
}

void TrayIcon::onConfigureTriggered()
{
    QProcess::startDetached("lxqt-config-powermanagement");
}

void TrayIcon::onAboutTriggered()
{
    QMessageBox::about(0,
                       tr("About"),
                       tr( "<p>"
                           "  <b>LXQt Power Management</b><br/>"
                           "  - Power Management for the LXQt Desktop Environment"
                           "</p>"
                           "<p>"
                           "  Authors:<br/>"
                           "  &nbsp; Christian Surlykke, Alec Moskvin<br/>"
                           "  &nbsp; - and others from the Razor and LXQt projects"
                           "</p>"
                           "<p>"
                           "  Copyright &copy; 2012-2014"
                           "</p>"
                        ));
}


void TrayIcon::onDisableIconTriggered()
{
    PowerManagementSettings().setShowIcon(false);
}

void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    qDebug() << "onActivated" << reason;
    if (Trigger == reason) emit toggleShowInfo();
}
