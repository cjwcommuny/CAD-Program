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
#define CURSOR_SIZE 5
#define ROW_HEIGHT_MODIFY 0.07
#define EXTEND_TEXT_FRAME_CONSTANT 0.02

typedef enum {
    ZOOM_IN,
    ZOOM_OUT
} ZoomType;

typedef enum {
    CURSOR_BLINK = 1
} timerID;

typedef enum {
    CURSOR_BLINK_TIME = 500
} mseconds;

typedef enum {
    NO_TYPE,
    TEXT,
    LINE,
    RECTANGLE,
    ELLIPSE,
    LOCUS,
} _2DDrawType;

typedef enum {
    DRAW,
    OPERATE,
} ModeType;

struct Point {
    double x;
    double y;
};

struct _2DhasEdge {
    struct Point *pointarray[MAXPOINT];
    int PointNum;
    bool *RelationMatrix; //could be generalizied
};

struct _2DCurve {
    struct Point *pointarray[MAXPOINT];
    int PointNum;
    double ratio;
};

struct _2DText {
    struct Point *pointarray[MAXPOINT];
    int PointNum;
    bool *RelationMatrix;

    int FontSize;
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
    struct Point *RotatePoint;
    struct Point *CenterPoint;
};

struct RegisterADT {
    struct obj *RegisterObj[MAXOBJ];
    int ObjNum;
    int ActiveOne;
};

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
static int SelectNum = 0;
static int TestCount = 0;

static bool isMouseDown = FALSE;
static bool isMouseDownMoving = FALSE;
static bool isDrawing = FALSE;
static bool isOperating = FALSE;
static bool isRotating = FALSE;
static bool isMovingObj = FALSE;
static bool isCancelSelect = FALSE;
static bool isSelectObj = FALSE;
static bool isCursorBlink = FALSE;
static bool isControlDown = FALSE;
static bool isSelectSelf = FALSE;

static struct Point *CurrentPoint, *PreviousPoint;
static struct RegisterADT *RegisterP;

static double angle;

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
void Draw_2DhasEdge(void);
void DrawLineByPoint(struct Point *point1, struct Point *point2);
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
void Move_2DhasEdge(struct obj *Obj, double dx, double dy);
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
void Zoom_2DhasEdge(struct obj *Obj, int zoom);
void InitText(void);
void GetTextFrameShape(void);
void Draw_2DText(void);
void DrawPureText(struct _2DText *text);
void DisplayCursor(double x, double y);
double CharWide(char ch);
void MoveText(struct obj *Obj, double dx, double dy);
void InsertChar(char *array, char element, int index, int length);
void ZoomText(struct obj *Obj, int zoom);

void Main()
{
    SetWindowTitle("CAD Program");
    InitGraphics();
    CurrentPoint = GetBlock(sizeof(struct Point));
    PreviousPoint = GetBlock(sizeof(struct Point));
    PreviousPoint->x = 0;
    PreviousPoint->y = 0;
    CurrentPoint->x = 0;
    CurrentPoint->y = 0;
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
                        isSelectObj = FALSE;
                    }
                    SetMode();
                    break;
                case VK_F2:
                    break;
                case VK_F3:
                    break;
                case VK_CONTROL:
                    isControlDown = TRUE;
                    break;
                case VK_LEFT: {
                    if (RegisterP->ActiveOne == -1 || RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) return;
                    struct _2DText *text = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;
                    if (text->CursorIndex == 0) return;
                    text->CursorIndex -= 1;
                    text->CursorPosition->x -= CharWide(text->TextArray[text->CursorIndex]);
                    RefreshAndDraw();
                    break;
                }
                case VK_RIGHT: {
                    if (RegisterP->ActiveOne == -1 || RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) return;
                    struct _2DText *text = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;
                    if (text->TextArray[text->CursorIndex] == '\0') return;
                    text->CursorPosition->x += CharWide(text->TextArray[text->CursorIndex]);
                    text->CursorIndex += 1;
                    RefreshAndDraw();
                    break;
                }
                case VK_BACK: {
                    if (RegisterP->ActiveOne == -1 || RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) return;
                    struct _2DText *text = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;
                    if (text->CursorIndex == 0) return;
                    char *TextCopy = GetBlock(MAX_TEXT_LENGTH * sizeof(char));
                    strcpy(TextCopy, text->TextArray + text->CursorIndex);
                    text->CursorPosition->x -= CharWide(text->TextArray[--(text->CursorIndex)]);
                    strcpy(text->TextArray + text->CursorIndex, TextCopy);
                    RefreshAndDraw();
                    break;
                }
                case VK_DELETE: {
                    if (isControlDown) {
                        DeleteObj();
                    } else {
                        if (RegisterP->ActiveOne == -1 || RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) return;
                        struct _2DText *text = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;
                        if (text->TextArray[text->CursorIndex] == '\0') return;
                        char *TextCopy = GetBlock(MAX_TEXT_LENGTH * sizeof(char));
                        strcpy(TextCopy, text->TextArray + text->CursorIndex + 1);
                        strcpy(text->TextArray + text->CursorIndex, TextCopy);
                        RefreshAndDraw();
                    }
                    break;
                }
            }
            break;
        case KEY_UP:
            isControlDown = FALSE;
            break;
    }
}

