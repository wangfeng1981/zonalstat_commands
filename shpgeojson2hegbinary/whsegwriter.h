/// Header Only 将水平线段数值写到栅格数据文件geotiff用于验证HSeg结果
/// #define W_HSEG_GEOTIFF_WRITER_IMPLEMENT
#ifndef W_HSEG_GEOTIFF_WRITER_H
#define W_HSEG_GEOTIFF_WRITER_H

#include <vector>
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "whsegline.h"
#include <string>
#include <cmath>
#include "wlevelhseg.h"

using std::string;
using std::vector;
using std::ofstream;


struct TLV{
	char tag[4];
	vector<unsigned char> bytes ;
	void print() ;
} ;

struct WHsegGeotiffWriter
{
	static bool writeToFile(wLevelHseg& lvlHseg,string outfilename,string& error);

};


struct WHsegTlvObject {
	vector<wLevelHseg> allLevelHsegs ;
	bool readFromFile(string hsegtlvfile,string& error) ;
	bool writeToFile( string hsegtlvfile,string& error) ;
private:
	void readTLV(FILE* pf,TLV& r_tlv) ;
} ;

#endif





















#ifdef W_HSEG_GEOTIFF_WRITER_IMPLEMENT

bool WHsegGeotiffWriter::writeToFile(wLevelHseg& lvlHseg,string outfilename,string& error)
{
	if( lvlHseg.ilevel <0 || lvlHseg.ilevel>20 ) {
		error = "bad ilevel. valid range should be [0,20]." ;
		return false ;
	}
	if( lvlHseg.hsegs.size()==0 ){
		error = "hsegs has zero elements." ;
		return false ;
	}

	int nxtile = 1;
	for(int il = 0 ;il < lvlHseg.ilevel ;++il ) nxtile*=2 ;
	const double reso = (360.0 / nxtile)/256.0 ; 

	double trans[6] = {0,reso,0,  0,0,-reso} ;
	int ileft = lvlHseg.hsegs[0].x0 ;
	int itop =  lvlHseg.hsegs[0].y ;
	int iright = ileft ;
	int ibottom = itop ;

	for(int ih = 1 ;ih < lvlHseg.hsegs.size() ; ++ ih ){
		ileft = std::min( ileft , lvlHseg.hsegs[ih].x0 ) ;
		iright = std::max( iright , lvlHseg.hsegs[ih].x1 ) ;
		itop = std::min( itop , lvlHseg.hsegs[ih].y ) ;
		ibottom = std::max( ibottom , lvlHseg.hsegs[ih].y ) ;
	}

	int xsize = iright - ileft +1 ;
	int ysize = ibottom - itop +1 ;
	if( xsize<=0 || ysize<=0 ){
		error = "xsize or ysize is zero." ;
		return false ;
	}

	vector<unsigned char> outdata(xsize*ysize,0) ;
	for(int ih = 0 ;ih < lvlHseg.hsegs.size() ; ++ ih ){
		WHseg& hs = lvlHseg.hsegs[ih] ;
		int iy = hs.y - itop ;
		for(int ix = hs.x0; ix <= hs.x1; ++ ix){
			int iix = ix - ileft ;
			outdata[iy*xsize+iix] +=1 ;
		}
	}
	trans[0] = -180 + ileft*reso ;
	trans[3] = 90 - itop*reso ;

	GDALAllRegister() ;
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if( driver==0 ){
		error = "bad driver";
		return false ;
	}
	GDALDataset* outds = driver->Create(outfilename.c_str(),
		xsize,ysize,1,GDT_Byte,0);
	if(outds==0){
		error = "failed to create tif file.";
		return false ;
	}
	outds->SetGeoTransform( trans ) ;  
	outds->SetProjection("GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]");  
	outds->GetRasterBand(1)->RasterIO(GF_Write,0,0,xsize,ysize,outdata.data()
		,xsize,ysize,GDT_Byte,0,0,0) ;
	GDALClose(outds) ;

	return true ;
}

void TLV::print() {
	printf("tag:%c%c%c%c, blen:%d\n" , tag[0],tag[1],tag[2],tag[3],bytes.size()) ;
}


