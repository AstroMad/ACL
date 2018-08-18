﻿//*********************************************************************************************************************************
//
// PROJECT:							Astronomy Class Library
// FILE:								HDBImage
// SUBSYSTEM:						Astronomical HDb classes
// LANGUAGE:						C++
// TARGET OS:						None.
// LIBRARY DEPENDANCE:	SCL, CFitsIO, boost.
// NAMESPACE:						ACL
// AUTHOR:							Gavin Blakeman (GGB)
// LICENSE:             GPLv2
//
//                      Copyright 2011-2017 Gavin Blakeman.
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
// OVERVIEW:						A number of classes supporting file IO of astronomical data including images.
//											The classes are designed to present a single interface to the other astro library
//											classes while supporting several different file types.
//
//											File types supported for input are:   FITS
//																								            SBIG image files
//                                                            DNG Files
//                                                            RAW Files
//                      File types supported for output are": FITS
//
//                      The CAstroFile class is modelled around the FITS file format and is designed to work seamlessly with FITS
//                      files. While opening other types of files is supported, some information will be created on the fly.
//                      To ensure that all information generated by image manipulation is saved correctly, the information must
//                      be saved in the FITS format. Information will be lost if it is saved in any other format.
//
//                      The CAstroFile class also provides some observation management functionality as there are keywords that
//                      are automatically linked to observation information.
//
//
// CLASSES INCLUDED:		CImageHDB       - Class to encapsulate image HDB data
//
// CLASS HIERARCHY:     CHDB
//                        - CImageHDB
//                        - CHDBAsciiTable
//                          - CAstrometryHDB
//                          - CHDBPhotometry
//                        - CHDBBinTable
//
// HISTORY:             2015-09-22 GGB - astroManager 2015.09 release
//                      2013-09-30 GGB - astroManager 2013.09 release.
//                      2013-03-22 GGB - astroManager 2013.03 release.
//                      2013-02-10 GGB - File created.
//
//*********************************************************************************************************************************

#ifndef ACL_HDBIMAGE_H
#define ACL_HDBIMAGE_H

#include "HDB.h"

  // Standard libraries

#include <memory>

struct WorldCoor;

namespace ACL
{
  /// @brief The CImageHDB class is used for storing HDU data for an image.
  /// This provides specialist support for images, such as centroid finding, star finding etc.
  /// @todo Refactor to change name to CHDBImage

  class CImageHDB : public CHDB
  {
  private:
    CAstroImage *data;                      ///< The actual image data in the block.
    MCL::TPoint2D<FP_t> pixelSize;          ///< Pixel size in microns. If specified.
    struct WorldCoor *WCSInformation;       ///< WCS information structure

  protected:
    virtual void keywordPixelSize(PFITSKeyword);

      // Special keyword function

    virtual bool specialKeyword(PFITSKeyword);
    virtual void WCSProcess();

  public:
    static PHDB createHDB(CAstroFile *);

    CImageHDB(CAstroFile *, std::string const &);
    explicit CImageHDB(CImageHDB const &);
    virtual ~CImageHDB();

    virtual PHDB createCopy() const;

      // FITS functions

    virtual void readFromFITS(fitsfile *);
    virtual void writeToFITS(fitsfile *);

    virtual int BITPIX() const;
    virtual void BITPIX(int);

    using CHDB::NAXIS;      // Functions are hidden.
    virtual NAXIS_t NAXIS() const;

    using CHDB::NAXISn;   // Functions are hidden.
    virtual AXIS_t NAXISn(NAXIS_t) const;

    virtual FP_t BSCALE() const;
    virtual void BSCALE(FP_t);

    virtual FP_t BZERO() const;
    virtual void BZERO(FP_t);

    virtual int PEDESTAL() const;

      // RAW functions

#ifdef USE_LIBRAW
    virtual bool loadFromRAW(LibRaw *);
#endif // USE_LIBRAW
    void loadFromRGBHP(SRGBHP_Ptr RGBData, EColour colour);

      // Information functions

    virtual EBlockType HDBType() const { return BT_IMAGE;}
    virtual std::string XTENSION() const;

      // Image information functions

    virtual AXIS_t width() const;
    virtual AXIS_t height() const;
    virtual bool isMonoImage() const;
    virtual bool isPolyImage() const;
    virtual void imageSet(CAstroImage *);
    virtual CAstroImage *imageGet();
    virtual MCL::TPoint2D<FP_t> &getPixelSize() { return pixelSize; }
    virtual FP_t imageExposure() const;

      // Rendering functions

    virtual renderImage_t *getRenderedImage() const;
    virtual void setImagePlaneRenderFunction(size_t, FP_t, FP_t, bool, ETransferFunction, FP_t);
    virtual void setImagePlaneColourValues(size_t, SColourRGB, FP_t);
    virtual void renderImage(ERenderMode);
    virtual FP_t blackPoint();
    virtual FP_t whitePoint();
    virtual FP_t getMaxValue() const;
    virtual FP_t getMeanValue() const;
    virtual FP_t getStDevValue() const;
    virtual FP_t getMinValue() const;
    virtual void objectProfile(MCL::TPoint2D<FP_t> centroid, AXIS_t radius, std::vector<boost::tuple<FP_t, FP_t> > &data) const;

      // Image manipulation functions

    virtual void imageCrop(MCL::TPoint2D<AXIS_t> origen, MCL::TPoint2D<AXIS_t> dims);
    virtual void imageFlip();
    virtual void imageFlop();
    virtual void imageRotate(FP_t);
    virtual void imageFloat(AXIS_t newWidth, AXIS_t newHeight, long newBkgnd);
    virtual void imageResample(AXIS_t, AXIS_t);
    virtual void binPixels(unsigned int);
    virtual void imageTransform(const MCL::TPoint2D<FP_t> &center, const MCL::TPoint2D<FP_t> &co, FP_t const &angle,
                                FP_t const &scale, MCL::TPoint2D<FP_t> const &pixelSize, std::unique_ptr<bool[]> &);

      // Calibration functions

    virtual void calibrationApplyDark(CAstroImage const &);
    virtual void calibrationApplyFlat(CAstroImage const &) { CODE_ERROR(ACL); }

      // Image Analysis functions

    virtual boost::optional<MCL::TPoint2D<FP_t> > centroid(MCL::TPoint2D<AXIS_t> const &, AXIS_t, int) const;
    virtual void findStars(TImageSourceContainer &imageSourceList, SFindSources const &) const;
    virtual bool plateSolve(AXIS_t);

      // Photometry functions

    virtual void pointPhotometry(SPPhotometryObservation);
    virtual boost::optional<double> FWHM(MCL::TPoint2D<FP_t> const &) const;

      // WCS functions

    virtual bool hasWCSData() const;
    virtual boost::optional<CAstronomicalCoordinates> pix2wcs(MCL::TPoint2D<FP_t> const &) const;
    virtual boost::optional<MCL::TPoint2D<FP_t>> wcs2pix(CAstronomicalCoordinates const &) const;
  };

  typedef std::shared_ptr<CImageHDB> CImageHDB_Ptr;

}   // namespace ACL

#endif // ACL_HDBIMAGE_H
