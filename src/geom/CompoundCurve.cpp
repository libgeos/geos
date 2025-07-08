/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <sstream>

#include <geos/geom/CompoundCurve.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/operation/BoundaryOp.h>
#include <geos/util.h>

namespace geos {
namespace geom {

CompoundCurve::CompoundCurve(std::vector<std::unique_ptr<SimpleCurve>>&& p_curves,
                             const GeometryFactory& gf)
    : Curve(gf),
      curves(std::move(p_curves)),
      envelope(computeEnvelopeInternal()) {
    validateConstruction();
}

CompoundCurve::CompoundCurve(const CompoundCurve& other)
    : Curve(other),
      curves(other.curves.size()),
      envelope(other.envelope)
{
    for (std::size_t i = 0; i < curves.size(); i++) {
        curves[i].reset(static_cast<SimpleCurve*>(other.curves[i]->clone().release()));
    }
}

CompoundCurve&
CompoundCurve::operator=(const CompoundCurve& other)
{
    curves.resize(other.curves.size());
    envelope = other.envelope;

    for (std::size_t i =0; i < curves.size(); i++) {
        curves[i].reset(static_cast<SimpleCurve*>(other.curves[i]->clone().release()));
    }

    return *this;
}

void
CompoundCurve::apply_ro(CoordinateFilter* cf) const
{
    for (const auto& curve : curves) {
        curve->apply_ro(cf);
    }
}

void
CompoundCurve::apply_ro(CoordinateSequenceFilter& csf) const
{
    for (const auto& curve : curves) {
        const auto& seq = *curve->getCoordinatesRO();
        for (std::size_t i = 0; i < seq.size(); i++) {
            if (csf.isDone()) {
                return;
            }
            csf.filter_ro(seq, i);
        }
    }
}

void
CompoundCurve::apply_rw(const CoordinateFilter* cf)
{
    for (auto& curve : curves) {
        curve->apply_rw(cf);
    }
}

void
CompoundCurve::apply_rw(CoordinateSequenceFilter&)
{
    throw util::UnsupportedOperationException();
}

std::unique_ptr<CompoundCurve>
CompoundCurve::clone() const
{
    return std::unique_ptr<CompoundCurve>(cloneImpl());
}

CompoundCurve*
CompoundCurve::cloneImpl() const
{
    return new CompoundCurve(*this);
}

int
CompoundCurve::compareToSameClass(const Geometry* g) const
{
    const CompoundCurve* curve = detail::down_cast<const CompoundCurve*>(g);
    return compare(curves, curve->curves);
}

Envelope
CompoundCurve::computeEnvelopeInternal() const
{
    Envelope e;
    for (const auto& curve : curves) {
        e.expandToInclude(curve->getEnvelopeInternal());
    }
    return e;
}

bool
CompoundCurve::equalsExact(const Geometry* other, double tolerance) const
{
    if (!isEquivalentClass(other)) {
        return false;
    }

    const CompoundCurve* otherCurve = static_cast<const CompoundCurve*>(other);
    if (curves.size() != otherCurve->curves.size()) {
        return false;
    }

    for (std::size_t i = 0; i < otherCurve->curves.size(); i++) {
        if (!curves[i]->equalsExact(otherCurve->curves[i].get(), tolerance)) {
            return false;
        }
    }

    return true;
}

bool
CompoundCurve::equalsIdentical(const Geometry* other) const
{
    if (!isEquivalentClass(other)) {
        return false;
    }

    const CompoundCurve* otherCurve = static_cast<const CompoundCurve*>(other);
    if (curves.size() != otherCurve->curves.size()) {
        return false;
    }

    for (std::size_t i = 0; i < otherCurve->curves.size(); i++) {
        if (!curves[i]->equalsIdentical(otherCurve->curves[i].get())) {
            return false;
        }
    }

    return true;
}

std::unique_ptr<Geometry>
CompoundCurve::getBoundary() const
{
    operation::BoundaryOp bop(*this);
    return bop.getBoundary();
}

const CoordinateXY*
CompoundCurve::getCoordinate() const
{
    for (const auto& curve : curves) {
        if (!curve->isEmpty()) {
            return curve->getCoordinate();
        }
    }

    return nullptr;
}

uint8_t
CompoundCurve::getCoordinateDimension() const
{
    return static_cast<std::uint8_t>(2 + hasZ() + hasM());
}

std::unique_ptr<CoordinateSequence>
CompoundCurve::getCoordinates() const
{
    auto ret = std::make_unique<CoordinateSequence>(0, hasZ(), hasM());
    for (const auto& curve : curves) {
        ret->add(*curve->getCoordinatesRO());
    }
    return ret;
}

const SimpleCurve*
CompoundCurve::getCurveN(std::size_t i) const
{
    return curves[i].get();
}

std::string
CompoundCurve::getGeometryType() const
{
    return "CompoundCurve";
}

GeometryTypeId
CompoundCurve::getGeometryTypeId() const
{
    return GEOS_COMPOUNDCURVE;
}

double
CompoundCurve::getLength() const
{
    double sum = 0;
    for (const auto& curve : curves) {
        sum += curve->getLength();
    }
    return sum;
}

std::size_t
CompoundCurve::getNumCurves() const
{
    return curves.size();
}

std::size_t
CompoundCurve::getNumPoints() const
{
    std::size_t n =0;
    for (const auto& curve : curves) {
        n += curve->getNumPoints();
    }
    return n;
}

bool
CompoundCurve::hasZ() const
{
    return std::any_of(curves.begin(), curves.end(), [](const auto& curve) {
        return curve->hasZ();
    });
}

bool
CompoundCurve::hasM() const
{
    return std::any_of(curves.begin(), curves.end(), [](const auto& curve) {
        return curve->hasM();
    });
}

bool
CompoundCurve::hasCurvedComponents() const
{
    for (const auto& curve : curves) {
        if (curve->hasCurvedComponents()) {
            return true;
        }
    }
    return false;
}

bool
CompoundCurve::isClosed() const
{
    if (isEmpty()) {
        return false;
    }

    const SimpleCurve& first = *curves.front();
    const SimpleCurve& last = *curves.back();

    return first.getCoordinateN(0) == last.getCoordinateN(last.getNumPoints() - 1);
}

bool
CompoundCurve::isEmpty() const
{
    return !std::any_of(curves.begin(), curves.end(), [](const auto& curve) {
        return !curve->isEmpty();
    });
}

void
CompoundCurve::normalize()
{
    throw util::UnsupportedOperationException();
}

std::unique_ptr<CompoundCurve>
CompoundCurve::reverse() const
{
    return std::unique_ptr<CompoundCurve>(reverseImpl());
}

CompoundCurve*
CompoundCurve::reverseImpl() const
{
    std::vector<std::unique_ptr<SimpleCurve>> reversed(curves.size());
    std::transform(curves.rbegin(), curves.rend(), reversed.begin(), [](const auto& curve) {
        return std::unique_ptr<SimpleCurve>(static_cast<SimpleCurve*>(curve->reverse().release()));
    });

    return getFactory()->createCompoundCurve(std::move(reversed)).release();
}

void
CompoundCurve::validateConstruction() const
{
    for (std::size_t i = 0; i < curves.size(); i++) {
        if (curves[i]->isEmpty()) {
            std::ostringstream ss;
            ss << "Section " << i << " of CompoundCurve is empty";
            throw util::IllegalArgumentException(ss.str());
        }
    }
    for (std::size_t i = 1; i < curves.size(); i++) {
        const CoordinateXY& end = curves[i-1]->getCoordinatesRO()->back<CoordinateXY>();
        const CoordinateXY& start = curves[i]->getCoordinatesRO()->front<CoordinateXY>();

        if (start != end) {
            std::ostringstream ss;
            ss << "Sections of CompoundCurve are not contiguous: " <<
                   "curve " << (i-1) << " ends at " << end <<
                   " ; curve " << i << " begins at " << start;

            throw util::IllegalArgumentException(ss.str());
        }
    }
}

}
}
