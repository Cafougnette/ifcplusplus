// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface
// (the "Fluent UI") and is provided only as referential material to supplement the
// Microsoft Foundation Classes Reference and related electronic documentation
// included with the MFC C++ library software.
// License terms to copy, use or distribute the Fluent UI are available separately.
// To learn more about our Fluent UI licensing program, please visit
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "IfcQueryMFC.h"
#include "IfcQueryDoc.h"
#include "IfcQueryMFCView.h"
#include "MainFrame.h"
#include "BuildingUtils.h"
#include "SceneGraph/SoWinCADViewer.h"

#include <ifcpp/reader/ReaderUtil.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/nodes/SoSeparator.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame
IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	ON_COMMAND(ID_FILE_PRINT, &CMainFrame::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CMainFrame::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnFilePrintPreview)
	ON_COMMAND_RANGE( 1002, 1003, &CMainFrame::OnClearWindowState )
	ON_COMMAND_RANGE( 1004, 1005, &CMainFrame::OnZoomToBoundings )
	ON_COMMAND_RANGE( 1000, 1001, &CMainFrame::OnLoadWallExample )
	ON_COMMAND_RANGE( 1006, 1007, &CMainFrame::OnWriteFileIFC )
	ON_COMMAND_RANGE( 1008, 1009, &CMainFrame::OnWriteFileWebGL )
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnUpdateFilePrintPreview)
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);
}

CMainFrame::~CMainFrame()
{
}

// message handler for Coin3D
COutputList* wndOutput_messages = nullptr;
std::wstring previous_msg_str;
void messageHandlerCoin3D( const SoError * error, void * userdata )
{
	if( error )
	{
		std::wstring msg_str;

		if( error->getTypeId() == SoDebugError::getClassTypeId() )
		{
			const SoDebugError* dbg = static_cast<const SoDebugError*>(error);
			
			switch( dbg->getSeverity() )
			{
			case SoDebugError::INFO:
				msg_str.append( L"info: " );
				break;
			case SoDebugError::WARNING:
				msg_str.append( L"warning: " );
				
				break;
			default: // error
				msg_str.append( L"error: " );
				break;
			}
		}
		else
		{
			msg_str.append( L"info: " );
		}

		const char* msg = error->getDebugString().getString();
		msg_str.append( s2ws( msg ) );

		if( previous_msg_str.compare( msg_str ) == 0 )
		{
			// same message, don't show again
			return;
		}
		previous_msg_str = msg_str;

		if( wndOutput_messages )
		{
			wndOutput_messages->AddString( msg_str.c_str() );

			int nCount = wndOutput_messages->GetCount();
			wndOutput_messages->SetCurSel( nCount - 1 );
			wndOutput_messages->SetTopIndex( nCount - 1 );
		}
		else
		{
			std::cout << msg_str.c_str() << std::endl;
		}
	}
}

