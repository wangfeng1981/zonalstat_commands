/// Header Only 使用GDAL库读取shp和geojson并转换为线段数组
/// geojson and ESRI shape reader by GDAL 
/// use 
/// #define W_GEOJSON_AND_SHP_READER_IMPLEMENT 
/// for implement
#ifndef W_GEOJSON_AND_SHP_READER_H
#define W_GEOJSON_AND_SHP_READER_H

#include <vector>
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "whsegline.h"
#include <string>
#include <cmath>

using std::string;
using std::vector;

struct wGeoJsonShpReader {

	///读取geojson或者shp文件，返回线段数组
	static bool readFromFile( string geojsonOrShpFilename,
			vector<WLineSeg>& ret_lines,
			double& ret_miny,double& ret_maxy,
			string& error) ;

private:
	///返回当前polygon的线段数量，注意不是 ret_lines 的数量，ret_lines 是总数量
	static int getPolygonLines(OGRPolygon* poly,
			vector<WLineSeg>& ret_lines,
			double& r_miny,double& r_maxy) ;

} ;


#endif

#ifdef W_GEOJSON_AND_SHP_READER_IMPLEMENT

bool wGeoJsonShpReader::readFromFile( string geojsonOrShpFilename,
			vector<WLineSeg>& ret_lines,
			double& ret_miny,double& ret_maxy,
			string& error) 
{
	GDALAllRegister() ;
	ret_miny =  999999 ;
	ret_maxy = -999999 ;

	// 打开shp或者json文件
	GDALDataset *poDS =(GDALDataset*)GDALOpenEx(geojsonOrShpFilename.c_str(),
		GDAL_OF_VECTOR,NULL, NULL, NULL);
	if( poDS==0 ){
		error = string("Failed to open ")+geojsonOrShpFilename ;
		return false ;
	}
	if( poDS->GetLayerCount()==0 ){
		error = string("Zero layer in ")+geojsonOrShpFilename ;
		GDALClose(poDS) ;
		return false ;
	}
	OGRLayer *poLayer=poLayer = poDS->GetLayer(0);// 这里与栅格不同，索引值从0开始记。
	OGRwkbGeometryType geoType = wkbFlatten(poLayer->GetGeomType()) ;
	if( geoType == wkbPolygon || geoType==wkbMultiPolygon )
	{
		int iFeature = 0;
		OGRFeature *poFeature;
		poLayer->ResetReading();
		while( (poFeature = poLayer->GetNextFeature()) != NULL ){
			//注意GetNextFeature 返回的指针需要调用者释放
			OGRGeometry *geometry = poFeature->GetGeometryRef() ;
			if(  wkbFlatten(geometry->getGeometryType()) == wkbPolygon ) 
			{
				OGRPolygon *poly =(OGRPolygon*) geometry ;
				double ymin,ymax;
				if( getPolygonLines(poly,ret_lines,ymin,ymax)>0 ){
					ret_miny = std::min(ret_miny,ymin);
					ret_maxy = std::max(ret_maxy,ymax);
				}
			}else if( wkbFlatten(geometry->getGeometryType()) == wkbMultiPolygon )
			{
				OGRMultiPolygon *mulpoly=(OGRMultiPolygon*) geometry ;
				int numpoly = mulpoly->getNumGeometries();
				for(int ipoly=0;ipoly<numpoly;++ipoly){
					OGRPolygon* polyRef= (OGRPolygon*)mulpoly->getGeometryRef( ipoly );	
					double ymin,ymax;
					if( getPolygonLines(polyRef,ret_lines,ymin,ymax) >0 ){	
						ret_miny = std::min(ret_miny,ymin);
						ret_maxy = std::max(ret_maxy,ymax);
					}
				}
			}
			OGRFeature::DestroyFeature(poFeature) ;
		}
		GDALClose(poDS) ;
		return true ;
	}else{
		GDALClose(poDS) ;
		error = "GeoType not polygon or mulPolygon." ;
		return false ;
	}
}


int wGeoJsonShpReader::getPolygonLines(OGRPolygon* poly,
										vector<WLineSeg>& ret_lines,
										double& r_miny,double& r_maxy)
{
	///一个polygon包含0个或1个外环，包含0个或者多个内环
	r_miny = 999999.0;
	r_maxy =-999999.0;
	int n = 0 ;
	//外环
	{
		const OGRLinearRing * lineRing = poly->getExteriorRing() ;
		if( lineRing==NULL ){
			return 0 ;
		}
		OGRLineString* lineString = (OGRLineString*)lineRing;
		OGRSimpleCurve* simCurve = (OGRSimpleCurve*)lineString;
		int numPts = simCurve->getNumPoints() ;
		if( numPts>3 ){
			for(int i = 0 ; i<numPts-1 ; ++i)
			{
				double x0 = simCurve->getX(i) ;
				double y0 = simCurve->getY(i) ;
				double x1 = simCurve->getX(i+1) ;
				double y1 = simCurve->getY(i+1) ;
				ret_lines.push_back( WLineSeg(x0,y0,x1,y1) ) ;
				r_miny = std::min(r_miny,y0) ;
				r_miny = std::min(r_miny,y1) ;
				r_maxy = std::max(r_maxy,y0) ;
				r_maxy = std::max(r_maxy,y1) ;
				++n;
			}
		}
	}
	
	
	//内环
	int numinter =  poly->getNumInteriorRings() ;
	for(int ii = 0 ; ii < numinter;++ ii ){
		const OGRLinearRing * lineRing1 = poly->getInteriorRing(ii) ;
		OGRLineString* lineString1 = (OGRLineString*)lineRing1;
		OGRSimpleCurve* simCurve1 = (OGRSimpleCurve*)lineString1;
		int numPts = simCurve1->getNumPoints() ;
		if( numPts>3 ){
			for(int i = 0 ; i<numPts-1;++i)
			{
				double x0 = simCurve1->getX(i) ;
				double y0 = simCurve1->getY(i) ;
				double x1 = simCurve1->getX(i+1) ;
				double y1 = simCurve1->getY(i+1) ;
				ret_lines.push_back( WLineSeg(x0,y0,x1,y1) ) ;
				++n;
			}
		}
	}
	return n ;
}


#endif


