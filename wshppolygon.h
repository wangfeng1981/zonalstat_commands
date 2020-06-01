/// fix bug when shp is bigger than raster. 2020-6-1
#ifndef WSHPPOLYGON_H
#define WSHPPOLYGON_H

#include "shapefil.h"
#include <vector>
#include <algorithm>
#include <string>
#include <time.h>



using std::string;
using std::vector;

struct WHoriLineSeg {
	int x0, y, x1;
};



struct WShpLineSeg {
	inline WShpLineSeg(double tx0,double ty0,double tx1,double ty1):
		x0(tx0),y0(ty0),x1(tx1),y1(ty1){} ;
	inline WShpLineSeg():
		x0(0),y0(0),x1(0),y1(0){} ;
	double x0, y0;
	double x1, y1;
};

struct WShpPolygon {

	///generate lines from shpfile.
	///@param filepath input xxx.shp
	///@param lines return lines
	static bool readLinesFromShapefile(const char* filepath , vector<WShpLineSeg>& lines);


	///@param lines input in proj coordinates
	///@param segs return hori-segments in image coordinates
	///@param topY input top latitude
	///@param topY input bottom latitude
	///@param resoY input y resolution (generally negative value.)
	static void convertLines2Segments(vector<WShpLineSeg>& lines, vector<WHoriLineSeg>& segs,
		const double topY,const int imageHeight,const double resoY,
		const double leftX,const int imageWidth,const double resoX);


	/// this is used for partial anaylyse , so need leftTop offset postiiton in image pixel coordinates.
	///@param dataBuffer input image data
	///@param partOffsetX left offset x
	///@param partOffsetY top offset y
	///@param partWidth width of this part
	///@param partHeight height of this part
	///@param segs horizonal segments
	///@param minvalid min valid value included
	///@param maxvalid max valid value included
	///@param retMin return valid min in image
	///@param retMax return valid max in image
	///@param retSum return valid sum in image
	///@param retCnt return valid count in image
	// template <typename T>
	// static void computeStatistic(T* dataBuffer,
	// 	const int partOffsetX, const int partOffsetY, const int partWidth, const int partHeight,
	// 	vector<WHoriLineSeg>& segs,
	// 	const double minvalid, const double maxvalid,
	// 	double& retMin, double& retMax, double& retSum, double& retCnt);

	///generate timestampid
	static string generateTimestampId(string prefix);

};


string WShpPolygon::generateTimestampId(string prefix  )
{
	time_t ts1 = time(NULL);
	char tempbuff[32];
	sprintf(tempbuff, "%ld", ts1);
	return prefix + string(tempbuff) ;
}

bool WShpPolygon::readLinesFromShapefile(const char* filepath, vector<WShpLineSeg>& lines) {

	SHPHandle handle = SHPOpen( filepath , "rb");
	int entities = 0;
	int shpType = 0;
	double minbound[20];
	double maxbound[20];
	SHPGetInfo(handle, &entities, &shpType,
		minbound, maxbound);
	if (entities == 0) {
		printf("error : WShpPolygon, entities is 0.\n");
		SHPClose(handle);
		return false;
	}
	
	if (shpType != SHPT_POLYGON ) {
		printf("error : WShpPolygon, shpType is not SHPT_POLYGON.\n");
		SHPClose(handle);
		return false;
	}

	//printf("debug entities: %d \n", entities);
	for (int i = 0; i < entities; ++i) {
		SHPObject * shp1 = SHPReadObject(handle, i);
		if (shp1 != 0) {
			//printf("debug id:%d  numParts:%d  numVert:%d\n",shp1->nShapeId, shp1->nParts, shp1->nVertices);
			//只考虑一个part 复杂多边形不考虑
			if (shp1->nParts == 1 && shp1->nVertices > 2 ) {
				for (int iv = shp1->panPartStart[0]; iv < shp1->nVertices - 1 ; ++iv) {
					lines.push_back(WShpLineSeg(
						shp1->padfX[iv] , 
						shp1->padfY[iv] ,
						shp1->padfX[iv+1] , 
						shp1->padfY[iv+1] 
					));
				}
			}
			SHPDestroyObject(shp1);
		}
	}
	SHPClose(handle);
	return true;
}


