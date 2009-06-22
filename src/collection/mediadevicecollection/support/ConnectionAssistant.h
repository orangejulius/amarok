/*
   Copyright (C) 2009 Alejandro Wainzinger <aikawarazuni@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#ifndef CONNECTIONASSISTANT_H
#define CONNECTIONASSISTANT_H

#include "mediadevicecollection_export.h"

#include <QObject>

class MediaDeviceInfo;

class QString;

/**
@class ConnectionAssistant

The ConnectionAssistant (CA) serves as a way for MediaDeviceCollectionFactory to register its
device type with the MediaDeviceMonitor (MDM). Once registered, the MDM can use the CA to
attempt to identify a newly plugged-in device, and retrieve the MediaDeviceInfo object necessary
for the Factory to connect to it.

*/

class MEDIADEVICECOLLECTION_EXPORT ConnectionAssistant : public QObject
{
    Q_OBJECT
    
public:
    virtual ~ConnectionAssistant();

    /**

    identify checks if a device identified by @param uid matches the type
    of device described by this ConnectionAssistant
    
    */
    virtual bool identify( const QString& udi );

    /**

    deviceInfo returns a pointer to a new MediaDeviceInfo of the type of device
    described by this ConnectionAssistant

    */

    virtual MediaDeviceInfo* deviceInfo( const QString& udi );
    
};

#endif // CONNECTIONASSISTANT_H
