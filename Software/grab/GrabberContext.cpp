/*
 * GrabberContext.cpp
 *
 *  Copyright (c) 2013 dreamer.dead@gmail.com
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

#include "GrabberContext.hpp"
#include "common/DebugOut.hpp"

unsigned char * GrabberContext::queryBuf(size_t reqSize) {
    const AllocatedBuf *bestFitBuf = NULL;
    for (int i = 0; i < _allocatedBufs.size(); ++i) {
        if (_allocatedBufs[i].isAvail
            && _allocatedBufs[i].size >= reqSize
            && ( bestFitBuf == NULL
                || _allocatedBufs[i].size < bestFitBuf->size))
        {
            bestFitBuf = &_allocatedBufs[i];
        }
    }
    if (bestFitBuf == NULL) {
        AllocatedBuf newBuffer;
        newBuffer.ptr = (unsigned char *)malloc(reqSize);

        if (newBuffer.ptr == NULL) {
            qCritical() << "Failed to allocate a new GrabberContext buffer"
                << ", queried size = " << reqSize;
            return NULL;
        }

        newBuffer.size = reqSize;
        _allocatedBufs.append(newBuffer);
        bestFitBuf = &_allocatedBufs.last();
    }
    return bestFitBuf->ptr;
}

void GrabberContext::releaseAllBufs() {
    for (QList<AllocatedBuf>::iterator iter = _allocatedBufs.begin();
         iter != _allocatedBufs.end(); ++iter) {
        iter->isAvail = true;
    }
}

namespace {
struct SelectAvail {
    SelectAvail() {}
    bool operator ()(const AllocatedBuf& buf) const { return buf.isAvail; }
};

struct FreeBuf {
    FreeBuf() {}
    void operator ()(const AllocatedBuf& buf) const { free(buf.ptr); }
};
}

void GrabberContext::freeReleasedBufs() {
    QList<AllocatedBuf>::iterator avails = std::remove_if(
        _allocatedBufs.begin(), _allocatedBufs.end(), SelectAvail());

    if (avails != _allocatedBufs.end()) {
        std::for_each(avails, _allocatedBufs.end(), FreeBuf());
        _allocatedBufs.erase(avails, _allocatedBufs.end());
    }
}
