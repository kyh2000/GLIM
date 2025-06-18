
// MFCApplication3Dlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication3.h"
#include "MFCApplication3Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
CPoint m_clickPoint;
bool m_bClicked = false;
int count = 0;
int m_nSelectedPointIndex = -1;
CString Radius;
CString circle;
static UINT RandomMoveThread(LPVOID pParam)
{
	CMFCApplication3Dlg* pDlg = reinterpret_cast<CMFCApplication3Dlg*>(pParam);
	if (!pDlg) return 1;

	CRect rect;
	pDlg->GetClientRect(&rect);

	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < pDlg->m_clickPoints.GetSize(); ++j)
		{
			int x = rand() % rect.Width();
			int y = rand() % rect.Height();
			pDlg->m_clickPoints[j] = CPoint(x, y); 
		}

		pDlg->Invalidate(TRUE); // false로 할시 모든 정원 남아있음. 정상작동 확인
		Sleep(500);
	}

	return 0;
}

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.
void DrawCircleThrough3Points(CPaintDC& dc, CPoint p1, CPoint p2, CPoint p3, int penWidth)
{
	// 3점 좌표 (double 형 변환)
	double x1 = p1.x, y1 = p1.y;
	double x2 = p2.x, y2 = p2.y;
	double x3 = p3.x, y3 = p3.y;

	double d = 2 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));
	if (fabs(d) < 1e-6) return; // 세 점이 일직선상에 있으면 그리지 않음

	double cx = ((x1 * x1 + y1 * y1) * (y2 - y3) + (x2 * x2 + y2 * y2) * (y3 - y1) + (x3 * x3 + y3 * y3) * (y1 - y2)) / d;
	double cy = ((x1 * x1 + y1 * y1) * (x3 - x2) + (x2 * x2 + y2 * y2) * (x1 - x3) + (x3 * x3 + y3 * y3) * (x2 - x1)) / d;
	double r = sqrt((cx - x1) * (cx - x1) + (cy - y1) * (cy - y1));

	// 페널과 브러시 준비 (채우기 없고 두께 penWidth)
	CPen pen(PS_SOLID, penWidth, RGB(0, 0, 0));
	CPen* pOldPen = dc.SelectObject(&pen);
	CBrush* pOldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);

	// 원 그리기 (Ellipse 대신 Polygon 형태로 근사 그리기)
	const int segments = 100;
	CPoint pts[segments + 1];
	for (int i = 0; i <= segments; i++)
	{
		double angle = 2 * 3.14 * i / segments;
		pts[i].x = static_cast<int>(cx + r * cos(angle));
		pts[i].y = static_cast<int>(cy + r * sin(angle));
	}

	dc.Polygon(pts, segments + 1);

	// 원래 객체 복원
	dc.SelectObject(pOldPen);
	dc.SelectObject(pOldBrush);
}

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedReset();
	afx_msg void OnBnClickedidrandom();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
//ON_BN_CLICKED(IDRESET, &CAboutDlg::OnBnClickedReset)
//ON_BN_CLICKED(idrandom, &CAboutDlg::OnBnClickedidrandom)
END_MESSAGE_MAP()


// CMFCApplication3Dlg 대화 상자



CMFCApplication3Dlg::CMFCApplication3Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION3_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, idradius, radius);
}

BEGIN_MESSAGE_MAP(CMFCApplication3Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDRESET, &CMFCApplication3Dlg::OnBnClickedReset)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(idrandom, &CMFCApplication3Dlg::OnBnClickedidrandom)
END_MESSAGE_MAP()


// CMFCApplication3Dlg 메시지 처리기

BOOL CMFCApplication3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowText(_T("GLIM 과제"));
	ModifyStyle(0, WS_CLIPCHILDREN);  // UI깜빡임 완화

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.
	SetDlgItemInt(idradius, 3);
	SetDlgItemInt(idcircle, 3);
	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CMFCApplication3Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}


