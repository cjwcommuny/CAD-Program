#include "graphics.h"
#include "extgraph.h"
#include "genlib.h"
#include "simpio.h"
#include "conio.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <windows.h>
#include <olectl.h>
#include <mmsystem.h>
#include <wingdi.h>
#include <ole2.h>
#include <ocidl.h>
#include <winuser.h>

#define MAXOBJ 50
#define MAXPOINT 10
#define DEFAULT_COLOR "Black"
#define SELECT_COLOR "Red"

typedef enum {
    CURRENT_POINT_TIMER = 1
} timerID;

typedef enum {
    CURRENT_POINT_MSECONDS = 5
} mseconds;

typedef enum {
    NO_TYPE,
    TEXT,
    LINE,
    RECTANGLE,
    ELLIPSE,
    LOCUS,
} TwoDDrawType;

typedef enum {
    DRAW,
    OPERATE,
} ModeType;

struct Point {
    double x;
    double y;
    double z;
};

/*struct TwoDobj {
    //void (*draw)(void);
    //void (*rotate)(void);
    //void (*move)(void);
    string color;
    int DegreePointFreedom;
};*/

struct TwoDhasEdge {
    //struct TwoDobj *obj;
    struct Point *pointarray[MAXPOINT];
    struct Point *CenterPoint;
    int PointNum;
    bool (*RelationMatrix)[4]; //could be generalizied
};

struct obj {
    void *objPointer;
    int DrawType;
    string color;
    int DegreePointFreedom;
};

struct RegisterADT {
    struct obj *RegisterObj[MAXOBJ];
    int ObjNum;
    int ActiveOne;
};

const bool RectangleMatrix[4][4] = {
    {0, 1, 0, 1},
    {1, 0, 1, 0},
    {0, 1, 0, 1},
    {1, 0, 1, 0}
};
static int mode; 
static int DrawWhat; 
static bool isDrawing = FALSE;
static bool isOperating = FALSE;
static struct Point *CurrentPoint;
static struct RegisterADT *RegisterP;

void KeyboardEventProcess(int key,int event);
void CharEventProcess(char c);
void MouseEventProcess(int x, int y, int button, int event);
//void TimerEventProcess(int timerID);

void GetCurrentPoint(void);
static void ChooseButton(void (*left1)(void), void (*left2)(void)/*, 
                         void (*right1)(void), void (*right2)(void), 
                         void (*middle1)(void), void (*middle2)(void)*/, int button);
static void ChooseMode(void (*draw)(void), void (*operate)(void));
static void LeftMouseDownDraw(void);
static void LeftMouseUpDraw(void);
static void LeftMouseMoveDraw(void);
static void LeftMouseDownOperate(void);
static void LeftMouseUpOperate(void);
void RefreshDisplay(void);
void InitRegister(void);
void Register(void *objPt, int type);
//struct obj *FindActive(void)
void DrawTwoDhasEdge(void);
void DrawLineByPoint(struct Point *point1, struct Point *point2);
void RefreshAndDraw(void);
void RefreshAndDraw2(void);
void InitRectangle(void);
void DrawRectangle(double x, double y, double width, double height);
void DrawRectangle2(void);
void UpdateRectangle(void);
bool CheckMouse(struct obj *Obj);
bool CheckConvexPolygon(struct obj *Obj);
bool CompareArray(int *array1, int *array2, int length);
void SetMode(void);
void SetFunction(void);
void PlaceHolder(void);

void Main()
{
    SetWindowTitle("CAD Program");
    InitGraphics();
	//InitConsole();
    CurrentPoint = GetBlock(sizeof(struct Point));
    //startTimer(CURRENT_POINT_TIMER, CURRENT_POINT_MSECONDS);
    InitRegister();

	registerKeyboardEvent(KeyboardEventProcess);
	registerCharEvent(CharEventProcess);
	registerMouseEvent(MouseEventProcess);
	//registerTimerEvent(TimerEventProcess);
}

void KeyboardEventProcess(int key,int event)
{
    switch (event) {
        case KEY_DOWN:
            switch (key) {
                case VK_F1:
                    SetMode();
                    break;
                case VK_F2:
                    break;
                case VK_F3:
                    break;
            }
            break;
        case KEY_UP:
            break;
    }
}

