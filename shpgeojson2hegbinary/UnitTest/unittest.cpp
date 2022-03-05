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

	GDALAllRegister() ;

	{
		cout<<"Unit test ESRI Shape......"<<endl ;
		string infile = "ne_50m_land.shp" ;//geojson or shp
		string outfile ="ne_50m_land.hseg.tlv" ;
		string maskfile="ne_50m_land.mask.tif" ;

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

		//测试
		WHsegTlvObject readHsegTlv ;
		bool readok = readHsegTlv.readFromFile( outfile , tlverror) ;
		if( readok == false ){
			cout<<"read tlv failed:"<<tlverror<<endl ;
			return 15 ;
		}else{
			cout<<"read tlv ok:"<<outfile<<endl ;
			cout<<"write back to mask2..."<<endl ;
			//write back to mask2....
			string mask2file = maskfile + "_mask2.tif" ;
			string maskerror ;
			bool maskok = WHsegGeotiffWriter::writeToFile( hsegTlvObject0.allLevelHsegs[5] ,
				 mask2file , maskerror) ;
			if( maskok==false ){
				cout<<"Write mask at level 5 failed:"<<maskerror<<endl ;
				return 15 ;
			}else{
				cout<<"write mask2file ok."<<endl ;
			}
		}

	}


	{
		cout<<"Unit test Geojson......"<<endl ;
		string infile = "test2.geojson" ;//geojson or shp
		string outfile ="test2.hseg.tlv" ;
		string maskfile="test2.mask.tif" ;

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

		//测试
		WHsegTlvObject readHsegTlv ;
		bool readok = readHsegTlv.readFromFile( outfile , tlverror) ;
		if( readok == false ){
			cout<<"read tlv failed:"<<tlverror<<endl ;
			return 15 ;
		}else{
			cout<<"read tlv ok:"<<outfile<<endl ;
			//write back to mask2....
			string mask2file = maskfile + "_mask2.tif" ;
			string maskerror ;
			bool maskok = WHsegGeotiffWriter::writeToFile( hsegTlvObject0.allLevelHsegs[5] ,
				 mask2file , maskerror) ;
			if( maskok==false ){
				cout<<"Write mask at level 5 failed:"<<maskerror<<endl ;
				return 15 ;
			}else{
				cout<<"write mask2file ok."<<endl ;
			}
		}

	}
	
	

	return 0;
}
