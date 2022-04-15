#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<iostream>
#include<algorithm>
#include<sys/timeb.h>
#include<Windows.h>
#include<opencv.hpp>
#include<opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#define IN_MAXSIZE 100
#define INCREASE 10
#define SElemType Single_Point 
#define FALSE 0
#define TURE 1
#define a 0     //a表示路 
#define b 1        //b表示墙 
using std::random_shuffle;
using namespace cv;
typedef struct 
{
    int x;    //行坐标 
    int y;      //列坐标 
    int direction = 1;     //方向
}SElemType;

typedef struct
{
    SElemType *base;
    SElemType *top;
    int stacksize;
}SqStack;

const int n = 47;
const int k = n / 2;
const int N = k * k;

// 迷宫的大小 n * n
int Init_Stack(SqStack *);     
int Push(SqStack *,SElemType);         
int Pop(SqStack *,SElemType *);  
int* randperm(int limit);
void DFSmaze(SElemType [],int [][n],int [][k]); 
void PrintMaze(int maze[][n]);
void draw(int x,int y,int flag);
bool MazePath(int maze[][n], SElemType start, SElemType end);
int Pass(SElemType pos);
bool StackEmpty(SqStack S);
void NextPos(SElemType* curpos, int direction);
void MakePrint(SElemType e, int print);
void PrintPath(SqStack *);

const int limit = 4; 
const int node_size = 12;
const int sleep_time = 5;
const int wait_key = 3;
int* choices;
int maze[n][n];
struct timeb seed;
// 长度 宽度 CV通道数
Mat mat(node_size * n, node_size * n, CV_8UC3);

int main()
{
    ftime(&seed);
    srand(seed.time * 1000 + seed.millitm);
    choices = (int*)malloc(limit * sizeof(int));
    choices = randperm(limit);

    int i,j;

    int smaze[k][k];
    // 结构体数组 
    SElemType r[(k)*(k)];   

    // 把所有点都先初始化为墙。
    for(i=0;i<n;i++)
        for(j=0;j<n;j++)
        {
            maze[i][j] = b;
        }

    // 先使得横坐标和纵坐标都为偶数的变为路。
    for(i=1;i<n-1;i=i+2)
        for(j=1;j<n-1;j=j+2)
        {
            maze[i][j] = a;
        }

    // 定义起始点和结束点。
    maze[1][0]=a;    
    maze[n-2][n-1]=a;

    // 存储结构体数组的横坐标和纵坐标。
    for(i=0;i<k*k;i++)   //结构体数组赋初值，索引从零开始
    {
        r[i].x = i/k;
        r[i].y = i%k;
    }
    // 数组中所有的初值全部置为FALSE，表示这些位置都没有被访问。
    for(i=0;i<k;i++) 
        for(j=0;j<k;j++)
            smaze[i][j] = FALSE;

    // DFS首先访问的节点。
    smaze[0][0] = TURE;  // 初始节点 
    DFSmaze(r,maze,smaze);
    PrintMaze(maze);     // 打印生成迷宫 
    namedWindow("Maze");
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (maze[i][j] == 1)
            {
                draw(i, j, 1);
            }
            else if(maze[i][j] == 0)
            {
                draw(i, j, 2);
            }
        }
    }
    imshow("Maze", mat);
    waitKey(1000);
    printf("Please enter 0 to continue.\n");
    SElemType start, end;
    start.x = 1; start.y = 0;
    end.x = n - 2; end.y = n - 1;
    MazePath(maze, start, end);
    system("pause");
    return 0;
}

void PrintPath(SqStack * s) {
    int count = 0;
    while (s->base < s->top)
    {
        printf("(%d, %d)-->", s->base->x, s->base->y);
        count++; s->base++;
        if (count % 8 == 0)
            printf("\n");
    }

}

void PrintMaze(int maze[][n]) {
    for(int i=0;i<n;i++)     // 打印生成迷宫 
    {    
        for(int j=0;j<n;j++)
        {
            printf("%d ",maze[i][j]);
        }
        printf("\n");
    }
}

