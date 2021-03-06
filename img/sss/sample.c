#include <stdio.h>
#include <mem.h>
#include <math.h>
#include <stdlib.h>
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned long  LONG;
typedef unsigned char  BYTE;

#include            "fileio.h"

#define IMGSIZ            256
#define PI 3.141592653
#define N 256

#ifndef BI_RGB
#define BI_RGB         0
#endif

#pragma pack(1)

typedef struct tagBITMAPFILEHEADER
{
    WORD   bfType;
    DWORD  bfSize;
    WORD   bfReserved1;
    WORD   bfReserved2;
    DWORD  bfOffBits;
}BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
    DWORD   biSize;
    LONG    biWidth;
    LONG    biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
}BITMAPINFOHEADER;

typedef struct tagRGBQUAD
{
       BYTE rgbBlue;
       BYTE rgbGreen;
       BYTE rgbRed;
       BYTE rgbReserved;
}RGBQUAD;

#pragma pack()
//////////////////////////////////////////////////////
unsigned char            winbuf[IMGSIZ + 128][IMGSIZ * 3];
unsigned int            hist[IMGSIZ];
float y[N]={0};
float a[N]={0};

//////////////////////////////////////////////////////

void Gray_Raw2Bmp(unsigned char *pbuf, DWORD nWidth, DWORD nHeight, char *pBmpName)
{
       BITMAPFILEHEADER  file_h;
       BITMAPINFOHEADER  info_h;
       DWORD             dwBmpSize=0;
       DWORD             dwRawSize=0;
       DWORD             dwLine=0;
       long              lCount, i;
       FILE             *out;
       char             *pData=NULL;
       RGBQUAD           rgbPal[256];

       out=fopen(pBmpName, "wb");

       file_h.bfType      = 0x4D42;
       file_h.bfReserved1 = 0;
       file_h.bfReserved2 = 0;
       file_h.bfOffBits   =  sizeof(rgbPal) + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
       info_h.biSize          = sizeof(BITMAPINFOHEADER);
       info_h.biWidth         = (DWORD)nWidth;
       info_h.biHeight        = (DWORD)nHeight;
       info_h.biPlanes        = 1;
       info_h.biBitCount      = 8;
       info_h.biCompression   = BI_RGB;
       info_h.biXPelsPerMeter = 0;
       info_h.biYPelsPerMeter = 0;
       info_h.biClrUsed       = 0;
       info_h.biClrImportant  = 0;



       dwLine=((((info_h.biWidth * info_h.biBitCount) + 31) &~ 31) >> 3) ;
       dwBmpSize=dwLine * info_h.biHeight;
       info_h.biSizeImage     = dwBmpSize;
       file_h.bfSize          = dwBmpSize + file_h.bfOffBits + 2;



       dwRawSize=info_h.biWidth*info_h.biHeight;
       pData=(char *)malloc(sizeof(char)*dwRawSize+16);

       if (pData)
       {
             memcpy(pData,pbuf, dwRawSize);

             for(i=0; i < 256; i++)
             {
                    rgbPal[i].rgbRed=(BYTE)(i%256);
                    rgbPal[i].rgbGreen=rgbPal[i].rgbRed;
                    rgbPal[i].rgbBlue=rgbPal[i].rgbRed;
                    rgbPal[i].rgbReserved=0;
             }

             fwrite((char *)&file_h, 1, sizeof(BITMAPFILEHEADER), out);
             fwrite((char *)&info_h, 1, sizeof(BITMAPINFOHEADER), out);
             fwrite((char *)rgbPal, 1, sizeof(rgbPal), out);

             lCount=dwRawSize;

             for(lCount-=(long)info_h.biWidth; lCount >= 0; lCount-=(long)info_h.biWidth)
             {
                    fwrite((pData+lCount), 1, (long)dwLine, out);
             }

             free(pData);
       }
       fclose(out);
       return;
}
////////////////////////////////////////////////////////////////////////
//			버퍼를 원하는 위도우에 복사
////////////////////////////////////////////////////////////////////////
void window(unsigned char *pbuf,unsigned char win){
	int i,j;
    for(i=0; i<IMGSIZ; i++) {
        for(j=0; j<IMGSIZ; j++) {
        	winbuf[i][(256*win) + j] = *((pbuf + (256*i)) + j);
        }
    }
}

////////////////////////////////////////////////////////////////////////
//			히스토그램 그래프 만들기
////////////////////////////////////////////////////////////////////////
void histogram(unsigned char *pbuf1,unsigned char *pbuf2){
	int i,j,pos=0;
	unsigned int top=0;
	float pp=0;

	for(i=0; i<IMGSIZ; i++)
		hist[i] = 0;

    for(i=0; i<IMGSIZ; i++)
        for(j=0; j<IMGSIZ; j++)
        	hist[*((pbuf1 + (256*i)) + j)]++;

    ///////////// 최대값 찾기 ///////////////
    for(i=0;i<256;i++){
    	pp = pp + (float)hist[i];
    	if(top < hist[i]){
    		top = hist[i];
    	}
    }

    pos = (unsigned int)(pp/(float)255);

    //printf("%d %f %d",top,pp,pos);
    //scanf("%d",&top);


    if(top > 50*pos){
    	top = pos;
    }

    top = 1200;
    //////////// 히스토그램 크기조정 /////////
    for(i=0;i<256;i++){
		hist[i] = (unsigned int)(((float)hist[i])*((float)255)/top);
	}
    top=0;
    for(i=0;i<256;i++){
		if(top > hist[i]){
			top = hist[i];
		}
	}

    ////////////////////// 그래프 만들기 ///////////////////////
    for(j=0;j<256;j++)
		for(i=255;255 - (unsigned char)hist[j] <= i;i--)
			*((pbuf2 + (256*i)) + j) = 0;

}

