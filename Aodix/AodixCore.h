/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Header
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define FULL_VERSION
#define MAX_INSTANCES		256
#define MAX_EVENTS			262144
#define MAX_TRACKS			256
#define MAX_PATTERNS		256
#define MAX_MARKERS			256
#define MAX_SIGNED_INT		0x7FFFFFFF
#define MAX_DSP_BLOCK_SIZE	2048
#define MAX_VST_FOLDERS		4
#define MAX_VST_LIB_ENTRIES	1024
#define NUM_RECENT_FILES	4
#define NUM_DSP_INPUTS		32
#define NUM_DSP_OUTPUTS		32
#define	TRACK_WIDTH			96
#define NR_EVENT_HEIGHT		14
#define MASTER_INSTANCE		256
#define INPUT_INSTANCE		258
#define IDM_VST_LIB			8192

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#undef UNICODE
#include "windows.h"
#include "stdio.h"
#include "math.h"
#include "objbase.h"
#include "shlobj.h"
#include "resource.h"
#include "mmsystem.h"
#include "float.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../argulib/argulib.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../vstsdk2.3/source/common/aeffectx.h"
#include "../vstsdk2.3/source/common/audioeffectx.h"
#include "../vstsdk2.3/source/common/aeffeditor.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../asiosdk2/common/asiosys.h"
#include "../asiosdk2/common/asio.h"
#include "../asiosdk2/common/asiolist.h"
#include "../asiosdk2/common/iasiodrv.h"
#include "../asiosdk2/common/asiodrivers.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// window procedures
LRESULT CALLBACK main_wnd_proc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK plug_wnd_proc(HWND,UINT,WPARAM,LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dialog procedures
LRESULT CALLBACK conf_dlg_proc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK boun_dlg_proc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK abou_dlg_proc(HWND,UINT,WPARAM,LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// host audiomaster callback
long VSTCALLBACK host_audiomaster(AEffect *effect, long opcode, long index, long value, void *ptr, float opt);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ADX_CONFIG
{
	// reserved
	char reserved[_MAX_PATH];

	// user GUI parameters
	int	instance_autolink;
	int	stop_wrap;
	int	rec_live;
	int	keyboard_layout;

	// asio driver index
	int	asio_driver_id;

	// asio driver sample rate
	double asio_driver_sample_rate;

	// midi input
	int midi_in_ch_rout;
	int midi_in_vl_rout;
	int midi_in_dv_open;

	// reserved fields
	int reserved2[14];

	// registration info
	char user_name[32];
	char user_reserved[32];

	// asio output pins assigns
	int asio_input_pin[NUM_DSP_INPUTS];
	int asio_output_pin[NUM_DSP_OUTPUTS];

	// vst plugin paths
	char vst_path[MAX_VST_FOLDERS][_MAX_PATH];

	// recent opened files paths
	char recent_file[NUM_RECENT_FILES][_MAX_PATH];

	// skin folder path
	char skin_path[_MAX_PATH];

	// open in fullscreen mode
	int fullscreen;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ADX_EVENT
{
	// event position and parameter
	int pos;
	int par;

	// properties
	unsigned char pat;
	unsigned char trk;
	unsigned char typ;
	unsigned char szd;

	// data
	unsigned char da0;
	unsigned char da1;
	unsigned char da2;
	unsigned char da3;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ADX_WIRE
{
	short instance_index;
	short pin_index;
	float value;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ADX_PIN
{
	ADX_WIRE* pwire;
	int num_wires;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ADX_INSTANCE
{
	// handle to vst instnace editor frame window
	HWND hwnd;

	// the vst instance pointer
	AEffect* peffect;

	// inputs and outputs pins
	float**	pins;
	float**	pous;

	// audio output pins / midi output connector vector
	ADX_PIN* pout_pin;
	ADX_PIN  mout_pin;

	// plugin alias name
	char alias[32];

	// dll path (where the vst dll is stored)
	char dll_path[_MAX_PATH];

	// counter of midi events queue size in processing block
	int midi_queue_size;

	// vst midi events for processing block
	VstMidiEvent midi_event[MAX_BLOCK_EVENTS];	

	// graphical screen position in routing
	int	x;
	int	y;

	// instance properties
	unsigned char process_mute;
	unsigned char process_thru;

	// midi-cc matrix vector
	unsigned char* pmidi_cc;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ADX_TRACK
{
	// track label
	char name[32];

	// mute / solo events
	unsigned char mute;
	unsigned char solo;
	unsigned char res0;
	unsigned char res1;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ADX_MARKER
{
	int	pos;
	int	flg;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ADX_PATTERN
{
	// pattern label
	char name[32];

	// pattern tracks
	ADX_TRACK track[MAX_TRACKS];

	// cue data of this pattern
	int	cue_sta;
	int	cue_end;
	int cue_stp;
	int cue_res;

	// user position
	int usr_pos;
	int usr_ppp;
	int usr_pre;
	int usr_mod;

	// reserved
	int reserved[8];

	// pattern markers array
	ADX_MARKER marker[MAX_MARKERS];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ADX_PROJECT
{
	// project name and info
	char name[32];
	char info[32];

	// project tempo (in beats per minute)
	double master_tempo;

	// patterns
	ADX_PATTERN pattern[MAX_PATTERNS];

	// master info
	int	master_numerator;
	int	master_denominator;
	int master_ppqn;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ADX_VST_LIB
{
	// path of each dll
	char dll_path[MAX_VST_LIB_ENTRIES][_MAX_PATH];

	// num vst dlls found
	int num_dlls;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CAodixCore
{
public:
	CAodixCore(HINSTANCE const hinstance);
	~CAodixCore(void);

public:
	void reset_user(void);
	void reset_project(void);
	void reset_pattern(ADX_PATTERN* pp);

public:
	void config_write(void);
	void config_read(void);

public:
	void recent_files_push(char* new_file);

public:
	void gui_create_edit(HWND const hwnd,int x,int y,int const w,int const h,char* default_text,int text_action_id);
	void gui_process_text_action(HWND const hwnd);
	void gui_format_note(int const key,char* buf);
	void gui_format_pos(int const pos,char* buf);
	void gui_format_time(int const samples,char* buf);
	void gui_mouse_down(HWND const hwnd,bool const is_double_click);
	void gui_mouse_up(HWND const hwnd);
	void gui_mouse_move(HWND const hwnd);
	void gui_mouse_scan_pin(HWND const hwnd,int const xm,int const ym,ADX_PIN* ppins,int const num_pins,int const pin_x,int const pin_y,int const is_double_click,int const is_midi_pin);
	void gui_timer(HWND const hwnd);
	void gui_command(HWND const hwnd,int const id);
	void gui_mouse_wheel(HWND const hwnd,int const delta);
	void gui_key_down(HWND const hwnd,int const keycode,int const flags);
	void gui_key_up(HWND const hwnd,int const keycode);
	void gui_key_pc_piano(int const keycode,int* pkeyval);
	void gui_delete_current_instance(HWND const hwnd);
	void gui_scroll_to_instance(void);
	void gui_vst_build_lib(void);
	void gui_scroll_iterate(HWND const hwnd,int const ym,int const seq_area_y,int const h);

public:
	int gui_vst_search(HMENU const pa_hmenu,char* pfolder);
	int	gui_get_event_height(ADX_EVENT* pe);

public:
	void paint_master_vumeter(HDC const hdc);
	void paint_block_shade(HDC const hdc,int const x,int const y,int const w,int const h,int const lx1,int const ly1,int const lx2,int const ly2);
	void paint_txt(HDC const hdc,int const x,int const y,char* txt,int const num_chars,int const type);
	void paint_seq_pos_big(HDC const hdc,int const x,int const y,int const pos);
	void paint_event(HDC const hdc,int const x,int const y,int const w,int const h,COLORREF const text_color,COLORREF const back_color,char* text);
	void paint_marker(HDC const hdc,COLORREF const color,int const type,char* label,int const y,int const w,int const label_offset,int const selected);
	void paint_wire(HDC const hdc,int const x1,int const y1,int const x2,int const y2,COLORREF const clr);
	void paint_pin_array(HDC const hdc,int const x,int const y,int const num_pins,ADX_PIN* ppin,int const master_o_x,int const master_o_y,int const rout_area_y);
	void paint_track_midi_vumeters(HDC const hdc,int const x,int const y,int const visible_tracks);
	void paint(HWND const hwnd,HDC const hdc,int const w,int const h);

public:
	void paint_sequencer(HWND const hwnd,HDC const hdc,int const w,int const h);
	void paint_routing(HWND const hwnd,HDC const hdc,int const w,int const h);

public:
	void dsp_work(void);
	void dsp_clear_input_buffers(void);
	void dsp_transport_play(void);
	void dsp_transport_stop(void);
	void dsp_vumeter_drive(float* psrc,float& vum,int const num_samples);

public:
	int	seq_sample_to_pos(int const sample);
	int	seq_pos_to_sample(int const pos);

public:
	void seq_delete_event(int const index);
	void seq_delete_events_at(int const pat,int const pos,int const pos_rng,int const trk,int const trk_rng);
	void seq_delete_events_at_pattern(int const pat);
	void seq_add_event(int const pos,unsigned char const pat,unsigned char const trk,unsigned char const typ,unsigned char const da0,unsigned char const da1,unsigned char const da2,unsigned char const da3,int const overwrite);
	void seq_add_evmid(int const pos,unsigned char const pat,unsigned char const trk,unsigned char const ins,unsigned char const da0,unsigned char const da1,unsigned char const da2,int const overwrite);

public:
	void instance_dll(HWND const hwnd,ADX_INSTANCE* pi,char* filename,int const param_x,int const param_y);
	void instance_free(ADX_INSTANCE* pi);
	void instance_open_editor(ADX_INSTANCE* pi,HWND const hwnd);
	void instance_close_editor(ADX_INSTANCE* pi);
	void instance_add_midi_event(ADX_INSTANCE* pi,int const track,unsigned char const md0,unsigned char const md1,unsigned char const md2,unsigned char const md3,int const deltaframe);
	void instance_midi_panic(ADX_INSTANCE* pi,bool const all_notes_off,bool const all_sounds_off);
	void instance_set_param(ADX_INSTANCE* pi,int const param_index,float const param_value);

public:
	void host_idle(void);
	void host_on_automation(AEffect* peffect,int const param,float const value);
	void host_on_events(AEffect* peffect,VstEvents* pevents);
	void host_update_window_caption(AEffect* peffect);

public:
	ADX_INSTANCE* instance_get_from_effect(AEffect* peffect,int* pindex);

public:
	void edit_undo_snapshot(bool combine=false);
	void edit_undo_combine_next(void);
	void edit_undo(void);
	void edit_insert(int const all_tracks);
	void edit_back(int const all_tracks);
	void edit_resize(int const all_tracks);
	void edit_copy(int const cut);
	void edit_paste(void);
	void edit_select_all(void);
	void edit_interpolate(void);
	void edit_transpose(int const amt,int const apply_quantize);
	void edit_randomize(void);
	void edit_add_new_marker(ADX_PATTERN* pp,int const position);
	void edit_add_wire(ADX_PIN* pp,int const instance_index,int const pin_index,float const gain);
	void edit_del_wire(ADX_PIN* pp,int const index);
	void edit_clr_pin(ADX_PIN* pp);

public:
	ADX_EVENT* edit_find_event(int pat,int pos,int trk);
	ADX_EVENT* edit_find_next_event(int pat,int pos,int trk,int len);
	ADX_EVENT* edit_find_prev_event(int pat,int pos,int trk,int len);

public:
	int	edit_get_quantization(void);
	int edit_quantize(int const position);
	
public:
	void import_adx_file_dlg(HWND const hwnd);
	void import_adx_pin(ADX_PIN* pp,int num_pins,FILE* pfile);
	void import_adx_file(HWND const hwnd,char* filename);
	void import_search_file(char* pfolder,char* pfilename,char* pmatchfile);
	int  import_localize_vst_dll(HWND const hwnd,char* pfilename);
	long import_reverse_long(long const data);
	void import_cub_file_dlg(HWND const hwnd);
	void import_cub_file(HWND const hwnd,char* filename);
	void import_midi_file_dlg(HWND const hwnd);
	void import_midi_file(HWND const hwnd,char* filename);
	long import_midi_read_long(FILE* pfile);
	int  import_midi_read_short(FILE* pfile);
	long import_midi_read_var_len(FILE* pfile);
	void import_midi_on_sysex(FILE* pfile,int const pattern_index,int const track,int const i_event_pos,unsigned char const stat);

public:
	void export_adx_file_dlg(HWND const hwnd);
	void export_adx_pin(ADX_PIN* pin_array,int num_pins,FILE* pfile);
	void export_adx_file(HWND const hwnd,char* filename);
	void export_cub_file_dlg(HWND const hwnd,int const is_fxp);
	void export_cub_file(HWND const hwnd,char* filename,int const is_fxp);
	void export_mid_file_dlg(HWND const hwnd);
	void export_mid_file(HWND const hwnd,char* filename);

public:
	void midi_in_init(void);
	void midi_in_process(BYTE const data0,BYTE const data1,BYTE const data2);
	void midi_in_close(void);

public:
	void asio_enter_cs(void);
	void asio_leave_cs(void);
	void asio_init(HWND const hwnd);
	void asio_close(HWND const hwnd);
	bool asio_is_inited(void);
	void asio_get_driver_name(int const id,char* buf,int const num_chars);
	long asio_init_list(void);
	void asio_fill_pin_menu(HMENU const hmenu,int const fill_inputs,int const init_id);

public:
	// application instance handler
	HINSTANCE hinstance_app;

	// window handler
	HWND hwnd_edit;

	// brush
	HBRUSH hbrush_edit;

	// gfx handler
	HDC hdc_gui;
	HDC hdc_gfx;
	HDC hdc_knb;

	// cursors
	HCURSOR	hcursor_diag;
	HCURSOR	hcursor_move;
	HCURSOR	hcursor_arro;
	HCURSOR	hcursor_szwe;
	HCURSOR	hcursor_szns;
	HCURSOR	hcursor_beam;
	HCURSOR	hcursor_wait;
	HCURSOR	hcursor_hacl;
	HCURSOR	hcursor_haop;

	// popup menus
	HMENU hmenu_vst_lib;
	HMENU hmenu_timesig;
	HMENU hmenu_ppqn;
	HMENU hmenu_patterns;
	HMENU hmenu_quantize;
	HMENU hmenu_midi_ch;
	HMENU hmenu_kbd_octave;
	HMENU hmenu_midi_mask;
	HMENU hmenu_program;
	HMENU hmenu_instance;
	HMENU hmenu_asio_input;
	HMENU hmenu_asio_output;

	// fonts
	HFONT hfont_terminal;

	// midi in devices
	HMIDIIN* p_hmidi_in;

public:
	// dsp variables
	float dsp_input_buffer[NUM_DSP_INPUTS][MAX_DSP_BLOCK_SIZE];
	float dsp_output_buffer[NUM_DSP_OUTPUTS][MAX_DSP_BLOCK_SIZE];
	float dsp_output_vumeter[NUM_DSP_OUTPUTS];
	float dsp_cpu_cost;
	long  dsp_block_size;
	long  dsp_highest_opin_count;

public:
	// master variables
	int	master_transport_sampleframe;
	int	master_transport_last_y;
	int	master_midi_vumeter[MAX_TRACKS];
	int	master_i_x;
	int	master_i_y;
	int	master_o_x;
	int	master_o_y;

public:
	ADX_PIN master_input_pin[NUM_DSP_INPUTS];

public:
	VstTimeInfo	master_time_info;

public:
	char app_dir[_MAX_PATH];
	char hlp_fil[_MAX_PATH];

public:
	ADX_CONFIG		cfg;
	ADX_PROJECT		project;
	ADX_INSTANCE	instance[MAX_INSTANCES];
	ADX_VST_LIB		vst_lib;

public:
	long instance_eff_id_currently_loading;

public:
	// event vectors
	ADX_EVENT seq_event[MAX_EVENTS];
	ADX_EVENT clipboard_event[MAX_EVENTS];
	ADX_EVENT undo_event[MAX_EVENTS];

public:
	// sequencer info
	int seq_num_events;

public:
	// clipboard info
	int clipboard_num_events;
	int clipboard_pos_sta;
	int clipboard_trk_sta;
	int clipboard_pos_len;
	int clipboard_trk_len;

public:
	// undo buffer info
	int undo_num_events;
	bool undo_combine;

public:
	// user variables
	int	user_page;
	int user_quantize;
	int user_midi_ch;
	int	user_kbd_note_offset;
	int	user_kbd_velo;
	int user_midi_mask;
	int user_midi_learn;
	int	user_pat;
	int user_trk;
	int user_trk_offset;
	int	user_row;
	int user_pressed;
	int user_instance;
	int user_instance_list_offset;
	int user_parameter;
	int user_parameter_list_offset;
	int user_lxm;
	int user_lym;
	int user_dragging_rout_instance_index;
	int user_dragging_rout_pin_index;
	int	user_edit_overwrite;
	int user_edit_step;
	int	user_block_pos_sta;
	int	user_block_trk_sta;
	int	user_block_pos_end;
	int	user_block_trk_end;
	int user_event_drag;
	int	user_marker_drag;
	int user_drag_offset;
	int user_input_pin;
	int user_output_pin;
	int	user_edit_text_action_id;
	int user_rout_offset_x;
	int user_rout_offset_y;
	int user_pr_width;
	int user_pr_note_width;
	int user_pat_prev;

	// draggin wire
	ADX_WIRE* user_pressed_wire;
	int user_pressed_wire_x;
	int user_pressed_wire_y;

	// user 32-bit float register for track the vst parameter value
	float user_parameter_val;

public:
	char user_last_file[_MAX_PATH];
	char user_midi_in_monitor[9];

public:
	// when is set, one gui refresh is posted for next timer iteration
	int	gui_is_dirty;

public:
	// system cpu speed
	double d_system_clocks_per_sec;

public:
	// explain itself ;-)
	int	aodix_version;

public:
	// asio properties
	long asio_num_drivers;
	long asio_num_inputs;
	long asio_num_outputs;
};