std::string previous_progress_type;
// message handler for IfcQuery
void CMainFrame::OnStatusMessage( const shared_ptr<StatusCallback::Message>& m )
{
	if( m->m_message_type == StatusCallback::MESSAGE_TYPE_PROGRESS_VALUE )
	{
		double progress_value = m->m_progress_value;
		if( m->m_progress_type.compare( "parse" ) == 0 )
		{
			progress_value = progress_value*0.5;
		}
		else if( m->m_progress_type.compare( "geometry" ) == 0 )
		{
			progress_value = 0.5 + progress_value*0.5;
		}

		progress_value *= 100.0;
		int current_progress = m_ctrl_progress.GetPos();
		if( int(progress_value) < current_progress )
		{
			if( current_progress < 100 )
			{
				std::cout << "progress decreasing" << std::endl;
			}
		}

		m_ctrl_progress.SetPos( int( progress_value ) );
		m_ctrl_progress.UpdateWindow();

		previous_progress_type = m->m_progress_type;
		return;
	}

	std::wstring message_txt;
	message_txt = m->m_message_text;
	if( m->m_message_type == StatusCallback::MESSAGE_TYPE_ERROR )
	{
		// TODO: make line red
	}

	bool printMessage = true;
	if( message_txt.size() > 0 )
	{
		if( message_txt.find( L"select" ) != std::string::npos )
		{
			if( m_internal_call_to_select )
			{
				return;
			}
			size_t begin_id_index = message_txt.find_first_of( L"#" );
			if( message_txt.length() > begin_id_index + 1 )
			{
				std::wstring node_name_id = message_txt.substr( begin_id_index + 1 );
				size_t last_index = node_name_id.find_first_not_of( L"-0123456789" );
				std::wstring id_str = node_name_id.substr( 0, last_index );
				if( id_str.length() > 0 )
				{
					const int id = std::stoi( id_str );
					m_internal_call_to_select = true;
					IfcQueryDoc* query_doc = getCurrentDocument();

					if( id < 0 )
					{
						// unselect
						printMessage = false;
						m_wndBuildingElements.unselectAllEntities();
						m_wndProperties.unselectAllEntities();
						m_wndSTEP.unselectAllEntities();
						
					}
					else
					{
						// select
						m_wndBuildingElements.setEntitySelected( id );
						m_wndProperties.setEntitySelected( id );
						m_wndSTEP.setEntitySelected( id );
					}
					
					m_internal_call_to_select = false;
				}
			}
		}

		if( wndOutput_messages && printMessage )
		{
			wndOutput_messages->AddString( message_txt.c_str() );
			int nCount = wndOutput_messages->GetCount();
			wndOutput_messages->SetCurSel( nCount - 1 );
			wndOutput_messages->SetTopIndex( nCount - 1 );
		}
		else
		{
			std::cout << message_txt.c_str() << std::endl;
		}
	}
}