void maketable(void){
	int i, j;
	float s, t;

	s=0;
	for(j=0;j<N;j++) s += y[j];

	a[0] = s/N;

	for(i=2;i<N;i+=2){
		t=i*PI/N;
		s=0;
		for(j=0;j<N;j++) s += y[j] * cos(t*j);
		a[i-1]=2*s/N;
		s=0;
		for(j=0;j<N;j++) s += y[j] * sin(t*j);
		a[i]=2*s/N;
	}
	if(i == N){
		s=0;
		t=i*PI/N;
		for(j=0;j<N;j++) s += y[j] * cos(t*j);
		a[i-1]=s/N;
	}
}


int main(void)
{

	int			status;
	int         nrow=256;
	int         ncol=256;
	int			maxlevel=256;
	int			i, j, k;
	int			max, spemax;
	int			hist[256], spehist[256];		//히스토그램 배열
	int			acc[256], speacc[256];		//누적화된 합 배열
	double			result, small;

	char            fname[30];		//파일명 배열

	unsigned char            buf1[IMGSIZ][IMGSIZ];		//버퍼1 배열
	unsigned char            buf2[IMGSIZ][IMGSIZ];		//버퍼2 배열
	unsigned char            buf3[IMGSIZ][IMGSIZ];
	unsigned char            buf4[IMGSIZ][IMGSIZ];

				//파일명과 buf1,buf2의 초기화
	            memset(fname, 0, 30);
	            memset(buf1, 0, IMGSIZ*IMGSIZ);
	            memset(buf2, 0, IMGSIZ*IMGSIZ);
	            memset(buf3, 0, IMGSIZ*IMGSIZ);
	            memset(buf4, 255, IMGSIZ*IMGSIZ);
	            window(buf4,0);
	            window(buf4,1);
	            window(buf4,2);
	            //히스토그램 평활화를 적용할 원본 이미지 불러오기
	            //printf("Enter input Base file name: ");

	            //scanf("%s", fname);
	            status=load_data("lena.raw", buf1, 'b', IMGSIZ, IMGSIZ);

	            if( status != SUCCESS )
	                        exit(1);

	            memset(buf2, 255, IMGSIZ*IMGSIZ);
	            histogram(buf1,buf2);
	            window(buf2,0);
	            //목적 히스토그램을 가지고 있는 이미지 불러오기
	            //printf("Enter input Histogram file name: ");

	            //scanf("%s", fname);
	            status=load_data("flower.raw", buf3, 'b', IMGSIZ, IMGSIZ);

	            if( status != SUCCESS )
	                        exit(1);

	            memset(buf2, 255, IMGSIZ*IMGSIZ);
				histogram(buf3,buf2);
				window(buf2,1);
	            //히스토그램 배열을 0으로 초기화
	            for(k=0; k<=maxlevel; k++) {
	            	hist[k]=0;
	            	spehist[k]=0;
	            }

	            //원본영상과 목적이미지에 대한 히스토그램 작성
	            for(i=0; i<=nrow; i++){
	            	for(j=0; j<=ncol; j++)
	            		hist[buf1[i][j]]++;
						spehist[buf3[i][j]]++;
	            }

	            //히스토그램 누적화합 배열을 0으로 초기화
	            for(k=0; k<=maxlevel; k++){
	            	acc[k]=0;
	            	speacc[k]=0;
	            }

	            //히스토그램에 대해 각각의 명암값에 대한 빈도수를 누적
	            acc[0]=hist[0];
	            speacc[0]=spehist[0];
	            for(i=1; i<255; i++){
	            	acc[i]=acc[i-1]+hist[i];
	            	speacc[i]=speacc[i-1]+spehist[i];
	            }

	            //빈도수의 최대값 찾기
	            max=0;
	            spemax=0;
				for(j=0; j<255; j++){
					if (acc[j]>max)
						max=acc[j];
				}

				for(j=0; j<255; j++){
					if (speacc[j]>spemax)
						spemax=speacc[j];
				}

				//누적된 빈도수의 최대값을 255로 바꾸는 연산
	            for(i=0; i<255; i++){
	                acc[i]=(unsigned char)(((float)acc[i])*(255/(float)max)+0.5);
	            }

	            for(i=0; i<255; i++){
	                speacc[i]=(unsigned char)(((float)speacc[i])*(255/(float)spemax)+0.5);
	            }

	            //buf2에 평활화된 이미지값을 저장
	            for(i=0; i<=nrow; i++){
	            	for(j=0; j<=ncol; j++)
	                	buf2[i][j]=acc[buf1[i][j]];
	            }


	            //평활화된 대상 히스토그램에 대한 역 평활화(명세화)
	            for(i=0; i<=255; i++){
	            	result = fabs(i - speacc[0]);
	            	small = 0;
	            		for(j=0; j<=255; j++){
	            			if (fabs(i-speacc[j]) < result){
	            				result = fabs(i - speacc[j]);
	            				small = j;
	            			}
	            		}
	            		spehist[i] = (unsigned char) small;
	            	}

	            //명세화 된 이미지 저장
	            for(i=0; i<=nrow; i++){
	               	for(j=0; j<=ncol; j++)
	                   	buf4[i][j]=spehist[buf2[i][j]];
	            }
	            memset(buf2, 255, IMGSIZ*IMGSIZ);
	            	            histogram(buf4,buf2);
	            	            window(buf2,2);
    Gray_Raw2Bmp(winbuf,IMGSIZ * 3, IMGSIZ + 64,"test.bmp");
}