void CharEventProcess(char c)
{
    switch (c) {
        case '\r': {
            if (RegisterP->ActiveOne == -1 || RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) return;
            struct _2DText *text = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;

            InsertChar(text->TextArray, '\0', text->CursorIndex, MAX_TEXT_LENGTH);
            break;
        }
        case 8: //BACKSPACE
            break;
        case 127: //DEL
            break;
        default: {
            if (RegisterP->ActiveOne == -1 || RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) return;
            struct _2DText *text = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;
            double ChWide = CharWide(c);

            if (text->pointarray[1]->x - text->CursorPosition->x < EXTEND_TEXT_FRAME_CONSTANT + ChWide) {
                text->pointarray[1]->x = text->pointarray[2]->x += ChWide;
            }
            InsertChar(text->TextArray, c, text->CursorIndex, MAX_TEXT_LENGTH);
            text->CursorPosition->x += ChWide;
            (text->CursorIndex)++;
            RefreshAndDraw();
            break;
        } 
    }
}

void TimerEventProcess(int timerID)
{
    switch (timerID) {
        case CURSOR_BLINK: {
            if (RegisterP->ActiveOne == -1) return;
            if (RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) return;
            struct _2DText *text = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;
            if (!(text->isCursorBlink)) return;
            SetEraseMode(!(text->isDisplayCursor));
            DisplayCursor(text->CursorPosition->x, text->CursorPosition->y);
            text->isDisplayCursor = !(text->isDisplayCursor);
            break;
        }
    }
}

void MouseEventProcess(int x, int y, int button, int event)
{
    PreviousPoint->x = CurrentPoint->x;
    PreviousPoint->y = CurrentPoint->y;
    CurrentPoint->x = ScaleXInches(x);
    CurrentPoint->y = GetWindowHeight() - ScaleXInches(y);

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
            draw();
            break;
        case OPERATE:
            operate();
            break;
    }
}

static void LeftMouseDownDraw(void)
{
    isDrawing = TRUE;
    ChooseDrawWhat(InitText, InitLine, InitRectangle, InitEllipse, PlaceHolder);
}

static void LeftMouseUpDraw(void)
{
    if (isMouseDownMoving == FALSE) {
        DeleteObj();
    } else if (RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType != TEXT) {
        RegisterP->ActiveOne = -1;
    }
    isDrawing = FALSE;
}

static void LeftMouseMoveDraw(void)
{
    if (isDrawing) {
        GetShape();
        if (RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType == TEXT) isSelectObj = TRUE;
        RefreshAndDraw();
    }
}

