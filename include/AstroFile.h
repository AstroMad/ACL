﻿//*********************************************************************************************************************************
//
// PROJECT:							Astronomy Class Library
// FILE:								CAstroFile
// SUBSYSTEM:						Astronomical File Classes
// LANGUAGE:						C++
// TARGET OS:						None.
// LIBRARY DEPENDANCE:	SCL, cfitsio, boost::filesystem, boost::UUID, LibRaw.
// NAMESPACE:						ACL
// AUTHOR:							Gavin Blakeman (GGB)
// LICENSE:             GPLv2
//
//                      Copyright 2010-2018 Gavin Blakeman.
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
// OVERVIEW:						A number of classes supporting file IO of astronomical data including images. The classes are designed to
//                      present a single interface to the other astro library classes while supporting several different file
//                      types.
//
//											File types supported are: FITS
//																								SBIG image files
//                                                RAW image files
//                                                DNG Image files.
//                      The CAstroFile class is modelled around the FITS file format and is designed to work seamlessly with FITS
//                      files. While opening other types of files is supported, some information will be created on the fly.
//                      To ensure that all information generated by image manipulation is saved correctly, the information must
//                      be saved in the FITS format. Information will be lost if it is saved in any other format.
//
//                      The CAstroFile class also provides some observation management functionality as there are keywords that
//                      are automatically linked to observation information.
//
//                      The first time a file is opened, it is tagged with a UUID to uniquely identify the file.
//
// CLASSES INCLUDED:		CAstroFile      - Base class for for accessing and using FITS files.
//
// CLASS HIERARCHY:     CAstroFile
//                        - CSBIGAstroFile
//                        - CRAWAstroFile
//
// HISTORY:             2015-09-22 GGB - astroManager 2015.09 release
//                      2013-09-30 GGB - astroManager 2013.09 release.
//                      2013-03-22 GGB - astroManager 2013.03 release.
//                      2013-01-20 GGB - astroManager 0000.00 Release.
//                      2012-11-11 GGB - Moved class CFitsAstroFile into its own file.
//                      2011-12-10 GGB - Removed HDB classes into seperate file HDB.h
//                      2011-06-04 GGB - Development of classes for astroManager
//
//*********************************************************************************************************************************

#ifndef ACL_ASTROFILE_H
#define ACL_ASTROFILE_H

  // Standard C++ library header files

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

  // ACL library header files

#include "AstroClass.h"
#include "AstroImage.h"
#include "Astrometry.h"
#include "AstronomicalTime.h"
#include "HDB.h"
#include "HDBAstrometry.h"
#include "HDBAsciiTable.h"
#include "HDBBinTable.h"
#include "HDBImage.h"
#include "HDBPhotometry.h"
#include "FITSKeyword.h"
#include "FITSMemoryFile.h"
#include "SourceExtraction.h"
#include "telescope.h"

  // Miscellaneous libraries

#include "boost/filesystem.hpp"
#include "fitsio.h"
#include <MCL>
#include <SCL>

namespace ACL
{
  class CAstroFile;

  typedef std::shared_ptr<CAstroFile> PAstroFile;         ///< Smart pointer to a CAstroFile object.
  typedef std::vector<PHDB> DHDBStore;

  typedef std::string(*FHDBType)();                       ///< Function template to return the class name of an HDB
  typedef bool(*FHDBTest)(fitsfile *);                    ///< Function template
  typedef PHDB(*FHDBCreate)(CAstroFile *, fitsfile *);    ///< Function template to create a new HDB from the passed HDU.

  struct SHDBRegister
  {
    FHDBType HDBClassName;
    FHDBTest testFunction;
    FHDBCreate createFunction;
  };
  typedef std::vector<SHDBRegister> DHDBRegister;
  typedef DHDBRegister::iterator DHDBRegisterIterator;

  struct SCalibrateImage
  {
      // Dark exposure information

    bool useDarkFrame;
    boost::filesystem::path darkFramePath;
    bool overrideDarkTemperature;
    bool overrideDarkExposure;

      //  Bias exposure information

    bool useBiasFrame;
    boost::filesystem::path biasFramePath;

      // Flat exposure information

    bool useFlatFrame;
    boost::filesystem::path flatFramePath;

      // Additional Information

    bool appendFrames;
    bool saveOriginal;
  };

  typedef boost::shared_ptr<SCalibrateImage> SCalibrateImage_Ptr;