bool horiLineInterLineSeg(const double horiY ,
	const double x0,//line in polygon
	const double y0,
	const double x1,
	const double y1, 
	int & resX ) // cross point to pixel x in image
{
	if ( y0  != y1 )
	{
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




///@param lines input in proj coordinates
///@param segs output in image coordinates
///@param topY input top latitude
///@param topY input bottom latitude
///@param resoY input y resolution (generally negative value.)
void WShpPolygon::convertLines2Segments(vector<WShpLineSeg>& lines, 
	vector<WHoriLineSeg>& segs , 
	const double topY , 
	const int imageHeight, 
	const double resoY ,
	const double leftX , 
	const int imageWidth ,
	const double resoX ) {

	vector<int> crossPointXVector;
	for (int iy = 0; iy < imageHeight; ++iy) {
		crossPointXVector.clear();
		double horiY = topY + resoY * iy;
		for (int il = 0; il < lines.size(); ++il) {
			int resX = -1;
			if ( horiLineInterLineSeg(horiY,	lines[il].x0, lines[il].y0,	lines[il].x1, lines[il].y1,	resX) ) {
				crossPointXVector.push_back(resX);
			}
		}
		if (crossPointXVector.size() > 0) {
			if (crossPointXVector.size() % 2 == 0) {
				//sort by x
				std::sort(crossPointXVector.begin(), crossPointXVector.end());
				for (int ix = 0; ix < crossPointXVector.size(); ix += 2) {
					int tx0 = (crossPointXVector[ix] - leftX) / resoX;
					int tx1 = (crossPointXVector[ix + 1] - leftX) / resoX;
					if (tx0 >= imageWidth || tx1 < 0) {
						//outside
					}
					else {
						WHoriLineSeg hseg;
						hseg.y = iy;
						hseg.x0 = std::max(  0, tx0 );
						hseg.x1 = std::min( imageWidth-1, tx1 ) ;
						segs.push_back(hseg);
					}
				}
			}
			else {
				
				printf("exception: WShpPolygon::convertLines2Segments has a odd cross point.\n");
			}
		}
	}
}

// template <typename T>
// void WShpPolygon::computeStatistic(T* dataBuffer,
// 	const int partOffsetX, const int partOffsetY, const int partWidth, const int partHeight,
// 	vector<WHoriLineSeg>& segs,
// 	const double minvalid, const double maxvalid,
// 	double& retMin, double& retMax, double& retSum, double& retCnt)
// {
// 	int partBottom = partOffsetY + partHeight;
// 	int partRight = partOffsetX + partWidth;
// 	for (int iseg = 0; iseg < segs.size(); ++iseg) {
// 		WHoriLineSeg& tseg = segs[iseg];
// 		if (tseg.y >= partOffsetY && tseg.y < partBottom) {
// 			// y inside part
// 			if (tseg.x0 >= partRight || tseg.x1 < partOffsetX) {
// 				// seg x outside do nothing.
// 			}
// 			else {
// 				// x inside part
// 				int txleft = max(tseg.x0, partOffsetX);
// 				int txright = min(tseg.x1, partRight - 1);
// 				int yInPart = tseg.y - partOffsetY;
// 				for (int fullx = txleft; fullx <= txright; ++fullx) {
// 					int xInPart = fullx - partOffsetX;
// 					int itInData = yInPart * partWidth + xInPart;
// 					T pxval = dataBuffer[itInData];
// 					if ( pxval >= minvalid && pxval <= maxvalid) {
// 						//valid pixel
// 						++retCnt;
// 						retSum += pxval;
// 						if (pxval > retMax) retMax = pxval;
// 						if (pxval < retMin) retMin = pxval;
// 					}
// 				}
// 			}
// 		}
// 	}
// }


#endif

