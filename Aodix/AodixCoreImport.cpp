/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Import Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::import_adx_file_dlg(HWND const hwnd)
{
	// filename holder
	char filename[_MAX_PATH];
	sprintf(filename,"*.adx");

	// open file dialog
	if(arg_file_dialog_open(hinstance_app,hwnd,"Open Aodix Project File",filename,"Aodix Project File (*.adx)\0*.adx\0\0","adx","",1,0))
	{
		// set wait cursor
		SetCursor(hcursor_wait);

		// enter critical section
		asio_enter_cs();

		// import adx file
		import_adx_file(hwnd,filename);

		// leave critical section
		asio_leave_cs();

		// post refresh
		gui_is_dirty=1;

		// set arrow cursor
		SetCursor(hcursor_arro);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::import_adx_pin(ADX_PIN* pin_array,int num_pins,FILE* pfile)
{
	// read pin data
	for(int p=0;p<num_pins;p++)
	{
		// get pin pointer
		ADX_PIN* pp=&pin_array[p];

		// read num wires
		fread(&pp->num_wires,sizeof(int),1,pfile);

		// read wire data
		if (pp->num_wires > 0)
		{
			pp->pwire=new ADX_WIRE[pp->num_wires];
			fread(pp->pwire,sizeof(ADX_WIRE),pp->num_wires,pfile);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::import_adx_file(HWND const hwnd,char* filename)
{
	// reset master position
	master_transport_sampleframe=0;

	// reset user and project
	reset_user();
	reset_project();

	// open file
	FILE* pfile=fopen(filename,"rb");

	if(pfile!=NULL)
	{
		// read header id
		long header_id=0;
		fread(&header_id,sizeof(long),1,pfile);

		// compare header id
		if(header_id!='ADX4')
		{
			// close file, not adx4 fileformat and return
			fclose(pfile);
			return;
		}

		// update last file opened
		sprintf(user_last_file,filename);

		// read version
		long project_version=0;
		fread(&project_version,sizeof(int),1,pfile);

		// compare header project version
		if(project_version<4010)
		{
			// unsupported old format, close file, return
			fclose(pfile);
			return;
		}

		// update recent files
		recent_files_push(filename);

		// read song properties
		fread(&project,sizeof(ADX_PROJECT),1,pfile);

		// read num events
		fread(&seq_num_events,sizeof(int),1,pfile);

		// read events
		fread(seq_event,sizeof(ADX_EVENT),seq_num_events,pfile);

		// update undo
		edit_undo_snapshot();
		edit_undo_combine_next();

		// read instance data
		for(int i=0;i<MAX_INSTANCES;i++)
		{
			// get instance pointer
			ADX_INSTANCE* pi=&instance[i];

			// get instance flag
			int const instance_flag=fgetc(pfile);

			// reset instance eff id currently loading
			instance_eff_id_currently_loading=0;

			// read instance data if plugin is instanced
			if(instance_flag)
			{
				// read instance path
				fread(pi->dll_path,sizeof(char),_MAX_PATH,pfile);

				// localize dll file
				if(import_localize_vst_dll(hwnd,i,pi->dll_path)==0)
				{
					// close file, dll not found
					fclose(pfile);
					MessageBox(hwnd,"Project loading stopped. Not all instances were loaded.","Aodix - Import ADX",MB_OK | MB_ICONERROR);
					return;
				}

				// read effect id
				fread(&instance_eff_id_currently_loading,sizeof(long),1,pfile);

				// instance plugin
				instance_dll(hwnd,pi,pi->dll_path,0,0);

				// read instance coordinates in dsp routing
				fread(&pi->x,sizeof(int),1,pfile);
				fread(&pi->y,sizeof(int),1,pfile);

				// read instance properties
				fread(&pi->process_mute,sizeof(unsigned char),1,pfile);
				fread(&pi->process_thru,sizeof(unsigned char),1,pfile);

				// read instance label alias
				fread(pi->alias,sizeof(char),32,pfile);

				// read instance output pin data
				import_adx_pin(pi->pout_pin,pi->peffect->numOutputs,pfile);

				// read instance midi output pin data
				import_adx_pin(&pi->mout_pin,1,pfile);

				// current program index holder
				int curr_prg_index=0;

				// read current selected program
				fread(&curr_prg_index,sizeof(int),1,pfile);

				// read instance chunk
				if(pi->peffect->flags & effFlagsProgramChunks)
				{
					// read chunk bytesize
					int chk_byte_size=0;
					fread(&chk_byte_size,sizeof(int),1,pfile);

					// bank chunk data pointer
					void* pchkdata=malloc(chk_byte_size);

					// read chunk data
					fread(pchkdata,chk_byte_size,1,pfile);

					// dispatch
					pi->peffect->dispatcher(pi->peffect,effSetChunk,0,chk_byte_size,pchkdata,0.0f);

					// free
					free(pchkdata);
				}
				else
				{
					// get safe number of programs
					int const safe_num_programs=min(pi->peffect->numPrograms,1);

					// program / param scanning
					for(int p=0;p<safe_num_programs;p++)
					{
						// set program
						pi->peffect->dispatcher(pi->peffect,effSetProgram,0,p,NULL,0.0f);

						// program name buffer
						char buf_prg[32];
						buf_prg[0]=0;

						// read program name
						fread(buf_prg,sizeof(char),32,pfile);

						// set program name
						pi->peffect->dispatcher(pi->peffect,effSetProgramName,0,0,buf_prg,0.0f);

						// read all parameters
						for(int pa=0;pa<pi->peffect->numParams;pa++)
						{
							float pa_value=0.0f;
							fread(&pa_value,sizeof(float),1,pfile);
							pi->peffect->setParameter(pi->peffect,pa,pa_value);
						}
					}
				}

				// read midi cc array
				if(pi->peffect->numParams)
					fread(pi->pmidi_cc,sizeof(unsigned char),pi->peffect->numParams,pfile);

				// set current program
				pi->peffect->dispatcher(pi->peffect,effSetProgram,0,curr_prg_index,NULL,0.0f);
			}
		}

		// read master input pin data
		import_adx_pin(master_input_pin,NUM_DSP_INPUTS,pfile);

		// read master i/o module screen coordinates
		fread(&master_i_x,sizeof(int),1,pfile);
		fread(&master_i_y,sizeof(int),1,pfile);
		fread(&master_o_x,sizeof(int),1,pfile);
		fread(&master_o_y,sizeof(int),1,pfile);

		// read user routing view offset
		fread(&user_rout_offset_x,sizeof(int),1,pfile);
		fread(&user_rout_offset_y,sizeof(int),1,pfile);

		// close file
		fclose(pfile);
	}

	// reset instance eff id currently loading
	instance_eff_id_currently_loading=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::import_search_file(char* pfolder,char* pfilename,char* pmatchfile) 
{   
	// set find path
	char findpath[_MAX_PATH];
	sprintf(findpath,"%s\\*.*",pfolder);

	// data variables
	WIN32_FIND_DATA ffd;
	HANDLE hFind=FindFirstFile(findpath,&ffd);

	// no files found
	if(hFind==INVALID_HANDLE_VALUE) 
		return;

	// search loop
	do
	{
		// check no dots
		if(ffd.cFileName[0]!='.')
		{
			if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// configure subpath string
				char subfindpath[_MAX_PATH];
				sprintf(subfindpath,"%s\\%s",pfolder,ffd.cFileName);

				// subsearch file folder (recursive call)
				import_search_file(subfindpath,pfilename,pmatchfile);
			}
			else
			{
				// file found, check for match
				if(_stricmp(ffd.cFileName,pfilename)==0)
					sprintf(pmatchfile,"%s\\%s",pfolder,ffd.cFileName);
			}
		}
	}
	while(FindNextFile(hFind,&ffd));

	// close find
	FindClose(hFind);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAodixCore::import_localize_vst_dll(HWND const hwnd,int index,char* pfilename)
{
	// check if current path exist
	FILE* pf=fopen(pfilename,"rb");

	// success file opening (file existing in passed string)
	if(pf!=NULL)
	{
		fclose(pf);
		return 1;
	}

	// file doesnt exist in current path, extract filetitle
	char filetitle[_MAX_PATH];
	filetitle[0]=0;
	GetFileTitle(pfilename,filetitle,_MAX_PATH);

	// execute search in vst path folder
	char matchfile[_MAX_PATH];
	matchfile[0]=0;

	// scan vst path folders
	for(int pi=0;pi<MAX_VST_FOLDERS;pi++)
	{
		// check that is not empty path
		if(cfg.vst_path[pi][0]!=0)
			import_search_file(cfg.vst_path[pi],filetitle,matchfile);
	}

	// check for match file existance
	if(matchfile[0]!=0)
	{
		// store match filepath in filename pointer
		sprintf(pfilename,matchfile);

		// relocation success
		return 1;
	}
	else
	{
		// format failed plugin location
		char buf[256];
		sprintf(buf,"Unable to locate Instance %.2X Plugin '%s'\nWould you like to browse for it?",index,filetitle);

		// verbose failed relocation
		if(MessageBox(hwnd,buf,"Aodix - VST Host",MB_YESNO | MB_ICONWARNING)==IDYES)
		{
			if(arg_file_dialog_open(hinstance_app,hwnd,"Choose VST Plugin",pfilename,"DLL Files (*.dll)\0*.dll\0\0","dll",cfg.vst_path[0],1,0))
			{
				if(pf=fopen(pfilename,"rb"))
				{
					fclose(pf);
					return 1;
				}
			}
		}
	}

	// failed location search
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long CAodixCore::import_reverse_long(long const data)
{
	return ((data&0xFF)<<24) | ((data&0xFF00)<<8) | ((data&0xFF0000)>>8) | ((data&0xFF000000)>>24);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::import_cub_file_dlg(HWND const hwnd)
{
	// filename holder
	char filename[_MAX_PATH];
	filename[0]=0;

	// open file dialog
	if(arg_file_dialog_open(hinstance_app,hwnd,"Import Cubase Preset(s) File",filename,"FXP Program Files (*.fxp)\0*.fxp\0FXB Bank Files (*.fxb)\0*.fxb\0All Preset(s) Files (*.fxp,*.fxb)\0*.fxp;*.fxb\0\0","","",3,0))
	{
		// set wait cursor
		SetCursor(hcursor_wait);

		// enter critical section
		asio_enter_cs();

		// import cubase preset file
		import_cub_file(hwnd,filename);

		// leave critical section
		asio_leave_cs();

		// post refresh
		gui_is_dirty=1;

		// set arrow cursor
		SetCursor(hcursor_arro);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::import_cub_file(HWND const hwnd,char* filename)
{
	ADX_INSTANCE* pi=&instance[user_instance];

	// effect instanced
	if(pi->peffect!=NULL)
	{
		// open file
		FILE* pfile=fopen(filename,"rb");

		// check file opening
		if(pfile!=NULL)
		{
			// read chunk magic 'CcnK'
			long chunk_magic=0;
			fread(&chunk_magic,sizeof(long),1,pfile);
			chunk_magic=import_reverse_long(chunk_magic);

			// compare chunk magic id
			if(chunk_magic!='CcnK')
			{
				// verbose magic mismatch
				MessageBox(hwnd,"Error: Not Valid Cubase Preset(s) File","Aodix - Import Cubase Preset(s)",MB_OK | MB_ICONERROR);

				// close file and return
				fclose(pfile);
				return;
			}

			// read bytesize of this chunk, excl. magic + byteSize
			long byte_size=0;
			fread(&byte_size,sizeof(long),1,pfile);
			byte_size=import_reverse_long(byte_size);

			// read chunk type identifier
			long fx_magic=0;
			fread(&fx_magic,sizeof(long),1,pfile);
			fx_magic=import_reverse_long(fx_magic);

			// read version
			long version=0;
			fread(&version,sizeof(long),1,pfile);
			version=import_reverse_long(version);

			// read fx unique id
			long fx_id=0;
			fread(&fx_id,sizeof(long),1,pfile);
			fx_id=import_reverse_long(fx_id);

			// compare fx unique id
			if(fx_id!=pi->peffect->uniqueID)
			{
				// verbose id mismatch
				MessageBox(hwnd,"Error: UniqueID Mismatch","Aodix - Import Cubase Bank (FXB)",MB_OK | MB_ICONERROR);

				// close file and return
				fclose(pfile);
				return;
			}

			// read fx version
			long fx_version=0;
			fread(&fx_version,sizeof(long),1,pfile);
			fx_version=import_reverse_long(fx_version);

			// read for preset (program) (.fxp) without chunk (magic = 'FxCk')
			if(fx_magic=='FxCk')
			{
				// read num params
				long num_params=0;
				fread(&num_params,sizeof(long),1,pfile);
				num_params=import_reverse_long(num_params);

				// read program name
				char prg_name[28];
				fread(prg_name,28,1,pfile);
				pi->peffect->dispatcher(pi->peffect,effSetProgramName,0,0,prg_name,0.0f);

				// read params
				for(int pa=0;pa<num_params;pa++)
				{
					long i_param_value=0;
					fread(&i_param_value,sizeof(long),1,pfile);
					i_param_value=import_reverse_long(i_param_value);
					float* pfloatvalue=(float*)(&i_param_value);
					pi->peffect->setParameter(pi->peffect,pa,*pfloatvalue);
				}

				// close file
				fclose(pfile);
				return;
			}

			// read for preset (program) (.fxp) with chunk (magic = 'FPCh')
			if(fx_magic=='FPCh')
			{
				// read num programs
				long num_programs=0;
				fread(&num_programs,sizeof(long),1,pfile);
				num_programs=import_reverse_long(num_programs);

				// read program name
				char prg_name[28];
				fread(prg_name,28,1,pfile);
				pi->peffect->dispatcher(pi->peffect,effSetProgramName,0,0,prg_name,0.0f);

				// read chunk size
				long chunk_size=0;
				fread(&chunk_size,sizeof(long),1,pfile);
				chunk_size=import_reverse_long(chunk_size);

				// alloc chunk memory
				void* pchkdata=malloc(chunk_size);

				// read chunk data
				fread(pchkdata,1,chunk_size,pfile);

				// set chunk dispatch (preset)
				pi->peffect->dispatcher(pi->peffect,effSetChunk,1,chunk_size,pchkdata,0.0f);

				// free chunk memory
				free(pchkdata);

				// close file
				fclose(pfile);
				return;
			}

			// read for bank (.fxb) without chunk (magic = 'FxBk')
			if(fx_magic=='FxBk')
			{
				// read num programs
				long num_programs=0;
				fread(&num_programs,sizeof(long),1,pfile);
				num_programs=import_reverse_long(num_programs);

				// read future
				char future[128];
				fread(future,128,1,pfile);

				// retrieve current program
				int const curr_prg_index=pi->peffect->dispatcher(pi->peffect,effGetProgram,0,0,NULL,0.0f);

				// read programs
				for(int p=0;p<num_programs;p++)
				{
					// set program
					pi->peffect->dispatcher(pi->peffect,effSetProgram,0,p,NULL,0.0f);

					// skip 24 bytes
					fseek(pfile,24,SEEK_CUR);

					// read num params
					long num_params=0;
					fread(&num_params,sizeof(long),1,pfile);
					num_params=import_reverse_long(num_params);

					// read program name
					char prg_name[28];
					fread(prg_name,28,1,pfile);
					pi->peffect->dispatcher(pi->peffect,effSetProgramName,0,0,prg_name,0.0f);

					// read params
					for(int pa=0;pa<num_params;pa++)
					{
						long i_param_value=0;
						fread(&i_param_value,sizeof(long),1,pfile);
						i_param_value=import_reverse_long(i_param_value);
						float* pfloatvalue=(float*)(&i_param_value);
						pi->peffect->setParameter(pi->peffect,pa,*pfloatvalue);
					}
				}

				// set current program
				pi->peffect->dispatcher(pi->peffect,effSetProgram,0,curr_prg_index,NULL,0.0f);

				// close file
				fclose(pfile);
				return;
			}

			// read for bank (.fxb) with chunk (magic = 'FBCh')
			if(fx_magic=='FBCh')
			{
				// read num programs
				long num_programs=0;
				fread(&num_programs,sizeof(long),1,pfile);
				num_programs=import_reverse_long(num_programs);

				// read future
				char future[128];
				fread(future,128,1,pfile);

				// read chunk size
				long chunk_size=0;
				fread(&chunk_size,sizeof(long),1,pfile);
				chunk_size=import_reverse_long(chunk_size);

				// alloc chunk memory
				void* pchkdata=malloc(chunk_size);

				// read chunk data
				fread(pchkdata,1,chunk_size,pfile);

				// set chunk dispatch (bank)
				pi->peffect->dispatcher(pi->peffect,effSetChunk,0,chunk_size,pchkdata,0.0f);

				// free chunk memory
				free(pchkdata);

				// close file
				fclose(pfile);
				return;
			}

			// verbose import failed
			MessageBox(hwnd,"Error: Unknown fxMagic Chunk","Aodix - Import Preset(s) File",MB_OK | MB_ICONERROR);

			// close file
			fclose(pfile);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::import_midi_file_dlg(HWND const hwnd)
{
	// filename holder
	char filename[_MAX_PATH];
	sprintf(filename,"*.mid");

	// channel spare flag
	int ch_spare=0;

	// open file dialog
	if(arg_file_dialog_open(hinstance_app,hwnd,"Import MIDI File",filename,"Standard MIDI Files (*.mid)\0*.mid\0\0","mid","",1,0))
	{
		// ask for mode
		if(MessageBox(hwnd,"Spread MIDI Channels Across Multiple Instances?","Aodix - Midi Import",MB_YESNO | MB_ICONQUESTION)==IDYES)
			ch_spare=1;

		// set wait cursor
		SetCursor(hcursor_wait);

		// enter critical section
		asio_enter_cs();

		// import midi file
		import_midi_file(hwnd,filename);

		// point events to current selected instance
		for(int e=0;e<seq_num_events;e++)
		{
			// get sequencer event pointer
			ADX_EVENT* pe=&seq_event[e];

			// check if it's in pattern
			if(pe->pat==user_pat && pe->typ!=5)
			{
				// spare across channels or use one instance only
				if(ch_spare)
				{
					pe->da0=user_instance+(pe->da1&0xF);
					pe->da1=pe->da1&0xF0;
				}
				else
				{
					pe->da0=user_instance;
				}
			}
		}

		// leave critical section
		asio_leave_cs();

		// post refresh
		gui_is_dirty=1;

		// set arrow cursor
		SetCursor(hcursor_arro);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::import_midi_file(HWND const hwnd,char* filename)
{
	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// set transport at begin
	master_transport_sampleframe=0;

	// open file
	FILE* pfile=fopen(filename,"rb");

	// return 0 if file cant be opened
	if(pfile!=NULL)
	{
		// buffer for chunk id input
		char kw_buff[4];

		// read in the first four characters into 'kw_buff'
		fread(kw_buff,sizeof(char),4,pfile);

		// see if 'kw_buff' contains "MThd"
		if(strncmp(kw_buff,"MThd",4)!=0)
		{
			// bad header id
			fclose(pfile);
			return;
		}

		// read the number 6 (header length) from the header
		unsigned long const header_len=import_midi_read_long(pfile);

		// check for correct length
		if(header_len!=6)
		{
			// bad header length
			fclose(pfile);
			return;
		}

		// clear events in current pattern
		seq_delete_events_at_pattern(user_pat);

		// reset pattern
		reset_pattern(&project.pattern[user_pat]);

		// read the file format
		unsigned short const format=import_midi_read_short(pfile);

		// read num tracks
		unsigned short const ntrks=import_midi_read_short(pfile);

		// set new master ppqn
		project.master_ppqn=import_midi_read_short(pfile);

		// set default user edit zoom and note pre-release
		pp->usr_ppp=arg_tool_clipped_assign(project.master_ppqn/64,1,MAX_SIGNED_INT);
		pp->usr_pre=project.master_ppqn/16;

		// running status
		unsigned char running_stat=0;

		// read tracks
		for(int ti=0;ti<ntrks;ti++)
		{
			// reset 'Mf_currtime'. If the file format is 2, then it probably
			// doesn't matter anyway, but for the other types (single and
			// multiple tracks), 'Mf_currtime' should measure the time elapsed
			// since the beginning of the track.
			long midi_currtime=0L;

			// indicate that 'mf_eot()' hasn't been called yet
			int eot_called=0;

			// read in the first four characters into 'kw_buff'
			fread(kw_buff,4,1,pfile);

			// see if 'kw_buff' contains "MTrk"
			if(strncmp(kw_buff,"MTrk",4)!=0)
			{
				// bad track id
				fclose(pfile);
				return;
			}

			// read the length of the track
			long tklen=import_midi_read_long(pfile);

			// read the data in the track
			while(tklen)
			{
				// get current file position
				long const curr_pos=ftell(pfile);

				// read midi event
				// read delta-time (time since last event)
				long const delta_time=import_midi_read_var_len(pfile);

				// update 'mf_currtime' (increment with delta)
				midi_currtime+=delta_time;

				// read event type
				int const stat=fgetc(pfile);

				// parameter being currently read
				int cur_param;

				// parameters
				unsigned char params[2];

				// is it a new event type?
				if(stat&0x80)
				{
					// set new running status
					running_stat=(unsigned char)stat;

					// start reading at 0th param
					cur_param=0;
				}
				else
				{
					// record 1st parameter
					params[0]=(unsigned char)stat;

					// start reading at 1st param
					cur_param=1;			
				}

				// num params
				int num_params=0;

				// sysex or meta
				if(running_stat>=0xF0)
				{
					num_params=-1;
				}
				else
				{
					// parameters
					if(running_stat<0xC0 || running_stat>=0xE0)
						num_params=2;
					else
						num_params=1;
				}

				// read the parameters corresponding to the status byte
				for(int i=num_params-cur_param;i>0;i--,cur_param++)
				{
					// input character
					int const c=fgetc(pfile);

					// record parameter
					params[cur_param]=(unsigned char)c;
				}

				// calculate aodix track index
				int const i_track=ti*2;

				// parse note off event
				if((running_stat&0xF0)==0x80 && user_midi_mask&0x1)
					seq_add_evmid(midi_currtime,user_pat,i_track,0,running_stat,params[0],params[1],0);

				// parse note on event
				if((running_stat&0xF0)==0x90 && user_midi_mask&0x2)
					seq_add_evmid(midi_currtime,user_pat,i_track,0,running_stat,params[0],params[1],0);

				// parse polyphonic aftertouch event
				if((running_stat&0xF0)==0xA0 && user_midi_mask&0x4)
					seq_add_event(midi_currtime,user_pat,i_track+1,3,0,running_stat,params[0],params[1],0);

				// parse controller change event
				if((running_stat&0xF0)==0xB0 && user_midi_mask&0x8)
					seq_add_event(midi_currtime,user_pat,i_track+1,3,0,running_stat,params[0],params[1],0);

				// parse program change event
				if((running_stat&0xF0)==0xC0 && user_midi_mask&0x10)
					seq_add_event(midi_currtime,user_pat,i_track+1,3,0,running_stat,params[0],0,0);

				// parse channel pressure event
				if((running_stat&0xF0)==0xD0 && user_midi_mask&0x20)
					seq_add_event(midi_currtime,user_pat,i_track+1,3,0,running_stat,params[0],params[1],0);

				// parse pitchbend change event
				if((running_stat&0xF0)==0xE0 && user_midi_mask&0x40)
					seq_add_event(midi_currtime,user_pat,i_track+1,3,0,running_stat,params[0],params[1],0);

				// parse system exclusive event
				if((running_stat&0xF0)==0xF0)
					import_midi_on_sysex(pfile,user_pat,i_track,midi_currtime,running_stat);

				// get readed bytes
				long const read_bytes=ftell(pfile)-curr_pos;

				// decrement track remaining bytes
				tklen-=read_bytes;
			}
		}

		// close file and return success
		fclose(pfile);

		// get midi file title and store in current pattern name
		char _fpath[_MAX_PATH];
		GetFileTitle(filename,_fpath,_MAX_PATH);
		_fpath[strlen(_fpath)-4]=0;
		_fpath[12]=0;
		sprintf(pp->name,_fpath);
	}
	else
	{
		// verbose failed
		MessageBox(hwnd,"Error: Operation Failed","Aodix - MIDI Import",MB_OK | MB_ICONERROR);	
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long CAodixCore::import_midi_read_var_len(FILE* pfile)
{
	// get value
	long value=fgetc(pfile);

	// check quantity length
	if(value&0x80)
	{
		value&=0x7f;

		int c;

		do
		{
			value=(value<<7)+((c=fgetc(pfile))&0x7f);
		}
		while(c&0x80);
	}

	return value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long CAodixCore::import_midi_read_long(FILE* pfile)
{
	// temporal
	unsigned long retval;
	int i;
	int c;

	// set retval to 0 initially
	retval=0L;

	// read in 4 bytes (long) into retval
	for (i=0;i<4;i++)
	{
		c=fgetc(pfile);
		retval<<=8;
		retval|=(unsigned char)c;
	}

	// all went well
	return retval;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAodixCore::import_midi_read_short(FILE* pfile)
{
	// temporal
	int r,i,c;

	// set retval to 0 initially
	r=0;

	// read in 2 bytes (short) into 'retval'
	for(i=0;i<2;i++)
	{
		c=fgetc(pfile);
		r<<=8;
		r|=(unsigned char)c;
	}

	// all went well
	return r;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::import_midi_on_sysex(FILE* pfile,int const pattern_index,int const track,int const i_event_pos,unsigned char const stat)
{
	// sysex event
	if(stat==0xF0)
	{
		// read length of meta-event
		long const length=import_midi_read_var_len(pfile);

		if(length>0)
		{
			// allocate memory for sysex-event
			unsigned char* data=new unsigned char[length];

			// read sysex
			fread(data,sizeof(unsigned char),length,pfile);

			// free memory
			delete[] data;
		}
	}

	// meta message
	if(stat==0xFF)
	{
		// type of meta-event
		int const msg_type=fgetc(pfile);

		// read length of meta-event
		long const length=import_midi_read_var_len(pfile);

		// check meta message size
		if(length>0)
		{
			// allocate memory for meta-event
			unsigned char* data=new unsigned char[length];

			// read meta event data
			fread(data,sizeof(unsigned char),length,pfile);

			// meta sequence name
			if(msg_type==0x03)
			{
				// get clipped length
				int const l=arg_tool_clipped_assign(length,0,12);

				// track name
				for(int i=0;i<l;i++)
					project.pattern[pattern_index].track[track].name[i]=data[i];

				// write zero
				project.pattern[pattern_index].track[track].name[l]=0;
			}

			// meta tempo change
			if(msg_type==0x51)
			{
				// get tempo
				double const d_tempo=256.0*double(60000000.0/double((data[0]<<16) | (data[1]<<8) | (data[2])));

				// get integral tempo
				int const int_tempo=int(d_tempo);	

				// get tempo most and less significant amount
				int const i_ms_tempo=(int_tempo>>8)&0xFF;
				int const i_ls_tempo=int_tempo&0xFF;

				// add tempo event
				seq_add_event(i_event_pos,pattern_index,track,5,i_ms_tempo,i_ls_tempo,0,0,0);
			}

			// free memory
			delete[] data;
		}
	}
}
