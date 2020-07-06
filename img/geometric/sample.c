#include <stdio.h>
#include <mem.h>
#include <stdlib.h>
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned long  LONG;
typedef unsigned char  BYTE;

#include            "fileio.h"

#define IMGSIZ            256
#define SWAP(x, y, t) ( (t)=(x), (x)=(y), (y)=(t) )

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
unsigned char			sort_buf[9];

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

/////////////////////////////////////////////////////////////////////////////////////////
void Line(BYTE x1,BYTE y1,BYTE X2, BYTE Y2,BYTE z)
{
  BYTE  incx,incy ,rev;
  BYTE  i,distance   ;
  BYTE  Dx,Dy,xerr,yerr ;

  rev=z;
  Dx = X2 - x1 ;
  Dy = Y2 - y1 ;
  if ( Dx > 0 )      incx = 1  ;       // 라인의 시작점이 왼쪽에 있을 경우
  else if (Dx == 0 ) incx = 0  ;       // 라인이 수직선일 경우
  else               incx = -1 ;           // 라인의 시작점이 오른쪽에 있을 경우

  if ( Dy > 0 )       incy = 1 ;       // 라인의 시작점이  위에 있을 경우
  else  if ( Dy == 0 ) incy = 0 ;      // 라인이 수평선일 경우
  else  		    incy = -1 ;      // 라인의 시작점이  밑에 있는 경우

  if(Dy<0) Dy=~Dy+1;    		     //  delta Y 절대값
  if(Dx<0) Dx=~Dx+1;     		     //  delta X 절대값

  if ( Dx > Dy ) distance = Dx ;
  else           distance = Dy ;
  //------------------------------------------------------------
  yerr = 0;
  xerr = 0;
  for(i=0;i<=distance+1;i++)
  {

    if(rev==0)      winbuf[x1][y1] = 0;
    else if(rev==1) winbuf[x1][y1] = 1;
    else
    {
      if(i == 0)winbuf[x1][y1] = 0;
      else  winbuf[x1][y1] = 1;
    }
    xerr += Dx ;
    yerr += Dy ;
    if ( xerr > distance )
    {
      xerr -= distance ;
      x1 += incx ;
    }

    if(yerr > distance )
    {
      yerr -= distance ;
      y1 += incy;
    }
  }
}
/////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//			정렬
////////////////////////////////////////////////////////////////////////
void sort(unsigned char list[], int n)
{
   int i, j, temp;
   for(i=n-1; i>0; i--){
		for(j=0; j<i; j++)
			if(list[j]>list[j+1])
				SWAP(list[j], list[j+1], temp);
   }
}
int main(void)
{
	//버퍼 선언
	unsigned char            buf1[IMGSIZ][IMGSIZ];
	unsigned char            buf2[IMGSIZ][IMGSIZ];
	unsigned char            buf3[IMGSIZ][IMGSIZ];

	int		status;
	int 	i,j,k;
	int 	mode;
	int		avg;

	float a,b,c,d;

	int div = 4;

	char name_s[5][13] = {"shepp_b1.raw","shepp_b2.raw","shepp_b3.raw","shepp_b4.raw","shepp_b5.raw"};

	//버퍼들의 초기화
	memset(buf1, 0, IMGSIZ*IMGSIZ);
	memset(buf2, 0, IMGSIZ*IMGSIZ);
	memset(buf3, 0, IMGSIZ*IMGSIZ);
	memset(winbuf, 0, (IMGSIZ + 64)*(IMGSIZ*3));

	mode = 0;
	a=b=c=d=0;

	// 코드 작성시작
	status = load_data("lena copy.raw", buf1, 'b', IMGSIZ, IMGSIZ);
	if( status != SUCCESS )
		exit(1);

	mode = 2;
	for(mode=1;mode<5;mode++){
		memset(winbuf, 0, (IMGSIZ + 64)*(IMGSIZ*3));
		memset(buf2, 0, IMGSIZ*IMGSIZ);
		switch(mode){
		case 1:
			//원본
			window(buf1,0);
			div = 4;

			//화면축소
			for(i=0;i<256;i=i+div){
				for(j=0;j<256;j=j+div){
					buf2[(i/div)][(j/div)] = buf1[i][j];
				}
			}
			for(i=0;i<256;i++){
				for(j=0;j<256;j++){
					buf3[i][j] = buf2[(i/div)][(j/div)];
				}
			}
			window(buf3,1);


			//화면확대
			for(i=3;i<256;i=i+div){
				for(j=30;j<256;j=j+div){

					avg = buf1[i][j]+buf1[i][j-1]+buf1[i][j-2]+buf1[i][j-3]+
							buf1[i-1][j]+buf1[i-1][j-1]+buf1[i-1][j-2]+buf1[i-1][j-3]+
							buf1[i-2][j]+buf1[i-2][j-1]+buf1[i-2][j-2]+buf1[i-2][j-3]+
							buf1[i-3][j]+buf1[i-3][j-1]+buf1[i-3][j-2]+buf1[i-3][j-3];

					avg /= (div * div);
					buf2[(i/div)][(j/div)] = (unsigned char)(avg);
				}
			}
			for(i=0;i<256;i++){
				for(j=0;j<256;j++){
					buf3[i][j] = buf2[(i/div)][(j/div)];
				}
			}
			window(buf3,2);

			//결과 출력
			Gray_Raw2Bmp(winbuf,IMGSIZ * 3, IMGSIZ + 64,"OUT1.bmp");
		break;

		case 2:
			//양선형 보간
			///////////////////////////////////////////////////////////////////////////////////////////
			window(buf1,0);

			div = 4;

			//화면축소
			for(i=0;i<256;i=i+div){
				for(j=0;j<256;j=j+div){
					buf3[(i/div)][(j/div)] = buf1[i][j];
				}
			}
			for(i=0;i<256;i++){
				for(j=0;j<256;j++){
					buf2[i][j] = buf3[(i/div)][(j/div)];
				}
			}
			window(buf2,1);

			for(i=0;i<256;i++){
				for(j=0;j<256;j++){
					a = (float)i/div;
					b = (float)j/div;
					c = a - (i/div);
					d = b - (j/div);

					buf2[i][j] = ((float)1-c)*((float)1-d)*buf3[i/div][j/div]+
								 ((float)1-c)*d*buf3[i/div][j/div + 1]+
								 c*((float)1-d)*buf3[i/div + 1][j/div]+
								 c*d*buf3[i/div + 1][j/div + 1];
				}
			}
			window(buf2,2);
			//결과 출력

			Gray_Raw2Bmp(winbuf,IMGSIZ * 3, IMGSIZ + 64,"OUT2.bmp");
			///////////////////////////////////////////////////////////////////////////////////////////

		break;
		case 3:
			//원본을 출력함
			window(buf1,0);
			//중간값 대체한 영상줄이기
			for(i=2;i<256;i=i+3){
				for(j=2;j<256;j=j+3){
					sort_buf[0] = buf1[i][j];
					sort_buf[1] = buf1[i-1][j];
					sort_buf[2] = buf1[i-2][j];
					sort_buf[3] = buf1[i][j-1];
					sort_buf[4] = buf1[i][j-2];
					sort_buf[5] = buf1[i-1][j-2];
					sort_buf[6] = buf1[i-2][j-2];
					sort_buf[7] = buf1[i-2][j-1];
					sort_buf[8] = buf1[i-2][j-2];

					sort(sort_buf, 9);

					//임시저장
					buf3[i/3][j/3] = sort_buf[4];

				}
			}
			//그냥 확대
			for(i=0;i<256;i++){
				for(j=0;j<256;j++){
					buf2[i][j] = buf3[i/3][j/3];
				}
			}
			window(buf2,1);

			//window(buf2,1);
			//그냥 3배축소
			for(i=0;i<256;i++){
				for(j=0;j<256;j++){
					buf3[i/3][j/3] = buf1[i][j];
				}
			}
			//그냥 확대
			for(i=0;i<256;i++){
				for(j=0;j<256;j++){
					buf2[i][j] = buf3[i/3][j/3];
				}
			}
			window(buf2,2);
			Gray_Raw2Bmp(winbuf,IMGSIZ * 3, IMGSIZ + 64,"OUT3.bmp");
		break;

		case 4:
			memset(buf2, 0, IMGSIZ*IMGSIZ);
			for(k=0;k<5;k++){
				status = load_data(name_s[k], buf1, 'b', IMGSIZ, IMGSIZ);
				if( status != SUCCESS )
					exit(1);


				//2번째 출력
				if(k==2){
					window(buf1,1);
				}else if(k==4){
					window(buf1,0);
				}
				// 중첩
				for(i=0;i<256;i++){
					for(j=0;j<256;j++){
						buf2[i][j] = buf2[i][j] + buf1[i][j]/5;
					}
				}
			}
			window(buf2,2);

			Gray_Raw2Bmp(winbuf,IMGSIZ * 3, IMGSIZ + 64,"OUT4.bmp");
		break;
		}
	}
}
