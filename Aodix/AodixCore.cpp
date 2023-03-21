/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAodixCore::CAodixCore(HINSTANCE const hinstance)
{
	// init master time info flags
	master_time_info.flags=kVstPpqPosValid | kVstTempoValid | kVstBarsValid | kVstTimeSigValid | kVstCyclePosValid;

	// set default blocksize
	dsp_block_size=MAX_DSP_BLOCK_SIZE;

	// init text action vars
	hwnd_edit=NULL;

	// aodix version
	aodix_version=4201;

	// id of effect currently loading
	instance_eff_id_currently_loading=0;

	// performance counter, top output pin count reset
	dsp_cpu_cost=0.0;
	dsp_highest_opin_count=3;

	// get system cycles per second
	d_system_clocks_per_sec=arg_sys_get_cpu_speed();

	// assign instance
	hinstance_app=hinstance;

	// get application executable path
	GetModuleFileName((HMODULE)hinstance,app_dir,_MAX_PATH);

	// get application path string length
	int const adl=strlen(app_dir);

	// extract dir
	for(int i=adl;i>0;i--)
	{
		if(app_dir[i]=='\\')
		{
			app_dir[i]=0;
			i=0;
		}
	}

	// set html help file path
	sprintf(hlp_fil,"%s\\aodix.chm",app_dir);

	// read configuration
	config_read();

	// reset sequencer data
	seq_num_events=0;

	// reset clipboard data
	clipboard_num_events=0;
	clipboard_pos_sta=0;
	clipboard_trk_sta=0;
	clipboard_pos_len=0;
	clipboard_trk_len=0;

	// reset undo data
	undo_num_events=0;
	undo_combine=false;

	// reset gui flags
	gui_is_dirty=0;

	// reset master
	master_transport_sampleframe=0;
	master_transport_last_y=0;

	// set all instances to null state during initialization
	for(i=0;i<MAX_INSTANCES;i++)
	{
		// get instance pointer
		ADX_INSTANCE* pi=&instance[i];

		// initialize instance struct
		pi->hwnd=NULL;
		pi->peffect=NULL;
		pi->pins=NULL;
		pi->pous=NULL;
		pi->pout_pin=NULL;
		pi->midi_queue_size=0;
		pi->x=0;
		pi->y=0;
		pi->process_mute=0;
		pi->process_thru=0;
		pi->dll_path[0]=0;

		// clear instance alias
		memset(pi->alias,0,32);
		sprintf(pi->alias,"---");
	}

	// initalize master input pin array
	for(i=0;i<NUM_DSP_INPUTS;i++)
	{
		master_input_pin[i].pwire=NULL;
		master_input_pin[i].num_wires=0;
	}

	// reset user and project
	reset_user();
	reset_project();

	// create ppqn menu
	hmenu_ppqn=CreatePopupMenu();
	arg_menu_add_item(hmenu_ppqn,"48",16384);
	arg_menu_add_item(hmenu_ppqn,"60",16385);
	arg_menu_add_item(hmenu_ppqn,"96",16386);
	arg_menu_add_item(hmenu_ppqn,"120",16387);
	arg_menu_add_item(hmenu_ppqn,"192",16388);
	arg_menu_add_item(hmenu_ppqn,"240",16389);
	arg_menu_add_item(hmenu_ppqn,"384",16390);
	arg_menu_add_item(hmenu_ppqn,"480",16391);
	arg_menu_add_item(hmenu_ppqn,"768",16392);
	arg_menu_add_item(hmenu_ppqn,"960",16393);

	// create time signature menu
	hmenu_timesig=CreatePopupMenu();
	arg_menu_add_item(hmenu_timesig,"3:4",16394);
	arg_menu_add_item(hmenu_timesig,"4:4",16395);
	arg_menu_add_item(hmenu_timesig,"5:4",16396);

	// create quantization menu
	hmenu_quantize=CreatePopupMenu();
	arg_menu_add_item(hmenu_quantize,"Previous\tCtrl+1"	,16396);
	arg_menu_add_item(hmenu_quantize,"Next\tCtrl+2"		,16397);
	arg_menu_add_item(hmenu_quantize,NULL				,16398);
	arg_menu_add_item(hmenu_quantize,"Free"				,16399);
	arg_menu_add_item(hmenu_quantize,NULL				,16400);
	arg_menu_add_item(hmenu_quantize,"128th"			,16401);
	arg_menu_add_item(hmenu_quantize,"64th"				,16402);
	arg_menu_add_item(hmenu_quantize,"32nd"				,16403);
	arg_menu_add_item(hmenu_quantize,"16th"				,16404);
	arg_menu_add_item(hmenu_quantize,"Eighth"			,16405);
	arg_menu_add_item(hmenu_quantize,"Quarter"			,16406);
	arg_menu_add_item(hmenu_quantize,"Half"				,16407);
	arg_menu_add_item(hmenu_quantize,"Whole"			,16408);
	arg_menu_add_item(hmenu_quantize,NULL				,16409);
	arg_menu_add_item(hmenu_quantize,"Dotted 128th"		,16410);
	arg_menu_add_item(hmenu_quantize,"Dotted 64th"		,16411);
	arg_menu_add_item(hmenu_quantize,"Dotted 32nd"		,16412);
	arg_menu_add_item(hmenu_quantize,"Dotted 16th"		,16413);
	arg_menu_add_item(hmenu_quantize,"Dotted Eighth"	,16414);
	arg_menu_add_item(hmenu_quantize,"Dotted Quarter"	,16415);
	arg_menu_add_item(hmenu_quantize,"Dotted Half"		,16416);
	arg_menu_add_item(hmenu_quantize,"Dotted Whole"		,16417);
	arg_menu_add_item(hmenu_quantize,NULL				,16418);
	arg_menu_add_item(hmenu_quantize,"Triplet 128th"	,16419);
	arg_menu_add_item(hmenu_quantize,"Triplet 64th"		,16420);
	arg_menu_add_item(hmenu_quantize,"Triplet 32nd"		,16421);
	arg_menu_add_item(hmenu_quantize,"Triplet 16th"		,16422);
	arg_menu_add_item(hmenu_quantize,"Triplet Eighth"	,16423);
	arg_menu_add_item(hmenu_quantize,"Triplet Quarter"	,16424);
	arg_menu_add_item(hmenu_quantize,"Triplet Half"		,16425);
	arg_menu_add_item(hmenu_quantize,"Triplet Whole"	,16426);

	// create midi channel menu
	hmenu_midi_ch=CreatePopupMenu();
	arg_menu_add_item(hmenu_midi_ch,"Previous\tCtrl+3",	16427);
	arg_menu_add_item(hmenu_midi_ch,"Next\tCtrl+4",		16428);
	arg_menu_add_item(hmenu_midi_ch,NULL,				16429);
	arg_menu_add_item(hmenu_midi_ch,"0 (01)",			16430);
	arg_menu_add_item(hmenu_midi_ch,"1 (02)",			16431);
	arg_menu_add_item(hmenu_midi_ch,"2 (03)",			16432);
	arg_menu_add_item(hmenu_midi_ch,"3 (04)",			16433);
	arg_menu_add_item(hmenu_midi_ch,"4 (05)",			16434);
	arg_menu_add_item(hmenu_midi_ch,"5 (06)",			16435);
	arg_menu_add_item(hmenu_midi_ch,"6 (07)",			16436);
	arg_menu_add_item(hmenu_midi_ch,"7 (08)",			16437);
	arg_menu_add_item(hmenu_midi_ch,"8 (09)",			16438);
	arg_menu_add_item(hmenu_midi_ch,"9 (10)",			16439);
	arg_menu_add_item(hmenu_midi_ch,"A (11)",			16440);
	arg_menu_add_item(hmenu_midi_ch,"B (12)",			16441);
	arg_menu_add_item(hmenu_midi_ch,"C (13)",			16442);
	arg_menu_add_item(hmenu_midi_ch,"D (14)",			16443);
	arg_menu_add_item(hmenu_midi_ch,"E (15)",			16444);
	arg_menu_add_item(hmenu_midi_ch,"F (16)",			16445);

	// create kbd octave menu
	hmenu_kbd_octave=CreatePopupMenu();
	arg_menu_add_item(hmenu_kbd_octave,"Previous\tCtrl+5"	,16446);
	arg_menu_add_item(hmenu_kbd_octave,"Next\tCtrl+6"		,16447);
	arg_menu_add_item(hmenu_kbd_octave,NULL					,16448);
	arg_menu_add_item(hmenu_kbd_octave,"0 (00)"			,16449);
	arg_menu_add_item(hmenu_kbd_octave,"1 (12)"			,16450);
	arg_menu_add_item(hmenu_kbd_octave,"2 (24)"			,16451);
	arg_menu_add_item(hmenu_kbd_octave,"3 (36)"			,16452);
	arg_menu_add_item(hmenu_kbd_octave,"4 (48)"			,16453);
	arg_menu_add_item(hmenu_kbd_octave,"5 (60)"			,16454);
	arg_menu_add_item(hmenu_kbd_octave,"6 (72)"			,16455);
	arg_menu_add_item(hmenu_kbd_octave,"7 (84)"			,16456);
	arg_menu_add_item(hmenu_kbd_octave,"8 (96)"			,16457);

	// create midi mask menu
	hmenu_midi_mask=CreatePopupMenu();
	arg_menu_add_item(hmenu_midi_mask,"8 - Note Off"				,16458);
	arg_menu_add_item(hmenu_midi_mask,"9 - Note On"					,16459);
	arg_menu_add_item(hmenu_midi_mask,"A - Polyphonic Aftertouch"	,16460);
	arg_menu_add_item(hmenu_midi_mask,"B - Continuous Controller"	,16461);
	arg_menu_add_item(hmenu_midi_mask,"C - Program Change"			,16462);
	arg_menu_add_item(hmenu_midi_mask,"D - Channel Pressure"		,16463);
	arg_menu_add_item(hmenu_midi_mask,"E - Pitchbend Wheel"			,16464);
	arg_menu_add_item(hmenu_midi_mask,"F - System Exclusive"		,16465);

	// create instance menu
	hmenu_instance=CreatePopupMenu();
	arg_menu_add_item(hmenu_instance,"Open Editor"						,16466);
	arg_menu_add_item(hmenu_instance,NULL								,16467);
	arg_menu_add_item(hmenu_instance,"Import Cubase Preset(s)..."		,16468);
	arg_menu_add_item(hmenu_instance,NULL								,16469);
	arg_menu_add_item(hmenu_instance,"Export Cubase Program (FXP)..."	,16470);
	arg_menu_add_item(hmenu_instance,"Export Cubase Bank (FXB)..."		,16471);
	arg_menu_add_item(hmenu_instance,NULL								,16472);
	arg_menu_add_item(hmenu_instance,"Delete Instance..."				,16473);

	// track midi vu reset
	for(int tv=0;tv<MAX_TRACKS;tv++)
		master_midi_vumeter[tv]=0;

	// load cursors
	hcursor_diag=LoadCursor(NULL,IDC_SIZENWSE);
	hcursor_move=LoadCursor(NULL,IDC_SIZEALL);
	hcursor_arro=LoadCursor(NULL,IDC_ARROW);
	hcursor_szwe=LoadCursor(NULL,IDC_SIZEWE);
	hcursor_szns=LoadCursor(NULL,IDC_SIZENS);
	hcursor_beam=LoadCursor(NULL,IDC_IBEAM);
	hcursor_wait=LoadCursor(NULL,IDC_WAIT);
	hcursor_hacl=LoadCursor(hinstance,MAKEINTRESOURCE(IDC_HC));
	hcursor_haop=LoadCursor(hinstance,MAKEINTRESOURCE(IDC_HO));

	// create fonts
	hfont_terminal=CreateFont(11,0,0,0,FW_NORMAL,0,0,0,OEM_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_MODERN,"Terminal");

	// build vst library
	hmenu_vst_lib=NULL;
	gui_vst_build_lib();

	// reset dsp output vumeters
	for(int o=0;o<NUM_DSP_OUTPUTS;o++)
		dsp_output_vumeter[o]=0.0f;

	// open asio driver list and store num of drivers
	asio_num_inputs=0;
	asio_num_outputs=0;
	asio_num_drivers=asio_init_list();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAodixCore::~CAodixCore(void)
{
	// destroy brush
	DeleteObject(hbrush_edit);

	// destroy menus
	DestroyMenu(hmenu_vst_lib);
	DestroyMenu(hmenu_timesig);
	DestroyMenu(hmenu_ppqn);
	DestroyMenu(hmenu_patterns);
	DestroyMenu(hmenu_quantize);
	DestroyMenu(hmenu_midi_ch);
	DestroyMenu(hmenu_kbd_octave);
	DestroyMenu(hmenu_midi_mask);
	DestroyMenu(hmenu_program);
	DestroyMenu(hmenu_instance);
	DestroyMenu(hmenu_asio_input);
	DestroyMenu(hmenu_asio_output);

	// delete cursors
	DestroyCursor(hcursor_diag);
	DestroyCursor(hcursor_move);
	DestroyCursor(hcursor_arro);
	DestroyCursor(hcursor_szwe);
	DestroyCursor(hcursor_szns);
	DestroyCursor(hcursor_beam);
	DestroyCursor(hcursor_wait);
	DestroyCursor(hcursor_hacl);
	DestroyCursor(hcursor_haop);

	// delete fonts
	DeleteObject(hfont_terminal);

	// stop midi input
	midi_in_close();

	// reset project
	reset_project();

	// write config file at exit
	config_write();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::reset_user(void)
{
	// reset user data
	user_pat=0;
	user_page=0;
	user_quantize=5;
	user_midi_ch=0;
	user_kbd_note_offset=48;
	user_midi_mask=0x7F;
	user_kbd_velo=127;
	user_midi_learn=0;
	user_trk=0;
	user_trk_offset=0;
	user_pressed=0;
	user_instance=0;
	user_instance_list_offset=0;
	user_parameter=0;
	user_parameter_list_offset=0;
	user_lxm=0;
	user_lym=0;
	user_dragging_rout_instance_index=0;
	user_dragging_rout_pin_index=0;
	user_edit_overwrite=1;
	user_edit_step=1;
	user_row=0;
	user_block_pos_sta=0;
	user_block_trk_sta=0;
	user_block_pos_end=0;
	user_block_trk_end=0;
	user_event_drag=0;
	user_marker_drag=0;
	user_drag_offset=0;
	user_input_pin=0;
	user_output_pin=0;
	user_edit_text_action_id=0;
	user_rout_offset_x=-256;
	user_rout_offset_y=-128;
	user_pr_width=256;
	user_pr_note_width=8;
	user_pat_prev = 0;

	// reset last vst parameter value
	user_parameter_val=0.0f;

	// reset last file opened
	user_last_file[0]=0;

	// reset midi-in user monitor
	memset(user_midi_in_monitor,0,9);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::reset_project(void)
{
	// clear project information
	memset(project.name,0,32);
	memset(project.info,0,32);

	// set default program information
	sprintf(project.name,"Untitled");
	sprintf(project.info,"Brief Comment");

	// reset master transport info
	project.master_tempo=120.0;
	project.master_ppqn=192;
	project.master_numerator=4;
	project.master_denominator=4;

	// free all vsti's
	for(int i=0;i<MAX_INSTANCES;i++)
	{
		ADX_INSTANCE* pi=&instance[i];
		instance_free(pi);
	}

	// reset pattern data
	for(int p=0;p<MAX_PATTERNS;p++)
		reset_pattern(&project.pattern[p]);

	// set default 'arrange' label to pattern 0
	sprintf(project.pattern[0].name,"Arrange");

	// reset sequencer events
	seq_num_events=0;

	// reset undo events
	undo_num_events=0;
	undo_combine=false;

	// reset master input pins
	for(p=0;p<NUM_DSP_INPUTS;p++)
		edit_clr_pin(&master_input_pin[p]);

	// reset master i/o module screen coordinates
	master_i_x=0;
	master_i_y=0;
	master_o_x=0;
	master_o_y=256;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::reset_pattern(ADX_PATTERN* pp)
{
	// default pattern name
	memset(pp->name,0,32);
	sprintf(pp->name,"---");

	// reset pattern cue-data (default: loop 1 bar, stop marker off)
	pp->cue_sta=0;
	pp->cue_end=project.master_ppqn*4;
	pp->cue_stp=0;
	pp->cue_res=0;

	// reset pattern user data
	pp->usr_pos=0;
	pp->usr_ppp=arg_tool_clipped_assign(project.master_ppqn/64,1,MAX_SIGNED_INT);
	pp->usr_pre=project.master_ppqn/16;
	pp->usr_mod=0;

	// reset reserved data
	memset(pp->reserved,0,sizeof(int)*8);

	// reset tracks
	for(int t=0;t<MAX_TRACKS;t++)
	{
		// get track pointer
		ADX_TRACK* pt=&pp->track[t];

		// default track properties
		memset(pt->name,0,32);
		sprintf(pt->name,"---");

		// reset mute and solo properties
		pt->mute=0;
		pt->solo=0;

		// reserved (init to 0)
		pt->res0=0;
		pt->res1=0;
	}

	// reset makers
	for(int m=0;m<MAX_MARKERS;m++)
	{
		// get marker pointer
		ADX_MARKER* pm=&pp->marker[m];

		// fill marker info
		pm->pos=0;
		pm->flg=0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::config_read(void)
{
	// init reserved fields
	for(int r=0;r<_MAX_PATH;r++)
		cfg.reserved[r]=0;

	// init skin path
	strcpy(cfg.skin_path, "Skins\\Blue");

	// init config vst path(s)
	for(int pi=0;pi<MAX_VST_FOLDERS;pi++)
		cfg.vst_path[pi][0]=0;

	// init recent file paths
	for(int rf=0;rf<NUM_RECENT_FILES;rf++)
		cfg.recent_file[rf][0]=0;

	// init user gui config values
	cfg.instance_autolink=0;
	cfg.rec_live=1;
	cfg.stop_wrap=0;
	cfg.keyboard_layout=0;
	cfg.fullscreen = 0;

	// init config asio driver
	cfg.asio_driver_id=-1;
	cfg.asio_driver_sample_rate=44100.0;

	// init midi in config
	cfg.midi_in_ch_rout=0;
	cfg.midi_in_vl_rout=0;
	cfg.midi_in_dv_open=1;

	// init reserved fields 2
	for(r=0;r<14;r++)
		cfg.reserved2[r]=0;

	// init user registration
	sprintf(cfg.user_name,"Aodix User");
	memset(cfg.user_reserved,0,32);

	// reset asio input pins
	for(int i=0;i<NUM_DSP_INPUTS;i++)
		cfg.asio_input_pin[i]=i;

	// reset asio output pins
	for(int o=0;o<NUM_DSP_OUTPUTS;o++)
		cfg.asio_output_pin[o]=o;

	// format config file
	char cfg_filename[_MAX_PATH];
	sprintf(cfg_filename,"%s\\Aodix.cfg",app_dir);

	// open config file
	FILE* pfile=fopen(cfg_filename,"rb");

	// read config file if opened
	if(pfile!=NULL)
	{
		fread(&cfg,1,sizeof(ADX_CONFIG),pfile);
		fclose(pfile);
	}
	else
	{
		// write default config is not existing cfg file
		config_write();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::config_write(void)
{
	// format config file
	char cfg_filename[_MAX_PATH];
	sprintf(cfg_filename,"%s\\Aodix.cfg",app_dir);

	// open config file for write
	FILE* pfile=fopen(cfg_filename,"wb");

	// write config file if opened
	if(pfile!=NULL)
	{
		fwrite(&cfg,1,sizeof(ADX_CONFIG),pfile);
		fclose(pfile);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::recent_files_push(char* new_file)
{
	// aux file holder
	char new_entry[_MAX_PATH];
	memcpy(new_entry,new_file,_MAX_PATH);

	// last index to push
	int last_index=NUM_RECENT_FILES-1;

	// first of all compare if new file opened exist in the recent files list
	for(int rf=0;rf<NUM_RECENT_FILES;rf++)
	{
		// compare
		if(strcmp(new_entry,cfg.recent_file[rf])==0)
			last_index=rf;
	}

	// update recent file list
	for(rf=(last_index-1);rf>=0;rf--)
		memcpy(cfg.recent_file[rf+1],cfg.recent_file[rf],_MAX_PATH);

	// copy new file
	strcpy(cfg.recent_file[0],new_entry);
}
