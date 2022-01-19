//---------------------------------------------------------------------------
#include "_LF.h"

#define AOI_W 32
#define AOI_H 32

#ifndef __BCPLUSPLUS__
inline void randomize( )
{
    srand((unsigned)time(NULL));
}

inline int random( int d )
{
    return (int)((double)rand()/(double)RAND_MAX * d);
}
#endif


#define __CHECK_AWP_ROI_IMAGE__(pImage) \
    if (pImage == NULL)  \
        return NULL;      \
    if (pImage->sSizeX != 20 || pImage->sSizeY != 24)\
        return NULL;                                 \
    if (pImage->bChannels != 1)                      \
        return NULL;                                 \
    if (pImage->dwType != AWP_BYTE)                  \
        return NULL;


static awpRect GetAOIRect(awpPoint p1, awpPoint p2,double c1, double c2)
{
    double L = (p2.X - p1.X);
    int w = (int)(L / c1);//(roi.EyeDistance();
    int h = w;
    awpRect aoi_r;
    aoi_r.left  = p1.X - (short)(L*c2);
    aoi_r.right = aoi_r.left +w;
    aoi_r.top = p1.Y - (short)(L*c2);
    aoi_r.bottom = aoi_r.top + h;

    return  aoi_r;
}

//---------------------------------------------------------------------------
// преобразование точки из локальной системы координат LCS в глобальную ситему
// координат. GCS
// p  - коордитнаты точки в LCS
// fi - угол поворота LCG относительно GCS [-Pi/2....Pi/2]
// dx - смещение LCS относительно GCS по x
// dy - смещение LCS относительно GCS по y
// возвращаемое значение - координаты точки в GCS
static awp2DPoint LcsToGcsPoint(awp2DPoint p, double fi, double dx, double dy)
{
    double sn = sin (fi);
    double cs = cos(fi);
    awp2DPoint res;
    res.X = p.X*cs - p.Y*sn + dx;
    res.Y = p.Y*cs + p.X*sn + dy;
    return res;
}
//преобразование из глобальной системы коордлинат GCS в локальную систему LCS
// p   - координаты точки в GCS
// fi  - угол поворота LCS относительно GCS
// lcs - начало LCS относительно GCS
static awp2DPoint GcsToLcsPoint(awp2DPoint p, double fi, awp2DPoint lcs)
{
    awp2DPoint res;
    //смещение точки
    res.X = p.X - lcs.X;
    res.Y = p.Y - lcs.Y;
    //вращение точки
    double sn = sin (fi);
    double cs = cos(fi);
    res.X = res.X*cs - res.Y*sn;
    res.Y = res.Y*cs + res.X*sn;
    return res;
}
//ROI functions-----------------------------------------------------------------
// конструктор
void TROI::TROI(awpPoint pp1, awpPoint pp2)
{
   p    = pp1;
   p1   = pp2;
   index = -1;
   memset(err, 0, sizeof(err));
   r = GetAOIRect(p,p1, 0.4, 0.7);
   memset(class_name, 0, sizeof(class_name));

}
// возвращает угол поворота ROI от -PI/2 до PI/2
double TROI::Orientation() const
{
    double dx = p1.X - p.X;
    double dy = p1.Y - p.Y;
    if (dx == 0)
        return 3.14159 / 2;
    else
        return atan(dy/dx);
}
// возвращает угол поворота от -90 до 90 градусов
double   TROI::OprientationGrad() const
{
	double fi = Orientation();
	return 180*fi / 3.14159;
}

//возвращает расстояния между глазами
double TROI::EyeDistance() const
{
    return sqrt((double)(p1.X - p.X)*(p1.X - p.X) + (p1.Y-p.Y)*(p1.Y - p.Y));
}
//возвращает ширину прямоугольной области вокруг ROI
double TROI::Width()const
{
    return  2.0*EyeDistance();
}
//возвращает высоту прямоугольной области вокруг ROI
double TROI::Height()const
{
    return 1.2*Width();
}

awpPoint TROI::GetRoiCenter() const
{
	AWPDOUBLE d = EyeDistance() /2;
	awpPoint ce;
	ce.X =(p.X+p1.X)/2; 
	ce.Y =(p.Y +p1.Y)/2;
	AWPDOUBLE fi = AWP_PI / 2 + Orientation();
	awpPoint res;
	res.X = (AWPSHORT)(ce.X + d*cos(fi));
	res.Y = (AWPSHORT)(ce.Y + d*sin(fi));
	return res;
}

//возвращает прямоугольник в GCS
awpRect TROI::GetROIRectGCS() const
{
    awpRect res;
    awpPoint lt = gLeftTop();
    awpPoint lb = gLeftBottom();
    awpPoint rt = gRightTop();
    awpPoint rb = gRightBottom();

    res.left   = AWP_MIN(lt.X, lb.X);
    res.right  = AWP_MAX(rt.X, rb.X);

    res.top    = AWP_MIN(lt.Y, rt.Y);
    res.bottom = AWP_MAX(lb.Y, rb.Y);

    return res;
}
//возвращает левый верхний угол ROI в GCS
awpPoint TROI::gLeftTop() const
{
  awpPoint res;
  awp2DPoint pp;
  awp2DPoint lt = lLeftTop();
  double fi = Orientation();
  pp = LcsToGcsPoint(lt, fi, p.X, p.Y);
  res.X = (AWPSHORT)pp.X;
  res.Y = (AWPSHORT)pp.Y;
  return res;
}
//возвращает левый-нижний угол ROI в GCS
awpPoint TROI::gLeftBottom() const
{
  awpPoint res;
  awp2DPoint pp;
  awp2DPoint lt = lLeftBottom();
  double fi = Orientation();
  pp = LcsToGcsPoint(lt, fi, p.X, p.Y);
  res.X = (AWPSHORT)pp.X;
  res.Y = (AWPSHORT)pp.Y;
  return res;
}
//возвращает правый-верхний угол ROI в GCS
awpPoint TROI::gRightTop() const
{
  awpPoint res;
  awp2DPoint pp;
  awp2DPoint lt = lRightTop();
  double fi = Orientation();
  pp = LcsToGcsPoint(lt, fi, p.X, p.Y);
  res.X = (AWPSHORT)pp.X;
  res.Y = (AWPSHORT)pp.Y;
  return res;
}
//возвращает правый-нижний угол ROI в GCS
awpPoint TROI::gRightBottom() const
{
  awpPoint res;
  awp2DPoint pp;
  awp2DPoint lt = lRightBottom();
  double fi = Orientation();
  pp = LcsToGcsPoint(lt, fi, p.X, p.Y);
  res.X = (AWPSHORT)pp.X;
  res.Y = (AWPSHORT)pp.Y;
  return res;
}