  /// @brief The CAstroFile class corresponds to a FITS file.
  /// @details The class is used to encapsulate a FITS file. When an image is opened from any source, it is opened into the
  /// CAstroFile class. When the file is saved, it is automatically saved as a FITS file. The FITS file format has been chosen as
  /// the storage format for the application.<br>
  /// The FITS files used are all conforming FITS files. There may be additional HDU's attached to store application specific
  /// information, however, this is all done within the FITS standard.<br>
  /// When an astroFile is opened for the first time, or created, it is assigned a UUID to be able to track the file through the
  /// ARID database.<br>
  /// As the class is used to store all the information that can be found in a FITS file, there are a large number of pass-through
  /// functions that are used to manipulate data. This is the facade pattern This encapsulation is preferred to the idiom of
  /// requesting the contained objects and then operating on the contained objects.<br>
  /// The paradigm used is that the FITS file represents a single image. While multiple HDUs are supported, they should all refer
  /// to the same image. There are assumptions (mother of all evil... I know) made in the code that support this. It is possible to
  /// build a fairly complex FITS file if required, however the paradigm is as described below.<br>
  ///   @li One image per file
  ///   @li The primary HDU contains the primary image.
  ///   @li The file may contain additional table HDUs to store astrometry and photometry information. By using these tables, the
  ///       image does not have to be solved everytime it is opened, the information can be simply retrieved from the tables.
  ///   @li The image may contain additional images. These can be the dark frame, the light frame and the bias frame.
  ///   @li It is possible that additional HDUs contain other images, (radar, UV, etc)
  ///   @li Actions that are undertaken on the primary image are applied to all HDUs. (Where the HDU type is supported.) IE, if
  ///       an image is rotated, all the images will be rotated, the astrometry and photometry HDU will also be rotated and the
  ///       WCS information will be rotated. (If I can work out the maths)
  ///   @li Additional restrictions can be applied if required...

  class CAstroFile
  {
  private:
    PHDBAstrometry astrometryHDB_;                                      ///< The HDB storing astrometry specific information.
    PHDBPhotometry photometryHDB_;                                      ///< The HDB that stores photometry information

  protected:
    std::string imageName_;                                                         ///< The image name to identify the file.
    static DHDBRegister HDBRegister;                                                ///< All the registered HDB types.
    std::unique_ptr<CObservatory> observationLocation;                              ///< The location of the observation location
    std::unique_ptr<CWeather> observationWeather;                                   ///< Weather at time of observation
    std::unique_ptr<CAstroTime> observationTime;                                    ///< Time of the observation
    std::unique_ptr<CTargetAstronomy> observationTarget;                            ///< Coordinates of the observation.
    std::unique_ptr<CTelescope> observationTelescope;                               ///< Inforrmation about the telescope used.
    DHDBStore HDB;                                                                  ///< All the data blocks (HDU) in the file.
    mutable bool bDirty;                                                            ///< Has the data changed since the last save?
    mutable bool bHasData;                                                          ///< Is there data in the file?

    virtual void loadFromSBIG(boost::filesystem::path const &);
#ifdef USE_LIBRAW
    virtual void loadFromRaw();
#endif // USE_LIBRAW

    virtual void loadFromFITS(fitsfile *);
    virtual void saveAsFITS(fitsfile *);

    void deleteAllData();

    void loadExtension(fitsfile *, int);

      // Special Keyword functions

    virtual void processSpecialKeywords();
    virtual void processObservationTelescope();
    virtual void processObservationTime();
    virtual void processTargetCoordinates();
    virtual void processObservationLocation();
    virtual void processWeather();

    virtual CImageHDB_Ptr createImageHDB(std::string const &);
    virtual CATableHDB_Ptr createATableHDB(std::string const &);
    virtual CBTableHDB_Ptr createBTableHDB(std::string const &);

      // Calibration functions

    virtual bool validateDarkDuration(PAstroFile &);
    virtual bool validateDarkTemperature(PAstroFile &);
    virtual bool validateImageDimensions(PAstroFile &);
    virtual void calibrateFlat(PAstroFile, SCalibrateImage_Ptr);
    virtual void calibrateDark(PAstroFile, PAstroFile, SCalibrateImage_Ptr);

  public:
    explicit CAstroFile();
    CAstroFile(CAstroFile const &);
    CAstroFile(std::string const &);
    CAstroFile(CAstroImage *);
    virtual ~CAstroFile();		// Ensure that this is a virtual class.

      // Factory functions.

    virtual std::unique_ptr<CAstroFile> createCopy() const;


    void copyKeywords(CAstroFile const &, DHDBStore::size_type);