int Init_Stack(SqStack *t)     //初始栈 
{
    t->base = (SElemType *)malloc(IN_MAXSIZE*sizeof(SElemType));
    if(!(t->base))
        return 0;
    t->top = t->base;
    t->stacksize = IN_MAXSIZE;
    return 1;   
}

int Push(SqStack *t,SElemType e)   //入栈 
{
    if(t->top-t->base>=t->stacksize)    
    {
        t->base = (SElemType *)realloc(t->base,(t->stacksize+INCREASE)*sizeof(SElemType));
        if(!t->base)
            return 0;
        t->top = t->base + t->stacksize;    
        t->stacksize += INCREASE; 
    }
    *(t->top++) = e;
    return 1;  
} 

int Pop(SqStack *t,SElemType *e)     //出栈 
{

    if(t->top==t->base)
        return 0;
    *e = *(--t->top);
    return 1;
}

/*
循环弹栈，弹出一个，则将其二维数组中的FALSE改为TURE表示该元素已经被访问了。
然后寻找其上下左右没有被访问的元素，找到一个，则将夹在两个蓝色圈中的墙打破，使其变成路，然后找到的未访问的元素入栈。
*/
void DFSmaze(SElemType r[N],int maze[n][n],int smaze[k][k]) //深度优先算法 
{
    SElemType e;             // 结构体变量e
    SqStack s;               // 栈里面的存储的为结构体
    if(Init_Stack(&s)==1)      // 初始栈 
        printf("OK\n"); 
    else
        printf("Fail\n");      
    Push(&s,r[0]);  //压栈r[0]
    printf("initializeing maze......\n");
    while(Pop(&s,&e))  //栈不空时，进行循环 
    {
        choices = randperm(limit);
        for(int i=0;i<limit;i++) {
            switch(choices[i]) {
                //0，1，2，3分别对应上右下左
                case 0: {
                    // 点不能为外墙并且该结点未被访问过
                    if(e.x - 1 >= 0 && smaze[e.x-1][e.y]!=TURE) // 上
                    {
                        // 该节点的状态变成访问过。
                        smaze[e.x-1][e.y] = TURE;
                        maze[2*e.x][2*e.y+1] = a; //打墙 
                        // 这里面r是一个一维结构体数组，每个结构分别存储X坐标和Y坐标。
                        Push(&s,r[(e.x-1)*k+e.y]);    
                    } 
                    break;
                }
                case 1: {
                    if(e.y + 1 < k  && smaze[e.x][e.y+1]!=TURE) // 右 
                    {
                        smaze[e.x][e.y+1] = TURE;      
                        maze[2*e.x+1][2*(e.y+1)] = a;    
                        Push(&s,r[(e.x)*k+e.y+1]);    
                    }
                    break;
                }
                case 2: {
                    if(e.x + 1 < k && smaze[e.x+1][e.y]!=TURE) // 下
                    {
                        smaze[e.x+1][e.y] = TURE;
                        maze[2*(e.x+1)][2*e.y+1] = a; 
                        Push(&s,r[(e.x+1)*k+e.y]);    
                    }
                    break;
                }
                case 3: {
                    if(e.y - 1 >= 0 && smaze[e.x][e.y-1]!=TURE) // 左 
                    {
                        smaze[e.x][e.y-1] = TURE;
                        maze[2*e.x+1][2*e.y] = a;
                        Push(&s,r[(e.x)*k+e.y-1]);    
                    }
                    break;
                }
            }
        }
    } 
} 

int* randperm(int limit)
{
    Sleep(sleep_time);
    ftime(&seed);
    srand(seed.time * 1000 + seed.millitm);
    int* numbers;
    numbers = (int*)malloc(limit * sizeof(int));
    for (int i = 0; i < limit; i++)
        numbers[i] = i;
    random_shuffle(numbers, numbers + limit);
    return numbers;
}

