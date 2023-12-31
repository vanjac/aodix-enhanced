/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Plugin Window Procedure Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK plug_wnd_proc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
	// extern aodix core pointer
	extern CAodixCore* gl_padx;

	// main application window handler
	extern HWND	gl_hwnd_main;

	// get vst effect pointer
	AEffect* peffect=(AEffect*)GetWindowLong(hwnd,GWL_USERDATA);

	// paint message
	if(message==WM_PAINT)
	{
		// client rectangle
		RECT r;

		// get window rect
		GetClientRect(hwnd,&r);

		// paint structure
		PAINTSTRUCT ps;

		// get window hdc for paint
		HDC const hdc=BeginPaint(hwnd,&ps);

		// paint gray frame
		FillRect(hdc,&r,0);

		// finish paint
		EndPaint(hwnd,&ps);
		return 0;
	}

	// close window message
	if(message==WM_CLOSE)
	{
		// check plugin editor window
		if(peffect!=NULL && (peffect->flags & effFlagsHasEditor))
			peffect->dispatcher(peffect,effEditClose,0,0,NULL,0.f);

		// destroy window
		DestroyWindow(hwnd);
		return 0;
	}

	// key down message
	if(message==WM_KEYDOWN)
	{
		if(gl_padx->asio_is_inited())
			gl_padx->gui_key_down(gl_hwnd_main,wparam,lparam);
		return 0;
	}

	// key up message
	if(message==WM_KEYUP)
	{
		if(gl_padx->asio_is_inited())
			gl_padx->gui_key_up(gl_hwnd_main,wparam);
		return 0;
	}

	// else (return default window proc)
	return DefWindowProc(hwnd,message,wparam,lparam);
}