    std::string getImageName();
    void imageName(std::string const &imageName) { imageName_ = imageName; }

    inline virtual bool isDirty() const { return bDirty; }
    inline virtual bool hasData() const { return bHasData; }
    inline virtual void isDirty(bool b) const { bDirty = b;}
    inline virtual void hasData(bool b) const { bHasData = b;}

      // IO Functions

    virtual void loadFromFile(boost::filesystem::path const &);                          // load entire file
    virtual void loadFromMemory(CFITSMemoryFile &);
    virtual void save(boost::filesystem::path const &);
    virtual void save(CFITSMemoryFile &);

      // HDB Management Functions

    bool createPrimaryHDB();
    bool createPrimaryImageHDB();
    static bool HDBRegisterClass(SHDBRegister &);
    void HDBAdd(PHDB);
    PHDB HDBCreateClass(fitsfile *);
    inline size_t HDBCount() const { return HDB.size();}
    std::string HDBName(DHDBStore::size_type = 0) const;
    EBlockType HDBType(DHDBStore::size_type = 0) const;
    EBlockType HDBType(std::string const &) const;
    PHDB &getHDB(DHDBStore::size_type = 0) const;

      // Keyword information

    bool keywordData(DHDBStore::size_type, std::string const &, std::string &, std::string &) const;
    CFITSKeyword const &keywordData(DHDBStore::size_type, std::string const &) const;
    bool keywordExists(DHDBStore::size_type, std::string const &) const;
    DKeywordStore::size_type keywordCount(DHDBStore::size_type) const;
    KWType keywordType(DHDBStore::size_type, std::string const &) const;
    DKeywordStore &keywords(DHDBStore::size_type);
    void keywordWrite(DHDBStore::size_type, PFITSKeyword &);
    bool keywordDelete(DHDBStore::size_type, std::string const &);

    void keywordWrite(DHDBStore::size_type, std::string const &, std::int8_t const &, std::string const &);
    void keywordWrite(DHDBStore::size_type, std::string const &, std::int16_t const &, std::string const &);
    void keywordWrite(DHDBStore::size_type, std::string const &, std::int32_t const &, std::string const &);
    void keywordWrite(DHDBStore::size_type, std::string const &, std::int64_t const &, std::string const &);
    void keywordWrite(DHDBStore::size_type, std::string const &, std::uint8_t const &, std::string const &);
    void keywordWrite(DHDBStore::size_type, std::string const &, std::uint16_t const &, std::string const &);
    void keywordWrite(DHDBStore::size_type, std::string const &, std::uint32_t const &, std::string const &);
    void keywordWrite(DHDBStore::size_type, std::string const &, float const &, std::string const &);
    void keywordWrite(DHDBStore::size_type, std::string const &, double const &, std::string const &);
    void keywordWrite(DHDBStore::size_type, std::string const &, std::string const &, std::string const &);

      // Axis functions.

    NAXIS_t NAXIS(size_t = 0) const;
    AXIS_t NAXISn(size_t = 0, size_t = 0) const;

      // Drill down access

    CAstroImage *getAstroImage(DHDBStore::size_type) const;
    void setAstroImage(DHDBStore::size_type, CAstroImage *);

      // Comments and history.

    void commentWrite(DHDBStore::size_type, const std::string &);
    std::string commentGet(DHDBStore::size_type) const;
    void historyWrite(DHDBStore::size_type, const std::string &);
    std::string historyGet(DHDBStore::size_type) const;

      // Image information

    bool isMonoImage(DHDBStore::size_type) const;
    bool isPolyImage(DHDBStore::size_type) const;
    virtual AXIS_t imageHeight(DHDBStore::size_type = 0) const;
    virtual AXIS_t imageWidth(DHDBStore::size_type = 0) const;
    virtual FP_t blackPoint(DHDBStore::size_type = 0);
    virtual FP_t whitePoint(DHDBStore::size_type = 0);
    virtual FP_t imageMax(DHDBStore::size_type = 0) const;
    virtual FP_t imageMean(DHDBStore::size_type = 0) const;
    virtual FP_t imageStdev(DHDBStore::size_type = 0) const;
    virtual FP_t imageMin(DHDBStore::size_type = 0) const;
    void objectProfile(DHDBStore::size_type, MCL::TPoint2D<FP_t> centroid, AXIS_t radius, std::vector<boost::tuple<FP_t, FP_t> > &data) const;
    virtual std::string imageFilter() const;
    virtual FP_t imageExposure(DHDBStore::size_type = 0) const;