static void LeftMouseDownOperate(void)
{
    int i, j;
    int objnum = RegisterP->ObjNum;
    bool SelectFlag = FALSE;

    isOperating = TRUE;
    if (CheckRotate()) {
        isRotating = TRUE;
    } else {
        for (i = 0; i < objnum; i++) {
            if (CheckMouse(RegisterP->RegisterObj[i])) {
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
                RefreshAndDraw();
                SelectFlag = TRUE;
                break;
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
    }
}

static void LeftMouseUpOperate(void)
{
    if (!isMouseDownMoving && isCancelSelect) { //cancel selecting this obj
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

    if (isMovingObj && isMouseDown) {
        for (i = 0; i < RegisterP->ObjNum; i++) {
            if (RegisterP->RegisterObj[i]->color == SELECT_COLOR) MoveObj(RegisterP->RegisterObj[i], x1-x0, y1-y0);
        }
    } else if (isRotating && isMouseDown) {
        DrawWhat = RegisterP->RegisterObj[RegisterP->ActiveOne]->DrawType;
        rotate(x0, y0, x1, y1);
    }
}

void Move_2DhasEdge(struct obj *Obj, double dx, double dy)
{
    int i;

    for (i = 0; i < ((struct _2DhasEdge *) Obj->objPointer)->PointNum; i++) {
        ((struct _2DhasEdge *) Obj->objPointer)->pointarray[i]->x += dx;
        ((struct _2DhasEdge *) Obj->objPointer)->pointarray[i]->y += dy;
    }
}

void MoveObj(struct obj *Obj, double dx, double dy)
{
    switch (Obj->DrawType) {
        case TEXT:
            MoveText(Obj, dx, dy);
            break;
        case LINE:
            Move_2DhasEdge(Obj, dx, dy);
            break;
        case RECTANGLE:
            Move_2DhasEdge(Obj, dx, dy);
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
    switch (DrawWhat) {
        case NO_TYPE:
            break;
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
            PlaceHolder();
            break;
        default:
            break;
    }
}

void InitRectangle(void)
{
    struct _2DhasEdge *rectangle = GetBlock(sizeof(struct _2DhasEdge));
    int i;

    Register(rectangle, RECTANGLE);

    rectangle->PointNum = 4;
    for (i = 0; i < rectangle->PointNum; i++) {
        (rectangle->pointarray)[i] = GetBlock(sizeof(struct Point));
    }
    (rectangle->pointarray)[0]->x = CurrentPoint->x;
    (rectangle->pointarray)[0]->y = CurrentPoint->y;
    rectangle->RelationMatrix = RectangleMatrix;
}

void RefreshDisplay(void)
{
    SetEraseMode(TRUE);
    StartFilledRegion(1);
    DrawRectangle(0, 0, GetWindowWidth(), GetWindowHeight()); 
    EndFilledRegion();
    MovePen(CurrentPoint->x, CurrentPoint->y);
    SetEraseMode(FALSE);
}

void RefreshAndDraw(void)
{
    int i , objnum = RegisterP->ObjNum, temp;
    struct obj *position;
    string PenColor;
    int activeone = RegisterP->ActiveOne;

    RefreshDisplay();
    PenColor = GetPenColor();
    for (i = 0; i < objnum; i++) {
        position = RegisterP->RegisterObj[i];
        SetPenColor(position->color);
        DrawWhat = position->DrawType;
        RegisterP->ActiveOne = i;
        if (i == activeone) isSelectSelf = TRUE;
        else isSelectSelf = FALSE;
        if (position->color == SELECT_COLOR) {
            DrawRotatePoint(position);
        }
        ChooseDrawWhat(Draw_2DText, Draw_2DhasEdge, Draw_2DhasEdge, DrawEllipse, PlaceHolder);
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
    objP->objPointer = objPt;
    objP->DrawType = type;
    objP->color = DEFAULT_COLOR;
    objP->RotatePoint = GetBlock(sizeof(struct Point));
    objP->CenterPoint = GetBlock(sizeof(struct Point));
    (RegisterP->RegisterObj)[RegisterP->ObjNum] = objP;
    RegisterP->ActiveOne = RegisterP->ObjNum;
    RegisterP->ObjNum++;
}

void InitRegister(void)
{
    int i;

    RegisterP = GetBlock(sizeof(struct RegisterADT));
    for  (i = 0; i < MAXOBJ; i++) {
        (RegisterP->RegisterObj)[i] = NULL;
    }
    RegisterP->ObjNum = 0;
    RegisterP->ActiveOne = -1;
}

void Draw_2DhasEdge(void)
{
    int i, j, pointnum;
    struct _2DhasEdge *obj = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;
    struct obj *Obj = (RegisterP->RegisterObj)[RegisterP->ActiveOne];
    
    SetPenColor((RegisterP->RegisterObj)[RegisterP->ActiveOne]->color);
    Obj->CenterPoint->x = 0;
    Obj->CenterPoint->y = 0;
    pointnum = obj->PointNum; 
    for (i = 0; i < pointnum; i++) {
         Obj->CenterPoint->x += obj->pointarray[i]->x;
         Obj->CenterPoint->y += obj->pointarray[i]->y;
        for (j = i; j < pointnum; j++) {
            if (*(obj->RelationMatrix +pointnum*i+j)) DrawLineByPoint(obj->pointarray[i], obj->pointarray[j]);
        }
    }
    Obj->CenterPoint->x /= obj->PointNum;
    Obj->CenterPoint->y /= obj->PointNum;
    CreateRotatePoint((RegisterP->RegisterObj)[RegisterP->ActiveOne]);
}

void DrawLineByPoint(struct Point *point1, struct Point *point2)
{
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
    struct _2DhasEdge *temp = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;

    temp->pointarray[2]->x = CurrentPoint->x;
    temp->pointarray[2]->y = CurrentPoint->y;
    temp->pointarray[1]->x = temp->pointarray[2]->x;
    temp->pointarray[1]->y = temp->pointarray[0]->y;
    temp->pointarray[3]->x = temp->pointarray[0]->x;
    temp->pointarray[3]->y = temp->pointarray[2]->y;
}

bool CheckMouse(struct obj *Obj)
{
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
    struct _2DhasEdge *line = Obj->objPointer;
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
    for (i = 0; i < 4; i++) {
        select_point[i] = GetBlock(sizeof(struct Point));
    }
    select_point[0]->x = temp * fabs(y1-y0) + x0;
    select_point[0]->y = y0 - (select_point[0]->x - x0) * (x1 - x0) / (y1 - y0);
    select_point[1]->x = x0 - temp * fabs(y1-y0);
    select_point[1]->y = y0 - (select_point[1]->x - x0) * (x1 - x0) / (y1 - y0);
    
    select_point[2]->x = x1 - temp * fabs(y1-y0);
    select_point[2]->y = y1 - (select_point[2]->x - x1) * (x1 - x0) / (y1 - y0);
    select_point[3]->x = temp * fabs(y1-y0) + x1;
    select_point[3]->y = y1 - (select_point[3]->x - x1) * (x1 - x0) / (y1 - y0);
    result = CheckInsidePolygon(select_point, Obj->CenterPoint, RectangleMatrix, 4);
    for (i = 0; i < 4; i++) {
         free(select_point[i]);
    }
    return result;
}

bool CheckConvexPolygon(struct obj *Obj)
{
    struct _2DhasEdge *polygon = Obj->objPointer;

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
        for (j = i; j < PointNum; j++) {
            if (*(RelationMatrix + PointNum*i+j)) {
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
                } else {
                    k++;
                }
            }
        }
    }

    k = 0;

    for (i = 0; i < PointNum; i++) {
        for (j = i; j < PointNum; j++) {
            if (*(RelationMatrix + PointNum*i+j)) {
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
            printf("Click the left button of mouse and drag to draw the text box and then type the text!\n'");
            break;
        case 3:
            mode = OPERATE;
            printf("Selcect the object and move, rotate, or delete it!\n");
            break;
    }
    system("pause");
    FreeConsole();
}

void SetFunction(void)
{
    int input;

    switch (mode) {
        case DRAW:
            printf("What object do you want to draw?\n");
            printf("1:Line\n2:Rectangle\n3:Ellipse\n");
            scanf("%d", &input); //wrong input prompt
            switch (input) {
                case 1:
                    DrawWhat = LINE;
                    printf("Let's draw the line!\n");
                    break;
                case 2:
                    DrawWhat = RECTANGLE;
                    printf("Let's draw the rectangle!\n");
                    break;
                case 3:
                    DrawWhat = ELLIPSE;
                    printf("Let's draw the ellipse!\n");
                    break;
                case 4:
                    DrawWhat = LOCUS;
                    break;
            }
            break;
    }
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
    struct _2DhasEdge *temp = Obj->objPointer;
    struct Point *MiddlePoint = GetBlock(sizeof(struct Point));

    MiddlePoint->x = (temp->pointarray[1]->x + temp->pointarray[0]->x)/2;
    MiddlePoint->y = (temp->pointarray[1]->y + temp->pointarray[0]->y)/2;
    Obj->RotatePoint->x = 1.5*(MiddlePoint->x - Obj->CenterPoint->x) + Obj->CenterPoint->x;
    Obj->RotatePoint->y = 1.5*(MiddlePoint->y - Obj->CenterPoint->y) + Obj->CenterPoint->y;
}

void DrawRotatePoint2(struct obj *Obj)
{
    string PenColor;

    PenColor = GetPenColor();
    SetPenColor(Obj->color);
    DrawDottedLine(Obj->CenterPoint, Obj->RotatePoint);
    SetPenColor(POINT_COLOR);
    DrawPoint(GetCurrentX(), GetCurrentY());
    SetPenColor(PenColor);
}

void DrawDottedLine(struct Point *point1, struct Point *point2)
{
    double len; 
    double length = 0;
    double cos0;
    double sin0;
    bool flag = FALSE;
    bool EraseMode = GetEraseMode();

    len = sqrt(pow(point2->x - point1->x, 2) + pow(point2->y - point1->y, 2));
    if (len == 0) return;
    cos0 = (point2->x - point1->x)/len;
    sin0 = (point2->y - point1->y)/len;
    MovePen(point1->x, point1->y);
    SetEraseMode(FALSE);
    while (length <= len) {
        DrawLine(DL*cos0, DL*sin0);
        flag = !flag;
        SetEraseMode(flag);
        length += DL;
    }
    DrawLine((len-length+DL)*cos0, (len-length+DL)*sin0);
    SetEraseMode(EraseMode);
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
    if (RegisterP->ActiveOne != -1 && InsideRotatePoint(RegisterP->RegisterObj[RegisterP->ActiveOne])) return TRUE;
    else return FALSE;
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

void RotatePolygon(void) //also works when obj is _2DhasEdge
{
    struct _2DhasEdge *Polygon = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;
    int pointnum = Polygon->PointNum;
    int i;
    double cx = RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->x;
    double cy = RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->y;

    for (i = 0; i < pointnum; i++) {
        double tempX1 = Polygon->pointarray[i]->x;
        double tempY1 = Polygon->pointarray[i]->y;

        Polygon->pointarray[i]->x = (tempX1 - cx) * cos(angle) + (cy - tempY1) * sin(angle) + cx;
        Polygon->pointarray[i]->y = (tempX1 - cx) * sin(angle) + (tempY1 - cy) * cos(angle) + cy;
    }
}

void InitLine(void) // can be merged with InitRectangle()
{
    struct _2DhasEdge *line = GetBlock(sizeof(struct _2DhasEdge));

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
    ChooseDrawWhat(GetTextFrameShape, GetLineShape, GetRectangleShape, GetEllipseShape, PlaceHolder);
}

void GetLineShape()
{
    struct _2DhasEdge *temp = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;

    temp->pointarray[1]->x = CurrentPoint->x;
    temp->pointarray[1]->y = CurrentPoint->y;
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
    double x0 = ((struct _2DhasEdge *)(Obj->objPointer))->pointarray[0]->x;
    double y0 = ((struct _2DhasEdge *)(Obj->objPointer))->pointarray[0]->y;
    double x1 = ((struct _2DhasEdge *)(Obj->objPointer))->pointarray[1]->x;
    double y1 = ((struct _2DhasEdge *)(Obj->objPointer))->pointarray[1]->y;
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
    struct _2DCurve *ellipse = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;

    SetPenColor(Obj->color);
    Obj->CenterPoint->x = (ellipse->pointarray[0]->x + ellipse->pointarray[1]->x)/2;
    Obj->CenterPoint->y = (ellipse->pointarray[0]->y + ellipse->pointarray[1]->y)/2;
    DrawOriginalEllipse(Obj);
    CreateRotatePoint(Obj);
}

void InitEllipse(void)
{
    struct _2DCurve *ellipse = GetBlock(sizeof(struct _2DCurve));
    int i;

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
    struct _2DCurve *temp = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;

    temp->pointarray[1]->x = CurrentPoint->x;
    temp->pointarray[1]->y = temp->pointarray[0]->y;
    temp->ratio = DEFAULT_ELLIPSE_RATIO + fabs(CurrentPoint->y - temp->pointarray[1]->y) * RATIO_CONVERT;
    temp->pointarray[2]->x = (temp->pointarray[0]->x + temp->pointarray[1]->x)/2;
    temp->pointarray[2]->y = temp->pointarray[0]->y + fabs(temp->pointarray[1]->x - temp->pointarray[0]->x) * temp->ratio / 2;
}

void CreateRotatePointForEllipse(struct obj *Obj)
{
    struct _2DCurve *ellipse = Obj->objPointer;

    Obj->RotatePoint->x = 1.5 * (ellipse->pointarray[2]->x) - 0.5 * Obj->CenterPoint->x;
    Obj->RotatePoint->y = 1.5 * (ellipse->pointarray[2]->y) - 0.5 * Obj->CenterPoint->y;
}

void DrawOriginalEllipse(struct obj *Obj)
{
    double a, b;
    double cx, cy;
    double x0, x1, x2, y0, y1, y2;
    double X, Y, x, y;
    double angle0, angle = 0;
    double radius;
    struct _2DCurve *ellipse = Obj->objPointer;

    x0 = ellipse->pointarray[0]->x;
    y0 = ellipse->pointarray[0]->y;
    x1 = ellipse->pointarray[1]->x;
    y1 = ellipse->pointarray[1]->y;
    x2 = ellipse->pointarray[2]->x;
    y2 = ellipse->pointarray[2]->y;
    cx = Obj->CenterPoint->x;
    cy = Obj->CenterPoint->y;

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
        X = cx + radius * cos(angle + angle0);
        Y = cy + radius * sin(angle + angle0);
        DrawLine(X-x, Y-y);
        x = X;
        y = Y;
    }
    DrawLine(x1-X, y1-Y);
}

bool CheckEllipse(struct obj *Obj)
{
    struct _2DCurve *ellipse = Obj->objPointer;
    double cx, cy, a, b;
    double x, y;
    double cos0, sin0;

    cx = Obj->CenterPoint->x;
    cy = Obj->CenterPoint->y;
    a = sqrt(pow(ellipse->pointarray[0]->x - ellipse->pointarray[1]->x, 2) + pow(ellipse->pointarray[0]->y - ellipse->pointarray[1]->y, 2)) / 2;
    b = a * ellipse->ratio;
    x = CurrentPoint->x - cx;
    y = CurrentPoint->y - cy;
    cos0 = (ellipse->pointarray[1]->x - ellipse->pointarray[0]->x)/(2*a);
    sin0 = (ellipse->pointarray[1]->y - ellipse->pointarray[0]->y)/(2*a);
    return pow(x*cos0 + y*sin0, 2)/(a*a) + pow(x*sin0 - y*cos0, 2)/(b*b) <= 1;   
}

void MoveEllipse(struct obj *Obj, double dx, double dy)
{
    int i;

    for (i = 0; i < ((struct _2DCurve *) Obj->objPointer)->PointNum; i++) {
        ((struct _2DCurve *) Obj->objPointer)->pointarray[i]->x += dx;
        ((struct _2DCurve *) Obj->objPointer)->pointarray[i]->y += dy;
    }
}

void RotateEllipse(void)
{
    struct _2DCurve *ellipse = RegisterP->RegisterObj[RegisterP->ActiveOne]->objPointer;
    int pointnum = ellipse->PointNum;
    int i;
    double cx = RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->x;
    double cy = RegisterP->RegisterObj[RegisterP->ActiveOne]->CenterPoint->y;

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
            ZoomText(Obj, zoom);
            break;
        case LINE:
            Zoom_2DhasEdge(Obj, zoom);
            break;
        case RECTANGLE:
            Zoom_2DhasEdge(Obj, zoom);
            break;
        case ELLIPSE:
            ZoomEllipse(Obj, zoom);
            break;
        case LOCUS:
            break;
    }
    RefreshAndDraw();
}

void Zoom_2DhasEdge(struct obj *Obj, int zoom)
{
    struct _2DhasEdge *polygon = Obj->objPointer;
    int i, pointnum;
    double cx, cy;

    cx = Obj->CenterPoint->x;
    cy = Obj->CenterPoint->y;
    pointnum = polygon->PointNum;
    for (i = 0; i < pointnum; i++) {
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
    struct _2DCurve *polygon = Obj->objPointer;
    int i, pointnum;
    double cx, cy;

    cx = Obj->CenterPoint->x;
    cy = Obj->CenterPoint->y;
    pointnum = polygon->PointNum;
    for (i = 0; i < pointnum; i++) {
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
    struct _2DText *text = GetBlock(sizeof(struct _2DText));
    int i;

    Register(text, TEXT);
    text->PointNum = 4;
    text->RelationMatrix = RectangleMatrix;
    for (i = 0; i < text->PointNum; i++) {
        (text->pointarray)[i] = GetBlock(sizeof(struct Point));
    }
    (text->pointarray)[0]->x = CurrentPoint->x;
    (text->pointarray)[0]->y = CurrentPoint->y;
    
    text->FontSize = GetPointSize();
    text->TextArray = GetBlock( MAX_TEXT_LENGTH * sizeof(char));
    for (i = 0; i < MAX_TEXT_LENGTH; i++) {
        text->TextArray[i] = '\0';
    }
    text->CursorIndex = 0;
    text->CursorPosition = GetBlock(sizeof(struct Point));
    text->CursorPosition->x = CurrentPoint->x;
    text->CursorPosition->y = CurrentPoint->y - GetFontHeight();
    text->isCursorBlink = TRUE;
    text->isDisplayCursor = FALSE;
}


void GetTextFrameShape(void)
{
    struct _2DText *text = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;

    if (CurrentPoint->x > text->pointarray[0]->x) text->pointarray[2]->x = CurrentPoint->x;
    else text->pointarray[2]->x = text->pointarray[0]->x;
    text->pointarray[2]->y = text->pointarray[0]->y - GetFontHeight() - ROW_HEIGHT_MODIFY;
    text->pointarray[1]->x = text->pointarray[2]->x;
    text->pointarray[1]->y = text->pointarray[0]->y;
    text->pointarray[3]->x = text->pointarray[0]->x;
    text->pointarray[3]->y = text->pointarray[2]->y;
}

void Draw_2DText(void)
{
    int i, j, pointnum;
    struct _2DText *obj = (RegisterP->RegisterObj)[RegisterP->ActiveOne]->objPointer;
    struct obj *Obj = (RegisterP->RegisterObj)[RegisterP->ActiveOne];
    bool EraseMode;

    EraseMode = GetEraseMode();
    SetPenColor((RegisterP->RegisterObj)[RegisterP->ActiveOne]->color);
    Obj->CenterPoint->x = 0;
    Obj->CenterPoint->y = 0;
    pointnum = obj->PointNum;
    for (i = 0; i < pointnum; i++) {
         Obj->CenterPoint->x += obj->pointarray[i]->x;
         Obj->CenterPoint->y += obj->pointarray[i]->y;
        for (j = i; j < pointnum; j++) {
            if (*(obj->RelationMatrix +pointnum*i+j)) {
                if (!isSelectSelf) {
                    SetEraseMode(TRUE);
                    DrawLineByPoint(obj->pointarray[i], obj->pointarray[j]);
                    SetEraseMode(FALSE);
                } else DrawDottedLine(obj->pointarray[i], obj->pointarray[j]);
            }
        }
    }
    Obj->CenterPoint->x /= obj->PointNum;
    Obj->CenterPoint->y /= obj->PointNum;
    if (Obj->color == SELECT_COLOR || isMouseDownMoving) {
        startTimer(CURSOR_BLINK, CURSOR_BLINK_TIME);
    }
    DrawPureText(obj);
    SetEraseMode(EraseMode);
}

void DrawPureText(struct _2DText *text)
{
    bool EraseMode = GetEraseMode();
    char *color = GetPenColor();
    int PointSize = GetPointSize();

    SetPointSize(text->FontSize);
    SetPenColor(DEFAULT_COLOR);
    SetEraseMode(FALSE);
    MovePen(text->pointarray[0]->x, text->pointarray[0]->y - GetFontHeight());
    DrawTextString(text->TextArray);
    SetEraseMode(TRUE);
    SetPenColor(color);
    SetPointSize(PointSize);
}

void DisplayCursor(double x, double y)
{
    int PenSize = GetPenSize();

    SetPenSize(CURSOR_SIZE);
    MovePen(x, y);
    DrawLine(0, GetFontHeight());
    MovePen(x, y);
    SetPenSize(PenSize);
}

bool CheckText(struct obj *Obj)
{
    struct _2DText *text = Obj->objPointer;

    return CheckInsidePolygon(text->pointarray, Obj->CenterPoint, text->RelationMatrix, text->PointNum);
}

double CharWide(char ch)
{
    return TextStringWidth(CharToString(ch));
}

void MoveText(struct obj *Obj, double dx, double dy)
{
    int i;
    struct _2DText *text = Obj->objPointer;

    for (i = 0; i < text->PointNum; i++) {
        text->pointarray[i]->x += dx;
        text->pointarray[i]->y += dy;
    }
    text->CursorPosition->x += dx;
    text->CursorPosition->y += dy;
}

void InsertChar(char *array, char element, int index, int length)
{
    char *ArrayCopy = GetBlock(length * sizeof(char));
    int i;

    for (i = 0; i < length; i++) {
        if (array[i] == '\0') break;
    }
    if (i + 1 >= length) return;
    strcpy(ArrayCopy, array + index);
    array[index] = element;
    strcpy(array + index + 1, ArrayCopy);
    free(ArrayCopy);
}

void ZoomText(struct obj *Obj, int zoom)
{
    //struct _2DText *text = Obj->objPointer;
    //int PointSize = GetPointSize();

    /*switch (zoom) {
        case ZOOM_IN:
            text->pointarray[0]->y = text->pointarray[3]->y +(1 - SIZE_FACTOR) * (text->pointarray[0]->y - text->pointarray[3]->y);
            text->pointarray[2]->x = text->pointarray[3]->x +(1 - SIZE_FACTOR) * (text->pointarray[2]->x - text->pointarray[3]->x);
            text->pointarray[1]->y = text->pointarray[0]->y;
            text->pointarray[1]->x = text->pointarray[2]->x;
            text->FontSize--;
            break;
        case ZOOM_OUT:
            text->pointarray[0]->y = text->pointarray[3]->y +(1 + SIZE_FACTOR) * (text->pointarray[0]->y - text->pointarray[3]->y);
            text->pointarray[2]->x = text->pointarray[3]->x +(1 + SIZE_FACTOR) * (text->pointarray[2]->x - text->pointarray[3]->x);
            text->pointarray[1]->y = text->pointarray[0]->y;
            text->pointarray[1]->x = text->pointarray[2]->x;
            text->FontSize++;
            break;
    }*/
    
}

void PlaceHolder(void)
{
    //no operation
}
