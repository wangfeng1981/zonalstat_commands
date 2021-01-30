/// convert shape into lonlat lines
/// 增加对geojson输入文件的支持
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include <fstream>
#include <ctime> 
#include <chrono>
#include <iostream>
#include "../wshppolygon.h"
#include "../ajson5.h"
using namespace std;
using namespace ArduinoJson;

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
       [](unsigned char c){ return std::tolower(c); }
    );
    return s;
}

//geojson读取
bool readLinesFromGeojson( string geojsonFile , vector<WShpLineSeg>& lines)
{
	try{
		std::ifstream ifs(geojsonFile.c_str());
		ArduinoJson::DynamicJsonBuffer jsonBuffer;
		ArduinoJson::JsonObject& root = jsonBuffer.parseObject(ifs);
		bool hasfeatures = root.containsKey("features");
		if( hasfeatures == true )
		{
			JsonArray& featureArray = root["features"].as<JsonArray>() ;
			if( featureArray.size() > 0 ){
				JsonObject& featureOne = featureArray[0] ;
				bool hasgeo = featureOne.containsKey("geometry") ;
				if( hasgeo == true ){
					JsonObject& geo1 = featureOne["geometry"] ;
					if( geo1.containsKey("coordinates") ==true ){
							JsonArray& arr0 = geo1["coordinates"].as<JsonArray>() ;
							JsonArray& arr1 = arr0[0].as<JsonArray>() ;
							JsonArray& arr2 = arr1[0].as<JsonArray>() ;
							cout<<"num verts:"<<arr2.size()<<endl ;
							if( arr2.size()>1 )
							{
								for(int iv = 0 ; iv < arr2.size()-1 ; ++ iv )
								{
									JsonArray& vert0 = arr2[iv].as<JsonArray>() ;
									JsonArray& vert1 = arr2[iv+1].as<JsonArray>() ;
									WShpLineSeg seg1 ;
									seg1.x0 = vert0[0].as<double>() ;
									seg1.y0 = vert0[1].as<double>() ;
									seg1.x1 = vert1[0].as<double>() ;
									seg1.y1 = vert1[1].as<double>() ;
									lines.push_back(seg1) ;
								}
								return true ;
							}else{
								cout<<"num verts less than 2"<<endl ;
								return false ;
							}
					}else{
						cout<<"no coordinates."<<endl ;
						return false ;
					}
				}else{
					cout<<"no geometry."<<endl ;
					return false ;
				}
			}else{
				cout<<"zero features."<<endl ;
				return false ;
			}
		}else{
			cout<<"no features."<<endl ;
			return false ;
		}
	}catch(exception& e)
	{
		cout<<"exception:"<<e.what()<<endl ;
		return false ;
	}
}





int main(int argc,char** argv) {
	cout<<"A program to convert shp or geojson file into lonlat lines."<<endl ;
	cout<<"v1.0  by wangfeng_dq@piesat.cn 2019-6-1"<<endl ;
	cout<<"v2.0 add geojson support. 2021-1-30"<<endl ;
	cout<<"shp2lines some.shp lines.json"<<endl ;
	cout<<"shp2lines some.geojson lines.json"<<endl ;

	if( argc !=3 )
	{
		cout<<"params are not enough."<<endl ;
		return 11 ;
	}


	string shpfile = argv[1] ;
	string outfile = argv[2] ;

	if( shpfile.length() < 5 ){
		//not valid file path
		cout<<"Error : the input file path is short than 5, so it's not valid shp/geojson file path."<<endl ;
		return 12 ;
	}
	string lastPartInputFilePath = shpfile.substr( shpfile.length()-4, 4) ;
	bool isShp = true ;

	//判断是否shp文件，或者geojson文件
	lastPartInputFilePath = toLower(lastPartInputFilePath) ;
	if( lastPartInputFilePath == ".shp" )
	{
		isShp = true ;
	}else{
		isShp = false ;//geojson
	}
	cout<<"Is shp? "<<isShp<<endl ;

	vector<WShpLineSeg> linesVec ;
	cout<<"processing..."<<endl ;

	bool isok = false ;

	if( isShp == true )
	{
		isok = WShpPolygon::readLinesFromShapefile(
			shpfile.c_str() ,
			linesVec
			) ;  
	}else{
		isok = readLinesFromGeojson( shpfile , linesVec) ;
	} 

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