awpPoint TROI::aoiLeftTop() const
{
    awp2DPoint p1;
    awp2DPoint LCS;
    awp2DPoint plcs;
    awp2DPoint pres;
    awpPoint    res;

    double fi = 0;

    if (err[4] == 2)
        fi = -3.14/8;
    else if (err[4] == 3)
        fi = 3.14 / 8;

    LCS.X = p.X;
    LCS.Y = p.Y;

    p1.X = r.left;
    if (err[4] == 4)
       p1.X = (r.left + r.right) / 2;

    p1.Y = r.top;

    plcs = GcsToLcsPoint(p1, 0, LCS);
    pres = LcsToGcsPoint(plcs, fi, p.X, p.Y);

    res.X = (AWPSHORT)pres.X;
    res.Y = (AWPSHORT)pres.Y;

    return res;
}
awpPoint TROI::aoiLeftBottom() const
{
    awp2DPoint p1;
    awp2DPoint LCS;
    awp2DPoint plcs;
    awp2DPoint pres;
    awpPoint    res;

    double fi = 0;

    if (err[4] == 2)
        fi = -3.14/8;
    else if (err[4] == 3)
        fi = 3.14 / 8;

    LCS.X = p.X;
    LCS.Y = p.Y;

    p1.X = r.left;
    if (err[4] == 4)
       p1.X = (r.left + r.right) / 2;

    p1.Y = r.bottom;

    plcs = GcsToLcsPoint(p1, 0, LCS);
    pres = LcsToGcsPoint(plcs, fi, p.X, p.Y);

    res.X = (AWPSHORT)pres.X;
    res.Y = (AWPSHORT)pres.Y;

    return res;
}
awpPoint TROI::aoiRightTop() const
{
    awp2DPoint p1;
    awp2DPoint LCS;
    awp2DPoint plcs;
    awp2DPoint pres;
    awpPoint    res;

    LCS.X = p.X;
    LCS.Y = p.Y;

    p1.X = r.right;
    p1.Y = r.top;

    double fi = 0;

    if (err[4] == 2)
        fi = -3.14/8;
    else if (err[4] == 3)
        fi = 3.14 / 8;

    if (err[4] == 5)
       p1.X = (r.left + r.right) / 2;

    plcs = GcsToLcsPoint(p1, 0, LCS);
    pres = LcsToGcsPoint(plcs, fi, p.X, p.Y);

    res.X = (AWPSHORT)pres.X;
    res.Y = (AWPSHORT)pres.Y;

    return res;
}
awpPoint TROI::aoiRightBottom() const
{
    awp2DPoint p1;
    awp2DPoint LCS;
    awp2DPoint plcs;
    awp2DPoint pres;
    awpPoint    res;

    LCS.X = p.X;
    LCS.Y = p.Y;

    p1.X = r.right;
    if (err[4] == 5)
       p1.X = (r.left + r.right) / 2;
    p1.Y = r.bottom;

    double fi = 0;

    if (err[4] == 2)
        fi = -3.14/8;
    else if (err[4] == 3)
        fi = 3.14 / 8;

    plcs = GcsToLcsPoint(p1, 0, LCS);
    pres = LcsToGcsPoint(plcs, fi, p.X, p.Y);

    res.X = (AWPSHORT)pres.X;
    res.Y = (AWPSHORT)pres.Y;

    return res;
}

awpPoint TROI::aoiTop() const
{
    awp2DPoint p1;

    awp2DPoint LCS;
    awp2DPoint plcs;
    awp2DPoint pres;
    awpPoint    res;

    double fi = 0;

    if (err[4] == 2)
        fi = -3.14/8;
    else if (err[4] == 3)
        fi = 3.14 / 8;

    LCS.X = p.X;
    LCS.Y = p.Y;

    p1.X = (r.left + r.right)/2;
    p1.Y = r.top;

    plcs = GcsToLcsPoint(p1, 0, LCS);
    pres = LcsToGcsPoint(plcs, fi, p.X, p.Y);

    res.X = (AWPSHORT)pres.X;
    res.Y = (AWPSHORT)pres.Y;

    return res;
}

awpPoint TROI::aoiLeft() const
{
    awp2DPoint p1;
    awp2DPoint LCS;
    awp2DPoint plcs;
    awp2DPoint pres;
    awpPoint    res;

    double fi = 0;

    if (err[4] == 2)
        fi = -3.14/8;
    else if (err[4] == 3)
        fi = 3.14 / 8;

    LCS.X = p.X;
    LCS.Y = p.Y;

    p1.X = r.left;
    p1.Y = (r.bottom + r.top) / 2;

    plcs = GcsToLcsPoint(p1, 0, LCS);
    pres = LcsToGcsPoint(plcs, fi, p.X, p.Y);

    res.X = (AWPSHORT)pres.X;
    res.Y = (AWPSHORT)pres.Y;

    return res;
}

awpPoint TROI::aoiRight() const
{
    awp2DPoint p1;
    awp2DPoint LCS;
    awp2DPoint plcs;
    awp2DPoint pres;
    awpPoint    res;

    LCS.X = p.X;
    LCS.Y = p.Y;

    p1.X = r.right;
    p1.Y = (r.bottom + r.top) / 2;

    double fi = 0;

    if (err[4] == 2)
        fi = -3.14/8;
    else if (err[4] == 3)
        fi = 3.14 / 8;

    plcs = GcsToLcsPoint(p1, 0, LCS);
    pres = LcsToGcsPoint(plcs, fi, p.X, p.Y);

    res.X = (AWPSHORT)pres.X;
    res.Y = (AWPSHORT)pres.Y;

    return res;
}

awpRect  TROI::inflate(double scale)
{
    awpRect res;
	double w2 = (double)(r.right - r.left);
	AWPSHORT delta = (AWPSHORT)floor(0.5 + (w2*scale - w2) / 2.);

    res.left   = r.left - delta;
    res.top    = r.top - delta;
    res.right  = r.right+ delta;
    res.bottom = r.bottom+ delta;
    return res;
}

awpPoint TROI::aoiBottom() const
{
    awp2DPoint p1;
    awp2DPoint LCS;
    awp2DPoint plcs;
    awp2DPoint pres;
    awpPoint    res;

    LCS.X = p.X;
    LCS.Y = p.Y;

    p1.X = (r.right+ r.left) / 2;
    p1.Y = r.bottom;

    double fi = 0;

    if (err[4] == 2)
        fi = -3.14/8;
    else if (err[4] == 3)
        fi = 3.14 / 8;

    plcs = GcsToLcsPoint(p1, 0, LCS);
    pres = LcsToGcsPoint(plcs, fi, p.X, p.Y);

    res.X = (AWPSHORT)pres.X;
    res.Y = (AWPSHORT)pres.Y;

    return res;
}


// Local Coordinate System utilites
awp2DPoint TROI::lLeftTop() const
{
    awp2DPoint res;
    res.X = -Width() / 4.0;
    res.Y = -Height() / 4.0;
    return res;
}
awp2DPoint TROI::lLeftBottom() const
{
    awp2DPoint res;
    res.X = -Width() / 4.0;
    res.Y = 3*Height() / 4.0;
    return res;
}

awp2DPoint TROI::lRightTop() const
{
    awp2DPoint res;
    res.X = 3*Width() / 4.0;
    res.Y = -Height() / 4.0;
    return res;
}

awp2DPoint TROI::lRightBottom() const
{
    awp2DPoint res;
    res.X = 3*Width() / 4.0;
    res.Y = 3*Height() /4.0;
    return res;
}
//
bool TROI::IsROIInImage(awpImage* pImage)
{
    return awpRectInImage(pImage, &r) == AWP_OK;
}
//
bool TROI::IsPointInLCS(awp2DPoint pp)
{
   return (pp.X > -Width() /4 && pp.X < 3*Width() /4
   && pp.Y > -Height() / 4 && pp.Y < 3*Height() / 4);
}

