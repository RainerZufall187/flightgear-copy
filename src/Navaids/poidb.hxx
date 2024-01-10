/*
 * SPDX-FileCopyrightText: (C) Christian Schmitt, March 2013
 * SPDX_FileComment: points of interest management routines
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once


#include <simgear/compiler.h>


// forward decls
class SGPath;

namespace flightgear
{

// load and initialize the POI database
bool poiDBInit(const SGPath& path);

} // of namespace flightgear

