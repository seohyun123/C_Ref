#include <stdio.h>
#include <conio.h>
#include <graphics.h>//�׸��� �׸��� ���� �������
#include <bios.h> //bioskey()�Լ��� ����ϱ� ���� �������
#include <stdlib.h>//randommize()�� malloc()�Լ��� ����ϱ� ���� �������
#include <dos.h>//delay()�Լ��� ����ϱ� ���� �������

void main()
{
int gd=DETECT,gm; //���� ����
long ax[15],ay[15],bx[15],by[15];
long snx[100],sny[100];
int a,b,c,d=0,j=0,i=0,k,dx,dy,maxx,maxy,lx,si=30,s=23;
long int score=0,ca,cb,cax,cay,cbx,cby;
char *scor;
scor=(char *)malloc(10); //scor�� �޸� ���� ������
initgraph(&gd,&gm,"c:\\tc\\bgi"); //�׷��ȸ�� ����
//x,y�� �ִ밪 ����
maxx=getmaxx();
maxy=getmaxy();
setbkcolor(WHITE); //���� ���������
setcolor(8); //���ڻ��� �׷��̷� ��
rectangle(0,0,maxx,maxy);//�簢���׸���
//ȭ������ ��������׸��� APPLE�̶�� �۾� ��
setcolor(RED);
setfillstyle(SOLID_FILL,RED);
circle(10,10,5);
floodfill(10,10,RED);
setcolor(8);
outtextxy(20,10,"APPLE");
//������� �Ķ� ��ź�� �׸��� BOMB��°� ����
setcolor(BLUE);
setfillstyle(SOLID_FILL,BLUE);
circle(70,10,5);
floodfill(70,10,BLUE);
setcolor(8);
outtextxy(80,10,"BOMB");
setcolor(8);
outtextxy(120,10,"SCORE");
randomize();//�����ϰ� ���� ��� ���� ����
// ������ ���� �����Ƿ� ����� ��ź�� ���� ��ĥ�� �����Ƿ� �װ��� ��ġ�� �ʰ� �ϱ����� ���α׷���
for(a=0;a<20;a++)
{
b=random(maxx-100)+15;
c=random(maxy-60)+35;
if(a%2==1) //���̸� ����� �׸���
{
ax[j]=b;
ay[j]=c;
setcolor(RED);
setfillstyle(SOLID_FILL,RED);
circle(ax[j],ay[j],5);
floodfill(ax[j],ay[j],RED);
j ++;
}
else //�����̸� ��ź�� �׸���
{
bx[i]=b;
by[i]=c;
setcolor(BLUE);
setfillstyle(SOLID_FILL,BLUE);
circle(bx[i],by[i],5);
floodfill(bx[i],by[i],BLUE);
i++;
}
}
//���� �׸�
for(a=0;a<si;a++)
{
if(a==0){ snx[a]=maxx-30; sny[a]=maxy-300;}
else { snx[a]=snx[a-1];sny[a]=sny[a-1]+1;}
for(b=-1;b<=1;b++)
for(c=-1;c<=1;c++)
putpixel(snx[a]+b,sny[a]+c,2);
}
k=bioskey(0); //Ű���� ������� ȭ���� �������� �ִ� ������
//���� �����̴� �κ�
while(1) //���� �ݺ����� ����
{
if(kbhit()) k=bioskey(0);
if(k==283) break; //escŰ�� ġ�� ������ ������.
switch (k)
{
case 18432 : dx= 0; dy=-1; break; //up�� ������ ȭ���� ���� ������
case 18688 : dx=+1; dy=-1; break; //pg up�� ������ ȭ���� ������ ���� ������
case 20480 : dx= 0; dy=+1; break;// down�� ������ ȭ���� �Ʒ��� ������
case 20736 : dx=+1; dy=+1; break;//pg dn�� ������ ȭ���� ������ �Ʒ��� ������
case 19712 : dx=+1; dy= 0; break;//right�� ������ ȭ���� ���������� ������
case 18176 : dx=-1; dy=-1; break;//home�� ������ ȭ���� ���� ���� ������
case 19200 : dx=-1; dy= 0; break;//left�� ������ ȭ���� �������� ������
case 20224 : dx=-1; dy=+1; break;//end�� ������ ȭ���� ���� �Ʒ��� ������
}
//���� �����̴� �׸����� ǥ���ϱ����ؼ� ��ĭ ��ĭ ǥ���Ͽ��� ��������
for(a=si;a>0;a--)
{
snx[a]=snx[a-1];
sny[a]=sny[a-1];
}
snx[0]=snx[1]+dx;
sny[0]=sny[1]+dy;
for(b=-1;b<=1;b++)
for(c=-1;c<=1;c++)
{
putpixel(snx[0]+b,sny[0]+c,2);
putpixel(snx[si]+b,sny[si]+c,WHITE);
}
//���� ����� ������ ���� ���̸� ������� �Ͽ���
//���� ����� ������ ����� ������ó�� ǥ���ϱ� ���Ͽ� ����� ��������
for(h=0;h<10;h++)
{
if(snx[0]>ax[j]) cax=snx[0]-ax[j];
else cax=ax[j]-snx[0];
if(sny[0]>ay[j]) cay=sny[0]-ay[j];
else cay=ay[j]-sny[0];
if(snx[0]>bx[j]) cbx=snx[0]-bx[j];
else cbx=bx[j]-snx[0];
if(sny[0]>by[j]) cby=sny[0]-by[j];
else cby=by[j]-sny[0];
ca=(cax*cax)+(cay*cay);
cb=(cbx*cbx)+(cby*cby);
if(ca<=225)
{
score=score+10; //������ �÷���
si=si+5; //���� ���̸� ��� ��
s=s-3; //delay���� �ٿ��� �ӵ��� ��������
//����� ����
setcolor(WHITE);
setfillstyle(SOLID_FILL,WHITE);
circle(ax[j],ay[j],5);
floodfill(ax[j],ay[j],WHITE);
//����� �ִ� �ڸ� ���� �ʱ�ȭ
ax[j]=0;
ay[j]=0;
}
//���� ��ź�� ������ ��� d=1�̶� ǥ����
if(cb<=225) d=1;
}
//������ ȭ�鿡 ǥ����
sprintf(scor,"%d",score);
setcolor(8);
outtextxy(180,10,scor);
delay(s); //ȭ�鿡 delay���� �ɾ��༭ ���� �ӵ��� �����̴� ���� �׸��� ����� ��
if(score==100) //����� �ٸ��� ���
{
setcolor(8);
outtextxy(maxx/2-60, maxy/2+30,"WIN"); //ȭ�鿡 win����ϰ� ����
break;
getch();
}
//���� ���� ���� ��� ������ ����
if(snx[0]<=1||snx[0]>=maxx-1) d=1;
if(sny[0]<=1||sny[0]>=maxy-1) d=1;
if(d==1) //���� ��ź�� ��Ұų� ���� �������
{
setcolor(8);
outtextxy(maxx/2-50, maxy/2, "GAME OVER"); //ȭ�鿡 game over����ϰ� ����
break;
getch();
}
setcolor(WHITE);
outtextxy(180,10,scor);
}
getch();
closegraph();
}
