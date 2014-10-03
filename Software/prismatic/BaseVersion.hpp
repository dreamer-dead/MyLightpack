/*
 * BaseVersion.hpp
 *
 *  Created on: 06/25/2014
 *     Project: Prismatik
 *
 *  Copyright (c) 2014 dreamer-dead
 *
 *  Lightpack is an open-source, USB content-driving ambient lighting
 *  hardware.
 *
 *  Prismatik is a free, open-source software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Prismatik and Lightpack files is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BASEVERSION_HPP
#define BASEVERSION_HPP

#include <QString>
#include <QStringList>

class BaseVersion
{
public:
    BaseVersion()
        : _major(0)
        , _minor(0)
    {}

    BaseVersion(int major, int minor)
        : _major(major), _minor(minor)
    {
    }

    BaseVersion(const QString &version)
    {
        initWith(version);
    }

    void initWith(const QString &version)
    {
        const QStringList splittedVer = version.split(".");
        if (splittedVer.size() > 2 || splittedVer.size() < 1)
        {
            return;
        }

        _major = splittedVer[0].toInt();
        _minor = splittedVer.size() > 1 ? splittedVer[1].toInt() : 0;
    }

    bool isValid() const {
        return _major != 0 || _minor != 0;
    }

    int compare(const BaseVersion &other) const {
        const int dmj = this->_major - other._major;
        if (dmj)
            return dmj;
        return this->_minor - other._minor;
    }

    bool operator== (const BaseVersion &other) const {
        return this->compare(other) == 0;
    }

    bool operator!= (const BaseVersion &other) const {
        return this->compare(other) != 0;
    }

    bool operator< (const BaseVersion &other) const {
        return this->compare(other) < 0;
    }

    bool operator> (const BaseVersion &other) const {
        return this->compare(other) > 0;
    }

    bool operator>= (const BaseVersion &other) const {
        return *this > other || *this == other ;
    }

    bool operator<= (const BaseVersion &other) const {
        return *this < other || *this == other ;
    }

    QString toString() const {
        QString resut;
        return resut.sprintf("%u.%u", _major, _minor);
    }

protected:
    uint _major;
    uint _minor;
};

#endif // BASEVERSION_HPP