void draw(int x,int y,int flag)
{
    int red, green, black;
    if (flag == 1)
    {
        red = 0, green = 0, black = 0;
    }
    else if (flag == 2)
    {
        red = 0, green = 255, black = 255;
    }
    else if (flag == 3)
    {

        red = 255, green = 0, black = 0;
    }
    else if (flag == 4)
    {
        red = 0, green = 255, black = 0;
    }
    // 每个点像素为50?50
    for (int i = x * node_size; i < x * node_size + node_size; i++)
    {
        for (int j = y * node_size; j < y * node_size + node_size; j++)
        {
            mat.at<Vec3b>(i, j)[0] = black;
            mat.at<Vec3b>(i, j)[1] = green;
            mat.at<Vec3b>(i, j)[2] = red;
        }
    }
}

bool MazePath(int maze[][n], SElemType start, SElemType end)
{
    SqStack S;
    // SElemType* e;
    Init_Stack(&S);      // 初始栈 
    SElemType curpos = start;
    // 初始点变为红色
    draw(start.x, start.y, 3);
    imshow("Maze", mat);
    waitKey(wait_key);
    Push(&S, start);
    int direction;
    int curstep = 1;
    do {
        if(Pass(curpos)) {
            if(curpos.x == end.x && curpos.y == end.y) {
                PrintPath(&S);
                return true;
            }
            // 加入当前道路
            MakePrint(curpos, 2);
            NextPos(&curpos, curpos.direction);
            if(maze[curpos.x][curpos.y] != 0) {
                curpos.direction = curpos.direction % 4 + 1;
                continue;
            }
            Push(&S, curpos);
            draw(curpos.x, curpos.y, 3);
            imshow("Maze", mat);
            waitKey(wait_key);
            if(curpos.x == end.x && curpos.y == end.y) {
                PrintPath(&S);
                return true;
            }
            curstep++;
        } else {
            if(!StackEmpty(S)) {
                // 从栈中循环谈栈
                Pop(&S, &curpos);
                direction = Pass(curpos);
                if(direction) {
                    curpos.direction = direction - 1;
                }
                // 该块标识为不能通过
                MakePrint(curpos, 3);
                /////////////////////
                draw(curpos.x, curpos.y, 4);
                imshow("Maze", mat);
                waitKey(wait_key);
                while(!StackEmpty(S) && curpos.direction == 5) {
                    Pop(&S, &curpos);
                    // 该块标识为不能通过
                    MakePrint(curpos, 3);
                }
                if(curpos.direction <= 4 && Pass(curpos)) {
                    curpos.direction++;
                    Push(&S, curpos);

                    draw(curpos.x, curpos.y, 3);
                    imshow("Maze", mat);
                    waitKey(wait_key);

                    NextPos(&curpos, curpos.direction);
                    Push(&S, curpos);
                    MakePrint(curpos, 2);

                    draw(curpos.x, curpos.y, 3);
                    imshow("Maze", mat);
                    waitKey(wait_key);
                    if(curpos.x == end.x && curpos.y == end.y) {
                        PrintPath(&S);
                        return true;
                    }
                }
            }
        }
    } while(!StackEmpty(S));
    return false;
}

int Pass(SElemType pos)
{
    // 右
    if(pos.y+1 <= n-1 && maze[pos.x][pos.y+1] == 0) {
        printf("right\n");
        return 1;
    }

    // 下
    // attention
    if(pos.x+1 <= n-1 && maze[pos.x+1][pos.y] == 0) {
        printf("down\n");
        return 2;
    }

    // 左
    if(pos.y-1 >= 1 && maze[pos.x][pos.y-1] == 0) {
        printf("left\n");
        return 3;
    }

    // 上
    if(pos.x-1 >= 1 && maze[pos.x-1][pos.y] == 0) {
        printf("up\n");
        return 4;
    }

    printf("No way!\n");
    return 0;
}

bool StackEmpty(SqStack S) {
    if(S.top == S.base)
        return true;
    else
        return false;
}

// 1、2、3、4对应的搜索顺序分别为右、下、左、上
void NextPos(SElemType* curpos, int direction) 
{
    switch(direction) {
        case 1: {
            curpos->y++;
            break;
        }
        case 2: {
            curpos->x++;
            break;
        }
        case 3: {
            curpos->y--;
            break;
        }
        case 4: {
            curpos->x--;
            break;
        }
    }
}

// 留下标记
void MakePrint(SElemType e, int print)
{
    maze[e.x][e.y] = print;
}