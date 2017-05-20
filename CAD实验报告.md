#                                   												实验报告 #

* 组员:陈佳伟,赵梦雨,凌添翼.

* 本程序支持基本的功能:

  * 绘制,选中,移动,__旋转__,删除矩形
  * 绘制,选中,移动,__旋转__,删除直线
  * 绘制,选中,移动,__旋转__,删除椭圆
  * 绘制文本框,打字并利用左右键移动光标,删除或退格,在中间__插入__文字.
    * 选中文本框,移动文本框以及文字,删除文本

* 操作指南

  * 打开 exe 可执行文件后,按下 `F1` 弹出 Console ,输入 Console 中对应的__序号__并按下回车,如图:

   <img src="D:\桌面\2017-05-05 14 19 08.png\" width="600ppx"\>
    依次这样跟随指示进行操作.

  * 当选择画图时,在窗口点击鼠标左键并拖拽形成图形(与正常交互逻辑一致).
     * 注意:在画椭圆时,鼠标拖拽会画出椭圆,按住不放相对水平轴上移会增大椭圆竖直轴(增大b/a);反之,则会减小 `b/a`.
  * 当选择打字时,在窗口点击鼠标左键并拖拽形成文本框,然后进行输入.
  * 当选择操纵图形时,鼠标左键单击鼠标选中图形(此时图形或文本框会以__红色__显示),然后在图形范围内左键按下拖拽即可__移动__.或者直接点击并拖拽即可移动.如果想__旋转__图形,鼠标左键拖拽图形上由虚线连接的__小圆 环__,如图:
     <img src="D:\桌面\2017-05-05 14 37 40.png\" width="300ppx"\>

    图形即可随鼠标移动而旋转.如果想放缩图形,只需在选中图形状态下,滚动鼠标滚轮即可(文本暂不支持放缩).

  *   想要删除图形,选中后,按下 `Ctrl+Delete` 即可.

* 特色功能:
  * 当移动某图形经过另一图形时,被经过的图形不会被擦去.

* 有待提高的地方:

  * 屏幕闪烁:
    * 由于没有采用__双缓冲__方法,导致窗口作图时,不可避免闪烁.考虑到双缓冲方法要求利用 MFC ,且已经超过教学的要求,并且受制于时间,也就不将这一改进加入程序中.
  * 在选择基本模式时,考虑到直接画按钮与程序不太和谐,而又不会使用 Windows API 画按钮,只能退而求其次使用 Console 进行人机交互而不是用鼠标选择.
  * 在本程序中,对象的结构存储过于复杂, `struct Obj` 与具体的图形的结构完全可以合并成一个结构,并通过宏定义的方式模拟"继承"的特点.但由于时间所限,想到这一改进时已经太晚,就不再修改程序.

