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
#define DEFAULT_COLOR "White"
#define SELECT_COLOR "Red"

typedef enum {
    CURRENT_POINT_TIMER = 1
} timerID;

typedef enum {
    CURRENT_POINT_MSECONDS = 100
}

typedef enum {
    TEXT,
    LINE,
    RECTANGLE,
    ELLIPSE,
    LOCUS,
} 2DDrawType;

typedef enum {
    DRAW;
    OPERATE;
} mode;

struct Point {
    double x;
    double y;
    double z;
};

struct 2Dobj {
    void (*draw)(void);
    void (*rotate)(void);
    void (*move)(void);
    string color;
    int DegreePointFreedom;
};

struct 2DhasEdge {
    struct 2Dobj *obj;
    struct Point *pointarray[MAXPOINT];
    bool **RelationMatrix;
};

/*struct obj {
    void *objPointer;
    bool isActive;
};*/

struct RegisterADT {
    void *RegisterObj[MAXOBJ];
    int ObjNum;
    void *ActiveOne;
};

const bool RectangleMatrix[][] = {
    {0, 1, 1, 1},
    {1, 0, 1, 1},
    {1, 1, 0, 1},
    {1, 1, 1, 0}
};
static int mode; //Draw or Operate
static int DrawWhat; //DrawType
static bool isDrawing = FALSE;
static struct Point *CurrentPoint;
static struct RegisterADT *RegisterP;

void KeyboardEventProcess(int key,int event);
void CharEventProcess(char c);
void MouseEventProcess(int x, int y, int button, int event);
void TimerEventProcess(int timerID);

void GetCurrentPoint(void);
static void ChooseButton(void (*left1)(void), void (*left2)(void)/*, 
                         void (*right1)(void), void (*right2)(void), 
                         void (*middle1)(void), void (*middle2)(void)*/);
static void ChooseMode(void (*draw)(void)/*, void (*operate)(void)*/);
static void LeftMouseDownDraw(void);
static void LeftMouseUpDraw(void);
static void LeftMouseMoveDraw(void);
void RefreshDisplay(void);
void InitRegister(void);
struct obj *FindActive(void)

void Main()
{
    SetWindowTitle("CAD Program");
    InitGraphics();
	
    startTimer(CURRENT_POINT_TIMER, CURRENT_POINT_MSECONDS);
    InitRegister();

	registerKeyboardEvent(KeyboardEventProcess);
	registerCharEvent(CharEventProcess);
	registerMouseEvent(MouseEventProcess);
	registerTimerEvent(TimerEventProcess);
}

void KeyboardEventProcess(int key,int event)
{
    switch (event) {
        case KEY_DOWN:
            break;
        case KEY_DOWN:
            break;
    }
}

void CharEventProcess(char c)



void TimerEventProcess(int timerID)
{
    switch (timerID) {
        case CURRENT_POINT_TIMER:
            CurrentPoint = GetBlock(sizeof(struct Point));
            GetCurrentPoint();
            break;
    }
}

void MouseEventProcess(int x, int y, int button, int event)
{
    switch (event) {
        case BUTTON_DOWN:
            ChooseButton(LeftMouseDownDraw/*,,,,,*/);
            break;
        case BUTTON_UP:
            ChooseButton(LeftMouseUpDraw/*,,,,,*/);
            break;
        case MOUSE_MOVE:
            //ChooseButton();
            break;
    }
}

static void ChooseButton(void (*left1)(void)/*, void (*left2)(void), 
                         void (*right1)(void), void (*right2)(void), 
                         void (*middle1)(void), void (*middle2)(void)*/)
{
    switch (button) {
        case LEFT_BUTTON:
            ChooseMode(left1/*, left2*/);
            break;
        case RIGHT_BUTTON:
            //ChooseMode(right1, right2);
            break;
        case MIDDLE_BUTTON:
            //ChooseMode(middle1, middle2);
            break;
    }
}

static void ChooseMode(void (*draw)(void)/*, void (*operate)(void)*/)
{
    switch (mode) {
        case DRAW:
            draw();
            break;
        case OPERATE:
            //operate();
            break;
    }
}

static void LeftMouseDownDraw(void)
{
    isDrawing = TRUE;
    ChooseDrawWhat(/*,,*/InitRectangle/*,,*/);
}

static void LeftMouseUpDraw(void)
{
    isDrawing = FALSE;
}

static void LeftMouseMoveDraw(void)
{
    if (isDrawing) {

    }
}

void ChooseDrawWhat(/*void (*text)(void), 
                    void (*line)(void),*/
                    void (*rectangle)(void)/*, 
                    void (*ellipse)(void), 
                    void (*locus)(void)*/)
{
    switch (DrawingWhat) {
        case TEXT:
            text();
            break;
        case LINE:
            line();
            break;
        case RECTANGLE:
            rectangle();
            break;
        case ELLIPSE:
            ellipse();
            break;
        case LOCUS:
            locus();
            break;
    }
}

void InitRectangle(void)
{
    //struct obj *obj;
    struct 2DhasEdge *rectangle = GetBlock(sizeof(struct 2DhasEdge));
    int i;

    Register(rectangle);

    for (i = 0; i < 4; i++) {
        (rectangle->pointarray)[i] = GetBlock(sizeof(struct Point));
    }
    (rectangle->pointarray)[0]->x = CurrentPoint->x;
    (rectangle->pointarray)[0]->y = CurrentPoint->y;
    rectangle->RelationMatrix = RectangleMatrix;

    rectangle->obj = GetBlock(sizeof(struct 2Dobj));
    rectangle->obj->color = DEFAULT_COLOR;
    rectangle->obj->DegreePointFreedom = 2;
    //rectangle->obj->draw = 
    //rectangle->obj->rotate = 
    //rectangle->obj->move = 
}

void GetCurrentPoint(void)
{
    CurrentPoint->x = GetCurrentX();
    CurrentPoint->y = GetCurrentY();
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
    MovePen(CurrentPoint.x, CurrentPoint.y);
    SetEraseMode(FALSE);
}

struct Point *CopyPoint(struct Point *point)
{
    struct Point *DestinationPoint = GetBlock(sizeof(struct Point));
    
    DestinationPoint->x = point->x;
    DestinationPoint->y = point->y;
    return DestinationPoint;
}

void Register(void *objPt)
{
    //struct obj *objP = GetBlock(sizeof(struct obj));

    //objP->objPointer = objPt;
    //objP->isActive = TRUE;
    (RegisterP->RegisterObj)[(RegisterP->ObjNum)++] = objPt;
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

struct obj *FindActive(void)
{
    int i;
    struct obj *result;

    for (i = 0; result = (RegisterADT->RegisterObj)[i]; i++) { //not false
        if (result->isActive) return result;
    }
}