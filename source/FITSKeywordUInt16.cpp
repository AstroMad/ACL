﻿//**********************************************************************************************************************************
//
// PROJECT:							Astronomy Class Library
// FILE:								CFITSKeywordUShort
// SUBSYSTEM:						Templated functions for keywords
// LANGUAGE:						C++
// TARGET OS:						None.
// LIBRARY DEPENDANCE:	cfitsio, boost
// NAMESPACE:						ACL
// AUTHOR:							Gavin Blakeman. (GGB)
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
// OVERVIEW:            Classes to support the keywords in a FITS file. The class heirarchy enables the keywords to be stored in the
//                      STL container classes using a virtual base class.
//
// CLASSES INCLUDED:		CFITSKeywordSByte  - Handle 8 bit signed integers.
//
// CLASS HIERARCHY:     CFITSKeyword
//                        CFITSKeywordByte
//                        CFITSKeywordDouble
//                        CFITSKeywordFloat
//                        CFITSKeywordLong
//                        CFITSKeywordLongLong
//                        CFITSKeywordSByte
//                        CFITSKeywordShort
//                        CFITSKeywordString
//                        CFITSKeywordULong
//                        CFITSKeywordUShort
//
// HISTORY:             2016-04-10 - Classes rewritten and moved into seperate files.
//
//**********************************************************************************************************************************

#include "include/FITSKeywordUInt16.h"

  // Standard C++ library header files

#include <limits>
#include <stdexcept>
#include <string>

  // Miscellaneous library header files

#include "boost/locale.hpp"

namespace ACL
{
  /// @brief Copy constructor.
  /// @param[in] toCopy - The instance to copy.
  /// @throws None.
  /// @version 2016-04-09/GGB - Function created.

  CFITSKeywordUInt16::CFITSKeywordUInt16(CFITSKeywordUInt16 const &toCopy) : CFITSKeyword(toCopy.keyword_, toCopy.comment_)
  {
    (*this) = toCopy;
  }

  /// @brief Copy operator
  /// @param[in] rhs - Instance to copy.
  /// @version 2016-04-09/GGB - Function created.

  CFITSKeywordUInt16 &CFITSKeywordUInt16::operator=(CFITSKeywordUInt16  const &rhs)
  {
    keyword_ = rhs.keyword();
    value_ = rhs.value_;
    comment_ = rhs.comment();
    return (*this);
  }

  CFITSKeywordUInt16::operator std::int16_t() const
  {
    if ( (value_ <= std::numeric_limits<std::int16_t>::max()) &&
         (value_ >= std::numeric_limits<std::int16_t>::min()))
    {
      return (static_cast<std::int16_t>(value_));
    }
    else
    {
      throw std::range_error(boost::locale::translate("Unable to cast value ... out of range."));
    };
  }

  CFITSKeywordUInt16::operator std::int32_t() const
  {
    return (static_cast<std::int32_t>(value_));
  }

  CFITSKeywordUInt16::operator std::int64_t() const
  {
    return (static_cast<std::int64_t>(value_));
  }

  CFITSKeywordUInt16::operator std::uint16_t() const
  {
    return value_;
  }


  CFITSKeywordUInt16::operator std::uint32_t() const
  {
    return (static_cast<std::uint32_t>(value_));
  }

  CFITSKeywordUInt16::operator float() const
  {
    return (static_cast<float>(value_));
  }

  CFITSKeywordUInt16::operator double() const
  {
    return (static_cast<double>(value_));
  }

  CFITSKeywordUInt16::operator std::string() const
  {
    return std::to_string(value_);
  }

  /// @brief Creates a copy of this object.
  /// @returns Pointer to the copy.
  /// @throws std::bad_alloc
  /// @version 2018-09-22/GGB - Updated to use std::unique_ptr.
  /// @version 2011-07-18/GGB - Function created.

  std::unique_ptr<CFITSKeyword> CFITSKeywordUInt16::createCopy() const
  {
    return std::make_unique<CFITSKeywordUInt16>(*this);
  }

  /// @brief Returns the type of the data stored in this instance.
  /// @returns The type of keyword stored.
  /// @throws None.
  /// @version 2015-08-23/GGB - Function created.

  KWType CFITSKeywordUInt16::type() const
  {
    return KWT_UINT16;
  }

  /// @brief Write the keyword to the FITS file.
  /// @param[in] file - The FITS file pointer.
  /// @throws CFITSException
  /// @version 2016-04-09/GGB - Function created.

  void CFITSKeywordUInt16::writeToFITS(fitsfile *file) const
  {
    CFITSIO_TEST(fits_update_key, file, TUSHORT, keyword_.c_str(), const_cast<void *>(static_cast<void const *>(&value_)),
                                 comment_.c_str());
  }
}
