#include <stdio.h>
#include <conio.h>
#include <graphics.h>//그림을 그리기 위한 헤더파일
#include <bios.h> //bioskey()함수를 사용하기 위한 헤더파일
#include <stdlib.h>//randommize()와 malloc()함수를 사용하기 위한 헤더파일
#include <dos.h>//delay()함수를 사용하기 위한 헤더파일

void main()
{
int gd=DETECT,gm; //변수 정의
long ax[15],ay[15],bx[15],by[15];
long snx[100],sny[100];
int a,b,c,d=0,j=0,i=0,k,dx,dy,maxx,maxy,lx,si=30,s=23;
long int score=0,ca,cb,cax,cay,cbx,cby;
char *scor;
scor=(char *)malloc(10); //scor의 메모리 값을 정의함
initgraph(&gd,&gm,"c:\\tc\\bgi"); //그래픽모드 시작
//x,y의 최대값 정의
maxx=getmaxx();
maxy=getmaxy();
setbkcolor(WHITE); //배경색 흰색으로함
setcolor(8); //글자색을 그레이로 함
rectangle(0,0,maxx,maxy);//사각형그리기
//화면위에 빨간사과그리고 APPLE이라고 글씨 씀
setcolor(RED);
setfillstyle(SOLID_FILL,RED);
circle(10,10,5);
floodfill(10,10,RED);
setcolor(8);
outtextxy(20,10,"APPLE");
//사과옆에 파란 폭탄을 그리고 BOMB라는고 적음
setcolor(BLUE);
setfillstyle(SOLID_FILL,BLUE);
circle(70,10,5);
floodfill(70,10,BLUE);
setcolor(8);
outtextxy(80,10,"BOMB");
setcolor(8);
outtextxy(120,10,"SCORE");
randomize();//랜덤하게 값을 잡기 위해 설정
// 랜덤한 값을 잡으므로 사과와 폭탄이 서로 겹칠수 있으므로 그것이 겹치지 않게 하기위한 프로그램임
for(a=0;a<20;a++)
{
b=random(maxx-100)+15;
c=random(maxy-60)+35;
if(a%2==1) //참이면 사과를 그린다
{
ax[j]=b;
ay[j]=c;
setcolor(RED);
setfillstyle(SOLID_FILL,RED);
circle(ax[j],ay[j],5);
floodfill(ax[j],ay[j],RED);
j ++;
}
else //거짓이면 폭탄을 그린다
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
//뱀을 그림
for(a=0;a<si;a++)
{
if(a==0){ snx[a]=maxx-30; sny[a]=maxy-300;}
else { snx[a]=snx[a-1];sny[a]=sny[a-1]+1;}
for(b=-1;b<=1;b++)
for(c=-1;c<=1;c++)
putpixel(snx[a]+b,sny[a]+c,2);
}
k=bioskey(0); //키값을 얻기전에 화면이 정지해져 있는 상태임
//뱀을 움직이는 부분
while(1) //무한 반복으로 돌림
{
if(kbhit()) k=bioskey(0);
if(k==283) break; //esc키를 치면 게임을 끝낸다.
switch (k)
{
case 18432 : dx= 0; dy=-1; break; //up을 누르면 화면의 위로 움직임
case 18688 : dx=+1; dy=-1; break; //pg up을 누르면 화면의 오른쪽 위로 움직임
case 20480 : dx= 0; dy=+1; break;// down를 누르면 화면의 아래로 움직임
case 20736 : dx=+1; dy=+1; break;//pg dn를 누르면 화면의 오른쪽 아래로 움직임
case 19712 : dx=+1; dy= 0; break;//right를 누르면 화면의 오른쪽으로 움직임
case 18176 : dx=-1; dy=-1; break;//home를 누르면 화면의 왼쪽 위로 움직임
case 19200 : dx=-1; dy= 0; break;//left를 누르면 화면의 왼쪽으로 움직임
case 20224 : dx=-1; dy=+1; break;//end를 누르면 화면의 왼쪽 아래로 움직임
}
//뱀을 움직이는 그림으로 표현하기위해서 한칸 한칸 표시하였고 지워나감
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
//뱀이 사과를 먹으면 뱀의 길이를 길어지게 하였음
//뱀이 사과에 닿으면 사과를 먹은것처럼 표현하기 위하여 사과를 지워버림
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
score=score+10; //점수를 올려줌
si=si+5; //뱀의 길이를 길게 함
s=s-3; //delay값을 줄여서 속도를 빠르게함
//사과를 지움
setcolor(WHITE);
setfillstyle(SOLID_FILL,WHITE);
circle(ax[j],ay[j],5);
floodfill(ax[j],ay[j],WHITE);
//사과가 있던 자리 값을 초기화
ax[j]=0;
ay[j]=0;
}
//뱀이 폭탄을 만나는 경우 d=1이라 표시함
if(cb<=225) d=1;
}
//점수를 화면에 표시함
sprintf(scor,"%d",score);
setcolor(8);
outtextxy(180,10,scor);
delay(s); //화면에 delay값을 걸어줘서 같은 속도로 움직이는 듯한 그림을 만들어 줌
if(score==100) //사과를 다먹은 경우
{
setcolor(8);
outtextxy(maxx/2-60, maxy/2+30,"WIN"); //화면에 win출력하고 끝냄
break;
getch();
}
//뱀이 벽에 닿은 경우 끝내기 위함
if(snx[0]<=1||snx[0]>=maxx-1) d=1;
if(sny[0]<=1||sny[0]>=maxy-1) d=1;
if(d==1) //뱀이 폭탄을 닿았거나 벽에 닿았을때
{
setcolor(8);
outtextxy(maxx/2-50, maxy/2, "GAME OVER"); //화면에 game over출력하고 끝냄
break;
getch();
}
setcolor(WHITE);
outtextxy(180,10,scor);
}
getch();
closegraph();
}