//рисует прямоугольник, ограничивающий ROI в GCS
void TROI::DrawROIRect(awpImage* pImage, double scale, awpColor c)
{
    if (awpCheckImage(pImage) != AWP_OK)
        return;
    awpRect rr =  GetROIRectGCS();
    rr = awpResizeRect(rr, scale);
    if (awpRectInImage(pImage, &rr) != AWP_OK)
        return;
    if (pImage->dwType == AWP_BYTE && pImage->bChannels == 1)
    {
        // полутоновое изображение
        AWPBYTE br = (c.bRed + c.bGreen + c.bBlue) / 3;
        awpDrawRect(pImage, &rr, 0, br, 0);
    }
    else if (pImage->dwType == AWP_BYTE && pImage->bChannels == 3)
    {
        awpDrawCRect(pImage, &rr, c.bRed, c.bGreen, c.bBlue, 0);
    }
    else
        return;
}
//рисует "глаза" в GCS
void TROI::DrawROIPoints(awpImage* pImage, double scale, awpColor c)
{
    if (awpCheckImage(pImage) != AWP_OK)
        return;
    const int cross_size = int(EyeDistance() / 4);

    awp2DPoint le;
    awp2DPoint re;
	awp2DPoint ce;

    le.X = p.X;    // координаты левого глаза
    le.Y = p.Y;

    re.X = p1.X;  // координаты правого глаза
    re.Y = p1.Y;

	awpPoint cc = GetRoiCenter();
	ce.X = cc.X;
	ce.Y = cc.Y;
	ce.X *= scale;
	ce.Y *= scale;

    le.X *= scale; le.Y *= scale;
    re.X *= scale; re.Y *= scale;

    awpRect lle; // левый крест
    awpRect rle; // правый крест
	awpRect cle; // центральный крест

    lle.left   = (AWPSHORT)le.X - cross_size;
    lle.right  = (AWPSHORT)le.X + cross_size;
    lle.top    = (AWPSHORT)le.Y - cross_size;
    lle.bottom = (AWPSHORT)le.Y + cross_size;

    if (awpRectInImage(pImage, &lle) != AWP_OK)
        return;

    rle.left   = (AWPSHORT)re.X - cross_size;
    rle.right  = (AWPSHORT)re.X + cross_size;
    rle.top    = (AWPSHORT)re.Y - cross_size;
    rle.bottom = (AWPSHORT)re.Y + cross_size;

    if (awpRectInImage(pImage, &rle) != AWP_OK)
        return;

	cle.left   = (AWPSHORT)ce.X - cross_size;
    cle.right  = (AWPSHORT)ce.X + cross_size;
    cle.top    = (AWPSHORT)ce.Y - cross_size;
    cle.bottom = (AWPSHORT)ce.Y + cross_size;

    if (awpRectInImage(pImage, &cle) != AWP_OK)
        return;

    if (pImage->dwType == AWP_BYTE && pImage->bChannels == 1)
    {
        // полутоновое изображение
        AWPBYTE br = (c.bRed + c.bGreen + c.bBlue) / 3;
        awpDrawCross(pImage, &lle, 0, br,0);
        awpDrawCross(pImage, &rle, 0, br,0);
//        awpDrawCross(pImage, &cle, 0, br,0);

    }
    else if (pImage->dwType == AWP_BYTE && pImage->bChannels == 3)
    {
        awpDrawCCross(pImage, &lle, c.bRed, c.bGreen, c.bBlue,0);
        awpDrawCCross(pImage, &rle, c.bRed, c.bGreen, c.bBlue,0);
//        awpDrawCCross(pImage, &cle, c.bRed, c.bGreen, c.bBlue,0);
    }
    else
        return;
}
//рисует прямоугольник повернутый на угол fi в GCS
void TROI::DrawROI(awpImage* pImage, double scale, awpColor c)
{
    if (awpCheckImage(pImage) != AWP_OK)
        return;
    awpRect rr =  awpResizeRect(r, scale);
    if (awpRectInImage(pImage, &rr) != AWP_OK)
        return;

    if (pImage->dwType == AWP_BYTE && pImage->bChannels == 1)
    {
        // полутоновое изображение
        AWPBYTE br = (c.bRed + c.bGreen + c.bBlue) / 3;

        awpPoint pp  = gLeftTop();
        awpPoint pp1 = gRightTop();
        pp.X = AWPSHORT(pp.X*scale); pp.Y = AWPSHORT(pp.Y*scale);
        pp1.X = AWPSHORT(pp1.X*scale); pp1.Y = AWPSHORT(pp1.Y*scale);
        awpDrawLine(pImage, pp, pp1, 0, br, 0);

        pp  = pp1;
        pp1 = gRightBottom();
        pp1.X = AWPSHORT(pp1.X*scale); pp1.Y = AWPSHORT(pp1.Y*scale);
        awpDrawLine(pImage, pp, pp1, 0, br, 0);

        pp  = pp1;
        pp1 = gLeftBottom();
        pp1.X = AWPSHORT(pp1.X*scale); pp1.Y = AWPSHORT(pp1.Y*scale);
        awpDrawLine(pImage, pp, pp1, 0, br, 0);

        pp  = pp1;
        pp1 = gLeftTop();
        pp1.X = AWPSHORT(pp1.X*scale); pp1.Y = AWPSHORT(pp1.Y*scale);
        awpDrawLine(pImage, pp, pp1, 0, br, 0);

    }
    else if (pImage->dwType == AWP_BYTE && pImage->bChannels == 3)
    {
        awpPoint pp  = gLeftTop();
        awpPoint pp1 = gRightTop();
        pp.X = AWPSHORT(pp.X*scale); pp.Y = AWPSHORT(pp.Y*scale);
        pp1.X = AWPSHORT(pp1.X*scale); pp1.Y = AWPSHORT(pp1.Y*scale);
        awpDrawCLine(pImage, pp, pp1, c.bRed, c.bGreen, c.bBlue, 0);

        pp  = pp1;
        pp1 = gRightBottom();
        pp1.X = AWPSHORT(pp1.X*scale); pp1.Y = AWPSHORT(pp1.Y*scale);
        awpDrawCLine(pImage, pp, pp1, c.bRed, c.bGreen, c.bBlue, 0);

        pp  = pp1;
        pp1 = gLeftBottom();
        pp1.X = AWPSHORT(pp1.X*scale); pp1.Y = AWPSHORT(pp1.Y*scale);
        awpDrawCLine(pImage, pp, pp1, c.bRed, c.bGreen, c.bBlue, 0);

        pp  = pp1;
        pp1 = gLeftTop();
        pp1.X = AWPSHORT(pp1.X*scale); pp1.Y = AWPSHORT(pp1.Y*scale);
        awpDrawCLine(pImage, pp, pp1, c.bRed, c.bGreen, c.bBlue, 0);
    }
    else
        return;
}

void TROI::DrawAOI(awpImage* pImage, double scale, awpColor c)
{
    awpRect rr;
    rr = GetAOIRect(p,p1, 0.4, 0.7);
    rr =  awpResizeRect(rr, scale);
    int w = (rr.right - rr.left) / 10;
    int h = (rr.bottom - rr.top) / 10;
    awpPoint pp1;
    awpPoint pp2;

    pp1.X = rr.left;
    pp1.Y = rr.top;
    pp2.X = pp1.X + w;
    pp2.Y = pp1.Y;
    awpDrawCLine(pImage, pp1, pp2, c.bRed, c.bGreen, c.bBlue, 0);
    pp2.X = pp1.X ;
    pp2.Y = pp1.Y + h;
    awpDrawCLine(pImage, pp1, pp2, c.bRed, c.bGreen, c.bBlue, 0);


    pp1.X = rr.right;
    pp1.Y = rr.top;
    pp2.X = pp1.X - w;
    pp2.Y = pp1.Y;
    awpDrawCLine(pImage, pp1, pp2, c.bRed, c.bGreen, c.bBlue, 0);
    pp2.X = pp1.X ;
    pp2.Y = pp1.Y + h;
    awpDrawCLine(pImage, pp1, pp2, c.bRed, c.bGreen, c.bBlue, 0);

    pp1.X = rr.right;
    pp1.Y = rr.bottom;
    pp2.X = pp1.X - w;
    pp2.Y = pp1.Y;
    awpDrawCLine(pImage, pp1, pp2, c.bRed, c.bGreen, c.bBlue, 0);
    pp2.X = pp1.X ;
    pp2.Y = pp1.Y - h;
    awpDrawCLine(pImage, pp1, pp2, c.bRed, c.bGreen, c.bBlue, 0);

    pp1.X = rr.left;
    pp1.Y = rr.bottom;
    pp2.X = pp1.X + w;
    pp2.Y = pp1.Y;
    awpDrawCLine(pImage, pp1, pp2, c.bRed, c.bGreen, c.bBlue, 0);
    pp2.X = pp1.X ;
    pp2.Y = pp1.Y - h;
    awpDrawCLine(pImage, pp1, pp2, c.bRed, c.bGreen, c.bBlue, 0);

}