void CMainFrame::slotMessageWrapper( void* obj_ptr, shared_ptr<StatusCallback::Message> m )
{
	CMainFrame* myself = (CMainFrame*)obj_ptr;
	if( myself )
	{
		myself->OnStatusMessage( m );
	}
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if( CFrameWndEx::OnCreate( lpCreateStruct ) == -1 )
	{
		return -1;
	}

	BOOL bNameValid;

	m_message_target = shared_ptr<StatusCallback>( new StatusCallback() );
	m_message_target->setMessageCallBack( this, &CMainFrame::slotMessageWrapper );
		
	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	// add button in window panel
	CMFCRibbonCategory *pCat = m_wndRibbonBar.GetCategory( 1 );
	if( pCat )
	{
		CMFCRibbonPanel* viewPanel = pCat->GetPanel( 0 );
		if( viewPanel )
		{
			m_btnZoomToBoundings = new CMFCRibbonButton( 1004, TEXT( "Zoom to boundings" ), NULL, 0, 0, 0, 0 );
			viewPanel->Add( m_btnZoomToBoundings );
			//m_btn_clear_window_state = new CMFCRibbonButton( 1002, TEXT( "Reset window state" ), NULL, 0, 0, 0, 0 );
			//window_panel->Add( m_btn_clear_window_state );
		}


		// add panel and button for examples
		//CMFCRibbonPanel *pPanStoreyShift = pCat->AddPanel( TEXT( "Storey shift" ) );
		
		// TODO: add storey shift x, y, z
		CMFCRibbonSlider* sliderStoreyShiftX = new CMFCRibbonSlider();
		CMFCRibbonSlider* sliderStoreyShiftY = new CMFCRibbonSlider();
		CMFCRibbonSlider* sliderStoreyShiftZ = new CMFCRibbonSlider();
		sliderStoreyShiftX->SetZoomButtons( true );
		sliderStoreyShiftX->SetPos( 50, TRUE );
		sliderStoreyShiftX->SetRange( 0, 100 );

		sliderStoreyShiftY->SetZoomButtons( true );
		sliderStoreyShiftY->SetPos( 50, TRUE );
		sliderStoreyShiftY->SetRange( 0, 100 );

		sliderStoreyShiftZ->SetZoomButtons( true );
		sliderStoreyShiftZ->SetPos( 50, TRUE );
		sliderStoreyShiftZ->SetRange( 0, 100 );

		//pPanStoreyShift->Add( sliderStoryShiftX );
		//pPanStoreyShift->Add( sliderStoryShiftY );
		//pPanStoreyShift->Add( sliderStoryShiftZ );
		// TODO: add methods onValueChangeShiftX/Y/Z
		

		// add panel and button for examples
		CMFCRibbonPanel *pPan = pCat->AddPanel( TEXT( "Examples" ) );
		m_btnLoadWall = new CMFCRibbonButton( 1000, TEXT( "Load wall example" ), NULL, 0, 0, 0, 0 );
		pPan->Add( m_btnLoadWall );
	}
	
	// add category "Export"
	LPCTSTR lpszName = L"Export";
	UINT uiSmallImagesResID = IDR_PROPERTIES;  // TODO: make bmp
	UINT uiLargeImagesResID = IDR_PROPERTIES;  // TODO: make bmp
	//CSize sizeSmallImage = CSize( 16, 16 ), CSize sizeLargeImage = CSize( 32, 32 ), int nInsertAt = -1, CRuntimeClass* pRTI = NULL);
	CMFCRibbonCategory *pCatExport = m_wndRibbonBar.AddCategory( L"Export", uiSmallImagesResID, uiLargeImagesResID );
	if( pCatExport )
	{
		// add panel and button for examples
		CMFCRibbonPanel *pPanIFC = pCatExport->AddPanel( TEXT( "IFC" ) );
		m_btnExportIFC = new CMFCRibbonButton( 1006, TEXT( "Write file" ), NULL, 0, 0, 0, 0 );
		//CMFCFileDialog* fileDialog = new CMFCFileDialog();
		CMFCRibbonCheckBox* ribbonCheckBox;

		pPanIFC->Add( m_btnExportIFC );

		CMFCRibbonPanel *pPanWebGL = pCatExport->AddPanel( TEXT( "WebGL" ) );
		m_btnExportWebGL = new CMFCRibbonButton( 1008, TEXT( "Write file" ), NULL, 0, 0, 0, 0 );
		pPanWebGL->Add( m_btnExportWebGL );
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	CString strTitlePane1;
	CString strTitlePane2;
	bNameValid = strTitlePane1.LoadString(IDS_STATUS_PANE1);
	ASSERT(bNameValid);
	bNameValid = strTitlePane2.LoadString(IDS_STATUS_PANE2);
	ASSERT(bNameValid);
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, strTitlePane1, TRUE), strTitlePane1);

	CRect progress_rect( 0, 0, 400, 40 );
	m_ctrl_progress.Create( WS_VISIBLE | WS_CHILD, progress_rect, &m_wndStatusBar, 1 );
	m_ctrl_progress.SetRange( 0, 100 );
	//m_wndStatusBar.AddExtendedElement( &m_ctrl_progress, strTitlePane2 );

	m_wndStatusBar.AddExtendedElement( new CMFCRibbonStatusBarPane( ID_STATUSBAR_PANE2, strTitlePane2, TRUE ), strTitlePane2 );

	SoDebugError::setHandlerCallback( &messageHandlerCoin3D, nullptr );

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Load menu item image (not placed on any standard toolbars):
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

	// create docking windows
	if (!CreateDockingWindows())
	{
		TRACE0("Failed to create docking windows\n");
		return -1;
	}

	m_wndBuildingElements.EnableDocking(CBRS_ALIGN_ANY);
	DockPane( &m_wndBuildingElements );
	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndOutput);
	
	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndProperties);

	//m_wndSTEP.EnableDocking( CBRS_ALIGN_ANY );
	//DockPane( &m_wndSTEP );

	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	bool reset_windows_state = true;
	if( reset_windows_state )
	{
		// TODO: add button to call CleanState
		theApp.CleanState();
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow( cs ) )
	{
		return FALSE;
	}
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	

	return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
	BOOL bNameValid = true;

	// Create tree view for project structure
	CString strClassView = L"Building elements";
	if (!m_wndBuildingElements.Create(strClassView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_CLASSVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Class View window\n");
		return FALSE;
	}

	// Create output window
	CString strOutputWnd;
	bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
	ASSERT(bNameValid);
	if( !m_wndOutput.Create( strOutputWnd, this, CRect( 0, 0, 100, 100 ), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI ) )
	{
		TRACE0("Failed to create Output window\n");
		return FALSE;
	}
	wndOutput_messages = &m_wndOutput.getOutputMessages();

	// Create properties window
	if (!m_wndProperties.Create( L"Properties", this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI ))
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE;
	}
	
	bool addSTEPBrowser = false;
	if( addSTEPBrowser )
	{
		DWORD dividerAlignment = CBRS_ALIGN_LEFT;
		CPaneDivider* divider = m_wndProperties.CreateDefaultPaneDivider( dividerAlignment, this );

		// Create properties window
		if( !m_wndSTEP.Create( L"STEP", this, CRect( 0, 0, 200, 200 ), TRUE, ID_VIEW_STEP_WND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI ) )
		{
			TRACE0( "Failed to create STEP window\n" );
			return FALSE;
		}
		//m_wndSTEP.DockToWindow( &m_wndProperties, CBRS_BOTTOM );

		//CPaneDivider* divider = m_wndProperties.GetDefaultPaneDivider();
		divider->AddPane( &m_wndProperties );
		divider->AddPane( &m_wndSTEP );
	}

	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hClassViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_CLASS_VIEW_HC : IDI_CLASS_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndBuildingElements.SetIcon(hClassViewIcon, FALSE);

	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);
	//m_wndSTEP.SetIcon( hPropertiesBarIcon, FALSE );
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG

