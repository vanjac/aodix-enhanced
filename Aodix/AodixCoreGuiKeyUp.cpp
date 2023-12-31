/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Gui (Key Up) Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_key_up(HWND const hwnd,int const keycode)
{
	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// user releasing key in note mode
	if(pp->usr_mod==0)
	{
		// pc midi keyboard flag
		int pc_kbd_not=-1;

		// jump key released
		if(keycode=='1')
			pc_kbd_not=-2;

		// pc keyboard released iterate
		gui_key_pc_piano(keycode,&pc_kbd_not);

		// send event if pc kbd flag is set
		if(pc_kbd_not!=-1)
		{
			// get note-off index
			int const note_index=user_kbd_note_offset+pc_kbd_not;

			// get current instance
			ADX_INSTANCE* pi=&instance[user_instance];

			// enter critical section
			asio_enter_cs();

			// send note off message
			instance_add_midi_event(&instance[user_instance],user_trk,0x80+user_midi_ch,note_index,64,0,0);

			// leave critical section
			asio_leave_cs();

			// rec note-off
			if(user_record && user_row==0)
			{
				// scan for unfinished sequencer note events
				if(pc_kbd_not>=0)
				{
					// scan events
					for(int e=0;e<seq_num_events;e++)
					{
						// get event pointer
						ADX_EVENT* pe=&seq_event[e];

						// check if note event is unfinished and match with released keyboard note
						if(pe->typ==EVT_NOT && pe->par==0 && pe->da2==note_index)
						{
							// finish note off event (live else offline)
							if((master_time_info.flags & kVstTransportPlaying) && cfg.rec_live)
								pe->par=arg_tool_clipped_assign(seq_sample_to_pos(master_transport_sampleframe)-pe->pos,1,MAX_SIGNED_INT);
							else
								pe->par=arg_tool_clipped_assign((pp->usr_pos-pe->pos)-pp->usr_pre,1,MAX_SIGNED_INT);
						}
					}
				}

				// scan for unfinished sequencer jump events
				if(pc_kbd_not==-2)
				{
					// scan events
					for(int e=0;e<seq_num_events;e++)
					{
						// get event pointer
						ADX_EVENT* pe=&seq_event[e];

						// check if jump event is unfinished
						if(pe->typ==EVT_JMP && pe->par==0)
						{
							// finish jump event (live else offline)
							if((master_time_info.flags & kVstTransportPlaying) && cfg.rec_live)
								pe->par=arg_tool_clipped_assign(seq_sample_to_pos(master_transport_sampleframe)-pe->pos,1,MAX_SIGNED_INT);
							else
								pe->par=arg_tool_clipped_assign(pp->usr_pos-pe->pos,1,MAX_SIGNED_INT);
						}
					}
				}
			}

			// post refresh
			gui_is_dirty=1;
		}
	}

	// user releasing key in pattern mode
	if(pp->usr_mod==1)
	{
		// flag
		int pc_kbd_pat=-1;

		// select keyup code -> pattern index
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

		// finish pattern events
		if(user_record && pc_kbd_pat>=0 && user_row==0)
		{
			// scan for unfinished sequencer pattern events
			for(int e=0;e<seq_num_events;e++)
			{
				// get event pointer
				ADX_EVENT* pe=&seq_event[e];

				// check if pattern event is unfinished
				if(pe->typ==EVT_PAT && pe->par==0 && pe->da0==pc_kbd_pat)
				{
					// finish pattern event (live else offline)
					if((master_time_info.flags & kVstTransportPlaying) && cfg.rec_live)
						pe->par=arg_tool_clipped_assign(seq_sample_to_pos(master_transport_sampleframe)-pe->pos,1,MAX_SIGNED_INT);
					else
						pe->par=arg_tool_clipped_assign(pp->usr_pos-pe->pos,1,MAX_SIGNED_INT);
				}
			}

			// post refresh
			gui_is_dirty=1;
		}
	}
}