void TROI::DrawAOIRect(awpImage* pImage, double scale, awpColor c)
{
    awpRect rr = GetAOIRect(p,p1,0.4,0.7);
    rr =  awpResizeRect(rr, scale);
    awpDrawCRect(pImage, &rr, c.bRed, c.bGreen, c.bBlue, 0);
}



// функция формирует эталонный прямоугольник лица либо 20*25, либо 40*50 (в

// зависимости от значения переменной big) по указанным точкам глаз pp и pp1
awpImage* TROI::GetRoiImage(awpImage* pImage, bool big) const
{
    awpImage* img = NULL;
    if (awpCheckImage(pImage) != AWP_OK)
        return img;
//    if (!IsROIInImage(pImage))
//        return img;
    int ww = (int)Width();
    int hh = (int)Height();
    int w = 20;
    int h = 24;
    if (big) {w*=2; h*=2;}
    double scale = (double)ww / (double)w;
    if (awpCreateImage(&img, w,h, pImage->bChannels, pImage->dwType) != AWP_OK)
        return img;
    awp2DPoint pp;
    awp2DPoint pp1;
    double dx = p.X -ww / 4.0;
    double dy = p.Y - hh / 4.0;
    double fi = Orientation();
    double sn = sin(fi);
    double cs = cos(fi);
    awpColor* src_c = (awpColor*)pImage->pPixels;
    awpColor* dst_c = (awpColor*)img->pPixels;
    AWPBYTE* src_b = (AWPBYTE*)pImage->pPixels;
    AWPBYTE* dst_b = (AWPBYTE*)img->pPixels;
    int x,y;
    for (int i = 0; i < h; i++)    {
        for (int j = 0; j < w; j++)       {
           pp1.X = j;
           pp1.Y = i;

           pp.X = pp1.X*scale*cs - pp1.Y*scale*sn + dx;
           pp.Y = pp1.Y*scale*cs + pp1.X*scale*sn + dy;
           if (pp.X < 0 || pp.X >= pImage->sSizeX || pp.Y < 0 || pp.Y >= pImage->sSizeY)
                continue;
           x = (int)pp.X;
           y = (int)pp.Y;
           switch(img->bChannels)
           {
                case 1:
                    dst_b[i*w + j] = src_b[y*pImage->sSizeX + x];
                break;
                case 3:
                    dst_c[i*w + j] = src_c[y*pImage->sSizeX + x-1];
                break;
           } // закрывается switch
    }   } // закрываются циклы развертки
     return img;
}

//
awpImage* TROI::GetRoiImageSource(awpImage* pImage)
{
    awpImage* img = NULL;
    if (awpCheckImage(pImage) != AWP_OK)
        return img;
    if (!IsROIInImage(pImage))
        return img;
    int ww = (int)Width();
    int hh = (int)Height();
    int w = ww;
    int h = hh;
    double scale = (double)ww / (double)w;
    if (awpCreateImage(&img, w,h, pImage->bChannels, pImage->dwType) != AWP_OK)
        return img;
    awp2DPoint pp;
    awp2DPoint pp1;
    double dx = p.X -ww / 4.0;
    double dy = p.Y - hh / 4.0;
    double fi = Orientation();
    double sn = sin(fi);
    double cs = cos(fi);
    awpColor* src_c = (awpColor*)pImage->pPixels;
    awpColor* dst_c = (awpColor*)img->pPixels;
    AWPBYTE* src_b = (AWPBYTE*)pImage->pPixels;
    AWPBYTE* dst_b = (AWPBYTE*)img->pPixels;
    int x,y;
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
           pp1.X = j;
           pp1.Y = i;

           pp.X = pp1.X*scale*cs - pp1.Y*scale*sn + dx;
           pp.Y = pp1.Y*scale*cs + pp1.X*scale*sn + dy;
           if (pp.X < 0 || pp.X >= pImage->sSizeX || pp.Y < 0 || pp.Y >= pImage->sSizeY)
                continue;
           x = (int)pp.X;
           y = (int)pp.Y;
           switch(img->bChannels)
           {
                case 1:
                    dst_b[i*w + j] = src_b[y*pImage->sSizeX + x];
                break;
                case 3:
                    dst_c[i*w + j] = src_c[y*pImage->sSizeX + x];
                break;
           }

        }
    }
    return img;
}

// функция размывает изображение
awpImage* TROI::GetRoiLowFrequency(awpImage* pImage)
{
    awpImage* src = NULL;
    awpCopyImage(pImage, &src);
    awpResize(src,20,25);
    if (src == NULL)
        return src;
    int w = 20;
    int h = 25;
    awpResize(src, w, h);
    awpImage* img = NULL;
    if (awpCreateImage(&img, 4, 5, 1, AWP_BYTE) != AWP_OK)
        return img;
    int v = 0;
    awpPoint p;
    AWPBYTE* bi = (AWPBYTE*)src->pPixels;
    AWPBYTE* br = (AWPBYTE*)img->pPixels;
    for (int i = 0; i < 5; i++)    {
        for (int j = 0; j < 4; j++)        {
            p.X = j*5 + 2;
            p.Y = i*5 + 2;
            v = 0;
            for (int k = p.Y - 2; k < p.Y + 2; k++)       {
                for (int m = p.X - 2; m < p.X + 2; m++)        {
                    v += bi[k*src->sSizeX + m];
                }
            }
            v /= 25;
            br[i*img->sSizeX + j] = v;
        }
    }
    awpReleaseImage(&src);
    awpResize(img, pImage->sSizeX, pImage->sSizeY);
    //awpFilter(img, AWP_BLUR_MORE , AWP_FILTER_METHOD_ORDINARY);
    return img;
    }

/*
awpImage* TROI::GetLeftEyeImage(awpImage* pImage)
{
    awpImage* img = GetRoiFeatures(pImage);
    if (img == NULL)
        return NULL;
   awpImage* eye = NULL;
   awpRect r;
   r.left  = 1;
   r.right = 10;
   r.top = 1;
   r.bottom = 11;
   awpCopyRect(img, &eye, &r);
   awpReleaseImage(&img);
   return eye;
}*/