void CharEventProcess(char c)
{

}
/*
void TimerEventProcess(int timerID)
{
    switch (timerID) {
        case CURRENT_POINT_TIMER:
            GetCurrentPoint();
            break;
    }
}
*/
void MouseEventProcess(int x, int y, int button, int event)
{
    CurrentPoint->x = ScaleXInches(x);
    CurrentPoint->y = GetWindowHeight() - ScaleXInches(y);
    //printf("coodinate: %f, %f\n", CurrentPoint->x, CurrentPoint->y);

    switch (event) {
        case BUTTON_DOWN:
            ChooseButton(LeftMouseDownDraw, LeftMouseDownOperate/*,,,,*/, button);
            break;
        case BUTTON_UP:
            ChooseButton(LeftMouseUpDraw, LeftMouseUpOperate/*,,,,*/, button);
            break;
        case MOUSEMOVE:
            //ChooseButton(LeftMouseMoveDraw/*,,,,,*/, button);
            ChooseMode(LeftMouseMoveDraw, PlaceHolder);
            break;
    }
}

static void ChooseButton(void (*left1)(void), void (*left2)(void)/*, 
                         void (*right1)(void), void (*right2)(void), 
                         void (*middle1)(void), void (*middle2)(void)*/, int button)
{
    //printf("TEST:ChooseButton\n");
    switch (button) {
        case LEFT_BUTTON:
            ChooseMode(left1, left2);
            break;
        case RIGHT_BUTTON:
            //ChooseMode(right1, right2);
            break;
        case MIDDLE_BUTTON:
            //ChooseMode(middle1, middle2);
            break;
    }
}

static void ChooseMode(void (*draw)(void), void (*operate)(void))
{
    switch (mode) {
        case DRAW:
            //printf("TEST:mode:DRAW\n");
            draw();
            break;
        case OPERATE:
            operate();
            break;
    }
}

static void LeftMouseDownDraw(void)
{
    //printf("TEST:LeftMouseDownDraw\n");
    isDrawing = TRUE;
    ChooseDrawWhat(/*,,*/InitRectangle/*,,*/);
    //printf("TEST:LeftMouseDownDraw over\n");
}

static void LeftMouseUpDraw(void)
{
    //printf("TEST:LeftMouseUpDraw\n");
    isDrawing = FALSE;
    RegisterP->ActiveOne = -1;
    //DrawWhat = NO_TYPE;
}

static void LeftMouseMoveDraw(void)
{
    //printf("TEST:LeftMouseMoveDraw\n");
    if (isDrawing) {
        //SetPenColor();
        RefreshAndDraw();
        //ChooseDrawWhat(/*,,*/DrawTwoDhasEdge/*,,*/);
    }
}

static void LeftMouseDownOperate(void)
{
    int i/*, j = 0*/;
    int objnum = RegisterP->ObjNum;
    bool flag = 0;
    //printf("objnum:%d\n", objnum);
    //int SelcectArray[MAXOBJ];
    //printf("TEST:LeftMouseDownOperate\n");
    isOperating = TRUE;
    for (i = 0; i < objnum; i++) {
        if (CheckMouse(RegisterP->RegisterObj[i])) {
            printf("TEST:here\n");
            //SelcectArray[j] = i;
            DrawWhat = RegisterP->RegisterObj[i]->DrawType;
            RegisterP->RegisterObj[i]->color = SELECT_COLOR;
            RegisterP->ActiveOne = i;
            ChooseDrawWhat(/*,,*/DrawTwoDhasEdge/*,,*/);
            flag = 1;
            //printf("TEST:here\n");
            break;
            //j++;
        }
    }
    //printf("TEST:here\n");
    if (!flag /*&& RegisterP->RegisterObj[i]->color == SELECT_COLOR*/) {
        //printf("TEST:here\n");
        for (i = 0; i < objnum; i++) {
            RegisterP->RegisterObj[i]->color = DEFAULT_COLOR;
        }
        RefreshAndDraw2();
    }
}

static void LeftMouseUpOperate(void)
{

}

void ChooseDrawWhat(/*void (*text)(void), 
                    void (*line)(void),*/
                    void (*rectangle)(void)/*, 
                    void (*ellipse)(void), 
                    void (*locus)(void)*/)
{
    //printf("TEST:ChooseDrawWhat\n");
    switch (DrawWhat) {
        case NO_TYPE:
            break;
        case TEXT:
            //text();
            break;
        case LINE:
            //line();
            break;
        case RECTANGLE:
            //printf("TEST:case:RECTANGLE\n");
            rectangle();
            break;
        case ELLIPSE:
            //ellipse();
            break;
        case LOCUS:
            //locus();
            break;
    }
}

