#include <stdio.h>
#include <mem.h>
#include <stdlib.h>
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned long  LONG;
typedef unsigned char  BYTE;

#include            "fileio.h"
//#include            "kssm.c"


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
//////////////////////////////////////////////////////

int Gray_Raw2Bmp(unsigned char *pbuf, DWORD nWidth, DWORD nHeight, char *pBmpName)
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
       return 1;
}
////////////////////////////////////////////////////////////////////////
//			버퍼를 원하는 위도우에 복사
////////////////////////////////////////////////////////////////////////
void window(unsigned char *pbuf,unsigned char win){
	int i,j,pos=0;
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
////////////////////////////////////////////////////////////////////////
//			이큐함수
////////////////////////////////////////////////////////////////////////
void eq(unsigned char *pbuf1,unsigned char *pbuf2){

	unsigned int sum[256];
	int k,i,j,tmp,top=0;

	///////// 최기화  ////////////////
	for(k=0;k<256;k++){
		hist[k]=0;
	}

	///////// 히스트 만들기  ////////////////
    for(i=0; i<IMGSIZ; i++){
        for(j=0; j<IMGSIZ; j++){
        	hist[*((pbuf1 + (256*i)) + j)]++;
        }
    }

	sum[0] = hist[0];

	for(k=1;k<=255;k++){
		sum[k] = sum[k-1] + hist[k];
	}

	///////// 최대값 찾기  ////////////////
	top = 0;
	for(i=0;i<256;i++){
		if(top < sum[i]){
			top = sum[i];
		}
	}
	////////// 크기압축  /////////////
	for(i=0;i<256;i++)
		sum[i] = (int)(((float)sum[i])*((float)255)/top);

	//노멀라이즈 한 영상 저장
	for(i=0;i<256;i++){
		for(j=0;j<256;j++){
			tmp =sum[*((pbuf1 + (256*i)) + j)];
			*((pbuf2 + (256*i)) + j) = (unsigned char)tmp;
		}
	}

}
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
////////////////////////////////////////////////////////////////////////
//			메인 함수
////////////////////////////////////////////////////////////////////////
int main(void)
{
int         status;
int         i, j, cmd=1;
int rr;
float		avg;
char        fname[30];
unsigned char            mid[5]={0};

unsigned char            center;
unsigned char            buf1[IMGSIZ][IMGSIZ];
unsigned char            buf2[IMGSIZ][IMGSIZ];
unsigned char            buf3[IMGSIZ][IMGSIZ];
char filename[7][12]={"lena.raw","lena.raw","lena.raw","lena.raw","lena.raw","lena_s.raw","lena.raw"};

            memset(fname, 0, 30);
            memset(buf1, 0, IMGSIZ*IMGSIZ);
            memset(buf2, 0, IMGSIZ*IMGSIZ);
            memset(buf3, 0, IMGSIZ*IMGSIZ);
            memset(winbuf, 0, (IMGSIZ + 64)*(IMGSIZ*3));

            //printf("Enter input file name: ");
            //scanf("%s", fname);


for(rr=1;rr<7;rr++){
memset(buf1, 0, IMGSIZ*IMGSIZ);
memset(buf2, 0, IMGSIZ*IMGSIZ);
memset(buf3, 0, IMGSIZ*IMGSIZ);
memset(winbuf, 0, (IMGSIZ + 64)*(IMGSIZ*3));
            status = load_data(filename[rr], buf1, 'b', IMGSIZ, IMGSIZ);

            if( status != SUCCESS )
                        exit(1);

            switch(rr){
            case 1:
				//히스토그램 노멀라이즈;
				window(buf1,0);

				eq(buf1,buf2);
				window(buf2,1);

				memset(buf1, 255, IMGSIZ*IMGSIZ);
				histogram(buf2,buf1);
				window(buf1,2);

				Gray_Raw2Bmp(winbuf,IMGSIZ * 3, IMGSIZ + 64,"Equalization.bmp");
            break;

            case 2:
				//블러링
				window(buf1,0);
				for(i=1; i<IMGSIZ-1; i++) {
					for(j=1; j<IMGSIZ-1; j++) {
					avg = buf1[i-1][j-1]+buf1[i-1][j]+buf1[i-1][j+1]+
						  buf1[i][j-1]+buf1[i][j]+buf1[i][j+1]+
						  buf1[i+1][j-1]+buf1[i+1][j]+buf1[i+1][j+1];
					avg /= 9.;
					buf2[i][j] = (unsigned char)(avg+0.5);
					}
				}
				window(buf2,1);
				memset(buf1, 255, IMGSIZ*IMGSIZ);
				histogram(buf2,buf1);
				window(buf1,2);

				Gray_Raw2Bmp(winbuf,IMGSIZ * 3, IMGSIZ + 64,"Blurring.bmp");
            break;

            case 3:
				//가우시안 스무딩
            window(buf1,0);
            for(i=1; i<IMGSIZ-1; i++) {
				for(j=1; j<IMGSIZ-1; j++) {
				avg = buf1[i-1][j-1]+2*buf1[i-1][j]+buf1[i-1][j+1]+
					  2*buf1[i][j-1]+4*buf1[i][j]+2*buf1[i][j+1]+
					  buf1[i+1][j-1]+2*buf1[i+1][j]+buf1[i+1][j+1];
				avg /= 16.;
				buf2[i][j] = (unsigned char)(avg+0.5);
				}
			}
            window(buf2,1);
            memset(buf1, 255, IMGSIZ*IMGSIZ);
			histogram(buf2,buf1);
			window(buf1,2);

			Gray_Raw2Bmp(winbuf,IMGSIZ * 3, IMGSIZ + 64,"Smoothing.bmp");
            break;

            case 4:
				//샤프닝
            window(buf1,0);

        	for(i=1; i<IMGSIZ-1; i++) {
				for(j=1; j<IMGSIZ-1; j++) {

					avg = 9*buf1[i][j] - buf1[i-1][j] - buf1[i+1][j] - buf1[i][j+1] - buf1[i][j-1] -
							buf1[i-1][j+1] - buf1[i-1][j-1] - buf1[i+1][j-1] - buf1[i+1][j+1];

					if(0 > avg){
						buf2[i][j] = 0;
					}else if(255 < avg){
						buf2[i][j] = 255;
					}else{
						buf2[i][j] = (unsigned char)(avg);
					}
				}
			}
        	window(buf2,1);
			memset(buf1, 255, IMGSIZ*IMGSIZ);
			histogram(buf2,buf1);
			window(buf1,2);

			Gray_Raw2Bmp(winbuf,IMGSIZ * 3, IMGSIZ + 64,"Sharpening.bmp");
            break;

            case 5:
				//미디엄 필터링
				window(buf1,0);

				for(i=1; i<IMGSIZ-1; i++){
					for(j=1; j<IMGSIZ-1; j++){
						mid[0] = buf1[i][j];
						mid[1] = buf1[i-1][j];
						mid[2] = buf1[i+1][j];
						mid[3] = buf1[i][j-1];
						mid[4] = buf1[i][j+1];

						sort(mid, 5);

						buf2[i][j] = mid[2];
					}
				}
				window(buf2,1);
				memset(buf1, 255, IMGSIZ*IMGSIZ);
				histogram(buf2,buf1);
				window(buf1,2);

				Gray_Raw2Bmp(winbuf,IMGSIZ * 3, IMGSIZ + 64,"Median.bmp");
            break;

            case 6:
				//샤프닝 고주파
				window(buf1,0);

				for(i=1; i<IMGSIZ-1; i++) {
					for(j=1; j<IMGSIZ-1; j++) {

						avg = 8*buf1[i][j] - buf1[i-1][j] - buf1[i+1][j] - buf1[i][j+1] - buf1[i][j-1] -
								buf1[i-1][j+1] - buf1[i-1][j-1] - buf1[i+1][j-1] - buf1[i+1][j+1];

						if(0 > avg){
							buf2[i][j] = 0;
						}else if(255 < avg){
							buf2[i][j] = 255;
						}else{
							buf2[i][j] = (unsigned char)(avg);
						}
					}
				}
				for(i=1; i<IMGSIZ-1; i++) {
					for(j=1; j<IMGSIZ-1; j++) {
						buf2[i][j] = ~buf2[i][j];
					}
				}

				window(buf2,1);
				memset(buf1, 255, IMGSIZ*IMGSIZ);
				histogram(buf2,buf1);
				window(buf1,2);

				Gray_Raw2Bmp(winbuf,IMGSIZ * 3, IMGSIZ + 64,"Sharpening_H.bmp");
				break;
            }
}

}
