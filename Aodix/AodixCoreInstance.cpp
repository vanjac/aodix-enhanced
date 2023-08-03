/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Instance Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef AEffect* (*PVSTMAIN)(audioMasterCallback audioMaster);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::instance_dll(HWND const hwnd,ADX_INSTANCE* pi,char* filename,int const param_x,int const param_y)
{
	// load dll file
	HMODULE	h_dll=LoadLibrary(filename);

	if(h_dll==NULL)
	{
		MessageBox(hwnd,"Error: LoadLibrary() Failed","Aodix - VST Host",MB_OK | MB_ICONERROR);
		return;
	}

	// get proc main address
	PVSTMAIN _pMain = NULL;
	_pMain=(PVSTMAIN)GetProcAddress(h_dll,"main");

	// VST 2.4 SDK adds "VSTPluginMain"
	if (!_pMain)
		_pMain=(PVSTMAIN)GetProcAddress(h_dll,"VSTPluginMain");

	// no plug's main entry function?
	if(!_pMain)
	{
		MessageBox(hwnd,"Error: GetProcAddress() Failed","Aodix - VST Host",MB_OK | MB_ICONERROR);
		return;
	}

	// zero midi queue
	pi->midi_queue_size=0;

	// reset instance routing screen position
	pi->x=param_x;
	pi->y=param_y;

	// reset instance properties
	pi->process_mute=0;
	pi->process_thru=0;

	// reset midi-out pin
	pi->mout_pin.pwire=NULL;
	pi->mout_pin.num_wires=0;

	// set dll path
	sprintf(pi->dll_path,filename);

	//obtain AEffect structure, create effect
	pi->peffect=_pMain(host_audiomaster);

	// no effect created by plug's main
	if(pi->peffect==NULL)
	{
		MessageBox(hwnd,"Error: Plugin's Main() Function Returned NULL Effect Address","Aodix - VST Host",MB_OK | MB_ICONERROR);
		return;
	}

	int const pre_i=pi->peffect->numInputs;
	int const pre_o=pi->peffect->numOutputs;
	int const pre_p=pi->peffect->numParams;

	// create input pins
	if(pi->peffect->numInputs>0)
	{
		// create input pins
		pi->pins=new float*[pi->peffect->numInputs];

		// create input pins buffer
		for(int i=0;i<pi->peffect->numInputs;i++)
			pi->pins[i]=new float[MAX_DSP_BLOCK_SIZE];
	}

	// create output pins
	if(pi->peffect->numOutputs>0)
	{
		// create output pins
		pi->pous=new float*[pi->peffect->numOutputs];

		// create output pins buffer
		for(int o=0;o<pi->peffect->numOutputs;o++)
			pi->pous[o]=new float[MAX_DSP_BLOCK_SIZE];

		// maximum assigned master output pin (at least 1)
		int num_master_output_pins=1;
		for(int oo=0;oo<NUM_DSP_OUTPUTS;oo++)
			if(cfg.asio_output_pin[oo]<asio_num_outputs)
				num_master_output_pins=oo+1;

		// create audio output pin(s) array
		pi->pout_pin=new ADX_PIN[pi->peffect->numOutputs];

		// init pin output array
		for(int o=0;o<pi->peffect->numOutputs;o++)
		{
			// get pin pointer
			ADX_PIN* pp=&pi->pout_pin[o];

			// reset wire array
			pp->pwire=NULL;
			pp->num_wires=0;

			// add wire to master
			if (cfg.instance_autolink)
				edit_add_wire(pp, MASTER_INSTANCE, o%num_master_output_pins, 1.0);
		}
	}

	// check num vst params
	if(pi->peffect->numParams>0)
	{
		// create midi cc array
		pi->pmidi_cc=new unsigned char[pi->peffect->numParams];

		// reset midi cc controllers to 0xFF (255), unassigned
		for(int p=0;p<pi->peffect->numParams;p++)
			pi->pmidi_cc[p]=0xFF;
	}

	// init effect name
	char effect_name[64]; // in VST 2.3 the limit was 32 bytes, in 2.4 it was raised to 64 bytes
	memset(effect_name,0,sizeof(effect_name));
	pi->peffect->dispatcher(pi->peffect,effGetEffectName,0,0,effect_name,0.0f);

	// check if plugin effect name was set
	if(effect_name[0]!=0)
	{
		// copy to alias name and truncate
		memcpy(pi->alias,effect_name,sizeof(pi->alias));
		pi->alias[sizeof(pi->alias) - 1] = 0;
	}
	else
	{
		// set dll title in string
		GetFileTitle(pi->dll_path,pi->alias,24);

		// kick away extension
		pi->alias[strlen(pi->alias)-4]=0;
	}

	// success instance, init plugin 
	pi->peffect->dispatcher(pi->peffect,effOpen,0,0,NULL,0.0f);
	pi->peffect->dispatcher(pi->peffect,effSetSampleRate,0,0,NULL,cfg.asio_driver_sample_rate);
	pi->peffect->dispatcher(pi->peffect,effSetBlockSize,0,dsp_block_size,NULL,0.0f);
	pi->peffect->dispatcher(pi->peffect,effMainsChanged,0,1,NULL,0.0f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::instance_free(ADX_INSTANCE* pi)
{
	// delete vst instance
	if(pi->peffect!=NULL)
	{
		// delete input pins
		if(pi->peffect->numInputs>0)
		{
			// delete input buffers
			for(int i=0;i<pi->peffect->numInputs;i++)
				delete[] pi->pins[i];

			// free pin array
			delete[] pi->pins;
			pi->pins=NULL;
		}

		// delete output pins
		if(pi->peffect->numOutputs>0)
		{
			// scan outputs
			for(int o=0;o<pi->peffect->numOutputs;o++)
			{
				// delete output buffer
				delete[] pi->pous[o];

				// clear output pin
				edit_clr_pin(&pi->pout_pin[o]);
			}

			// free pouts array
			delete[] pi->pous;
			pi->pous=NULL;

			// free output pin array
			delete[] pi->pout_pin;
			pi->pout_pin=NULL;
		}

		// destroy midi cc matrix
		if(pi->peffect->numParams>0)
		{
			delete[] pi->pmidi_cc;
			pi->pmidi_cc=NULL;
		}

		// clear midi-out pin
		edit_clr_pin(&pi->mout_pin);

		// close plugin's editor
		instance_close_editor(pi);

		// close plugin's instance
		pi->peffect->dispatcher(pi->peffect,effMainsChanged,0,0,0,0.0f);
		pi->peffect->dispatcher(pi->peffect,effClose,0,0,0,0.0f);

		// set null vst aeffect plug
		pi->peffect=NULL;

		// reset instance struct members
		pi->hwnd=NULL;
		pi->midi_queue_size=0;
		pi->x=0;
		pi->y=0;
		pi->process_mute=0;
		pi->process_thru=0;

		// set no dll path
		pi->dll_path[0]=0;

		// resest alias
		memset(pi->alias,0,32);
		sprintf(pi->alias,"---");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ADX_INSTANCE* CAodixCore::instance_get_from_effect(AEffect* peffect,int* pindex)
{
	// scan all instances
	for(int i=0;i<MAX_INSTANCES;i++)
	{
		// instance pointer
		ADX_INSTANCE* pi=&instance[i];

		// check if is same instance
		if(pi->peffect==peffect)
		{
			// set index
			*pindex=i;

			// return pointer
			return pi;
		}
	}

	// else
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::instance_open_editor(ADX_INSTANCE* pi,HWND const hwnd)
{
	// close if previously open
	instance_close_editor(pi);

	// open vst gui
	if(hwnd!=NULL && pi->peffect && (pi->peffect->flags & effFlagsHasEditor))
	{
		// window frames
		int const frame_w=6;
		int const frame_h=22;

		// get parent window area size
		RECT r;
		GetClientRect(hwnd,&r);
		int const w=r.right-r.left;
		int const h=r.bottom-r.top;

		// get plugin editor window size
		ERect * er;
		pi->peffect->dispatcher(pi->peffect,effEditGetRect,0,0,&er,0.0f);

		// position window
		int const win_w=er->right-er->left;
		int const win_h=er->bottom-er->top;
		int const win_x=w/2-(win_w+frame_w)/2;
		int const win_y=h/2-(win_h+frame_h)/2;

		// pass screen coords
		POINT p;
		p.x=win_x;
		p.y=win_y;
		ClientToScreen(hwnd,&p);

		// create plugin parent window
		pi->hwnd=CreateWindowEx(WS_EX_TOOLWINDOW,"VSTAodixHost","Aodix - VST Instance Window",WS_SYSMENU | WS_CAPTION,p.x,p.y,win_w+frame_w,win_h+frame_h,hwnd,NULL,hinstance_app,NULL);

		// set window effect parameter
		SetWindowLong(pi->hwnd,GWL_USERDATA,(long)pi->peffect);

		// open editor window
		pi->peffect->dispatcher(pi->peffect,effEditOpen,0,0,pi->hwnd,0.0f);
		pi->peffect->dispatcher(pi->peffect,effEditTop,0,0,NULL,0.0f);

		// check for new size
		pi->peffect->dispatcher(pi->peffect,effEditGetRect,0,0,&er,0.0f);
		int const nw=er->right-er->left;
		int const nh=er->bottom-er->top;

		// misteriously resized(!)
		if(nw!=win_w || nh!=win_h)
		{
			p.x=w/2-(nw+frame_w)/2;
			p.y=h/2-(nh+frame_h)/2;
			ClientToScreen(hwnd,&p);
			SetWindowPos(pi->hwnd,NULL,p.x,p.y,nw+frame_w,nh+frame_h,SWP_NOZORDER);
		}

		// update caption
		host_update_window_caption(pi->peffect);

		// show window
		ShowWindow(pi->hwnd,SW_SHOWNORMAL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::instance_close_editor(ADX_INSTANCE* pi)
{
	// close window if opened
	if(pi->peffect!=NULL && pi->hwnd!=NULL && IsWindow(pi->hwnd))
	{
		// kill child plugin editor window
		if(pi->peffect->flags & effFlagsHasEditor)
			pi->peffect->dispatcher(pi->peffect,effEditClose,0,0,NULL,0.f);

		// close window
		DestroyWindow(pi->hwnd);

		// set null handle
		pi->hwnd=NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::instance_add_midi_event(ADX_INSTANCE* pi,int const track,unsigned char const md0,unsigned char const md1,unsigned char const md2,unsigned char const md3,int const deltaframe)
{
	// check if effect is instanced
	if(pi->peffect!=NULL)
	{
		// ignore incoming events if list is full
		if(pi->midi_queue_size<MAX_BLOCK_EVENTS)
		{
			// track midi vumeter update
			if((md0&0xF0)==0x90)
				master_midi_vumeter[track]=md2;

			// get new event pointer
			VstMidiEvent* pe=&pi->midi_event[pi->midi_queue_size];

			// prepare event
			pe->byteSize=24;
			pe->deltaFrames=deltaframe;
			pe->detune=0;
			pe->flags=0;
			pe->midiData[0]=md0;
			pe->midiData[1]=md1;
			pe->midiData[2]=md2;
			pe->midiData[3]=md3;
			pe->noteLength=0;
			pe->noteOffset=0;
			pe->noteOffVelocity=0;
			pe->reserved1=0;
			pe->reserved2=0;
			pe->type=kVstMidiType;

			// increment queue
			pi->midi_queue_size++;
		}
		else
		{
			// alert of instance process midi buffer overflow
			MessageBox(NULL,"Overflood","Aodix - MIDI Process",MB_OK | MB_ICONERROR);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::instance_midi_panic(ADX_INSTANCE* pi,bool const all_notes_off,bool const all_sounds_off)
{
	// check if effect is instanced
	if(pi->peffect!=NULL)
	{
		// check all notes off flag
		if(all_notes_off)
		{
			// all notes off thru all channels
			for(int c=0;c<16;c++)
				instance_add_midi_event(pi,user_trk,0xB0+c,123,0,0,0);
		}

		// check all sounds off flag
		if(all_sounds_off)
		{
			// all sounds off thru all channels
			for(int c=0;c<16;c++)
				instance_add_midi_event(pi,user_trk,0xB0+c,120,0,0,0); 
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::instance_set_param(ADX_INSTANCE* pi,int const param_index,float const param_value)
{
	// check if effect is instanced and alter parameter
	if(pi->peffect!=NULL)
		pi->peffect->setParameter(pi->peffect,param_index,param_value);
}
