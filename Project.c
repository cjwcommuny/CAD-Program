#include "graphics.h"
#include "extgraph.h"
#include "genlib.h"
#include "simpio.h"
#include "conio.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

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
#define POINT_COLOR "Magenta"
#define DL 0.05
#define POINT_R 0.05
#define ROTATE_POINT_RANGE 0.1
#define PI 3.14159
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
    //struct Point *CenterPoint;
    int PointNum;
    bool *RelationMatrix; //could be generalizied
};

struct obj {
    void *objPointer;
    int DrawType;
    string color;
    int DegreePointFreedom;
    struct Point *RotatePoint;
    struct Point *CenterPoint;
};

struct RegisterADT {
    struct obj *RegisterObj[MAXOBJ];
    int ObjNum;
    int ActiveOne;
};

/*const bool RectangleMatrix[4][4] = {
    {0, 1, 0, 1},
    {1, 0, 1, 0},
    {0, 1, 0, 1},
    {1, 0, 1, 0}
};*/
const bool RectangleMatrix[] ={
    0, 1, 0, 1,
    1, 0, 1, 0,
    0, 1, 0, 1,
    1, 0, 1, 0
};
const bool LineMatrix[] = {
    0, 1,
    1, 0
};
static int mode; 
static int DrawWhat; 
static bool isDrawing = FALSE;
static bool isOperating = FALSE;
static bool isRotating = FALSE;
static struct Point *CurrentPoint, *PreviousPoint;
static struct RegisterADT *RegisterP;
static int RedNum = 0;
static double angle;

void KeyboardEventProcess(int key,int event);
void CharEventProcess(char c);
void MouseEventProcess(int x, int y, int button, int event);
//void TimerEventProcess(int timerID);

void GetCurrentPoint(void);
static void ChooseButton(void (*left1)(void), void (*left2)(void)/*, 
                         void (*right1)(void), void (*right2)(void), 
                         void (*middle1)(void), void (*middle2)(void)*/, int button);
void ChooseDrawWhat(void (*text)(void), 
                    void (*line)(void),
                    void (*rectangle)(void), 
                    void (*ellipse)(void), 
                    void (*locus)(void));
static void ChooseMode(void (*draw)(void), void (*operate)(void));
static void LeftMouseDownDraw(void);
static void LeftMouseUpDraw(void);
static void LeftMouseMoveDraw(void);
static void LeftMouseDownOperate(void);
static void LeftMouseUpOperate(void);
static void LeftMouseMoveOperate(void);
void RefreshDisplay(void);
void InitRegister(void);
void Register(void *objPt, int type);
//struct obj *FindActive(void)
void DrawTwoDhasEdge(void);
void DrawLineByPoint(struct Point *point1, struct Point *point2);
//void RefreshAndDraw(void);
void RefreshAndDraw(void);
void InitRectangle(void);
void DrawRectangle(double x, double y, double width, double height);
void GetRectangleShape(void);
void UpdateRectangle(void);
bool CheckMouse(struct obj *Obj);
bool CheckConvexPolygon(struct obj *Obj);
bool CompareArray(int *array1, int *array2, int length);
void SetMode(void);
void SetFunction(void);
void PlaceHolder(void);
void MoveObj(struct obj *Obj, double dx, double dy);
void DrawRotatePoint(struct obj *Obj);
void CreateRotatePointForConvexPolygon(struct obj *Obj);
void DrawRotatePointForConvexPolygon(struct obj *Obj);
//void CreateRotatePointForRectangle(struct obj *Obj);
void DrawDottedLine(struct Point *point1, struct Point *point2);
void DrawPoint(double x, double y);
void DrawCenteredCircle(double x, double y, double r);
void test(void);
bool CheckRotate(void);
bool InsideRotatePoint(struct obj *Obj);
void rotate(double x1, double y1, double x2, double y2);
void RotatePolygon(void);
void InitLine(void);
void GetLineShape(void);