void ImageROIToPoints(awpImage* pImage, TROI* pRoi, awp2DPoint** ppPoints, int* count, int thr)
{
    int i,j;
    awp2DPoint pp;
    awp2DPoint pp_gcs;
    awp2DPoint lcs;
    lcs.X = pRoi->p.X;
    lcs.Y = pRoi->p.Y;

    int w = (int)pRoi->Width();
    int h = (int)pRoi->Height();
    int fi = (int)pRoi->Orientation();
    (*ppPoints) = (awp2DPoint*)malloc(w*h*sizeof(awp2DPoint));
    AWPBYTE* b = (AWPBYTE*)pImage->pPixels;
    (*count) = 0;
    for (i = pRoi->r.top; i < pRoi->r.bottom; i++ )
    {
        for (j = pRoi->r.left; j < pRoi->r.right; j++)
        {
            if (b[i*pImage->sSizeX + j] > thr)
            {
                pp_gcs.X = j; pp_gcs.Y = i;
                pp = GcsToLcsPoint(pp_gcs, fi, lcs);
                if (pRoi->IsPointInLCS(pp))
                {
                 (*ppPoints)[*count] = pp;
                 (*count)++;
                }
            }
        }
    }
}

bool GetROIMoments(awpImage* img, TROI* roi, double& m00, double& m01, double& m10,
double& m20, double& m02, double& m11)
{
  m00 = 0;
  m01 = 0;
  m10 = 0;
  m20 = 0;
  m02 = 0;
  m11 = 0;

  if (roi == NULL)
    return false;
  if (img == NULL)
    return false;
  if (img->dwType != AWP_BYTE)
    return false;
  if (img->bChannels != 1)
    return false;

  awpRect r = roi->r;
  if (awpRectInImage(img, &r) != AWP_OK)
    return false;

 int i,j;
 AWPBYTE* b = (AWPBYTE*)img->pPixels;
 for ( i = r.top; i < r.bottom; i++)
 {
    for (j = r.left; j < r.right; j++)
    {
        m00 += b[i*img->sSizeX + j];
        m01 += b[i*img->sSizeX + j]*i;
        m02 += b[i*img->sSizeX + j]*i*i;
        m10 += b[i*img->sSizeX + j]*j;
        m20 += b[i*img->sSizeX + j]*j*j;
        m11 += b[i*img->sSizeX + j]*i*j;
    }
 }
 return true;
}

// перемещает ROI на dx dy
TROI TranslateROI(TROI* roi, int dx, int dy)
{
    TROI res = *roi;

    res.p.X += dx;
    res.p.Y += dy;
    res.p1.X += dx;
    res.p1.Y += dy;

    res.r.left += dx;
    res.r.top += dy;
    res.r.right += dx;
    res.r.bottom += dy;


    return res;
}
//изменяет размеры ROI на scale
TROI ResizeROI(TROI* roi, double scale)
{
    TROI res = *roi;

    res.p.X = AWPSHORT(scale*res.p.X);
    res.p.Y = AWPSHORT(scale*res.p.Y);
    res.p1.X = AWPSHORT(scale*res.p1.X);
    res.p1.Y = AWPSHORT(scale*res.p1.Y);

    res.r.left = AWPSHORT(scale*res.r.left);
    res.r.top = AWPSHORT(scale*res.r.top);
    res.r.right = AWPSHORT(scale*res.r.right);
    res.r.bottom = AWPSHORT(scale*res.r.bottom);

    return res;
}


// поворачивает roi на заданный угол относительно некоторой точки
TROI RotateROI(TROI* roi, double angle, awpPoint center)
{
    TROI res = *roi;
    double rad_fi = 0;
    rad_fi = angle*3.14159 / 180.;

    res.p.X = AWPSHORT(center.X + (res.p.X - center.X)*cos(rad_fi) - (res.p.Y - center.Y)*sin(rad_fi));
    res.p.Y = AWPSHORT(center.Y + (res.p.X - center.X)*sin(rad_fi) + (res.p.Y - center.Y)*cos(rad_fi));

    res.p1.X = AWPSHORT(center.X + (res.p1.X - center.X)*cos(rad_fi) - (res.p1.Y - center.Y)*sin(rad_fi));
    res.p1.Y = AWPSHORT(center.Y + (res.p1.X - center.X)*sin(rad_fi) + (res.p1.Y - center.Y)*cos(rad_fi));
    return res;
}

//находит ц.м. прямоугольника
bool GetCenterMass(awpImage* pImage, TROI* r, awpPoint* p)
{
   p->X = 0;
   p->Y = 0;
   bool res = true;
   double m00 = 0;
   double m01 = 0;
   double m10 = 0;
   double m20 = 0;
   double m02 = 0;
   double m11 = 0;
   if (!GetROIMoments(pImage, r, m00, m01, m10, m20, m02, m11))
    return false;
   if (m00 == 0)
    return false;
   p->X = AWPSHORT(m10 / m00 + 0.5);
   p->Y = AWPSHORT(m01 / m00 + 0.5);
   return res;
}

//////
bool IsIntersect(TROI* r1, TROI* r2)
{
    if (r1 == NULL)
        return true;
    if (r2 == NULL)
        return true;
    if (IsPointInRect(r1->p, r2->GetROIRectGCS()))
        return true;
    if (IsPointInRect(r1->p1, r2->GetROIRectGCS()))
        return true;
    if (IsPointInRect(r2->p, r1->GetROIRectGCS()))
        return true;
    if (IsPointInRect(r2->p1, r1->GetROIRectGCS()))
        return true;
    awpPoint p = r1->gLeftTop();
    if (IsPointInRect(p, r2->GetROIRectGCS()))
        return true;
    p = r1->gLeftBottom();
    if (IsPointInRect(p, r2->GetROIRectGCS()))
        return true;
    p = r1->gRightTop();
    if (IsPointInRect(p, r2->GetROIRectGCS()))
        return true;
    p = r1->gRightBottom();
    if (IsPointInRect(p, r2->GetROIRectGCS()))
        return true;

    p = r2->gLeftTop();
    if (IsPointInRect(p, r1->GetROIRectGCS()))
        return true;
    p = r2->gLeftBottom();
    if (IsPointInRect(p, r1->GetROIRectGCS()))
        return true;
    p = r2->gRightTop();
    if (IsPointInRect(p, r1->GetROIRectGCS()))
        return true;
    p = r2->gRightBottom();
    if (IsPointInRect(p, r1->GetROIRectGCS()))
        return true;

    return false;
}

/*
            RoiDistanceAvg
    вычисляет расстояние между двумя roi
*/
double RoiDistanceAvg(TROI& roi1, TROI& roi2)
{
   double L1 = roi1.EyeDistance();
    double l1 = sqrt((double)(roi1.p.X - roi2.p.X)*(roi1.p.X - roi2.p.X) + (roi1.p.Y-roi2.p.Y)*(roi1.p.Y - roi2.p.Y));
   double l2 = sqrt((double)(roi1.p1.X - roi2.p1.X)*(roi1.p1.X - roi2.p1.X) + (roi1.p1.Y-roi2.p1.Y)*(roi1.p1.Y - roi2.p1.Y));
   double d1 = 100*l1 / L1;
   double d2 = 100*l2 / L1;
    if (d1 > d2)
        return d1;
    else
        return d2;
}
/*
            RoiDistanceMin
    вычисляет расстояние между двумя roi
*/
double RoiDistanceMin(TROI& roi1, TROI& roi2)
{
   double res = 0;
   double L1 = roi1.EyeDistance();
   double L2 = roi2.EyeDistance();
   double l1 = sqrt((double)(roi1.p.X - roi2.p.X)*(roi1.p.X - roi2.p.X) + (roi1.p.Y-roi2.p.Y)*(roi1.p.Y - roi2.p.Y));
   double l2 = sqrt((double)(roi1.p1.X - roi2.p1.X)*(roi1.p1.X - roi2.p1.X) + (roi1.p1.Y-roi2.p1.Y)*(roi1.p1.Y - roi2.p1.Y));
   double minL = L1<L2?L1:L2;
   res= 50*(l1+l2)/minL;
   return res;
}

