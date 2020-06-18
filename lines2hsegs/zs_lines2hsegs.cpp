/// convert shape into lonlat lines
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <ctime>
#include <chrono>
#include <iostream>
#include "../wshppolygon.h"
#include "../ajson5.h"

using namespace std;
using namespace ArduinoJson;


int main(int argc,char** argv) { 
	cout<<"A program to convert lonlat lines into horisegments."<<endl ;
	cout<<"v1.0  by wangfeng_dq@piesat.cn 2019-6-1"<<endl ;
	cout<<"v1.1 bugfixed in horiLineInterLineSeg 2020-6-18 "<<endl ;
	cout<<"lines2hsegs lines.json leftx topy resox resoy(should <0) imgwid imghei hsegs.json"<<endl ;

	if( argc !=9 )
	{
		cout<<"params are not enough."<<endl ;
		return 11 ;
	}


	string linesfile = argv[1] ;
	const double leftX = atof(argv[2]) ;
	const double topY  = atof(argv[3]) ;
	const double resoX = atof(argv[4]) ;
	const double resoY = atof(argv[5]) ;
	const int imageWidth = atof(argv[6]) ;
	const int imageHeight= atof(argv[7]) ;
	string outfile = argv[8] ;



	vector<WShpLineSeg> linesVec ;

	std::ifstream ifs(linesfile.c_str());
	ArduinoJson::DynamicJsonBuffer jsonBuffer;
	ArduinoJson::JsonObject& root = jsonBuffer.parseObject(ifs);
	JsonArray& pointsArray = root["points_array"].as<JsonArray>();

	int numvals = pointsArray.size() ;
	int numLines = numvals/4 ;
	linesVec.resize(numLines) ;
	cout<<"reading lines from json ..."<<endl ;
	for(int i = 0 ; i<numLines ; ++ i )
	{
		WShpLineSeg& line1 = linesVec[i] ;
		line1.x0 = pointsArray[i*4].as<double>() ;
		line1.y0 = pointsArray[i*4+1].as<double>() ;
		line1.x1 = pointsArray[i*4+2].as<double>() ;
		line1.y1 = pointsArray[i*4+3].as<double>() ;
	}


	ifs.close() ;
	cout<<"read num lines :"<< linesVec.size()<<endl ;
	cout<<"processing ..."<<endl ;
	vector<WHoriLineSeg> segs ;
	WShpPolygon::convertLines2Segments(linesVec , segs ,
		topY , imageHeight , resoY , 
		leftX , imageWidth , resoX ) ;

	{
		FILE* pf = fopen(outfile.c_str() , "w" ) ;
		if( pf==0 )
		{
			cout<<"Error: failed to open output file to write."<<endl ;
			return 13 ;
		}else
		{
			int num = segs.size() ;
			fprintf(pf,"{\"comments\":\"x0,y,x1\",\"num\":%d, \"seg_array\":[\n" , num) ;
			for(int i = 0 ; i<num ; ++ i )
			{
				WHoriLineSeg& s1 = segs[i] ;
				if( i== num-1 )
				{
					fprintf(pf,"%d,%d,%d\n", s1.x0,s1.y,s1.x1 ) ;
				}else
				{
					fprintf(pf,"%d,%d,%d,\n", s1.x0,s1.y,s1.x1) ;
				}
			}
			fprintf(pf,"]}\n") ;
			fclose(pf) ;
			cout<<"write segs num:"<<segs.size()<<endl ;
			cout<<"done."<<endl ;
		}
	}
	
	return 0 ;
}
