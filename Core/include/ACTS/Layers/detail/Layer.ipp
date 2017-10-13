// This file is part of the ACTS project.
//
// Copyright (C) 2016 ACTS project team
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <limits>

namespace Acts {

template <class T>
bool
Layer::onLayer(const T& pars, const BoundaryCheck& bcheck) const
{
  return isOnLayer(pars.position(), bcheck);
}

template <class T>
std::vector<SurfaceIntersection>
Layer::getCompatibleSurfaces(const T&                       pars,
                             PropDirection                  pDir,
                             const BoundaryCheck&           bcheck,
                             bool                           collectSensitive,
                             bool                           collectMaterial,
                             bool                           collectPassive,
                             int                            searchType,
                             const Surface*                 startSurface,
                             const Surface*                 endSurface,
                             const ICompatibilityEstimator* ice) const
{

  // prepare the surface intersections for return
  std::vector<SurfaceIntersection> cSurfaces;

  // fast exit - there is nothing to do nothing to do
  if (!m_surfaceArray || !m_approachDescriptor) return cSurfaces;

  // the test boudnary check is defined by the search type
  BoundaryCheck tCheck = searchType % 2 ? BoundaryCheck(false) : bcheck;

  // check if the parameter surface is already on the Layer
  const Surface* tSurface = &(pars.referenceSurface());

  // position and momentum/dir
  const Vector3D& pos = pars.position();
  const Vector3D  dir = (pDir == oppositeMomentum)
      ? Vector3D(-1. * pars.momentum().unit())
      : pars.momentum().unit();

  // check if you have to stop at the endSurface
  double maxPathLength = std::numeric_limits<double>::infinity();
  if (endSurface) {
    // intersect the end surface
    // - it is the final one don't use the bounday check at all
    Intersection endInter
        = endSurface->intersectionEstimate(pos, dir, pDir, false);
    // non-valid intersection with the end surface provided at this layer
    // indicates wrong direction or faulty setup
    // -> do not return compatible surfaces since they may lead you on a wrong
    // navigation path
    if (endInter.valid && endInter.pathLength > 0.)
      maxPathLength = endInter.pathLength;
    else
      return cSurfaces;
  }

  // (A) approach descriptor section
  //
  // the approach surfaces are in principle always testSurfaces
  // - the surface on approach is excluded via the veto
  // - the surfaces are only collected if needed
  if (m_approachDescriptor
      && (collectPassive || (collectMaterial && m_ssApproachSurfaces > 1))) {
    // the approach surfaces
    const std::vector<const Surface*>& approachSurfaces
        = m_approachDescriptor->containedSurfaces();
    // we loop through and veto
    // - if the approach surface is the parameter surface
    // - if the surface is not compatible with the collect
    for (auto& aSurface : approachSurfaces) {
      // skip if start or end surface
      if (aSurface == startSurface || aSurface == endSurface) continue;
      // we fill passive always, rest is only for material
      if (collectPassive || aSurface->associatedMaterial())
        testCompatibleSurface(
            cSurfaces, *aSurface, pos, dir, pDir, tCheck, maxPathLength, ice);
    }
  }

  // (B) sensitive surface section
  //
  bool collectPS = collectPassive || collectSensitive;
  // we have to search for if m_surfaceArray exists && either :
  // - collectPassive is set true : records everything
  // - collectSensitive is set true : direct request
  // - collectMaterial is set true and sensitive structure >1
  if (m_surfaceArray
      && (collectPS || (collectMaterial && m_ssSensitiveSurfaces > 1))) {
    // compatible test surfaces
    std::vector<const Surface*> ctestSurfaces;
    if (searchType <= 0) {
      // take all the test surfaces & their bin mates
      auto allTestSurfaces = m_surfaceArray->arrayObjects();
      // reserve twice the amount
      ctestSurfaces.reserve(allTestSurfaces.size());
      for (auto& atSurface : allTestSurfaces) {
        // skip start and end surface overlaps
        if (atSurface == startSurface || atSurface == endSurface) continue;
        // get the bin mates if they exist
        if (atSurface && atSurface->associatedDetectorElement()) {
          // get the bin mates
          auto bmElements
              = atSurface->associatedDetectorElement()->binmembers();
          for (auto& bmElement : bmElements) {
            // access the pain surface pointer
            const Surface* sSurface = &(bmElement->surface());
            // skip start and end surface overlaps
            if (sSurface == startSurface || sSurface == endSurface) continue;
            // only fill when really needed
            if (collectPS || sSurface->associatedMaterial())
              ctestSurfaces.push_back(sSurface);
          }
        }
        // only fill when really needed for checking
        if (collectPS || atSurface->associatedMaterial())
          ctestSurfaces.push_back(atSurface);
      }  // end loop over atSurface
    } else {
      // get the nominal test object
      auto tSurface = m_surfaceArray->object(pos);
      if (tSurface && tSurface->associatedDetectorElement()) {
        // get the detector elements
        auto dElement = tSurface->associatedDetectorElement();
        std::vector<const DetectorElementBase*> dElements = {dElement};
        // get the neighbours
        dElements.insert(dElements.begin(),
                         dElement->neighbours().begin(),
                         dElement->neighbours().end());
        // loop over all detector elements and add their surfaces and binmember
        // surfaces
        for (auto& ade : dElements) {
          // insert the surface
          const Surface* sSurface = &(ade->surface());
          // skip start and end surface overlaps
          if (sSurface == startSurface || sSurface == endSurface) continue;
          // same proecure as before
          if (collectPS || sSurface->associatedMaterial())
            ctestSurfaces.push_back(&(ade->surface()));
          // insert the bin members and the neighbors
          for (auto& abm : ade->binmembers()) {
            // and again for the bin members
            const Surface* bSurface = &(abm->surface());
            // skip start and end surface overlaps
            if (bSurface == startSurface || bSurface == endSurface) continue;
            // insert only when needed
            if (collectPS || bSurface->associatedMaterial())
              ctestSurfaces.push_back(bSurface);
          }
        }
      }
    }
    // sensitive surfaces and test them
    for (auto& ctSurface : ctestSurfaces)
      testCompatibleSurface(
          cSurfaces, *ctSurface, pos, dir, pDir, tCheck, maxPathLength, ice);

  }  // end of sensitive surfaces to exist

  // (C) this is the representing surface
  //
  // the layer surface itself is a testSurface
  const Surface* layerSurface = &surfaceRepresentation();
  // veto if it is the surface of the track parameter already
  if (tSurface != layerSurface && layerSurface != startSurface
      && layerSurface != endSurface
      && (collectPassive
          || (collectMaterial && layerSurface->associatedMaterial()))) {
    testCompatibleSurface(
        cSurfaces, *layerSurface, pos, dir, pDir, tCheck, maxPathLength, ice);
  }

  // only sort it if the intersection was done
  std::sort(cSurfaces.begin(), cSurfaces.end());

  // return
  return cSurfaces;
}

inline void
Layer::testCompatibleSurface(std::vector<SurfaceIntersection>& cSurfaces,
                             const Surface&                    surface,
                             const Vector3D&                   pos,
                             const Vector3D&                   dir,
                             PropDirection                     pDir,
                             const BoundaryCheck&              bcheck,
                             double                            maxPathLength,
                             const ICompatibilityEstimator*) const
{
  // check if you need to force the momentum direction
  bool fDirection = (pDir == anyDirection ? false : true);
  // the intersection
  Intersection sfIntersection
      = surface.intersectionEstimate(pos, dir, fDirection, bcheck);
  // check if intersection is valid and maxPathLength has not been exceeded
  if (sfIntersection.valid && sfIntersection.pathLength < maxPathLength) {
    // resulting propDirection
    PropDirection rDir
        = (sfIntersection.pathLength > 0 ? alongMomentum : oppositeMomentum);
    // and the surfaces & direction to push back - take all
    cSurfaces.push_back(SurfaceIntersection(sfIntersection, &surface, rDir));
  }
}

}  // end of namespace Acts