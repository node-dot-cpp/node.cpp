// windows_graphical.cpp : Defines the entry point for the application.
//

#ifdef NODECPP_USE_GMQUEUE
#include <infrastructure/inproc_queue.h>
#endif

#include <common.h>
#include <infrastructure/q_based_infrastructure.h>
#include <platforms/hwnd_queue.h>
#include "framework.h"
#include "windows_graphical_mt.h"
#include "some_node.h"
#include <infrastructure/node_thread_creation.h> // WORKER THREAD CREATION STAFF

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

	PoolForMainThreadT mqPool;

	struct SubscriptionState
	{
		MainWindow* mainWnd;

		struct Point
		{
			int x;
			int y;
			void notifyUpdated_x( int oldX ) { MessageBeep( 0xFFFFFFFF ); }
			void notifyUpdated_y( int oldY ) { MessageBeep( 0xFFFFFFFF ); }
		};
		Point screenPoint;
		void notifyUpdated_screenPoint( const Point& oldVal ) const { 
			auto f = fmt::format( "Old ScreenPoint: x = {}, y = {}\nNew ScreenPoint: x = {}, y = {}", oldVal.x, oldVal.y, screenPoint.x, screenPoint.y );
//			MessageBox( hWnd, f.c_str(), "Point", MB_OK );
			MessageBox( 0, f.c_str(), "Point", MB_OK );
		}
	};
	mtest::publishable_sample_NodecppWrapperForSubscriber<SubscriptionState, PoolForMainThreadT> mySubscriberStateWrapper;

public:
	MainWindow() : mySubscriberStateWrapper( mqPool ) {};

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

		GMQHwndTransport<GMQueueStatePublisherSubscriberTypeInfo> transport( gmqueue, hMainWnd, WM_USER + 1 );
		mqPool.setTransport( &transport );

		// NODECPP-required
		// not it's time to initialize our means
		ThreadStartupData data;
		WorkerThreadPostman postman( &hMainWnd ); // see above-mention magic
		preinitThreadStartupData( data, &postman );
		setThisThreadDescriptor( data);
		someNodeAddress = nodecpp::runNodeInAnotherThread<SomeNode>( useQueuePostman(), SomeNodeName );

		globalmq::marshalling::GmqPathHelper::PathComponents pc;
		pc.authority = "";
		pc.nodeName = SomeNodeName;
		pc.statePublisherOrConnPeerName = mtest::publishable_sample_NodecppWrapperForSubscriber<SubscriptionState, PoolForMainThreadT>::stringTypeID;
		GMQ_COLL string path = globalmq::marshalling::GmqPathHelper::compose( globalmq::marshalling::GmqPathHelper::Type::subscriptionRequest, pc );

		mySubscriberStateWrapper.subscribe( path );

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
			mtest::infrastructural::composeMessage<mtest::infrastructural::ScreenPoint>( msg, mtest::x = x, mtest::y = y );
			postInfrastructuralMsg( std::move( msg ), someNodeAddress );
			break;
		}
		case WM_USER:
		{
			InterThreadMsgPtr iptr( wParam );
			InterThreadMsg msg( iptr );
			switch ( msg.msgType )
			{
				case InterThreadMsgType::Infrastructural:
				{
					mtest::infrastructural::handleMessage( msg.msg,
						mtest::makeMessageHandler<mtest::infrastructural::ScreenPoint>([&](auto& parser){ 
							int x;
							int y;
							mtest::STRUCT_ScreenPoint_parse( parser, mtest::x = &(x), mtest::y = &(y) );
							auto f = fmt::format( "x = {}\ny = {}", x, y );
							MessageBox( hWnd, f.c_str(), "Point", MB_OK );
						}),
						mtest::makeDefaultMessageHandler([&](auto& parser, uint64_t msgID){ 
							auto f = fmt::format( "Unhandled message {}\n", msgID ); 
							MessageBox( hWnd, f.c_str(), "Point", MB_OK );
						})
					);
					break;
				}
				case InterThreadMsgType::GlobalMQ:
				{
					assert( false ); // unexpected
					break;
				}
			}
			break;
		}
		case WM_USER + 1:
		{
			nodecpp::platform::internal_msg::InternalMsg msg;
			msg.restoreFromPointer( reinterpret_cast<nodecpp::platform::internal_msg::InternalMsg*>( wParam ) );
			mqPool.onMessage( msg );
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
#ifdef NODECPP_USE_GMQUEUE
	using BufferT = GMQueueStatePublisherSubscriberTypeInfo::BufferT;
	using ComposerT = GMQueueStatePublisherSubscriberTypeInfo::ComposerT;

	// Init GMGueue
	gmqueue.template initStateConcentratorFactory<mtest::StateConcentratorFactory<BufferT, ComposerT>>();
	gmqueue.setAuthority( "" );
#endif

	return mainWindow.main( hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