void CMFCApplication3Dlg::OnPaint()
{

	CPaintDC dc(this);

	if (!m_bClicked) return; // 클릭 전에는 그리지 않음

	UpdateData(TRUE); // Edit Control의 내용을 변수로 복사
	GetDlgItemText(idradius, Radius); // Edit Control ID에서 문자열 가져오기
	GetDlgItemText(idcircle, circle); // Edit Control ID에서 문자열 가져오기
	int r = _ttoi(Radius);            // 점 두께
	int thickness = _ttoi(circle); ; // 선 두께
	const int sides = 20;
	const double PI = 3.14;
	CString coordText;
	for (int i = 0; i < m_clickPoints.GetCount(); i++)
	{
		CPoint center = m_clickPoints[i];
		CPoint pts[21];

		for (int j = 0; j <= sides; ++j)
		{
			double angle = 2 * PI * j / sides;
			pts[j].x = static_cast<int>(center.x + r * cos(angle));
			pts[j].y = static_cast<int>(center.y + r * sin(angle));
		}

		CBrush blackBrush(RGB(0, 0, 0));
		CBrush* pOldBrush = dc.SelectObject(&blackBrush);
		if (m_clickPoints.GetCount() == 3)
		{
			DrawCircleThrough3Points(dc, m_clickPoints[0], m_clickPoints[1], m_clickPoints[2], thickness);
		}
		dc.Polygon(pts, sides);
		dc.SelectObject(pOldBrush);

        coordText.Format(_T("(%d, %d)"), center.x, center.y);
        dc.TextOut(center.x + 5, center.y - 15, coordText);
	}
}


// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMFCApplication3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMFCApplication3Dlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CDialogEx::OnLButtonDblClk(nFlags, point);
}

void CMFCApplication3Dlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	const int hitRadius = 10; // 클릭 허용 범위 (픽셀)

	// 점들 중 클릭 지점과 가까운 점 찾기
	m_nSelectedPointIndex = -1;
	CString str;
	str.Format(_T("%d,%d"), point.x, point.y);

	CDialogEx::OnLButtonDown(nFlags, point);
	for (int i = 0; i < m_clickPoints.GetCount(); i++)
	{
		CPoint pt = m_clickPoints[i];
		int dx = pt.x - point.x;
		int dy = pt.y - point.y;
		if ((dx * dx + dy * dy) <= hitRadius * hitRadius)
		{
			m_nSelectedPointIndex = i;
			SetCapture();
			return;  // 점 선택하면 찍기 로직 건너뜀
		}
	}
	if (count >= 3)
	{
		AfxMessageBox(_T("점은 최대 3개까지만 찍을 수 있습니다."));
		return;
	}
	count++;
	m_clickPoints.Add(point);
	m_bClicked = TRUE;
	Invalidate();
	CDialogEx::OnLButtonDown(nFlags, point);

}



void CMFCApplication3Dlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if ((nFlags & MK_LBUTTON) && m_nSelectedPointIndex != -1)
	{
		// 선택된 점 위치를 마우스 위치로 이동
		m_clickPoints[m_nSelectedPointIndex] = point;
		Invalidate(); // 화면 다시 그리기
	}

	
	CDialogEx::OnMouseMove(nFlags, point);
}

void CMFCApplication3Dlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (m_nSelectedPointIndex != -1)
	{
		ReleaseCapture(); // 마우스 캡처 해제
		m_nSelectedPointIndex = -1;
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}


void CMFCApplication3Dlg::OnBnClickedidrandom()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_clickPoints.GetCount() != 3)
	{
		AfxMessageBox(_T("정원이 그려져야만 랜덤 이동이 가능합니다."));
		return;
	}

	// 랜덤 이동 시작
	AfxBeginThread(RandomMoveThread, this);
}
void CMFCApplication3Dlg::OnBnClickedReset()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_clickPoints.RemoveAll();
	count = 0;
	m_bClicked = FALSE;
	Invalidate();
}