* 程序的模块化框架

  * 库:只使用了教材库和老师提供的库

  * 本程序用 `struct` 结构来表示对象

    * 用以下结构管理所有画在窗口的对象:

      ```c
      struct RegisterADT {
          struct obj *RegisterObj[MAXOBJ];//存储对象的数组,对象由指向另一 struct 的指针来表示.
          int ObjNum; //用于存储对象的总数
          int ActiveOne;//用来存储正在处理(激活的)的对象在 RegisterObj 中的下标
      };
      ```

    * 每个点的坐标用一下结构表示:

      ```c
      struct Point {
          double x;
          double y;
      };
      ```

    * 用以下结构表示每一个对象共有的属性,而不论它们是矩形,直线,椭圆,还是文本:

      ```c
      struct obj {
          void *objPointer;//指向具体类型(如矩形,椭圆,直线,文本)的指针
          int DrawType;//指明该对象是什么类型的
          string color;//指明下次画这个对象时用什么颜色
          struct Point *RotatePoint;//存储旋转拖拽点的坐标信息.当用户的鼠标拖拽该点时,对象会随着鼠标的移动绕中心点旋转
          struct Point *CenterPoint;//存储该对象中心点的坐标信息,中心点是对象的旋转轴
      };
      ```

      * 其中 `DrawType` 指明该对象是什么类型的,可选类型有:

        ```c
        typedef enum {
            NO_TYPE,
            TEXT,//文本
            LINE,//直线
            RECTANGLE,//矩形
            ELLIPSE//椭圆
        } _2DDrawType;
        ```

      * 其中 `color` 的取值范围是:
        ```c
        #define DEFAULT_COLOR "Black"
        #define SELECT_COLOR "Red"
        #define POINT_COLOR "Magenta"
        ```

    * 二维平面有边图形(直线,矩形)的结构:

      ```c
      struct TwoDhasEdge {
          struct Point *pointarray[MAXPOINT];//存储该图形顶点的坐标
          int PointNum;//存储图形顶点的个数
          bool *RelationMatrix; //利用离散数学关系矩阵的方法,用布尔矩阵指明该图形各顶点之间的连线关系
      };
      ```

      * 矩形的关系矩阵为:

        ```c
        const bool RectangleMatrix[] = {
            0, 1, 0, 1,
            1, 0, 1, 0,
            0, 1, 0, 1,
            1, 0, 1, 0
        };// 1 意味着行列对应两点相连,0 意味着行列对应两点不相连,具体原理见附件A
        ```

      * 直线的关系矩阵为:

        ```c
        const bool LineMatrix[] = {
            0, 1,
            1, 0
        };
        ```

    * 二维平面椭圆的结构:

      ```c
      struct TwoDCurve {
          struct Point *pointarray[MAXPOINT]; //椭圆只需要三个顶点就可以确定其位置
          int PointNum;
          double ratio;//存储椭圆纵轴b和横轴a的比例
      };
      ```

    * 二维平面文本的结构:

      ```c
      struct TwoDText {
          struct Point *pointarray[MAXPOINT];
          int PointNum;
          bool *RelationMatrix;
      //以上是文本框(矩形的信息)
          char *TextArray;//指向文本内容的指针
          struct Point *CursorPosition;//存储光标在图形界面中的位置
          int CursorIndex;//存储光标在文本中的位置
          bool isCursorBlink;//判断光标是否闪烁
      };
      ```

  * 综上,结构图为:![](file:///D:\图片\文件_000.jpeg) 

  * 用 `void MouseEventProcess(int x, int y, int button, int event)` 处理鼠标信息.

  * 用 `void KeyboardEventProcess(int key,int event)` 和 `void CharEventProcess(char c)` 处理键盘信息.

  * 用 `void TimerEventProcess(int timerID)` 使用计时器处理光标闪烁.

  * 绘制图形的大致步骤:

    * 鼠标左键摁下时,调用 `void InitRectangle(void)` ,`void InitEllipse(void)` ,`void InitLine(void)` 和 `void InitText(void)` 为对象开辟内存,并初始化对象的基本信息(如顶点,颜色,关系矩阵等)
    * 鼠标左键按下并移动时,实时画出图像.
      * 对于直线,矩形以及文本的文本框,直接遍历对应的 `RelationMatrix`,如果某个行列对应的值为1,那么就将行列对应的点连接起来;否则不连.__这里之所以没有使用更简便的方法绘制图形,主要考虑了函数的通用性,只需要调用这个函数就可以画出所有二维平面的多边形(当然也包括直线).__
      * 具体 `RelationMatrix` 的数学知识见附录A.
    * 鼠标左键弹起时判定绘图完成.

  * 其他操作的具体实现步骤略过,只介绍代码的技术难点:

    * 如何实现移动,旋转,放缩图形时,不会擦除被覆盖的图形?

      * 每次改变对象形状时,都对全屏进行刷新,并依据存储在结构中的对象信息全部重绘图像.

    * 如何进行窗口整体刷新?

      * 由于初次设计时,老师尚未提供有关函数,于是我自己写了一个 `void RefreshDisplay(void)`,基本原理是用擦除模式画一个和窗口一样大的矩形,然后在橡皮擦模式下填充它.

    * 如何改变代码中 `switch` 套 `switch` 结构导致让人不容易看清的"火箭型"代码?

      * 把第二三层 `switch` 写入单独的函数,并传入函数指针进行调用(此处可以进一步修改为宏或者 `inline` 函数减少调用函数的开销),如:

        ```c
        static void ChooseButton(void (*left1)(void), void (*left2)(void), int button);
        static void ChooseDrawWhat(void (*text)(void), 
                            	   void (*line)(void),
                            	   void (*rectangle)(void), 
                            	   void (*ellipse)(void));
        static void ChooseMode(void (*draw)(void), void (*operate)(void));
        ```

    * 如何画出"歪"的椭圆?

      * 由于教材库只给出了画"位置端正"的椭圆的函数,于是我自己重写了一个__一般位置椭圆__的函数:

        ```c
        void DrawOriginalEllipse(struct obj *Obj);
        ```

        基本思想是将椭圆这一对象中存储的三个顶点经过线性代数中的二维平移矩阵,二维旋转矩阵$[\begin{smallmatrix}cos\theta&-sin\theta \\ sin\theta&cos\theta\end{smallmatrix}]$的操作重新存储.并画"歪"的椭圆.画椭圆时,每隔 2° 进行采样,从集合中心到椭圆上的点的距离由椭圆极坐标形式(原点在椭圆中心) ${r}^2({b}^2{cos}^2\theta+{a}^2{sin}^2\theta)={a}^2{b}^2$ 并稍加处理给出,以直代曲作出椭圆轮廓.

    * 如何判断鼠标坐标在"歪"的椭圆之内?

      * 这里并没有用一个套住椭圆的矩形框作为识别区域,而是由__一般位置椭圆方程__判断(其中$({x}_c, {y}_c)$是椭圆中心坐标, $\theta$ 是椭圆的倾斜角), 当中心点和鼠标位置在同侧时说明鼠标坐标在椭圆内:
        $$
        \frac{{\biggr((x-{x}_c)cos\theta+(y-{y}_c)sin\theta\biggr)}^2}{{a}^2}+\frac{{\biggr(-(x-{x}_c)sin\theta+(y-{y}_c)cos\theta\biggr)}^2}{{b}^2}=1
        $$

    * 如何判断用户选中矩形时,鼠标落在("歪的")矩形内?

      * 利用平面解析几何知识,将矩形四条边看成平面上四条直线.每条直线将平面划分为两个区域.鼠标坐标点分别代入四条直线的一般方程 $Ax + By + C = 0$ 的左式 $Ax + By + C$,会得到一组数.当鼠标点在矩形内时,这组数和矩形的中心点坐标代入四条直线所得到的一组数的正负号对应完全一致.由此可以判断鼠标是否在矩形内,类似地,可以判断鼠标是否在椭圆内.

        这里用到的函数是:

        ```c
        bool CheckInsidePolygon(struct Point **select_point, struct Point *ReferencePoint, bool *RelationMatrix, int PointNum);

        bool CheckEllipse(struct obj *Obj);
        ```

    * 如何判断用户用鼠标选中了直线(因为直线很细,难以选中)?

      * 在直线外再加一个透明矩形(会随直线旋转),用上述方法判断鼠标是否落在透明矩形范围内.

        这里用的函数:

        ```c
        bool CheckLine(struct obj *Obj);
        ```

    * 如何实现的图形放缩?

      * 将图形中的每个点相对于中心点同比例远离,并同时更新 `RotatePoint`.

    * 移动图形时有两种情况:一种是已经选中了,然后在拖拽;另一种是直接点击拖拽,如何处理?

      * 通过设置全局变量来进行判断(太复杂,具体略去不表).

    * 如何在设计程序时进行修改?

      * 使用 Github 进行版本控制与分支合并.

* 程序设计心得体会

  ​	此程序部分采用了面向对象, 事件驱动的思想,将每个图形都看成一个个__对象__来处理.每个具体图形都__继承__了一般图形的特点(有颜色,有 `RoatePoint`,有 `CenterPoint` ). 并且同一函数(如 `void Zoom(struct obj *Obj, int zoom)` )处理不同的对象(__多态__).不过使用 C 语言实现面向对象编程障碍颇多,此程序大概也只能算做是"拙劣的模仿".做完该程序后,刚好看到 "Python源码剖析"这本书,才知道继承可以用宏来模拟,将宏放在结构开始并用强制转换来实现多态,但由于时间所限也就不改程序了.

  ​	对于 Windows 编程,由于对 MFC 等不熟悉,只是采用老师及教材提供并且封装的函数,这样也极大地限制了编程.

* 附录A

  ​	有边图形的 Relation Matrix(以三阶为例,这意味着这个图形有三个顶点):

  ​	
  $$
  \mathbf{R} = \begin{bmatrix}
  c_{11} & c_{12} & c_{13}\\
  c_{21} & c_{22} & c_{23}\\
  c_{31} & c_{32} & c_{33}\\
  \end{bmatrix}
  $$
  ​	其中 $$c_{ij} \in \{0, 1\}$$, 当 $$c_{ij}$$ 取 0 时,表示第 i个顶点和第 j 个顶点之间没有连线,反之则有连线.易知,此程序的矩阵都是__实对称矩阵__.

  ​	例如矩形的 Relation Matrix 为:
  $$
  \mathbf{R}_{rectangle} = \begin{bmatrix}
  0 & 1 & 0 & 1 \\
  1 & 0 & 1 & 0 \\
  0 & 1 & 0 & 1 \\
  1 & 0 & 1 & 0 \\
  \end{bmatrix}
  $$





  ​	而直线的 Relation Matrix 为:
$$
  \mathbf{R}_{line} = \begin{bmatrix}
  0 & 1 \\
  1 & 0 \\
  \end{bmatrix}
$$