/*
            RoiDistanceMax
    вычисляет расстояние между двумя roi
*/
double RoiDistanceMax(TROI& roi1, TROI& roi2)
{
   double res = 0;
   double L1 = roi1.EyeDistance();
   double L2 = roi2.EyeDistance();
   double l1 = sqrt((double)(roi1.p.X - roi2.p.X)*(roi1.p.X - roi2.p.X) + (roi1.p.Y-roi2.p.Y)*(roi1.p.Y - roi2.p.Y));
   double l2 = sqrt((double)(roi1.p1.X - roi2.p1.X)*(roi1.p1.X - roi2.p1.X) + (roi1.p1.Y-roi2.p1.Y)*(roi1.p1.Y - roi2.p1.Y));
   double maxL = L1>L2?L1:L2;
   res= 50*(l1+l2)/maxL;
   return res;
}
/*
            RoiDistance1
    вычисляет расстояние между двумя roi
*/
double RoiDistance1(TROI& roi1, TROI& roi2)
{
   double res = 0;
   double L1 = roi1.EyeDistance();
   double l1 = sqrt((double)(roi1.p.X - roi2.p.X)*(roi1.p.X - roi2.p.X) + (roi1.p.Y-roi2.p.Y)*(roi1.p.Y - roi2.p.Y));
   double l2 = sqrt((double)(roi1.p1.X - roi2.p1.X)*(roi1.p1.X - roi2.p1.X) + (roi1.p1.Y-roi2.p1.Y)*(roi1.p1.Y - roi2.p1.Y));
   res= 100*(l1+l2)/L1;
   return res;
}
/*
            RoiDistance2
    вычисляет расстояние между двумя roi
*/
double RoiDistance2(TROI& roi1, TROI& roi2)
{
   double res = 0;
   double L2 = roi2.EyeDistance();
   double l1 = sqrt((double)(roi1.p.X - roi2.p.X)*(roi1.p.X - roi2.p.X) + (roi1.p.Y-roi2.p.Y)*(roi1.p.Y - roi2.p.Y));
   double l2 = sqrt((double)(roi1.p1.X - roi2.p1.X)*(roi1.p1.X - roi2.p1.X) + (roi1.p1.Y-roi2.p1.Y)*(roi1.p1.Y - roi2.p1.Y));
   res= 50*(l1+l2)/L2;
   return res;
}

/*
        Points2Rois
    преобразует массив точек в массив roi
*/
int Points2Rois(awpPoint* pPoints, int Num, TROI** ppRois, AWPWORD MinDistance, AWPWORD MaxDistance, double Angle)
{
    if (pPoints == NULL || Num <= 0 || ppRois == NULL || MinDistance > MaxDistance)
        return 0;
    *ppRois = NULL;
    int i = 0, j= 0, count = 0;
    awpPoint p,p1;
    int w;
    for ( i = 0; i < Num; i++)
    {
        p = pPoints[i];
        for (j = i; j < Num; j++)
        {
            p1 = pPoints[j];
            w = p1.X - p.X;
            if (w > MinDistance && w < MaxDistance)
            {
                TROI roi;//(p, p1);
                roi.TROI(p, p1);
                if( roi.r.left < 0 || roi.r.top < 0 || roi.r.right < 0 || roi.r.bottom < 0 )
                    continue;
                double angle = fabs(roi.Orientation());
                if (angle < Angle)
                {
                   count++;
                   *ppRois = (TROI*)realloc(*ppRois, count*sizeof(TROI));
                   (*ppRois)[count-1].TROI(roi.p, roi.p1);
                }
            }
        }
    }
    return count;
}
/*
        ClusterRois
    собрает близкие roi в кластеры
*/
int ClusterRois(TROI* pRois, int Num, TROI** ppClusters)
{
  if (pRois == NULL || Num <= 0 || ppClusters == NULL)
    return 0;
  int i = 0, j = 0, count = 0;
  double r = 0;
  for (i = 0; i < Num; i++)
    pRois[i].index = -1;

  for (i = 0; i < Num; i++)
  {
    if (pRois[i].index >= 0)
        continue;
    pRois[i].index = count;
    for (j = i; j < Num; j++)
    {
        if (pRois[j].index >= 0)
            continue;
        r = RoiDistanceAvg(pRois[i], pRois[j]);
        if (r < 25)
            pRois[j].index = count;
    }
    count++;
  }
  *ppClusters = (TROI*)malloc(count*sizeof(TROI));
  memset(*ppClusters, 0, count*sizeof(TROI));
  for (i = 0; i < Num; i++)
  {
    (*ppClusters)[pRois[i].index].index++;
    (*ppClusters)[pRois[i].index].p.X += pRois[i].p.X;
    (*ppClusters)[pRois[i].index].p.Y += pRois[i].p.Y;
    (*ppClusters)[pRois[i].index].p1.X += pRois[i].p1.X;
    (*ppClusters)[pRois[i].index].p1.Y += pRois[i].p1.Y;
  }
  for (i = 0; i < count; i++)
  {
     if ((*ppClusters)[i].index == 0)
        continue;

    (*ppClusters)[i].p.X /= (*ppClusters)[i].index;
    (*ppClusters)[i].p.Y /= (*ppClusters)[i].index;
    (*ppClusters)[i].p1.X /= (*ppClusters)[i].index;
    (*ppClusters)[i].p1.Y /= (*ppClusters)[i].index;
    (*ppClusters)[i].TROI((*ppClusters)[i].p, (*ppClusters)[i].p1);
  }
  return count;
}

bool AoiRect2Roi(awpRect r, TROI& roi, double c1, double c2)
{
    bool res = false;
    if (r.left < 0 || r.top < 0 || r.right < 0 || r.bottom < 0)
        return res;
    if (awpCheckRect(&r) != AWP_OK)
        return res;
    int w = r.right - r.left;
//    if (w != h)
//        return res;
    res = true;
    awpPoint p1; // левый глаз
    awpPoint p2; // правый глаз

    /**/
    p1.X = r.left +  (unsigned short)(w*c2*c1);
    p2.X = p1.X + (unsigned short)(w*c1);

    int L = p2.X - p1.X;
    p1.Y = r.top  +  (unsigned short)(L *c2);
    p2.Y = p1.Y;

    roi.TROI(p1, p2);
    return res;
}

////////////////////////////////////////////////////////////////////////////////
TLFRoi::TLFRoi()
{
   memset(&m_roi, 0, sizeof(TROI));
}

TROI TLFRoi::GetRoi()
{
    return m_roi;
}

void TLFRoi::SetRoi(awpPoint lp, awpPoint rp)
{
   m_roi.TROI(lp,rp);
}