void Main()
{
    SetWindowTitle("CAD Program");
    InitGraphics();
	//InitConsole();
    //test();
    CurrentPoint = GetBlock(sizeof(struct Point));
    PreviousPoint = GetBlock(sizeof(struct Point));
    PreviousPoint->x = 0;
    PreviousPoint->y = 0;
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
    PreviousPoint->x = CurrentPoint->x;
    PreviousPoint->y = CurrentPoint->y;
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
            ChooseMode(LeftMouseMoveDraw, LeftMouseMoveOperate);
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
    ChooseDrawWhat(PlaceHolder, InitLine, InitRectangle, PlaceHolder, PlaceHolder);
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
        GetShape();
        RefreshAndDraw();
        //ChooseDrawWhat(/*,,*/DrawTwoDhasEdge/*,,*/);
    }
}

static void LeftMouseDownOperate(void)
{
    int i, j;
    int objnum = RegisterP->ObjNum;
    bool SelectFlag = 0;
    //printf("objnum:%d\n", objnum);
    //int SelcectArray[MAXOBJ];
    //printf("TEST:LeftMouseDownOperate\n");
    if (CheckRotate()) {
        isRotating = TRUE;
    } else {
        isOperating = TRUE;
        for (i = 0; i < objnum; i++) {
            if (CheckMouse(RegisterP->RegisterObj[i])) {
                //printf("TEST:here\n");
                //SelcectArray[j] = i;
                DrawWhat = RegisterP->RegisterObj[i]->DrawType;
                RegisterP->ActiveOne = i;
                if (RegisterP->RegisterObj[i]->color == SELECT_COLOR) {
                    RegisterP->RegisterObj[i]->color = DEFAULT_COLOR;
                    RedNum--;
                    RegisterP->ActiveOne = -1;
                } else {
                    if (RedNum >= 1) {
                        for (j = 0; j < objnum; j++) {
                            RegisterP->RegisterObj[j]->color = DEFAULT_COLOR;
                        }
                    }
                    RegisterP->RegisterObj[i]->color = SELECT_COLOR;
                    DrawRotatePoint(RegisterP->RegisterObj[i]);
                    RedNum++;
                    RegisterP->ActiveOne = i;
                }
                RefreshAndDraw();
                //ChooseDrawWhat(/*,,*/DrawTwoDhasEdge/*,,*/);
                SelectFlag = 1;
                
                //printf("TEST:here\n");
                break;
                //j++;
            }
        }
        //printf("TEST:here\n");
        if (!SelectFlag/*&& RegisterP->RegisterObj[i]->color == SELECT_COLOR*/) {
            //printf("TEST:here\n");
            for (i = 0; i < objnum; i++) {
                RegisterP->RegisterObj[i]->color = DEFAULT_COLOR;
            }
            RefreshAndDraw();
        }
    }
}

static void LeftMouseUpOperate(void)
{
    isOperating = FALSE;
    isRotating = FALSE;
}

static void LeftMouseMoveOperate(void)
{
    int i;
    double x0 = PreviousPoint->x;
    double y0 = PreviousPoint->y;
    double x1 = CurrentPoint->x;
    double y1 = CurrentPoint->y;

    if (isOperating) {
        for (i = 0; i < RegisterP->ObjNum; i++) {
            if (RegisterP->RegisterObj[i]->color == SELECT_COLOR) MoveObj(RegisterP->RegisterObj[i], x1-x0, y1-y0);
        }
    } else if (isRotating) {
        rotate(x0, y0, x1, y1);
    }
}

void MoveObj(struct obj *Obj, double dx, double dy)
{
    int i;
    //printf("TEST:dx, dy = %f, %f", dx, dy);
    switch (Obj->DrawType) {
        case TEXT:
            break;
        case LINE:
            break;
        case RECTANGLE:
            /*for (i = 0; i <4; i++) {
                printf("x0, y0 = %f, %f\n", ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->x, 
                                            ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->y);
            }*/
            for (i = 0; i < ((struct TwoDhasEdge *) Obj->objPointer)->PointNum; i++) {
                ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->x += dx;
                ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->y += dy;
            }
            /*for (i = 0; i <4; i++) {
                printf("x1, y1 = %f, %f\n", ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->x, 
                                            ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->y);
            }*/
            break;
        case ELLIPSE:
            break;
    }
    RefreshAndDraw();
}

