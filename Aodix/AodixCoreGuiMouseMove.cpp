/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Gui (Mouse Move) Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_mouse_move(HWND const hwnd)
{
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

	// get mouse drag delta
	int const i_mx_dif=xm-user_lxm;
	int const i_my_dif=ym-user_lym;

	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// get mouse position
	int const ym_seq_pos=pp->usr_pos+int(ym-seq_cent_y)*pp->usr_ppp;

	// master change / automate tempo
	if(user_pressed==PRESS_TEMPO)
	{
		// get tempo value
		double d_tempo=project.master_tempo;

		// tempo finetune with control
		if(GetKeyState(VK_CONTROL)<0)
			d_tempo-=double(i_my_dif)*0.1;
		else
			d_tempo-=double(i_my_dif);

		// minimun tempo clamp
		if(d_tempo<10.0)
			d_tempo=10.0;

		// maximun tempo clamp
		if(d_tempo>250.0)
			d_tempo=250.0;

		// get current sequencer transport playback position
		int const play_pos=seq_sample_to_pos(master_transport_sampleframe);

		// set new tempo
		project.master_tempo=d_tempo;

		// automation enabled and effect is not null
		if(master_time_info.flags & kVstAutomationWriting)
		{
			// get integral tempo
			int const int_tempo=int(d_tempo*256.0);

			// update undo (combining)
			edit_undo_snapshot(true);

			// write automation tempo (live else offline)
			if((master_time_info.flags & kVstTransportPlaying) && cfg.rec_live)
				seq_add_event(play_pos,user_pat,user_trk,EVT_TMP,(int_tempo>>8)&0xFF,(int_tempo&0xFF),0,0,user_edit_overwrite);
			else
				seq_add_event(pp->usr_pos,user_pat,user_trk,EVT_TMP,(int_tempo>>8)&0xFF,(int_tempo&0xFF),0,0,user_edit_overwrite);
		}

		// set sequencer position
		master_transport_sampleframe=seq_pos_to_sample(play_pos);

		// post refresh
		gui_is_dirty=1;
	}

	// sequencer drag pr separator
	if(user_pressed==PRESS_PR_SEP)
	{
		user_pr_width=arg_tool_clipped_assign(xm-user_drag_offset,104,768);
		gui_is_dirty=1;
	}

	// edit change sequencer zoom
	if(user_pressed==PRESS_SEQ_ZOOM)
	{
		pp->usr_ppp=arg_tool_clipped_assign(pp->usr_ppp-i_my_dif,1,project.master_ppqn/4);
		gui_is_dirty=1;
	}

	// edit change kbd velocity
	if(user_pressed==PRESS_VELOCITY)
	{
		user_kbd_velo=arg_tool_clipped_assign(user_kbd_velo-i_my_dif,1,127);
		gui_is_dirty=1;
	}

	// edit change note pre-release
	if(user_pressed==PRESS_PRE_RELEASE)
	{
		pp->usr_pre=arg_tool_clipped_assign(pp->usr_pre-i_my_dif,0,MAX_SIGNED_INT);
		gui_is_dirty=1;
	}

	// vst instance list slider change
	if(user_pressed==PRESS_INST_SCROLL)
	{
		user_instance_list_offset=arg_tool_clipped_assign(user_instance_list_offset+i_my_dif,0,245);
		gui_is_dirty=1;
	}

	// vst parameter list drag move
	if(user_pressed==PRESS_PARAM_SCROLL)
	{
		// get current instance pointer
		ADX_INSTANCE* pi=&instance[user_instance];

		// check if instance is allocated and num params exceed 16
		if(pi->peffect!=NULL && pi->peffect->numParams>16)
		{
			// set new list offset value and refresh
			user_parameter_list_offset=arg_tool_clipped_assign(user_parameter_list_offset+i_my_dif,0,pi->peffect->numParams-11);
			gui_is_dirty=1;
		}
	}

	// vst parameter list tweak
	if(user_pressed==PRESS_PARAM_TWEAK)
	{
		ADX_INSTANCE* pi=&instance[user_instance];

		// check if instance is allocated and user parameter is in valid range
		if(pi->peffect!=NULL && user_parameter>=0 && user_parameter<pi->peffect->numParams)
		{
			// finetune while control key is pressed
			if(GetKeyState(VK_CONTROL)<0)
				user_parameter_val-=float(i_my_dif)*0.001f;
			else
				user_parameter_val-=float(i_my_dif)*0.01f;

			// clip minimun
			if(user_parameter_val<0.0f)
				user_parameter_val=0.0f;

			// clip maximun
			if(user_parameter_val>1.0f)
				user_parameter_val=1.0f;

			// set new value
			pi->peffect->setParameter(pi->peffect,user_parameter,user_parameter_val);

			// send automation
			host_on_automation(pi->peffect,user_parameter,user_parameter_val);
		}
	}

	// sequencer drag current pattern cue start marker
	if(user_pressed==PRESS_CUE_START)
	{
		pp->cue_sta=arg_tool_clipped_assign(ym_seq_pos+user_drag_offset,0,pp->cue_end);
		gui_scroll_iterate(hwnd,ym,seq_area_y,h);
		gui_is_dirty=1;
	}

	// sequencer drag current pattern cue end marker
	if(user_pressed==PRESS_CUE_END)
	{
		pp->cue_end=arg_tool_clipped_assign(ym_seq_pos+user_drag_offset,pp->cue_sta,MAX_SIGNED_INT);
		gui_scroll_iterate(hwnd,ym,seq_area_y,h);
		gui_is_dirty=1;
	}

	// sequencer drag current pattern position marker
	if(user_pressed==PRESS_MARKER)
	{
		pp->marker[user_marker_drag].pos=arg_tool_clipped_assign(ym_seq_pos+user_drag_offset,0,MAX_SIGNED_INT);
		gui_scroll_iterate(hwnd,ym,seq_area_y,h);
		gui_is_dirty=1;
	}

	// sequencer drag current pattern stop marker
	if(user_pressed==PRESS_CUE_STOP)
	{
		pp->cue_stp=arg_tool_clipped_assign(ym_seq_pos+user_drag_offset,0,MAX_SIGNED_INT);
		gui_scroll_iterate(hwnd,ym,seq_area_y,h);
		gui_is_dirty=1;
	}

	// sequencer drag current position
	if(user_pressed==PRESS_SEQ_DRAG)
	{
		pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos-i_my_dif*pp->usr_ppp,0,MAX_SIGNED_INT);
		gui_is_dirty=1;
	}

	// sequencer relocate event
	if(user_pressed==PRESS_EVENT_MOVE || user_pressed==PRESS_PR_EVENT_MOVE)
	{
		// get sequencer event beign relocated
		ADX_EVENT* pe=&seq_event[user_event_drag];

		// set new event position
		if(!(GetKeyState(VK_SHIFT)<0))
			pe->pos=arg_tool_clipped_assign(ym_seq_pos+user_drag_offset,0,MAX_SIGNED_INT);

		// set event track or event
		if(user_pressed==PRESS_EVENT_MOVE)
			pe->trk=arg_tool_clipped_assign(user_trk_offset+(xm-(TRACK_WIDTH+user_pr_width))/TRACK_WIDTH,0,MAX_TRACKS-1);

		// transpose event note in piano roll (horizontal move)
		if(user_pressed==PRESS_PR_EVENT_MOVE)
		{
			// get piano roll screen port and new note
			int const pr_vn=(user_pr_width-8)/user_pr_note_width;
			int const pr_no=user_kbd_note_offset-(pr_vn/2)+12;
			int const pr_nn=arg_tool_clipped_assign(pr_no+(xm-(TRACK_WIDTH+4))/user_pr_note_width,0,127);

			// compare event note
			if(pr_nn!=pe->da2)
			{
				// note off / note on
				instance_add_midi_event(&instance[pe->da0],pe->trk,0x80+(pe->da1&0xF),pe->da2,0x40,0,0);
				instance_add_midi_event(&instance[pe->da0],pe->trk,0x90+(pe->da1&0xF),pr_nn,pe->da3,0,0);
			}

			// set new event note
			pe->da2=pr_nn;
		}

		// scroll iterate
		gui_scroll_iterate(hwnd,ym,seq_area_y,h);

		// post refresh
		gui_is_dirty=1;
	}

	// sequencer resize event
	if(user_pressed==PRESS_EVENT_SIZE)
	{
		// get sequencer event beign resized
		ADX_EVENT* pe=&seq_event[user_event_drag];

		// set new event duration
		int old_size=pe->par;
		pe->par=arg_tool_clipped_assign((ym_seq_pos+user_drag_offset)-pe->pos,0,MAX_SIGNED_INT);

		// resize all events in selection
		if(GetKeyState(VK_SHIFT)<0)
			edit_adjust_size(pe->par-old_size,user_event_drag);

		// scroll iterate
		gui_scroll_iterate(hwnd,ym,seq_area_y,h);

		// post refresh
		gui_is_dirty=1;
	}

	// sequencer mark block
	if(user_pressed==PRESS_BLOCK_MARK)
	{
		// mark block length
		user_block_pos_end=arg_tool_clipped_assign(ym_seq_pos,0,MAX_SIGNED_INT);
		user_block_trk_end=arg_tool_clipped_assign(((user_trk_offset+(xm-(TRACK_WIDTH+user_pr_width))/TRACK_WIDTH))+1,0,MAX_TRACKS);

		// scroll iterate
		gui_scroll_iterate(hwnd,ym,seq_area_y,h);

		// post refresh
		gui_is_dirty=1;
	}

	// sequencer view mouse cursor hoovering
	if(user_page==0 && user_pressed==PRESS_NONE)
	{
		// sequencer piano roll separator check
		if(arg_tool_check_plane_xy(xm,ym,TRACK_WIDTH+user_pr_width-4,seq_area_y-32,4,32+seq_area_h))
			SetCursor(hcursor_szwe);

		// sequencer time drag check
		if(arg_tool_check_plane_xy(xm,ym,40,seq_area_y,56,seq_area_h))
			SetCursor(hcursor_haop);

		// event detection in piano-roll view
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
				if(pe->pat==user_pat && pe->trk==user_trk && pe->typ==EVT_NOT)
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
							// update last coords and return
							user_lxm=xm;
							user_lym=ym;

							// event resizing cursor
							if(pe->szd && ym>=(e_b-4))
							{
								SetCursor(hcursor_szns);
								return;
							}

							// event re-locating cursor
							SetCursor(hcursor_move);
							return;
						}
					}
				}
			}
		}

		// event detection in tracker view
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
							// update last coords and return
							user_lxm=xm;
							user_lym=ym;

							// event resizing cursor
							if(pe->szd && ym>=(e_b-4))
							{
								SetCursor(hcursor_szns);
								return;
							}

							// event re-locating cursor
							SetCursor(hcursor_move);
							return;
						}
					}
				}
			}
		}
	}

	// routing instance module dragging
	if(user_pressed==PRESS_INST_MOVE)
	{
		ADX_INSTANCE* pi=&instance[user_instance];
		pi->x+=i_mx_dif;
		pi->y+=i_my_dif;
		gui_is_dirty=1;
	}

	// refresh while routing wire dragging or midi out link wire dragging
	if(user_pressed==PRESS_MIDI_WIRE || user_pressed==PRESS_AUDIO_WIRE)
		gui_is_dirty=1;

	// routing master input module dragging
	if(user_pressed==PRESS_MASTER_IN)
	{
		master_i_x+=i_mx_dif;
		master_i_y+=i_my_dif;
		gui_is_dirty=1;
	}

	// routing master output module dragging
	if(user_pressed==PRESS_MASTER_OUT)
	{
		master_o_x+=i_mx_dif;
		master_o_y+=i_my_dif;
		gui_is_dirty=1;
	}

	// routing view offset dragging
	if(user_pressed==PRESS_ROUTING_PAN)
	{
		user_rout_offset_x-=i_mx_dif;
		user_rout_offset_y-=i_my_dif;
		gui_is_dirty=1;
	}

	// block mark in piano-roll
	if(user_pressed==PRESS_PR_BLOCK_MARK)
	{
		// mark block length
		user_block_pos_end=arg_tool_clipped_assign(ym_seq_pos,0,MAX_SIGNED_INT);
		user_block_trk_end=user_trk+1;

		// scroll iterate
		gui_scroll_iterate(hwnd,ym,seq_area_y,h);

		// post refresh
		gui_is_dirty=1;
	}

	// tweak gain value
	if(user_pressed==PRESS_WIRE_GAIN)
	{
		// get wire value
		float wire_value=user_pressed_wire->value;

		// tweak wire value
		wire_value-=float(i_my_dif)*0.01f;

		// clip lowest gain
		if(wire_value<0.0f)
			wire_value=0.0f;

		// clip highest gain
		if(wire_value>1.0f)
			wire_value=1.0f;

		// set value
		user_pressed_wire->value=wire_value;

		// post refresh
		gui_is_dirty=1;
	}

	// update last coords
	user_lxm=xm;
	user_lym=ym;
}
