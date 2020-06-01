/// convert shape into lonlat lines
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <iostream>
#include <ctime>
#include <chrono>
#include <iostream>
#include "../wshppolygon.h"
using namespace std;



int main(int argc,char** argv) {
	cout<<"A program to convert shp file into lonlat lines."<<endl ;
	cout<<"v1.0  by wangfeng_dq@piesat.cn 2019-6-1"<<endl ;
	cout<<"shp2lines some.shp lines.json"<<endl ;

	if( argc !=3 )
	{
		cout<<"params are not enough."<<endl ;
		return 11 ;
	}


	string shpfile = argv[1] ;
	string outfile = argv[2] ;

	vector<WShpLineSeg> linesVec ;
	cout<<"processing..."<<endl ;
	bool isok = WShpPolygon::readLinesFromShapefile(
		shpfile.c_str() , 
		linesVec
		) ;

	if( isok )
	{//output to json
		FILE* pf = fopen(outfile.c_str() , "w" ) ;
		if( pf==0 )
		{
			cout<<"Error: failed to open output file to write."<<endl ;
			return 13 ;
		}else
		{
			int nlines = linesVec.size() ;
			fprintf(pf,"{\"comments\":\"x0,y0,x1,y1\",\"numlines\":%d, \"points_array\":[\n" , nlines) ;
			for(int i = 0 ; i<nlines ; ++ i )
			{
				WShpLineSeg& l1 = linesVec[i] ;
				if( i== nlines-1 )
				{
					fprintf(pf,"%f,%f,%f,%f\n", l1.x0,l1.y0,l1.x1,l1.y1 ) ;
				}else
				{
					fprintf(pf,"%f,%f,%f,%f,\n", l1.x0,l1.y0,l1.x1,l1.y1 ) ;
				}
			}
			fprintf(pf,"]}\n") ;
			fclose(pf) ;
			cout<<"done."<<endl ;
		}
	}else
	{
		cout<<"Error: failed to readLinesFromShapefile."<<endl ;
		return 12 ;
	}


	return 0 ;
}