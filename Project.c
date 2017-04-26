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
#define MAX_TEXT_LENGTH 200
#define DEFAULT_COLOR "Black"
#define SELECT_COLOR "Red"
#define POINT_COLOR "Magenta"
#define DL 0.04
#define POINT_R 0.05
#define ROTATE_POINT_RANGE 0.1
#define PI 3.14159
#define LINE_ROTATE_POINT_DISTANCE 0.5
#define LINE_SELECT_WIDTH 0.2
#define DEFAULT_ELLIPSE_RATIO 0.2
#define RATIO_LIMIT 0.1
#define RATIO_CONVERT 1
#define D_ANGLE (3.1416/60)
#define SIZE_FACTOR 0.1
#define INIT_TEXT_FRAME_WIDTH 4
#define INIT_TEXT_FRAME_HEIGHT 3

typedef enum {
    ZOOM_IN,
    ZOOM_OUT
} ZoomType;

typedef enum {
    //CURRENT_POINT_TIMER = 1
    CURSOR_BLINK = 1
} timerID;

typedef enum {
    //CURRENT_POINT_MSECONDS = 5
    CURSOR_BLINK_TIME = 500
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

struct TwoDCurve {
    struct Point *pointarray[MAXPOINT];
    int PointNum;
    double ratio;
};

struct TwoDText {
    //struct TwoDhasEdge *frame;
    struct Point *pointarray[MAXPOINT];
    int PointNum;
    bool *RelationMatrix;

    char *TextArray;
    struct Point *CursorPosition;
    int CursorIndex;
    bool isCursorBlink;
    bool isDisplayCursor;
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
const bool RectangleMatrix[] = {
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
static bool isMouseDown = FALSE;
static bool isMouseDownMoving = FALSE;
static bool isDrawing = FALSE;
static bool isOperating = FALSE;
static bool isRotating = FALSE;
static bool isMovingObj = FALSE;
static bool isCancelSelect = FALSE;
static bool isSelectObj = FALSE;
//static bool SelectFlag = FALSE;
static bool isCursorBlink = FALSE;
//static bool SelectFlag = FALSE;
static struct Point *CurrentPoint, *PreviousPoint;
static struct RegisterADT *RegisterP;
static int SelectNum = 0;
static double angle;
int TestCount = 0;
//int CURSOR_BLINK = 1;
//int CURSOR_BLINK_TIME = 500;


void KeyboardEventProcess(int key,int event);
void CharEventProcess(char c);
void MouseEventProcess(int x, int y, int button, int event);
void TimerEventProcess(int timerID);

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
bool CheckLine(struct obj *Obj);
bool CheckConvexPolygon(struct obj *Obj);
bool CheckText(struct obj *Obj);
bool CompareArray(int *array1, int *array2, int length);
void SetMode(void);
void SetFunction(void);
void PlaceHolder(void);
void MoveObj(struct obj *Obj, double dx, double dy);
void DrawRotatePoint(struct obj *Obj);
void CreateRotatePointForConvexPolygon(struct obj *Obj);
void DrawRotatePoint2(struct obj *Obj);
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
void GetShape(void);
void CreateRotatePoint(struct obj *Obj);
void CreateRotatePointForLine(struct obj* Obj);
void MoveTwoDhasEdge(struct obj *Obj, double dx, double dy);
bool CheckInsidePolygon(struct Point **select_point, struct Point *ReferencePoint, bool *RelationMatrix, int PointNum);
void DeleteObj(void);
void RemoveElement(void **array, int index, int length);
void DrawEllipse(void);
void InitEllipse(void);
void GetEllipseShape(void);
void CreateRotatePointForEllipse(struct obj *Obj);
void DrawOriginalEllipse(struct obj *Obj);
bool CheckEllipse(struct obj *Obj);
void MoveEllipse(struct obj *Obj, double dx, double dy);
void RotateEllipse(void);
void Zoom(struct obj *Obj, int zoom);
void ZoomTwoDhasEdge(struct obj *Obj, int zoom);
void InitText(void);
void DrawTextFrame(struct TwoDhasEdge *polygon, char *color);
void GetTextFrameShape(void);
void DrawTwoDText(void);
void DrawPureText(struct TwoDText *text);
void DisplayCursor(double x, double y);
double CharWide(char ch);
void MoveText(struct obj *Obj, double dx, double dy);

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
    CurrentPoint->x = 0;
    CurrentPoint->y = 0;
    //startTimer(CURRENT_POINT_TIMER, CURRENT_POINT_MSECONDS);
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
            switch (key) {
                case VK_F1:
                    if (RegisterP->ActiveOne != -1) {
                        RegisterP->RegisterObj[RegisterP->ActiveOne]->color = DEFAULT_COLOR;
                        RegisterP->ActiveOne = -1;
                    }
                    SetMode();
                    break;
                case VK_F2:
                    break;
                case VK_F3:
                    break;
                case VK_DELETE:
                    DeleteObj();
                    break;
                case VK_LEFT:
                    if (RegisterP->ActiveOne == -1 || RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) return;
                    break;
                case VK_RIGHT:
                    if (RegisterP->ActiveOne == -1 || RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) return;
                    break;
                case VK_BACK:
                    if (RegisterP->ActiveOne == -1 || RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) return;
                    break;
                case DELETE:
                    if (RegisterP->ActiveOne == -1 || RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) return;
                    break;
            }
            break;
        case KEY_UP:
            break;
    }
}

void CharEventProcess(char c)
{
    switch (c) {
        //case '\r':
            //break;
        case 8: //BACKSPACE
            break;
        case 127: //DEL
            break;
        //case 112:
            //break;
        default:
            if (RegisterP->ActiveOne == -1 || RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) return;
            struct TwoDText *text = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;
            if (text->CursorIndex == MAX_TEXT_LENGTH-1) return;//could be improved
            text->TextArray[text->CursorIndex] = c;
            text->CursorPosition->x += CharWide(text->TextArray[text->CursorIndex]);
            text->TextArray[++(text->CursorIndex)] = '\0';
            RefreshAndDraw();
            break;
    }
}

void TimerEventProcess(int timerID)
{
    //printf("TEST:2: %d\n", ((struct TwoDText *)(RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer))->isDisplayCursor);
    switch (timerID) {
        case CURSOR_BLINK:
            if (RegisterP->ActiveOne == -1) return;
            if (RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) return;
            struct TwoDText *text = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;
            //printf("TEST:here\n");
            if (!(text->isCursorBlink)) return;
            //printf("TEST:here\n");
            SetEraseMode(!(text->isDisplayCursor));
            //printf("TEST: %d\n", text->isDisplayCursor);
            //printf("TEST:2: %d\n", ((struct TwoDText *)(RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer))->isDisplayCursor);
            DisplayCursor(text->CursorPosition->x, text->CursorPosition->y);
            //SetEraseMode(erasemode);
            text->isDisplayCursor = !(text->isDisplayCursor);
            break;
    }
}

void MouseEventProcess(int x, int y, int button, int event)
{
    PreviousPoint->x = CurrentPoint->x;
    PreviousPoint->y = CurrentPoint->y;
    CurrentPoint->x = ScaleXInches(x);
    CurrentPoint->y = GetWindowHeight() - ScaleXInches(y);
    //printf("coodinate: %f, %f\n", CurrentPoint->x, CurrentPoint->y);

    switch (event) {
        case BUTTON_DOWN:
            isMouseDown = TRUE;
            ChooseButton(LeftMouseDownDraw, LeftMouseDownOperate/*,,,,*/, button);
            break;
        case BUTTON_UP:
            isMouseDown = FALSE;
            ChooseButton(LeftMouseUpDraw, LeftMouseUpOperate/*,,,,*/, button);
            isMouseDownMoving = FALSE;
            break;
        case MOUSEMOVE:
            //ChooseButton(LeftMouseMoveDraw/*,,,,,*/, button);
            if (isMouseDown) isMouseDownMoving = TRUE;
            ChooseMode(LeftMouseMoveDraw, LeftMouseMoveOperate);
            break;
        case ROLL_UP:
            if (RegisterP->ActiveOne != -1) {
                Zoom(RegisterP->RegisterObj[RegisterP->ActiveOne], ZOOM_OUT);
            }
            break;
        case ROLL_DOWN:
            if (RegisterP->ActiveOne != -1) {
                Zoom(RegisterP->RegisterObj[RegisterP->ActiveOne], ZOOM_IN);
            }
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
    ChooseDrawWhat(InitText, InitLine, InitRectangle, InitEllipse, PlaceHolder);
    //printf("TEST:LeftMouseDownDraw over\n");
}

static void LeftMouseUpDraw(void)
{
    //printf("TEST:LeftMouseUpDraw\n");
    //printf("TEST:2: %d\n", ((struct TwoDText *)(RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer))->isDisplayCursor);
    /*if (!isMouseDownMoving && RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) {
        DeleteObj();
    }*/
    if (RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType == TEXT) {
        //printf("TEST:here\n");
        //printf("TEST:3: %d\n", ((struct TwoDText *)(RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer))->isDisplayCursor);
        //startTimer(CURSOR_BLINK, CURSOR_BLINK_TIME);
        //printf("TEST:here\n");
        //isSelectObj = TRUE;
    } else {
        RegisterP->ActiveOne = -1;
    }
    //printf("TEST:objnum:%d\n", RegisterP->ObjNum);
    isDrawing = FALSE;
    //DrawWhat = NO_TYPE;
}

static void LeftMouseMoveDraw(void)
{
    //printf("TEST:LeftMouseMoveDraw\n");
    //printf("TEST:here\n");
    if (isDrawing) {
        //SetPenColor();
        //printf("TEST:here %d\n", TestCount++);
        //printf("TEST:here\n");
        GetShape();
        //printf("TEST:here %d\n", TestCount++);
        if (RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType == TEXT) isSelectObj = TRUE;
        RefreshAndDraw();
        //ChooseDrawWhat(/*,,*/DrawTwoDhasEdge/*,,*/);
    }
}

static void LeftMouseDownOperate(void)
{
    int i, j;
    int objnum = RegisterP->ObjNum;
    bool SelectFlag = FALSE;
    //printf("objnum:%d\n", objnum);
    //int SelcectArray[MAXOBJ];
    //printf("TEST:LeftMouseDownOperate\n");
    //SelectFlag = FALSE;
    //SelectFlag = FALSE;
    isOperating = TRUE;
    if (CheckRotate()) {
        isRotating = TRUE;
    } else {
        for (i = 0; i < objnum; i++) {
            if (CheckMouse(RegisterP->RegisterObj[i])) {
                //printf("TEST:here\n");
                //SelcectArray[j] = i;
                //DrawWhat = RegisterP->RegisterObj[i]->DrawType;
                //RegisterP->ActiveOne = i;
                if (RegisterP->ActiveOne == i) {
                    isCancelSelect = TRUE;
                } else {
                    if (SelectNum >= 1) {
                        for (j = 0; j < objnum; j++) {
                            RegisterP->RegisterObj[j]->color = DEFAULT_COLOR;
                        }
                    }
                    RegisterP->RegisterObj[i]->color = SELECT_COLOR;
                    DrawRotatePoint(RegisterP->RegisterObj[i]);
                    SelectNum++;
                    RegisterP->ActiveOne = i;
                    isSelectObj = TRUE;
                }
                isMovingObj = TRUE;
                /*if (RegisterP->RegisterObj[i]->color == SELECT_COLOR) {
                    RegisterP->RegisterObj[i]->color = DEFAULT_COLOR;
                    SelectNum--;
                    RegisterP->ActiveOne = -1;
                } else {
                    if (SelectNum >= 1) {
                        for (j = 0; j < objnum; j++) {
                            RegisterP->RegisterObj[j]->color = DEFAULT_COLOR;
                        }
                    }
                    RegisterP->RegisterObj[i]->color = SELECT_COLOR;
                    DrawRotatePoint(RegisterP->RegisterObj[i]);
                    SelectNum++;
                    RegisterP->ActiveOne = i;
                }*/
                RefreshAndDraw();
                //ChooseDrawWhat(/*,,*/DrawTwoDhasEdge/*,,*/);
                //SelectFlag = 1;
                
                //printf("TEST:here\n");
                SelectFlag = TRUE;
                break;
                //j++;
            }
        }
        if (!SelectFlag) {
            isSelectObj = FALSE;
            for (j = 0; j < objnum; j++) {
                RegisterP->RegisterObj[j]->color = DEFAULT_COLOR;
            }
            RegisterP->ActiveOne = -1;
            RefreshAndDraw();
        }
        //printf("TEST:here\n");
        /*if (SelectNum == 0) {
            //printf("TEST:here\n");
            for (i = 0; i < objnum; i++) {
                RegisterP->RegisterObj[i]->color = DEFAULT_COLOR;
            }
            if (RegisterP->ActiveOne != -1) RegisterP->RegisterObj[RegisterP->ActiveOne]->color = SELECT_COLOR;
            RefreshAndDraw();
        }*/
    }
}

static void LeftMouseUpOperate(void)
{
    //if (SelectNum == 0/*!SelectFlag*/) {
        //printf("TEST:here\n");
        /*for (i = 0; i < objnum; i++) {
        RegisterP->RegisterObj[i]->color = DEFAULT_COLOR;
        }*/
        /*if (RegisterP->ActiveOne != -1) {
            RegisterP->RegisterObj[RegisterP->ActiveOne]->color = SELECT_COLOR;
            RefreshAndDraw();
        }*/
    //}
    //printf("TEST:%d, %d\n", isMouseDownMoving, isCancelSelect);
    if (!isMouseDownMoving && isCancelSelect) { //cancel selecting this obj
        //printf("TEST:%d\n", RegisterP->ActiveOne);
        if (RegisterP->ActiveOne != -1) RegisterP->RegisterObj[RegisterP->ActiveOne]->color = DEFAULT_COLOR;
        isSelectObj = FALSE;
        RefreshAndDraw();
        SelectNum--;
        RegisterP->ActiveOne = -1;
    }
    isOperating = FALSE;
    isRotating = FALSE;
    isCancelSelect = FALSE;
    isMovingObj = FALSE;
}

static void LeftMouseMoveOperate(void)
{
    int i;
    double x0 = PreviousPoint->x;
    double y0 = PreviousPoint->y;
    double x1 = CurrentPoint->x;
    double y1 = CurrentPoint->y;
    //printf("TEST:%d, %d, %d\n",isMovingObj, isRotating, isMouseDown);
    //if (isMouseDown) isMouseDownMoving = TRUE;
    if (isMovingObj && isMouseDown) {
        for (i = 0; i < RegisterP->ObjNum; i++) {
            if (RegisterP->RegisterObj[i]->color == SELECT_COLOR) MoveObj(RegisterP->RegisterObj[i], x1-x0, y1-y0);
        }
    } else if (isRotating && isMouseDown) {
        rotate(x0, y0, x1, y1);
    }
}

void MoveTwoDhasEdge(struct obj *Obj, double dx, double dy)
{
    int i;

    for (i = 0; i < ((struct TwoDhasEdge *) Obj->objPointer)->PointNum; i++) {
        ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->x += dx;
        ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->y += dy;
    }
}

void MoveObj(struct obj *Obj, double dx, double dy)
{
    //printf("TEST:dx, dy = %f, %f", dx, dy);
    switch (Obj->DrawType) {
        case TEXT:
            MoveText(Obj, dx, dy);
            break;
        case LINE:
            MoveTwoDhasEdge(Obj, dx, dy);
            break;
        case RECTANGLE:
            /*for (i = 0; i <4; i++) {
                printf("x0, y0 = %f, %f\n", ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->x, 
                                            ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->y);
            }*/
            MoveTwoDhasEdge(Obj, dx, dy);
            /*for (i = 0; i < ((struct TwoDhasEdge *) Obj->objPointer)->PointNum; i++) {
                ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->x += dx;
                ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->y += dy;
            }*/
            /*for (i = 0; i <4; i++) {
                printf("x1, y1 = %f, %f\n", ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->x, 
                                            ((struct TwoDhasEdge *) Obj->objPointer)->pointarray[i]->y);
            }*/
            break;
        case ELLIPSE:
            MoveEllipse(Obj, dx, dy);
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
            text();
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
            ellipse();
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
    //if (activeone = RegisterP->ActiveOne == -1) return;
    for (i = 0; i < objnum; i++) {
        position = RegisterP->RegisterObj[i];
        SetPenColor(position->color);
        DrawWhat = position->DrawType;
        RegisterP->ActiveOne = i;
        if (position->color == SELECT_COLOR) {
            DrawRotatePoint(position);
        }
        ChooseDrawWhat(DrawTwoDText, DrawTwoDhasEdge, DrawTwoDhasEdge, DrawEllipse, PlaceHolder);
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
    //printf("TEST:activeone: %d\n", RegisterP->ActiveOne);
    RegisterP->ObjNum++;
    //printf("TEST:here\n");
}

void InitRegister(void)
{
    int i;

    RegisterP = GetBlock(sizeof(struct RegisterADT));
    for  (i = 0; i < MAXOBJ; i++) {
        (RegisterP->RegisterObj)[i] = NULL;//GetBlock(sizeof(struct obj));
    }
    RegisterP->ObjNum = 0;
    RegisterP->ActiveOne = -1;
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
    //printf("TEST:DrawTwoDhasEdge\n");
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
    CreateRotatePoint((RegisterP->RegisterObj)[RegisterP->ActiveOne]);
    //CreateRotatePointForConvexPolygon((RegisterP->RegisterObj)[RegisterP->ActiveOne]);
    //printf("TEST:center point:%f, %f\n", obj->CenterPoint->x, obj->CenterPoint->y);
    //SetPenColor(PenColor);
}


void DrawTextFrame(struct TwoDhasEdge *polygon, char *color)
{
    int i, j;
    char * PreColor;
    int pointnum;
    //printf("TEST:here\n");
    RefreshAndDraw();
    PreColor = GetPenColor();
    SetPenColor(color);
    printf("TEST:color: %s\n", color);
    pointnum = polygon->PointNum;
    for (i = 0; i < pointnum; i++) {
        for (j = i; j < pointnum; j++) {
            //printf("TEST:index: %d, x: %f, y: %f\n", i, polygon->pointarray[i]->x, polygon->pointarray[i]->y);
            //printf("TEST:here\n");
            //printf("TEST:matrix: %d\n", polygon->RelationMatrix[i][j]);
            if (*(polygon->RelationMatrix +pointnum*i+j)) DrawDottedLine(polygon->pointarray[i], polygon->pointarray[j]);
        }
    }
    SetPenColor(PreColor);
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
            return CheckText(Obj);
        case LINE:
            return CheckLine(Obj);
        case RECTANGLE:
            return CheckConvexPolygon(Obj);
        case ELLIPSE:
            return CheckEllipse(Obj);
        case LOCUS:
            break;
    }
}

bool CheckLine(struct obj *Obj)
{
    int i;
    struct TwoDhasEdge *line = Obj->objPointer;
    double x = CurrentPoint->x;
    double y = CurrentPoint->y;
    double x0 = line->pointarray[0]->x;
    double y0 = line->pointarray[0]->y;
    double x1 = line->pointarray[1]->x;
    double y1 = line->pointarray[1]->y;
    double cx = Obj->CenterPoint->x;
    double cy = Obj->CenterPoint->y;
    struct Point * select_point[4];
    double X1, X2, X3, X4, Y1, Y2, Y3, Y4;
    bool result;
    double temp = LINE_SELECT_WIDTH /2 / sqrt(pow(y1-y0, 2) + pow(x1-x0, 2));
    //double  = LINE_SELECT_WIDTH /2 * fabs(y1-y0) / sqrt(pow(y1-y0, 2);
    for (i = 0; i < 4; i++) {
        select_point[i] = GetBlock(sizeof(struct Point));
    }
    select_point[0]->x = temp * fabs(y1-y0) + x0;
    select_point[0]->y = y0 - (select_point[0]->x - x0) * (x1 - x0) / (y1 - y0);//temp * fabs(x1-x0) + y0;
    select_point[1]->x = x0 - temp * fabs(y1-y0);//select_point[0]->x - 2 * x0;
    select_point[1]->y = y0 - (select_point[1]->x - x0) * (x1 - x0) / (y1 - y0);
    
    select_point[2]->x = x1 - temp * fabs(y1-y0);
    select_point[2]->y = y1 - (select_point[2]->x - x1) * (x1 - x0) / (y1 - y0);//temp * fabs(x1-x0) - y1;
    select_point[3]->x = temp * fabs(y1-y0) + x1;//select_point[2]->x + 2 * x1;
    select_point[3]->y = y1 - (select_point[3]->x - x1) * (x1 - x0) / (y1 - y0);//select_point[2]->y + 2 * y1;
    result = CheckInsidePolygon(select_point, Obj->CenterPoint, RectangleMatrix, 4);
    for (i = 0; i < 4; i++) {
         free(select_point[i]);
    }
    return result;
}

bool CheckConvexPolygon(struct obj *Obj)
{
    //int i, j, k = 0;
    //double x = CurrentPoint->x;
    //double y = CurrentPoint->y;
    struct TwoDhasEdge *polygon = Obj->objPointer;
    //double cx = Obj->CenterPoint->x;
    //double cy = Obj->CenterPoint->y;
    //int isRegion[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 
    //int isRegionCP[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    //int pointnum = polygon->PointNum;
    //printf("TEST:centerpoint: %f, %f\n", cx, cy);
    //printf("TEST:CheckConvexPolygon\n");
    /*for (i = 0; i < polygon->PointNum; i++) {
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
    }*/
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
    return CheckInsidePolygon(polygon->pointarray, Obj->CenterPoint, polygon->RelationMatrix, polygon->PointNum);//CompareArray(isRegion, isRegionCP, sizeof(isRegion)/sizeof(isRegion[0]));
}

bool CheckInsidePolygon(struct Point **select_point, struct Point *ReferencePoint, bool *RelationMatrix, int PointNum)
{
    int i, j, k;
    int isRegion[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 
    int isRegionCP[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    double x = CurrentPoint->x;
    double y = CurrentPoint->y;
    
    k = 0;
    for (i = 0; i < PointNum; i++) {
        //printf("TEST:loop-\n");
        for (j = i; j < PointNum; j++) {
            //printf("TEST:loop0\n");
            if (*(RelationMatrix + PointNum*i+j)) {
                //printf("TEST:loop:%d, %d\n", i, j);
                //printf("TEST:x: %f, y %f\n", polygon->pointarray[i]->x, polygon->pointarray[i]->y);
                if (select_point[i]->x == select_point[j]->x) {
                    if (x > select_point[i]->x) isRegion[k] = 1;
                    k++;
                } else if (select_point[i]->y == select_point[j]->y) {
                    if (y > select_point[i]->y) isRegion[k] = 1;
                    k++;
                } else if ((x - select_point[i]->x)*(select_point[j]->y - select_point[i]->y)/(select_point[j]->x - select_point[i]->x)
                     < 
                    (y - select_point[i]->y)) {
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
    for (i = 0; i < PointNum; i++) {
        //printf("TEST:loop-\n");
        for (j = i; j < PointNum; j++) {
            //printf("TEST:loop0\n");
            if (*(RelationMatrix + PointNum*i+j)) {
                //printf("TEST:loop1:%d, %d\n", i, j);
                if (select_point[i]->x == select_point[j]->x) {
                    if (ReferencePoint->x > select_point[i]->x) isRegionCP[k] = 1;
                    k++;
                } else if (select_point[i]->y == select_point[j]->y) {
                    if (ReferencePoint->y > select_point[i]->y) isRegionCP[k] = 1;
                    k++;
                } else if ((ReferencePoint->x - select_point[i]->x)*(select_point[j]->y - select_point[i]->y)/(select_point[j]->x - select_point[i]->x)
                     < 
                    (ReferencePoint->y - select_point[i]->y)) {
                        isRegionCP[k++] = 1;
                        //printf("TEST:loop2:%d, %d\n", i, j);
                } else {
                    k++;
                }
            }
        }
    }
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

    RefreshAndDraw();
    cancelTimer(CURSOR_BLINK);
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
            DrawWhat = TEXT;
            //SetFunction();
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
            DrawRotatePoint2(Obj);
            break;
        case RECTANGLE:
            DrawRotatePoint2(Obj);
            break;
        case ELLIPSE:
            DrawRotatePoint2(Obj);
            break;
        default:
            break;
    }
}

void CreateRotatePointForConvexPolygon(struct obj *Obj)
{
    struct TwoDhasEdge *temp = Obj->objPointer;
    struct Point *MiddlePoint = GetBlock(sizeof(struct Point));

    MiddlePoint->x = (temp->pointarray[1]->x + temp->pointarray[0]->x)/2;
    MiddlePoint->y = (temp->pointarray[1]->y + temp->pointarray[0]->y)/2;
    Obj->RotatePoint->x = 1.5*(MiddlePoint->x - Obj->CenterPoint->x) + Obj->CenterPoint->x;
    Obj->RotatePoint->y = 1.5*(MiddlePoint->y - Obj->CenterPoint->y) + Obj->CenterPoint->y;
}

void DrawRotatePoint2(struct obj *Obj)
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
    double len; //= sqrt(pow(point2->x - point1->x, 2) + pow(point2->y - point1->y, 2));
    //printf("TEST:X: %f, Y %f\n", point2->x - point1->x, point2->y - point1->y);
    double length = 0;
    double cos0;// = (point2->x - point1->x)/len;
    double sin0;// = (point2->y - point1->y)/len;
    bool flag = FALSE;
    bool EraseMode = GetEraseMode();
    //printf("TEST:EraseMode1: %d\n", EraseMode);
    //printf("here\n");
    //printf("TEST:color:%s\n", GetPenColor());
    len = sqrt(pow(point2->x - point1->x, 2) + pow(point2->y - point1->y, 2));
    if (len == 0) return;
    cos0 = (point2->x - point1->x)/len;
    sin0 = (point2->y - point1->y)/len;
    MovePen(point1->x, point1->y);
    //printf("TEST:1: %f, %f\n2: %f, %f\n", point1->x, point1->y, point2->x, point2->y);
    //printf("TEST:point:%f, %f\n", point1->x, point1->y);
    //printf("TEST:here\n");
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
    DrawLine((len-length+DL)*cos0, (len-length+DL)*sin0);
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
    //int i;
    //int objnum = RegisterP->ObjNum;

    if (RegisterP->ActiveOne != -1 && InsideRotatePoint(RegisterP->RegisterObj[RegisterP->ActiveOne])) return TRUE;
    else return FALSE;
    /*for (i = 0; i < objnum; i++) {
        if (RegisterP->RegisterObj[i]->color == SELECT_COLOR) {
            if (InsideRotatePoint(RegisterP->RegisterObj[i])) {
                RegisterP->ActiveOne = i;
                //printf("TEST:rotate: %d\n", i);
                return TRUE;
            }
        }
    }
    return FALSE;*/
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
    ChooseDrawWhat(PlaceHolder, RotatePolygon, RotatePolygon, RotateEllipse, PlaceHolder);
    RefreshAndDraw();
}

void RotatePolygon(void) //also works when obj is TwoDhasEdge
{
    struct TwoDhasEdge *Polygon = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;
    int pointnum = Polygon->PointNum;
    int i;
    double cx = RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->x;
    double cy = RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->y;
    //double tempX2 = RegisterP->RegisterObj[RegisterP->ActiveOne]->RotatePoint->x;
    //double tempY2 = RegisterP->RegisterObj[RegisterP->ActiveOne]->RotatePoint->y;
    //printf("TEST:rotate:%d\n", RegisterP->ActiveOne);
    for (i = 0; i < pointnum; i++) {
        double tempX1 = Polygon->pointarray[i]->x;
        double tempY1 = Polygon->pointarray[i]->y;

        Polygon->pointarray[i]->x = (tempX1 - cx) * cos(angle) + (cy - tempY1) * sin(angle) + cx;
        Polygon->pointarray[i]->y = (tempX1 - cx) * sin(angle) + (tempY1 - cy) * cos(angle) + cy;
    }
    //RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->x = (tempX2 - cx) * cos(angle) + (cy - tempY2) * sin(angle) + cx;
    //RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->y = (tempX2 - cx) * sin(angle) + (tempY2 - cy) * cos(angle) + cy;
}

void InitLine(void) // can be merged with InitRectangle()
{
    struct TwoDhasEdge *line = GetBlock(sizeof(struct TwoDhasEdge));

    Register(line, LINE);
    //printf("TEST:here\n");
    line->PointNum = 2;
    (line->pointarray)[0] = GetBlock(sizeof(struct Point));
    (line->pointarray)[1] = GetBlock(sizeof(struct Point));
    (line->pointarray)[0]->x = CurrentPoint->x;
    (line->pointarray)[0]->y = CurrentPoint->y;
    line->RelationMatrix = LineMatrix;
    //printf("TEST:here\n");
}

void GetShape(void)
{
    //printf("TEST:here\n");
    ChooseDrawWhat(GetTextFrameShape, GetLineShape, GetRectangleShape, GetEllipseShape, PlaceHolder);
}

void GetLineShape()
{
    //printf("TEST:here\n");
    //printf("TEST:%d\n", RegisterP->ActiveOne);
    struct TwoDhasEdge *temp = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;
    //printf("TEST:here\n");
    temp->pointarray[1]->x = CurrentPoint->x;
    temp->pointarray[1]->y = CurrentPoint->y;
    //printf("TEST:here %d\n", TestCount++);
}

void CreateRotatePoint(struct obj *Obj) //can be optimized, similar to ChooseDrawWhat
{
    switch (Obj->DrawType) {
        case TEXT:
            break;
        case LINE:
            CreateRotatePointForLine(Obj);
            break;
        case RECTANGLE:
            CreateRotatePointForConvexPolygon(Obj);
            break;
        case ELLIPSE:
            CreateRotatePointForEllipse(Obj);
            break;
        case LOCUS:
            break;
    }
}

void CreateRotatePointForLine(struct obj* Obj)
{
    double x0 = ((struct TwoDhasEdge *)(Obj->objPointer))->pointarray[0]->x;
    double y0 = ((struct TwoDhasEdge *)(Obj->objPointer))->pointarray[0]->y;
    double x1 = ((struct TwoDhasEdge *)(Obj->objPointer))->pointarray[1]->x;
    double y1 = ((struct TwoDhasEdge *)(Obj->objPointer))->pointarray[1]->y;
    double cx = Obj->CenterPoint->x;
    double cy = Obj->CenterPoint->y;
    int i;

    if ((x0 < x1 && y0 < y1) || (x0 > x1 && y1 > y0)) i = -1;
    else i = 1;
    Obj->RotatePoint->x = i * LINE_ROTATE_POINT_DISTANCE * fabs(y1-y0) / sqrt(pow(y1-y0, 2) + pow(x1-x0, 2)) + cx;
    Obj->RotatePoint->y = cy - (Obj->RotatePoint->x - cx) * (x1 - x0) / (y1 - y0); //LINE_ROTATE_POINT_DISTANCE * fabs(x1-x0) / sqrt(pow(y1-y0, 2) + pow(x1-x0, 2)) + cy;
}

void DeleteObj(void)
{
    if (RegisterP->ActiveOne != -1) {
        RemoveElement(RegisterP->RegisterObj, RegisterP->ActiveOne, RegisterP->ObjNum);
        RegisterP->ObjNum--;
        RegisterP->ActiveOne = -1;
    }
    RefreshAndDraw();
}

void RemoveElement(void **array, int index, int length)
{
    int i;

    for (i = index; i < length-1; i++) {
        array[i] = array[i+1];
    }
}

void DrawEllipse(void)
{
    struct obj *Obj = (RegisterP->RegisterObj)[RegisterP->ActiveOne];
    struct TwoDCurve *ellipse = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;

    SetPenColor(Obj->color);
    //Obj->CenterPoint->x = 0;
    //Obj->CenterPoint->y = 0;

    /*DrawEllipticalArc(fabs(ellipse->pointarray[1]->x - ellipse->pointarray[0]->x)/2, 
                      fabs(ellipse->pointarray[2]->y - ellipse->pointarray[0]->y), 
                      0, 360);*/
    Obj->CenterPoint->x = (ellipse->pointarray[0]->x + ellipse->pointarray[1]->x)/2;
    Obj->CenterPoint->y = (ellipse->pointarray[0]->y + ellipse->pointarray[1]->y)/2;
    DrawOriginalEllipse(Obj);
    CreateRotatePoint(Obj);
}

void InitEllipse(void)
{
    struct TwoDCurve *ellipse = GetBlock(sizeof(struct TwoDCurve));
    int i;
    //printf("TEST:here\n");
    Register(ellipse, ELLIPSE);
    ellipse->PointNum = 3;
    for (i = 0; i < ellipse->PointNum; i++) {
        (ellipse->pointarray)[i] = GetBlock(sizeof(struct Point));
    }
    (ellipse->pointarray)[0]->x = CurrentPoint->x;
    (ellipse->pointarray)[0]->y = CurrentPoint->y;
    ellipse->ratio = DEFAULT_ELLIPSE_RATIO;
}

void GetEllipseShape(void)
{
    struct TwoDCurve *temp = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;

    temp->pointarray[1]->x = CurrentPoint->x;
    temp->pointarray[1]->y = temp->pointarray[0]->y;
    //printf("TEST:ratio: %f\n", temp->ratio);
    //printf("TEST: %f\n", fabs(temp->pointarray[1]->x - temp->pointarray[0]->x));
    temp->ratio = DEFAULT_ELLIPSE_RATIO + fabs(CurrentPoint->y - temp->pointarray[1]->y) * RATIO_CONVERT;
    //if (fabs(temp->pointarray[1]->x - temp->pointarray[0]->x) > RATIO_LIMIT) temp->ratio = DEFAULT_ELLIPSE_RATIO + fabs(CurrentPoint->y - temp->pointarray[1]->y)/fabs(temp->pointarray[1]->x - temp->pointarray[0]->x);
    temp->pointarray[2]->x = (temp->pointarray[0]->x + temp->pointarray[1]->x)/2;
    temp->pointarray[2]->y = temp->pointarray[0]->y + fabs(temp->pointarray[1]->x - temp->pointarray[0]->x) * temp->ratio / 2;
    //printf("TEST:ratio: %f\n", temp->ratio);
    //printf("TEST:y2: %f\n", temp->pointarray[2]->y);
}

void CreateRotatePointForEllipse(struct obj *Obj)
{
    struct TwoDCurve *ellipse = Obj->objPointer;

    Obj->RotatePoint->x = 1.5 * (ellipse->pointarray[2]->x) - 0.5 * Obj->CenterPoint->x;
    Obj->RotatePoint->y = 1.5 * (ellipse->pointarray[2]->y) - 0.5 * Obj->CenterPoint->y;
    //Obj->RotatePoint->x = ellipse->pointarray[2]->x;
    //Obj->RotatePoint->y = 1.5*(ellipse->pointarray[2]->y - Obj->CenterPoint->y) + Obj->CenterPoint->y;
}

void DrawOriginalEllipse(struct obj *Obj)//(struct Point *point0, struct Point *point1, struct Point *point2)
{
    double a, b;
    double cx, cy;
    double x0, x1, x2, y0, y1, y2;
    double X, Y, x, y;
    double angle0, angle = 0;
    double radius;
    struct TwoDCurve *ellipse = Obj->objPointer;

    x0 = ellipse->pointarray[0]->x;
    y0 = ellipse->pointarray[0]->y;
    x1 = ellipse->pointarray[1]->x;
    y1 = ellipse->pointarray[1]->y;
    x2 = ellipse->pointarray[2]->x;
    y2 = ellipse->pointarray[2]->y;
    cx = Obj->CenterPoint->x;
    cy = Obj->CenterPoint->y;
    /*printf("TEST:x0: %f, y0: %f\n", x0, y0);
    printf("TEST:x1: %f, y1: %f\n", x1, y1);
    printf("TEST:x2: %f, y2: %f\n", x2, y2);
    printf("TEST:cx: %f, cy: %f\n", cx, cy);
    printf("\n\n");*/
    //printf("TEST:here\n");
    a = sqrt(pow(x0-x1, 2) + pow(y0-y1, 2)) / 2;
    b = sqrt(pow(x2-cx, 2) + pow(y2-cy, 2));

    radius = a;
    if (x1 == x0 && y1 == y0) return; //all the part of angle modification could be merged
    angle0 = acos((x1-x0)/(2*a));
    if ((x0 < x1 && y0 > y1) || (x0 > x1 && y0 > y1)) {
        angle0 = -angle0;
    }
    x = x1;
    y = y1;
    while ((angle += D_ANGLE) < 2*PI) {
        MovePen(x, y);
        radius = sqrt((pow(tan(angle), 2) + 1) /
                      (1/(a*a) + pow(tan(angle), 2)/(b*b)));
        //radius = sqrt(a*a*pow(cos(angle), 2) + b*b*pow(sin(angle), 2));
        X = cx + radius * cos(angle + angle0);
        Y = cy + radius * sin(angle + angle0);
        DrawLine(X-x, Y-y);
        x = X;
        y = Y;
        //angle += D_ANGLE;
    }
    DrawLine(x1-X, y1-Y);

    /*do {
        
    } while ((angle += D_ANGLE) < 2*PI);*/
}

bool CheckEllipse(struct obj *Obj)
{
    struct TwoDCurve *ellipse = Obj->objPointer;
    double cx, cy, a, b;
    double x, y;
    double cos0, sin0;
    //printf("TEST:here\n");
    cx = Obj->CenterPoint->x;
    cy = Obj->CenterPoint->y;
    a = sqrt(pow(ellipse->pointarray[0]->x - ellipse->pointarray[1]->x, 2) + pow(ellipse->pointarray[0]->y - ellipse->pointarray[1]->y, 2)) / 2;
    b = a * ellipse->ratio;
    x = CurrentPoint->x - cx;
    y = CurrentPoint->y - cy;
    //printf("TEST:x: %f, y:%f\n", x, y);
    //printf("TEST:x0: %f, y0: %f\n", ellipse->pointarray[0]->x - cx, ellipse->pointarray[0]->y - cy);
    //printf("TEST:x1: %f, y1: %f\n", ellipse->pointarray[1]->x - cx, ellipse->pointarray[1]->y - cy);
    //printf("TEST:x2: %f, y2: %f\n", ellipse->pointarray[2]->x - cx, ellipse->pointarray[2]->y - cy);
    //printf("TEST:a: %f, b: %f\n", a, b);
    cos0 = (ellipse->pointarray[1]->x - ellipse->pointarray[0]->x)/(2*a);
    sin0 = (ellipse->pointarray[1]->y - ellipse->pointarray[0]->y)/(2*a);
    //printf("TEST:x*sin0 - y*cos0: %f, x*cos0 + y*sin0: %f\n", x*cos0 + y*sin0, x*sin0 - y*cos0);
    //printf("TEST:result %d\n", pow(x*cos0 + y*sin0, 2)/(a*a) + pow(x*sin0 - y*cos0, 2)/(b*b) <= 1);
    return pow(x*cos0 + y*sin0, 2)/(a*a) + pow(x*sin0 - y*cos0, 2)/(b*b) <= 1;   
}

void MoveEllipse(struct obj *Obj, double dx, double dy)
{
    int i;

    for (i = 0; i < ((struct TwoDCurve *) Obj->objPointer)->PointNum; i++) {
        ((struct TwoDCurve *) Obj->objPointer)->pointarray[i]->x += dx;
        ((struct TwoDCurve *) Obj->objPointer)->pointarray[i]->y += dy;
    }
}

void RotateEllipse(void)
{
    struct TwoDCurve *ellipse = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;
    int pointnum = ellipse->PointNum;
    int i;
    double cx = RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->x;
    double cy = RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->y;
    //double tempX2 = RegisterP->RegisterObj[RegisterP->ActiveOne]->RotatePoint->x;
    //double tempY2 = RegisterP->RegisterObj[RegisterP->ActiveOne]->RotatePoint->y;
    //printf("TEST:rotate:%d\n", RegisterP->ActiveOne);
    for (i = 0; i < pointnum; i++) {
        double tempX1 = ellipse->pointarray[i]->x;
        double tempY1 = ellipse->pointarray[i]->y;

        ellipse->pointarray[i]->x = (tempX1 - cx) * cos(angle) + (cy - tempY1) * sin(angle) + cx;
        ellipse->pointarray[i]->y = (tempX1 - cx) * sin(angle) + (tempY1 - cy) * cos(angle) + cy;
    }
}

void Zoom(struct obj *Obj, int zoom)
{
    switch (Obj->DrawType) {
        case TEXT:
            break;
        case LINE:
            ZoomTwoDhasEdge(Obj, zoom);
            break;
        case RECTANGLE:
            ZoomTwoDhasEdge(Obj, zoom);
            break;
        case ELLIPSE:
            ZoomEllipse(Obj, zoom);
            break;
        case LOCUS:
            break;
    }
    RefreshAndDraw();
}

void ZoomTwoDhasEdge(struct obj *Obj, int zoom)
{
    struct TwoDhasEdge *polygon = Obj->objPointer;
    int i, pointnum;
    //double distance;
    double cx, cy;

    cx = Obj->CenterPoint->x;
    cy = Obj->CenterPoint->y;
    pointnum = polygon->PointNum;
    for (i = 0; i < pointnum; i++) {
        //distance = sqrt(pow(polygon->pointarray[i]->x - cx, 2) + pow(polygon->pointarray[i]->y - cy, 2));
        if (zoom == ZOOM_IN) {
            polygon->pointarray[i]->x = (1 - SIZE_FACTOR) * (polygon->pointarray[i]->x - cx) + cx;
            polygon->pointarray[i]->y = (1 - SIZE_FACTOR) * (polygon->pointarray[i]->y - cy) + cy;
        } else if (zoom == ZOOM_OUT) {
            polygon->pointarray[i]->x = (1 + SIZE_FACTOR) * (polygon->pointarray[i]->x - cx) + cx;
            polygon->pointarray[i]->y = (1 + SIZE_FACTOR) * (polygon->pointarray[i]->y - cy) + cy;
        }
    }
}

void ZoomEllipse(struct obj *Obj, int zoom)
{
    struct TwoDCurve *polygon = Obj->objPointer;
    int i, pointnum;
    //double distance;
    double cx, cy;

    cx = Obj->CenterPoint->x;
    cy = Obj->CenterPoint->y;
    pointnum = polygon->PointNum;
    for (i = 0; i < pointnum; i++) {
        //distance = sqrt(pow(polygon->pointarray[i]->x - cx, 2) + pow(polygon->pointarray[i]->y - cy, 2));
        if (zoom == ZOOM_IN) {
            polygon->pointarray[i]->x = (1 - SIZE_FACTOR) * (polygon->pointarray[i]->x - cx) + cx;
            polygon->pointarray[i]->y = (1 - SIZE_FACTOR) * (polygon->pointarray[i]->y - cy) + cy;
        } else if (zoom == ZOOM_OUT) {
            polygon->pointarray[i]->x = (1 + SIZE_FACTOR) * (polygon->pointarray[i]->x - cx) + cx;
            polygon->pointarray[i]->y = (1 + SIZE_FACTOR) * (polygon->pointarray[i]->y - cy) + cy;
        }
    }
}

void InitText(void)
{
    struct TwoDText *text = GetBlock(sizeof(struct TwoDText));
    int i;

    Register(text, TEXT);
    text->PointNum = 4;
    //printf("TEST:here\n");
    /*text->frame = GetBlock(sizeof(struct TwoDhasEdge));
    text->frame->PointNum = 4;
    for (i = 0; i < text->frame->PointNum; i++) {
        (text->frame->pointarray)[i] = GetBlock(sizeof(struct Point));
    }
    (text->frame->pointarray)[0]->x = CurrentPoint->x;
    (text->frame->pointarray)[0]->y = CurrentPoint->y + GetFontHeight();
    (text->frame->pointarray)[1]->x = (text->frame->pointarray)[0]->x + INIT_TEXT_FRAME_WIDTH;
    (text->frame->pointarray)[1]->y = (text->frame->pointarray)[0]->y;
    (text->frame->pointarray)[2]->x = (text->frame->pointarray)[1]->x;
    (text->frame->pointarray)[2]->y = (text->frame->pointarray)[1]->y - INIT_TEXT_FRAME_HEIGHT;
    (text->frame->pointarray)[3]->x = (text->frame->pointarray)[0]->x;
    (text->frame->pointarray)[3]->y = (text->frame->pointarray)[2]->y;
    text->frame->RelationMatrix = RectangleMatrix;
    DrawTextFrame(text->frame, RegisterP->RegisterObj[RegisterP->ActiveOne]->color);*/
    //printf("TEST:here\n");
    text->RelationMatrix = RectangleMatrix;
    for (i = 0; i < text->PointNum; i++) {
        (text->pointarray)[i] = GetBlock(sizeof(struct Point));
    }
    (text->pointarray)[0]->x = CurrentPoint->x;
    (text->pointarray)[0]->y = CurrentPoint->y;// + GetFontHeight();
    
    text->TextArray = GetBlock( MAX_TEXT_LENGTH * sizeof(char));
    for (i = 0; i < MAX_TEXT_LENGTH; i++) {
        text->TextArray[i] = '\0';
    }
    text->CursorIndex = 0;
    text->CursorPosition = GetBlock(sizeof(struct Point));
    text->CursorPosition->x = CurrentPoint->x;
    text->CursorPosition->y = CurrentPoint->y - GetFontHeight(); //????????
    text->isCursorBlink = TRUE;
    text->isDisplayCursor = FALSE;
}


void GetTextFrameShape(void)
{
    struct TwoDText *text = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;
    //printf("TEST: pointnum: %d\n", text->PointNum);
    text->pointarray[2]->x = CurrentPoint->x;
    text->pointarray[2]->y = CurrentPoint->y;
    text->pointarray[1]->x = text->pointarray[2]->x;
    text->pointarray[1]->y = text->pointarray[0]->y;
    text->pointarray[3]->x = text->pointarray[0]->x;
    text->pointarray[3]->y = text->pointarray[2]->y;
}
/*void GetTextFrameShape(void)
{
    struct TwoDText * text = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;

    text->pointarray[2]->x = CurrentPoint->x;
    text->pointarray[2]->y = CurrentPoint->y;
    text->pointarray[1]->x = text->pointarray[2]->x;
    text->pointarray[1]->y = text->pointarray[0]->y;
    text->pointarray[3]->x = text->pointarray[0]->x;
    text->pointarray[3]->y = text->pointarray[2]->y;
}*/

void DrawTwoDText(void)
{
    int i, j, pointnum;
    struct TwoDText *obj = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;
    struct obj *Obj = (RegisterP->RegisterObj)[RegisterP->ActiveOne];
    bool EraseMode;

    EraseMode = GetEraseMode();
    //SetEraseMode(isCancelSelect);
    //printf("TEST:%d\n", isCancelSelect);
    SetPenColor((RegisterP->RegisterObj)[RegisterP->ActiveOne]->color);
    Obj->CenterPoint->x = 0;
    Obj->CenterPoint->y = 0;
    pointnum = obj->PointNum;
    for (i = 0; i < pointnum; i++) {
         Obj->CenterPoint->x += obj->pointarray[i]->x;
         Obj->CenterPoint->y += obj->pointarray[i]->y;
        for (j = i; j < pointnum; j++) {
            //printf("TEST:here\n");
            //printf("TEST:matrix: %d\n", obj->RelationMatrix[i][j]);
            if (*(obj->RelationMatrix +pointnum*i+j)) {
                if (!isSelectObj) {
                    SetEraseMode(TRUE);
                    DrawLineByPoint(obj->pointarray[i], obj->pointarray[j]);
                } else DrawDottedLine(obj->pointarray[i], obj->pointarray[j]);
            }
        }
    }
    Obj->CenterPoint->x /= obj->PointNum;
    Obj->CenterPoint->y /= obj->PointNum;
    if (Obj->color == SELECT_COLOR || isMouseDownMoving) {
        //printf("TEST:here\n");
        startTimer(CURSOR_BLINK, CURSOR_BLINK_TIME);
    }
    if (1/*Obj->color == SELECT_COLOR*/) {
        DrawPureText(obj);
        //printf("TEST:%s\n", obj->TextArray);
    }
    SetEraseMode(EraseMode);
}

void DrawPureText(struct TwoDText *text)
{
    bool EraseMode = GetEraseMode();
    char *color = GetPenColor();

    SetPenColor(DEFAULT_COLOR);
    SetEraseMode(FALSE);
    MovePen(text->pointarray[0]->x, text->pointarray[0]->y - GetFontHeight());
    DrawTextString(text->TextArray);
    SetEraseMode(TRUE);
    SetPenColor(color);
}

void DisplayCursor(double x, double y)
{
    char cursor[] = {'|', '\0'};

    MovePen(x, y);
    DrawTextString(cursor);
}

bool CheckText(struct obj *Obj)
{
    struct TwoDText *text = Obj->objPointer;

    return CheckInsidePolygon(text->pointarray, Obj->CenterPoint, text->RelationMatrix, text->PointNum);
}

double CharWide(char ch)
{
    return TextStringWidth(CharToString(ch));
}

void MoveText(struct obj *Obj, double dx, double dy)
{
    int i;
    struct TwoDText *text = Obj->objPointer;

    for (i = 0; i < text->PointNum; i++) {
        text->pointarray[i]->x += dx;
        text->pointarray[i]->y += dy;
    }
    text->CursorPosition->x += dx;
    text->CursorPosition->y += dy;
}