      // Image transform actions

    virtual void flipImage(DHDBStore::size_type =0);
    virtual void flopImage(DHDBStore::size_type =0);
    virtual void rotateImage(DHDBStore::size_type, FP_t);
    virtual void imageFloat(DHDBStore::size_type, AXIS_t newWidth, AXIS_t newHeight, long newBkgnd);
    virtual void imageResample(DHDBStore::size_type, AXIS_t, AXIS_t);
    virtual void binPixels(DHDBStore::size_type, unsigned int);
    virtual void TRS(DHDBStore::size_type, MCL::TPoint2D<FP_t> const &, MCL::TPoint2D<FP_t> const &, FP_t, FP_t, MCL::TPoint2D<FP_t> const &, std::unique_ptr<bool> &);
    virtual void imageCrop(DHDBStore::size_type hdb, AXIS_t xo, AXIS_t yo, AXIS_t xd, AXIS_t yd);
    virtual void imageCrop(DHDBStore::size_type hdb, MCL::TPoint2D<AXIS_t> origen, MCL::TPoint2D<AXIS_t> dims);

      // Image actions

    virtual renderImage_t *getRenderedImage(DHDBStore::size_type) const;
    virtual std::optional<MCL::TPoint2D<FP_t>> centroid(DHDBStore::size_type, MCL::TPoint2D<AXIS_t> const &, AXIS_t, int) const;
    virtual void setImagePlaneRenderFunction(DHDBStore::size_type, size_t, FP_t, FP_t, bool, ETransferFunction, FP_t);
    virtual void setImagePlaneColourValues(DHDBStore::size_type, size_t, SColourRGB, FP_t);
    virtual void renderImage(DHDBStore::size_type, ERenderMode);

      // Site/Observation information

    virtual CWeather *getObservationWeather();
    virtual CObservatory *getObservationLocation();
    virtual CTelescope *getObservationTelescope() { return observationTelescope.get(); }
    virtual CAstroTime const &getObservationTime() const;
    virtual void setObservationWeather(std::unique_ptr<CWeather>);
    //virtual void setObservationLocation(PLocation);
    virtual void setObservationTime(CAstroTime const &);

      // Target information

    virtual CAstronomicalCoordinates const &getTargetCoordinates() const;
    virtual std::string getObservationTarget() const;

      // Image Analysis Functions

    virtual void findStars(DHDBStore::size_type, TImageSourceContainer &, const SFindSources &parameters) const;
    virtual bool plateSolve(DHDBStore::size_type, AXIS_t);

      // Astrometry functions

    PHDBAstrometry createAstrometryHDB();
    PHDBAstrometry astrometryHDB();
    bool hasAstrometryHDB() const;
    bool astrometryObjectAdd(SPAstrometryObservation);
    bool astrometryObjectRemove(std::string const &);
    void astrometryObjectRemoveAll();
    void astrometryCalculatePlateConstants();
    bool astrometryCheckRequisites() const;
    SPAstrometryObservation astrometryObjectFirst();
    SPAstrometryObservation astrometryObjectNext();
    size_t astrometryObjectCount() const;

      // Photometry functions

    size_t photometryObjectCount() const;
    PHDBPhotometry createPhotometryHDB();
    PHDBPhotometry photometryHDB();
    bool hasPhotometryHDB() const;
    bool photometryObjectAdd(SPPhotometryObservation);
    bool photometryObjectRemove(std::string const &);
    void photometryObjectRemoveAll();
    SPPhotometryObservation photometryObjectFirst();
    SPPhotometryObservation photometryObjectNext();
    std::optional<FP_t> FWHM();                                     // Determine the mean Full-Width-Half-Max for the image.
    std::optional<FP_t> FWHM(DHDBStore::size_type, MCL::TPoint2D<FP_t> const &) const;  // Determine the Full-Width-Half-Max for an image star.
    virtual void pointPhotometry(DHDBStore::size_type, SPPhotometryObservation);

      // WCS functions

    bool hasWCSData(DHDBStore::size_type) const;
    std::optional<CAstronomicalCoordinates> pix2wcs(DHDBStore::size_type, MCL::TPoint2D<FP_t> const &) const;
    std::optional<MCL::TPoint2D<FP_t>> wcs2pix(DHDBStore::size_type, CAstronomicalCoordinates const &) const;

      // Calibration functions

    virtual void calibrateImage(SCalibrateImage_Ptr);
  };

}     // namespace ACL

#endif // ACL_ASTROFILE_H
