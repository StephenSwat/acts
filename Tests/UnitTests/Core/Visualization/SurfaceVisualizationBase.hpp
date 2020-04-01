// This file is part of the Acts project.
//
// Copyright (C) 2020 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Acts/Visualization/GeometryVisualization.hpp"
#include "Acts/Visualization/IVisualization.hpp"

#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Surfaces/AnnulusBounds.hpp"
#include "Acts/Surfaces/ConeBounds.hpp"
#include "Acts/Surfaces/ConeSurface.hpp"
#include "Acts/Surfaces/ConvexPolygonBounds.hpp"
#include "Acts/Surfaces/CylinderBounds.hpp"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/DiamondBounds.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/DiscTrapezoidBounds.hpp"
#include "Acts/Surfaces/EllipseBounds.hpp"
#include "Acts/Surfaces/LineBounds.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RadialBounds.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"

#include <fstream>
#include <string>

namespace Acts {
namespace SurfaceVisualization {

/// Helper method to visualiza all types of surfaces
///
/// @param helper The visualziation helper
/// @param triangulate The directive whether to create triangular meshes
/// @param tag The test tag (mode) identification
/// @param suffix The file suffix for writing
/// @param msuffix the (optional) material file suffix
static inline void test(IVisualization& helper, bool triangulate,
                        const std::string& tag, const std::string& suffix,
                        const std::string& msuffix = "") {
  auto gctx = GeometryContext();
  auto identity = std::make_shared<Transform3D>(Transform3D::Identity());
  std::ofstream stream;
  std::ofstream* mstream = nullptr;
  if (not msuffix.empty()) {
    mstream = new std::ofstream;
  }

  double halfPhiSector = M_PI / 4.;
  double centralPhi = M_PI / 2.;

  // Helper method to prepare the streams & helpers
  auto prepare = [&](const std::string& fileBase) -> void {
    // First finish unfinished business
    helper.write(stream, mstream);
    helper.clear();
    stream.close();
    // Start the new one
    stream.open(fileBase + tag + suffix);
    if (mstream != nullptr) {
      mstream->close();
      mstream->open(fileBase + tag + msuffix);
      stream << "mtllib " << fileBase + tag + msuffix << "\n";
    }
  };

  //----------------------------------------------------
  // Cone Surface section
  IVisualization::ColorType coneColor = {252, 160, 0};
  std::vector<std::shared_ptr<ConeSurface>> coneSurfaces;

  double coneAlpha = 0.245;
  double coneMinZ = 0.;
  double coneCutZ = 2.;
  double coneMaxZ = 10.;
  // Full Cone
  prepare("ConeSurface");
  auto coneBounds =
      std::make_shared<ConeBounds>(coneAlpha, -coneCutZ, coneMaxZ);
  auto cone = Surface::makeShared<ConeSurface>(identity, coneBounds);
  coneSurfaces.push_back(cone);
  Visualization::drawSurface(helper, *cone, gctx, Transform3D::Identity(), 72,
                             triangulate, coneColor);
  // Sectoral Cone
  prepare("ConeSurfaceSector");
  coneBounds = std::make_shared<ConeBounds>(coneAlpha, coneMinZ, coneMaxZ,
                                            halfPhiSector);
  cone = Surface::makeShared<ConeSurface>(identity, coneBounds);
  coneSurfaces.push_back(cone);
  Visualization::drawSurface(helper, *cone, gctx, Transform3D::Identity(), 72,
                             triangulate, coneColor);
  // Sectoral Cone Shifted
  prepare("ConeSurfaceSectorShifted");
  coneBounds = std::make_shared<ConeBounds>(coneAlpha, coneCutZ, coneMaxZ,
                                            halfPhiSector, centralPhi);
  cone = Surface::makeShared<ConeSurface>(identity, coneBounds);
  coneSurfaces.push_back(cone);
  Visualization::drawSurface(helper, *cone, gctx, Transform3D::Identity(), 72,
                             triangulate, coneColor);

  // All in one for radial bounds
  std::vector<Transform3D> threeCones = {
      Transform3D(Translation3D{-0.5 * coneMaxZ, 0., 0.}),
      Transform3D(Translation3D{0., 0., 0.}),
      Transform3D(Translation3D{0.75 * coneMaxZ, 0., 0.})};

  prepare("All_ConeSurfaces");
  for (size_t ic = 0; ic < coneSurfaces.size(); ++ic) {
    Visualization::drawSurface(helper, *coneSurfaces[ic], gctx, threeCones[ic],
                               72, triangulate, coneColor);
  }
  //----------------------------------------------------
  // Cylinder surface section
  IVisualization::ColorType cylinderColor = {0, 196, 252};
  std::vector<std::shared_ptr<CylinderSurface>> cylinderSurfaces;

  double cylinderRadius = 5.;
  double cylinderHalfZ = 10.;

  // Full Cylinder
  prepare("CylinderSurface");
  auto cylinderBounds =
      std::make_shared<CylinderBounds>(cylinderRadius, cylinderHalfZ);
  auto cylinder =
      Surface::makeShared<CylinderSurface>(identity, cylinderBounds);
  cylinderSurfaces.push_back(cylinder);
  Visualization::drawSurface(helper, *cylinder, gctx, Transform3D::Identity(),
                             72, triangulate, cylinderColor);
  helper.write(stream);
  // Sectoral Cone
  prepare("CylinderSurfaceSector");
  cylinderBounds = std::make_shared<CylinderBounds>(
      cylinderRadius, cylinderHalfZ, halfPhiSector);
  cylinder = Surface::makeShared<CylinderSurface>(identity, cylinderBounds);
  cylinderSurfaces.push_back(cylinder);
  Visualization::drawSurface(helper, *cylinder, gctx, Transform3D::Identity(),
                             72, triangulate, cylinderColor);

  // Sectoral Cone Shifted
  prepare("CylinderSurfaceSectorShifted");
  cylinderBounds = std::make_shared<CylinderBounds>(
      cylinderRadius, cylinderHalfZ, halfPhiSector, centralPhi);
  cylinder = Surface::makeShared<CylinderSurface>(identity, cylinderBounds);
  cylinderSurfaces.push_back(cylinder);
  Visualization::drawSurface(helper, *cylinder, gctx, Transform3D::Identity(),
                             72, triangulate, cylinderColor);

  // All in one for radial bounds
  std::vector<Transform3D> threeCylinders = {
      Transform3D(Translation3D{-2 * cylinderRadius, 0., 0.}),
      Transform3D(Translation3D{0., 0., 0.}),
      Transform3D(Translation3D{2.5 * cylinderRadius, 0., 0.})};

  prepare("All_CylinderSurfaces");
  for (size_t ic = 0; ic < cylinderSurfaces.size(); ++ic) {
    Visualization::drawSurface(helper, *cylinderSurfaces[ic], gctx,
                               threeCylinders[ic], 72, triangulate,
                               cylinderColor);
  }

  /// ------------- planar bounding box
  /// @param name of the file
  auto boundingBox2D = [&](const RectangleBounds& rBounds,
                           const std::string& name) -> void {
    // First finish unfinished business
    helper.write(stream, mstream);
    helper.clear();
    stream.close();

    IVisualization::ColorType bbColor = {126, 126, 126};

    std::ofstream bbStream;
    std::string bbtag = "_bbox2D";
    bbStream.open(name + tag + bbtag + suffix);

    if (mstream != nullptr) {
      mstream->close();
      mstream->open(name + tag + bbtag + msuffix);
      bbStream << "mtllib " << name + tag + bbtag + msuffix << "\n";
    }

    auto bbVertices = rBounds.vertices();
    std::vector<Vector3D> bbVertices3D;
    for (const auto& bbv : bbVertices) {
      // slight shift to the back for visibility
      bbVertices3D.push_back({bbv.x(), bbv.y(), -0.01});
    }
    helper.face(bbVertices3D, bbColor);
    helper.write(bbStream, mstream);
    if (mstream != nullptr) {
      mstream->close();
    }
    bbStream.close();
    helper.clear();
  };

  //----------------------------------------------------
  // Disc Surface section
  IVisualization::ColorType discColor = {126, 252, 0};

  double discRmin = 5.;
  double discRmax = 10.;

  std::vector<std::shared_ptr<DiscSurface>> radialSurfaces;

  // Full Disc
  prepare("DiscSurfaceFull");
  auto radialBounds = std::make_shared<RadialBounds>(0., discRmax);
  auto disc = Surface::makeShared<DiscSurface>(identity, radialBounds);
  radialSurfaces.push_back(disc);
  Visualization::drawSurface(helper, *disc, gctx, Transform3D::Identity(), 72,
                             triangulate, discColor);

  // Full Sectoral Disc
  prepare("DiscSurfaceFullSector");
  radialBounds = std::make_shared<RadialBounds>(0., discRmax, halfPhiSector);
  disc = Surface::makeShared<DiscSurface>(identity, radialBounds);
  radialSurfaces.push_back(disc);
  Visualization::drawSurface(helper, *disc, gctx, Transform3D::Identity(), 72,
                             triangulate, discColor);

  // Full Sectoral Shifted Disc
  prepare("DiscSurfaceFullSectorShifted");
  radialBounds =
      std::make_shared<RadialBounds>(0., discRmax, halfPhiSector, centralPhi);
  disc = Surface::makeShared<DiscSurface>(identity, radialBounds);
  radialSurfaces.push_back(disc);
  Visualization::drawSurface(helper, *disc, gctx, Transform3D::Identity(), 72,
                             triangulate, discColor);

  // Full Ring
  prepare("DiscSurfaceRing");
  radialBounds = std::make_shared<RadialBounds>(discRmin, discRmax);
  disc = Surface::makeShared<DiscSurface>(identity, radialBounds);
  radialSurfaces.push_back(disc);
  Visualization::drawSurface(helper, *disc, gctx, Transform3D::Identity(), 72,
                             triangulate, discColor);

  // Full Sectoral Rin g
  prepare("DiscSurfaceRingSector");
  radialBounds =
      std::make_shared<RadialBounds>(discRmin, discRmax, halfPhiSector);
  disc = Surface::makeShared<DiscSurface>(identity, radialBounds);
  radialSurfaces.push_back(disc);
  Visualization::drawSurface(helper, *disc, gctx, Transform3D::Identity(), 72,
                             triangulate, discColor);

  // Full Sectoral Shifted Ring
  prepare("DiscSurfaceRingSectorShifted");
  radialBounds = std::make_shared<RadialBounds>(discRmin, discRmax,
                                                halfPhiSector, centralPhi);
  disc = Surface::makeShared<DiscSurface>(identity, radialBounds);
  radialSurfaces.push_back(disc);
  Visualization::drawSurface(helper, *disc, gctx, Transform3D::Identity(), 72,
                             triangulate, discColor);

  // All in one for radial bounds
  std::vector<Transform3D> sixDiscs = {
      Transform3D(Translation3D{-2.0 * discRmax, 1.5 * discRmax, 0.}),
      Transform3D(Translation3D{0., 1.5 * discRmax, 0.}),
      Transform3D(Translation3D{2.5 * discRmax, 1.5 * discRmax, 0.}),
      Transform3D(Translation3D{-2.0 * discRmax, -1.5 * discRmax, 0.}),
      Transform3D(Translation3D{0., -1.5 * discRmax, 0.}),
      Transform3D(Translation3D{2.5 * discRmax, -1.5 * discRmax, 0.})};
  prepare("All_DiscSurfaces_RadialBounds");
  for (size_t ir = 0; ir < radialSurfaces.size(); ++ir) {
    Visualization::drawSurface(helper, *radialSurfaces[ir], gctx, sixDiscs[ir],
                               72, triangulate, discColor);
  }

  std::vector<std::shared_ptr<DiscSurface>> anomalDiscSurfaces;
  IVisualization::ColorType discAnomalColor = {94, 186, 2};

  double annulusMinPhi = 0.75;
  double annulusMaxPhi = 1.35;
  Vector2D offset(-4., 2.);
  prepare("DiscAnulusBounds");
  auto annulus = std::make_shared<AnnulusBounds>(
      discRmin, discRmax, annulusMinPhi, annulusMaxPhi, offset);
  disc = Surface::makeShared<DiscSurface>(identity, annulus);
  anomalDiscSurfaces.push_back(disc);
  Visualization::drawSurface(helper, *disc, gctx, Transform3D::Identity(), 72,
                             triangulate, discAnomalColor);

  double discTrapezoidHxRmin = 3.;
  double discTrapezoidHxRmax = 6.;
  prepare("DiscTrapezoidBounds");
  auto discTrapezoid = std::make_shared<DiscTrapezoidBounds>(
      discTrapezoidHxRmin, discTrapezoidHxRmax, discRmin, discRmax);
  disc = Surface::makeShared<DiscSurface>(identity, discTrapezoid);
  anomalDiscSurfaces.push_back(disc);
  Visualization::drawSurface(helper, *disc, gctx, Transform3D::Identity(), 72,
                             triangulate, discAnomalColor);

  // All in one for radial bounds
  std::vector<Transform3D> twoAnomalDiscs = {
      Transform3D(Translation3D{-5., 0., 0.}),
      Transform3D(Translation3D{5., 0., 0.})};
  prepare("All_DiscSurfaces_AnomalBounds");
  for (size_t id = 0; id < anomalDiscSurfaces.size(); ++id) {
    Visualization::drawSurface(helper, *anomalDiscSurfaces[id], gctx,
                               sixDiscs[id], 72, triangulate, discAnomalColor);
  }

  //----------------------------------------------------
  // Plane Surface section
  IVisualization::ColorType planeColor = {0, 0, 256};
  std::vector<std::shared_ptr<PlaneSurface>> planarSurfaces;

  // Ellipse shaped : Full Ellipse
  double ellipseR0min = 2;
  double ellipseR0max = 4;
  double ellipseR1min = 3;
  double ellipseR1max = 6;
  std::string name = "PlaneSurfaceEllipse";
  prepare(name);
  auto ellipse = std::make_shared<EllipseBounds>(ellipseR0min, ellipseR0max,
                                                 ellipseR1min, ellipseR1max);
  auto plane = Surface::makeShared<PlaneSurface>(identity, ellipse);
  planarSurfaces.push_back(plane);
  Visualization::drawSurface(helper, *plane, gctx, Transform3D::Identity(), 72,
                             triangulate, planeColor);
  boundingBox2D(ellipse->boundingBox(), name);

  // ConvexPolygon shaped example: Triangle
  name = "PlaneSurfaceTriangleRegular";
  prepare(name);
  std::vector<Vector2D> tvertices = {{-3, -1.5}, {3, -1.5}, {0, 4.5}};
  auto triangle = std::make_shared<ConvexPolygonBounds<3>>(tvertices);
  plane = Surface::makeShared<PlaneSurface>(identity, triangle);
  planarSurfaces.push_back(plane);
  Visualization::drawSurface(helper, *plane, gctx, Transform3D::Identity(), 72,
                             triangulate, planeColor);
  boundingBox2D(triangle->boundingBox(), name);

  // ConvexPolygon shaped example: Triangle
  name = "PlaneSurfaceTriangleGeneral";
  prepare(name);
  tvertices = {{-1., 4.5}, {4, 6.5}, {3, 8.5}};
  triangle = std::make_shared<ConvexPolygonBounds<3>>(tvertices);
  plane = Surface::makeShared<PlaneSurface>(identity, triangle);
  planarSurfaces.push_back(plane);
  Visualization::drawSurface(helper, *plane, gctx, Transform3D::Identity(), 72,
                             triangulate, planeColor);
  boundingBox2D(triangle->boundingBox(), name);

  // ConvexPolygon shaped example: Triangle
  name = "PlaneSurfaceConvexPolygonGeneral";
  prepare(name);
  tvertices = {{-1., 4.5}, {4, 6.5}, {6, 8.5}, {0, 10.5}, {-3, 6.2}};
  auto dynamicpolygon =
      std::make_shared<ConvexPolygonBounds<PolygonDynamic>>(tvertices);
  plane = Surface::makeShared<PlaneSurface>(identity, dynamicpolygon);
  planarSurfaces.push_back(plane);
  Visualization::drawSurface(helper, *plane, gctx, Transform3D::Identity(), 72,
                             triangulate, planeColor);
  boundingBox2D(dynamicpolygon->boundingBox(), name);

  // Diamond shaped
  name = "PlaneSurfaceDiamond";
  prepare(name);
  auto diamond = std::make_shared<DiamondBounds>(3., 6., 2., 2., 4.);
  plane = Surface::makeShared<PlaneSurface>(identity, diamond);
  planarSurfaces.push_back(plane);
  Visualization::drawSurface(helper, *plane, gctx, Transform3D::Identity(), 72,
                             triangulate, planeColor);
  boundingBox2D(diamond->boundingBox(), name);

  // Rectangle plane
  name = "PlaneSurfaceRectangle";
  prepare(name);
  auto rectangle = std::make_shared<RectangleBounds>(3., 7.);
  plane = Surface::makeShared<PlaneSurface>(identity, rectangle);
  planarSurfaces.push_back(plane);
  Visualization::drawSurface(helper, *plane, gctx, Transform3D::Identity(), 72,
                             triangulate, planeColor);
  boundingBox2D(rectangle->boundingBox(), name);

  // Off-centered Rectangle plane:
  name = "PlaneSurfaceRectangleOffcentered";
  prepare(name);
  rectangle =
      std::make_shared<RectangleBounds>(Vector2D{1., 2.}, Vector2D{15., 12.});
  plane = Surface::makeShared<PlaneSurface>(identity, rectangle);
  Visualization::drawSurface(helper, *plane, gctx, Transform3D::Identity(), 72,
                             triangulate, planeColor);
  boundingBox2D(rectangle->boundingBox(), name);

  // Last closing of the streams
  helper.write(stream, mstream);
  stream.close();
  if (mstream != nullptr) {
    mstream->close();
  }
}

}  // namespace SurfaceVisualization
}  // namespace Acts
