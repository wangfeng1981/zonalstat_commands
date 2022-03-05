/// Header Only 线段与水平线段结构体
//line segments, lines

#ifndef W_HSEG_LINE_H
#define W_HSEG_LINE_H

#include <vector>

using std::vector;

///HSEG水平线段对象
struct WHseg {
	int x0, y, x1;
};


///任意两点线段
struct WLineSeg {
	inline WLineSeg(double tx0,double ty0,double tx1,double ty1):
		x0(tx0),y0(ty0),x1(tx1),y1(ty1){} ;
	inline WLineSeg():
		x0(0),y0(0),x1(0),y1(0){} ;
	double x0, y0;
	double x1, y1;
};

#endif