void ChooseDrawWhat(void (*text)(void), 
                    void (*line)(void),
                    void (*rectangle)(void), 
                    void (*ellipse)(void), 
                    void (*locus)(void))
{
    //printf("TEST:ChooseDrawWhat\n");
    switch (DrawWhat) {
        case NO_TYPE:
            break;
        case TEXT:
            PlaceHolder();
            //text();
            break;
        case LINE:
            line();
            break;
        case RECTANGLE:
            //printf("TEST:case:RECTANGLE\n");
            rectangle();
            break;
        case ELLIPSE:
            PlaceHolder();
            //ellipse();
            break;
        case LOCUS:
            PlaceHolder();
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
    //rectangle->CenterPoint = GetBlock(sizeof(struct Point));
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

/*void GetCurrentPoint(void)
{
    CurrentPoint->x = GetMouseX();
    CurrentPoint->y = GetMouseY();
}*/

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

/*void RefreshAndDraw(void)
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
            ChooseDrawWhat(PlaceHolder, GetLineShape, GetRectangleShape, PlaceHolder, PlaceHolder);
        } else {
            ChooseDrawWhat(PlaceHolder, DrawTwoDhasEdge, DrawTwoDhasEdge, PlaceHolder, PlaceHolder);
        }
    }
}*/

void RefreshAndDraw(void)
{
    int i , objnum = RegisterP->ObjNum, temp;
    struct obj *position;
    string PenColor;
    int activeone = RegisterP->ActiveOne;
    //printf("TEST:RefreshAndDraw\n");
    RefreshDisplay();

    PenColor = GetPenColor();
    temp = RegisterP->ActiveOne;
    for (i = 0; i < objnum; i++) {
        position = RegisterP->RegisterObj[i];
        SetPenColor(position->color);
        DrawWhat = position->DrawType;
        RegisterP->ActiveOne = i;
        if (position->color == SELECT_COLOR) {
            DrawRotatePoint(position);
        }
        ChooseDrawWhat(PlaceHolder, DrawTwoDhasEdge, DrawTwoDhasEdge, PlaceHolder, PlaceHolder);
    }
    SetPenColor(PenColor);
    RegisterP->ActiveOne = activeone;
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
    objP->RotatePoint = GetBlock(sizeof(struct Point));
    objP->CenterPoint = GetBlock(sizeof(struct Point));
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
    struct obj *Obj = (RegisterP->RegisterObj)[RegisterP->ActiveOne];
    //string PenColor;
    
    //PenColor = GetPenColor();
    SetPenColor((RegisterP->RegisterObj)[RegisterP->ActiveOne]->color);
    Obj->CenterPoint->x = 0;
    Obj->CenterPoint->y = 0;
    pointnum = obj->PointNum; 
    //printf("TEST:%d\n", pointnum);
    for (i = 0; i < pointnum; i++) {
         Obj->CenterPoint->x += obj->pointarray[i]->x;
         Obj->CenterPoint->y += obj->pointarray[i]->y;
        for (j = i; j < pointnum; j++) {
            //printf("TEST:here\n");
            //printf("TEST:matrix: %d\n", obj->RelationMatrix[i][j]);
            if (*(obj->RelationMatrix +pointnum*i+j)) DrawLineByPoint(obj->pointarray[i], obj->pointarray[j]);
        }
    }
    Obj->CenterPoint->x /= obj->PointNum;
    Obj->CenterPoint->y /= obj->PointNum;
    CreateRotatePointForConvexPolygon((RegisterP->RegisterObj)[RegisterP->ActiveOne]);
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

void GetRectangleShape(void)
{
    //struct TwoDhasEdge *temp = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;
    struct TwoDhasEdge *temp = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;
    //printf("TEST: pointnum: %d\n", temp->PointNum);
    temp->pointarray[2]->x = CurrentPoint->x;
    temp->pointarray[2]->y = CurrentPoint->y;
    temp->pointarray[1]->x = temp->pointarray[2]->x;
    temp->pointarray[1]->y = temp->pointarray[0]->y;
    temp->pointarray[3]->x = temp->pointarray[0]->x;
    temp->pointarray[3]->y = temp->pointarray[2]->y;
    //printf("TEST: pointnum:%d\n", ((struct TwoDhasEdge *)((RegisterP->RegisterObj[RegisterP->ActiveOne])->objPointer))->PointNum);
    //UpdateRectangle();
    //DrawTwoDhasEdge();
    //printf("TEST:\n");
    //TEST:getchar();
    //printf("Rectangle: 0: %f, %f, 1: %f, %f, 2: %f, %f, 3: %f, %f\n", temp->pointarray[0]->x, temp->pointarray[0]->y, temp->pointarray[1]->x, temp->pointarray[1]->y, 
    //temp->pointarray[2]->x, temp->pointarray[2]->y, temp->pointarray[3]->x, temp->pointarray[3]->y);
}

/*void UpdateRectangle(void)
{
    struct TwoDhasEdge *temp = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;
    //printf("TEST: pointnum: %d\n", temp->PointNum);
    temp->pointarray[2]->x = CurrentPoint->x;
    temp->pointarray[2]->y = CurrentPoint->y;
    temp->pointarray[1]->x = temp->pointarray[2]->x;
    temp->pointarray[1]->y = temp->pointarray[0]->y;
    temp->pointarray[3]->x = temp->pointarray[0]->x;
    temp->pointarray[3]->y = temp->pointarray[2]->y;
}*/

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
    int i, j, k = 0;
    double x = CurrentPoint->x;
    double y = CurrentPoint->y;
    struct TwoDhasEdge *polygon = Obj->objPointer;
    double cx = Obj->CenterPoint->x;
    double cy = Obj->CenterPoint->y;
    int isRegion[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //error:beyond array
    int isRegionCP[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int pointnum = polygon->PointNum;
    //printf("TEST:centerpoint: %f, %f\n", cx, cy);
    //printf("TEST:CheckConvexPolygon\n");
    for (i = 0; i < polygon->PointNum; i++) {
        //printf("TEST:loop-\n");
        for (j = i; j < polygon->PointNum; j++) {
            //printf("TEST:loop0\n");
            if (*(polygon->RelationMatrix +pointnum*i+j)) {
                //printf("TEST:loop:%d, %d\n", i, j);
                //printf("TEST:x: %f, y %f\n", polygon->pointarray[i]->x, polygon->pointarray[i]->y);
                if (polygon->pointarray[i]->x == polygon->pointarray[j]->x) {
                    if (x > polygon->pointarray[i]->x) isRegion[k] = 1;
                    k++;
                } else if (polygon->pointarray[i]->y == polygon->pointarray[j]->y) {
                    if (y > polygon->pointarray[i]->y) isRegion[k] = 1;
                    k++;
                } else if ((x - polygon->pointarray[i]->x)*(polygon->pointarray[j]->y - polygon->pointarray[i]->y)/(polygon->pointarray[j]->x - polygon->pointarray[i]->x)
                     < 
                    (y - polygon->pointarray[i]->y)) {
                        isRegion[k++] = 1;
                        //printf("TEST:loop2:%d, %d\n", i, j);
                } else {
                    k++;
                }
            }
        }
    }
    k = 0;
    //printf("TEST:here\n");
    for (i = 0; i < polygon->PointNum; i++) {
        //printf("TEST:loop-\n");
        for (j = i; j < polygon->PointNum; j++) {
            //printf("TEST:loop0\n");
            if (*(polygon->RelationMatrix +pointnum*i+j)) {
                //printf("TEST:loop1:%d, %d\n", i, j);
                if (polygon->pointarray[i]->x == polygon->pointarray[j]->x) {
                    if (cx > polygon->pointarray[i]->x) isRegionCP[k] = 1;
                    k++;
                } else if (polygon->pointarray[i]->y == polygon->pointarray[j]->y) {
                    if (cy > polygon->pointarray[i]->y) isRegionCP[k] = 1;
                    k++;
                } else if ((cx - polygon->pointarray[i]->x)*(polygon->pointarray[j]->y - polygon->pointarray[i]->y)/(polygon->pointarray[j]->x - polygon->pointarray[i]->x)
                     < 
                    (cy - polygon->pointarray[i]->y)) {
                        isRegionCP[k++] = 1;
                        //printf("TEST:loop2:%d, %d\n", i, j);
                } else {
                    k++;
                }
            }
        }
    }
    /*printf("isRegion:\n");
    for (i = 0; i < sizeof(isRegion)/sizeof(isRegion[0]); i++) {
        printf("%d ", isRegion[i]);
        printf("\n");
    }
    printf("isRegionCP:\n");
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

    for (i = 0; i < length; i++) {
        if (array1[i] != array2[i]) {
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

void DrawRotatePoint(struct obj *Obj)
{
    switch (Obj->DrawType) {
        case LINE:
            break;
        case RECTANGLE:
            DrawRotatePointForConvexPolygon(Obj);
            break;
        case ELLIPSE:
            break;
    }
}

void CreateRotatePointForConvexPolygon(struct obj *Obj)
{
    struct TwoDhasEdge *temp = Obj->objPointer;
    struct Point *MiddlePoint = GetBlock(sizeof(struct Point));

    MiddlePoint->x = (temp->pointarray[1]->x + temp->pointarray[2]->x)/2;
    MiddlePoint->y = (temp->pointarray[1]->y + temp->pointarray[2]->y)/2;
    Obj->RotatePoint->x = 1.5*(MiddlePoint->x - Obj->CenterPoint->x) + Obj->CenterPoint->x;
    Obj->RotatePoint->y = 1.5*(MiddlePoint->y - Obj->CenterPoint->y) + Obj->CenterPoint->y;
}

void DrawRotatePointForConvexPolygon(struct obj *Obj)
{
    string PenColor;
    //printf("TEST:here\n");
    PenColor = GetPenColor();
    SetPenColor(Obj->color);
    //printf("TEST:window: %f, %f\n", GetWindowWidth(), GetWindowHeight());
    //printf("TEST:%s\n", GetPenColor());
    DrawDottedLine(Obj->CenterPoint, Obj->RotatePoint);
    SetPenColor(POINT_COLOR);
    DrawPoint(GetCurrentX(), GetCurrentY());
    SetPenColor(PenColor);
}

/*void CreateRotatePointForRectangle(struct obj *Obj)
{
    struct TwoDhasEdge *temp = Obj->objPointer;
    struct Point *MiddlePoint = GetBlock(sizeof(struct Point));
    struct Point *RotatePoint = GetBlock(sizeof(struct Point));
    string PenColor;
    double len;
    //printf("TEST:here\n");
    //printf("TEST:Obj:\n");
    MiddlePoint->x = (temp->pointarray[2]->x + temp->pointarray[3]->x)/2;
    MiddlePoint->y = (temp->pointarray[2]->y + temp->pointarray[3]->y)/2;
    //printf("TEST:here\n");
    //len = sqrt(pow(temp->CenterPoint->x - MiddlePoint->x, 2) + pow(temp->CenterPoint->y - MiddlePoint->y, 2));
    RotatePoint->x = 1.5*(MiddlePoint->x - temp->CenterPoint->x) + temp->CenterPoint->x;
    RotatePoint->y = 1.5*(MiddlePoint->y - temp->CenterPoint->y) + temp->CenterPoint->y;
    //printf("TEST:here\n");
    DrawDottedLine(temp->CenterPoint, RotatePoint);
    //printf("TEST:here\n");
    PenColor = GetPenColor();
    SetPenColor(POINT_COLOR);
    DrawPoint(GetCurrentX(), GetCurrentY());
    SetPenColor(PenColor);
}*/

void DrawDottedLine(struct Point *point1, struct Point *point2)
{
    double len = sqrt(pow(point2->x - point1->x, 2) + pow(point2->y - point1->y, 2));
    //printf("TEST:X: %f, Y %f\n", point2->x - point1->x, point2->y - point1->y);
    double length = 0;
    double cos0 = (point2->x - point1->x)/len;
    double sin0 = (point2->y - point1->y)/len;
    bool flag = FALSE;
    bool EraseMode = GetEraseMode();
    //printf("TEST:EraseMode1: %d\n", EraseMode);
    //printf("here\n");
    //printf("TEST:color:%s\n", GetPenColor());
    MovePen(point1->x, point1->y);
    //printf("TEST:1: %f, %f\n2: %f, %f\n", point1->x, point1->y, point2->x, point2->y);
    //printf("TEST:point:%f, %f\n", point1->x, point1->y);
    //printf("here\n");
    //printf("TEST: %f\n", len);
    //SetPenColor("Red");
    SetEraseMode(FALSE);
    while (length <= len) {
        //printf("here\n");
        //DrawLine(1, 1);
        DrawLine(DL*cos0, DL*sin0);
        //printf("TEST:cos:%f , sin:%f\n", cos0, sin0);
        flag = !flag;
        SetEraseMode(flag);
        length += DL;
    }
    SetEraseMode(EraseMode);
    //printf("TEST:EraseMode2: %d\n", EraseMode);
    //printf("here\n");
}

void DrawPoint(double x, double y)
{
    DrawCenteredCircle(x, y, POINT_R);
}

void DrawCenteredCircle(double x, double y, double r)
{
    MovePen(x + r, y);
    DrawArc(r, 0.0, 360.0);
}

void test(void)
{
    struct Point *point1 = GetBlock(sizeof(struct Point));
    struct Point *point2 = GetBlock(sizeof(struct Point));
    point1->x = 0;
    point1->y = 0;
    point2->x = 2;
    point2->y = 1;
    DrawDottedLine(point1, point2);
}

bool CheckRotate(void) 
{
    int i;
    int objnum = RegisterP->ObjNum;

    for (i = 0; i < objnum; i++) {
        if (RegisterP->RegisterObj[i]->color == SELECT_COLOR) {
            if (InsideRotatePoint(RegisterP->RegisterObj[i])) {
                RegisterP->ActiveOne = i;
                //printf("TEST:rotate: %d\n", i);
                return TRUE;
            }
        }
    }
    return FALSE;
}

bool InsideRotatePoint(struct obj *Obj) 
{
    double x = Obj->RotatePoint->x;
    double y = Obj->RotatePoint->y;
    double r2 = pow(ROTATE_POINT_RANGE, 2);

    if (pow(CurrentPoint->x - x, 2) + pow(CurrentPoint->y - y, 2) <= r2) return TRUE;
    else return FALSE;
}

void rotate(double x1, double y1, double x2, double y2) //coule be no arguments
{
    double xc = RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->x;
    double yc = RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->y;
    //double r1, r2, l;
    //double tempX, tempY;
    //int i;

    //r1 = sqrt(pow(x1-xc, 2) + pow(y1-yc, 2));
    //r2 = sqrt(pow(x2-xc, 2) + pow(y2-yc, 2));
    //l = sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));
    //cos0 = (r1*r1 + r2*r2 -l*l) / (2*r1*r2);
    angle = atan((y2-yc)/(x2-xc)) - atan((y1-yc)/(x1-xc));
    if (x1 - xc < 0) {
        angle -= PI;
    }
    if (x2 - xc < 0) {
        angle += PI;
    }
    ChooseDrawWhat(PlaceHolder, PlaceHolder, RotatePolygon, PlaceHolder, PlaceHolder);
    RefreshAndDraw();
}

void RotatePolygon(void) //also works when obj is TwoDhasEdge
{
    struct TwoDhasEdge *Polygon = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;
    int pointnum = Polygon->PointNum;
    int i;
    double xc = RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->x;
    double yc = RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->y;
    //printf("TEST:rotate:%d\n", RegisterP->ActiveOne);
    for (i = 0; i < pointnum; i++) {
        double tempX = Polygon->pointarray[i]->x;
        double tempY = Polygon->pointarray[i]->y;

        Polygon->pointarray[i]->x = (tempX - xc) * cos(angle) + (yc - tempY) * sin(angle) + xc;
        Polygon->pointarray[i]->y = (tempX - xc) * sin(angle) + (tempY - yc) * cos(angle) + yc;
    }
}

void InitLine(void) // can be merged with InitRectangle()
{
    struct TwoDhasEdge *line = GetBlock(sizeof(struct TwoDhasEdge));

    Register(line, LINE);
    line->PointNum = 2;
    (line->pointarray)[0] = GetBlock(sizeof(struct Point));
    (line->pointarray)[1] = GetBlock(sizeof(struct Point));
    (line->pointarray)[0]->x = CurrentPoint->x;
    (line->pointarray)[0]->y = CurrentPoint->y;
    line->RelationMatrix = LineMatrix;
}

void GetShape(void)
{
    ChooseDrawWhat(PlaceHolder, GetLineShape, GetRectangleShape, PlaceHolder, PlaceHolder);
}

void GetLineShape()
{
    struct TwoDhasEdge *temp = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;

    temp->pointarray[1]->x = CurrentPoint->x;
    temp->pointarray[1]->y = CurrentPoint->y;
}