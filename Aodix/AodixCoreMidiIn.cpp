/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Midi Input Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK midi_callback(HMIDIIN hMidiIn,UINT wMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
	// extern aodix core pointer and handlers
	extern CAodixCore* gl_padx;

	// midi in data message
	if(wMsg==MIM_DATA)
	{
		// get midi message information
		unsigned char mi_data0=(dwParam1)&0x000000FF;
		unsigned char mi_data1=(dwParam1>>8)&0x000000FF;
		unsigned char mi_data2=(dwParam1>>16)&0x000000FF;

		// override midi-in channel
		if(gl_padx->cfg.midi_in_ch_rout)
			mi_data0=(mi_data0&0xF0)+gl_padx->user_midi_ch;

		// override midi-in velocity (only on note-on message with velo higher than 0)
		if(gl_padx->cfg.midi_in_vl_rout && (mi_data0&0xF0)==0x90 && mi_data2>0)
			mi_data2=gl_padx->user_kbd_velo;

		// call host midi-in process
		gl_padx->midi_in_process(mi_data0,mi_data1,mi_data2);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::midi_in_init(void)
{
	// set midi handler(s) pointer to null
	p_hmidi_in=NULL;

	// get midi input devices present in system
	int const num_midi_in_devices=midiInGetNumDevs();

	// check config
	if(cfg.midi_in_dv_open && num_midi_in_devices>0)
	{
		// create midi handlers
		p_hmidi_in=new HMIDIIN[num_midi_in_devices];

		// open all midi input devices
		for(int mid=0;mid<num_midi_in_devices;mid++)
		{
			// set midi in handler to null
			p_hmidi_in[mid]=NULL;

			// open and get return value
			HRESULT hresult=midiInOpen(&p_hmidi_in[mid],mid,(DWORD)midi_callback,0,CALLBACK_FUNCTION);

			// midi in success, start midi
			if(hresult==MMSYSERR_NOERROR)
				midiInStart(p_hmidi_in[mid]);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::midi_in_process(BYTE const data0,BYTE const data1,BYTE const data2)
{
	// get user instance pointer
	ADX_INSTANCE* pi=&instance[user_instance];

	// get user pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// check edit midi mask
	if((1<<((data0>>4)-8))&user_midi_mask)
	{
		// if instance is set, send midi event to vst
		if(pi->peffect!=NULL)
		{
			// enter critical section
			asio_enter_cs();

			// add event
			instance_add_midi_event(pi,user_trk,data0,data1,data2,0,0);

			// leave critical section
			asio_leave_cs();
		}

		// sequencer record instance midi events
		if(user_record)
		{
			// update undo (combining)
			edit_undo_snapshot(true);

			// write midi event with transport playing and live
			if((master_time_info.flags & kVstTransportPlaying) && cfg.rec_live)
			{
				// add midi event and refresh
				seq_add_evmid(seq_sample_to_pos(master_transport_sampleframe),user_pat,user_trk,user_instance,data0,data1,data2,user_edit_overwrite);
			}
			else
			{
				// write event with transport stopped / offline
				int const is_note_off_a=(data0&0xF0)==0x80;
				int const is_note_off_b=(data0&0xF0)==0x90 && data2==0;
				int const is_note_off_c=is_note_off_a || is_note_off_b;

				// check if note off, override with prerelease
				seq_add_evmid(pp->usr_pos-(pp->usr_pre*is_note_off_c),user_pat,user_trk,user_instance,data0,data1,data2,user_edit_overwrite);

				// step if note on event
				if(user_edit_step && ((data0&0xF0)==0x90 && data2>0))
					pp->usr_pos=arg_tool_clipped_assign(pp->usr_pos+edit_get_quantization(),0,MAX_SIGNED_INT);
			}
		}
	}

	// process learn when a midi cc is received)
	if(user_midi_learn && (data0&0xF0)==0xB0 && pi->peffect!=NULL && user_parameter<pi->peffect->numParams)
	{
		pi->pmidi_cc[user_parameter]=data1;
		user_midi_learn=0;
	}

	// process midi cc wrapper matrix
	if((data0&0xF0)==0xB0)
	{
		int const midi_cc=int(data1);
		float const f_midi_cc_value=float(data2)/127.0f;

		// scan all instances
		for(int i=0;i<MAX_INSTANCES;i++)
		{
			// get instance
			ADX_INSTANCE* pi=&instance[i];

			if(pi->peffect!=NULL)
			{
				// scan all parameters
				for(int p=0;p<pi->peffect->numParams;p++)
				{
					// check is controller assigned match controller input
					if(pi->pmidi_cc[p]==midi_cc)
					{
						// change vst parameter
						pi->peffect->setParameter(pi->peffect,p,f_midi_cc_value);

						// recording automation event
						if(master_time_info.flags & kVstAutomationWriting)
						{
							// midi wrapped automation event position
							int new_event_pos=pp->usr_pos;

							// write midi wrapped automation event (live else offline)
							if((master_time_info.flags & kVstTransportPlaying) && cfg.rec_live)
								new_event_pos=seq_sample_to_pos(master_transport_sampleframe);

							// update undo (combining)
							edit_undo_snapshot(true);

							// add automation event
							seq_add_event(new_event_pos,user_pat,user_trk,EVT_AUT,i,(p>>8)&0xFF,p&0xFF,(data2<<1),user_edit_overwrite);
						}
					}
				}
			}
		}
	}

	// monitor log info (ignore active sensing)
	if(data0<0xF0)
	{
		// update midi-in log
		sprintf(user_midi_in_monitor,"%.2X %.2X %.2X",data0,data1,data2);

		// post refresh
		gui_is_dirty=1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::midi_in_close(void)
{
	// check that midi input handler(s) pointer is valid
	if(p_hmidi_in!=NULL)
	{
		// get midi input devices
		int const num_midi_in_devices=midiInGetNumDevs();

		// close all midi input devices
		for(int mid=0;mid<num_midi_in_devices;mid++)
		{
			midiInStop(p_hmidi_in[mid]);
			midiInClose(p_hmidi_in[mid]);
		}

		// delete midi handlers
		delete[] p_hmidi_in;
	}

	// set null midi handler(s) pointer
	p_hmidi_in=NULL;
}