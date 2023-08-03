/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Host Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long VSTCALLBACK host_audiomaster(AEffect *effect, long opcode, long index, long value, void *ptr, float opt)
{
	// extern aodix core pointer
	extern CAodixCore* gl_padx;

	// instance wants host vst sdk implemented version
	if(opcode==audioMasterVersion)
		return 2300;

	// instance wants to automate a parameter
	if(opcode==audioMasterAutomate)
	{
		gl_padx->host_on_automation(effect,index,opt);
		return 0;
	}

	// instance wants id of the effect currently loading from project import
	if(opcode==audioMasterCurrentId)
		return gl_padx->instance_eff_id_currently_loading;

	// instance calls host idle procedure
	if(opcode==audioMasterIdle)
	{
		gl_padx->host_idle();
		return 0;
	}

	// instance looking for connected pin (!!! remember: here 0 is true!!!)
	if(opcode==audioMasterPinConnected)
		return 0;

	// instance wants midi
	if(opcode==audioMasterWantMidi)
		return 0;

	// instance wants timeinfo
	if(opcode==audioMasterGetTime)
		return long(&gl_padx->master_time_info);

	// instance wants host to process events
	if(opcode==audioMasterProcessEvents)
	{
		gl_padx->host_on_events(effect,(VstEvents*)ptr);
		return 0;
	}

	// instance wants to set host time
	if(opcode==audioMasterSetTime)
		return 0;

	// instance wants tempo at sample position
	if(opcode==audioMasterTempoAt)
		return long(gl_padx->master_time_info.tempo*10000.0);

	// instance wants number of host automatable parameters
	if(opcode==audioMasterGetNumAutomatableParameters)
	{
		if(effect)
			return effect->numParams;
		return 0;
	}

	// instance wants host parameter quantization
	if(opcode==audioMasterGetParameterQuantization)
		return 0;

	// instance notifies that its IO status changed (crap)
	if(opcode==audioMasterIOChanged)
		return 0;

	// instance notifies host that it needs idle calls
	if(opcode==audioMasterNeedIdle)
		return 0;

	// instance notifies change in editor's size
	if(opcode==audioMasterSizeWindow)
		return 0;

	// instance wants to know host sample rate
	if(opcode==audioMasterGetSampleRate)
	{
		// set dsp sample rate
		effect->dispatcher(effect,effSetSampleRate,0,0,NULL,(float)gl_padx->cfg.asio_driver_sample_rate);

		// return host sample rate
		return (long)gl_padx->cfg.asio_driver_sample_rate;
	}

	// instance wants to know host block size
	if(opcode==audioMasterGetBlockSize)
	{
		// set dsp blocksize
		effect->dispatcher(effect,effSetBlockSize,0,gl_padx->dsp_block_size,NULL,0.0f);

		// return host block size
		return gl_padx->dsp_block_size;
	}

	// instance wants host input latency (we return 0 cos we're faster than light)
	if(opcode==audioMasterGetInputLatency)
		return 0;

	// instance wants host output latency (we return 0 cos we're faster than light)
	if(opcode==audioMasterGetOutputLatency)
		return 0;

	// instance wants host previous effect
	if(opcode==audioMasterGetPreviousPlug)
		return 0;

	// instance wants host next effect
	if(opcode==audioMasterGetNextPlug)
		return 0;

	// instance wants the following
	if(opcode==audioMasterWillReplaceOrAccumulate)
		return 0;

	// instance wants host current process level
	if(opcode==audioMasterGetCurrentProcessLevel)
		return 0;

	// instance wants host automation state
	if(opcode==audioMasterGetAutomationState)
		return 0;

	// instance wants host vendor string in ptr
	if(opcode==audioMasterGetVendorString)
	{
		sprintf((char*)ptr,"Arguru Software");
		return 0;
	}

	// instance wants host product string in ptr
	if(opcode==audioMasterGetProductString)
	{
		sprintf((char*)ptr,"Aodix");
		return 0;
	}

	// instance wants host version
	if(opcode==audioMasterGetVendorVersion)
		return gl_padx->aodix_version;

	// instance wants specific opcode
	if(opcode==audioMasterVendorSpecific)
		return 0;

	// instance wants to set icon (in editor window?)
	if(opcode==audioMasterSetIcon)
		return 0;

	// instance wants host capabilities
	if(opcode==audioMasterCanDo)
	{
		// host can send vst events
		if(strcmp((char*)ptr,"sendVstEvents")==0)
			return 1;

		// host can send vst midi events
		if(strcmp((char*)ptr,"sendVstMidiEvent")==0)
			return 1;

		// host can send vst time info
		if(strcmp((char*)ptr,"sendVstTimeInfo")==0)
			return 1;

		// host can receive vst events
		if(strcmp((char*)ptr,"receiveVstEvents")==0)
			return 1;

		// host can receive vst midi events
		if(strcmp((char*)ptr,"receiveVstMidiEvent")==0)
			return 1;

		// host can receive vst time info
		if(strcmp((char*)ptr,"receiveVstTimeInfo")==0)
			return 1;

		// host can supply idle
		if(strcmp((char*)ptr,"supplyIdle")==0)
			return 1;

		// host cannot accept IO changes
		if(strcmp((char*)ptr,"acceptIOChanges")==0)
			return 0;

		// host cannot report connection changes
		if(strcmp((char*)ptr,"reportConnectionChanges")==0)
			return 0;

		// host cannot size window
		if(strcmp((char*)ptr,"sizeWindow")==0)
			return 0;

		// host doesnt support async processing
		if(strcmp((char*)ptr,"asyncProcessing")==0)
			return 0;

		// host doesnt support offline processing
		if(strcmp((char*)ptr,"offline")==0)
			return 0;

		// host doesnt support shell
		if(strcmp((char*)ptr,"supportShell")==0)
			return 0;

		// host doesnt support openfileselector
		if(strcmp((char*)ptr,"openFileSelector")==0)
			return 0;

		// host doesnt support editfile
		if(strcmp((char*)ptr,"editFile")==0)
			return 0;

		// host doesnt support close file selector
		if(strcmp((char*)ptr,"closeFileSelector")==0)
			return 0;

		// host doesnt support shell categories
		if(strcmp((char*)ptr,"shellCategory")==0)
			return 0;

		// host doesnt support start/stop processing
		if(strcmp((char*)ptr,"startStopProcess")==0)
			return 0;

		// else
		return 0;
	}

	// instance wants host language
	if(opcode==audioMasterGetLanguage)
		return kVstLangEnglish;

	// instance wants host plugins directory
	if(opcode==audioMasterGetDirectory)
		return long(gl_padx->cfg.vst_path[0]);

	// instance wants host to update its display (editors, etc)
	if(opcode==audioMasterUpdateDisplay)
	{
		// update vst editor window caption
		gl_padx->host_update_window_caption(effect);

		// something has changed, post gui update
		gl_padx->gui_is_dirty=1;
		return 0;
	}

	// else
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::host_idle(void)
{
	// scan all instances
	for(int i=0;i<MAX_INSTANCES;i++)
	{
		// get plugin instance
		ADX_INSTANCE* pi=&instance[i];

		// instance is allocated?
		if(pi->peffect!=NULL)
		{
			// call plugin's idle
			pi->peffect->dispatcher(pi->peffect,effIdle,0,0,NULL,0.0f);

			// call also plugin's effidle if editor opened
			if(pi->hwnd!=NULL && IsWindow(pi->hwnd) && (pi->peffect->flags & effFlagsHasEditor))
				pi->peffect->dispatcher(pi->peffect,effEditIdle,0,0,NULL,0.0f);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::host_update_window_caption(AEffect* peffect)
{
	// instance index
	int i_index=0;

	// get instance pointer and index
	ADX_INSTANCE* pi=instance_get_from_effect(peffect,&i_index);

	// update plugin window if opened
	if(pi!=NULL && pi->peffect!=NULL && pi->hwnd!=NULL && IsWindow(pi->hwnd))
	{
		// get selected program
		int const p_index=pi->peffect->dispatcher(pi->peffect,effGetProgram,0,0,NULL,0.0f);

		// set plugin window caption
		char buf[128];
		char prg_buf[32];
		prg_buf[0]=0;

		// get program name
		peffect->dispatcher(peffect,effGetProgramName,0,0,prg_buf,0.0f);

		// format string
		sprintf(buf,"%.2X: %s - %.2X: %s",i_index,pi->alias,p_index,prg_buf);

		// set window caption
		SetWindowText(pi->hwnd,buf);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::host_on_automation(AEffect* peffect,int const param,float const value)
{
	extern HWND gl_hwnd_main;

	// check if effect sending automation is loaded and param index is in valid range
	if(peffect!=NULL && param>=0 && param<peffect->numParams)
	{
		// automation enabled and effect is not null
		if((master_time_info.flags & kVstAutomationWriting) && peffect!=NULL)
		{
			// instance index and next event position
			int instance_index=0;
			int new_event_pos=project.pattern[user_pat].usr_pos;

			// get instance pointer and index
			ADX_INSTANCE* pi=instance_get_from_effect(peffect,&instance_index);

			// write automation event (live else offline)
			if((master_time_info.flags & kVstTransportPlaying) && cfg.rec_live)
				new_event_pos=seq_sample_to_pos(master_transport_sampleframe);

			// update undo (combining)
			edit_undo_snapshot(true);

			// add automation event
			seq_add_event(new_event_pos,user_pat,user_trk,4,instance_index,(param>>8)&0xFF,(param&0xFF),int(value*255.0f),user_edit_overwrite);
		}

		// get user instance
		ADX_INSTANCE* pui=&instance[user_instance];

		// select param if instance automated is same as user selected instance
		if(peffect==pui->peffect)
		{
			// select param
			user_parameter=param;

			// new parameter index before parameter list offset
			if(user_parameter<user_parameter_list_offset)
				user_parameter_list_offset=user_parameter;

			// new parameter after parameter list offset
			if(user_parameter>(user_parameter_list_offset+10))
				user_parameter_list_offset=user_parameter-10;
		}

		// post refresh
		gui_is_dirty=1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::host_on_events(AEffect* peffect,VstEvents* pevents)
{
	// instance index
	int i_index=0;

	// get instance pointer and index
	ADX_INSTANCE* pi=instance_get_from_effect(peffect,&i_index);

	// update plugin window if opened
	if(pi!=NULL && pi->peffect!=NULL)
	{
		// scan midi-out wire links
		for(int w=0;w<pi->mout_pin.num_wires;w++)
		{
			// get wire pointer
			ADX_WIRE* pw=&pi->mout_pin.pwire[w];

			// get destination midi link instance
			ADX_INSTANCE* pi_mol=&instance[pw->instance_index];

			// check that effect is opened
			if(pi_mol!=NULL)
			{
				// scan incoming event stream
				for(int e=0;e<pevents->numEvents;e++)
				{
					// get event pointer
					VstMidiEvent* pmi=(VstMidiEvent*)pevents->events[e];

					// check destinator queue
					if(pi_mol->midi_queue_size<MAX_BLOCK_EVENTS)
					{
						// get instance event pointer
						VstMidiEvent* ptr_routed_event=&pi_mol->midi_event[pi_mol->midi_queue_size];

						// copy event from vst host audio master
						*ptr_routed_event=*pmi;

						// scale velocity
						if((ptr_routed_event->midiData[0]&0xF0)==0x90)
							ptr_routed_event->midiData[2]=arg_tool_clipped_assign(int(float(ptr_routed_event->midiData[2])*pw->value),1,127);

						// increment queue
						pi_mol->midi_queue_size++;
					}
				}
			}
		}
	}
}
