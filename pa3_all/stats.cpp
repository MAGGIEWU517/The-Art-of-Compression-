#include "cs221util/PNG.h"
#include "cs221util/RGBAPixel.h"

#include "stats.h"
#include <iostream>
#include <utility>
#include <vector>
#include <cmath>

using namespace std;
using namespace cs221util;

	// initialize the private vectors so that, for each color,  entry 
	// (x,y) is the cumulative sum of the the color values from (0,0)
	// to (x,y). Similarly, the sumSq vectors are the cumulative
	// sum of squares from (0,0) to (x,y).
	stats::stats(PNG & im){
		unsigned int width = im.width();
		unsigned int height = im.height();


        sumRed.resize(width, vector<long>(height, 0));
        sumGreen.resize(width, vector<long>(height, 0));
        sumBlue.resize(width, vector<long>(height, 0));
		sumsqRed.resize(width, vector<long>(height, 0));
        sumsqGreen.resize(width, vector<long>(height, 0));
        sumsqBlue.resize(width, vector<long>(height, 0));

        cout<<"start"<<endl;
			long sumr = 0;
			long sumg = 0;
			long sumb = 0;
			long sumsqr = 0;
			long sumsqg = 0;
			long sumsqb = 0;
			for(unsigned int j=0; j<height; j++){
				RGBAPixel* pxl = im.getPixel(0,j);
			    sumr = sumr + pxl->r;
				sumg = sumg + pxl->g;
				sumb = sumb + pxl->b;
				sumsqr = sumsqr + (pxl->r)*(pxl->r);
				sumsqg = sumsqg + (pxl->g)*(pxl->g);
				sumsqb = sumsqb + (pxl->b)*(pxl->b);

				sumRed[0][j] = sumr;
				sumGreen[0][j] = sumg;
				sumBlue[0][j] = sumb;
				sumsqRed[0][j] = sumsqr;
				sumsqGreen[0][j] = sumsqg;
				sumsqBlue[0][j] = sumsqb;
			}

			for(unsigned int i = 0; i<width; i++){
				RGBAPixel* pxl = im.getPixel(i,0);
			    sumr = sumr + pxl->r;
				sumg = sumg + pxl->g;
				sumb = sumb + pxl->b;
				sumsqr = sumsqr + (pxl->r)*(pxl->r);
				sumsqg = sumsqg + (pxl->g)*(pxl->g);
				sumsqb = sumsqb + (pxl->b)*(pxl->b);

				sumRed[i][0] = sumr;
				sumGreen[i][0] = sumg;
				sumBlue[i][0] = sumb;
				sumsqRed[i][0] = sumsqr;
				sumsqGreen[i][0] = sumsqg;
				sumsqBlue[i][0] = sumsqb;
			}

		for(unsigned int i =1; i<width; i++){
			for(unsigned int j=1; j<height; j++){
				RGBAPixel* pxl = im.getPixel(i,j);

				sumRed[i][j] = sumRed[i-1][j]+sumRed[i][j-1]-sumRed[i-1][j-1]+ pxl->r;
				sumGreen[i][j] = sumGreen[i-1][j]+sumGreen[i][j-1]-sumGreen[i-1][j-1]+ pxl->g;
				sumBlue[i][j] = sumBlue[i-1][j]+sumBlue[i][j-1]-sumBlue[i-1][j-1]+ pxl->b;

			
				sumsqRed[i][j] = sumsqRed[i-1][j]+sumsqRed[i][j-1]-sumsqRed[i-1][j-1]+ (pxl->r)*(pxl->r);
				sumsqGreen[i][j] = sumsqGreen[i-1][j]+sumsqGreen[i][j-1]-sumsqGreen[i-1][j-1]+ (pxl->g)*(pxl->g);
				sumsqBlue[i][j] = sumsqBlue[i-1][j]+sumsqBlue[i][j-1]-sumsqBlue[i-1][j-1]+ (pxl->b)*(pxl->b);
				
			}
		}
		cout<<"final"<<endl;
		
	} 

	// given a rectangle, compute its sum of squared deviations from 
        // mean, over all color channels. Will be used to make split when
	// building tree.
	/* @param ul is (x,y) of the upper left corner of the rectangle 
	* @param lr is (x,y) of the lower right corner of the rectangle */
	long stats::getScore(pair<int,int> ul, pair<int,int> lr){
		long area = rectArea(ul,lr);

		long double diff;
		long sumr = getSum('r', ul,lr);
		long sumg = getSum('g', ul,lr);
		long sumb = getSum('b', ul,lr);
		long sumsqr = getSumSq('r', ul,lr);
		long sumsqg = getSumSq('g', ul,lr);
		long sumsqb = getSumSq('b', ul,lr);


		long double diffr = sumsqr - ((sumr*sumr)/(double)area);
		long double diffg = sumsqg - ((sumg*sumg)/(double)area);
		long double diffb = sumsqb - ((sumb*sumb)/(double)area);

		diff = diffr + diffg + diffb;

		return diff;

	};

	// given a rectangle, return the average color value over the 
	// rectangle as a pixel.
	/* Each color component of the pixel is the average value of that 
	* component over the rectangle.
	* @param ul is (x,y) of the upper left corner of the rectangle 
	* @param lr is (x,y) of the lower right corner of the rectangle */
	RGBAPixel stats::getAvg(pair<int,int> ul, pair<int,int> lr){
		long Redsum = getSum('r',ul,lr);
		long Greensum = getSum('g',ul,lr);
		long Bluesum = getSum('b',ul,lr);
		long totaln = rectArea(ul,lr);
		
		long double redavg = (double)Redsum/(double)totaln;
		long double greenavg = (double)Greensum/(double)totaln;
	    long double blueavg = (double)Bluesum/(double)totaln;

		return RGBAPixel(redavg,greenavg,blueavg);
		
	}

	// given a rectangle, return the number of pixels in the rectangle
	/* @param ul is (x,y) of the upper left corner of the rectangle 
	* @param lr is (x,y) of the lower right corner of the rectangle */
	long stats::rectArea(pair<int,int> ul, pair<int,int> lr){
		int x1 = ul.first;
		int y1 = ul.second;

		int x2 = lr.first;
		int y2 = lr.second;

		if(x1>x2 || y1>y2){
			cout<<"wrong input of ul and lr"<<endl;
			return -1;
		}
	
		int xdiff = x2 - x1 + 1;
		int ydiff = y2 - y1 + 1;

		long totaln = xdiff * ydiff;

		return totaln;

	}

	/* returns the sums of all pixel values across all color channels.
	* useful in computing the score of a rectangle
	* PA3 function
	* @param channel is one of r, g, or b
	* @param ul is (x,y) of the upper left corner of the rectangle 
	* @param lr is (x,y) of the lower right corner of the rectangle */
	long stats::getSum(char channel, pair<int,int> ul, pair<int,int> lr){
		int x1 = ul.first;
		int x2 = lr.first;
		int y1 = ul.second;
		int y2 = lr.second;

		if(x1==0 && y1==0){
			if(channel == 'r'){
				return sumRed[x2][y2];
			}else if(channel == 'g'){
				return sumGreen[x2][y2];
			}else if(channel == 'b'){
				return sumBlue[x2][y2];
			}else{
				return 0;
				cout<<"for x1=0 y1=0, channel is wrong";
			}
		}else if(x1==0 && y1!=0){
			if(channel == 'r'){
				long sumr = sumRed[x2][y2]- sumRed[x2][y1-1];
				return sumr;
			}else if(channel == 'g'){
				long sumg = sumGreen[x2][y2] - sumGreen[x2][y1-1];
				return sumg;
			}else if(channel == 'b'){
				long sumb = sumBlue[x2][y2] - sumBlue[x2][y1-1];
				return sumb;
			}else{
				return 0;
				cout<<"with x1=0 channel does not exist"<<endl;
			}
		}else if(y1 == 0 && x1!=0){
		    if(channel == 'r'){
				long sumr = sumRed[x2][y2] - sumRed[x1-1][y2];
				return sumr;
			}else if(channel == 'g'){
				long sumg = sumGreen[x2][y2] - sumGreen[x1-1][y2];
				return sumg;
			}else if(channel == 'b'){
				long sumb = sumBlue[x2][y2] - sumBlue[x1-1][y2];
				return sumb;
			}else{
				return 0;
				cout<<"with y1=0 channel does not exist"<<endl;
			}
		}else{
			if(channel == 'r'){
			long sumr = sumRed[x2][y2] + sumRed[x1-1][y1-1] - sumRed[x1-1][y2]-sumRed[x2][y1-1];
			return sumr;
		}else if(channel == 'g'){
			long sumg = sumGreen[x2][y2] + sumGreen[x1-1][y1-1] - sumGreen[x1-1][y2]-sumGreen[x2][y1-1];
			return sumg;
		}else if(channel == 'b'){
			long sumb = sumBlue[x2][y2] + sumBlue[x1-1][y1-1] - sumBlue[x1-1][y2]-sumBlue[x2][y1-1];
			return sumb;
		}else{
			return 0;
			cout<<"channel does not exist"<<endl;
		}
	}
}

	/* returns the sums of squares of all pixel values across all color channels.
	* useful in computing the score of a rectangle
	* PA3 function
	* @param channel is one of r, g, or b
	* @param ul is (x,y) of the upper left corner of the rectangle 
	* @param lr is (x,y) of the lower right corner of the rectangle */
	long stats::getSumSq(char channel, pair<int,int> ul, pair<int,int> lr){
		int x1=ul.first;
		int x2=lr.first;
		int y1=ul.second;
		int y2=lr.second;

		
		if(x1==0 && y1==0){
			if(channel == 'r'){
				return sumsqRed[x2][y2];
			}else if(channel == 'g'){
				return sumsqGreen[x2][y2];
			}else if(channel == 'b'){
				return sumsqBlue[x2][y2];
			}else{
				return 0;
				cout<<"for x1=0 y1=0, channel is wrong sq";
			}
		}else if(x1==0 && y1!=0){
			if(channel == 'r'){
				long sumsqr = sumsqRed[x2][y2]- sumsqRed[x2][y1-1];
				return sumsqr;
			}else if(channel == 'g'){
				long sumsqg = sumsqGreen[x2][y2] - sumsqGreen[x2][y1-1];
				return sumsqg;
			}else if(channel == 'b'){
				long sumsqb = sumsqBlue[x2][y2] - sumsqBlue[x2][y1-1];
				return sumsqb;
			}else{
				return 0;
				cout<<"with x1=0 channel does not exist"<<endl;
			}
		}else if(y1 == 0 && x1!=0){
		    if(channel == 'r'){
				long sumsqr = sumsqRed[x2][y2] - sumsqRed[x1-1][y2];
				return sumsqr;
			}else if(channel == 'g'){
				long sumsqg = sumsqGreen[x2][y2] - sumsqGreen[x1-1][y2];
				return sumsqg;
			}else if(channel == 'b'){
				long sumsqb = sumsqBlue[x2][y2] - sumsqBlue[x1-1][y2];
				return sumsqb;
			}else{
				return 0;
				cout<<"with y1=0 channel does not exist"<<endl;
			}
		}else{
			if(channel == 'r'){
			long sumsqr = sumsqRed[x2][y2] + sumsqRed[x1-1][y1-1] - sumsqRed[x1-1][y2]-sumsqRed[x2][y1-1];
			return sumsqr;
		}else if(channel == 'g'){
			long sumsqg = sumsqGreen[x2][y2] + sumsqGreen[x1-1][y1-1] - sumsqGreen[x1-1][y2]-sumsqGreen[x2][y1-1];
			return sumsqg;
		}else if(channel == 'b'){
			long sumsqb = sumsqBlue[x2][y2] + sumsqBlue[x1-1][y1-1] - sumsqBlue[x1-1][y2]-sumsqBlue[x2][y1-1];
			return sumsqb;
		}else{
			return 0;
			cout<<"channel does not exist"<<endl;
		}
	}
}