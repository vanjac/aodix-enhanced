/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core DSP Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::dsp_work(void)
{
	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// temporal sequencer flags
	int jump=0;

	// highest pin count flag
	int hpc=3;

	// tempo change flag
	double tempo_change=0.0;

	// track flags
	int track_on[MAX_TRACKS];

	// track solo mode
	int solo_mode=0;

	// scan track flags
	for(int t=0;t<MAX_TRACKS;t++)
	{
		// default enabled
		track_on[t]=1;

		// mute track
		if(pp->track[t].mute)
			track_on[t]=0;

		// enable solo scan
		if(pp->track[t].solo)
			solo_mode=1;
	}

	// track(s) solo
	if(solo_mode)
	{
		for(int t=0;t<MAX_TRACKS;t++)
		{
			track_on[t]=0;

			if(pp->track[t].solo)
				track_on[t]=1;
		}
	}

	// clear dsp output buffers
	for(int mo=0;mo<NUM_DSP_OUTPUTS;mo++)
		arg_dsp_zero(dsp_output_buffer[mo],dsp_block_size);

	// instance working flag
	bool i_worked[MAX_INSTANCES];

	// init instance buffers
	for(int i=0;i<MAX_INSTANCES;i++)
	{
		// set non-worked flag
		i_worked[i]=false;

		// get instance pointer
		ADX_INSTANCE* pi=&instance[i];

		if(pi->peffect!=NULL)
		{
			// clear pins samples
			for(int ci=0;ci<pi->peffect->numInputs;ci++)
				arg_dsp_zero(pi->pins[ci],dsp_block_size);

			// clear pouts samples
			for(int co=0;co<pi->peffect->numOutputs;co++)
				arg_dsp_zero(pi->pous[co],dsp_block_size);
		}
	}

	// sequencer position to sample conversion constants
	double const d_points_per_second=project.master_tempo*double(project.master_ppqn)/60.0;
	double const d_samples_per_point=cfg.asio_driver_sample_rate/d_points_per_second;

	// get samples per point integer
	int const i_samples_per_point=int(d_samples_per_point*16.0);

	// get transport and pattern cue positions
	int const trn_pos_sample=(master_transport_sampleframe<<4)/i_samples_per_point;
	int const cue_sta_sample=(pp->cue_sta*i_samples_per_point)>>4;
	int const cue_end_sample=(pp->cue_end*i_samples_per_point)>>4;
	int const cue_stp_sample=(pp->cue_stp*i_samples_per_point)>>4;

	// get transport cycle status
	int const trn_cycle=master_time_info.flags & kVstTransportCycleActive;

	// fill audio master time info structure
	master_time_info.samplePos=double(master_transport_sampleframe);
	master_time_info.sampleRate=cfg.asio_driver_sample_rate;
	master_time_info.nanoSeconds=0.0;
	master_time_info.ppqPos=double(trn_pos_sample)/double(project.master_ppqn);
	master_time_info.tempo=(cfg.asio_driver_sample_rate*60.0)/(double(int(i_samples_per_point*project.master_ppqn))/16.0);
	master_time_info.barStartPos=(int(master_time_info.ppqPos)/project.master_numerator)*project.master_numerator;
	master_time_info.cycleStartPos=double(pp->cue_sta)/double(project.master_ppqn);
	master_time_info.cycleEndPos=double(pp->cue_end)/double(project.master_ppqn);
	master_time_info.timeSigNumerator=project.master_numerator;
	master_time_info.timeSigDenominator=project.master_denominator;
	master_time_info.smpteOffset=0;
	master_time_info.smpteFrameRate=0;
	master_time_info.samplesToNextClock=0;

	// transport process
	if(master_time_info.flags & kVstTransportPlaying)
	{
		// get start and end sample indexes of this dsp block
		int const block_sample_sta=master_transport_sampleframe;
		int const block_sample_end=master_transport_sampleframe+dsp_block_size;

		// scan all sequencer events
		for(int e=0;e<seq_num_events;e++)
		{
			// get event pointer
			ADX_EVENT* pe=&seq_event[e];

			// convert event location to 32-bit integer sample index stamp
			int const event_sample_sta=(pe->pos*i_samples_per_point)>>4;
			int const event_sample_end=((pe->pos+pe->par*pe->szd)*i_samples_per_point)>>4;

			// check if event is in current audio block scope, current pattern and track is active
			if(event_sample_sta<block_sample_end && event_sample_end>=block_sample_sta && pe->pat==user_pat && track_on[pe->trk] && !(trn_cycle && event_sample_sta>=cue_end_sample))
			{
				// note event
				if(pe->typ==0)
				{
					// note on
					if(event_sample_sta>=block_sample_sta)
						instance_add_midi_event(&instance[pe->da0],pe->trk,0x90+(pe->da1&0xF),pe->da2,pe->da3,0,event_sample_sta-block_sample_sta);

					// note off
					if(event_sample_end<block_sample_end)
						instance_add_midi_event(&instance[pe->da0],pe->trk,0x80+(pe->da1&0xF),pe->da2,0x40,0,event_sample_end-block_sample_sta);
				}

				// pattern event
				if(pe->typ==1)
				{
					// get referred marker
					ADX_MARKER* pm=&project.pattern[pe->da0].marker[pe->da1];

					// get marker offset
					int const pe_marker_offset=pm->flg*pm->pos;

					// get note tranpose amount
					int const pe_note_transpose=int(pe->da2)-128;

					// get velo tranpose amount
					int const pe_velo_transpose=int(pe->da3)-128;

					// scan all called pattern (sub-events)
					for(int se=0;se<seq_num_events;se++)
					{
						// get sub-event
						ADX_EVENT* pse=&seq_event[se];

						// get sub-event position offset
						int const i_sub_event_position_off=(pe->pos+pse->pos)-pe_marker_offset;

						// convert sub-event location to 32-bit integer sample index stamp
						int const sub_event_sample_sta=(i_sub_event_position_off*i_samples_per_point)>>4;
						int const sub_event_sample_end=((i_sub_event_position_off+pse->par*pse->szd)*i_samples_per_point)>>4;

						// check if event is in current audio block scope, current pattern and track is active
						if(sub_event_sample_sta<block_sample_end && sub_event_sample_end>=block_sample_sta && pse->pat==pe->da0 && !(trn_cycle && sub_event_sample_sta>=cue_end_sample))
						{
							// sub note event
							if(pse->typ==0)
							{
								// check sub note-on
								if(sub_event_sample_sta>=block_sample_sta && sub_event_sample_sta<event_sample_end)
								{
									// get transposed note
									int const transposed_note=arg_tool_clipped_assign(int(pse->da2)+pe_note_transpose,1,127);

									// get transposed velo
									int const transposed_velo=arg_tool_clipped_assign(int(pse->da3)+pe_velo_transpose,1,127);

									// send midi sub note-on event
									instance_add_midi_event(&instance[pse->da0],pe->trk,0x90+(pse->da1&0xF),transposed_note,transposed_velo,0,sub_event_sample_sta-block_sample_sta);
								}

								// check sub note-off
								if(sub_event_sample_end<block_sample_end && sub_event_sample_end<event_sample_end)
								{
									// calculate transposed note
									int const transposed_note=arg_tool_clipped_assign(int(pse->da2)+pe_note_transpose,1,127);

									// send midi sub note-off event
									instance_add_midi_event(&instance[pse->da0],pe->trk,0x80+(pse->da1&0xF),transposed_note,0x40,0,sub_event_sample_end-block_sample_sta);
								}
							}

							// sub midi automation event
							if(pse->typ==3)
								instance_add_midi_event(&instance[pse->da0],pe->trk,pse->da1,pse->da2,pse->da3,0,sub_event_sample_end-block_sample_sta);

							// sub vst automation event
							if(pse->typ==4)
								instance_set_param(&instance[pse->da0],(pse->da1<<8)|(pse->da2),float(pse->da3)/255.0f);

							// sub tempo automation event
							if(pse->typ==5)
								tempo_change=double(pse->da0)+double(pse->da1)*0.00390625;
						}
					}
				}

				// jump event
				if(pe->typ==2 && master_transport_sampleframe!=event_sample_end)
					jump=event_sample_end;

				// midi automation event
				if(pe->typ==3)
					instance_add_midi_event(&instance[pe->da0],pe->trk,pe->da1,pe->da2,pe->da3,0,event_sample_sta-block_sample_sta);

				// vst automation event
				if(pe->typ==4)
					instance_set_param(&instance[pe->da0],(pe->da1<<8)|(pe->da2),float(pe->da3)/255.0f);

				// tempo automation event
				if(pe->typ==5)
					tempo_change=double(pe->da0)+double(pe->da1)*0.00390625;
			}
		}
	}

	// master input work
	for(i=0;i<NUM_DSP_INPUTS;i++)
	{
		// get pin pointer
		ADX_PIN* pp=&master_input_pin[i];

		// scan wires
		for(int w=0;w<pp->num_wires;w++)
		{
			// get wire pointer
			ADX_WIRE* pw=&pp->pwire[w];

			// mix master input in other plugin input
			if(pw->instance_index<MASTER_INSTANCE)
			{
				ADX_INSTANCE* pdi=&instance[pw->instance_index];
				if(pdi->peffect)
					arg_dsp_gmix(dsp_input_buffer[i],pdi->pins[pw->pin_index],dsp_block_size,pw->value*pw->value);
			}

			// mix master input in master out
			if(pw->instance_index==MASTER_INSTANCE)
			{
				arg_dsp_gmix(dsp_input_buffer[i],dsp_output_buffer[pw->pin_index],dsp_block_size,pw->value*pw->value);
							
				// update highest pin count
				if(pw->pin_index>hpc)
					hpc=pw->pin_index;
			}
		}
	}

	// routing work
	bool all_worked=false;

	// bubble routing loop iteration
	while(!all_worked)
	{
		all_worked=true;

		// scan all instances
		for(i=0;i<MAX_INSTANCES;i++)
		{
			// get working candidate machine
			ADX_INSTANCE* pi=&instance[i];

			// only instanced slots and not-working machine should be evaluated
			if(pi->peffect!=NULL && i_worked[i]==false)
			{
				// check if machine indexed 'i' can work in this iteration
				bool can_work=true;

				// scan destination instances
				for(int ii=0;ii<MAX_INSTANCES;ii++)
				{
					// get destination instance
					ADX_INSTANCE* pii=&instance[ii];

					// skip current machine or uninstanced machine
					if(pii->peffect!=NULL && ii!=i)
					{	
						// check if any output is connected to our machine
						for(int io=0;io<pii->peffect->numOutputs;io++)
						{
							// get output pin pointer
							ADX_PIN* pp=&pii->pout_pin[io];

							// scan wires
							for(int w=0;w<pp->num_wires;w++)
							{
								// get wire pointer
								ADX_WIRE* pw=&pp->pwire[w];

								// check if instance 'ii' (source) wich is connected to candidate didnt work
								// so candidate must wait to next iteration
								if(pw->instance_index==i && i_worked[ii]==false)
									can_work=false;
							}
						}

						// check if any midi-out pin(s) is connected to source instance
						for(int w=0;w<pii->mout_pin.num_wires;w++)
						{
							// get wire pointer
							ADX_WIRE* pw=&pii->mout_pin.pwire[w];

							// check if any midi-out link is connected to working candidate instance and source instance still didnt worked
							// so candidate must wait to next iteration
							if(pw->instance_index==i && i_worked[ii]==false)
								can_work=false;
						}
					}
				}

				// work if flag set
				if(can_work)
				{
					// set worked flag
					i_worked[i]=true;

					// sort events by delta
					if(pi->midi_queue_size>1)
					{
						bool all_sorted=false;

						while(!all_sorted)
						{
							all_sorted=true;

							// scan block events
							for(int ev=0;ev<(pi->midi_queue_size-1);ev++)
							{
								// check event pair
								if(pi->midi_event[ev].deltaFrames>pi->midi_event[ev+1].deltaFrames)
								{
									VstMidiEvent tev=pi->midi_event[ev];
									pi->midi_event[ev]=pi->midi_event[ev+1];
									pi->midi_event[ev+1]=tev;
									all_sorted=false;
								}
							}
						}
					}

					// prepare event group
					VstEvents event_group;
					event_group.numEvents=pi->midi_queue_size;
					event_group.reserved=0;

					// place event(s)
					for(int eq=0;eq<pi->midi_queue_size;eq++)
						event_group.events[eq]=(VstEvent*)&pi->midi_event[eq];

					// empty list
					pi->midi_queue_size=0;

					// vst processing, send the events
					pi->peffect->dispatcher(pi->peffect,effProcessEvents,0,0,&event_group,0.0f);

					// plugin process
					if(pi->peffect->flags & effFlagsCanReplacing)
						pi->peffect->processReplacing(pi->peffect,pi->pins,pi->pous,dsp_block_size);
					else
						pi->peffect->process(pi->peffect,pi->pins,pi->pous,dsp_block_size);
				
					// output rout
					for(int o=0;o<pi->peffect->numOutputs;o++)
					{
						// instance process mute
						if(pi->process_mute)
							arg_dsp_zero(pi->pous[o],dsp_block_size);

						// instance process thru (bypass)
						if(pi->process_thru)
						{
							if(o<pi->peffect->numInputs)
								arg_dsp_copy(pi->pins[o],pi->pous[o],dsp_block_size);
							else
								arg_dsp_zero(pi->pous[o],dsp_block_size);
						}

						// get audio-output pin pointer
						ADX_PIN* pp=&pi->pout_pin[o];

						// scan wires
						for(int w=0;w<pp->num_wires;w++)
						{
							// get wire pointer
							ADX_WIRE* pw=&pp->pwire[w];

							// mix instance out in other plugin input
							if(pw->instance_index<MASTER_INSTANCE)
							{
								ADX_INSTANCE* pdi=&instance[pw->instance_index];
								if(pdi->peffect)
									arg_dsp_gmix(pi->pous[o],pdi->pins[pw->pin_index],dsp_block_size,pw->value*pw->value);
							}

							// mix instance out in master out
							if(pw->instance_index==MASTER_INSTANCE)
							{
								arg_dsp_gmix(pi->pous[o],dsp_output_buffer[pw->pin_index],dsp_block_size,pw->value*pw->value);
							
								// update highest pin count
								if(pw->pin_index>hpc)
									hpc=pw->pin_index;
							}
						}
					}
				}
				else
				{
					all_worked=false;
				}
			}
		}
	}

	// drive master vumeter
	for(mo=0;mo<NUM_DSP_OUTPUTS;mo++)
		dsp_vumeter_drive(dsp_output_buffer[mo],dsp_output_vumeter[mo],dsp_block_size);

	// transport advance
	if(master_time_info.flags & kVstTransportPlaying)
	{
		// transport increment (and jump command dispatch)
		if(jump)
			master_transport_sampleframe=jump;
		else
			master_transport_sampleframe+=dsp_block_size;

		// tempo change dispatch
		if(tempo_change>=10.0)
		{
			// get current sequencer position
			int const trn_new_sample=(master_transport_sampleframe<<4)/i_samples_per_point;

			// set new tempo
			project.master_tempo=tempo_change;

			// set sequencer position
			master_transport_sampleframe=seq_pos_to_sample(trn_new_sample+1);
		}

		// perform transport cycle
		if(trn_cycle && master_transport_sampleframe>=cue_end_sample)
		{
			// all sounds off thru all instances
			for(i=0;i<MAX_INSTANCES;i++)
				instance_midi_panic(&instance[i],true,false);

			// wrap master sampleframe pos
			master_transport_sampleframe=cue_sta_sample;

			// scroll back if rec live is switched
			if(cfg.rec_live)
				pp->usr_pos=pp->cue_sta;

			// post refresh
			gui_is_dirty=1;
		}

		// check pattern cue-stop
		if(pp->cue_stp>0 && master_transport_sampleframe>=cue_stp_sample)
		{
			// stop wrap
			if(cfg.stop_wrap)
				pp->usr_pos=pp->cue_stp;

			// stop transport
			master_time_info.flags&=~kVstTransportPlaying;

			// post refresh
			gui_is_dirty=1;
		}
	}

	// set new highest output pin count
	dsp_highest_opin_count=hpc;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::dsp_clear_input_buffers()
{
	for(int i=0;i<NUM_DSP_INPUTS;i++)
		arg_dsp_zero(dsp_input_buffer[i],dsp_block_size);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::dsp_transport_play(void)
{
	// enter critical section
	asio_enter_cs();

	// undo if recording
	edit_undo_snapshot();
	edit_undo_combine_next();

	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// send all notes off/all sounds off if transport were playing
	if(master_time_info.flags & kVstTransportPlaying)
	{
		// scan all instances
		for(int i=0;i<MAX_INSTANCES;i++)
			instance_midi_panic(&instance[i],true,true);
	}

	// set transport sample frame position from user pattern edit position
	master_transport_sampleframe=seq_pos_to_sample(pp->usr_pos);

	// launch transport
	master_time_info.flags|=kVstTransportPlaying;

	// leave critical section
	asio_leave_cs();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::dsp_transport_stop(void)
{
	// enter critical section
	asio_enter_cs();

	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// stop wrap
	if(cfg.stop_wrap)
		pp->usr_pos=edit_quantize(seq_sample_to_pos(master_transport_sampleframe));

	// stop transport
	master_time_info.flags&=~kVstTransportPlaying;

	// send all notes off (all sounds off too if transport is stopped)
	for(int i=0;i<MAX_INSTANCES;i++)
		instance_midi_panic(&instance[i],true,true);

	// leave critical section
	asio_leave_cs();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::dsp_vumeter_drive(float* psrc,float& vum,int const num_samples)
{
	// drive vu
	for(int s=0;s<num_samples;s++)
	{
		// get peak
		float const f_peak=fabsf(psrc[s]);

		// vumeter peak raise
		if(f_peak>vum)
			vum=f_peak;
	}
}
