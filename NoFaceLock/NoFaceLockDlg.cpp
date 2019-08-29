
#include "stdafx.h"
#include "NoFaceLock.h"
#include "NoFaceLockDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CNoFaceLockDlg::CNoFaceLockDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNoFaceLockDlg::IDD, pParent),
	m_cameraRefreshMS(0),
	m_faceLockTimeoutMS(0),
	m_minFaceWidth(0),
	m_minFaceHeight(0),
	m_camera(NULL),
	m_lastMaxWidth(0),
	m_lastMaxHeight(0),
	m_initialized(false),
	m_tuneMode(FALSE),
	m_cascadeFileName(_T("")),
	m_faceCascade(NULL),
	m_storage(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNoFaceLockDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_CAMERA_TIME, m_cameraRefreshMS);
	DDV_MinMaxInt(pDX, m_cameraRefreshMS, 0, 65535);
	DDX_Text(pDX, IDC_FACE_TIME, m_faceLockTimeoutMS);
	DDV_MinMaxInt(pDX, m_faceLockTimeoutMS, 2500, 65535);
	DDX_Text(pDX, IDC_FACE_WIDTH, m_minFaceWidth);
	DDV_MinMaxInt(pDX, m_minFaceWidth, 0, 65535);
	DDX_Text(pDX, IDC_FACE_HEIGHT, m_minFaceHeight);
	DDV_MinMaxInt(pDX, m_minFaceHeight, 0, 65535);
	DDX_Check(pDX, IDC_TUNE_MODE, m_tuneMode);

	if (pDX->m_bSaveAndValidate)
	{
		WriteConfiguration();
		SetTimers();
	}
}

BEGIN_MESSAGE_MAP(CNoFaceLockDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_TUNE_MODE, &CNoFaceLockDlg::OnBnClickedTuneMode)
	ON_BN_CLICKED(IDB_APPLY, &CNoFaceLockDlg::OnBnClickedApply)
	ON_BN_CLICKED(IDC_REVERT, &CNoFaceLockDlg::OnBnClickedRevert)
END_MESSAGE_MAP()

// CNoFaceLockDlg message handlers

BOOL CNoFaceLockDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Reads from .ini file.  Default values will be provided
	// if configuration doesn't exist, and then configuration will be written.
	ReadConfiguration();

	// update dialog box controls with values from config
	CDataExchange ddx(this, false);
	DoDataExchange(&ddx);

	CNoFaceLockDlg::InitOpenCV();
	SetTimers();

	// enable update of settings on edit
	m_initialized = true;

	// Tooltip not working yet, so commenting out for now
#ifdef false
	m_toolTip.Create(this, TTS_ALWAYSTIP);
	m_toolTip.SetTipBkColor(RGB(255, 255, 0));
	m_toolTip.SetTipTextColor(RGB(255, 0, 0));
	m_toolTip.SendMessage(WM_SETTEXT, 0, (LPARAM)_T("Workstation would be locked if not in tune mode."));
	m_toolTip.SetDelayTime(1000);
	m_toolTip.ShowWindow(SW_SHOWNORMAL);
#endif

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// Sets the timers for no face timeout and camera refresh rate
//
void CNoFaceLockDlg::SetTimers()
{
	if (m_faceLockTimeoutMS < 1000)
	{
		m_faceLockTimeoutMS = 1000;
	}

	SetTimer(ID_CAMERA_TIMER, m_cameraRefreshMS, NULL);
	SetTimer(ID_FACE_TIMER, m_faceLockTimeoutMS, NULL);
}

void CNoFaceLockDlg::ResetNoFaceTimeout()
{
	SetTimer(ID_FACE_TIMER, m_faceLockTimeoutMS, NULL);
}

