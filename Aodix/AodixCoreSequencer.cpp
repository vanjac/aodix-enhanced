/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Sequencer Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAodixCore::seq_sample_to_pos(int const sample)
{
	// constants
	double const d_points_per_second=project.master_tempo*double(project.master_ppqn)/60.0;
	double const d_samples_per_point=cfg.asio_driver_sample_rate/d_points_per_second;

	// get samples per point integer
	int const i_samples_per_point=int(d_samples_per_point*16.0);

	// convert sample to midi-pos
	return (sample<<4)/i_samples_per_point;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAodixCore::seq_pos_to_sample(int const pos)
{
	// constants
	double const d_points_per_second=project.master_tempo*double(project.master_ppqn)/60.0;
	double const d_samples_per_point=cfg.asio_driver_sample_rate/d_points_per_second;

	// get samples per point integer
	int const i_samples_per_point=int(d_samples_per_point*16.0);

	// convert midi-pos to sample
	return (pos*i_samples_per_point)>>4;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::seq_add_event(int const pos,unsigned char const pat,unsigned char const trk,unsigned char const typ,unsigned char const da0,unsigned char const da1,unsigned char const da2,unsigned char const da3,int const overwrite)
{
	// if overwrite, delete all events at same pattern,position and track	
	if(overwrite)
		seq_delete_events_at(pat,pos,1,trk,1);

	// place new event at end of the vector (if free event bin is available)
	if(seq_num_events<MAX_EVENTS)
	{
		// get new event pointer
		ADX_EVENT* pe=&seq_event[seq_num_events++];

		// set position and duration
		pe->pos=pos;
		pe->par=0;

		// set properties
		pe->pat=pat;
		pe->trk=trk;
		pe->typ=typ;
		pe->szd=(typ<EVT_MID);

		// set data
		pe->da0=da0;
		pe->da1=da1;
		pe->da2=da2;
		pe->da3=da3;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::seq_add_evmid(int const pos,unsigned char const pat,unsigned char const trk,unsigned char const ins,unsigned char const da0,unsigned char const da1,unsigned char const da2,int const overwrite)
{
	// note off flag
	int const is_note_off_a=(da0&0xF0)==0x80;
	int const is_note_off_b=(da0&0xF0)==0x90 && da2==0;

	// midi note off
	if(is_note_off_a || is_note_off_b)
	{
		// scan events
		for(int e=0;e<seq_num_events;e++)
		{
			// get event pointer
			ADX_EVENT* pe=&seq_event[e];

			// type is note on, duration is zero, channel match and key match
			if(pe->typ==EVT_NOT && pe->par==0 && (da0&0xF)==pe->da1 && da1==pe->da2)
				pe->par=arg_tool_clipped_assign(pos-pe->pos,0,MAX_SIGNED_INT);
		}

		return;
	}

	// midi note on
	if((da0&0xF0)==0x90 && da2>0)
	{
		seq_add_event(pos,pat,trk,EVT_NOT,ins,da0&0xF,da1,da2,overwrite);
		return;
	}

	// other midi event type
	seq_add_event(pos,pat,trk,EVT_MID,ins,da0,da1,da2,overwrite);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::seq_delete_event(int const index)
{
	// one less event
	seq_num_events--;

	// pop events
	for(int i=index;i<seq_num_events;i++)
		seq_event[i]=seq_event[i+1];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::seq_delete_events_at(int const pat,int const pos,int const pos_rng,int const trk,int const trk_rng)
{
	// scan all events
	for(int ev=0;ev<seq_num_events;ev++)
	{
		// get event
		ADX_EVENT *pe=&seq_event[ev];

		// overwrite and return if event matches with given parameters
		if(pe->pat==pat && pe->pos>=pos && pe->pos<(pos+pos_rng) && pe->trk>=trk && pe->trk<(trk+trk_rng))
		{
			// kick out the event
			seq_delete_event(ev);

			// decrease the counter (avoid event skip)
			ev--;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::seq_delete_events_at_pattern(int const pat)
{
	// scan all events
	for(int ev=0;ev<seq_num_events;ev++)
	{
		// get event
		ADX_EVENT *pe=&seq_event[ev];

		// overwrite and return if event pattern matches with given pattern
		if(pe->pat==pat)
		{
			// kick out the event
			seq_delete_event(ev);

			// decrease the counter (avoid event skip)
			ev--;
		}
	}
}