void InitRectangle(void)
{
    //struct obj *obj;
    struct TwoDhasEdge *rectangle = GetBlock(sizeof(struct TwoDhasEdge));
    int i;
    //printf("TEST:InitRectangle\n");
    Register(rectangle, RECTANGLE);

    rectangle->PointNum = 4;
    for (i = 0; i < rectangle->PointNum; i++) {
        (rectangle->pointarray)[i] = GetBlock(sizeof(struct Point));
    }
    (rectangle->pointarray)[0]->x = CurrentPoint->x;
    (rectangle->pointarray)[0]->y = CurrentPoint->y;
    rectangle->RelationMatrix = RectangleMatrix;
    rectangle->CenterPoint = GetBlock(sizeof(struct Point));
    //printf("TEST: matrix0:%d\n", RectangleMatrix[0][0]);
    //printf("TEST: matrix1:%d\n", **(rectangle->RelationMatrix));
    
    //rectangle->obj = GetBlock(sizeof(struct TwoDobj));
    //rectangle->obj->color = DEFAULT_COLOR;
    //rectangle->obj->DegreePointFreedom = 2;
    //rectangle->obj->draw = 
    //rectangle->obj->rotate = 
    //rectangle->obj->move = 
    //printf("TEST: rectangle 0: %f, %f\n", (rectangle->pointarray)[0]->x, (rectangle->pointarray)[0]->y);
}

void GetCurrentPoint(void)
{
    CurrentPoint->x = GetMouseX();
    CurrentPoint->y = GetMouseY();
}

void RefreshDisplay(void)
{
    //double CurrentX, CurrentY;

    //CurrentX = GetCurrentX();
    //CurrentY = GetCurrentY();
    SetEraseMode(TRUE);
    StartFilledRegion(1);
    DrawRectangle(0, 0, GetWindowWidth(), GetWindowHeight()); //是否修改成整个屏幕的大小
    EndFilledRegion();
    MovePen(CurrentPoint->x, CurrentPoint->y);
    SetEraseMode(FALSE);
}

void RefreshAndDraw(void)
{
    int i , objnum = RegisterP->ObjNum, temp;
    struct obj *position;
    //printf("TEST:RefreshAndDraw\n");
    RefreshDisplay();

    temp = RegisterP->ActiveOne;
    for (i = 0; i < objnum; i++) {
        position = RegisterP->RegisterObj[i];
        DrawWhat = position->DrawType;
        RegisterP->ActiveOne = i;
        if (i == temp) {
            //printf("TEST: activeposition:%d\n", i);
            ChooseDrawWhat(/*,,*/DrawRectangle2/*,,*/);
        } else {
            ChooseDrawWhat(/*,,*/DrawTwoDhasEdge/*,,*/);
        }
    }
}

void RefreshAndDraw2(void)
{
    int i , objnum = RegisterP->ObjNum, temp;
    struct obj *position;
    printf("TEST:RefreshAndDraw\n");
    RefreshDisplay();

    temp = RegisterP->ActiveOne;
    for (i = 0; i < objnum; i++) {
        position = RegisterP->RegisterObj[i];
        DrawWhat = position->DrawType;
        RegisterP->ActiveOne = i;
        ChooseDrawWhat(/*,,*/DrawTwoDhasEdge/*,,*/);
    }
}

struct Point *CopyPoint(struct Point *point)
{
    struct Point *DestinationPoint = GetBlock(sizeof(struct Point));
    
    DestinationPoint->x = point->x;
    DestinationPoint->y = point->y;
    return DestinationPoint;
}

void Register(void *objPt, int type)
{
    struct obj *objP = GetBlock(sizeof(struct obj));//error prompt:too much object
    //printf("TEST:Register\n");
    objP->objPointer = objPt;
    objP->DrawType = type;
    objP->color = DEFAULT_COLOR;
    (RegisterP->RegisterObj)[RegisterP->ObjNum] = objP;
    //printf("TEST: register num:%d\n", RegisterP->ObjNum);
    //printf("TEST: DrawType: %d\n", (RegisterP->RegisterObj)[(RegisterP->ObjNum)-1]->DrawType);
    RegisterP->ActiveOne = RegisterP->ObjNum;
    RegisterP->ObjNum++;
}

