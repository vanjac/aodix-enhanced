/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Gui (Mouse Up) Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_mouse_up(HWND const hwnd)
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

	// get dsp routing area
	int const rout_area_y=232;
	int const rout_area_h=h-rout_area_y;

	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// master transport cycle switch button released
	if(user_pressed==PRESS_CYCLE)
		master_time_info.flags^=kVstTransportCycleActive;

	// master live-rec switch button released
	if(user_pressed==PRESS_LIVE)
		cfg.rec_live=!cfg.rec_live;

	// master stop-wrap switch button released
	if(user_pressed==PRESS_STOP_WRAP)
		cfg.stop_wrap=!cfg.stop_wrap;

	// master transport play button released
	if(user_pressed==PRESS_PLAY)
		dsp_transport_play();	

	// master transport stop button released
	if(user_pressed==PRESS_STOP)
		dsp_transport_stop();

	// master transport rec-events button released
	if(user_pressed==PRESS_REC)
		user_record=!user_record;

	// master transport rec-automation button released
	if(user_pressed==PRESS_AUTO)
		master_time_info.flags^=kVstAutomationWriting;

	// edit rec-step button released
	if(user_pressed==PRESS_STEP)
		user_edit_step=!user_edit_step;

	// edit overwrite button released
	if(user_pressed==PRESS_OVER)
		user_edit_overwrite=!user_edit_overwrite;

	// edit page button released
	if(user_pressed==PRESS_PAGE)
		user_page=!user_page;

	// edit mode button released
	if(user_pressed==PRESS_SEQ_MODE)
		pp->usr_mod=!pp->usr_mod;

	// routing midi-out link wire released
	if(user_pressed==PRESS_MIDI_WIRE)
	{
		// check if midi link wire is placed in any other instance midi link input
		for(int i=0;i<MAX_INSTANCES;i++)
		{
			// get destination instance
			ADX_INSTANCE* pi=&instance[i];

			// check destination instance is not wire source instance and destination effect is loaded
			if(i!=user_dragging_rout_instance_index && pi->peffect!=NULL)
			{
				// get instance module screen coordinates
				int const i_x=pi->x-user_rout_offset_x;
				int const i_y=rout_area_y+pi->y-user_rout_offset_y;

				// check mouse position and add the new wire
				if(arg_tool_check_plane_xy(xm,ym,i_x+pi->peffect->numInputs*8,i_y,8,8))
					edit_add_wire(&instance[user_dragging_rout_instance_index].mout_pin,i,0,1.0f);
			}
		}
	}

	//if control is not pressed
	if(GetKeyState(VK_CONTROL)>=0)
	{
		// sequencer current pattern cue-stop marker released, quantize it
		if(user_pressed==PRESS_CUE_STOP)
			pp->cue_stp=edit_quantize(pp->cue_stp);

		// sequencer current pattern cue-start marker released, quantize it
		if(user_pressed==PRESS_CUE_START)
			pp->cue_sta=edit_quantize(pp->cue_sta);

		// sequencer current pattern cue-end marker released, quantize it
		if(user_pressed==PRESS_CUE_END)
			pp->cue_end=edit_quantize(pp->cue_end);

		// sequencer current pattern marker released, quantize it
		if(user_pressed==PRESS_MARKER)
			pp->marker[user_marker_drag].pos=edit_quantize(pp->marker[user_marker_drag].pos);

		// sequencer edit position drag released, quantize user position if control is not pressed
		if(user_pressed==PRESS_SEQ_DRAG)
			pp->usr_pos=edit_quantize(pp->usr_pos);

		// sequencer edit event relocated, quantize event position if control and shift are unpressed
		if((user_pressed==PRESS_EVENT_MOVE || user_pressed==PRESS_PR_EVENT_MOVE) && GetKeyState(VK_SHIFT)>=0)
			seq_event[user_event_drag].pos=edit_quantize(seq_event[user_event_drag].pos);
	}

	// sequencer edit event finished, send note-off to instance if event was a note
	if(user_pressed==PRESS_EVENT_MOVE || user_pressed==PRESS_EVENT_SIZE || user_pressed==PRESS_PR_EVENT_MOVE)
	{
		// get dragging event
		ADX_EVENT* pe=&seq_event[user_event_drag];

		// send note off
		if(pe->typ==EVT_NOT)
			instance_add_midi_event(&instance[pe->da0],pe->trk,0x80+(pe->da1&0xF),pe->da2,0x40,0,0);
	}

	// sequencer block mark released, quantize user position
	if(user_pressed==PRESS_BLOCK_MARK)
		pp->usr_pos=edit_quantize(pp->usr_pos);

	// routing wire drag released
	if(user_pressed==PRESS_AUDIO_WIRE)
	{
		// draggin number of pin(s)
		int num_master_output_pins=0;

		// count highest master output pin
		for(int oo=0;oo<NUM_DSP_OUTPUTS;oo++)
		{
			// check master input module pin assignment
			if(cfg.asio_output_pin[oo]<asio_num_outputs)
				num_master_output_pins=oo;
		}

		// increment one
		++num_master_output_pins;

		// get master output module screen coordinates
		int const m_o_x=master_o_x-user_rout_offset_x;
		int const m_o_y=rout_area_y+master_o_y-user_rout_offset_y;

		// pointer to source pin
		ADX_PIN* ptr_src_pin=NULL;

		// draggin number of pin(s)
		int num_src_pins=0;

		// dragging instance wire
		if(user_dragging_rout_instance_index<MAX_INSTANCES)
		{
			ptr_src_pin=instance[user_dragging_rout_instance_index].pout_pin;
			num_src_pins=instance[user_dragging_rout_instance_index].peffect->numOutputs;
		}

		// dragging master input wire
		if(user_dragging_rout_instance_index==INPUT_INSTANCE)
		{
			ptr_src_pin=master_input_pin;
			num_src_pins=0;

			// count highest master input pin
			for(int ii=0;ii<NUM_DSP_INPUTS;ii++)
			{
				// check master input module pin assignment
				if(cfg.asio_input_pin[ii]<asio_num_inputs)
					num_src_pins=ii;
			}

			// increment one
			++num_src_pins;
		}

		// check if wire is placed to one master output
		if(arg_tool_check_plane_xy(xm,ym,m_o_x,m_o_y,NUM_DSP_OUTPUTS*8,8))
		{
			// get control key state
			if(GetKeyState(VK_CONTROL)<0 && num_master_output_pins>0)
			{
				// multiple attach
				for(int o=0;o<num_src_pins;o++)
					edit_add_wire(&ptr_src_pin[o],MASTER_INSTANCE,o%num_master_output_pins,1.0f);
			}
			else
			{
				// single attach
				edit_add_wire(&ptr_src_pin[user_dragging_rout_pin_index],MASTER_INSTANCE,(xm-m_o_x)/8,1.0f);
			}
		}

		// check if wire is placed in any other instance input
		for(int i=0;i<MAX_INSTANCES;i++)
		{
			// get destination instance
			ADX_INSTANCE* pi=&instance[i];

			// check destination instance is not wire source instance and destination effect is loaded
			if(i!=user_dragging_rout_instance_index && pi->peffect!=NULL && pi->peffect->numInputs>0)
			{
				// get instance module screen coordinates
				int const i_x=pi->x-user_rout_offset_x;
				int const i_y=rout_area_y+pi->y-user_rout_offset_y;

				// check coordinates
				if(arg_tool_check_plane_xy(xm,ym,i_x,i_y,pi->peffect->numInputs*8,8))
				{
					// get control key state
					if(GetKeyState(VK_CONTROL)<0)
					{
						// multiple connection attach
						for(int o=0;o<num_src_pins;o++)
							edit_add_wire(&ptr_src_pin[o],i,o%pi->peffect->numInputs,1.0f);
					}
					else
					{
						// single connection attach
						edit_add_wire(&ptr_src_pin[user_dragging_rout_pin_index],i,(xm-i_x)/8,1.0f);
					}
				}
			}
		}
	}

	// release gadget press and refresh
	if(user_pressed)
	{
		user_pressed=PRESS_NONE;
		gui_is_dirty=1;
	}

	// release mouse capture
	ReleaseCapture();
}
