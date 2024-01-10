/*
 * SPDX-FileCopyrightText: (C) Christian Schmitt, March 2013
 * SPDX_FileComment: points of interest management routines
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <istream>              // std::ws
#include "poidb.hxx"

#include <simgear/compiler.h>
#include <simgear/debug/logstream.hxx>
#include <simgear/math/sg_geodesy.hxx>
#include <simgear/misc/sg_path.hxx>
#include <simgear/structure/exception.hxx>
#include <simgear/io/iostreams/sgstream.hxx>

#include <Navaids/NavDataCache.hxx>


using std::string;

static FGPositioned::Type
mapPOITypeToFGPType(int aTy)
{
  switch (aTy) {
  case 10: return FGPositioned::COUNTRY;
  case 12: return FGPositioned::CITY;
  case 13: return FGPositioned::TOWN;
  case 14: return FGPositioned::VILLAGE;
  
  case 1000: return FGPositioned::VISUAL_REPORTING_POINT;
  case 1001: return FGPositioned::WAYPOINT;

  default:
    throw sg_range_exception("Unknown POI type", "FGNavDataCache::readPOIFromStream");
  }
}

namespace flightgear
{

    const int LINES_IN_POI_DAT = 769019;

static PositionedID readPOIFromStream(std::istream& aStream, NavDataCache* cache,
                                        FGPositioned::Type type = FGPositioned::INVALID)
{
    if (aStream.eof()) {
        return 0;
    }

    aStream >> std::ws;
    if (aStream.peek() == '#') {
        aStream >> skipeol;
        return 0;
    }
    
  int rawType;
  aStream >> rawType;
  double lat, lon;
  std::string name;
  aStream >> lat >> lon;
  getline(aStream, name);

  SGGeod pos(SGGeod::fromDeg(lon, lat));
  name = simgear::strutils::strip(name);

  // the type can be forced by our caller, but normally we use the value
  // supplied in the .dat file
  if (type == FGPositioned::INVALID) {
    type = mapPOITypeToFGPType(rawType);
  }
  if (type == FGPositioned::INVALID) {
    return 0;
  }

  return cache->createPOI(type, name, pos, name, false);
}

// load and initialize the POI database
bool poiDBInit(const SGPath& path)
{
    sg_gzifstream in( path );
    if ( !in.is_open() ) {
        SG_LOG( SG_NAVAID, SG_ALERT, "Cannot open file: " << path );
      return false;
    }

    unsigned int lineNumber = 0;
    NavDataCache* cache = NavDataCache::instance();
    while (!in.eof()) {
      readPOIFromStream(in, cache);

        ++lineNumber;
        if ((lineNumber % 100) == 0) {
            // every 100 lines
            unsigned int percent = (lineNumber * 100) / LINES_IN_POI_DAT;
            cache->setRebuildPhaseProgress(NavDataCache::REBUILD_POIS, percent);
        }
    } // of stream data loop

    return true;
}

} // of namespace flightgear
