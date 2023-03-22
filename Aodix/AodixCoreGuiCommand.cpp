/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Gui Command
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <htmlhelp.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_command(HWND const hwnd,int const id)
{
	// get instance
	ADX_INSTANCE* pi=&instance[user_instance];
		
	// ppqn menu
	if(id>=16384 && id<=16393)
	{
		int const mo_index=id-16384;

		// select ppqn from menu option index
		switch(mo_index)
		{
		case 0:	project.master_ppqn=48;		break;
		case 1:	project.master_ppqn=60;		break;
		case 2:	project.master_ppqn=96;		break;
		case 3:	project.master_ppqn=120;	break;
		case 4:	project.master_ppqn=192;	break;
		case 5:	project.master_ppqn=240;	break;
		case 6:	project.master_ppqn=384;	break;
		case 7:	project.master_ppqn=480;	break;
		case 8:	project.master_ppqn=768;	break;
		case 9:	project.master_ppqn=960;	break;
		}

		// post refresh
		gui_is_dirty=1;
	}

	// time signature menu
	if(id>=16394 && id<=16396)
	{
		// set new signature index
		project.master_numerator=id-16391;

		// post refresh
		gui_is_dirty=1;
	}

	// quantization menu
	if(id>=16396 && id<=16426)
	{
		// prev quantization
		if(id==16396)
			user_quantize=arg_tool_clipped_assign(user_quantize-1,0,27);

		// next quantization
		if(id==16397)
			user_quantize=arg_tool_clipped_assign(user_quantize+1,0,27);

		// quantization option
		if(id>=16399)
			user_quantize=id-16399;

		// post refresh
		gui_is_dirty=1;
	}

	// midi channel menu
	if(id>=16427 && id<=16445)
	{
		// prev midi channel
		if(id==16427)
			user_midi_ch=arg_tool_clipped_assign(user_midi_ch-1,0,15);

		// next midi channel
		if(id==16428)
			user_midi_ch=arg_tool_clipped_assign(user_midi_ch+1,0,15);

		// midi channel options
		if(id>=16430)
			user_midi_ch=id-16430;

		// post refresh
		gui_is_dirty=1;
	}

	// kbd octave menu
	if(id>=16446 && id<=16457)
	{
		// prev kbd octave
		if(id==16446)
			user_kbd_note_offset=arg_tool_clipped_assign(user_kbd_note_offset-12,0,96);

		// next kbd octave
		if(id==16447)
			user_kbd_note_offset=arg_tool_clipped_assign(user_kbd_note_offset+12,0,96);

		// kbd octave option
		if(id>=16449)
			user_kbd_note_offset=(id-16449)*12;

		// post refresh
		gui_is_dirty=1;
	}

	// midi mask menu
	if(id>=16458 && id<=16465)
	{
		// set new midi mask
		user_midi_mask=user_midi_mask^(1<<(id-16458));

		// post refresh
		gui_is_dirty=1;
	}

	// instance menu
	if(pi->peffect!=NULL && id>=16466 && id<=16473)
	{
		// get index
		int const mo_index=id-16466;

		// select instance menu option
		switch(mo_index)
		{
		case 0:	instance_open_editor(pi,hwnd);		break;
		case 2:	import_cub_file_dlg(hwnd);			break;
		case 4:	export_cub_file_dlg(hwnd,1);		break;
		case 5:	export_cub_file_dlg(hwnd,0);		break;
		case 7:	gui_delete_current_instance(hwnd);	break;
		}

		// post refresh
		gui_is_dirty=1;
	}

	// instance program selector menu
	if(pi->peffect!=NULL && id>=16474 && id<(16474+pi->peffect->numPrograms))
	{
		// set current selected program
		pi->peffect->dispatcher(pi->peffect,effSetProgram,0,id-16474,NULL,0.0f);

		// update vst editor window caption
		host_update_window_caption(pi->peffect);

		// post refresh
		gui_is_dirty=1;
	}

	// pattern menu
	if(id>=17498 && id<=17753)
	{
		// set index
		user_pat=id-17498;

		// post refresh
		gui_is_dirty=1;
	}

	// vst lib instance menu
	if(id>=IDM_VST_LIB && id<(IDM_VST_LIB+vst_lib.num_dlls))
	{
		// set wait cursor
		SetCursor(hcursor_wait);

		// get index
		int const mo_index=id-IDM_VST_LIB;

		// enter critical section
		asio_enter_cs();

		// get instance pointer
		ADX_INSTANCE* pi=&instance[user_instance];

		// default instance position
		int instance_x=16+user_rout_offset_x+(user_instance&0xF)*48;
		int instance_y=16+user_rout_offset_y+(user_instance>>4)*24;

		// set instance position in mouse position
		if(user_lym>=232)
		{
			instance_x=user_rout_offset_x+user_lxm;
			instance_y=user_rout_offset_y+(user_lym-256);
		}

		// free and instance
		instance_free(pi);
		instance_dll(hwnd,pi,vst_lib.dll_path[mo_index],instance_x,instance_y);

		// leave critical section
		asio_leave_cs();

		// post refresh
		gui_is_dirty=1;

		// set arrow cursor
		SetCursor(hcursor_arro);
	}

	// asio input pin assign menu
	if(id>=17756 && id<=17785)
	{
		// assign new asio input pin
		cfg.asio_input_pin[user_input_pin]=id-17756;

		// post refresh
		gui_is_dirty=1;
	}

	// asio output pin assign menu
	if(id>=17788 && id<=17817)
	{
		// assign new asio output pin
		cfg.asio_output_pin[user_output_pin]=id-17788;

		// post refresh
		gui_is_dirty=1;
	}

	// initialize
	if(id==ID_FILE_NEW40001 && MessageBox(hwnd,"Are You Sure?","Aodix - New Project",MB_YESNO | MB_ICONWARNING)==IDYES)
	{
		// stop transport
		dsp_transport_stop();

		// enter critical section
		asio_enter_cs();

		// reset user and project
		reset_user();
		reset_project();

		// leave critical section
		asio_leave_cs();

		// post refresh
		gui_is_dirty=1;
	}

	// open aodix project
	if(id==ID_FILE_OPEN40002)
		import_adx_file_dlg(hwnd);

	// save aodix project
	if(id==ID_FILE_SAVE40025)
	{
		if(user_last_file[0]!=0)
			export_adx_file(hwnd,user_last_file);
		else
			export_adx_file_dlg(hwnd);
	}

	// save aodix project as
	if(id==ID_FILE_SAVEAS)
		export_adx_file_dlg(hwnd);

	// import midi file		
	if(id==ID_FILE_IMPORTMIDIFILE)
		import_midi_file_dlg(hwnd);

	// export midi file
	if(id==ID_FILE_EXPORTMIDIFILE)
		export_mid_file_dlg(hwnd);

	// bounce project
	if(id==ID_FILE_BOUNCE40029)
	{
		// close midi
		midi_in_close();

		// shutdown asio
		asio_close(hwnd);

		// stop transport
		master_time_info.flags&=~kVstTransportPlaying;

		// refresh (show asio close info/status)
		gui_is_dirty=1;

		// bounce dialog
		DialogBox(hinstance_app,(LPCTSTR)IDD_BOUNCE,hwnd,(DLGPROC)boun_dlg_proc);

		// re-init asio
		asio_init(hwnd);

		// restart midi
		midi_in_init();

		// post refresh
		gui_is_dirty=1;
	}

	// configuration
	if(id==ID_FILE_CONFIGURATION)
		DialogBox(hinstance_app,(LPCTSTR)IDD_CONFIG,hwnd,(DLGPROC)conf_dlg_proc);

	// exit aodix app
	if(id==ID_FILE_EXIT)
		SendMessage(hwnd,WM_CLOSE,0,0);

	// recent files
	if(id>=ID_FILE_RECENTFILE1 && id<=ID_FILE_RECENTFILE4)
	{
		// get recent file slot
		char* prf_slot_buf=cfg.recent_file[id-ID_FILE_RECENTFILE1];

		// check that recent file slot is not empty
		if(prf_slot_buf[0]!=0)
		{
			// prompt string
			char buf[64];
			sprintf(buf,"Save Changes To '%s'?",project.name);

			// prompt for save changes
			if(MessageBox(hwnd,buf,"Aodix",MB_YESNO | MB_ICONQUESTION)==IDYES)
			{
				// check for save/save as
				if(user_last_file[0]!=0)
					export_adx_file(hwnd,user_last_file);
				else
					export_adx_file_dlg(hwnd);
			}

			// set wait cursor
			SetCursor(hcursor_wait);

			// enter critical section
			asio_enter_cs();

			// load file
			import_adx_file(hwnd,prf_slot_buf);

			// leave critical section
			asio_leave_cs();

			// post refresh
			gui_is_dirty=1;

			// set arrow cursor
			SetCursor(hcursor_arro);
		}
	}

	// edit undo last operation
	if(id==ID_EDIT_UNDOCLIPBOARDOPERATION)
	{
		edit_undo();
		gui_is_dirty=1;
	}

	// edit cut
	if(id==ID_EDIT_CUT40012)
	{			
		edit_copy(1);
		gui_is_dirty=1;
	}

	// edit copy
	if(id==ID_EDIT_COPY40013)
	{
		edit_copy(0);
		gui_is_dirty=1;
	}

	// edit paste
	if(id==ID_EDIT_PASTE40014)
	{		
		edit_paste();
		gui_is_dirty=1;
	}

	// edit select all
	if(id==ID_EDIT_SELECTALL)
	{
		edit_select_all();
		gui_is_dirty=1;
	}

	// edit quantize notes
	if(id==ID_EDIT_QUANTIZENOTES)
	{
		edit_transpose(0,1);
		gui_is_dirty=1;
	}

	// edit randomize
	if(id==ID_EDIT_RANDOMIZE)
	{
		edit_randomize();
		gui_is_dirty=1;
	}

	// edit interpolate
	if(id==ID_EDIT_INTERPOLATE)
	{
		edit_interpolate();
		gui_is_dirty=1;
	}

	// show compiled html help
	if(id==ID_HELP_CONTENT)
		HtmlHelp(hwnd,hlp_fil,HH_DISPLAY_TOPIC,0);

	// about dialog
	if(id==ID_HELP_ABOUT)
		DialogBox(hinstance_app,(LPCTSTR)IDD_ABOUTBOX,hwnd,(DLGPROC)abou_dlg_proc);

	// visit aodix enhanced website
	if(id==ID_HELP_WEBSITE)
		WinExec("explorer https://chroma.zone/aodix-enhanced/",SW_SHOWNORMAL);
}