void InitRegister(void)
{
    int i;

    RegisterP = GetBlock(sizeof(struct RegisterADT));
    for  (i = 0; i < MAXOBJ; i++) {
        (RegisterP->RegisterObj)[i] = NULL;//GetBlock(sizeof(struct obj));
    }
    RegisterP->ObjNum = 0;
}

/*struct obj *FindActive(void)
{
    int i;
    struct obj *result;

    for (i = 0; result = (RegisterADT->RegisterObj)[i]; i++) { //not false
        if (result->isActive) return result;
    }
}*/

void DrawTwoDhasEdge(void)
{
    int i, j, pointnum;
    struct TwoDhasEdge *obj = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;
    //string PenColor;
    
    //PenColor = GetPenColor();
    SetPenColor((RegisterP->RegisterObj)[RegisterP->ActiveOne]->color);
    obj->CenterPoint->x = 0;
    obj->CenterPoint->y = 0;
    pointnum = obj->PointNum; 
    //printf("TEST:%d\n", pointnum);
    for (i = 0; i < pointnum; i++) {
         obj->CenterPoint->x += obj->pointarray[i]->x;
         obj->CenterPoint->y += obj->pointarray[i]->y;
        for (j = i; j < pointnum; j++) {
            //printf("TEST:here\n");
            //printf("TEST:matrix: %d\n", obj->RelationMatrix[i][j]);
            if (obj->RelationMatrix[i][j]) DrawLineByPoint(obj->pointarray[i], obj->pointarray[j]);
        }
    }
    obj->CenterPoint->x /= obj->PointNum;
    obj->CenterPoint->y /= obj->PointNum;
    //printf("TEST:center point:%f, %f\n", obj->CenterPoint->x, obj->CenterPoint->y);
    //SetPenColor(PenColor);
}

void DrawLineByPoint(struct Point *point1, struct Point *point2)
{
    //printf("TEST:DrawLineByPoint\n");
    MovePen(point1->x, point1->y);
    DrawLine((point2->x)-(point1->x), (point2->y)-(point1->y));
}

void DrawRectangle(double x, double y, double width, double height)
{
    MovePen(x, y);
    DrawLine(width, 0);
    DrawLine(0, height);
    DrawLine(-width, 0);
    DrawLine(0, -height);
}

void DrawRectangle2(void)
{
    struct TwoDhasEdge *temp = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;
    //printf("TEST: pointnum:%d\n", ((struct TwoDhasEdge *)((RegisterP->RegisterObj[RegisterP->ActiveOne])->objPointer))->PointNum);
    UpdateRectangle();
    DrawTwoDhasEdge();
    //printf("TEST:\n");
    //TEST:getchar();
    //printf("Rectangle: 0: %f, %f, 1: %f, %f, 2: %f, %f, 3: %f, %f\n", temp->pointarray[0]->x, temp->pointarray[0]->y, temp->pointarray[1]->x, temp->pointarray[1]->y, 
    //temp->pointarray[2]->x, temp->pointarray[2]->y, temp->pointarray[3]->x, temp->pointarray[3]->y);
}

void UpdateRectangle(void)
{
    struct TwoDhasEdge *temp = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;
    //printf("TEST: pointnum: %d\n", temp->PointNum);
    temp->pointarray[2]->x = CurrentPoint->x;
    temp->pointarray[2]->y = CurrentPoint->y;
    temp->pointarray[1]->x = temp->pointarray[2]->x;
    temp->pointarray[1]->y = temp->pointarray[0]->y;
    temp->pointarray[3]->x = temp->pointarray[0]->x;
    temp->pointarray[3]->y = temp->pointarray[2]->y;
}

bool CheckMouse(struct obj *Obj)
{
    //printf("TEST:CheckMouse\n");
    switch (Obj->DrawType) {
        case TEXT:
            break;
        case LINE:
            break;
        case RECTANGLE:
            return CheckConvexPolygon(Obj);
        case ELLIPSE:
            break;
        case LOCUS:
            break;
    }
}

