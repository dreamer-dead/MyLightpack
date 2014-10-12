/*
 * GrabberContext.hpp
 *
 *  Created on: 12/24/2013
 *     Project: Prismatik
 *
 *  Copyright (c) 2013 tim
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

#ifndef GRABBERCONTEXT_HPP
#define GRABBERCONTEXT_HPP

#include <QList>
#include <QRgb>

#include "GrabberBase.hpp"

class GrabbedArea;

struct AllocatedBuf {
    AllocatedBuf()
        : size(0)
        , ptr(NULL)
        , isAvail(true)
    {}

    size_t size;
    unsigned char * ptr;
    bool isAvail;
};

class GrabberContext {
public:
    GrabberContext()
        : grabWidgets(0)
        , grabResult(0) {
    }

    ~GrabberContext() {
//        releaseAllBufs();
//        freeReleasedBufs();
    }

    unsigned char * queryBuf(size_t reqSize);
    void releaseAllBufs();
    void freeReleasedBufs();
    int buffersCount() const { return _allocatedBufs.size(); }

public:
    const GrabberBase::GrabbedAreas *grabWidgets;
    QList<QRgb> *grabResult;

private:
    QList<AllocatedBuf> _allocatedBufs;
};


#endif // GRABBERCONTEXT_HPP
