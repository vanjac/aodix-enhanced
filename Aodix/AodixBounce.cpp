/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Bounce Dialog Procedure Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void write_int32(int value,FILE* pfile)
{
	fwrite(&value,sizeof(value),1,pfile);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void write_int16(short value,FILE* pfile)
{
	fwrite(&value,sizeof(value),1,pfile);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void write_wav_header(FILE* pfile,int num_samples,int num_channels,int sample_rate)
{
	long const fmt_chunk_size=16;
	int const data_chunk_size = num_samples * num_channels * sizeof(float);
	int const riff_size = 4 + (8 + fmt_chunk_size) + (8 + data_chunk_size);

	write_int32('FFIR',pfile);
	write_int32(riff_size,pfile);

	write_int32('EVAW',pfile);

	// format chunk
	write_int32(' tmf',pfile);
	write_int32(fmt_chunk_size,pfile);
	write_int16(3,pfile); // format code: IEEE float
	write_int16(num_channels,pfile);
	write_int32(sample_rate,pfile);
	write_int32(sample_rate * num_channels * sizeof(float), pfile); // data rate
	write_int16(num_channels * sizeof(float), pfile); // block size
	write_int16(sizeof(float) * 8, pfile); // bits per sample

	// data chunk
	write_int32('atad',pfile);
	write_int32(data_chunk_size,pfile);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK boun_dlg_proc(HWND hdlg,UINT message,WPARAM wparam,LPARAM lparam)
{
	// extern aodix core pointer
	extern CAodixCore* gl_padx;

	// strings for window caption
	char buf[_MAX_PATH];

	// init dialog
	if(message==WM_INITDIALOG)
	{
		// get control pointers
		HWND const hwnd_edit1=GetDlgItem(hdlg,IDC_BOUNCE_EDIT1);
		HWND const hwnd_prog1=GetDlgItem(hdlg,IDC_BOUNCE_PROGRESS1);
		HWND const hwnd_butt2=GetDlgItem(hdlg,IDC_BOUNCE_BUTTON2);

		// init progress bar control
		SendMessage(hwnd_prog1,PBM_SETBKCOLOR,0,(LPARAM)(COLORREF)0x00404040);
		SendMessage(hwnd_prog1,PBM_SETBARCOLOR,0,(LPARAM)(COLORREF)0x00F0F0F0);

		// disable stop button
		EnableWindow(hwnd_butt2,FALSE);

		// set project name wav file
		sprintf(buf,"%s.wav",gl_padx->project.name);
		SetWindowText(hwnd_edit1,buf);

		// set focus to user name field
		SetFocus(hwnd_edit1);

		return TRUE;
	}

	// command
	if(message==WM_COMMAND)
	{
		// get window controls
		HWND const hwnd_edit1=GetDlgItem(hdlg,IDC_BOUNCE_EDIT1);
		HWND const hwnd_butt1=GetDlgItem(hdlg,IDC_BOUNCE_BUTTON1);
		HWND const hwnd_butt2=GetDlgItem(hdlg,IDC_BOUNCE_BUTTON2);
		HWND const hwnd_butt3=GetDlgItem(hdlg,IDC_BOUNCE_BUTTON3);
		HWND const hwnd_chec1=GetDlgItem(hdlg,IDC_BOUNCE_CHECK1);
		HWND const hwnd_chec2=GetDlgItem(hdlg,IDC_BOUNCE_CHECK2);
		HWND const hwnd_stat1=GetDlgItem(hdlg,IDC_BOUNCE_STATUS1);
		HWND const hwnd_prog1=GetDlgItem(hdlg,IDC_BOUNCE_PROGRESS1);

		// get command id
		int const command_id=LOWORD(wparam);

		// browse file
		if(command_id==IDC_BOUNCE_BUTTON3)
		{
			// filename holder
			sprintf(buf,"%s.wav",gl_padx->project.name);

			// save file dialog
			if(arg_file_dialog_save(gl_padx->hinstance_app,hdlg,"Export WAV File",buf,"WAV Files (32-Bit Float Stereo) (*.wav)\0*.wav\0\0","wav",""))
				SetWindowText(hwnd_edit1,buf);

			return TRUE;
		}

		// start render
		if(command_id==IDC_BOUNCE_BUTTON1)
		{
			// message holder
			MSG msg;

			// get current pattern pointer
			ADX_PATTERN* pp=&gl_padx->project.pattern[gl_padx->user_pat];

			// get check status
			int const check_cue_range=SendMessage(hwnd_chec1,BM_GETSTATE,0,0) & BST_CHECKED;
			int const check_multifile=SendMessage(hwnd_chec2,BM_GETSTATE,0,0) & BST_CHECKED;

			// check for stop marker
			if(check_cue_range==0 && pp->cue_stp<=0)
			{
				SetWindowText(hwnd_stat1,"Requires Stop Marker");
				return TRUE;
			}

			// get file name
			GetWindowText(hwnd_edit1,buf,_MAX_PATH);

			// disable controls
			EnableWindow(hwnd_edit1,FALSE);
			EnableWindow(hwnd_butt1,FALSE);
			EnableWindow(hwnd_butt2,TRUE);
			EnableWindow(hwnd_butt3,FALSE);
			EnableWindow(hwnd_chec1,FALSE);
			EnableWindow(hwnd_chec2,FALSE);

			// focus stop button
			SetFocus(hwnd_butt2);

			// status rendering
			SetWindowText(hwnd_stat1,"Rendering...");

			// file pointers
			FILE* pfiles[NUM_DSP_OUTPUTS];

			// see what master output wires are set
			unsigned char master_output_wired[NUM_DSP_OUTPUTS];

			// zero output wires
			memset(master_output_wired,0,NUM_DSP_OUTPUTS);

			// scan master input module
			for(int p=0;p<NUM_DSP_INPUTS;p++)
			{
				// get pin pointer
				ADX_PIN* pp=&gl_padx->master_input_pin[p];

				// scan wires
				for(int w=0;w<pp->num_wires;w++)
				{
					// get wire pointer
					ADX_WIRE* pw=&pp->pwire[w];

					// check if wire is connected to output master and flag it
					if(pw->instance_index==MASTER_INSTANCE)
						master_output_wired[pw->pin_index]=1;
				}
			}

			// scan instances
			for(int i=0;i<MAX_INSTANCES;i++)
			{
				// get instance pointer
				ADX_INSTANCE* pi=&gl_padx->instance[i];

				// instance set
				if(pi->peffect!=NULL)
				{
					// scan instance output pins
					for(int o=0;o<pi->peffect->numOutputs;o++)
					{
						// get pin pointer
						ADX_PIN* pp=&pi->pout_pin[o];

						// scan wires
						for(int w=0;w<pp->num_wires;w++)
						{
							// get wire pointer
							ADX_WIRE* pw=&pp->pwire[w];

							// check if wire is connected to output master and flag it
							if(pw->instance_index==MASTER_INSTANCE)
								master_output_wired[pw->pin_index]=1;
						}
					}
				}
			}

			// store current pattern cycle state
			int const curr_cycle_state=gl_padx->master_time_info.flags & kVstTransportCycleActive;

			// number of samples to write
			int num_samples=0;

			// rendering to stop marker
			if(check_cue_range==0)
			{
				SendMessage(hwnd_prog1,PBM_SETRANGE32,0,pp->cue_stp);
				num_samples=gl_padx->seq_pos_to_sample(pp->cue_stp);
				pp->usr_pos=0;
				gl_padx->master_transport_sampleframe=0;
				gl_padx->gui_is_dirty=1;
			}

			// rendering cue range
			if(check_cue_range==1)
			{
				int end = min(pp->cue_end,pp->cue_stp);
				SendMessage(hwnd_prog1,PBM_SETRANGE32,pp->cue_sta,end);
				num_samples=gl_padx->seq_pos_to_sample(end - pp->cue_sta);
				pp->usr_pos=pp->cue_sta;
				gl_padx->master_transport_sampleframe=gl_padx->seq_pos_to_sample(pp->cue_sta);
				gl_padx->gui_is_dirty=1;
			}

			// check rendering method
			if(check_multifile==1)
			{
				// multifile rendering
				for(int fi=0;fi<NUM_DSP_OUTPUTS;fi++)
				{
					// check if output is wired
					if(master_output_wired[fi])
					{
						// format proper file name
						char track_file[_MAX_PATH];
						sprintf(track_file,"%s.%.2d",buf,fi);

						// open file(s)
						pfiles[fi]=fopen(track_file,"wb");
						write_wav_header(pfiles[fi], num_samples, 1, gl_padx->cfg.asio_driver_sample_rate);
					}
				}
			}
			else
			{
				int num_channels = 0;
				for(int o=0;o<NUM_DSP_OUTPUTS;o++)
					if(master_output_wired[o])
						num_channels++;

				// single interleaved file
				pfiles[0]=fopen(buf,"wb");
				write_wav_header(pfiles[0], num_samples, num_channels, gl_padx->cfg.asio_driver_sample_rate);
			}

			// disable transport cycle, launch transport
			gl_padx->master_time_info.flags&=~kVstTransportCycleActive;
			gl_padx->master_time_info.flags|=kVstTransportPlaying;

			int cur_sample=0;

			// render while transport dont wrap
			while(cur_sample<num_samples)
			{
				// clear dsp input buffers
				gl_padx->dsp_clear_input_buffers();

				// work render
				gl_padx->dsp_work();

				// write out to file stream
				for(int s=0;s<gl_padx->dsp_block_size && cur_sample<num_samples;s++,cur_sample++)
				{
					// write interleaved samples of each output
					for(int o=0;o<NUM_DSP_OUTPUTS;o++)
					{
						// write only wired channels
						if(master_output_wired[o])
							fwrite(&gl_padx->dsp_output_buffer[o][s],sizeof(float),1,pfiles[check_multifile*o]);
					}
				}

				// get sequencer position
				int const c_s_pos=gl_padx->seq_sample_to_pos(gl_padx->master_transport_sampleframe);

				// set progress
				SendMessage(hwnd_prog1,PBM_SETPOS,c_s_pos,0);

				// server pending mesages (only paint and timer)
				while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
					DispatchMessage(&msg);
			}

			// stop transport
			gl_padx->master_time_info.flags&=~kVstTransportPlaying;

			// finalize file(s)
			if(check_multifile==1)
			{
				// close multiple files
				for(int fi=0;fi<NUM_DSP_OUTPUTS;fi++)
				{
					// check if output is wired
					if(master_output_wired[fi])
						fclose(pfiles[fi]);
				}
			}
			else
			{
				// close single file
				fclose(pfiles[0]);
			}

			// stop transport
			gl_padx->master_time_info.flags&=~kVstTransportPlaying;

			// restore transport cycle status
			if(curr_cycle_state)
				gl_padx->master_time_info.flags|=kVstTransportCycleActive;

			// send all notes off/all sounds off
			for(int i=0;i<MAX_INSTANCES;i++)
				gl_padx->instance_midi_panic(&gl_padx->instance[i],true,true);

			// re-enable controls
			EnableWindow(hwnd_edit1,TRUE);
			EnableWindow(hwnd_butt1,TRUE);
			EnableWindow(hwnd_butt2,FALSE);
			EnableWindow(hwnd_butt3,TRUE);
			EnableWindow(hwnd_chec1,TRUE);
			EnableWindow(hwnd_chec2,TRUE);

			// focus file name box
			SetFocus(hwnd_edit1);

			// finish rendering
			SetWindowText(hwnd_stat1,"Render Finished");

			// post refresh
			gl_padx->gui_is_dirty=1;

			return TRUE;
		}

		// stop transport
		if(command_id==IDC_BOUNCE_BUTTON2)
		{
			gl_padx->master_time_info.flags&=~kVstTransportPlaying;

			return TRUE;
		}

		// end dialog
		if(command_id==IDOK || command_id==IDCANCEL)
		{
			// if player is running, stop else end dialog
			if(gl_padx->master_time_info.flags & kVstTransportPlaying)
				gl_padx->master_time_info.flags&=~kVstTransportPlaying;
			else
				EndDialog(hdlg,LOWORD(wparam));

			return TRUE;
		}
	}

	// not dispatched message
	return FALSE;
}