void TLFRoi::SetRoi(TROI& roi)
{
    memcpy(&m_roi, &roi, sizeof(TROI));
}
//---------------------------------------------------------------------------
void TLFRoi::SetRoi(const TROI* pRoi)
{
    if (pRoi != NULL)
        memcpy(&m_roi, pRoi, sizeof(TROI));
}
//---------------------------------------------------------------------------
void TLFRoi::Shift(int dx, int dy)
{
   m_roi.p.X += dx;
   m_roi.p.Y += dy;
   m_roi.p1.X += dx;
   m_roi.p1.Y += dy;
   m_roi.TROI(m_roi.p, m_roi.p1);
}
//---------------------------------------------------------------------------
// 0 < f < Fmax = 10
void TLFRoi::Scale(double f)
{
    if (f <= 0 || f > 10)
        return;
    awpPoint center; // this is a center of LOC (Local Coordinate System)
    center.X = (m_roi.p.X + m_roi.p1.X) / 2;
    center.Y = (m_roi.p.Y + m_roi.p1.Y) / 2;

    // search out p1 and p2 in the LOC
    awpPoint p, p1;
    p.X = m_roi.p.X - center.X;
    p.Y = m_roi.p.Y - center.Y;

    p1.X = m_roi.p1.X - center.X;
    p1.Y = m_roi.p1.Y - center.Y;

    // scale points
    p.X = AWPSHORT(f*p.X);
    p.Y = AWPSHORT(f*p.Y);
    p1.X =AWPSHORT(f*p1.X);
    p1.Y =AWPSHORT(f*p1.Y);

    m_roi.p.X = p.X + center.X;
    m_roi.p.Y = p.Y + center.Y;

    m_roi.p1.X = p1.X + center.X;
    m_roi.p1.Y = p1.Y + center.Y;

   m_roi.TROI(m_roi.p, m_roi.p1);

}




awpImage* TLFRoi::GetRoiImage(awpImage* pImageIn, int Lxs, int Lys, awpPoint P1s, awpPoint P2s)
{
    AWPBYTE* ImIn=NULL; AWPBYTE* ImOut=NULL;
    awpImage* pImageOut = NULL;
    int Lx=pImageIn->sSizeX,Ly=pImageIn->sSizeY,BACKGROUND=128,xint,yint;
    int x1=m_roi.p.X, y1=m_roi.p.Y, x2=m_roi.p1.X, y2=m_roi.p1.Y;
    int X1=P1s.X,Y1=P1s.Y,X2=P2s.X,Y2=P2s.Y;
    double R,r,A0,B0,sin_a,cos_a,x,y,dx,dy;
    r=sqrt((double)(x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
    R=sqrt((double)(X2-X1)*(X2-X1)+(Y2-Y1)*(Y2-Y1));
    if(R==0||r==0) return NULL;
    if(pImageIn->bChannels!=1)
        awpConvert(pImageIn, AWP_CONVERT_3TO1_BYTE);
    awpCreateImage(&pImageOut,Lxs,Lys,1,AWP_BYTE);
    ImIn=(AWPBYTE*)pImageIn->pPixels;
    ImOut=(AWPBYTE*)pImageOut->pPixels;
    sin_a=((X2-X1)*(y2-y1)-(Y2-Y1)*(x2-x1))/R/r;
    cos_a=((X2-X1)*(x2-x1)+(Y2-Y1)*(y2-y1))/R/r;
    A0=x1-X1*r/R*cos_a+Y1*r/R*sin_a;
    B0=y1-X1*r/R*sin_a-Y1*r/R*cos_a;
    for(int in=0;in<Lys;in++)
    {
        for(int jn=0;jn<Lxs;jn++)
        {
	        x=A0+jn*r/R*cos_a-in*r/R*sin_a;
	        y=B0+jn*r/R*sin_a+in*r/R*cos_a;
	        xint=(int)x; yint=(int)y; dx=x-xint; dy=y-yint;
	        if(xint>0&&xint<Lx-2&&yint>0&&yint<Ly-2)
	            ImOut[Lxs*in+jn]=(int)(ImIn[pImageIn->sSizeX*yint+xint]*(1-dx)*(1-dy))+(int)(ImIn[pImageIn->sSizeX*yint+(xint+1)]*dx*(1-dy))+(int)(ImIn[pImageIn->sSizeX*(yint+1)+xint]*(1-dx)*dy)+(int)(ImIn[pImageIn->sSizeX*(yint+1)+(xint+1)]*dx*dy);
            else
                ImOut[Lxs*in+jn]=BACKGROUND;
        }
    }
    return pImageOut;
}

//---------------------------------------------------------------------------

/*
   DONE: перенести эту функцию в специальный файл
   геометрических функций
   возвращает высоту прямоугольника
*/
awp2DPoint awpGetPointsCentroid(awp2DPoint* p, int count)
{
    awp2DPoint res;
    res.X = 0;
    res.Y = 0;
    if (count <= 0)
        return res;
    for (int i = 0; i < count; i++)
    {
        res.X += p[i].X;
        res.Y += p[i].Y;
    }
    res.X /= count;
    res.Y /= count;
    return res;
}
awpPoint  awpGetPointsCentroid(awpPoint* p, int count)
{
    awpPoint res;
    res.X = 0;
    res.Y = 0;
    if (count <= 0)
        return res;
    for (int i = 0; i < count; i++)
    {
        res.X += p[i].X;
        res.Y += p[i].Y;
    }
    res.X /= count;
    res.Y /= count;
    return res;
}


/*
   DONE: перенести эту функцию в специальный файл
   геометрических функций
   возвращает высоту прямоугольника
*/


/*
   Возвращает левый верхний угол прямоугольника
*/
awpPoint awpGetLeftTopRectCorner(awpRect r)
{
    awpPoint result;
    result.X = r.left;
    result.Y = r.top;
    return result;
}

awpPoint awpGetRightBottomRectCorner(awpRect r)
{
    awpPoint result;
    result.X = r.right;
    result.Y = r.bottom;
    return result;
}

/*
   DONE: перенести эту функцию в специальный файл
   геометрических функций
   выполняет параллельный перенос прямоугольника в точку p
*/
awpRect awpTranslateRect(awpRect r, awpPoint p)
{
    r.left += p.X;
    r.top  += p.Y;
    r.right += p.X;
    r.bottom += p.Y;
    return r;
}
/*
   DONE: перенести эту функцию в специальный файл
   геометрических функций
   выполняет параллельный перенос прямоугольника в точку p
*/
awpRect awpTranslateRect(awpRect r, awp2DPoint p)
{
    r.left = AWPSHORT(p.X + r.left);
    r.top  = AWPSHORT(p.Y + r.top);
    r.right = AWPSHORT(p.X + r.right);
    r.bottom = AWPSHORT(p.Y + r.bottom);
    return r;
}
/*
   DONE: перенести эту функцию в специальный файл
   геометрических функций
   переносит точку на dx, dy
*/
awpPoint awpTranslatePoint (awpPoint p, int dx, int dy)
{
    awpPoint result;
    result.X = p.X + dx;
    result.Y = p.Y + dy;
    return result;
}
/*
   DONE: перенести эту функцию в специальный файл
   геометрических функций
   переносит точку на dx, dy
*/
awpPoint awpTranslatePoint (awp2DPoint p, int dx, int dy)
{
    awpPoint result;
	result.X = AWPSHORT(p.X + dx);
    result.Y = AWPSHORT(p.Y + dy);
    return result;
}
/*
   DONE: перенести эту функцию в специальный файл
   геометрических функций
   возвращает щирину прямоугольника
*/
int awpRectWidth(awpRect r)
{
    return r.right - r.left;
}
/*
   DONE: перенести эту функцию в специальный файл
   геометрических функций
   возвращает высоту прямоугольника
*/
int awpRectHeight(awpRect r)
{
    return r.bottom - r.top;
}

awpRect  awpCreateRect(awpPoint lt, awpPoint rb)
{
    awpRect r;

    r.left = lt.X;
    r.top  = lt.Y;
    r.right = rb.X;
    r.bottom = rb.Y;
    return r;
}

awpRect awpResizeRect(awpRect r, double scale)
{
    r.left = AWPSHORT(scale*r.left);
    r.top  = AWPSHORT(scale*r.top);
    r.right = AWPSHORT(scale*r.right);
    r.bottom = AWPSHORT(scale*r.bottom);
    return r;
}


// проверяет принадлежность "целой" точки прямоугольнику
AWPBOOL IsPointInRect(awpPoint p, awpRect r)
{
	if (p.X >= r.left && p.X <= r.right && p.Y >= r.top && p.Y <= r.bottom)
		return TRUE;
	else
		return FALSE;
}
//находит центр прямоугольника
awpPoint RectCenter (awpRect r)
{
    awpPoint p;
    p.X = (r.left + r.right) /2;
    p.Y = (r.top + r.bottom) /2;
    return p;
}

// получение исходных данных AOI
// преобразованных к размеру AOI_W и AOI_H
awpImage* GetAOISource(awpImage* pImage, awpPoint p1, awpPoint p2)
{
    if (pImage == NULL)
        return NULL;
    awpRect aoiRect = GetAOIRect(p1, p2, 0.4, 0.7);
    int w = AOI_W; // размеры aoi
    int h = AOI_H;
    awpImage* aoi_img = NULL;
    if (awpCreateImage(&aoi_img, w,h, 1, AWP_BYTE) != AWP_OK)
        return NULL;
    int x,y;
    double scale = (double)(aoiRect.right - aoiRect.left) / (double)w;
    int dx = aoiRect.left;
    int dy = aoiRect.top;
    awp2DPoint pp;
    awp2DPoint pp1;
    TROI roi; roi.TROI(p1,p2);
    double fi = roi.Orientation();
    double sn = sin(fi);
    double cs = cos(fi);
    AWPBYTE* src_b = (AWPBYTE*)pImage->pPixels;
    AWPBYTE* dst_b = (AWPBYTE*)aoi_img->pPixels;
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
           pp1.X = j;
           pp1.Y = i;

           pp.X = pp1.X*scale*cs - pp1.Y*scale*sn + dx;
           pp.Y = pp1.Y*scale*cs + pp1.X*scale*sn + dy;
           if (pp.X < 0 || pp.X >= pImage->sSizeX || pp.Y < 0 || pp.Y >= pImage->sSizeY)
           {
                awpReleaseImage(&aoi_img);
                return NULL;
           }
           x = (int)pp.X;
           y = (int)pp.Y;
           dst_b[i*w + j] = src_b[y*pImage->sSizeX + x];
        }
    }
    return  aoi_img;
}

