/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Gui (Mouse Down) Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_mouse_down(HWND const hwnd,bool const is_double_click)
{
	// set mouse capture and keyboard focus
	SetCapture(hwnd);
	SetFocus(hwnd);

	// process text action
	gui_process_text_action(hwnd);

	// get area size
	RECT r;
	GetClientRect(hwnd,&r);
	int const w=r.right-r.left;
	int const h=r.bottom-r.top;

	// get mouse coordinates
	POINT cp;
	GetCursorPos(&cp);
	ScreenToClient(hwnd,&cp);
	int const xm=cp.x;
	int const ym=cp.y;

	// get sequencer area
	int const seq_area_y=264;
	int const seq_area_h=h-seq_area_y;
	int const seq_cent_y=seq_area_y+seq_area_h/2;

	// get dsp routing area
	int const rout_area_y=232;
	int const rout_area_h=h-rout_area_y;

	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// get mouse screen y sequencer position
	int const ym_seq_pos=arg_tool_clipped_assign(pp->usr_pos+int(ym-seq_cent_y)*pp->usr_ppp,0,MAX_SIGNED_INT);

	// get current pattern cue-loop markers screen coordinates
	int const y_cue_sta=seq_cent_y+(pp->cue_sta-pp->usr_pos)/pp->usr_ppp;
	int const y_cue_end=seq_cent_y+(pp->cue_end-pp->usr_pos)/pp->usr_ppp;

	// master pattern cue-loop switch
	if(arg_tool_check_plane_xy(xm,ym,195,17,32,16))
	{
		user_pressed=1;
		gui_is_dirty=1;
	}

	// master cfg live-rec switch
	if(arg_tool_check_plane_xy(xm,ym,227,17,32,16))
	{
		user_pressed=2;
		gui_is_dirty=1;
	}

	// master cfg play-scroll follow
	if(arg_tool_check_plane_xy(xm,ym,195,33,64,16))
	{
		user_pressed=3;
		gui_is_dirty=1;
	}

	// master change tempo
	if(arg_tool_check_plane_xy(xm,ym,3,49,128,16))
	{
		user_pressed=4;
		gui_is_dirty=1;
	}

	// master change ppqn
	if(arg_tool_check_plane_xy(xm,ym,131,49,128,16))
	{
		// check proper ppqn menu item
		switch(project.master_ppqn)
		{
		case 48:	CheckMenuRadioItem(hmenu_ppqn,0,9,0,MF_BYPOSITION);	break;
		case 60:	CheckMenuRadioItem(hmenu_ppqn,0,9,1,MF_BYPOSITION);	break;
		case 96:	CheckMenuRadioItem(hmenu_ppqn,0,9,2,MF_BYPOSITION);	break;
		case 120:	CheckMenuRadioItem(hmenu_ppqn,0,9,3,MF_BYPOSITION);	break;
		case 192:	CheckMenuRadioItem(hmenu_ppqn,0,9,4,MF_BYPOSITION);	break;
		case 240:	CheckMenuRadioItem(hmenu_ppqn,0,9,5,MF_BYPOSITION);	break;
		case 384:	CheckMenuRadioItem(hmenu_ppqn,0,9,6,MF_BYPOSITION);	break;
		case 480:	CheckMenuRadioItem(hmenu_ppqn,0,9,7,MF_BYPOSITION);	break;
		case 768:	CheckMenuRadioItem(hmenu_ppqn,0,9,8,MF_BYPOSITION);	break;
		case 960:	CheckMenuRadioItem(hmenu_ppqn,0,9,9,MF_BYPOSITION);	break;
		}

		// track pop-up menu
		arg_menu_track(hmenu_ppqn,hwnd,195,65);
	}

	// master change transport-signature
	if(arg_tool_check_plane_xy(xm,ym,3,65,128,16))
	{
		CheckMenuRadioItem(hmenu_timesig,0,2,project.master_numerator-3,MF_BYPOSITION);
		arg_menu_track(hmenu_timesig,hwnd,67,81);
	}

	// master transport play
	if(arg_tool_check_plane_xy(xm,ym,3,85,64,28))
	{
		user_pressed=7;
		gui_is_dirty=1;
	}

	// master transport stop
	if(arg_tool_check_plane_xy(xm,ym,67,85,64,28))
	{
		user_pressed=8;
		gui_is_dirty=1;
	}

	// master transport rec-events
	if(arg_tool_check_plane_xy(xm,ym,131,85,64,28))
	{
		user_pressed=9;
		gui_is_dirty=1;
	}

	// master transport rec-automation
	if(arg_tool_check_plane_xy(xm,ym,195,85,64,28))
	{
		user_pressed=10;
		gui_is_dirty=1;
	}

	// edit step switch
	if(arg_tool_check_plane_xy(xm,ym,195,133,32,16))
	{
		user_pressed=11;
		gui_is_dirty=1;
	}

	// edit overwrite switch
	if(arg_tool_check_plane_xy(xm,ym,227,133,32,16))
	{
		user_pressed=12;
		gui_is_dirty=1;
	}

	// edit page switch
	if(arg_tool_check_plane_xy(xm,ym,195,149,64,16))
	{
		user_pressed=13;
		gui_is_dirty=1;
	}

	// edit quantization menu
	if(arg_tool_check_plane_xy(xm,ym,3,165,128,16))
	{
		CheckMenuRadioItem(hmenu_quantize,3,30,user_quantize+3,MF_BYPOSITION);
		arg_menu_track(hmenu_quantize,hwnd,67,181);
	}

	// edit midi channel menu
	if(arg_tool_check_plane_xy(xm,ym,3,181,128,16))
	{
		CheckMenuRadioItem(hmenu_midi_ch,3,18,user_midi_ch+3,MF_BYPOSITION);
		arg_menu_track(hmenu_midi_ch,hwnd,67,197);
	}

	// edit kbd note offset menu
	if(arg_tool_check_plane_xy(xm,ym,3,197,128,16))
	{
		CheckMenuRadioItem(hmenu_kbd_octave,3,11,(user_kbd_note_offset/12)+3,MF_BYPOSITION);
		arg_menu_track(hmenu_kbd_octave,hwnd,67,213);
	}

	// edit midi mask menu
	if(arg_tool_check_plane_xy(xm,ym,3,213,128,16))
	{
		// check menu items according midi mask
		for(int mmi=0;mmi<8;mmi++)
		{
			if(user_midi_mask&(1<<mmi))
				CheckMenuItem(hmenu_midi_mask,mmi,MF_BYPOSITION | MF_CHECKED);
			else
				CheckMenuItem(hmenu_midi_mask,mmi,MF_BYPOSITION | MF_UNCHECKED);
		}

		// track midi mask flag menu
		arg_menu_track(hmenu_midi_mask,hwnd,67,229);
	}

	// edit pattern sequencer zoom
	if(arg_tool_check_plane_xy(xm,ym,131,165,128,16))
	{
		user_pressed=14;

		// default zooming
		if(is_double_click)
			pp->usr_ppp=arg_tool_clipped_assign(project.master_ppqn/64,1,MAX_SIGNED_INT);

		// post refresh
		gui_is_dirty=1;
	}

	// edit sequencer mode
	if(arg_tool_check_plane_xy(xm,ym,131,181,128,16))
	{
		user_pressed=15;
		gui_is_dirty=1;
	}

	// edit default note velocity
	if(arg_tool_check_plane_xy(xm,ym,131,197,128,16))
	{
		user_pressed=16;

		// default velocity if double click
		if(is_double_click)
			user_kbd_velo=100;

		gui_is_dirty=1;
	}

	// edit default note pre-release
	if(arg_tool_check_plane_xy(xm,ym,131,213,128,16))
	{
		user_pressed=17;

		// default pre-release if double click
		if(is_double_click)
			pp->usr_pre=project.master_ppqn/16;

		gui_is_dirty=1;
	}

	// edit current vst-instance menu
	if(arg_tool_check_plane_xy(xm,ym,265,197,64,16) || arg_tool_check_plane_xy(xm,ym,505,197,16,16))
	{
		// get instance
		ADX_INSTANCE* pi=&instance[user_instance];

		// if instance is allocated, popup instance options menu, else show vst library menu
		if(pi->peffect!=NULL)
			arg_menu_track(hmenu_instance,hwnd,329,213);
		else
			arg_menu_track(hmenu_vst_lib,hwnd,329,213);

		// post refresh
		gui_is_dirty=1;
	}

	// edit change current vst-instance alias label
	if(arg_tool_check_plane_xy(xm,ym,349,197,124,16))
	{
		// get instance
		ADX_INSTANCE* pi=&instance[user_instance];

		// change label if instance is allocated
		if(pi->peffect!=NULL)
			gui_create_edit(hwnd,349,197,124,16,pi->alias,515);
	}

	// edit current vst-instance index spinner
	if(arg_tool_check_plane_xy(xm,ym,473,197,32,16))
	{
		// get spin index
		int const spin_index=(xm>=489);

		// set new index
		user_instance=arg_tool_clipped_assign(user_instance+(spin_index*2)-1,0,MAX_INSTANCES-1);
		user_pressed=18+spin_index;

		// scroll to instance
		gui_scroll_to_instance();

		// post refresh
		gui_is_dirty=1;
	}

	// edit current vst-instance program list menu
	if(arg_tool_check_plane_xy(xm,ym,265,213,64,16) || arg_tool_check_plane_xy(xm,ym,505,213,16,16))
	{
		// get instance
		ADX_INSTANCE* pi=&instance[user_instance];

		// check instance and number of programs
		if(pi->peffect!=NULL && pi->peffect->numPrograms>1)
		{
			// destroy previous menu
			DestroyMenu(hmenu_program);

			// create new menu
			hmenu_program=CreatePopupMenu();

			// scan program names
			for(int p=0;p<pi->peffect->numPrograms;p++)
			{
				char buf_prg[80];
				sprintf(buf_prg,"%.2X:\t",p);
				pi->peffect->dispatcher(pi->peffect,effGetProgramNameIndexed,p,0,buf_prg+4,0.0f);
				arg_menu_add_item(hmenu_program,buf_prg,16474+p);

				// modify separator flags
				if(p>0 && (p&0x1F)==0)
					ModifyMenu(hmenu_program,p,MF_BYPOSITION+MF_MENUBARBREAK,16474+p,buf_prg);
			}

			// check current selected program
			int const cp=pi->peffect->dispatcher(pi->peffect,effGetProgram,0,0,NULL,0.0f);
			CheckMenuRadioItem(hmenu_program,0,(pi->peffect->numPrograms-1),cp,MF_BYPOSITION);

			// show program menu
			arg_menu_track(hmenu_program,hwnd,329,229);
		}
	}

	// edit change current vst-instance program label
	if(arg_tool_check_plane_xy(xm,ym,349,213,124,16))
	{
		// get instance
		ADX_INSTANCE* pi=&instance[user_instance];

		// check if instance is loaded
		if(pi->peffect!=NULL)
		{
			char buf_prg[_MAX_PATH];
			buf_prg[0]=0;
			pi->peffect->dispatcher(pi->peffect,effGetProgramName,0,0,buf_prg,0.0f);
			gui_create_edit(hwnd,349,213,124,16,buf_prg,1);
		}
	}

	// edit current instance program index spinner
	if(arg_tool_check_plane_xy(xm,ym,473,213,32,16))
	{
		// get instance
		ADX_INSTANCE* pi=&instance[user_instance];

		// get spin index
		int const spin_index=(xm>=489);

		// check if instance if loaded and if have more than 1 program
		if(pi->peffect!=NULL && pi->peffect->numPrograms>1)
		{
			// get new program index
			int const new_prg_index=arg_tool_clipped_assign(pi->peffect->dispatcher(pi->peffect,effGetProgram,0,0,NULL,0.0f)+(spin_index*2)-1,0,pi->peffect->numPrograms-1);

			// set new program
			pi->peffect->dispatcher(pi->peffect,effSetProgram,0,new_prg_index,NULL,0.0f);

			// update vst editor window caption
			host_update_window_caption(pi->peffect);
		}

		// press button and refresh
		user_pressed=20+spin_index;

		// post refresh
		gui_is_dirty=1;
	}

	// vst instance list slider
	if(arg_tool_check_plane_xy(xm,ym,265,17,16,176))
	{
		user_pressed=22;

		// post refresh
		gui_is_dirty=1;
	}

	// vst instance list select
	if(arg_tool_check_plane_xy(xm,ym,281,17,208,176))
	{
		// set new instance index
		user_instance=user_instance_list_offset+(ym-17)/16;
		user_parameter_list_offset=0;
		user_parameter=0;

		// get new instance index
		ADX_INSTANCE* pi=&instance[user_instance];

		// bring plugin window to top if editor is already opened
		if(pi->peffect!=NULL && pi->hwnd && IsWindow(pi->hwnd))
			SetWindowPos(pi->hwnd,HWND_TOP,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);

		// center instance in routing window if ctrl+clicked
		if(pi->peffect!=NULL && GetKeyState(VK_CONTROL)<0)
		{
			this->user_rout_offset_x = pi->x-(w/2)+64;
			this->user_rout_offset_y = pi->y-(rout_area_h/2)+24;
		}

		// double click
		if(is_double_click)
		{
			// open vst editor if plugin is instanced, else show vst library menu
			if(pi->peffect!=NULL)
				instance_open_editor(pi,hwnd);
			else
				arg_menu_track(hmenu_vst_lib,hwnd,301,33+(user_instance-user_instance_list_offset)*16);
		}

		// post refresh
		gui_is_dirty=1;
	}

	// vst instance list mute
	if(arg_tool_check_plane_xy(xm,ym,489,17,16,176))
	{
		// get instance
		ADX_INSTANCE* pi=&instance[user_instance_list_offset+(ym-17)/16];

		// instance mute
		pi->process_mute=!pi->process_mute;

		// post refresh
		gui_is_dirty=1;
	}

	// vst instance list thru (bypass)
	if(arg_tool_check_plane_xy(xm,ym,505,17,16,176))
	{
		// get instance
		ADX_INSTANCE* pi=&instance[user_instance_list_offset+(ym-17)/16];

		// instance thru (bypass)
		pi->process_thru=!pi->process_thru;

		// post refresh
		gui_is_dirty=1;
	}

	// vst parameter list offset drag
	if(arg_tool_check_plane_xy(xm,ym,527,17,16,176))
	{
		user_pressed=23;

		// post refresh
		gui_is_dirty=1;
	}

	// vst parameter list tweak
	if(arg_tool_check_plane_xy(xm,ym,543,17,240,176))
	{
		// get instance
		ADX_INSTANCE* pi=&instance[user_instance];

		// get temporal selected parameter
		int const temp_parameter=user_parameter_list_offset+(ym-17)/16;

		if(pi->peffect && temp_parameter<pi->peffect->numParams)
		{
			// disable midi learn
			user_midi_learn=0;

			// set new index and retrieve current vst parameter value
			user_parameter=temp_parameter;
			user_parameter_val=pi->peffect->getParameter(pi->peffect,temp_parameter);

			// process learn
			if(is_double_click)
				user_midi_learn=1;

			// set parameter tweaking flag
			user_pressed=24;

			// post refresh
			gui_is_dirty=1;
		}
	}

	// project properties change name label
	if(arg_tool_check_plane_xy(xm,ym,853,197,192,16))
		gui_create_edit(hwnd,853,197,168,16,project.name,513);

	// project properties changue info label
	if(arg_tool_check_plane_xy(xm,ym,853,213,168,16))
		gui_create_edit(hwnd,853,213,168,16,project.info,514);

	/*
	// irc line list slider change
	if(arg_tool_check_plane_xy(xm,ym,1131,17,16,240))
	{
	user_pressed=42;
	gui_refresh(hwnd);
	}

	// irc nick list slider change
	if(arg_tool_check_plane_xy(xm,ym,1051,17,16,240))
	{
	user_pressed=43;
	gui_refresh(hwnd);
	}
	*/

	// sequencer page
	if(user_page==0)
	{
		// pattern list
		if(arg_tool_check_plane_xy(xm,ym,0,seq_area_y-32,TRACK_WIDTH-48,16) || arg_tool_check_plane_xy(xm,ym,TRACK_WIDTH-16,seq_area_y-32,16,16))
		{
			// destroy previous pattern menu
			DestroyMenu(hmenu_patterns);

			// create new pattern menu
			hmenu_patterns=CreatePopupMenu();

			// pattern string buffer
			char buf_pat[80];

			// scan pattern names
			for(int p=0;p<MAX_PATTERNS;p++)
			{
				sprintf(buf_pat,"%.2X:\t%s",p,project.pattern[p].name);
				arg_menu_add_item(hmenu_patterns,buf_pat,17498+p);

				// modify separator flags
				if(p>0 && (p&0x1F)==0)
					ModifyMenu(hmenu_patterns,p,MF_BYPOSITION+MF_MENUBARBREAK,17498+p,buf_pat);
			}

			// check current selected pattern
			CheckMenuRadioItem(hmenu_patterns,0,MAX_PATTERNS-1,user_pat,MF_BYPOSITION);

			// show pattern menu
			arg_menu_track(hmenu_patterns,hwnd,28,seq_area_y-16);
		}

		// pattern spinner
		if(arg_tool_check_plane_xy(xm,ym,TRACK_WIDTH-48,seq_area_y-32,32,16))
		{
			// get spin index
			int const spin_index=(xm>=(TRACK_WIDTH-32));

			// set new pattern index
			user_pat=arg_tool_clipped_assign(user_pat+(spin_index*2)-1,0,MAX_PATTERNS-1);

			// set spin pressed button flag index
			user_pressed=26+spin_index;

			// post refresh	
			gui_is_dirty=1;
		}

		// pattern cue-start marker drag
		if(arg_tool_check_plane_xy(xm,ym,0,y_cue_sta-16,40,17))
		{
			user_drag_offset=(y_cue_sta-ym)*pp->usr_ppp;
			user_pressed=28;
			gui_is_dirty=1;
			return;
		}

		// pattern cue-end marker drag
		if(arg_tool_check_plane_xy(xm,ym,0,y_cue_end,40,17))
		{
			user_drag_offset=(y_cue_end-ym)*pp->usr_ppp;
			user_pressed=29;
			gui_is_dirty=1;
			return;
		}

		// left-side (position bar) click
		if(arg_tool_check_plane_xy(xm,ym,0,seq_area_y,40,seq_area_h))
		{
			// scan for stop
			int const sy=seq_cent_y+(pp->cue_stp-pp->usr_pos)/pp->usr_ppp;

			// check if stop market clicked
			if(pp->cue_stp>0 && arg_tool_check_plane_xy(xm,ym,0,sy-16,40,17))
			{
				if(is_double_click)
				{
					pp->cue_stp=0;
					gui_is_dirty=1;
					return;
				}
				else
				{
					user_drag_offset=(sy-ym)*pp->usr_ppp;
					user_pressed=6;
					gui_is_dirty=1;
					return;
				}
			}

			// scan for marker draggin
			for(int m=(MAX_MARKERS-1);m>=0;m--)
			{
				// get marker pointer
				ADX_MARKER* pm=&pp->marker[m];

				// check if flag is set
				if(pm->flg)
				{
					// get marker position
					int const my=seq_cent_y+(pp->marker[m].pos-pp->usr_pos)/pp->usr_ppp;

					// check if clicked
					if(arg_tool_check_plane_xy(xm,ym,0,my-16,40,17))
					{
						// disable marker if double click
						if(is_double_click)
						{
							// set flag off
							pm->flg=0;

							// post refresh
							gui_is_dirty=1;
							return;
						}

						// set drag marker mode
						user_marker_drag=m;
						user_drag_offset=(my-ym)*pp->usr_ppp;
						user_pressed=30;

						// post refresh
						gui_is_dirty=1;
						return;
					}
				}
			}

			// add new pattern marker or set stop position
			if(is_double_click)
			{
				// set current pattern stop marker at cursor position
				if(GetKeyState(VK_CONTROL)<0)
				{
					pp->cue_stp=ym_seq_pos;
					gui_is_dirty=1;
					return;
				}
				else
				{
					// add new pattern marker
					edit_add_new_marker(pp,ym_seq_pos);
					gui_is_dirty=1;
					return;
				}
			}
		}

		// sequencer piano roll separator check
		if(arg_tool_check_plane_xy(xm,ym,TRACK_WIDTH+user_pr_width-4,seq_area_y-32,4,32+seq_area_h))
		{
			// set size cursor
			SetCursor(hcursor_szwe);

			// set pr separator drag action
			user_pressed=5;
			user_drag_offset=xm-(user_pr_width-3);
			gui_is_dirty=1;
			return;
		}

		// sequencer position drag
		if(arg_tool_check_plane_xy(xm,ym,40,seq_area_y,56,seq_area_h))
		{
			// set hand closed cursor
			SetCursor(hcursor_hacl);

			// set position drag action
			user_pressed=31;
			gui_is_dirty=1;
		}

		// mouse click in piano-roll view
		if(arg_tool_check_plane_xy(xm,ym,TRACK_WIDTH+4,seq_area_y,user_pr_width-8,seq_area_h))
		{
			// get piano roll screen port
			int const pr_vn=(user_pr_width-8)/user_pr_note_width;
			int const pr_no=user_kbd_note_offset-(pr_vn/2)+12;

			// scan all sequencer events (from top to bottom)
			for(int e=(seq_num_events-1);e>=0;e--)
			{
				// get sequencer event pointer
				ADX_EVENT* pe=&seq_event[e];

				// check event parameters
				if(pe->pat==user_pat && pe->trk==user_trk && pe->typ==0)
				{
					// get event horizontal screen coordinates
					int const e_x=TRACK_WIDTH+4+(int(pe->da2)-pr_no)*user_pr_note_width;
					int const e_r=e_x+user_pr_note_width;

					// check event horizontal screen coordinates
					if(xm>=e_x && xm<e_r)
					{
						// get event vertical screen coordinates
						int const e_y=seq_cent_y+(pe->pos-pp->usr_pos)/pp->usr_ppp;
						int const e_b=e_y+gui_get_event_height(pe);

						// check event vertical screen coordinates
						if(ym>=e_y && ym<e_b)
						{
							// update undo
							edit_undo_snapshot();

							// delete event
							if(is_double_click)
							{
								seq_delete_event(e);
								gui_is_dirty=1;
								return;
							}

							// send note-on from event
							instance_add_midi_event(&instance[pe->da0],pe->trk,0x90+(pe->da1&0xF),pe->da2,pe->da3,0,0);

							// resize event
							if(pe->szd && ym>=(e_b-4))
							{
								user_event_drag=e;
								user_drag_offset=(e_b-ym)*pp->usr_ppp;
								user_pressed=33;
								return;
							}

							// relocate event
							user_event_drag=e;
							user_drag_offset=(e_y-ym)*pp->usr_ppp;
							user_pressed=40;
							return;
						}
					}
				}
			}

			// piano roll enter new note
			if(is_double_click)
			{
				// get quantization
				int const i_quantize=edit_get_quantization();

				// get new pos and note
				int const ne_pos=(ym_seq_pos/i_quantize)*i_quantize;
				int const ne_not=arg_tool_clipped_assign(pr_no+(xm-(TRACK_WIDTH+4))/user_pr_note_width,0,127);

				// update undo (combining)
				edit_undo_snapshot(true);

				// add new event
				seq_add_event(ne_pos,user_pat,user_trk,0,user_instance,user_midi_ch,ne_not,user_kbd_velo,0);

				// get new sequencer event pointer
				ADX_EVENT* pe=&seq_event[seq_num_events-1];

				// set event duration
				pe->par=ym_seq_pos-ne_pos;

				// send note-on from event
				instance_add_midi_event(&instance[pe->da0],pe->trk,0x90+(pe->da1&0xF),pe->da2,pe->da3,0,0);

				// resize event
				user_drag_offset=0;
				user_event_drag=seq_num_events-1;
				user_pressed=33;

				// post refresh and return
				gui_is_dirty=1;
				return;
			}

			// piano roll mark block start
			user_block_pos_end=user_block_pos_sta=edit_quantize(ym_seq_pos);
			user_block_trk_sta=user_trk;
			user_block_trk_end=user_trk+1;

			// set block marking status and refresh
			user_pressed=41;
			gui_is_dirty=1;
		}

		// mouse click in tracker view
		if(arg_tool_check_plane_xy(xm,ym,TRACK_WIDTH+user_pr_width,seq_area_y,w-(TRACK_WIDTH+user_pr_width),seq_area_h))
		{
			// scan all sequencer events (from top to bottom)
			for(int e=(seq_num_events-1);e>=0;e--)
			{
				// get sequencer event pointer
				ADX_EVENT* pe=&seq_event[e];

				// check event parameters
				if(pe->pat==user_pat)
				{
					// get event horizontal screen coordinates
					int const e_x=TRACK_WIDTH+TRACK_WIDTH/2+user_pr_width+(pe->trk-user_trk_offset)*TRACK_WIDTH;
					int const e_r=e_x+TRACK_WIDTH/2;

					// check event horizontal screen coordinates
					if(xm>=e_x && xm<e_r)
					{
						// get event vertical screen coordinates
						int const e_y=seq_cent_y+(pe->pos-pp->usr_pos)/pp->usr_ppp;
						int const e_b=e_y+gui_get_event_height(pe);

						// check event vertical screen coordinates
						if(ym>=e_y && ym<e_b)
						{
							// update undo
							edit_undo_snapshot();

							// delete event
							if(is_double_click)
							{
								seq_delete_event(e);
								gui_is_dirty=1;
								return;
							}

							// send note-on from event
							if(pe->typ==0)
								instance_add_midi_event(&instance[pe->da0],pe->trk,0x90+(pe->da1&0xF),pe->da2,pe->da3,0,0);

							// resize event
							if(pe->szd && ym>=(e_b-4))
							{
								user_event_drag=e;
								user_drag_offset=(e_b-ym)*pp->usr_ppp;
								user_pressed=33;
								return;
							}

							// relocate event
							user_event_drag=e;
							user_drag_offset=(e_y-ym)*pp->usr_ppp;
							user_pressed=32;
							return;
						}
					}
				}
			}

			// check clicking mode
			if(is_double_click)
			{
				// mouse positioning on double click
				user_trk=user_trk_offset+(xm-(TRACK_WIDTH+user_pr_width))/TRACK_WIDTH;
				pp->usr_pos=edit_quantize(ym_seq_pos);
				user_row=0;

				// post refresh
				gui_is_dirty=1;
			}
			else
			{
				// mark block start
				user_block_pos_end=user_block_pos_sta=edit_quantize(ym_seq_pos);
				user_block_trk_end=user_block_trk_sta=arg_tool_clipped_assign(user_trk_offset+(xm-(TRACK_WIDTH+user_pr_width))/TRACK_WIDTH,0,MAX_TRACKS);

				// set block marking status and refresh
				user_pressed=34;
				gui_is_dirty=1;
			}
		}

		// change current pattern label
		if(arg_tool_check_plane_xy(xm,ym,0,seq_area_y-16,TRACK_WIDTH,16) && user_page==0)
		{
			gui_create_edit(hwnd,0,seq_area_y-16,TRACK_WIDTH,16,pp->name,512);
			gui_is_dirty=1;
		}

		// check track(s) actions (mute, solo, label)
		for(int t=0;t<MAX_TRACKS;t++)
		{
			// get track index
			int const ti=user_trk_offset+t;

			if(ti<MAX_TRACKS)
			{
				// get track pointer
				ADX_TRACK* pt=&pp->track[user_trk_offset+t];

				// track coord x
				int const trk_x=TRACK_WIDTH+user_pr_width+t*TRACK_WIDTH;

				// track mute
				if(arg_tool_check_plane_xy(xm,ym,trk_x,seq_area_y-32,16,16))
				{
					pt->mute=!pt->mute;
					gui_is_dirty=1;
				}

				// track solo
				if(arg_tool_check_plane_xy(xm,ym,trk_x+16,seq_area_y-32,16,16))
				{
					pt->solo=!pt->solo;
					gui_is_dirty=1;
				}

				// track label
				if(arg_tool_check_plane_xy(xm,ym,trk_x,seq_area_y-16,TRACK_WIDTH,16))
				{
					gui_create_edit(hwnd,trk_x,seq_area_y-16,TRACK_WIDTH,16,pt->name,2+ti);
					gui_is_dirty=1;
				}
			}
		}
	}

	// routing page
	if(user_page==1 && arg_tool_check_plane_xy(xm,ym,0,rout_area_y,w,rout_area_h))
	{
		// scan all instances in reverse order
		for(int i=(MAX_INSTANCES-1);i>=0;i--)
		{
			// get instance pointer
			ADX_INSTANCE* pi=&instance[i];

			// check if instance is loaded
			if(pi->peffect!=NULL)
			{
				// get num in and out pins (audio + midi)
				int const i_ni=pi->peffect->numInputs+1;
				int const i_no=pi->peffect->numOutputs+1;

				// get instance module screen coordinates and box width
				int const i_x=pi->x-user_rout_offset_x;
				int const i_y=rout_area_y+pi->y-user_rout_offset_y;
				int const i_w=arg_tool_clipped_assign(max(i_ni,i_no)*8,128,MAX_SIGNED_INT);

				// instance mute
				if(arg_tool_check_plane_xy(xm,ym,i_x,i_y+8,16,16))
				{
					pi->process_mute=!pi->process_mute;
					gui_is_dirty=1;
					return;
				}

				// instance thru (bypass)
				if(arg_tool_check_plane_xy(xm,ym,i_x+16,i_y+8,16,16))
				{
					pi->process_thru=!pi->process_thru;
					gui_is_dirty=1;
					return;
				}

				// drag instance wire
				if(arg_tool_check_plane_xy(xm,ym,i_x,i_y+40,pi->peffect->numOutputs*8,8))
				{
					// set pin index
					user_dragging_rout_pin_index=(xm-i_x)/8;
					user_dragging_rout_instance_index=i;
					user_pressed=36;
					gui_is_dirty=1;
					return;
				}

				// drag instance midi-out link
				if(arg_tool_check_plane_xy(xm,ym,i_x+pi->peffect->numOutputs*8,i_y+40,8,8))
				{
					user_dragging_rout_instance_index=i;
					user_dragging_rout_pin_index=0;
					user_pressed=25;
					gui_is_dirty=1;
					return;
				}

				// select instance box
				if(arg_tool_check_plane_xy(xm,ym,i_x,i_y,i_w,48))
				{
					// update selected instance
					user_instance=i;

					// scroll to instance
					gui_scroll_to_instance();

					// select first parameter
					user_parameter_list_offset=0;
					user_parameter=0;

					// open editor if double click case
					if(is_double_click)
						instance_open_editor(pi,hwnd);

					// bring plugin window to top if opened
					if(pi->hwnd!=NULL && IsWindow(pi->hwnd))
						SetWindowPos(pi->hwnd,HWND_TOP,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);

					// update new last mouse coord registers
					user_lxm=xm;
					user_lym=ym;

					// set instance box draggin mode, refresh and return
					user_pressed=35;
					gui_is_dirty=1;
					return;
				}

				// scan instance audio output pins
				gui_mouse_scan_pin(hwnd,xm,ym,pi->pout_pin,pi->peffect->numOutputs,i_x,i_y+40,is_double_click,0);

				// scan instance midi output pins
				gui_mouse_scan_pin(hwnd,xm,ym,&pi->mout_pin,1,i_x+pi->peffect->numOutputs*8,i_y+40,is_double_click,1);

				// return if pin scan success
				if(user_pressed==44 || user_pressed==45)
				{
					gui_is_dirty=1;
					return;
				}
			}
		}

		// get master input module screen coordinates
		int const m_i_x=master_i_x-user_rout_offset_x;
		int const m_i_y=rout_area_y+master_i_y-user_rout_offset_y;

		// change master input module position
		if(arg_tool_check_plane_xy(xm,ym,m_i_x,m_i_y,NUM_DSP_INPUTS*8,16))
		{
			user_pressed=37;
			gui_is_dirty=1;
			return;
		}

		// change master input pin assignment
		if(arg_tool_check_plane_xy(xm,ym,m_i_x,m_i_y+16,NUM_DSP_INPUTS*8,8))
		{
			// set input index and post refresh
			user_input_pin=(xm-m_i_x)/8;
			gui_is_dirty=1;

			// destroy and create menu
			DestroyMenu(hmenu_asio_input);
			hmenu_asio_input=CreatePopupMenu();

			// show asio input assignment menu
			asio_fill_pin_menu(hmenu_asio_input,1,17754);
			arg_menu_track(hmenu_asio_input,hwnd,m_i_x+user_input_pin*8,m_i_y+24);

			// post refresh and return
			gui_is_dirty=1;
			return;
		}

		// drag master input pin wire
		if(arg_tool_check_plane_xy(xm,ym,m_i_x,m_i_y+24,NUM_DSP_INPUTS*8,8))
		{
			// set pin index
			user_dragging_rout_pin_index=(xm-m_i_x)/8;
			user_dragging_rout_instance_index=INPUT_INSTANCE;
			user_pressed=36;
			gui_is_dirty=1;
			return;
		}

		// scan master-input pins wire set
		gui_mouse_scan_pin(hwnd,xm,ym,master_input_pin,NUM_DSP_INPUTS,m_i_x,m_i_y+24,is_double_click,0);

		// return if pin scan success
		if(user_pressed==44 || user_pressed==45)
		{
			gui_is_dirty=1;
			return;
		}

		// get master output module screen coordinates
		int const m_o_x=master_o_x-user_rout_offset_x;
		int const m_o_y=rout_area_y+master_o_y-user_rout_offset_y;

		// change master output module position
		if(arg_tool_check_plane_xy(xm,ym,m_o_x,m_o_y+8,NUM_DSP_INPUTS*8,16))
		{
			user_pressed=38;
			gui_is_dirty=1;
			return;
		}

		// change master output pin assignment
		if(arg_tool_check_plane_xy(xm,ym,m_o_x,m_o_y,NUM_DSP_OUTPUTS*8,8))
		{
			// set output index
			user_output_pin=(xm-m_o_x)/8;
			gui_is_dirty=1;

			// destroy and create menu
			DestroyMenu(hmenu_asio_output);
			hmenu_asio_output=CreatePopupMenu();

			// show asio output assignment menu
			asio_fill_pin_menu(hmenu_asio_output,0,17786);
			arg_menu_track(hmenu_asio_output,hwnd,m_o_x+user_output_pin*8,m_o_y+16);
			return;
		}

		// drop vst library menu on routing screen (insert plugin)
		if(is_double_click)
		{
			// get previous instance
			int const user_previous_instance=user_instance;

			// scan until free instance
			for(int fi=0;fi<MAX_INSTANCES;fi++)
			{
				// free instance found
				if(instance[fi].peffect==NULL)
				{
					// select new instance
					user_instance=fi;

					// show vst library menu in routing screen
					arg_menu_track(hmenu_vst_lib,hwnd,xm,ym);

					// post refresh and return
					gui_is_dirty=1;
					return;
				}
			}
		}
		else
		{
			// drag offset
			user_pressed=39;

			// post refresh
			gui_is_dirty=1;
		}
	}

	// update new last mouse coordinate registers
	user_lxm=xm;
	user_lym=ym;
}
