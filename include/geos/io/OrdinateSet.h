/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 ISciences LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/util/GEOSException.h>

namespace geos {
namespace io {

/**
 * \class OrdinateSet
 * \brief Utility class to manipulate a set of flags indicating whether
 *        X, Y, Z, or M dimensions are present.
 *        Based on JTS EnumSet<Ordinate>.
 */
class GEOS_DLL OrdinateSet {
private:
    enum Ordinate : unsigned char {
        X = 1,
        Y = 2,
        Z = 4,
        M = 8,
    };

    enum Ordinates : unsigned char {
        XY = Ordinate::X | Ordinate::Y,
        XYZ = Ordinate::X | Ordinate::Y | Ordinate::Z,
        XYM = Ordinate::X | Ordinate::Y | Ordinate::M,
        XYZM = Ordinate::X | Ordinate::Y | Ordinate::Z | Ordinate::M
    };

    explicit OrdinateSet(Ordinates o) : m_value(o), m_changesAllowed(true) {}

    Ordinates m_value;
    bool m_changesAllowed;

public:

    static OrdinateSet createXY() {
        return OrdinateSet(Ordinates::XY);
    }

    static OrdinateSet createXYZ() {
        return OrdinateSet(Ordinates::XYZ);
    }

    static OrdinateSet createXYM() {
        return OrdinateSet(Ordinates::XYM);
    }

    static OrdinateSet createXYZM() {
        return OrdinateSet(Ordinates::XYZM);
    }

    void setZ(bool value) {
        if (hasZ() != value) {
            if (m_changesAllowed) {
                m_value = static_cast<Ordinates>(static_cast<unsigned char>(m_value) ^ Ordinate::Z);
            } else {
                throw util::GEOSException("Cannot add additional ordinates.");
            }
        }
    }

    void setM(bool value) {
        if (hasM() != value) {
            if (m_changesAllowed){
                m_value = static_cast<Ordinates>(static_cast<unsigned char>(m_value) ^ Ordinate::M);
            } else {
                throw util::GEOSException("Cannot add additional ordinates.");
            }
        }
    }

    bool hasZ() const {
        return static_cast<unsigned char>(m_value) & static_cast<unsigned char>(Ordinate::Z);
    }

    bool hasM() const {
        return static_cast<unsigned char>(m_value) & static_cast<unsigned char>(Ordinate::M);
    }

     int size() const {
        return 2 + hasZ() + hasM();
    }

    bool changesAllowed() const {
        return m_changesAllowed;
    }

    void setChangesAllowed(bool allowed) {
        m_changesAllowed = allowed;
    }

    bool operator==(const OrdinateSet& other) const {
        return this->m_value == other.m_value;
    }

    bool operator!=(const OrdinateSet& other) const {
        return !(*this == other);
    }

};

}
}
