/// Header Only  将任意线段数组转换成标准等经纬图像坐标的水平线段数组
/// convert lines to HSEG
/// use
/// #define W_LEVEL_HSEG_IMPLEMENT
#ifndef W_LEVEL_HSEG_H
#define W_LEVEL_HSEG_H
#include <cmath>
#include <iostream>
#include <vector>
#include "whsegline.h"
#include <algorithm>


using std::cout;
using std::endl;
using std::vector;

struct wLevelHseg {
	unsigned char ilevel ;
	vector<WHseg> hsegs ;
	inline wLevelHseg():ilevel(0){}

	///初始化
	void init(
		vector<WLineSeg>& lines, 
		double topY , //矢量文件longlat坐标yMax值
		double bottomY , //矢量文件longlat坐标yMin值
		unsigned char lvl //层级，从0开始
		) ;

private:
	/// 水平线如果与线段相交返回true反之返回false
	bool horiLineInterLineSeg(double horiY ,
								const double x0,//line in polygon
								const double y0,
								const double x1,
								const double y1,
								double & resX) ; 

	
} ;


#endif


#ifdef W_LEVEL_HSEG_IMPLEMENT


bool wLevelHseg::horiLineInterLineSeg(double horiY ,
	const double x0,//line in polygon
	const double y0,
	const double x1,
	const double y1, 
	double & resX ) // cross point to pixel x in image //bugfixed 2020-6-18
{
	if ( y0  != y1 )
	{
		if(y0==horiY || y1==horiY){
			horiY += 0.00000001;
		}
		if ( x0 == x1 )
		{
			if ((horiY - y0)*(horiY - y1) <= 0)
			{
				resX = x0;
				return true;
			}
		}
		else
		{
			double k = (y0 - y1) / (x0 - x1);
			double b = y0 - k * x0;
			if ((horiY - y0)*(horiY - y1) <= 0)
			{
				resX = (horiY - b) / k;
				return true;
			}
		}
	}
	return false;
}





void wLevelHseg::init(
		vector<WLineSeg>& lines, 
		double topY , //矢量文件longlat坐标yMax值
		double bottomY , //矢量文件longlat坐标yMin值
		unsigned char lvl //层级，从0开始
		) 
{
	//最大层级20层
	if( lvl > 20 ) {
		cout<<"level should not greater than 20."<<endl;
		return ;
	} 
	this->ilevel = lvl ;
	this->hsegs.clear() ;
	this->hsegs.reserve(1024) ;
	int nxtile = 1;
	for(int il = 0 ;il < lvl;++il ) nxtile*=2 ;
	const double reso = (360.0 / nxtile)/256.0 ; 
	//cout<<"reso:"<<reso<<endl ;

	//vector<int> crossPointXVector;//bugfixed 2020-6-18
	vector<double> crossPointXVector;crossPointXVector.reserve(8); //每个水平线的全部交点
	if( topY > 90.0 ) topY = 90.0 ;
	if( bottomY < -90.0 ) bottomY = -90.0;
	if( topY < bottomY ) {
		cout<<"top-y coordinate should not lower than bottom-y."<<endl;
		return ;//top坐标小于bottom坐标退出
	}

	const int fullHeight = 180.0 / reso ;
	const int fullWidth  = 360.0 / reso ;

	int topIY = (90.0 - topY)/reso ;
	int bottomIY = (90.0 - bottomY)/reso ;
	if( bottomIY >= fullHeight )  bottomIY = fullHeight-1 ;
	double halfReso = reso/2.0 ;

	for (int iy = topIY; iy <= bottomIY; ++iy) {
		crossPointXVector.clear();
		double horiY = 90.0 - reso * iy - halfReso;

		for (int il = 0; il < lines.size(); ++il) {
			double crossX = -1;//use double replace int 2020-6-18
			if ( horiLineInterLineSeg(
					horiY,
					lines[il].x0,lines[il].y0,
					lines[il].x1,lines[il].y1,
					crossX) 
				) 
			{
				crossPointXVector.push_back(crossX);
			}
		}
		//cout<<"horiY,ncross:"<<horiY<<","<<crossPointXVector.size() <<endl ;
		if (crossPointXVector.size() > 0) {
			if (crossPointXVector.size() % 2 == 0) {
				//水平线的交点一定是偶数
				//sort by x
				std::sort(crossPointXVector.begin(), crossPointXVector.end());
				for (int ix = 0; ix < crossPointXVector.size(); ix += 2) {
					int tx0 = (crossPointXVector[ix]     + 180.0) / reso;
					int tx1 = (crossPointXVector[ix + 1] + 180.0) / reso;
					if (tx0 >= fullWidth || tx1 < 0) {
						//outside
					}
					else {
						WHseg hseg;
						hseg.y = iy;
						hseg.x0 = std::max(  0, tx0 );
						hseg.x1 = std::min( fullWidth-1, tx1 ) ;
						hsegs.push_back(hseg);
					}
				}
			}
			else {
				//水平线的交点一定是偶数
				printf("exception: WShpPolygon::convertLines2Segments has a odd cross point.\n");
			}
		}
	}
}


#endif

