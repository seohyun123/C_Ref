#include            <stdio.h>
#include            "fileio.h"

#define IMGSIZ            256

main()
{
int         status;
int i=0,j,k,top=0;
char            fname[30];
int nrow = 256, ncol=256;
unsigned char            buf1[IMGSIZ][IMGSIZ];

int hist[256];
int sum[256];

///////////////////
//printf("Enter input file name: ");
//scanf("%s", fname);
memset(buf1, 0, IMGSIZ*IMGSIZ);
status = load_data("lena.raw", buf1, 'b', IMGSIZ, IMGSIZ);

/////////////////////////////////////////////////////////////

//����Ʈ  0 ���� �ʱ�ȭ
for(k=0;k<256;k++){
	hist[k]=0;
}
//����Ʈ ���� ī��Ʈ
for(i=0;i<nrow;i++){
	for(j=0;j<ncol;j++){
		hist[buf1[i][j]]++;
	}
}

//����Ʈ ���� ȭ�����
for(i=0;i<256;i++){
	printf("%d : %d\n",i,hist[i]);
}
//scanf("%s", &fname);

//����Ʈ ��ֶ����� ��....
sum[0] = hist[0];
for(k=1;k<=255;k++){
	sum[k] = sum[k-1] + hist[k];
}
for(k=1;k<=255;k++){
	sum[k] = (int)(((float)sum[k]*255)/(float)65535 + 0.5);
}

//��ֶ����� �� ���� ����
for(i=0;i<nrow;i++){
	for(j=0;j<ncol;j++){
		buf1[i][j] = sum[buf1[i][j]];
	}
}
save_data("lena22.raw", buf1, 'b', IMGSIZ, IMGSIZ);

// ������ ã��
for(k=0;k<256;k++){
	if(top < hist[k]){
		top = hist[k];
	}
}

printf("top : %d\n",top);

scanf("%s",&fname);
//���η��� ����
}
