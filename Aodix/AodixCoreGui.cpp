/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Gui Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WNDPROC gl_wp_orig_edit_proc;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// keynote strings
char const gl_str_not[]={"C-C#D-D#E-F-F#G-G#A-A#B-"};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT APIENTRY gui_edit_subclass_proc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam) 
{
	// extern aodix core pointer and window
	extern CAodixCore*	gl_padx;
	extern HWND			gl_hwnd_main;

	// handle return
	if(uMsg==WM_KEYDOWN && wParam==VK_RETURN)
	{
		gl_padx->gui_process_text_action(gl_hwnd_main);
		return 0;
	}

	// original procedure
	return CallWindowProc(gl_wp_orig_edit_proc,hwnd,uMsg,wParam,lParam); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_create_edit(HWND const hwnd,int x,int y,int const w,int const h,char* default_text,int text_action_id)
{
	// destroy previous text box if popup textbox is already opened
	if(user_edit_text_action_id>0)
		DestroyWindow(hwnd_edit);

	// create edit box
	hwnd_edit=CreateWindow("EDIT",NULL,WS_CHILD | WS_VISIBLE | ES_LEFT,x+4,y+4,w-8,h-8,hwnd,(HMENU)NULL,(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),NULL);

	// subclass the edit control.
	gl_wp_orig_edit_proc=(WNDPROC)SetWindowLong(hwnd_edit,GWL_WNDPROC,(LONG)gui_edit_subclass_proc); 

	// set action id
	user_edit_text_action_id=text_action_id;

	// add text to the window.
	SendMessage(hwnd_edit,WM_SETTEXT,0,(LPARAM)default_text); 

	// set all selection
	SendMessage(hwnd_edit,EM_SETSEL,0,-1); 

	// set font
	SendMessage(hwnd_edit,WM_SETFONT,(UINT)hfont_terminal,TRUE);

	// set text focus
	SetFocus(hwnd_edit);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_process_text_action(HWND const hwnd)
{
	// check if textbox action is performed
	if(user_edit_text_action_id>0)
	{
		// string holder
		char str[32];

		// get text (max 32 chars, incl 0)
		GetWindowText(hwnd_edit,str,32);

		// change program name
		if(user_edit_text_action_id==1)
		{
			// get instance
			ADX_INSTANCE* pi=&instance[user_instance];

			// check if effect is instanced
			if(pi->peffect!=NULL)
			{
				// clamp vst program name
				str[24]=0;

				// dispatch set program name
				pi->peffect->dispatcher(pi->peffect,effSetProgramName,0,0,str,0.0f);

				// update vst editor window caption
				host_update_window_caption(pi->peffect);
			}
		}

		// change track name
		if(user_edit_text_action_id>=2 && user_edit_text_action_id<(2+MAX_TRACKS))
		{
			//get track pointer
			ADX_TRACK* pt=&project.pattern[user_pat].track[user_edit_text_action_id-2];

			// clear and rename track label
			memset(pt->name,0,32);
			sprintf(pt->name,str);
		}

		// change current pattern name
		if(user_edit_text_action_id==512)
		{
			//get pattern pointer
			ADX_PATTERN* pp=&project.pattern[user_pat];

			// clear and rename pattern label
			memset(pp->name,0,32);
			sprintf(pp->name,str);
		}

		// change project name
		if(user_edit_text_action_id==513)
		{
			// clear and rename project label
			memset(project.name,0,32);
			sprintf(project.name,str);
		}

		// change project info
		if(user_edit_text_action_id==514)
		{
			// clear and rename project info
			memset(project.info,0,32);
			sprintf(project.info,str);
		}

		// change current instance alias
		if(user_edit_text_action_id==515)
		{
			// get instance
			ADX_INSTANCE* pi=&instance[user_instance];

			// check if effect is instanced
			if(pi->peffect!=NULL)
			{
				// clear and change instance alias label
				memset(pi->alias,0,32);
				sprintf(pi->alias,str);

				// update vst editor window caption
				host_update_window_caption(pi->peffect);
			}
		}

		// end text action
		user_edit_text_action_id=0;

		// destroy the text window
		DestroyWindow(hwnd_edit);

		// post refresh
		gui_is_dirty=1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_format_note(int const key,char* buf)
{
	// format buffer
	buf[0]=gl_str_not[(key%12)*2+0];
	buf[1]=gl_str_not[(key%12)*2+1];
	buf[2]='0'+key/12;
	buf[3]=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_format_pos(int const pos,char* buf)
{
	sprintf(buf,"%.3d:%.1d:%.3d",pos/(project.master_numerator*project.master_ppqn),(pos/project.master_ppqn)%project.master_numerator,pos%project.master_ppqn);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_format_time(int const samples,char* buf)
{
	// calculate double time
	double const samples_per_ms=cfg.asio_driver_sample_rate*0.001;
	double const ms=double(samples)/samples_per_ms;

	// integer time
	int const i_mse=int(ms);
	int const i_sec=i_mse/1000;

	// format string
	sprintf(buf,"%.2d:%.2d:%.3d",(i_sec/60)%60,i_sec%60,i_mse%1000);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_timer(HWND const hwnd)
{
	// get area size
	RECT r;
	GetClientRect(hwnd,&r);
	int const w=r.right-r.left;
	int const h=r.bottom-r.top;

	// temporal vars
	int const seq_area_y=264;
	int const seq_area_h=h-seq_area_y;
	int const seq_cent_y=seq_area_y+seq_area_h/2;
	int const seq_visi_t=(w-user_pr_width)/TRACK_WIDTH;

	// get master play position
	int const play_pos=seq_sample_to_pos(master_transport_sampleframe);

	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// temporal char holder
	char buf[256];

	// get paint dc
	HDC const hdc=GetDC(hwnd);

	// update watchclock(s) when transport active (playing)
	if(master_time_info.flags & kVstTransportPlaying)
	{
		// master position display
		paint_seq_pos_big(hdc,10,25,play_pos);

		// master timer		
		gui_format_time(master_transport_sampleframe,buf);
		paint_txt(hdc,135,21,buf,9,2);
	}

	// master tempo
	sprintf(buf,"%.3d.%.1d",int(project.master_tempo),int(project.master_tempo*10.0)%10);
	paint_txt(hdc,71,53,buf,5,0);

	// master cpu
	sprintf(buf,"%.2d.%.1d",int(dsp_cpu_cost),int(dsp_cpu_cost*10.0)%10);
	paint_txt(hdc,159,37,buf,4,2);

	// refresh master output vumeter
	paint_master_vumeter(hdc);

	// master vumeter decay
	for(int o=0;o<NUM_DSP_OUTPUTS;o++)
	{
		// check vumeter level
		if(dsp_output_vumeter[o]>1.0e-8f)
			dsp_output_vumeter[o]*=0.9f;
	}

	// update track midi vumeters
	if(user_page==0)
		paint_track_midi_vumeters(hdc,TRACK_WIDTH+user_pr_width,seq_area_y-30,seq_visi_t);

	// transport playback position
	if((master_time_info.flags & kVstTransportPlaying) && user_page==0)
	{
		// sequencer clip region
		arg_gdi_set_clip_rgn(hdc,0,seq_area_y,w,seq_area_h);

		// invert player transport rect
		int const i_ty=seq_cent_y+(play_pos-pp->usr_pos)/pp->usr_ppp;

		// only paint bar if y screen position has changed
		if(i_ty!=master_transport_last_y)
		{
			// clear last inversion
			arg_gdi_paint_invert_rect(hdc,0,master_transport_last_y,w,2);

			// paint play invert-bar pos
			arg_gdi_paint_invert_rect(hdc,0,i_ty,w,2);

			// register last screen-y position
			master_transport_last_y=i_ty;

			// scroll page follow view if live active
			if(cfg.rec_live && i_ty>h)
			{
				// set new position
				pp->usr_pos=edit_quantize(arg_tool_clipped_assign(play_pos+(seq_area_h/2)*pp->usr_ppp,0,MAX_SIGNED_INT));
				
				// post refresh
				gui_is_dirty=1;
			}
		}

		// end clip
		arg_gdi_end_clip_rgn(hdc);
	}

	// waiting midi-input cc (flash rec-learn indicator)
	if(user_midi_learn)
		BitBlt(hdc,853,177,32,16,hdc_gfx,128+((GetTickCount()/512)&1)*32,572,SRCCOPY);

	// end paint dc
	ReleaseDC(hwnd,hdc);

	// dispatch update flag
	if(gui_is_dirty)
	{
		// set update flag off
		gui_is_dirty=0;

		// post redraw
		InvalidateRect(hwnd,NULL,FALSE);
	}

	// host idle
	host_idle();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_delete_current_instance(HWND const hwnd)
{
	// get current selected instance pointer
	ADX_INSTANCE* pi=&instance[user_instance];

	// confirm dialog
	if(pi->peffect!=NULL && MessageBox(hwnd,"Warning: This Operation Cant Be Undone\n\nAre You Sure?","Aodix - Delete Instance",MB_YESNO | MB_ICONWARNING)==IDYES)
	{
		// enter critical section
		asio_enter_cs();

		// disconnect master input wires connected to current machine
		for(int p=0;p<NUM_DSP_INPUTS;p++)
		{
			// get master input pin pointer
			ADX_PIN* pp=&master_input_pin[p];

			// scan wires
			for(int w=0;w<pp->num_wires;w++)
			{
				// get master input wire pointer
				ADX_WIRE* pw=&pp->pwire[w];

				// disconnect wire
				if(pw->instance_index==user_instance)
					edit_del_wire(pp,w--);
			}
		}

		// disconnect all machines connected to current machine (wich will be deleted)
		for(int i=0;i<MAX_INSTANCES;i++)
		{
			// ignore current machine
			if(i!=user_instance)
			{
				// get source instance
				ADX_INSTANCE* pcoi=&instance[i];

				// check if source instance is loaded
				if(pcoi->peffect!=NULL)
				{
					// scan source output pins
					for(int o=0;o<pcoi->peffect->numOutputs;o++)
					{
						// get master input pin pointer
						ADX_PIN* pp=&pcoi->pout_pin[o];

						// scan wires
						for(int w=0;w<pp->num_wires;w++)
						{
							// get instance wire pointer
							ADX_WIRE* pw=&pp->pwire[w];

							// disconnect wire
							if(pw->instance_index==user_instance)
								edit_del_wire(pp,w--);
						}
					}

					// scan source midi-out link wires
					for(int w=0;w<pcoi->mout_pin.num_wires;w++)
					{
						// get instance wire pointer
						ADX_WIRE* pw=&pcoi->mout_pin.pwire[w];

						// if source midi-out link is connected to current instance to be deleted, unwire that connection
						if(pw->instance_index==user_instance)
							edit_del_wire(&pcoi->mout_pin,w--);
					}
				}
			}
		}

		// free instance
		instance_free(pi);

		// leave critical section
		asio_leave_cs();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_scroll_to_instance()
{
	// instance before instance list offset
	if(user_instance<user_instance_list_offset)
		user_instance_list_offset=user_instance;

	// instance after instance list offset
	if(user_instance>(user_instance_list_offset+10))
		user_instance_list_offset=user_instance-10;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_scroll_iterate(HWND const hwnd,int const ym,int const seq_area_y,int const h)
{
	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// top scroller
	if(ym<seq_area_y)
		pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos-pp->usr_ppp*8,0,MAX_SIGNED_INT);

	// bottom scroller
	if(ym>(h-19))
		pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos+pp->usr_ppp*8,0,MAX_SIGNED_INT);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAodixCore::gui_vst_search(HMENU const pa_hmenu,char* pfolder)
{
	// get vst library pointer
	ADX_VST_LIB* plib=&vst_lib;

	// set find path
	char findpath[_MAX_PATH];
	sprintf(findpath,"%s\\*.*",pfolder);

	// data variables
	WIN32_FIND_DATA ffd;
	HANDLE hFind=FindFirstFile(findpath,&ffd);

	// no files found
	if(hFind==INVALID_HANDLE_VALUE) 
		return 0;

	// found items
	int item_count=0;

	// search dlls
	do
	{
		// check no dots
		if(ffd.cFileName[0]!='.')
		{
			if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// directory
				HMENU const h_sub_menu=CreatePopupMenu();

				// configure subpath string
				char subfindpath[_MAX_PATH];
				sprintf(subfindpath,"%s\\%s",pfolder,ffd.cFileName);

				// subsearch files in dir (recursive), and append dir
				if(gui_vst_search(h_sub_menu,subfindpath)>0)
				{
					// append menu
					if(item_count>0 && (item_count&0x1F)==0)
						AppendMenu(pa_hmenu,MFT_STRING | MF_POPUP | MF_MENUBARBREAK,(UINT_PTR)h_sub_menu,ffd.cFileName);
					else
						AppendMenu(pa_hmenu,MFT_STRING | MF_POPUP,(UINT_PTR)h_sub_menu,ffd.cFileName);

					// increment found items
					item_count++;
				}
				else
				{
					// no dll items found in subdir
					DestroyMenu(h_sub_menu);
				}
			}
			else
			{
				// is dll extension flag
				bool is_dll_gext=false;

				// get filename length
				int const fstrl=strlen(ffd.cFileName);

				// lowercase extension
				_strlwr(ffd.cFileName+fstrl-4);

				// check for dll extension
				if(ffd.cFileName[fstrl-4]=='.' && ffd.cFileName[fstrl-3]=='d' && ffd.cFileName[fstrl-2]=='l' && ffd.cFileName[fstrl-1]=='l')
					is_dll_gext=true;

				// is dll extension? (and check for max dll's count list overflow)
				if(is_dll_gext && plib->num_dlls<MAX_VST_LIB_ENTRIES)
				{
					// temp item
					MENUITEMINFO item;

					// dll label
					char dll_label[_MAX_PATH];
					sprintf(dll_label,ffd.cFileName);
					dll_label[strlen(dll_label)-4]=0;

					// fill up the menu struct
					item.cbSize		=	sizeof(MENUITEMINFO);
					item.fMask		=	MIIM_ID | MIIM_TYPE | MIIM_SUBMENU | MIIM_STATE;
					item.hSubMenu	=	NULL;
					item.wID		=	IDM_VST_LIB+plib->num_dlls;
					item.dwTypeData	=	dll_label;
					item.cch		=	UINT(strlen(dll_label));
					item.fState		=	MFS_ENABLED;

					if(item_count>0 && (item_count&0x1F)==0)
						item.fType	=	MFT_STRING | MF_MENUBARBREAK;
					else
						item.fType	=	MFT_STRING;

					// insert into menu
					InsertMenuItem(pa_hmenu,0,FALSE,&item);

					// configure dll path string
					char dll_path[_MAX_PATH];
					sprintf(dll_path,"%s\\%s",pfolder,ffd.cFileName);

					// add to dll library
					sprintf(plib->dll_path[plib->num_dlls++],dll_path);

					// increment found items
					item_count++;
				}
			}
		}
	}
	while(FindNextFile(hFind,&ffd));

	// close find
	FindClose(hFind);

	// return num found items
	return item_count;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_vst_build_lib(void)
{
	// destroy previous library menu if already allocated
	if(hmenu_vst_lib!=NULL)
		DestroyMenu(hmenu_vst_lib);

	// create new menu
	hmenu_vst_lib=CreatePopupMenu();

	// set no dlls found
	vst_lib.num_dlls=0;

	// fill vst dll library menu dir tree
	for(int pi=0;pi<MAX_VST_FOLDERS;pi++)
	{
		// search in folder if configured
		if(cfg.vst_path[pi][0]!=0)
			gui_vst_search(hmenu_vst_lib,cfg.vst_path[pi]);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAodixCore::gui_get_event_height(ADX_EVENT* pe)
{
	return arg_tool_clipped_assign((1-pe->szd)*NR_EVENT_HEIGHT+pe->szd*(pe->par/project.pattern[user_pat].usr_ppp),2,MAX_SIGNED_INT);
}