void CMainFrame::OnSize( UINT nType, int cx, int cy )
{
	CFrameWndEx::OnSize( nType, cx, cy );
	CMFCRibbonBaseElement* first_ele = m_wndStatusBar.GetElement( 1 );
	if( first_ele )
	{
		//RECT rc;
		//first_ele->GetSize()
		// Reposition the progress control
		//m_wndpProgress.SetWindowPos( &wndTop, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );
	}
}

// CMainFrame message handlers
void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(TRUE);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
	}

	m_wndOutput.UpdateFonts();
	RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}


void CMainFrame::OnFilePrint()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_PRINT);
	}
}

void CMainFrame::OnFilePrintPreview()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_CLOSE);  // force Print Preview mode closed
	}
}

void CMainFrame::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsPrintPreview());
}

void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CFrameWndEx::OnSettingChange(uFlags, lpszSection);
	m_wndOutput.UpdateFonts();
}

BOOL CMainFrame::PreTranslateMessage( MSG* pMsg )
{
	if( pMsg->message == WM_KEYDOWN )
	{
		if( pMsg->wParam == VK_DELETE )
		{
			IfcQueryMFCView* queryView = getCurrentView();
			if( queryView )
			{
				SoWinCADViewer* viewer = queryView->getViewer();

				for( auto it_selected = viewer->m_vec_selected_nodes.begin(); it_selected != viewer->m_vec_selected_nodes.end(); ++it_selected )
				{
					shared_ptr<SceneGraphUtils::SelectionContainer>& selectionContainer = *it_selected;
					if( selectionContainer->m_node.valid() )
					{
						selectionContainer->m_node->removeAllChildren();
					}
				}
				viewer->unselectAllNodes();
			}

			return TRUE;
		}
		

		if( pMsg->wParam == VK_DOWN )
		{
			::TranslateMessage( pMsg );
			::DispatchMessage( pMsg );
			return (1);

		}
		else if( pMsg->wParam == VK_UP )
		{
			
		}
		else
		{

		}
	}

	return CWnd::PreTranslateMessage( pMsg );
}

void CMainFrame::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	// TODO: Add your message handler code here and/or call default
	AfxMessageBox( L"Key down!" );
	CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}


void CMainFrame::OnUpdate()
{
	IfcQueryDoc* query_doc = getCurrentDocument();
	if( !query_doc )
	{
		m_wndBuildingElements.clearBuildingElementsTreeView();
		return;
	}

	m_wndBuildingElements.setBuildingModel( query_doc->m_ifc_model );
	m_wndBuildingElements.updateBuildingElementsTreeView();
	m_wndProperties.setBuildingModel( query_doc->m_ifc_model );
	m_wndSTEP.setBuildingModel( query_doc->m_ifc_model );
}

