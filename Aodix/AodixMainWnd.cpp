/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Main Window Procedure Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK main_wnd_proc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
	// extern aodix core pointer
	extern CAodixCore*	gl_padx;

	// command message
	if(message==WM_COMMAND)
	{
		gl_padx->gui_command(hwnd,LOWORD(wparam));
		return 0;
	}

	// paint message
	if(message==WM_PAINT)
	{
		// temporal stack
		RECT cr;
		PAINTSTRUCT ps;

		// get client size
		GetClientRect(hwnd,&cr);

		// get window hdc for paint
		HDC const hdc=BeginPaint(hwnd,&ps);

		// paint gui in memory dc
		gl_padx->paint(hwnd,gl_padx->hdc_gui,cr.right,cr.bottom);

		// blit gui dc to physical paint dc
		BitBlt(hdc,0,0,cr.right,cr.bottom,gl_padx->hdc_gui,0,0,SRCCOPY);

		// finnish window paint
		EndPaint(hwnd,&ps);
		return 0;
	}

	// gui timer message
	if(message==WM_TIMER)
	{
		gl_padx->gui_timer(hwnd);
		return 0;
	}

	// key down message
	if(message==WM_KEYDOWN)
	{
		gl_padx->gui_key_down(hwnd,wparam,lparam);
		return 0;
	}

	// key up message
	if(message==WM_KEYUP)
	{
		gl_padx->gui_key_up(hwnd,wparam);
		return 0;
	}

	// left mouse button down
	if(message==WM_LBUTTONDOWN)
	{
		gl_padx->gui_mouse_down(hwnd,false);
		return 0;
	}

	// left mouse button double click
	if(message==WM_LBUTTONDBLCLK)
	{
		gl_padx->gui_mouse_down(hwnd,true);
		return 0;
	}

	// left mouse button up
	if(message==WM_LBUTTONUP)
	{
		gl_padx->gui_mouse_up(hwnd);
		return 0;
	}

	// mouse move
	if(message==WM_MOUSEMOVE)
	{
		gl_padx->gui_mouse_move(hwnd);
		return 0;
	}

	// mouse wheel
	if(message==0x20A)
	{
		gl_padx->gui_mouse_wheel(hwnd,short(HIWORD(wparam)));
		return 0;
	}

	// mouse drop down
	if(message==WM_INITMENU)
	{
		// get mainframe menu handler
		HMENU const hmenu=GetMenu(hwnd);

		// aodix mainframe menu dropdown
		if(HMENU(wparam)==hmenu)
		{

			// enable save and save as items
			EnableMenuItem(GetSubMenu(hmenu,0),3,MF_BYPOSITION | MF_ENABLED);
			EnableMenuItem(GetSubMenu(hmenu,0),4,MF_BYPOSITION | MF_ENABLED);

			// update recent file list
			for(int rf=0;rf<NUM_RECENT_FILES;rf++)
			{
				char buf[_MAX_PATH];
				sprintf(buf,"%.2d: %s",rf+1,gl_padx->cfg.recent_file[rf]);
				ModifyMenu(GetSubMenu(hmenu,0),15+rf,MF_BYPOSITION,ID_FILE_RECENTFILE1+rf,buf);
			}
		}

		return 0;
	}

	// close window
	if(message==WM_CLOSE)
	{
		// close window
		if(MessageBox(hwnd,"Are You Sure?","Aodix - Close",MB_YESNO | MB_ICONQUESTION)==IDYES)
		{
			// close asio
			gl_padx->asio_close(hwnd);

			// kill main timer
			KillTimer(hwnd,1);

			// destroy window
			DestroyWindow(hwnd);
		}

		return 0;
	}

	// destroy window
	if(message==WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}

	// colour text labels
	if(message==WM_CTLCOLOREDIT)
	{
		// Set the text background color.
		SetBkColor((HDC)wparam,GetPixel(gl_padx->hdc_gfx,321,321));

		// Set the text foreground color.
		SetTextColor((HDC)wparam,0xFFFFFF);

		// Return the control background brush.
		return (LRESULT)gl_padx->hbrush_edit;
	}

	// else (default procedure)
	return DefWindowProc(hwnd,message,wparam,lparam);
}
