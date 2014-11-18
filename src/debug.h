/***************************************************************************
 *   Copyright (C) 2006 by Carsten Weinhold                                *
 *   carsten.weinhold@gmx.de                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef __SSS_DEBUG_H
#define __SSS_DEBUG_H

#include <qfile.h>

/**
@author Carsten Weinhold
*/
class DbgLog {

public:
    DbgLog();
    ~DbgLog();

    static bool isEnabled()             { return instance; };
    static void setEnabled(bool enable);
    static void log(const QString &msg) { if (instance) instance->logMessage(msg); };

private:
    void logMessage(const QString &msg);

    QFile   logFile;
    QString logPrefix;

    static DbgLog *instance;
};

#endif // __SSS_DEBUG_H

