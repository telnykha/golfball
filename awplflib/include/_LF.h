/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                  Locate Framework  Computer Vision Library
//
// Copyright (C) 2004-2018, NN-Videolab.net, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the NN-Videolab.net or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//      Locate Framework  (LF) Computer Vision Library.
//		File: _LF.h
//		Purpose: Contains internals for Locate Face 
//
//      CopyRight 2004-2018 (c) NN-Videolab.net
//M*/

#ifndef ___LF_H__
#define ___LF_H__

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <fstream>
#include <list>
#include <algorithm>
#include <string>
#include <sstream>
#include <assert.h>
#include <float.h>
#include <iostream>
#ifdef WIN32
 #include <strstream>
#endif
#include <map>
#include <vector>
#include <cctype>
#include <string>

#ifdef WIN32
    #include <io.h>
    #include <direct.h>
#else
    #include <dirent.h>
#endif
#ifdef _OMP_
	#include <omp.h>
#endif

extern "C"
{
#include "awpipl.h"  
}
#include "LF.h"

typedef struct tagawpColor
{
	/** red channel */
	AWPBYTE bRed;
	/** green channel */
	AWPBYTE bGreen;
	/** blue channel */
	AWPBYTE bBlue;
} awpColor;


// определение структуры пр€моугольника интереса
typedef struct tagRectOfInterst
{
	awpPoint p;          /*точка прив€зки - левый глаз*/
	awpPoint p1;         /*точка прив€зки - правый глаз*/
	awpRect  r;          /*пр€моугольнк*/
	int      index;      /*ID, используетс€ в системе сопровождени€*/
	double  err[8];      /*оценки дл€ пр€моугольника
						 err[0] - интегральна€ оценка
						 err[1] - рассто€ние от изображени€ до шаблона
						 err[2] - рассто€ние от изображени€ левого глаза до шаблона
						 err[3] - рассто€ние от изображени€ правого глаза до шаблона
						 err[4] - оценка присутстви€ цвета в roi
						 err[5] - зарезервировано*/

	char class_name[32];

	void     TROI(awpPoint pp1, awpPoint pp2);
	double   Orientation() const;
	double   OprientationGrad() const;
	double   EyeDistance() const;
	double   Width() const;
	double   Height() const;
	awpRect  GetROIRectGCS() const;
	awpPoint GetRoiCenter() const;
	// координаты ограничивающего пр€моугольника в GCS (Global Coordinate System)
	awpPoint gLeftTop() const;
	awpPoint gLeftBottom() const;
	awpPoint gRightTop() const;
	awpPoint gRightBottom() const;
	//  оординаты ограничивающего пр€моугольника в LCS (Local Coordinate Sysyem)
	awp2DPoint lLeftTop() const;
	awp2DPoint lLeftBottom() const;
	awp2DPoint lRightTop() const;
	awp2DPoint lRightBottom() const;

	// координаты пр€моугольника AOI
	awpPoint aoiLeftTop() const;
	awpPoint aoiLeftBottom() const;
	awpPoint aoiRightTop() const;
	awpPoint aoiRightBottom() const;

	awpPoint aoiTop() const;
	awpPoint aoiLeft() const;
	awpPoint aoiRight() const;
	awpPoint aoiBottom() const;

	awpRect  inflate(double scale);

	// визуализаци€ ROI (пр€моугольника интереса)
	bool IsROIInImage(awpImage* pImage);

	void DrawROIRect(awpImage* pImage, double scale, awpColor c);
	void DrawROIPoints(awpImage* pImage, double scale, awpColor c);
	void DrawROI(awpImage* pImage, double scale, awpColor c);
	void DrawAOI(awpImage* pImage, double scale, awpColor c);
	void DrawAOIRect(awpImage* pImage, double scale, awpColor c);

	// проверка точки
	bool IsPointInLCS(awp2DPoint pp);
	awpImage* GetRoiImage(awpImage* pImage, bool big = false) const;
	awpImage* GetRoiImageSource(awpImage* pImage);
	awpImage* GetRoiLowFrequency(awpImage* pImage);
	awpImage* GetRoiFeatures(awpImage* pImage, bool big = false);
}TROI;

/*
Region of interest
implementation LFUtils.cpp
*/
class TLFRoi : public  TLFObject
{
protected:
	TROI        m_roi;
public:
	TLFRoi();
	TROI     GetRoi();
	void     SetRoi(awpPoint lp, awpPoint rp);
	void     SetRoi(TROI& roi);
	void     SetRoi(const TROI* pRoi);
	void     Shift(int dx, int dy);
	void     Scale(double f);
	awpImage* GetRoiImage(awpImage* pImageIn, int Lxs, int Lys, awpPoint P1s, awpPoint P2s);
	virtual const char* GetName()
	{
		return "TLFRoi";
	}
};


#include "LFVector.h"
#include "LFDistance.h"
#include "LFHistogramm.h"
#include "LFBuffer.h"
#include "LFCSFeature.h"

#include "LFUtils.h"
#include "LFFeatures.h"
#include "LFSample.h"
#include "LFCluster.h"
#include "LFWeak.h"
#include "LFStrong.h"
#include "LFDetector.h"
#include "LFAttrFilter.h"
#include "LFColorDetector.h"
#include "LFScanners.h"
#include "LFEye.h"
#include "LFGeometry.h"
#include "LFImageProcessor.h"
#include "LFThresholdProc.h"
#include "LFEngine.h"
#include "LFParameter.h"
#include "LFAvgFeature.h"
#include "LFSabotage.h"
#include "LFPredictors.h"
#include "LFZone.h"
#include "LFLabColor.h"
#include "LFMotion.h"
#include "LFFace.h"
#include "LFFileUtils.h"
#include "LFSemantic.h"
#include "LFDatabase.h"
#include "LFCSBoost.h"
#include "LFBuilder.h"
#include "LFMCDetector.h"
#include "LFLandmarks.h"
/** \defgroup LFFactory
*	Locate Framework factory routines 
*   @{
*/
ILFPredictor* LFCreatePredictor(const char* lpName, ILFDetectEngine* engine);
ILFFeature*   LFCreateFeature(ILFFeature* feature);
ILFFeature*   LFCreateFeature(const char* lpName, int sx, int sy, int w, int h);
ILFFeature*   LFCreateFeature(const char* lpName, TiXmlElement* parent);
ILFFeature*   LFCreateFeature(TiXmlElement* parent);

ILFWeak*      LFCreateWeak(ILFWeak* weak);
ILFWeak*      LFCreateWeak(const char* lpName);
ILFStrong*    LFCreateStrong(const char* lpName);
awpRect		  LFRectToFeatureBlock(const char* lpName, awpRect& rect);

/** @} */ /*  end of LFFactory group */

template <class T>
std::string TypeToStr( const T & t )
{
std::ostringstream oss; // create a stream
oss << t; // insert value to stream
return oss.str(); // extract string and copy
}

template <class T>
T StrToType( std::string const& val )
{
std::stringstream ss; // create a stream
ss << val; // insert value to stream
T t;
ss >> t;
return t;
}

#endif// __LF_H__
