/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Edit Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAodixCore::edit_get_quantization(void)
{
	// get current ppqn
	int const i_ppqn=project.master_ppqn;

	// select quantize
	switch(user_quantize)
	{
		// standard
	case 2:	return i_ppqn/32;
	case 3:	return i_ppqn/16;
	case 4:	return i_ppqn/8;
	case 5:	return i_ppqn/4;
	case 6:	return i_ppqn/2;
	case 7:	return i_ppqn;
	case 8:	return i_ppqn*2;
	case 9:	return i_ppqn*4;

		// dotted
	case 11: return (i_ppqn*3)/64;
	case 12: return (i_ppqn*3)/32;
	case 13: return (i_ppqn*3)/16;
	case 14: return (i_ppqn*3)/8;
	case 15: return (i_ppqn*3)/4;
	case 16: return (i_ppqn*3)/2;
	case 17: return (i_ppqn*3);
	case 18: return (i_ppqn*3)*2;

		// triplets
	case 20: return i_ppqn/48;
	case 21: return i_ppqn/24;
	case 22: return i_ppqn/12;
	case 23: return i_ppqn/6;
	case 24: return i_ppqn/3;
	case 25: return (i_ppqn*8)/12;
	case 26: return (i_ppqn*8)/6;
	case 27: return (i_ppqn*8)/3;
	}

	// default quantization
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAodixCore::edit_quantize(int const position)
{
	// get quantization points
	int const i_q=edit_get_quantization();

	// return quantized position
	return ((position+(i_q/2))/i_q)*i_q;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_undo_snapshot(bool combine)
{
	if(undo_combine&&combine)
		return;

	// store number of events for undo buffer
	undo_num_events=seq_num_events;

	// store edit event array in undo buffer
	for(int e=0;e<undo_num_events;e++)
		undo_event[e]=seq_event[e];

	// combine next snapshot
	undo_combine=combine;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_undo_combine_next(void)
{
	undo_combine=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_undo(void)
{
	// swap undo data
	int num_events_tmp=undo_num_events;
	undo_num_events=seq_num_events;
	seq_num_events=num_events_tmp;

	// don't combine next snapshot
	undo_combine=false;

	// temporary event storage
	ADX_EVENT e_tmp;

	// num events to swap
	int num_swap=max(undo_num_events,seq_num_events);

	// swap undo events
	for(int e=0;e<num_swap;e++)
	{
		e_tmp=undo_event[e];
		undo_event[e]=seq_event[e];
		seq_event[e]=e_tmp;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_insert(int const all_tracks)
{
	// set wait cursor
	SetCursor(hcursor_wait);

	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// get quantization
	int const i_quantize=edit_get_quantization();

	// update undo
	edit_undo_snapshot();

	// scan sequencer events events
	for(int e=0;e<seq_num_events;e++)
	{
		// get event pointer
		ADX_EVENT* pe=&seq_event[e];

		// push event (current track only)
		if(all_tracks==0 && pe->pat==user_pat && pe->pos>=pp->usr_pos && pe->trk==user_trk)
			pe->pos+=i_quantize;

		// push event (all tracks)
		if(all_tracks==1 && pe->pat==user_pat && pe->pos>=pp->usr_pos)
			pe->pos+=i_quantize;
	}

	// set arrow cursor
	SetCursor(hcursor_arro);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_back(int const all_tracks)
{
	// set wait cursor
	SetCursor(hcursor_wait);

	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// get quantization
	int const i_quantize=edit_get_quantization();

	// update undo
	edit_undo_snapshot();

	// delete everything in pos (current track only)
	if(all_tracks==0)
		seq_delete_events_at(user_pat,pp->usr_pos,i_quantize,user_trk,1);

	// delete everything in pos (all tracks)
	if(all_tracks==1)
		seq_delete_events_at(user_pat,pp->usr_pos,i_quantize,0,MAX_TRACKS);

	// scan sequencer events
	for(int e=0;e<seq_num_events;e++)
	{
		// get event pointer
		ADX_EVENT* pe=&seq_event[e];

		// pull event (current track only)
		if(all_tracks==0 && pe->pat==user_pat && pe->pos>=pp->usr_pos && pe->trk==user_trk)
			pe->pos-=i_quantize;

		// pull event (all tracks)
		if(all_tracks==1 && pe->pat==user_pat && pe->pos>=pp->usr_pos)
			pe->pos-=i_quantize;
	}

	// set arrow cursor
	SetCursor(hcursor_arro);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_resize(int const all_tracks)
{
	// set wait cursor
	SetCursor(hcursor_wait);

	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// update undo
	edit_undo_snapshot();

	// start by trying to shorten events
	int last_end_pos=0;

	// scan sequencer events
	for(int e=0;e<seq_num_events;e++)
	{
		// get event pointer
		ADX_EVENT* pe=&seq_event[e];

		if(pe->pat==user_pat && (all_tracks==1 || pe->trk==user_trk)
			&& pe->szd && pe->pos < pp->usr_pos)
		{
			int end_pos = pe->pos + pe->par;
			if(end_pos > pp->usr_pos)
			{
				// cut event
				pe->par = pp->usr_pos - pe->pos;
				last_end_pos=MAX_SIGNED_INT;
			}
			else if(end_pos < pp->usr_pos && end_pos > last_end_pos)
				last_end_pos=end_pos;
		}
	}

	// couldn't find any event to shorten, so now try to extend events
	if(last_end_pos != MAX_SIGNED_INT)
	{
		// scan sequencer events
		for(int e=0;e<seq_num_events;e++)
		{
			// get event pointer
			ADX_EVENT* pe=&seq_event[e];

			if(pe->pat==user_pat && (all_tracks==1 || pe->trk==user_trk) && pe->szd)
			{
				int end_pos = pe->pos + pe->par;
				// extend event
				if(end_pos==last_end_pos)
					pe->par = pp->usr_pos - pe->pos;
			}
		}
	}

	// set arrow cursor
	SetCursor(hcursor_arro);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_copy(int const cut)
{
	// set wait cursor
	SetCursor(hcursor_wait);

	// get block length
	int const user_block_pos_len=user_block_pos_end-user_block_pos_sta;
	int const user_block_trk_len=user_block_trk_end-user_block_trk_sta;

	// range selected
	if(user_block_pos_len>0 && user_block_trk_len>0)
	{
		// reset event clipboard
		clipboard_num_events=0;

		// copy sequencer events to clipboard
		for(int e=0;e<seq_num_events;e++)
		{
			// get sequencer event pointer
			ADX_EVENT* pe=&seq_event[e];

			// check if event is in block range and copy to event clipboard buffer
			if(pe->pat==user_pat && pe->pos>=user_block_pos_sta && pe->pos<user_block_pos_end && pe->trk>=user_block_trk_sta && pe->trk<user_block_trk_end)
				clipboard_event[clipboard_num_events++]=*pe;
		}

		// set clipboard offset and length
		clipboard_pos_sta=user_block_pos_sta;
		clipboard_trk_sta=user_block_trk_sta;
		clipboard_pos_len=user_block_pos_len;
		clipboard_trk_len=user_block_trk_len;

		// perform cut
		if(cut)
		{
			// update undo
			edit_undo_snapshot();

			// cut events within block range
			seq_delete_events_at(user_pat,user_block_pos_sta,user_block_pos_len,user_block_trk_sta,user_block_trk_len);
		}
	}

	// set arrow cursor
	SetCursor(hcursor_arro);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_select_all(void)
{
	// select all
	user_block_pos_sta=0;
	user_block_pos_end=MAX_SIGNED_INT>>1;
	user_block_trk_sta=0;
	user_block_trk_end=MAX_TRACKS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_transpose(int const amt,int const apply_quantize)
{
	// set wait cursor
	SetCursor(hcursor_wait);

	// get block length
	int const user_block_pos_len=user_block_pos_end-user_block_pos_sta;
	int const user_block_trk_len=user_block_trk_end-user_block_trk_sta;

	// update undo
	edit_undo_snapshot(!apply_quantize);

	// range selected
	if(user_block_pos_len>0 && user_block_trk_len>0)
	{
		// scan sequencer events
		for(int e=0;e<seq_num_events;e++)
		{
			// get event pointer
			ADX_EVENT* pe=&seq_event[e];

			// check if event is in block range and it's a note event
			if(pe->pat==user_pat && pe->pos>=user_block_pos_sta && pe->pos<user_block_pos_end && pe->trk>=user_block_trk_sta && pe->trk<user_block_trk_end)
			{
				// transpose note event
				if(pe->typ==EVT_NOT)
				{
					switch(user_row)
					{
					case 0:	pe->da2=arg_tool_clipped_assign(int(pe->da2)+amt,0,127);		break;
					case 1: pe->da0=arg_tool_clipped_assign(int(pe->da0)+amt*16,0,255);		break;
					case 2: pe->da0=arg_tool_clipped_assign(int(pe->da0)+amt,0,255);		break;
					case 3: pe->da1=arg_tool_clipped_assign(int(pe->da1)+amt*16,0,15);		break;
					case 4: pe->da1=arg_tool_clipped_assign(int(pe->da1)+amt,0,15);			break;
					case 5: pe->da2=arg_tool_clipped_assign(int(pe->da2)+amt*16,0,127);		break;
					case 6: pe->da2=arg_tool_clipped_assign(int(pe->da2)+amt,0,127);		break;
					case 7: pe->da3=arg_tool_clipped_assign(int(pe->da3)+amt*16,0,127);		break;
					case 8: pe->da3=arg_tool_clipped_assign(int(pe->da3)+amt,0,127);		break;
					}
				}

				// transpose pattern, vst automation or tempo automation event
				if(pe->typ==EVT_PAT || pe->typ==EVT_AUT || pe->typ==EVT_TMP)
				{
					switch(user_row)
					{
					case 0:	pe->da2=arg_tool_clipped_assign(int(pe->da2)+amt,0,255);		break;
					case 1: pe->da0=arg_tool_clipped_assign(int(pe->da0)+amt*16,0,255);		break;
					case 2: pe->da0=arg_tool_clipped_assign(int(pe->da0)+amt,0,255);		break;
					case 3: pe->da1=arg_tool_clipped_assign(int(pe->da1)+amt*16,0,255);		break;
					case 4: pe->da1=arg_tool_clipped_assign(int(pe->da1)+amt,0,255);		break;
					case 5: pe->da2=arg_tool_clipped_assign(int(pe->da2)+amt*16,0,255);		break;
					case 6: pe->da2=arg_tool_clipped_assign(int(pe->da2)+amt,0,255);		break;
					case 7: pe->da3=arg_tool_clipped_assign(int(pe->da3)+amt*16,0,255);		break;
					case 8: pe->da3=arg_tool_clipped_assign(int(pe->da3)+amt,0,255);		break;
					}
				}

				// transpose midi event
				if(pe->typ==EVT_MID)
				{
					switch(user_row)
					{
					case 0:	pe->da2=arg_tool_clipped_assign(int(pe->da3)+amt,0,127);		break;
					case 1: pe->da0=arg_tool_clipped_assign(int(pe->da0)+amt*16,0,255);		break;
					case 2: pe->da0=arg_tool_clipped_assign(int(pe->da0)+amt,0,255);		break;
					case 3: pe->da1=arg_tool_clipped_assign(int(pe->da1)+amt*16,0,255);		break;
					case 4: pe->da1=arg_tool_clipped_assign(int(pe->da1)+amt,0,255);		break;
					case 5: pe->da2=arg_tool_clipped_assign(int(pe->da2)+amt*16,0,127);		break;
					case 6: pe->da2=arg_tool_clipped_assign(int(pe->da2)+amt,0,127);		break;
					case 7: pe->da3=arg_tool_clipped_assign(int(pe->da3)+amt*16,0,127);		break;
					case 8: pe->da3=arg_tool_clipped_assign(int(pe->da3)+amt,0,127);		break;
					}
				}

				// quantize event
				if(apply_quantize)
					pe->pos=edit_quantize(pe->pos);
			}
		}
	}

	// set arrow cursor
	SetCursor(hcursor_arro);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_paste(void)
{
	// set wait cursor
	SetCursor(hcursor_wait);

	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// update undo
	edit_undo_snapshot();

	// check for clipboard events
	if(clipboard_num_events>0)
	{
		// if overwrite, remove events within paste range
		if(user_edit_overwrite)
		{
			// check whenever we are pasting a block or all events from pattern
			if(clipboard_pos_len==MAX_SIGNED_INT)
				seq_delete_events_at_pattern(user_pat);
			else
				seq_delete_events_at(user_pat,pp->usr_pos,clipboard_pos_len,user_trk,clipboard_trk_len);
		}

		// paste events
		for(int ce=0;ce<clipboard_num_events;ce++)
		{
			// check for available event slot
			if(seq_num_events<MAX_EVENTS)
			{
				// clipboard event pointer
				ADX_EVENT* pce=&clipboard_event[ce];

				// new sequencer event pointer
				ADX_EVENT* pe=&seq_event[seq_num_events];

				// copy
				*pe=*pce;

				// adapt event location
				pe->pat=user_pat;
				pe->pos=pp->usr_pos+(pe->pos-clipboard_pos_sta);
				pe->trk=user_trk+(pe->trk-clipboard_trk_sta);

				// increment events
				seq_num_events++;
			}
		}
	}

	// set arrow cursor
	SetCursor(hcursor_arro);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_add_new_marker(ADX_PATTERN* pp,int const position)
{
	// scan all markers (skipping 0 marker, always placed in
	for(int m=1;m<MAX_MARKERS;m++)
	{
		// get marker pointer
		ADX_MARKER* pm=&pp->marker[m];

		// find first disabled flags
		if(pm->flg==0)
		{
			// found first disabled flag, update it
			pm->pos=edit_quantize(arg_tool_clipped_assign(position,0,MAX_SIGNED_INT));
			pm->flg=1;

			// dont continue
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_interpolate(void)
{
	// set wait cursor
	SetCursor(hcursor_wait);

	// space between interpolated events
	int i_quantize;
	if(edit_get_quantization()>1)
		i_quantize=edit_get_quantization();
	else
		i_quantize=1;

	// get block length
	int const user_block_pos_len=user_block_pos_end-user_block_pos_sta;
	int const user_block_trk_len=user_block_trk_end-user_block_trk_sta;

	// update undo
	edit_undo_snapshot();

	// range selected
	if(user_block_pos_len>0 && user_block_trk_len>0)
	{
		// each selected track
		for(int track=user_block_trk_sta;track<user_block_trk_end;track++)
		{
			// get first/last events inside selection
			ADX_EVENT* e_sta=edit_find_next_event(user_pat,user_block_pos_sta,track,user_block_pos_len);
			ADX_EVENT* e_end=edit_find_prev_event(user_pat,user_block_pos_end,track,user_block_pos_len);

			// must be at least 2 events at different times inside block
			if(e_sta && e_end && e_sta->pos<e_end->pos)
			{
				// time between first/last event
				double pos_diff=e_end->pos-e_sta->pos;

				// step by quantize amount
				for(int cur_pos=e_sta->pos;cur_pos<e_end->pos;cur_pos+=i_quantize)
				{
					// get event at cur_pos
					ADX_EVENT* pe=edit_find_event(user_pat,cur_pos,track);

					// create new event if doesn't exist
					if(!pe)
					{
						pe=&seq_event[seq_num_events];
						*pe=*e_sta;
						pe->pos=cur_pos;
						seq_num_events++;
					}

					// fraction of time from start to end
					double t=(cur_pos-e_sta->pos)/pos_diff;

					// interpolate data based on selected row
					if(user_row==1 || user_row==2)
						pe->da0=(int)(e_sta->da0+t*(e_end->da0-e_sta->da0));
					else if(user_row==3 || user_row==4)
						pe->da1=(int)(e_sta->da1+t*(e_end->da1-e_sta->da1));
					else if(user_row==5 || user_row==6 || user_row==0)
						pe->da2=(int)(e_sta->da2+t*(e_end->da2-e_sta->da2));
					else if(user_row==7 || user_row==8)
						pe->da3=(int)(e_sta->da3+t*(e_end->da3-e_sta->da3));
				}
			}
		}
	}

	// set arrow cursor
	SetCursor(hcursor_arro);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_randomize(void)
{
	// set wait cursor
	SetCursor(hcursor_wait);

	// get block length
	int const user_block_pos_len=user_block_pos_end-user_block_pos_sta;
	int const user_block_trk_len=user_block_trk_end-user_block_trk_sta;

	// update undo
	edit_undo_snapshot();

	// range selected
	if(user_block_pos_len>0 && user_block_trk_len>0)
	{
		// scan and transpose sequencer events
		for(int e=0;e<seq_num_events;e++)
		{
			// get event pointer
			ADX_EVENT* pe=&seq_event[e];

			// check if event is in block range and it's a note event
			if(pe->pat==user_pat && pe->pos>=user_block_pos_sta && pe->pos<user_block_pos_end && pe->trk>=user_block_trk_sta && pe->trk<user_block_trk_end)
			{
				// randomize note event
				if(pe->typ==EVT_NOT)
				{
					// select row
					switch(user_row)
					{
					case 0:	pe->da2=rand()&0x7F;	break;
					case 1:	pe->da0=rand()&0xFF;	break;
					case 2:	pe->da0=rand()&0xFF;	break;
					case 3:	pe->da1=rand()&0x0F;	break;
					case 4:	pe->da1=rand()&0x0F;	break;
					case 5:	pe->da2=rand()&0x7F;	break;
					case 6:	pe->da2=rand()&0x7F;	break;
					case 7:	pe->da3=rand()&0x7F;	break;
					case 8:	pe->da3=rand()&0x7F;	break;
					}
				}

				// randomize pattern or vst automation event
				if(pe->typ==EVT_PAT || pe->typ==EVT_AUT)
				{
					// select row
					switch(user_row)
					{
					case 1:	pe->da0=rand()&0xFF;	break;
					case 2:	pe->da0=rand()&0xFF;	break;
					case 3:	pe->da1=rand()&0xFF;	break;
					case 4:	pe->da1=rand()&0xFF;	break;
					case 5:	pe->da2=rand()&0xFF;	break;
					case 6:	pe->da2=rand()&0xFF;	break;
					case 7:	pe->da3=rand()&0xFF;	break;
					case 8:	pe->da3=rand()&0xFF;	break;
					}
				}

				// randomize midi automation event
				if(pe->typ==EVT_MID)
				{
					// select row
					switch(user_row)
					{
					case 1:	pe->da0=rand()&0xFF;	break;
					case 2:	pe->da0=rand()&0xFF;	break;
					case 3:	pe->da1=rand()&0x7F;	break;
					case 4:	pe->da1=rand()&0x7F;	break;
					case 5:	pe->da2=rand()&0x7F;	break;
					case 6:	pe->da2=rand()&0x7F;	break;
					case 7:	pe->da3=rand()&0x7F;	break;
					case 8:	pe->da3=rand()&0x7F;	break;
					}
				}

				// randomize tempo automation event
				if(pe->typ==EVT_TMP)
				{
					// select row
					switch(user_row)
					{
					case 1:	pe->da0=(rand()&0x7F)+0x10;	break;
					case 2:	pe->da0=(rand()&0x7F)+0x10;	break;
					case 3:	pe->da1=rand()&0xFF;	break;
					case 4:	pe->da1=rand()&0xFF;	break;
					case 5:	pe->da2=rand()&0xFF;	break;
					case 6:	pe->da2=rand()&0xFF;	break;
					case 7:	pe->da3=rand()&0xFF;	break;
					case 8:	pe->da3=rand()&0xFF;	break;
					}
				}
			}
		}
	}

	// set arrow cursor
	SetCursor(hcursor_arro);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAodixCore::edit_is_solo_mode(int pat)
{
	// get pattern pointer
	ADX_PATTERN* pp=&project.pattern[pat];

	// scan tracks
	for(int t=0;t<MAX_TRACKS;t++)
	{
		if(pp->track[t].solo)
			return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_toggle_mute(int pat,int trk)
{
	// get pattern pointer
	ADX_PATTERN* pp=&project.pattern[pat];

	// toggle track mute
	ADX_TRACK *pt = &pp->track[trk];
	pt->mute=!pt->mute;

	// stop muted notes
	if(pt->mute && !edit_is_solo_mode(pat))
		dsp_stop_playing_notes(pat,trk,false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_toggle_solo(int pat,int trk)
{
	// get pattern pointer
	ADX_PATTERN* pp=&project.pattern[pat];

	// get track pointer
	ADX_TRACK *pt = &pp->track[trk];

	// if this is the first solo track, mute all other non-muted tracks
	if(!pt->solo && !edit_is_solo_mode(pat))
		dsp_stop_playing_notes(pat,trk,true);

	// toggle track mute
	pt->solo=!pt->solo;

	// if this isn't the last solo track, mute this track
	if(!pt->solo && edit_is_solo_mode(pat))
		dsp_stop_playing_notes(pat,trk,false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_add_wire(ADX_PIN* pp,int const instance_index,int const pin_index,float const gain)
{
	// search for existing wire to same pin
	for(int w=0;w<pp->num_wires;w++)
	{
		ADX_WIRE* pw=&pp->pwire[w];

		// wire already exists
		if (pw->instance_index==instance_index && pw->pin_index==pin_index)
			return;
	}

	// enter critical section
	asio_enter_cs();

	// create new wire array
	ADX_WIRE* pnw=new ADX_WIRE[pp->num_wires+1];

	// copy old wires
	for(int w=0;w<pp->num_wires;w++)
		pnw[w]=pp->pwire[w];

	// get new wire pointer
	ADX_WIRE* pw=&pnw[pp->num_wires];

	// init new wire
	pw->instance_index=instance_index;
	pw->pin_index=pin_index;
	pw->value=gain;

	// delete old wires
	delete[] pp->pwire;

	// set new wire list
	pp->pwire=pnw;

	// increment num wires
	pp->num_wires++;

	// leave critical section
	asio_leave_cs();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_del_wire(ADX_PIN* pp,int const index)
{
	// enter critical section
	asio_enter_cs();

	// create new wire array
	ADX_WIRE* pnw=new ADX_WIRE[pp->num_wires-1];

	// counter
	int x=0;

	// copy old wires
	for(int w=0;w<pp->num_wires;w++)
	{
		// copy all wires but index
		if(w!=index)
			pnw[x++]=pp->pwire[w];
	}

	// delete old wires
	delete[] pp->pwire;

	// set new wire list
	pp->pwire=pnw;

	// increment num wires
	pp->num_wires--;

	// leave critical section
	asio_leave_cs();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::edit_clr_pin(ADX_PIN* pp)
{
	// delete wires
	delete[] pp->pwire;
	
	// clear pin
	pp->num_wires=0;
	pp->pwire=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ADX_EVENT* CAodixCore::edit_find_event(int pat,int pos,int trk)
{
	// scan sequencer events
	for(int e=0;e<seq_num_events;e++)
	{
		// get event pointer
		ADX_EVENT* pe=&seq_event[e];

		// event matches parameters
		if(pe->pat==pat && pe->pos==pos && pe->trk==trk)
			return pe;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ADX_EVENT* CAodixCore::edit_find_next_event(int pat,int pos,int trk,int len)
{
	int pos_last=pos+len;
	ADX_EVENT* found=NULL;

	// scan sequencer events
	for(int e=0;e<seq_num_events;e++)
	{
		// get event pointer
		ADX_EVENT* pe=&seq_event[e];

		// event matches parameters and is earlier than found
		if(pe->pat==pat && pe->pos>=pos && pe->pos<pos_last && pe->trk==trk)
		{
			pos_last=pe->pos;
			found=pe;
		}
	}
	return found;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ADX_EVENT* CAodixCore::edit_find_prev_event(int pat,int pos,int trk,int len)
{
	int pos_first=pos-len;
	ADX_EVENT* found=NULL;

	// scan sequencer events
	for(int e=0;e<seq_num_events;e++)
	{
		// get event pointer
		ADX_EVENT* pe=&seq_event[e];

		// event matches parameters and is later than found
		if(pe->pat==pat && pe->pos<=pos && pe->pos>pos_first && pe->trk==trk)
		{
			pos_first=pe->pos;
			found=pe;
		}
	}
	return found;
}
