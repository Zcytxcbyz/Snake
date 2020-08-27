#include "framework.h"
#include "Snake.h"


#define ALLOWACCELERATE 0
#define MAX_LOADSTRING 100
#define BLOCKSIZE 10
#define XOFFECT 30
#define YOFFECT 30
#define XCOFFECT 22
#define YCOFFECT 22
#define SNAKE 1
#define FOOD 2
#define WALL 3
#define INITLEN 2

HINSTANCE hInst;                         
HWND m_hWnd;                             
WCHAR szTitle[MAX_LOADSTRING];           
WCHAR szWindowClass[MAX_LOADSTRING];      
CRect ClientRect;                        
int** PixelsMatrix;
int PixelsMatrixWidth, PixelsMatrixHeight;
int MarkNum = 0;
std::deque<CPoint> SnakeBody;
std::list<CPoint> FoodData;
enum class Direction { up, down, left, right };
bool GameRunning = false;
std::mutex mtx;
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void                Paint();
void                FoodGen();
void                FoodDataSet();
void                Initializate();
void                Move();
CPoint              GetFront();
CPoint              GetBack();
int*                GetPointPtr(CPoint);
bool                AutoObstacle(CPoint);
Direction           GetDirection();
void                Front();
void                Up();
void                Down();
void                Left();
void                Right();
void                Reset();
LONG WINAPI         MyUnhandledExceptionFilter(EXCEPTION_POINTERS* exp);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SNAKE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNAKE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SNAKE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; 
   int srcWidth = GetSystemMetrics(SM_CXSCREEN);
   int srcHeight = GetSystemMetrics(SM_CYSCREEN);
   int xWidth = 700;
   int yHeight = 500;
   int xLeft = (srcWidth - xWidth) / 2;
   int yTop = (srcHeight - yHeight) / 2;
   HWND hWnd = CreateWindowW(
       szWindowClass,
       szTitle,
       WS_CAPTION | WS_SYSMENU ,
       xLeft, 
       yTop, 
       xWidth, 
       yHeight, 
       nullptr, 
       nullptr,
       hInstance, 
       nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   m_hWnd = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDC_STARTBTN:
        {
            DestroyWindow(GetDlgItem(hWnd, IDC_STARTBTN));
            PixelsMatrixWidth = (ClientRect.Width() - XOFFECT - XCOFFECT) / BLOCKSIZE;
            PixelsMatrixHeight = (ClientRect.Height() - YOFFECT - YCOFFECT) / BLOCKSIZE;
            srand(time(0));
            Initializate();
            GameRunning = true;
            std::thread tpaint(Paint);
            tpaint.detach();
            std::thread tmove(Move);
            tmove.detach();
			std::thread tfods(FoodDataSet);
			tfods.detach();
            std::thread tfood(FoodGen);
            tfood.detach();
        }
        break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_KEYDOWN:
    {
        if (GameRunning) 
        {
			switch (wParam)
			{
            case VK_UP: 
            {
                std::thread t(Up); 
                t.detach();
            } 
            break;
			case VK_DOWN: 
            {
				std::thread t(Down);
				t.detach();
			}
            break;
			case VK_LEFT:
			{
				std::thread t(Left);
				t.detach();
			}
            break;
			case VK_RIGHT:
			{
				std::thread t(Right);
				t.detach();
			}
            break;
			case 'W':
			{
				std::thread t(Up);
				t.detach();
			}
            break;
			case 'S':
			{
				std::thread t(Down);
				t.detach();
			}
            break;
			case 'A':
			{
				std::thread t(Left);
				t.detach();
			}
            break;
			case 'D':
			{
				std::thread t(Right);
				t.detach();
			}
            break;
			}
        }
    }
    break;
    case WM_CREATE:
    {
        GetClientRect(hWnd, ClientRect);
        int Btn_Width = 200;
        int Btn_Height = 100;
        int Btn_Left = (ClientRect.Width() - Btn_Width) / 2;
        int Btn_Top = (ClientRect.Height() - Btn_Height) / 2;
        HFONT hFont = CreateFont(60, 0, 0, 0, 0, FALSE, FALSE,
            0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("微软雅黑"));
        HWND hBtn = CreateWindow(_T("Button"), _T("开 始"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            Btn_Left, Btn_Top, Btn_Width, Btn_Height, hWnd,
            (HMENU)IDC_STARTBTN, hInst, NULL);
        SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
    break;
    case WM_ERASEBKGND:
        return TRUE;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void Paint()
{
    while (GameRunning)
    {
        HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
        HBRUSH greenBrush = CreateSolidBrush(RGB(0, 255, 0));
        HBRUSH blackBrush = CreateSolidBrush(BLACK_BRUSH);
        HPEN nullPen = CreatePen(PS_NULL, 0, NULL);
        HDC DC = GetDC(m_hWnd);
        HBRUSH bkBrush = CreateSolidBrush(GetBkColor(DC));
        HDC dcMemory = CreateCompatibleDC(DC);
        HBITMAP bmp = CreateCompatibleBitmap(DC, ClientRect.Width(), ClientRect.Height());
        SelectObject(dcMemory, bmp);
        FillRect(dcMemory, ClientRect, bkBrush);
        SelectObject(dcMemory, nullPen);
        for (int i = 0; i < PixelsMatrixHeight; ++i) {
            for (int j = 0; j < PixelsMatrixWidth; ++j) {
                switch (PixelsMatrix[i][j])
                {
                case NULL: SelectObject(dcMemory, bkBrush); break;
                case SNAKE: SelectObject(dcMemory, greenBrush); break;
                case FOOD: SelectObject(dcMemory, redBrush); break;
                case WALL: SelectObject(dcMemory, blackBrush); break;
                }
                Rectangle(dcMemory, j * BLOCKSIZE + XOFFECT, i * BLOCKSIZE + YOFFECT,
                    (j + 1) * BLOCKSIZE + 1 + XOFFECT, (i + 1) * BLOCKSIZE + 1 + YOFFECT);
            }
        }
        DeleteObject(greenBrush);
        DeleteObject(redBrush);
        DeleteObject(bkBrush);
        DeleteObject(nullPen);
        CString str;
        str.Format(_T("%d"), MarkNum);
        SetTextAlign(dcMemory, TA_RIGHT);
        TextOut(dcMemory, ClientRect.right - 5, 5, str, str.GetLength());
        BitBlt(DC, 0, 0, ClientRect.Width(), ClientRect.Height(), dcMemory, 0, 0, SRCCOPY);
        ReleaseDC(m_hWnd, DC);
        DeleteDC(dcMemory);
        DeleteObject(bmp);
        Sleep(13);
    }
}

void FoodGen()
{
    while (GameRunning) 
    {
        if (FoodData.empty())
        {
            int count = rand() % 5 + 1;
            for (int i = 0; i < count; ++i)
            {
                int x = rand() % (PixelsMatrixWidth - 1) + 1;
                int y = rand() % (PixelsMatrixHeight - 1) + 1;
                while (PixelsMatrix[y][x] != NULL)
                {
                    x = rand() % (PixelsMatrixWidth - 1) + 1;
                    y = rand() % (PixelsMatrixHeight - 1) + 1;
                }
                mtx.lock();
                PixelsMatrix[y][x] = FOOD;
                FoodData.push_back(CPoint(x, y));
                mtx.unlock();
            }
        }
    }
}
void FoodDataSet() 
{
    while (GameRunning)
    {
        for (auto iter = FoodData.begin(); iter != FoodData.end(); )
        {
            if (PixelsMatrix[iter->y][iter->x] != FOOD)
            {
                mtx.lock();
                FoodData.erase(iter++);
                ++MarkNum;
                CPoint pback = GetBack();
                SnakeBody.push_back(pback);
                *GetPointPtr(pback) = SNAKE;
                mtx.unlock();
            }
            else
            {
                ++iter;
            }
        }
    }
}
void Initializate()
{
    PixelsMatrix = new int* [PixelsMatrixHeight];
	for (int i = 0; i < PixelsMatrixHeight; ++i) {
		PixelsMatrix[i] = new int[PixelsMatrixWidth];
		for (int j = 0; j < PixelsMatrixWidth; ++j) {
			if (i == 0 || i == PixelsMatrixHeight - 1 || j == 0 || j == PixelsMatrixWidth - 1)
			{
				PixelsMatrix[i][j] = WALL;
			}
			else
			{
				PixelsMatrix[i][j] = NULL;
			}
		}
	}
    int CenterAreaLeft = PixelsMatrixWidth / 2 - 10;
    int CenterAreaRight = PixelsMatrixWidth / 2 + 10;
	int CenterAreaTop = PixelsMatrixHeight / 2 - 10;
	int CenterAreaBottom = PixelsMatrixHeight / 2 + 10;
    int StartX = rand() % 20 + CenterAreaLeft;
    int StartY = rand() % 20 + CenterAreaTop;
    int EndX, EndY;
    if (rand() % 2) 
    {
        EndX = StartX + INITLEN;
        EndY = StartY;
    }
    else 
    {
        EndX = StartX;
        EndY = StartY + INITLEN;
    }
    for (int i = StartX; i <= EndX; ++i) {
		for (int j = StartY; j <= EndY; ++j) {
            SnakeBody.push_back(CPoint(i, j));
		}
    }
    for (auto iter = SnakeBody.begin(); iter != SnakeBody.end(); ++iter) 
    {
        PixelsMatrix[iter->y][iter->x] = SNAKE;
    }
}

void Move()
{
    while (GameRunning)
    {
        CPoint pfront = GetFront();
        CPoint pback = SnakeBody.back();
        if (AutoObstacle(pfront)) return;
        mtx.lock();
        *GetPointPtr(pfront) = SNAKE;
        *GetPointPtr(pback) = NULL;
        SnakeBody.push_front(pfront);
        SnakeBody.pop_back();
        mtx.unlock();
        Sleep(250);
    }
}

CPoint GetFront()
{
    std::deque<CPoint>::iterator first = SnakeBody.begin();
    std::deque<CPoint>::iterator second = SnakeBody.begin() + 1;
    return CPoint(2 * first->x - second->x, 2 * first->y - second->y);
}

CPoint GetBack()
{
    std::deque<CPoint>::iterator first = SnakeBody.end() - 1;
    std::deque<CPoint>::iterator second = SnakeBody.end() - 2;
    return CPoint(2 * first->x - second->x, 2 * first->y - second->y);
}

int* GetPointPtr(CPoint point)
{
    return &PixelsMatrix[(int)point.y][(int)point.x];
}

bool AutoObstacle(CPoint front)
{
    int* pfront = GetPointPtr(front);
    if (*pfront == SNAKE || *pfront == WALL) 
    {
        KillTimer(m_hWnd, IDC_MOVETIMER);
        KillTimer(m_hWnd, IDC_DRAWTIMER);
        mtx.lock();
        GameRunning = false;
        mtx.unlock();
        CString str;
        str.Format(_T("游戏结束。分数: %d"), MarkNum);
        int MBRESULT = MessageBox(NULL, str, _T("贪吃蛇"), MB_OK);
        if (MBRESULT == IDOK) 
        {
            Reset();
        }
        return true;
    }
    return false;
}

Direction GetDirection()
{
	std::deque<CPoint>::iterator first = SnakeBody.begin();
	std::deque<CPoint>::iterator second = SnakeBody.begin() + 1;
    if (first->x == second->x) {
        if (first->y > second->y) 
        {
            return Direction::down;
        }
        else 
        {
            return Direction::up;
        }
    }
    if (first->y == second->y){

		if (first->x > second->x)
		{
			return Direction::right;
		}
		else
		{
			return Direction::left;
		}
    }
}
#if ALLOWACCELERATE
void Front() 
{
	CPoint pfront = GetFront();
	CPoint pback = SnakeBody.back();
	if (AutoObstacle(pfront, pback)) return;
	*GetPointPtr(pfront) = SNAKE;
	*GetPointPtr(pback) = NULL;
    SnakeBody.push_front(pfront);
	SnakeBody.pop_back();
}
#endif
void Up()
{
    if (GetDirection() == Direction::left || GetDirection() == Direction::right)
    {
		CPoint pfront = CPoint(SnakeBody.front().x, SnakeBody.front().y - 1);
		CPoint pback = SnakeBody.back();
        if (AutoObstacle(pfront)) return;
        mtx.lock();
		*GetPointPtr(pfront) = SNAKE;
		*GetPointPtr(pback) = NULL;
        SnakeBody.push_front(pfront);
		SnakeBody.pop_back();
        mtx.unlock();
    }
#if ALLOWACCELERATE
    else if (GetDirection() == Direction::up) 
    {
        Front();
    }
#endif
}

void Down()
{
	if (GetDirection() == Direction::left || GetDirection() == Direction::right)
	{
		CPoint pfront = CPoint(SnakeBody.front().x, SnakeBody.front().y + 1);
		CPoint pback = SnakeBody.back();
        if (AutoObstacle(pfront)) return;
        mtx.lock();
		*GetPointPtr(pfront) = SNAKE;
		*GetPointPtr(pback) = NULL;
        SnakeBody.push_front(pfront);
		SnakeBody.pop_back();
        mtx.unlock();
	}
#if ALLOWACCELERATE
	else if (GetDirection() == Direction::down)
	{
        Front();
	}
#endif
}

void Left()
{
	if (GetDirection() == Direction::up || GetDirection() == Direction::down)
	{
        CPoint pfront = CPoint(SnakeBody.front().x - 1, SnakeBody.front().y);
		CPoint pback = SnakeBody.back();
        if (AutoObstacle(pfront)) return;
        mtx.lock();
		*GetPointPtr(pfront) = SNAKE;
		*GetPointPtr(pback) = NULL;
        SnakeBody.push_front(pfront);
		SnakeBody.pop_back();
        mtx.unlock();
	}
#if ALLOWACCELERATE
	else if (GetDirection() == Direction::left)
	{
        Front();
	}
#endif
}

void Right()
{
	if (GetDirection() == Direction::up || GetDirection() == Direction::down)
	{
		CPoint pfront = CPoint(SnakeBody.front().x + 1, SnakeBody.front().y);
		CPoint pback = SnakeBody.back();
        if (AutoObstacle(pfront)) return;
        mtx.lock();
		*GetPointPtr(pfront) = SNAKE;
		*GetPointPtr(pback) = NULL;
        SnakeBody.push_front(pfront);
		SnakeBody.pop_back();
        mtx.unlock();
	}
#if ALLOWACCELERATE
	else if (GetDirection() == Direction::right)
	{
        Front();
	}
#endif
}

void Reset()
{
    free(PixelsMatrix);
    MarkNum = 0;
    FoodData.clear();
    SnakeBody.clear();
    srand(time(0));
	Initializate();
	GameRunning = true;
	std::thread tpaint(Paint);
	tpaint.detach();
	std::thread tmove(Move);
	tmove.detach();
	std::thread tfods(FoodDataSet);
	tfods.detach();
	std::thread tfood(FoodGen);
	tfood.detach();
}

LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS* exp)
{
    FatalAppExit(0, _T("An unknown error occurred."));
    return EXCEPTION_EXECUTE_HANDLER;
}

