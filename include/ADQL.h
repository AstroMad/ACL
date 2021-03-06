﻿//*********************************************************************************************************************************
//
// PROJECT:							Astronomy Class Library
// FILE:								SIMBADParser
// SUBSYSTEM:						A system to parse SIMBAD query returns.
// LANGUAGE:						C++
// TARGET OS:						None.
// LIBRARY DEPENDANCE:	None.
// NAMESPACE:						ACL
// AUTHOR:							Gavin Blakeman (GGB)
// LICENSE:             GPLv2
//
//                      Copyright 2016-2020 Gavin Blakeman.
//                      This file is part of the Astronomy Class Library (ACL)
//
//                      ACL is free software: you can redistribute it and/or modify it under the terms of the GNU General
//                      Public License as published by the Free Software Foundation, either version 2 of the License, or
//                      (at your option) any later version.
//
//                      ACL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
//                      implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//                      for more details.
//
//                      You should have received a copy of the GNU General Public License along with ACL.  If not,
//                      see <http://www.gnu.org/licenses/>.
//
//
// OVERVIEW:						A colllection of routines for parsing the returned data from SIMBAD database queries.
//                      This is integrated into the remainder of the ACL library.
//
//
// CLASSES INCLUDED:		None
//
// CLASS HIERARCHY:     None.
//
//
// HISTORY:             2016-04-24 GGB - File created.
//
//*********************************************************************************************************************************

#ifndef ACL_ADQL_H
#define ACL_ADQL_H

  // Standard C++ library header files.

#include <cstdint>
#include <initializer_list>
#include <string>
#include <tuple>
#include <vector>

  // Miscellaneous library header files.

#include <GCL>
#include <SCL>

  // ACL library header files

#include "AstronomicalCoordinates.h"
#include "config.h"
#include "targetAstronomy.h"

namespace ACL
{

  class CADQL : public GCL::sqlWriter
  {
  public:
    using index_t = std::uint32_t;

  private:
    std::string fieldDelimiter = ";";

  protected:

  public:
    CADQL() {}
  };

}   // namespace ACL

#endif // ACL_ADQL_H
