/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019 Daniel Baston <dbaston@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/polygonize/HoleAssigner.java 0b3c7e3eb0d3e
 *
 **********************************************************************/

#include <geos/operation/polygonize/HoleAssigner.h>
#include <geos/util/Interrupt.h>

namespace geos {
namespace operation {
namespace polygonize {

void
HoleAssigner::buildIndex() {
    for (EdgeRing* shell : m_shells) {
        m_shellIndex.insert(shell->getRingInternal()->getEnvelopeInternal(), shell);
    }
}

void
HoleAssigner::assignHolesToShells(std::vector<EdgeRing*> & holes, std::vector<EdgeRing*> & shells)
{
    HoleAssigner assigner(shells);
    assigner.assignHolesToShells(holes);

}

void HoleAssigner::assignHolesToShells(std::vector<EdgeRing*> & holes) {
    for (const auto& holeER : holes) {
        assignHoleToShell(holeER);
        GEOS_CHECK_FOR_INTERRUPTS();
    }
}

void
HoleAssigner::assignHoleToShell(EdgeRing* holeER)
{
    EdgeRing* shell = findEdgeRingContaining(holeER);

    if(shell != nullptr) {
        shell->addHole(holeER);
    }
}

std::vector<EdgeRing*>
HoleAssigner::findShells(const geom::Envelope& e) {
    std::vector<EdgeRing*> shells;
    m_shellIndex.query(e, shells);

    return shells;
}

EdgeRing*
HoleAssigner::findEdgeRingContaining(EdgeRing* testEr) {
    const geos::geom::Envelope* e = testEr->getRingInternal()->getEnvelopeInternal();

    std::vector<EdgeRing*> candidateShells = findShells(*e);

    return testEr->findEdgeRingContaining(candidateShells);
}

}
}
}
