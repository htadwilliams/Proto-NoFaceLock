#pragma once

#include "opencv2\opencv.hpp"
#include "afxwin.h"

#define ID_CAMERA_TIMER			42
#define ID_FACE_TIMER			23

// Default config values
#define FACE_TIMEOUT_MS			65535
#define DELAY_CAMERA_MS			250
#define MIN_FACE_WIDTH			130
#define MIN_FACE_HEIGHT			130
#define DEFAULT_TUNE_MODE		TRUE
#define CASCADE_FILE_NAME		_T( "haarcascade_frontalface_alt2.xml" )

// Used for configuration
#define APP_NAME				_T( "NoFaceLock" )
#define KEY_CAMERA_DELAY		_T( "CameraDelayMS" )
#define KEY_NOFACE_TIMEOUT		_T( "NoFaceTimeoutMS" )
#define KEY_CASCADE_FILE_NAME	_T( "CascadeFileName" )
#define KEY_MIN_FACE_WIDTH		_T( "MinFaceWidth" )
#define KEY_MIN_FACE_HEIGHT		_T( "MinFaceHeight" )
#define KEY_TUNE_MODE			_T( "TuneMode" )
#define INI_FILE_NAME			_T( "NoFaceLock.ini" )

// Video capture window caption / name
// OpenCV wants a const char *, can't use _T()
#define CAPTION_VIDEO			"NoFaceLock - Video Capture"

// Used to read and write configuration settings
typedef struct tagProfileTable
{
	int*		pReadInto;
	LPCTSTR		key;
	int			value;
} PROFILE_TABLE;

class CNoFaceLockDlg : public CDialogEx
{
public:
	CNoFaceLockDlg(CWnd* pParent = NULL);	// standard constructor

	enum { IDD = IDD_NOFACELOCK_DIALOG };

	protected:
	virtual void InitOpenCV();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

protected:
	HICON						m_hIcon;

	// OpenCV members
	CvCapture*					m_camera;
	CvHaarClassifierCascade*	m_faceCascade;
	CvMemStorage*				m_storage;
	int							m_lastMaxWidth;
	int							m_lastMaxHeight;
	bool						m_initialized;

	// configurable settings
	// The timer for camera frame refresh is set to this value in Milliseconds
	int							m_cameraRefreshMS;

	// The timer for timeout and locking the system happens on this interval in milliseconds
	int							m_faceLockTimeoutMS;

	// Minimum size of recognized face.  
	// If a face isn't recognized this size in the no face timeout period, the system is locked.
	int							m_minFaceWidth;
	int							m_minFaceHeight;

	BOOL						m_tuneMode;
	CString						m_cascadeFileName;

	virtual void DetectFaces( IplImage *img );
	virtual void UpdateFace( int width, int height );
	virtual void Lock( void );
	virtual void ReadConfiguration( void );
	virtual void WriteConfiguration( void );
	virtual void SetTimers( void );
	virtual void ResetNoFaceTimeout( void );
	virtual bool IsFaceBigEnough( int width, int height );

	// Generated message map functions
	afx_msg BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedTuneMode();
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedRevert();
};
