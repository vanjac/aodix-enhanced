/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Paint Sequencer
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char note_name[128][4]=
{
	"C-0","C#0","D-0","D#0","E-0","F-0","F#0","G-0","G#0","A-0","A#0","B-0",
	"C-1","C#1","D-1","D#1","E-1","F-1","F#1","G-1","G#1","A-1","A#1","B-1",
	"C-2","C#2","D-2","D#2","E-2","F-2","F#2","G-2","G#2","A-2","A#2","B-2",
	"C-3","C#3","D-3","D#3","E-3","F-3","F#3","G-3","G#3","A-3","A#3","B-3",
	"C-4","C#4","D-4","D#4","E-4","F-4","F#4","G-4","G#4","A-4","A#4","B-4",
	"C-5","C#5","D-5","D#5","E-5","F-5","F#5","G-5","G#5","A-5","A#5","B-5",
	"C-6","C#6","D-6","D#6","E-6","F-6","F#6","G-6","G#6","A-6","A#6","B-6",
	"C-7","C#7","D-7","D#7","E-7","F-7","F#7","G-7","G#7","A-7","A#7","B-7",
	"C-8","C#8","D-8","D#8","E-8","F-8","F#8","G-8","G#8","A-8","A#8","B-8",
	"C-9","C#9","D-9","D#9","E-9","F-9","F#9","G-9","G#9","A-9","A#9","B-9",
	"C-A","C#A","D-A","D#A","E-A","F-A","F#A","G-A"
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char midi_name[8][4]={"Off","On ","Aft","CC ","Prg","Prs","Pit","Sys"};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_sequencer(HWND const hwnd,HDC const hdc,int const w,int const h)
{
    if(user_page==0 && h>240)
	{
        // temporal text buffer holder
        char buf_a[256];

        // get current pattern pointer
        ADX_PATTERN* pp=&project.pattern[user_pat];

        // get user quantization
        int const i_quantize=edit_get_quantization();

        // get master pos
        int const i_master_play_pos=seq_sample_to_pos(master_transport_sampleframe);

        // back color
        COLORREF const bck_color=GetPixel(hdc_gfx,0,0);

        // alphablend structure
        BLENDFUNCTION bf;
        bf.AlphaFormat=0;
        bf.BlendFlags=0;
        bf.BlendOp=0;

		// sequencer client area
		int const seq_area_y=264;
		int const seq_area_h=h-seq_area_y;
		int const seq_cent_y=seq_area_y+seq_area_h/2;
		int const seq_area_b=seq_area_y+seq_area_h;

		// number of visible tracks
		int const seq_visi_t=(w-user_pr_width)/TRACK_WIDTH;

		// track offset left fix
		if(user_trk<user_trk_offset)
			user_trk_offset=user_trk;

		// track offset right fix
		if(user_trk>=(user_trk_offset+(seq_visi_t-2)))
			user_trk_offset=user_trk-(seq_visi_t-2);

		// track clip region
		arg_gdi_set_clip_rgn(hdc,0,seq_area_y-32,w,32);

		// draw top-left pattern selector
		BitBlt(hdc,0,seq_area_y-32,TRACK_WIDTH,32,hdc_gfx,320,272,SRCCOPY);
		sprintf(buf_a,"%.2X",user_pat);
		paint_txt(hdc,32,236,buf_a,2,0);

		// draw pattern selector dec spin
		if(user_pressed==PRESS_PAT_PREV)
			BitBlt(hdc,TRACK_WIDTH-48,seq_area_y-32,16,16,hdc_gfx,32,0,SRCCOPY);

		// draw pattern selector inc spin
		if(user_pressed==PRESS_PAT_NEXT)
			BitBlt(hdc,TRACK_WIDTH-32,seq_area_y-32,16,16,hdc_gfx,32,16,SRCCOPY);

		// paint pattern label
		paint_txt(hdc,4,252,pp->name,14,0);

		// draw piano-roll track
		BitBlt(hdc,TRACK_WIDTH,seq_area_y-32,user_pr_width-8,32,hdc_gfx,0,640,SRCCOPY);
		BitBlt(hdc,TRACK_WIDTH+user_pr_width-8,seq_area_y-32,8,32,hdc_gfx,760,640,SRCCOPY);

		// draw track frames
		for(int t=0;t<seq_visi_t;t++)
		{
			// get track index and screen coordinate
			int const ti=user_trk_offset+t;
			int const tx=TRACK_WIDTH+user_pr_width+t*TRACK_WIDTH;

			// decide whenever track is visible
			if(ti<MAX_TRACKS)
			{
				// get track pointer
				ADX_TRACK* pt=&pp->track[ti];

				// selected track flag
				int const stf=(ti==user_trk);

				// track frame
				BitBlt(hdc,tx,seq_area_y-32,TRACK_WIDTH,32,hdc_gfx,320+(stf*TRACK_WIDTH),304,SRCCOPY);

				// blit mute
				if(pt->mute)
					BitBlt(hdc,tx,seq_area_y-32,16,16,hdc_gfx,192,0,SRCCOPY);

				// blit solo
				if(pt->solo)
					BitBlt(hdc,tx+16,seq_area_y-32,16,16,hdc_gfx,224,0,SRCCOPY);

				// paint track index
				sprintf(buf_a,"%.3d",ti);
				paint_txt(hdc,tx+43,seq_area_y-28,buf_a,3,3+stf*2);

				// paint track label
				paint_txt(hdc,tx+4,seq_area_y-12,pt->name,14,stf*4);
			}
			else
			{
				// blank track
				BitBlt(hdc,tx,seq_area_y-32,TRACK_WIDTH,32,hdc_gfx,320,336,SRCCOPY);
			}
		}

		// paint track vumeters
		paint_track_midi_vumeters(hdc,TRACK_WIDTH+user_pr_width,seq_area_y-30,seq_visi_t);

		// clip sequencer region
		arg_gdi_set_clip_rgn(hdc,0,seq_area_y,w,seq_area_h);

		// fill sequencer background with solid color
		arg_gdi_paint_solid_rect(hdc,0,seq_area_y,w,seq_area_h,bck_color);

		// get gridlines viewport range
		int const i_bg_rng=(seq_area_h/2)*pp->usr_ppp;
		int const i_bg_min=pp->usr_pos-i_bg_rng;
		int const i_bg_max=pp->usr_pos+i_bg_rng;
		int const i_bg_div=project.master_ppqn/4;

		// gridline flags
		int const i_gridline_visible_tick=(pp->usr_ppp<=(project.master_ppqn/20));
		int const i_gridline_visible_beat=(pp->usr_ppp<=(project.master_ppqn/5));

		// gridline for each tick
		for(int bg=i_bg_min;bg<i_bg_max;bg+=i_bg_div)
		{
			int const bgi=bg/i_bg_div;
			int const tick_pos=bgi*i_bg_div;
			int const bgy=seq_cent_y+(tick_pos-pp->usr_pos)/pp->usr_ppp;

			// draw guide if visible
			if(bgi>=0)
			{
				// gridline label
				int const i_guid_seconds=seq_pos_to_sample(tick_pos)/int(cfg.asio_driver_sample_rate);
				sprintf(buf_a,"%.2d:%.2d   %.3d:%.1d",(i_guid_seconds/60)%60,i_guid_seconds%60,tick_pos/(project.master_ppqn*project.master_numerator),(tick_pos/project.master_ppqn)%project.master_numerator);

				// tick gridline
				if(i_gridline_visible_tick)
					arg_gdi_paint_solid_rect(hdc,TRACK_WIDTH,bgy,w-TRACK_WIDTH,1,bck_color+0x00080808);

				// beat gridline
				if(i_gridline_visible_beat && (bgi%4)==0)
				{
					if(i_gridline_visible_tick)
					{
						SetTextColor(hdc,bck_color+0x00202020);
						TextOut(hdc,6,bgy+2,buf_a,13);
						arg_gdi_paint_solid_rect(hdc,0,bgy,w,1,bck_color+0x00202020);
					}
					else
					{
						arg_gdi_paint_solid_rect(hdc,TRACK_WIDTH,bgy,w-TRACK_WIDTH,1,bck_color+0x00080808);
					}
				}

				// measure gridline
				if((bgi%(project.master_numerator*4))==0)
				{
					if(i_gridline_visible_beat)
					{
						SetTextColor(hdc,bck_color+0x00404040);
						TextOut(hdc,6,bgy+2,buf_a,13);
						arg_gdi_paint_solid_rect(hdc,0,bgy,w,1,bck_color+0x00404040);
					}
					else
					{
						arg_gdi_paint_solid_rect(hdc,TRACK_WIDTH,bgy,w-TRACK_WIDTH,1,bck_color+0x00202020);
					}
				}

				// pattern gridline
				if((bgi%(project.master_numerator*16))==0)
				{
					SetTextColor(hdc,0);
					TextOut(hdc,7,bgy+3,buf_a,13);
					SetTextColor(hdc,bck_color+0x00808080);
					TextOut(hdc,6,bgy+2,buf_a,13);
					arg_gdi_paint_solid_rect(hdc,0,bgy,w,1,bck_color+0x00808080);
				}
			}
		}

		// get piano roll screen port
		int const pr_vn=(user_pr_width-8)/user_pr_note_width;
		int const pr_no=user_kbd_note_offset-(pr_vn/2)+12;

		// paint piano-roll grid
		for(int prn=0;prn<pr_vn;prn++)
		{
			int const pr_ni=pr_no+prn;
			int const pr_nx=TRACK_WIDTH+4+prn*user_pr_note_width;

			COLORREF pr_cl=bck_color+0x00060606;

			if((pr_ni%12)==0 || pr_ni==128)
				pr_cl=bck_color+0x00101010;

			if(pr_ni==(user_kbd_note_offset+12))
				pr_cl=bck_color+0x00202020;

			if(pr_ni==user_kbd_note_offset || pr_ni==(user_kbd_note_offset+24))
				pr_cl=bck_color+0x00404040;

			if(pr_ni>=0 && pr_ni<=128)
				arg_gdi_paint_solid_rect(hdc,pr_nx,seq_area_y,1,seq_area_h,pr_cl);
		}

		// piano roll octaves
		for(int prn=0;prn<pr_vn;prn++)
		{
			int const pr_ni=pr_no+prn;

			if((pr_ni%12)==0 && pr_ni>=0 && pr_ni<100)
			{
				buf_a[0]='0'+pr_ni/10;
				buf_a[1]='0'+pr_ni%10;
				paint_txt(hdc,TRACK_WIDTH+6+prn*user_pr_note_width,seq_area_y+1,buf_a,2,2);
			}
		}

		// draw back track frames
		for(int t=1;t<seq_visi_t;t++)
		{
			int const tfx=TRACK_WIDTH+user_pr_width+t*TRACK_WIDTH;
			arg_gdi_paint_solid_rect(hdc,tfx-1,seq_area_y,1,seq_area_h,bck_color-0x00202020);
			arg_gdi_paint_solid_rect(hdc,tfx,seq_area_y,1,seq_area_h,bck_color+0x00202020);
		}

		// paint sequencer position separator(s)
		arg_gdi_paint_solid_rect(hdc,45,seq_area_y,1,seq_area_h,bck_color-0x00202020);
		arg_gdi_paint_solid_rect(hdc,46,seq_area_y,1,seq_area_h,bck_color+0x00202020);

		// paint piano roll separator corners
		BitBlt(hdc,TRACK_WIDTH,seq_area_y,4,16,hdc_gfx,192,572,SRCCOPY);
		BitBlt(hdc,TRACK_WIDTH+user_pr_width-4,seq_area_y,4,16,hdc_gfx,192,572,SRCCOPY);
		BitBlt(hdc,TRACK_WIDTH,seq_area_y+seq_area_h-16,4,16,hdc_gfx,200,572,SRCCOPY);
		BitBlt(hdc,TRACK_WIDTH+user_pr_width-4,seq_area_y+seq_area_h-16,4,16,hdc_gfx,200,572,SRCCOPY);

		// paint piano roll separator bars
		for(int t=(seq_area_y+16);t<(seq_area_y+seq_area_h-16);t+=16)
		{
			BitBlt(hdc,TRACK_WIDTH,t,4,16,hdc_gfx,196,572,SRCCOPY);
			BitBlt(hdc,TRACK_WIDTH+user_pr_width-4,t,4,16,hdc_gfx,196,572,SRCCOPY);
		}

		// get last visible track index
		int const last_visible_track=user_trk_offset+seq_visi_t;

		// scan all sequencer events
		for(int ev=0;ev<seq_num_events;ev++)
		{
			// get sequencer event pointer
			ADX_EVENT* pe=&seq_event[ev];

			// check event parameters
			if(pe->pat==user_pat && pe->trk>=user_trk_offset && pe->trk<last_visible_track)
			{
				// get event screen coordinates
				int const e_x=TRACK_WIDTH+user_pr_width+(pe->trk-user_trk_offset)*TRACK_WIDTH;
				int const e_y=seq_cent_y+(pe->pos-pp->usr_pos)/pp->usr_ppp;
				int const e_h=gui_get_event_height(pe);
				int const e_b=e_y+e_h;

				// visible events only
				if(e_y<seq_area_b && e_b>seq_area_y)
				{
					// note event
					if(pe->typ==0)
					{
						// paint tracker event
						sprintf(buf_a,"%s %.2X %.2X %.2X %.2X",note_name[pe->da2],pe->da0,pe->da1,pe->da2,pe->da3);
						paint_event(hdc,e_x,e_y,TRACK_WIDTH,e_h,0x00808080+pe->da3*0x00010101,0x00806060,buf_a);

						// paint event in pr
						if(pe->trk==user_trk)
						{
							// get piano roll event screen x
							int const pr_ni=int(pe->da2)-pr_no;
							int const pr_ex=pr_ni*user_pr_note_width;

							// paint piano roll event
							if(pr_ni>=0 && pr_ni<pr_vn)
							{
								arg_gdi_paint_solid_rect(hdc,TRACK_WIDTH+4+pr_ex,e_y,user_pr_note_width,e_h,0x0);
								arg_gdi_paint_solid_rect(hdc,TRACK_WIDTH+5+pr_ex,e_y+1,user_pr_note_width-2,e_h-2,0x00806060+pe->da3*0x00010101);
							}
						}
					}

					// pattern event
					if(pe->typ==1)
					{
						sprintf(buf_a,"Pat %.2X %.2X %.2X %.2X",pe->da0,pe->da1,pe->da2,pe->da3);
						paint_event(hdc,e_x,e_y,TRACK_WIDTH,e_h,0x00FFFFFF,0x00608060,buf_a);

						// pattern event label
						SetTextColor(hdc,0x00FFFFFF);
						sprintf(buf_a,project.pattern[pe->da0].name);
						buf_a[15]=0;
						TextOut(hdc,e_x+3,e_y+16,buf_a,strlen(buf_a));
					}

					// jump event
					if(pe->typ==2)
						paint_event(hdc,e_x,e_y,TRACK_WIDTH,e_h,0x00FFFFFF,0x00404040,"Jmp           ");

					// midi automation event
					if(pe->typ==3)
					{
						int msg=pe->da1>>4;
						char *name;
						if(msg>=8)
							name=midi_name[msg-8];
						else
							name="Mid";
						sprintf(buf_a,"%s %.2X %.2X %.2X %.2X",name,pe->da0,pe->da1,pe->da2,pe->da3);
						paint_event(hdc,e_x,e_y,TRACK_WIDTH,NR_EVENT_HEIGHT,0x00FFFFFF,0x00606080,buf_a);
					}

					// vst automation event
					if(pe->typ==4)
					{
						sprintf(buf_a,"Aut %.2X %.2X %.2X %.2X",pe->da0,pe->da1,pe->da2,pe->da3);
						paint_event(hdc,e_x,e_y,TRACK_WIDTH,NR_EVENT_HEIGHT,0x00FFFFFF,0x00404040+pe->da3/2,buf_a);
					}

					// tempo automation event
					if(pe->typ==5)
					{
						sprintf(buf_a,"Tmp %.2X %.2X %.2X %.2X",pe->da0,pe->da1,pe->da2,pe->da3);
						paint_event(hdc,e_x,e_y,TRACK_WIDTH,NR_EVENT_HEIGHT,0x00FFFFFF,0x00404040+pe->da0/2,buf_a);
					}
				}
			}
		}

		// invert user row cursor
		int const i_cursor_x=TRACK_WIDTH+user_pr_width+3+((user_trk-user_trk_offset)*TRACK_WIDTH);

		// select inverted row position
		switch(user_row)
		{
		case 0: arg_gdi_paint_invert_rect(hdc,i_cursor_x+0,seq_cent_y,18,12);	break;
		case 1:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+24,seq_cent_y,6,12);	break;
		case 2:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+30,seq_cent_y,6,12);	break;
		case 3:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+42,seq_cent_y,6,12);	break;
		case 4:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+48,seq_cent_y,6,12);	break;
		case 5:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+60,seq_cent_y,6,12);	break;
		case 6:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+66,seq_cent_y,6,12);	break;
		case 7:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+78,seq_cent_y,6,12);	break;
		case 8:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+84,seq_cent_y,6,12);	break;
		}

		// paint pattern marker(s)
		for(int m=0;m<MAX_MARKERS;m++)
		{
			// get marker pointer
			ADX_MARKER* pm=&pp->marker[m];

			// paint marker if enabled
			if(pm->flg)
			{
				sprintf(buf_a," Index %.2X",m);
				int const drag_marker=(user_pressed==PRESS_MARKER)*(user_marker_drag==m);
				paint_marker(hdc,0x0088C8C8+drag_marker*0x00101010,2,buf_a,seq_cent_y+(pm->pos-pp->usr_pos)/pp->usr_ppp,w,-16,drag_marker);
			}
		}

		// paint pattern cue-loop start marker
		gui_format_pos(pp->cue_sta,buf_a);
		int const drag_start=(user_pressed==PRESS_CUE_START);
		paint_marker(hdc,0x00A0B0A0+drag_start*0x00101010,0,buf_a,seq_cent_y+(pp->cue_sta-pp->usr_pos)/pp->usr_ppp,w,-16,drag_start);

		// paint pattern cue-loop end marker
		gui_format_pos(pp->cue_end,buf_a);
		int const drag_end=(user_pressed==PRESS_CUE_END);
		paint_marker(hdc,0x00A0B0A0+drag_end*0x00101010,3,buf_a,seq_cent_y+(pp->cue_end-pp->usr_pos)/pp->usr_ppp,w,1,drag_end);

		// paint pattern cue-stop marker
		if(pp->cue_stp>0)
		{
			gui_format_pos(pp->cue_stp,buf_a);
			int const drag_stop=(user_pressed==PRESS_CUE_STOP);
			paint_marker(hdc,0x0060C0A0+drag_stop*0x00101010,1,buf_a,seq_cent_y+(pp->cue_stp-pp->usr_pos)/pp->usr_ppp,w,-16,drag_stop);
		}

		// get block length
		int const user_block_pos_len=user_block_pos_end-user_block_pos_sta;
		int const user_block_trk_len=user_block_trk_end-user_block_trk_sta;

		// draw block mark
		if(user_block_trk_len>0 && user_block_pos_len>0)
		{
			// get block mark vertical coordinates
			int const src_bm_y=seq_cent_y+(user_block_pos_sta-pp->usr_pos)/pp->usr_ppp;
			int const scr_bm_h=user_block_pos_len/pp->usr_ppp;

			// start clip
			paint_block_shade(hdc,TRACK_WIDTH+user_pr_width+(user_block_trk_sta-user_trk_offset)*TRACK_WIDTH,src_bm_y,user_block_trk_len*TRACK_WIDTH,scr_bm_h,TRACK_WIDTH+user_pr_width,seq_area_y,w,h);

			// invert block mark in piano roll view
			if(user_trk>=user_block_trk_sta && user_trk<(user_block_trk_sta+user_block_trk_len))
				paint_block_shade(hdc,TRACK_WIDTH+4,src_bm_y,user_pr_width-8,scr_bm_h,0,seq_area_y,w,h);
		}

		// alphablend edit quantize size shade
		bf.SourceConstantAlpha=24;
		AlphaBlend(hdc,0,seq_cent_y,w,i_quantize/pp->usr_ppp,hdc_gfx,240,0,16,16,bf);

		// alphablend edit cursor position
		bf.SourceConstantAlpha=96;
		AlphaBlend(hdc,0,seq_cent_y,w,1,hdc_gfx,240,0,16,16,bf);

		// invert player transport rect
		if(master_time_info.flags & kVstTransportPlaying)
		{
			// get transport gridline coordinate
			int const i_ty=seq_cent_y+(i_master_play_pos-pp->usr_pos)/pp->usr_ppp;

			// paint play-gridline
			arg_gdi_paint_invert_rect(hdc,0,i_ty,w,2);

			// update last gridline coordinate 
			master_transport_last_y=i_ty;
		}

		// end clip
		arg_gdi_end_clip_rgn(hdc);
	}
}