void WHsegTlvObject::readTLV(FILE* pf,TLV& r_tlv) 
{
	r_tlv.bytes.clear() ;
	fread( r_tlv.tag , 1 , 4 ,  pf ) ;
	int blen = 0 ;
	fread( &blen , 4 , 1 , pf) ;
	r_tlv.bytes.resize( blen ) ;
	fread( r_tlv.bytes.data() , 1 , blen , pf) ;
}



bool WHsegTlvObject::readFromFile(string hsegtlvfile,string& error) 
{
	allLevelHsegs.clear() ;
	FILE* pf = fopen( hsegtlvfile.c_str() , "rb" ) ;
	if( pf==0 ){
		error = "Failed to read file " + hsegtlvfile ;
		return false ;
	}

	//00
	{
		TLV tlv ;
		readTLV( pf , tlv) ;
		tlv.print() ;
		if( tlv.bytes.size() != 1 ){
			error = "tlv00 len is not 1." ;
			return false ;
		}
		allLevelHsegs.resize( (int)tlv.bytes[0] ) ;
		cout<<"level num:"<<allLevelHsegs.size()<<endl ;
	}

	//
	for(int il = 0 ; il < allLevelHsegs.size() ; ++ il ){
		TLV tlv ;
		readTLV(pf,tlv) ;
		tlv.print() ;
		int nhseg = tlv.bytes.size()/12 ;
		cout<<"level:"<<il<<", num hseg:"<<nhseg<<endl ;
		allLevelHsegs[il].ilevel = il ;
		if( nhseg>0 ){
			int* intDataPtr = (int*)tlv.bytes.data() ;
			allLevelHsegs[il].hsegs.resize( nhseg ) ;
			for(int ih = 0 ; ih < nhseg; ++ ih )
			{
				allLevelHsegs[il].hsegs[ih].x0 = intDataPtr[ih*3+0] ;
				allLevelHsegs[il].hsegs[ih].y  = intDataPtr[ih*3+1] ;
				allLevelHsegs[il].hsegs[ih].x1 = intDataPtr[ih*3+2] ;
			}
		}
	}

	fclose(pf) ;
	return true ;

}



bool WHsegTlvObject::writeToFile( string hsegtlvfile,string& error) 
{
	if( allLevelHsegs.size() == 0 ){
		error = "Zero allLevelHsegs." ;
		return false ;
	}
	if( allLevelHsegs.size() > 21 ){
		error = "AllLevelHsegs.size greater 21." ;
		return false ;
	}

	ofstream ofs( hsegtlvfile.c_str() , std::ios::out|std::ios::binary);
	if( ofs.good()==false ){
		error = "Failed to open output file:" + hsegtlvfile ;
		return false ;
	}

	char tag[5]="NLVL" ;
	int value00size = 1 ;//1Byte
	unsigned char lvlNum = (unsigned char)allLevelHsegs.size();
	ofs.write( tag , 4) ;
	ofs.write( (char*)&value00size , 4) ;
	ofs.write( (char*)&lvlNum , 1) ;

	for(int il = 0 ; il < allLevelHsegs.size() ; ++ il ){
		sprintf(tag,"L%03d",il) ;
		ofs.write( tag , 4) ;
		int valbytes = allLevelHsegs[il].hsegs.size()*12;
		ofs.write( (char*)&valbytes , 4) ;
		if( valbytes > 0 ) {
			vector<WHseg>& hsegvec = allLevelHsegs[il].hsegs ;
			vector<int> datavec; datavec.resize(3*hsegvec.size()) ;
			for(int ih = 0 ; ih < hsegvec.size() ; ++ ih ){
				datavec[ih*3+0] = hsegvec[ih].x0 ;
				datavec[ih*3+1] = hsegvec[ih].y ;
				datavec[ih*3+2] = hsegvec[ih].x1 ;
			}
			cout<<"write int "<<datavec.size()<<" as Bytes(KB) "<<datavec.size()*4/1024.0<<endl ;
			ofs.write( (char*)datavec.data() , datavec.size() * 4) ;
		}
	}

	ofs.close() ;

	return true ;
}


#endif