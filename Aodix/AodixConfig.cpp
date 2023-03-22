/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Config Dialog Procedure Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void conf_browse_folder(HWND const hwnd,char* buf)
{
	// init browse info struct
	BROWSEINFO BrowseInfo;
	memset(&BrowseInfo,0,sizeof(BrowseInfo));
	BrowseInfo.hwndOwner=hwnd;
	BrowseInfo.pszDisplayName=buf;
	BrowseInfo.lpszTitle="Select Directory";
	BrowseInfo.ulFlags=BIF_RETURNONLYFSDIRS;

	// throw display dialog
	LPITEMIDLIST pList=SHBrowseForFolder(&BrowseInfo);

	if(pList!=NULL)
	{
		// convert from MIDLISt to real string path
		SHGetPathFromIDList(pList,buf);

		// global pointer to the shell's IMalloc interface.  
		LPMALLOC pMalloc; 

		// Get the shell's allocator and free the PIDL returned by
		// SHBrowseForFolder.
		if (SUCCEEDED(SHGetMalloc(&pMalloc))) 
			pMalloc->Free(pList);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK conf_dlg_proc(HWND hdlg,UINT message,WPARAM wparam,LPARAM lparam)
{
	// extern aodix core pointer and handlers
	extern CAodixCore* gl_padx;

	// main application window handler
	extern HWND	gl_hwnd_main;

	// strings for window caption
	char buf[_MAX_PATH];

	// init dialog
	if(message==WM_INITDIALOG)
	{
		// get user key and license edit pointers
		HWND const hwnd_user_edit=GetDlgItem(hdlg,IDC_USER_NAME_EDIT);
		
		// set user key and license key
		SetWindowText(hwnd_user_edit,gl_padx->cfg.user_name);
		EnableWindow(hwnd_user_edit,TRUE);

		// get asio driver combo pointer
		HWND const hwnd_asio_comb=GetDlgItem(hdlg,IDC_ASIO_DRIVER_COMBO);

		// fill asio menu with ASIO drivers
		if(gl_padx->asio_num_drivers>0)
		{
			// check if current device is in asio list range, else default null driver
			if(gl_padx->cfg.asio_driver_id>=gl_padx->asio_num_drivers)
				gl_padx->cfg.asio_driver_id=-1;

			// fill asio menu with asio driver names
			for(long asio_di=0;asio_di<gl_padx->asio_num_drivers;asio_di++)
			{
				// add driver option
				gl_padx->asio_get_driver_name(asio_di,buf,32);
				SendMessage(hwnd_asio_comb,(UINT)CB_ADDSTRING,0,(LPARAM)buf);
			}

			// set asio driver combo selected index
			SendMessage(hwnd_asio_comb,(UINT)CB_SETCURSEL,gl_padx->cfg.asio_driver_id,0);
		}
		else
		{
			// no asio drivers found, disable menu
			EnableWindow(hwnd_asio_comb,FALSE);

			// set null driver
			gl_padx->cfg.asio_driver_id=-1;
		}

		// get asio sample rate combo pointer
		HWND const hwnd_rate_comb=GetDlgItem(hdlg,IDC_ASIO_RATE_COMBO);

		// add rate options
		SendMessage(hwnd_rate_comb,(UINT)CB_ADDSTRING,0,(LPARAM)"8000 Hz");
		SendMessage(hwnd_rate_comb,(UINT)CB_ADDSTRING,0,(LPARAM)"11025 Hz (Low CPU)");
		SendMessage(hwnd_rate_comb,(UINT)CB_ADDSTRING,0,(LPARAM)"22050 Hz");
		SendMessage(hwnd_rate_comb,(UINT)CB_ADDSTRING,0,(LPARAM)"32000 Hz");
		SendMessage(hwnd_rate_comb,(UINT)CB_ADDSTRING,0,(LPARAM)"44100 Hz (CD Audio, MP3)");
		SendMessage(hwnd_rate_comb,(UINT)CB_ADDSTRING,0,(LPARAM)"48000 Hz (DAT)");
		SendMessage(hwnd_rate_comb,(UINT)CB_ADDSTRING,0,(LPARAM)"88200 Hz");
		SendMessage(hwnd_rate_comb,(UINT)CB_ADDSTRING,0,(LPARAM)"96000 Hz (Production)");
		SendMessage(hwnd_rate_comb,(UINT)CB_ADDSTRING,0,(LPARAM)"192000 Hz");

		// set asio sample rate combo selected index
		switch(int(gl_padx->cfg.asio_driver_sample_rate))
		{
		case 8000:		SendMessage(hwnd_rate_comb,(UINT)CB_SETCURSEL,0,0);	break;
		case 11025:		SendMessage(hwnd_rate_comb,(UINT)CB_SETCURSEL,1,0);	break;
		case 22050:		SendMessage(hwnd_rate_comb,(UINT)CB_SETCURSEL,2,0);	break;
		case 32000:		SendMessage(hwnd_rate_comb,(UINT)CB_SETCURSEL,3,0);	break;
		case 44100:		SendMessage(hwnd_rate_comb,(UINT)CB_SETCURSEL,4,0);	break;
		case 48000:		SendMessage(hwnd_rate_comb,(UINT)CB_SETCURSEL,5,0);	break;
		case 88200:		SendMessage(hwnd_rate_comb,(UINT)CB_SETCURSEL,6,0);	break;
		case 96000:		SendMessage(hwnd_rate_comb,(UINT)CB_SETCURSEL,7,0);	break;
		case 192000:	SendMessage(hwnd_rate_comb,(UINT)CB_SETCURSEL,8,0);	break;
		}

		// set vst path routes
		SetDlgItemText(hdlg,IDC_VST_PATH_EDIT1,gl_padx->cfg.vst_path[0]);
		SetDlgItemText(hdlg,IDC_VST_PATH_EDIT2,gl_padx->cfg.vst_path[1]);
		SetDlgItemText(hdlg,IDC_VST_PATH_EDIT3,gl_padx->cfg.vst_path[2]);
		SetDlgItemText(hdlg,IDC_VST_PATH_EDIT4,gl_padx->cfg.vst_path[3]);

		// set skin path
		SetDlgItemText(hdlg,IDC_SKIN_PATH_EDIT,gl_padx->cfg.skin_path);

		// get kbd layout combo pointer
		HWND const hwnd_kbd_comb=GetDlgItem(hdlg,IDC_KBD_LAYOUT_COMBO);

		// add kbd layout combo options
		SendMessage(hwnd_kbd_comb,(UINT)CB_ADDSTRING,0,(LPARAM)"1. QWERTY (Standard, US, Etc)");
		SendMessage(hwnd_kbd_comb,(UINT)CB_ADDSTRING,0,(LPARAM)"2. AZERTY (French)");
		SendMessage(hwnd_kbd_comb,(UINT)CB_ADDSTRING,0,(LPARAM)"3. QWERTZ (German)");

		// set kbd layout combo selected index
		SendMessage(hwnd_kbd_comb,(UINT)CB_SETCURSEL,gl_padx->cfg.keyboard_layout,0);

		// get midi in list pointer
		HWND const hwnd_midi_in_list=GetDlgItem(hdlg,IDC_MIDI_IN_LIST);

		// get midi input devices
		int const num_midi_in_devices=midiInGetNumDevs();

		// init midi in list
		if(num_midi_in_devices>0)
		{
			for(int mid=0;mid<num_midi_in_devices;mid++)
			{
				MIDIINCAPS mic;
				midiInGetDevCaps(mid,&mic,sizeof(MIDIINCAPS));
				sprintf(buf,"%.3d: %s (Vers. %d)",mid,mic.szPname,mic.vDriverVersion);
				SendMessage(hwnd_midi_in_list,(UINT)LB_ADDSTRING,0,(LPARAM)buf);
			}
		}
		else
		{
			// no midi in devices
			SendMessage(hwnd_midi_in_list,(UINT)LB_ADDSTRING,0,(LPARAM)"No MIDI Input Devices");
			EnableWindow(hwnd_midi_in_list,FALSE);
		}

		// check midi in options
		CheckDlgButton(hdlg,IDC_CHECK_MIDI_CH_ROUT,gl_padx->cfg.midi_in_ch_rout);
		CheckDlgButton(hdlg,IDC_CHECK_MIDI_VL_ROUT,gl_padx->cfg.midi_in_vl_rout);
		CheckDlgButton(hdlg,IDC_CHECK_MIDI_IN_OPEN,gl_padx->cfg.midi_in_dv_open);

		// check instance autolink to master
		CheckDlgButton(hdlg,IDC_AUTOLINK_CHECK,gl_padx->cfg.instance_autolink);

		// check fullscreen option
		CheckDlgButton(hdlg,IDC_FULLSCREEN_CHECK,gl_padx->cfg.fullscreen);

		return TRUE;
	}

	// command
	if(message==WM_COMMAND)
	{
		// get param low word
		int const lw_par=LOWORD(wparam);

		// asio control pannel
		if(lw_par==IDC_ASIO_CONTROL_PANEL)
		{
			ASIOControlPanel();
			return TRUE;
		}

		// vst folder browser 1
		if(lw_par==IDC_VST_PATH_BUTTON1)
		{
			GetDlgItemText(hdlg,IDC_VST_PATH_EDIT1,buf,_MAX_PATH);
			conf_browse_folder(hdlg,buf);
			SetDlgItemText(hdlg,IDC_VST_PATH_EDIT1,buf);
			return TRUE;
		}

		// vst folder browser 2
		if(lw_par==IDC_VST_PATH_BUTTON2)
		{
			GetDlgItemText(hdlg,IDC_VST_PATH_EDIT2,buf,_MAX_PATH);
			conf_browse_folder(hdlg,buf);
			SetDlgItemText(hdlg,IDC_VST_PATH_EDIT2,buf);
			return TRUE;
		}

		// vst folder browser 3
		if(lw_par==IDC_VST_PATH_BUTTON3)
		{
			GetDlgItemText(hdlg,IDC_VST_PATH_EDIT3,buf,_MAX_PATH);
			conf_browse_folder(hdlg,buf);
			SetDlgItemText(hdlg,IDC_VST_PATH_EDIT3,buf);
			return TRUE;
		}

		// vst folder browser 4
		if(lw_par==IDC_VST_PATH_BUTTON4)
		{
			GetDlgItemText(hdlg,IDC_VST_PATH_EDIT4,buf,_MAX_PATH);
			conf_browse_folder(hdlg,buf);
			SetDlgItemText(hdlg,IDC_VST_PATH_EDIT4,buf);
			return TRUE;
		}

		// skin folder browser
		if(lw_par==IDC_SKIN_PATH_BUTTON)
		{
			GetDlgItemText(hdlg,IDC_SKIN_PATH_EDIT,buf,_MAX_PATH);
			conf_browse_folder(hdlg,buf);
			SetDlgItemText(hdlg,IDC_SKIN_PATH_EDIT,buf);
			return TRUE;
		}

		// dialog ok, accept changes
		if(lw_par==IDC_APPLY_BUTTON || lw_par==IDOK) 
		{
			// set wait cursor
			SetCursor(gl_padx->hcursor_wait);

			// get user name
			GetDlgItemText(hdlg,IDC_USER_NAME_EDIT,gl_padx->cfg.user_name,32);

			// get asio driver and sample rate combo pointer
			HWND const hwnd_asio_comb=GetDlgItem(hdlg,IDC_ASIO_DRIVER_COMBO);
			HWND const hwnd_rate_comb=GetDlgItem(hdlg,IDC_ASIO_RATE_COMBO);

			// check if there's available asio drivers
			if(gl_padx->asio_num_drivers>0)
			{
				// shutdown asio driver
				gl_padx->asio_close(hdlg);

				// select new driver
				gl_padx->cfg.asio_driver_id=SendMessage(hwnd_asio_comb,(UINT)CB_GETCURSEL,0,0);

				// select sample rate option index
				switch(SendMessage(hwnd_rate_comb,(UINT)CB_GETCURSEL,0,0))
				{
				case 0:	gl_padx->cfg.asio_driver_sample_rate=8000.0;	break;
				case 1:	gl_padx->cfg.asio_driver_sample_rate=11025.0;	break;
				case 2:	gl_padx->cfg.asio_driver_sample_rate=22050.0;	break;
				case 3:	gl_padx->cfg.asio_driver_sample_rate=32000.0;	break;
				case 4:	gl_padx->cfg.asio_driver_sample_rate=44100.0;	break;
				case 5:	gl_padx->cfg.asio_driver_sample_rate=48000.0;	break;
				case 6:	gl_padx->cfg.asio_driver_sample_rate=88200.0;	break;
				case 7:	gl_padx->cfg.asio_driver_sample_rate=96000.0;	break;
				case 8:	gl_padx->cfg.asio_driver_sample_rate=192000.0;	break;
				}

				// re-launch asio
				gl_padx->asio_init(hdlg);
			}

			// set new vst paths
			GetDlgItemText(hdlg,IDC_VST_PATH_EDIT1,gl_padx->cfg.vst_path[0],_MAX_PATH);
			GetDlgItemText(hdlg,IDC_VST_PATH_EDIT2,gl_padx->cfg.vst_path[1],_MAX_PATH);
			GetDlgItemText(hdlg,IDC_VST_PATH_EDIT3,gl_padx->cfg.vst_path[2],_MAX_PATH);
			GetDlgItemText(hdlg,IDC_VST_PATH_EDIT4,gl_padx->cfg.vst_path[3],_MAX_PATH);

			// set new skin path
			GetDlgItemText(hdlg,IDC_SKIN_PATH_EDIT,gl_padx->cfg.skin_path,_MAX_PATH);

			// refresh library
			gl_padx->gui_vst_build_lib();

			// get kbd layout combo pointer
			HWND const hwnd_kbd_comb=GetDlgItem(hdlg,IDC_KBD_LAYOUT_COMBO);

			// set new kbd layout
			gl_padx->cfg.keyboard_layout=SendMessage(hwnd_kbd_comb,(UINT)CB_GETCURSEL,0,0);

			// close midi
			gl_padx->midi_in_close();

			// set new midi in configuration
			gl_padx->cfg.midi_in_ch_rout=IsDlgButtonChecked(hdlg,IDC_CHECK_MIDI_CH_ROUT);
			gl_padx->cfg.midi_in_vl_rout=IsDlgButtonChecked(hdlg,IDC_CHECK_MIDI_VL_ROUT);
			gl_padx->cfg.midi_in_dv_open=IsDlgButtonChecked(hdlg,IDC_CHECK_MIDI_IN_OPEN);

			// set instance autolink
			gl_padx->cfg.instance_autolink=IsDlgButtonChecked(hdlg,IDC_AUTOLINK_CHECK);

			// set fullscreen option
			gl_padx->cfg.fullscreen=IsDlgButtonChecked(hdlg,IDC_FULLSCREEN_CHECK);

			// restart midi
			gl_padx->midi_in_init();

			// set arrow cursor
			SetCursor(gl_padx->hcursor_arro);

			// post refresh
			gl_padx->gui_is_dirty=1;

			// close
			EndDialog(hdlg,LOWORD(wparam));
			return TRUE;
		}

		// dialog cancel
		if(lw_par==IDC_CANCEL_BUTTON || lw_par==IDCANCEL)
		{
			// close
			EndDialog(hdlg,LOWORD(wparam));
			return TRUE;
		}
	}

	// not dispatched message
	return FALSE;
}