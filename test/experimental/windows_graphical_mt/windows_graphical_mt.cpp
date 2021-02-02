// windows_graphical.cpp : Defines the entry point for the application.
//

#include <common.h>
#include <infrastructure/q_based_infrastructure.h>
#include "framework.h"
#include "windows_graphical_mt.h"
#include "some_node.h"

// NODECPP-required
// magic class defining what to do with (how to deliver) msg supplied to postInterThreadMsg()
// In this particular example we use system call PostMessage to a window defined by hWnd
class WorkerThreadPostman : public InterThreadMessagePostmanBase
{
	HWND* hWnd;
public: 
	WorkerThreadPostman( HWND* hWnd_ ) : hWnd( hWnd_ ) {}
	void postMessage( InterThreadMsg&& msg ) override
	{
		// create a move-copy in heap, otherwise the msg will be destructed at the end of this call (and, potentially, before it will be received and processed)
		PostMessage(*hWnd, WM_USER + msg.targetThreadID.nodeID, WPARAM(msg.convertToPointer()), 0 );
	}
};

// NOTE: main thread Postman is given by a call to useQueuePostman()

// WORKER THREAD CREATION STAFF
template<class NodeT, class ThreadStartupDataT>
void nodeThreadMain( void* pdata )
{
	ThreadStartupDataT* sd = reinterpret_cast<ThreadStartupDataT*>(pdata);
	NODECPP_ASSERT( nodecpp::module_id, ::nodecpp::assert::AssertLevel::critical, pdata != nullptr ); 
	ThreadStartupDataT startupData = *sd;
	nodecpp::stddealloc( sd, 1 );
	QueueBasedNodeLoop<NodeT> r( startupData );
	r.init();
	r.run();
}

template<class NodeT, class PostmanT>
NodeAddress runNodeInAnotherThread( PostmanT* postman )
{
	auto startupDataAndAddr = QueueBasedNodeLoop<NodeT>::getInitializer(postman); // TODO: consider implementing q-based Postman (as lib-defined)
	using InitializerT = typename QueueBasedNodeLoop<NodeT>::Initializer;
	InitializerT* startupData = nodecpp::stdalloc<InitializerT>(1);
	*startupData = startupDataAndAddr.first;
	size_t threadIdx = startupDataAndAddr.second.slotId;
	nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"about to start Listener thread with threadID = {}...", threadIdx );
	std::thread t1( nodeThreadMain<NodeT, InitializerT>, (void*)(startupData) );
	// startupData is no longer valid
	startupData = nullptr;
	t1.detach();
	nodecpp::log::default_log::info( nodecpp::log::ModuleID(nodecpp::nodecpp_module_id),"...starting Listener thread with threadID = {} completed at Master thread side", threadIdx );
	return startupDataAndAddr.second;
}



// Forward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

class MainWindow 
{
	static constexpr size_t MAX_LOADSTRING = 100;
	HINSTANCE hInst;                                // current instance
	WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
	WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

	// NODECPP-required
	// (will be replaced by a right way to do that)
	HWND hMainWnd = 0; // where to post node-related messages
	NodeAddress someNodeAddress; // address to be used to send messages to the Node

public:
	MainWindow() {};

	int main(_In_ HINSTANCE hInstance,
						 _In_opt_ HINSTANCE hPrevInstance,
						 _In_ LPWSTR    lpCmdLine,
						 _In_ int       nCmdShow)
	{
		UNREFERENCED_PARAMETER(hPrevInstance);
		UNREFERENCED_PARAMETER(lpCmdLine);

		// TODO: Place code here.

		// Initialize global strings
		LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadStringW(hInstance, IDC_WINDOWSGRAPHICAL, szWindowClass, MAX_LOADSTRING);
		MyRegisterClass(hInstance);

		// Perform application initialization:
		if (!InitInstance (hInstance, nCmdShow))
		{
			return FALSE;
		}

		// NODECPP-required
		// not it's time to initialize our means
		ThreadStartupData data;
		WorkerThreadPostman postman( &hMainWnd ); // see above-mention magic
		preinitThreadStartupData( data, &postman );
		setThisThreadDescriptor( data);
		someNodeAddress = runNodeInAnotherThread<SomeNode>( useQueuePostman() );

		HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSGRAPHICAL));

		MSG msg;

		// Main message loop:
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		return (int) msg.wParam;
	}


private:
	//
	//  FUNCTION: MyRegisterClass()
	//
	//  PURPOSE: Registers the window class.
	//
	ATOM MyRegisterClass(HINSTANCE hInstance)
	{
		WNDCLASSEXW wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style          = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc    = ::WndProc;
		wcex.cbClsExtra     = 0;
		wcex.cbWndExtra     = 0;
		wcex.hInstance      = hInstance;
		wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSGRAPHICAL));
		wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSGRAPHICAL);
		wcex.lpszClassName  = szWindowClass;
		wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		return RegisterClassExW(&wcex);
	}

	//
	//   FUNCTION: InitInstance(HINSTANCE, int)
	//
	//   PURPOSE: Saves instance handle and creates main window
	//
	//   COMMENTS:
	//
	//        In this function, we save the instance handle in a global variable and
	//        create and display the main program window.
	//
	BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
	{
	   hInst = hInstance; // Store instance handle in our global variable

	   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	   if (!hWnd)
	   {
		  return FALSE;
	   }

	   hMainWnd = hWnd;

	   ShowWindow(hWnd, nCmdShow);
	   UpdateWindow(hWnd);

	   return TRUE;
	}

public:
	//
	//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
	//
	//  PURPOSE: Processes messages for the main window.
	//
	//  WM_COMMAND  - process the application menu
	//  WM_PAINT    - Paint the main window
	//  WM_DESTROY  - post a quit message and return
	//
	//
	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_COMMAND:
			{
				int wmId = LOWORD(wParam);
				// Parse the menu selections:
				switch (wmId)
				{
				case IDM_ABOUT:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				// TODO: Add any drawing code that uses hdc here...
				EndPaint(hWnd, &ps);
			}
			break;
		case WM_LBUTTONUP:
		{
			unsigned int x = lParam & 0xFFFF;
			unsigned int y = (unsigned int)(lParam >> 16);
			// NODECPP-required
			// prepare and post message
			Message msg;
			m::infrastructural::composeMessage<m::infrastructural::ScreenPoint>( msg, m::x = x, m::y = y );
			postInfrastructuralMsg( std::move( msg ), InterThreadMsgType::Infrastructural, someNodeAddress );
			break;
		}
		case WM_USER:
		{
			InterThreadMsgPtr iptr( wParam );
			InterThreadMsg msg( iptr );
			m::infrastructural::handleMessage( msg.msg,
				m::makeMessageHandler<m::infrastructural::ScreenPoint>([&](auto& parser){ 
					int x;
					int y;
					m::STRUCT_ScreenPoint_parse( parser, m::x = &(x), m::y = &(y) );
					auto f = fmt::format( "x = {}\ny = {}", x, y );
					MessageBox( hWnd, f.c_str(), "Point", MB_OK );
				}),
				m::makeDefaultMessageHandler([&](auto& parser, uint64_t msgID){ 
					auto f = fmt::format( "Unhandled message {}\n", msgID ); 
					MessageBox( hWnd, f.c_str(), "Point", MB_OK );
				})
			);
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

}; // MainWindow

MainWindow mainWindow; // the only global object

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return mainWindow.WndProc( hWnd, message, wParam, lParam);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	return mainWindow.main( hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