// Read configurtion if it exists.  Supply default values if it doesn't.
void CNoFaceLockDlg::ReadConfiguration()
{
	PROFILE_TABLE profileTable[] =
	{
		// pReadInto			key						default
		&m_cameraRefreshMS,		KEY_CAMERA_DELAY,		DELAY_CAMERA_MS,
		&m_faceLockTimeoutMS,	KEY_NOFACE_TIMEOUT,		FACE_TIMEOUT_MS,
		&m_minFaceWidth,		KEY_MIN_FACE_WIDTH,		MIN_FACE_WIDTH,
		&m_minFaceHeight,		KEY_MIN_FACE_HEIGHT,	MIN_FACE_HEIGHT,
		&m_tuneMode,			KEY_TUNE_MODE,			DEFAULT_TUNE_MODE,
	};

	// Special case for the one string we have
	TCHAR* profileBuffer = new TCHAR[_MAX_PATH];
	GetPrivateProfileString(
		APP_NAME,
		KEY_CASCADE_FILE_NAME,
		CASCADE_FILE_NAME,
		profileBuffer,
		_MAX_PATH,
		INI_FILE_NAME);
	m_cascadeFileName = profileBuffer;
	delete[] profileBuffer;

	for (int profileIndex = 0; profileIndex < SIZEOF(profileTable); profileIndex++)
	{
		*profileTable[profileIndex].pReadInto = GetPrivateProfileInt(
			APP_NAME,
			profileTable[profileIndex].key,
			profileTable[profileIndex].value,
			INI_FILE_NAME);
	}
}

// Writes configuration to disk
void CNoFaceLockDlg::WriteConfiguration()
{
	PROFILE_TABLE profileTable[] =
	{
		// pReadInto not used when writing
		//		key					default
		NULL,	KEY_CAMERA_DELAY,		m_cameraRefreshMS,
		NULL,	KEY_NOFACE_TIMEOUT,		m_faceLockTimeoutMS,
		NULL,	KEY_MIN_FACE_WIDTH,		m_minFaceWidth,
		NULL,	KEY_MIN_FACE_HEIGHT,	m_minFaceHeight,
		NULL,	KEY_TUNE_MODE,			m_tuneMode,
	};

	CString profileString;
	for (int profileIndex = 0; profileIndex < SIZEOF(profileTable); profileIndex++)
	{
		profileString.Format(_T("%d"), profileTable[profileIndex].value);
		WritePrivateProfileString(
			APP_NAME,
			profileTable[profileIndex].key,
			(LPCTSTR)profileString,
			INI_FILE_NAME);
	}

	// Special case for the one string we have
	WritePrivateProfileString(
		APP_NAME,
		KEY_CASCADE_FILE_NAME,
		m_cascadeFileName,
		INI_FILE_NAME);
}

// Loads cascade file used for face detection, creates storage for image capture, and 
// initialize camera to capture from
void CNoFaceLockDlg::InitOpenCV()
{
	// Haar Cascade file, used for Face Detection.
	char* faceCascadeFilename = "haarcascade_frontalface_alt2.xml";

	// create a window
	cvNamedWindow(CAPTION_VIDEO, 1);

	// Load the HaarCascade classifier for face detection.
	m_faceCascade = (CvHaarClassifierCascade*)cvLoad(faceCascadeFilename, 0, 0, 0);

	if (!m_faceCascade)
	{
		exit(1);
	}

	// setup memory buffer; needed by the face detector 
	m_storage = cvCreateMemStorage(0);

	// initialize camera 
	m_camera = cvCaptureFromCAM(0);
}

void CNoFaceLockDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CNoFaceLockDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		/* Commented out aborted attempt to paint some state text on the dialog, may use it later
		CDC* pDC = GetDC();

		BeginPaint(  );

		CRect windowRect;
		GetClientRect( &windowRect );
		CBitmap* pcBitmap = new CBitmap();
		pcBitmap->CreateCompatibleBitmap( pDC, windowRect.Width(), windowRect.Height() );
		CString* pString = new CString();

		pString->Format( _T("Max width: %d\r\nMax height: %d"), m_lastMaxWidth, m_lastMaxHeight );

		CDC* pMemDC = new CDC();
		pMemDC->CreateCompatibleDC( pDC );
		pMemDC->SelectObject( pcBitmap );
		pMemDC->ExtTextOut( 0, 0, ETO_OPAQUE, windowRect, (LPCTSTR) pString, pString->GetLength(), NULL );

		pDC->BitBlt( 0, 0, windowRect.Width(), windowRect.Height(), pMemDC, 0, 0, SRCCOPY );

		ReleaseDC( pDC );
		delete pcBitmap;
		delete pString;
		delete pMemDC;

		EndPaint();
		*/

		CDialogEx::OnPaint();
	}
}

