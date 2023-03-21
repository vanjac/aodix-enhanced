/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Gui (Key Down) Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_key_down(HWND const hwnd,int const keycode,int const flags)
{
	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// shift pressed, control pressed
	if(GetKeyState(VK_SHIFT)<0 && GetKeyState(VK_CONTROL)<0)
	{
		// track solo toggle
		if(keycode=='M')
		{
			ADX_TRACK *pt = &pp->track[user_trk];
			pt->solo=!pt->solo;
			gui_is_dirty=1;
		}
	}

	// shift pressed, no control pressed
	if(GetKeyState(VK_SHIFT)<0 && GetKeyState(VK_CONTROL)>=0)
	{
		// sequencer go previous track
		if(keycode==VK_TAB)
		{
			// if first track, set first row
			if(user_trk>0)
				user_trk--;

			// set first row
			user_row=0;

			// post refresh
			gui_is_dirty=1;
		}
	}

	// shift no pressed, control pressed
	if(GetKeyState(VK_SHIFT)>=0 && GetKeyState(VK_CONTROL)<0)
	{
		// return to previous pattern
		if(keycode==VK_RETURN)
		{
			// set pattern
			user_pat=user_pat_prev;

			// post refresh
			gui_is_dirty=1;
		}

		// go prev track event
		if(keycode==VK_UP)
		{
			int user_old_pos=0;

			// scan sequencer events
			for(int e=0;e<seq_num_events;e++)
			{
				// get event pointer
				ADX_EVENT* pe=&seq_event[e];

				// check if current position has been raised by event position
				if(pe->pat==user_pat && pe->trk==user_trk && pe->pos>user_old_pos && pe->pos<pp->usr_pos)
					user_old_pos=pe->pos;
			}

			// update position and refresh gui
			pp->usr_pos=user_old_pos;

			// post refresh
			gui_is_dirty=1;
		}

		// go next track event
		if(keycode==VK_DOWN)
		{
			int user_old_pos=MAX_SIGNED_INT;

			// scan sequencer events
			for(int e=0;e<seq_num_events;e++)
			{
				// get event pointer
				ADX_EVENT* pe=&seq_event[e];

				// check if current position has been decreased by current event position
				if(pe->pat==user_pat && pe->trk==user_trk && pe->pos<user_old_pos && pe->pos>pp->usr_pos)
					user_old_pos=pe->pos;
			}

			// update user position
			if(user_old_pos!=MAX_SIGNED_INT)
				pp->usr_pos=user_old_pos;

			// post refresh
			gui_is_dirty=1;
		}

		// go prev 8 track(s) in sequencer
		if(keycode==VK_LEFT)
		{
			// set new track and row
			user_trk=arg_tool_clipped_assign(user_trk-8,0,MAX_TRACKS-1);
			user_row=0;

			// post refresh
			gui_is_dirty=1;
		}

		// go next 8 track(s) in sequencer
		if(keycode==VK_RIGHT)
		{
			// set new track and row
			user_trk=arg_tool_clipped_assign(user_trk+8,0,MAX_TRACKS-1);
			user_row=0;

			// post refresh
			gui_is_dirty=1;
		}

		// sequencer transpose block 1 semitone down
		if(keycode==VK_F1)
		{
			edit_transpose(-1,0);
			gui_is_dirty=1;
		}

		// sequencer transpose block 1 semitone up
		if(keycode==VK_F2)
		{
			edit_transpose(1,0);
			gui_is_dirty=1;
		}

		// prev quantization
		if(keycode=='1')
		{
			user_quantize=arg_tool_clipped_assign(user_quantize-1,0,27);
			gui_is_dirty=1;
		}

		// next quantization
		if(keycode=='2')
		{
			user_quantize=arg_tool_clipped_assign(user_quantize+1,0,27);
			gui_is_dirty=1;
		}

		// prev midi channel
		if(keycode=='3')
		{
			user_midi_ch=arg_tool_clipped_assign(user_midi_ch-1,0,15);
			gui_is_dirty=1;
		}

		// next midi channel
		if(keycode=='4')
		{
			user_midi_ch=arg_tool_clipped_assign(user_midi_ch+1,0,15);
			gui_is_dirty=1;
		}

		// prev user keyboard octave
		if(keycode=='5')
		{
			user_kbd_note_offset=arg_tool_clipped_assign(user_kbd_note_offset-12,0,96);
			gui_is_dirty=1;
		}

		// next user keyboard octave
		if(keycode=='6')
		{
			user_kbd_note_offset=arg_tool_clipped_assign(user_kbd_note_offset+12,0,96);
			gui_is_dirty=1;
		}

		// sequencer insert (all tracks)
		if(keycode==VK_INSERT)
		{
			edit_insert(1);
			gui_is_dirty=1;
		}

		// sequencer backspace (all tracks)			
		if(keycode==VK_BACK)
		{
			edit_back(1);
			gui_is_dirty=1;
		}

		// resize command
		if(keycode==VK_OEM_3) // `~ key
		{
			edit_resize(1);
			gui_is_dirty=1;
		}

		// file save(as) aodix project
		if(keycode=='S')
			gui_command(hwnd,ID_FILE_SAVE40025);

		// sequencer mark block start
		if(keycode=='B')
		{
			user_block_pos_sta=pp->usr_pos;
			user_block_trk_sta=user_trk;
			gui_is_dirty=1;
		}

		// sequencer mark block length
		if(keycode=='E')
		{
			user_block_pos_end=arg_tool_clipped_assign(pp->usr_pos,0,MAX_SIGNED_INT);
			user_block_trk_end=arg_tool_clipped_assign(user_trk+1,0,MAX_SIGNED_INT);
			gui_is_dirty=1;
		}

		// sequencer undo
		if(keycode=='Z')
			gui_command(hwnd,ID_EDIT_UNDOCLIPBOARDOPERATION);

		// sequencer cut to clipboard
		if(keycode=='X')
			gui_command(hwnd,ID_EDIT_CUT40012);

		// sequencer copy to clipboard
		if(keycode=='C')
			gui_command(hwnd,ID_EDIT_COPY40013);

		// sequencer paste from clipboard
		if(keycode=='V')
			gui_command(hwnd,ID_EDIT_PASTE40014);

		// sequencer edit select all
		if(keycode=='A')
			gui_command(hwnd,ID_EDIT_SELECTALL);
		
		// sequencer edit interpolate
		if(keycode=='I')
			gui_command(hwnd,ID_EDIT_INTERPOLATE);

		// sequencer edit quantize notes
		if(keycode=='Q')
			gui_command(hwnd,ID_EDIT_QUANTIZENOTES);

		// sequencer edit randomize
		if(keycode=='R')
			gui_command(hwnd,ID_EDIT_RANDOMIZE);

		// current pattern set start cue
		if(keycode==VK_HOME)
		{
			pp->cue_sta=pp->usr_pos;

			if(pp->cue_sta>pp->cue_end)
				pp->cue_end=pp->cue_sta;

			gui_is_dirty=1;
		}

		// current pattern set end cue
		if(keycode==VK_END)
		{
			pp->cue_end=pp->usr_pos;

			if(pp->cue_end<pp->cue_sta)
				pp->cue_sta=pp->cue_end;

			gui_is_dirty=1;
		}

		// sequencer go to 0 position and refresh
		if(keycode==VK_PRIOR)
		{
			pp->usr_pos=0;
			gui_is_dirty=1;
		}

		// sequencer go to last event
		if(keycode==VK_NEXT)
		{
			pp->usr_pos=0;

			// scan sequencer events
			for(int e=0;e<seq_num_events;e++)
			{
				// get event position
				ADX_EVENT* pe=&seq_event[e];

				// check if event position is higher than user position, and update
				if(pe->pos>pp->usr_pos && pe->pat==user_pat)
					pp->usr_pos=pe->pos;
			}

			// post refresh
			gui_is_dirty=1;
		}

		// track mute toggle
		if(keycode=='M')
		{
			ADX_TRACK *pt = &pp->track[user_trk];
			pt->mute=!pt->mute;
			gui_is_dirty=1;
		}
	}

	// no shift pressed and no control pressed
	if(GetKeyState(VK_SHIFT)>=0 && GetKeyState(VK_CONTROL)>=0)
	{
		// sequencer go next track
		if(keycode==VK_TAB)
		{
			if(user_trk<(MAX_TRACKS-1))
			{
				user_row=0;
				user_trk++;
				gui_is_dirty=1;
			}			
		}

		// sequencer go next pattern
		if(keycode==VK_ADD)
		{
			user_pat=arg_tool_clipped_assign(user_pat+1,0,MAX_PATTERNS-1);
			gui_is_dirty=1;
		}

		// sequencer go prev pattern
		if(keycode==VK_SUBTRACT)
		{
			user_pat=arg_tool_clipped_assign(user_pat-1,0,MAX_PATTERNS-1);
			gui_is_dirty=1;
		}

		// sequencer decrement user row
		if(keycode==VK_LEFT)
		{
			if(user_row==0)
			{
				if(user_trk>0)
				{
					user_row=8;
					user_trk--;
				}
			}
			else
			{
				user_row--;
			}

			gui_is_dirty=1;
		}

		// sequencer increment user row
		if(keycode==VK_RIGHT)
		{
			if(user_row==8)
			{
				if(user_trk<(MAX_TRACKS-1))
				{
					user_row=0;
					user_trk++;
				}
			}
			else
			{
				user_row++;
			}

			gui_is_dirty=1;
		}

		// sequencer decrement pattern user pos
		if(keycode==VK_UP)
		{
			pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos-edit_get_quantization(),0,MAX_SIGNED_INT);
			gui_is_dirty=1;
		}

		// sequencer increment pattern user pos
		if(keycode==VK_DOWN)
		{
			pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos+edit_get_quantization(),0,MAX_SIGNED_INT);
			gui_is_dirty=1;
		}

		// sequencer decrement pattern user pos (measures)
		if(keycode==VK_PRIOR)
		{
			pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos-(project.master_ppqn*project.master_numerator),0,MAX_SIGNED_INT);
			gui_is_dirty=1;
		}

		// sequencer increment pattern user pos (measures)
		if(keycode==VK_NEXT)
		{
			pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos+(project.master_ppqn*project.master_numerator),0,MAX_SIGNED_INT);
			gui_is_dirty=1;
		}

		// select next instance
		if(keycode==VK_MULTIPLY)
		{
			user_instance=arg_tool_clipped_assign(user_instance+1,0,MAX_INSTANCES-1);
			gui_scroll_to_instance();
			gui_is_dirty=1;
		}

		// select prev instance
		if(keycode==VK_DIVIDE)
		{
			user_instance=arg_tool_clipped_assign(user_instance-1,0,MAX_INSTANCES-1);
			gui_scroll_to_instance();
			gui_is_dirty=1;
		}

		// delete key (multifunction)
		if(keycode==VK_DELETE)
		{
			// abort and forget midi link
			if(user_midi_learn)
			{
				// get instance
				ADX_INSTANCE* pi=&instance[user_instance];

				// check instance vst parameter selected
				if(pi->peffect!=NULL && user_parameter<pi->peffect->numParams)
				{
					// forget midi cc (0xFF)
					pi->pmidi_cc[user_parameter]=0xFF;

					// learn off
					user_midi_learn=0;

					// post refresh
					gui_is_dirty=1;
				}

				// dont do any other action
				return;
			}

			// sequencer delete at position
			if(user_page==0)
			{
				// delete sequencer events at same position and track
				seq_delete_events_at(user_pat,pp->usr_pos,edit_get_quantization(),user_trk,1);

				// user position step
				if(user_edit_step)
					pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos+edit_get_quantization(),0,MAX_SIGNED_INT);

				// post refresh
				gui_is_dirty=1;
			}

			// dsp rout delete current instance
			if(user_page==1)	
			{
				gui_delete_current_instance(hwnd);
				gui_is_dirty=1;
			}
		}

		// sequencer insert (current track)
		if(keycode==VK_INSERT)
		{
			edit_insert(0);
			gui_is_dirty=1;
		}

		// sequencer backspace (current track)
		if(keycode==VK_BACK)
		{
			edit_back(0);
			gui_is_dirty=1;
		}

		// resize command
		if(keycode==VK_OEM_3) // `~ key
		{
			edit_resize(0);
			gui_is_dirty=1;
		}

		// master rec events switch
		if(keycode==VK_F1)
		{
			master_time_info.flags^=kVstTransportRecording;
			gui_is_dirty=1;
		}

		// master rec events switch
		if(keycode==VK_F2)
		{
			master_time_info.flags^=kVstAutomationWriting;
			gui_is_dirty=1;
		}

		// master transport cycle switch
		if(keycode==VK_F3)
		{
			master_time_info.flags^=kVstTransportCycleActive;
			gui_is_dirty=1;
		}

		// master switch live mode
		if(keycode==VK_F4)
		{
			cfg.rec_live=!cfg.rec_live;
			gui_is_dirty=1;
		}

		// master page stop wrap
		if(keycode==VK_F5)
		{
			cfg.stop_wrap=!cfg.stop_wrap;
			gui_is_dirty=1;
		}

		// edit step switch
		if(keycode==VK_F6)
		{
			user_edit_step=!user_edit_step;
			gui_is_dirty=1;
		}

		// edit overwrite switch
		if(keycode==VK_F7)
		{
			user_edit_overwrite=!user_edit_overwrite;
			gui_is_dirty=1;
		}

		// switch sequencer/dsp rout page
		if(keycode==VK_F8)
		{
			user_page=!user_page;
			gui_is_dirty=1;
		}

		if(keycode==VK_F9)
		{
			pp->usr_mod=!pp->usr_mod;
			gui_is_dirty=1;
		}

		// master transport switch
		if(keycode==VK_SPACE)
		{
			// switch play/stop
			if(master_time_info.flags & kVstTransportPlaying)
				dsp_transport_stop();
			else
				dsp_transport_play();

			// post refresh
			gui_is_dirty=1;
		}

		// set pattern cursor in pattern start marker
		if(keycode==VK_HOME)
		{
			pp->usr_pos=pp->cue_sta;
			gui_is_dirty=1;
		}

		// set pattern cursor in pattern end marker
		if(keycode==VK_END)
		{
			pp->usr_pos=pp->cue_end;
			gui_is_dirty=1;
		}

		// return key (multifunction)
		if(keycode==VK_RETURN)
		{
			// scan events looking for pattern events
			for(int e=0;e<seq_num_events;e++)
			{
				// get event pointer
				ADX_EVENT* pe=&seq_event[e];

				// check event data
				if(pe->typ==1 && pe->pat==user_pat && pe->pos==pp->usr_pos && pe->trk==user_trk)
				{
					// get called pattern pointer
					ADX_PATTERN* pp=&project.pattern[pe->da0];

					// get called marker pointer
					ADX_MARKER* pm=&pp->marker[pe->da1];

					// set cue loop
					pp->cue_sta=pp->usr_pos=pm->pos*pm->flg;
					pp->cue_end=pp->cue_sta+pe->par;

					// store previous pattern
					user_pat_prev = user_pat;

					// wrap
					user_pat=pe->da0;

					// post refresh and return
					gui_is_dirty=1;
					return;
				}
			}

			// sequencer enter program change midi event (note mode)
			if(master_time_info.flags & kVstTransportRecording)
			{
				// get current instance
				ADX_INSTANCE* pi=&instance[user_instance];

				// if effect instanced, add midi program change message event (with current instance program index else first program)
				if(pi->peffect!=NULL)
					seq_add_event(pp->usr_pos,user_pat,user_trk,3,user_instance,0xC0+user_midi_ch,pi->peffect->dispatcher(pi->peffect,effGetProgram,0,0,NULL,0.0f),0,user_edit_overwrite);
				else
					seq_add_event(pp->usr_pos,user_pat,user_trk,3,user_instance,0xC0+user_midi_ch,0,0,user_edit_overwrite);

				// user position step
				if(user_edit_step)
					pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos+edit_get_quantization(),0,MAX_SIGNED_INT);

				// post refresh
				gui_is_dirty=1;
			}
		}

		// get previous to message key state
		int const key_state=flags&(1<<30);

		// user entering decimal instance data
		if(user_row>=1)
		{
			// flag
			int pc_kbd_dig=-1;

			// select hex value
			switch(keycode)
			{
			case '0':	pc_kbd_dig=0;	break;
			case '1':	pc_kbd_dig=1;	break;
			case '2':	pc_kbd_dig=2;	break;
			case '3':	pc_kbd_dig=3;	break;
			case '4':	pc_kbd_dig=4;	break;
			case '5':	pc_kbd_dig=5;	break;
			case '6':	pc_kbd_dig=6;	break;
			case '7':	pc_kbd_dig=7;	break;
			case '8':	pc_kbd_dig=8;	break;
			case '9':	pc_kbd_dig=9;	break;
			case 'A':	pc_kbd_dig=10;	break;
			case 'B':	pc_kbd_dig=11;	break;
			case 'C':	pc_kbd_dig=12;	break;
			case 'D':	pc_kbd_dig=13;	break;
			case 'E':	pc_kbd_dig=14;	break;
			case 'F':	pc_kbd_dig=15;	break;
			}

			// entering hex data
			if(pc_kbd_dig>=0)
			{
				// scan sequencer events
				for(int e=0;e<seq_num_events;e++)
				{
					// get event pointer
					ADX_EVENT* pe=&seq_event[e];

					// check event location
					if(pe->pat==user_pat && pe->trk==user_trk && pe->pos==pp->usr_pos)
					{
						// change event data in user row
						switch(user_row)
						{
						case 1:	pe->da0=(pc_kbd_dig<<4)|(pe->da0&0xF);	break;
						case 2:	pe->da0=(pc_kbd_dig)|(pe->da0&0xF0);	break;
						case 3:	pe->da1=(pc_kbd_dig<<4)|(pe->da1&0xF);	break;
						case 4:	pe->da1=(pc_kbd_dig)|(pe->da1&0xF0);	break;
						case 5:	pe->da2=(pc_kbd_dig<<4)|(pe->da2&0xF);	break;
						case 6:	pe->da2=(pc_kbd_dig)|(pe->da2&0xF0);	break;
						case 7:	pe->da3=(pc_kbd_dig<<4)|(pe->da3&0xF);	break;
						case 8:	pe->da3=(pc_kbd_dig)|(pe->da3&0xF0);	break;
						}
					}
				}

				// user position step
				if(user_edit_step)
					pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos+edit_get_quantization(),0,MAX_SIGNED_INT);

				// post refresh
				gui_is_dirty=1;
			}
		}

		// pc midi keyboard process (only if key was unpressed)
		if(!key_state)
		{
			// user entering notes
			if(pp->usr_mod==0 && user_row==0)
			{
				// flag
				int pc_kbd_not=-1;

				// jump key
				if(keycode=='1')
					pc_kbd_not=-2;

				// pc keyboard iterate
				gui_key_pc_piano(keycode,&pc_kbd_not);

				// kbd enter midi program or jump event
				if((master_time_info.flags & kVstTransportRecording) && pc_kbd_not==-2 && user_row==0)
				{
					// new event flags
					int const t_live=(master_time_info.flags & kVstTransportPlaying) && cfg.rec_live;
					int const t_event_pos=(t_live*seq_sample_to_pos(master_transport_sampleframe))+(!t_live*pp->usr_pos);

					// write jump event
					if(pc_kbd_not==-2)
						seq_add_event(t_event_pos,user_pat,user_trk,2,0,0,0,0,user_edit_overwrite);

					// user position step if offline
					if(!t_live && user_edit_step)
						pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos+edit_get_quantization(),0,MAX_SIGNED_INT);

					// post refresh
					gui_is_dirty=1;
				}

				// pc keyboard note event
				if(pc_kbd_not>=0)
				{
					// get note-on index
					int const note_index=user_kbd_note_offset+pc_kbd_not;

					// get current instance
					ADX_INSTANCE* pi=&instance[user_instance];

					// enter critical section
					asio_enter_cs();

					// send midi note on
					instance_add_midi_event(&instance[user_instance],user_trk,0x90+user_midi_ch,note_index,user_kbd_velo,0,0);

					// leave critical section
					asio_leave_cs();

					// record note-on event to the sequencer
					if(master_time_info.flags & kVstTransportRecording)
					{
						// write midi note on event with transport playing and live
						if((master_time_info.flags & kVstTransportPlaying) && cfg.rec_live)
						{
							// add midi note on event and refresh
							seq_add_event(seq_sample_to_pos(master_transport_sampleframe),user_pat,user_trk,0,user_instance,user_midi_ch,note_index,user_kbd_velo,user_edit_overwrite);
						}
						else
						{
							// write midi note on event with transport stopped / offline
							seq_add_event(pp->usr_pos,user_pat,user_trk,0,user_instance,user_midi_ch,note_index,user_kbd_velo,user_edit_overwrite);

							// user position step
							if(user_edit_step)
								pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos+edit_get_quantization(),0,MAX_SIGNED_INT);
						}

						// post refresh
						gui_is_dirty=1;
					}
				}
			}

			// user entering patterns
			if(pp->usr_mod==1 && user_row==0)
			{
				// flag
				int pc_kbd_pat=-1;

				// select keydown code -> pattern index
				switch(keycode)
				{
				case '0':	pc_kbd_pat=0;	break;
				case '1':	pc_kbd_pat=1;	break;
				case '2':	pc_kbd_pat=2;	break;
				case '3':	pc_kbd_pat=3;	break;
				case '4':	pc_kbd_pat=4;	break;
				case '5':	pc_kbd_pat=5;	break;
				case '6':	pc_kbd_pat=6;	break;
				case '7':	pc_kbd_pat=7;	break;
				case '8':	pc_kbd_pat=8;	break;
				case '9':	pc_kbd_pat=9;	break;
				case 'A':	pc_kbd_pat=10;	break;
				case 'B':	pc_kbd_pat=11;	break;
				case 'C':	pc_kbd_pat=12;	break;
				case 'D':	pc_kbd_pat=13;	break;
				case 'E':	pc_kbd_pat=14;	break;
				case 'F':	pc_kbd_pat=15;	break;
				case 'G':	pc_kbd_pat=16;	break;
				case 'H':	pc_kbd_pat=17;	break;
				case 'I':	pc_kbd_pat=18;	break;
				case 'J':	pc_kbd_pat=19;	break;
				case 'K':	pc_kbd_pat=20;	break;
				case 'L':	pc_kbd_pat=21;	break;
				case 'M':	pc_kbd_pat=22;	break;
				case 'N':	pc_kbd_pat=23;	break;
				case 'O':	pc_kbd_pat=24;	break;
				case 'P':	pc_kbd_pat=25;	break;
				case 'Q':	pc_kbd_pat=26;	break;
				case 'R':	pc_kbd_pat=27;	break;
				case 'S':	pc_kbd_pat=28;	break;
				case 'T':	pc_kbd_pat=29;	break;
				case 'U':	pc_kbd_pat=30;	break;
				case 'V':	pc_kbd_pat=31;	break;
				case 'W':	pc_kbd_pat=32;	break;
				case 'X':	pc_kbd_pat=33;	break;
				case 'Y':	pc_kbd_pat=34;	break;
				case 'Z':	pc_kbd_pat=35;	break;
				}

				// record pattern event (always offline)
				if((master_time_info.flags & kVstTransportRecording) && pc_kbd_pat>=0)
				{
					// write pattern event
					if((master_time_info.flags & kVstTransportPlaying) && cfg.rec_live)
					{
						// write pattern event with live transport played (quantized)
						seq_add_event(seq_sample_to_pos(master_transport_sampleframe),user_pat,user_trk,1,pc_kbd_pat,0,0x80,0x80,user_edit_overwrite);
					}
					else
					{
						// write pattern event with transport stopped / offline
						seq_add_event(pp->usr_pos,user_pat,user_trk,1,pc_kbd_pat,0,0x80,0x80,user_edit_overwrite);

						// user position step
						if(user_edit_step)
							pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos+edit_get_quantization(),0,MAX_SIGNED_INT);
					}

					// post refresh
					gui_is_dirty=1;
				}
			}
		}
	}
}