void CMainFrame::OnViewChange( UINT nCmdID )
{
	IfcQueryDoc* query_doc = getCurrentDocument();
	if( !query_doc )
	{
		return;
	}

	m_wndBuildingElements.setBuildingModel( query_doc->m_ifc_model );

}

void CMainFrame::DocToFrame()
{
	//AfxMessageBox( CMainFrame::DocToFrame was called );
}

void CMainFrame::OnClearWindowState( UINT ID )
{
	theApp.CleanState();
	AdjustDockingLayout();
}

void CMainFrame::OnZoomToBoundings( UINT ID )
{
	zoomToModel();
}

void CMainFrame::zoomToModel()
{
	IfcQueryMFCView* query_view = getCurrentView();
	if( query_view )
	{
		query_view->zoomToModelNode();
	}
}

IfcQueryMFCView* CMainFrame::getCurrentView()
{
	CView* current_view = GetActiveView();
	if( current_view )
	{
		IfcQueryMFCView* query_view = dynamic_cast<IfcQueryMFCView*>(current_view);
		return query_view;
	}
	return nullptr;
}

IfcQueryDoc* CMainFrame::getCurrentDocument()
{
	CDocument* pDoc = GetActiveDocument();
	if( !pDoc )
	{
		return nullptr;
	}
	IfcQueryDoc* query_doc = dynamic_cast<IfcQueryDoc*>(pDoc);
	return query_doc;
}

void CMainFrame::OnLoadWallExample( UINT ID )
{
	IfcQueryDoc* query_doc = getCurrentDocument();
	if( !query_doc )
	{
		return;
	}

	std::wstring file_path = L"../Example1CreateWallAndWriteFile/SimpleWall.ifc";
	int	file_exists = PathFileExists( file_path.c_str() );
	if( file_exists != 1 )
	{
		file_path = L"../Example2LoadIfcFileModelOnly/SimpleWall.ifc";
		
		file_exists = PathFileExists( file_path.c_str() );
		if( file_exists != 1 )
		{
			shared_ptr<StatusCallback::Message> message_error( new StatusCallback::Message() );
			message_error->m_message_type = StatusCallback::MESSAGE_TYPE_ERROR;
			message_error->m_message_text = L"Example file ";
			message_error->m_message_text.append( file_path );
			message_error->m_message_text.append( L" not found." );
			OnStatusMessage( message_error );
			return;
		}
	}

	theApp.m_pDocManager->OpenDocumentFile( file_path.c_str() );
}

void CMainFrame::OnWriteFileIFC( UINT ID )
{
	IfcQueryDoc* query_doc = getCurrentDocument();
	if( !query_doc )
	{
		return;
	}

	IfcQueryMFCView* queryView = getCurrentView();
	if( queryView )
	{
		SoWinCADViewer* viewer = queryView->getViewer();

		if( viewer->m_vec_selected_nodes.size() == 0 )
		{
			// export complete model
		}
		else
		{
			// export only selected objects
			for( auto it_selected = viewer->m_vec_selected_nodes.begin(); it_selected != viewer->m_vec_selected_nodes.end(); ++it_selected )
			{
				shared_ptr<SceneGraphUtils::SelectionContainer>& selectionContainer = *it_selected;

			}
		}
	}
}

void CMainFrame::OnWriteFileWebGL( UINT ID )
{
	IfcQueryDoc* query_doc = getCurrentDocument();
	if( !query_doc )
	{
		return;
	}

	IfcQueryMFCView* queryView = getCurrentView();
	if( queryView )
	{
		SoWinCADViewer* viewer = queryView->getViewer();

		if( viewer->m_vec_selected_nodes.size() == 0 )
		{
			// export complete model
		}
		else
		{
			// export only selected objects
			for( auto it_selected = viewer->m_vec_selected_nodes.begin(); it_selected != viewer->m_vec_selected_nodes.end(); ++it_selected )
			{
				shared_ptr<SceneGraphUtils::SelectionContainer>& selectionContainer = *it_selected;

			}
		}
	}
}