HCURSOR CNoFaceLockDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CNoFaceLockDlg::OnDestroy()
{
	cvReleaseCapture(&m_camera);
	cvDestroyWindow(CAPTION_VIDEO);
	cvReleaseHaarClassifierCascade(&m_faceCascade);
	cvReleaseMemStorage(&m_storage);

	WriteConfiguration();

	// call default implementation
	CDialogEx::OnDestroy();
}

// Updates the last 
void CNoFaceLockDlg::UpdateFace(int width, int height)
{
	if (IsFaceBigEnough(width, height))
	{
		CString cFaceInfoString;

		m_lastMaxWidth = width;
		m_lastMaxHeight = height;

		cFaceInfoString.Format(_T("%d"), m_lastMaxHeight);
		GetDlgItem(IDC_LAST_HEIGHT)->SendMessage(WM_SETTEXT, 0, (LPARAM)(LPCTSTR)cFaceInfoString);
		cFaceInfoString.Format(_T("%d"), m_lastMaxWidth);
		GetDlgItem(IDC_LAST_WIDTH)->SendMessage(WM_SETTEXT, 0, (LPARAM)(LPCTSTR)cFaceInfoString);

		ResetNoFaceTimeout();
	}
}

void CNoFaceLockDlg::DetectFaces(IplImage* img)
{
	// detect faces
	CvSeq* faces = cvHaarDetectObjects(
		img,
		m_faceCascade,
		m_storage,
		1.1,
		3,
		0,					// CV_HAAR_DO_CANNY_PRUNNING,
		cvSize(40, 40));

	/* for each face found, draw a red box */
	int maxWidth = 0;
	int maxHeight = 0;

	for (int i = 0; i < (faces ? faces->total : 0); i++)
	{
		CvRect* r = (CvRect*)cvGetSeqElem(faces, i);

		if (r->width > maxWidth)
		{
			maxWidth = r->width;
		}

		if (r->height > maxHeight)
		{
			maxHeight = r->height;
		}

		UpdateFace(maxWidth, maxHeight);

		cvRectangle(
			img,
			cvPoint(r->x, r->y),
			cvPoint(r->x + r->width, r->y + r->height),
			CV_RGB(255, 0, 0), 1, 8, 0);
	}

	// display video 
	cvShowImage(CAPTION_VIDEO, img);
}

// Locks the workstation if no face timeout occurs
void CNoFaceLockDlg::Lock()
{
	// prevents dangerous timeout from locking user out of system entirely
	if (m_faceLockTimeoutMS < 1000)
	{
		return;
	}

	if (m_tuneMode)
	{
		MessageBeep(0);
	}
	else
	{
		LockWorkStation();
	}
}

// Returns true if face meets minimum requirements
bool CNoFaceLockDlg::IsFaceBigEnough(int width, int height)
{
	bool isFaceBigEnough = true;

	if ((width < m_minFaceWidth) && (height < m_minFaceWidth))
	{
		isFaceBigEnough = false;
	}

	return isFaceBigEnough;
}

// Handle WM_TIMER
void CNoFaceLockDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
		case ID_CAMERA_TIMER:
		{
			IplImage* frame = cvQueryFrame(m_camera);
			if (NULL != frame)
			{
				// 'fix' frame - apparently not needed now but leaving in for the case
				// where cameras that need it are identified. So far I haven't found one...
				// cvFlip( frame, frame, -1 );  // Turns camera image upside-down

				// detect faces and display video (should be decoupled)
				DetectFaces(frame);
			}
		}
		break;

		case ID_FACE_TIMER:
		{
			Lock();
		}
		break;

		default: break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CNoFaceLockDlg::OnBnClickedTuneMode()
{
	CButton* pCheckBox = (CButton*)GetDlgItem(IDC_TUNE_MODE);

	m_tuneMode = pCheckBox->GetCheck();
}

void CNoFaceLockDlg::OnBnClickedApply()
{
	CDataExchange* pDX = new CDataExchange(this, true);
	DoDataExchange(pDX);
}


void CNoFaceLockDlg::OnBnClickedRevert()
{
	CDataExchange* pDX = new CDataExchange(this, false);
	DoDataExchange(pDX);
}