// получение исходных данных AOI
// преобразованных к размеру AOI_W и AOI_H
awpImage* GetAOIImage(awpImage* pImage, awpPoint p1, awpPoint p2)
{
    if (pImage == NULL)
        return NULL;
    awpRect aoiRect = GetAOIRect(p1, p2, 0.4, 0.7);
    int w = aoiRect.right - aoiRect.left; // размеры aoi
    int h = aoiRect.bottom - aoiRect.top;
    awpImage* aoi_img = NULL;
    if (awpCreateImage(&aoi_img, w,h, 1, AWP_BYTE) != AWP_OK)
        return NULL;
    int x,y;
    double scale = (double)(aoiRect.right - aoiRect.left) / (double)w;
    int dx = aoiRect.left;
    int dy = aoiRect.top;
    awp2DPoint pp;
    awp2DPoint pp1;
    TROI roi; roi.TROI(p1,p2);
    double fi = roi.Orientation();
    double sn = sin(fi);
    double cs = cos(fi);
    AWPBYTE* src_b = (AWPBYTE*)pImage->pPixels;
    AWPBYTE* dst_b = (AWPBYTE*)aoi_img->pPixels;
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
           pp1.X = j;
           pp1.Y = i;

           pp.X = pp1.X*scale*cs - pp1.Y*scale*sn + dx;
           pp.Y = pp1.Y*scale*cs + pp1.X*scale*sn + dy;
           if (pp.X < 0 || pp.X >= pImage->sSizeX || pp.Y < 0 || pp.Y >= pImage->sSizeY)
           {
                awpReleaseImage(&aoi_img);
                return NULL;
           }
           x = (int)pp.X;
           y = (int)pp.Y;
           dst_b[i*w + j] = src_b[y*pImage->sSizeX + x];
        }
    }
    return  aoi_img;
}

static awpPoint _awpGetCenterMass(awpImage* img, awpRect* r)
{
    awpPoint res;
    res.X = (r->left + r->right) / 2;
    res.Y = (r->top + r->bottom) / 2;
    AWPBYTE* b = (AWPBYTE*)img->pPixels;
    double sum, x0, y0;
    sum = 0;
    x0 = 0;
    y0 = 0;

    for (int i = r->top; i < r->bottom; i++)
    {
        for (int j = r->left; j < r->right; j++)
        {
            sum+= b[i*img->sSizeX + j];
            x0 += j*b[i*img->sSizeX + j];
            y0 += i*b[i*img->sSizeX + j];
        }
    }
    res.X = (AWPSHORT)floor(x0 / (double)sum + 0.5);
	res.Y = (AWPSHORT)floor(y0 / (double)sum + 0.5);
    return res;
}
// depricated
static awpPoint _awpGetCenterMass1(awpImage* imgh, awpImage* imgv, awpRect* r)
{
    awpPoint res;
    res.X = (r->left + r->right) / 2;
    res.Y = (r->top + r->bottom) / 2;
    AWPDOUBLE* v = (AWPDOUBLE*)imgv->pPixels;
    AWPDOUBLE* h = (AWPDOUBLE*)imgh->pPixels;
    double sum, x0, y0;
    sum = 0;
    x0 = 0;
    y0 = 0;

    for (int i = r->top; i < r->bottom; i++)
    {
        for (int j = r->left; j < r->right; j++)
        {
           // if (/*v[i*imgh->sSizeX + j] > 0 && */h[i*imgh->sSizeX + j] < 0)
            {
              double g = sqrt(v[i*imgh->sSizeX + j]*v[i*imgh->sSizeX + j] + h[i*imgh->sSizeX + j]*h[i*imgh->sSizeX + j]);
              sum += g;
              x0 += j*g;//*fabs(v[i*imgh->sSizeX + j]) + fabs(h[i*imgh->sSizeX + j]);
              y0 += i*g;//*fabs(v[i*imgh->sSizeX + j]) + fabs(h[i*imgh->sSizeX + j]);
            }
        }
    }
    if (sum > 0 )
    {
		res.X = (AWPSHORT)floor(x0 / (double)sum + 0.5);
		res.Y = (AWPSHORT)floor(y0 / (double)sum + 0.5);
    }
    return res;
}

static awpPoint _awpGetCenterMax(awpImage* imgh, awpImage* imgv, awpRect* r)
{
    awpPoint res;
    res.X = (r->left + r->right) / 2;
    res.Y = (r->top + r->bottom) / 2;
    AWPDOUBLE* v = (AWPDOUBLE*)imgv->pPixels;
    AWPDOUBLE* h = (AWPDOUBLE*)imgh->pPixels;
    double max = -100;
    double y0 = 0;

    for (int i = r->top; i < r->bottom; i++)
    {
        for (int j = r->left; j < r->right; j++)
        {
            if (max < sqrt(v[i*imgh->sSizeX + j]*v[i*imgh->sSizeX + j] + h[i*imgh->sSizeX + j]*h[i*imgh->sSizeX + j]))
            {
                max  =  sqrt(v[i*imgh->sSizeX + j]*v[i*imgh->sSizeX + j] + h[i*imgh->sSizeX + j]*h[i*imgh->sSizeX + j]);;
                y0 = i;
            }
        }
    }

	res.Y = (AWPSHORT)floor(y0 + 0.5);
    return res;
}



#ifdef __BCPLUSPLUS__
    #pragma package(smart_init)
#endif
