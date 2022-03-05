#include <vector>
#include <fstream>
#include "gdal_priv.h"
#include <iostream>
#include "ogrsf_frmts.h"
#include "whsegline.h"

#include "wgeojsonandshpreader.h"
#define W_GEOJSON_AND_SHP_READER_IMPLEMENT
#include "wgeojsonandshpreader.h"
#undef W_GEOJSON_AND_SHP_READER_IMPLEMENT

#include "wlevelhseg.h"
#define W_LEVEL_HSEG_IMPLEMENT
#include "wlevelhseg.h"
#undef W_LEVEL_HSEG_IMPLEMENT

#include "whsegwriter.h"
#define W_HSEG_GEOTIFF_WRITER_IMPLEMENT
#include "whsegwriter.h"
#undef W_HSEG_GEOTIFF_WRITER_IMPLEMENT

using namespace std;



int main(int argc , char* argv[]) {
	cout<<"A program to convert shp or geojson into hseg.tlv binary format. 2022-2-20. wangfengdev@163.com"<<endl ;
	cout<<"All feature in layer zero will be used in hseg.tlv data."<<endl ;
	cout<<"v1.0.0 created 2022-2-20"<<endl ;
	cout<<"v1.0.1 created 2022-3-2"<<endl ;
	cout<<"v1.0.2 created 2022-3-5"<<endl ;
	cout<<"v1.0.3 created 2022-3-5"<<endl ;
	cout<<"usage: shpgeojson2hegbinary xxx.shp/xxx.geojson output.heg.tlv [mask.tif]"<<endl ;
	GDALAllRegister() ;

	if( argc==3 || argc==4 ){
		//this ok
	}else{
		cout<<"error, argc not 3 or 4"<<endl ;
		return 11 ;
	}

	string infile = argv[1] ;//geojson or shp
	string outfile = argv[2] ;
	string maskfile = "" ;
	if( argc==4 ) maskfile = argv[3] ;

	cout<<"infile:"<<infile<<endl ;
	cout<<"outfile:"<<outfile<<endl ;
	cout<<"maskfile:"<<maskfile<<endl ;

	// 打开shp或者json文件
	vector<WLineSeg> vectorLines ; vectorLines.reserve(200) ;
	string vecError;
	double vecMiny,vecMaxy;
	bool vecOk = wGeoJsonShpReader::readFromFile(infile,vectorLines,
		vecMiny,vecMaxy,vecError);
	if( vecOk==false){
		cout<<"wGeoJsonShpReader failed:"<<vecError<<endl ;
		return 11 ;
	}
	cout<<"line num:"<<vectorLines.size()<<endl ;

	//测试生成的HSeg生成Mask是否正确
	//string writeError;
	//WHsegGeotiffWriter::writeToFile(levelHSeg, infile+"_mask1km.tif" ,writeError) ;

	//写入二进制文件hseg.tlv
	WHsegTlvObject hsegTlvObject0 ;
	hsegTlvObject0.allLevelHsegs.resize(8) ;//0-7
	for(int ilvl = 0 ; ilvl < hsegTlvObject0.allLevelHsegs.size() ; ++ ilvl ){
		hsegTlvObject0.allLevelHsegs[ilvl].init(vectorLines,vecMaxy,vecMiny,ilvl) ;
		cout<<"ilvl:"<<ilvl<<", hsegs:"<<hsegTlvObject0.allLevelHsegs[ilvl].hsegs.size()<<endl ;
	}
	string tlverror;
	bool writeOK = hsegTlvObject0.writeToFile(outfile,tlverror) ;
	if( writeOK==false ){
		cout<<"write tlv failed:"<<tlverror <<endl;
		return 14 ;
	}else{
		cout<<"write ok:"<<outfile<<endl ;
	}

	if( maskfile!= "" ){
		cout<<"write maskfile at level 5:"<<maskfile<<endl ;
		string maskerror ;
		bool maskok = WHsegGeotiffWriter::writeToFile( hsegTlvObject0.allLevelHsegs[5] , maskfile , maskerror) ;
		if( maskok==false ){
			cout<<"Write mask at level 5 failed:"<<maskerror<<endl ;
			return 15 ;
		}
	}

	return 0;
}