bool CheckConvexPolygon(struct obj *Obj)
{
    int i, j;
    double x = CurrentPoint->x;
    double y = CurrentPoint->y;
    struct TwoDhasEdge *polygon = Obj->objPointer;
    double cx = polygon->CenterPoint->x;
    double cy = polygon->CenterPoint->y;
    int isRegion[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //error:beyond array
    int isRegionCP[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    printf("TEST:CheckConvexPolygon\n");
    for (i = 0; i < polygon->PointNum; i++) {
        //printf("TEST:loop-\n");
        for (j = i; j < polygon->PointNum; j++) {
            //printf("TEST:loop0\n");
            if (polygon->RelationMatrix[i][j]) {
                //printf("TEST:loop1:%d, %d\n", i, j);
                if (polygon->pointarray[i]->x == polygon->pointarray[j]->x && y - polygon->pointarray[i]->y > 0) {
                        isRegion[i] = 1;
                } else if (polygon->pointarray[j]->y == polygon->pointarray[i]->y && x - polygon->pointarray[i]->x > 0) {
                        isRegion[i] = 1;
                } else if ((x - polygon->pointarray[i]->x)*(polygon->pointarray[j]->y - polygon->pointarray[i]->y)
                     < 
                    (y - polygon->pointarray[i]->y)*(polygon->pointarray[j]->x - polygon->pointarray[i]->x)) {
                        isRegion[i] = 1;
                        //printf("TEST:loop2:%d, %d\n", i, j);
                    }
            }
        }
    }
    //printf("TEST:here\n");
    for (i = 0; i < polygon->PointNum; i++) {
        //printf("TEST:loop-\n");
        for (j = i; j < polygon->PointNum; j++) {
            //printf("TEST:loop0\n");
            if (polygon->RelationMatrix[i][j]) {
                //printf("TEST:loop1:%d, %d\n", i, j);
                if (polygon->pointarray[i]->x == polygon->pointarray[j]->x && cy - polygon->pointarray[i]->y > 0) {
                        isRegionCP[i] = 1;
                } else if (polygon->pointarray[j]->y == polygon->pointarray[i]->y && cx - polygon->pointarray[i]->x > 0) {
                        isRegionCP[i] = 1;
                } else if ((cx - polygon->pointarray[i]->x)*(polygon->pointarray[j]->y - polygon->pointarray[i]->y)
                     < 
                    (cy - polygon->pointarray[i]->y)*(polygon->pointarray[j]->x - polygon->pointarray[i]->x)) {
                        isRegionCP[i] = 1;
                        //printf("TEST:loop2:%d, %d\n", i, j);
                    }
            }
        }
    }
    /*printf("isRegion:");
    for (i = 0; i < sizeof(isRegion)/sizeof(isRegion[0]); i++) {
        printf("%d ", isRegion[i]);
        printf("\n");
    }
    printf("isRegionCP:");
    for (i = 0; i < sizeof(isRegion)/sizeof(isRegion[0]); i++) {
        printf("%d ", isRegionCP[i]);
        printf("\n");
    }
    //printf("TEST:here\n");*/
    return CompareArray(isRegion, isRegionCP, sizeof(isRegion)/sizeof(isRegion[0]));
}

bool CompareArray(int *array1, int *array2, int length)
{
    bool flag = TRUE;
    int i;

    while (--length) {
        if (array1[length] != array2[length]) {
            flag = FALSE;
            break;
        }
    }
    return flag;
}

void SetMode(void)
{
    int input;

    InitConsole();
    printf("Which mode do you want? Choose the index from below.\n");
    printf("1: Draw object\n2: Type text\n3: Operate object\n");
    scanf("%d", &input); //wrong input prompt
    switch (input) {
        case 1:
            mode = DRAW;
            SetFunction();
            break;
        case 2:
            mode = DRAW; //draw and text are handled together
            SetFunction();
            break;
        case 3:
            mode = OPERATE;
            //SetFunction();
            break;
    }
    //how to close the console?
}

void SetFunction(void)
{
    int input;

    switch (mode) {
        case DRAW:
            printf("What object do you want to draw?\n");
            printf("1:Line\n2:Rectangle\n3:Ellipse\n4:Any curve\n");
            scanf("%d", &input); //wrong input prompt
            switch (input) {
                case 1:
                    DrawWhat = LINE;
                    break;
                case 2:
                    DrawWhat = RECTANGLE;
                    break;
                case 3:
                    DrawWhat = ELLIPSE;
                    break;
                case 4:
                    DrawWhat = LOCUS;
                    break;
            }
            break;
        /*case OPERATE:

            printf("What operation do you want?\n");
            printf("1:Move\n2:Rotate\n3:Change size\n");
            scanf("%d", &input);
            switch (input) {
                case 1:
                    break;
                case 2:
                    break;
                case 3:
                    break;
            }
            break;*/
    }
}

void PlaceHolder(void)
{
    //no operate
}