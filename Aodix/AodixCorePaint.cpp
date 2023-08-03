/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Paint Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_master_vumeter(HDC const hdc)
{
	// num peaks
	int const num_peaks=dsp_highest_opin_count+1;

	// vumeter height
	int const vumeter_height=64/num_peaks;

	// dynamic range (in db)
	float const db_range=84.0f;

	// scan peaks
	for(int pk=0;pk<num_peaks;pk++)
	{
		// get linear peak level
		float const linear_peak_level=dsp_output_vumeter[pk];

		// vumeter peak width
		int vumeter_width=0;

		// get db magnitude
		if(linear_peak_level>0.0f)
		{
			// get peak graphic magnitude
			float const peak_db=20.0f*log10f(linear_peak_level);
			float const peak_wd=(db_range+peak_db)*(213.0f/db_range);

			// set vu width
			vumeter_width=arg_tool_clipped_assign((int)peak_wd,0,213);
		}

		// vumeter y position
		int const vu_y=31+(vumeter_height*pk);

		// paint vumeter bar
		if(vumeter_width>0)
			BitBlt(hdc,798,vu_y,vumeter_width,vumeter_height-1,hdc_gfx,64,304,SRCCOPY);
		
		// paint remaining bar
		if(vumeter_width<213)
			BitBlt(hdc,798+vumeter_width,vu_y,213-vumeter_width,vumeter_height-1,hdc_gfx,64+vumeter_width,336,SRCCOPY);

		// clipping peak led
		BitBlt(hdc,1012,vu_y,4,vumeter_height-1,hdc_gfx,278,336-(linear_peak_level>1.0f)*32,SRCCOPY);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_block_shade(HDC const hdc,int const x,int const y,int const w,int const h,int const lx1,int const ly1,int const lx2,int const ly2)
{
	// alphablend structure
	BLENDFUNCTION bf;

	// fill ab struct
	bf.AlphaFormat=0;
	bf.BlendFlags=0;
	bf.BlendOp=0;
	bf.SourceConstantAlpha=64;

	// get rectangle coordinates
	int x1=x;
	int y1=y;
	int x2=x+w;
	int y2=y+h;

	// clamp rectangle coordinates
	if(x1<lx1)
		x1=lx1;

	if(y1<ly1)
		y1=ly1;

	if(x2>lx2)
		x2=lx2;

	if(y2>ly2)
		y2=ly2;

	// get coordinates
	int const i_bm_x=x1;
	int const i_bm_y=y1;
	int const i_bm_w=x2-x1;
	int const i_bm_h=y2-y1;

	// check block size and mark
	if(i_bm_w>0 && i_bm_h>0)
	{
		// frame
		arg_gdi_paint_solid_rect(hdc,i_bm_x,i_bm_y,i_bm_w,1,0x00FFFFFF);
		arg_gdi_paint_solid_rect(hdc,i_bm_x,i_bm_y,1,i_bm_h,0x00FFFFFF);
		arg_gdi_paint_solid_rect(hdc,i_bm_x+1,i_bm_y+i_bm_h-1,i_bm_w-1,1,0x00FFFFFF);
		arg_gdi_paint_solid_rect(hdc,i_bm_x+i_bm_w-1,i_bm_y+1,1,i_bm_h-1,0x00FFFFFF);

		// alphablend edit block shade
		AlphaBlend(hdc,i_bm_x,i_bm_y,i_bm_w,i_bm_h,hdc_gfx,256,0,16,16,bf);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_txt(HDC const hdc,int const x,int const y,char* txt,int const num_chars,int const type)
{
	// set source type origin
	int const yo=588+type*8;

	// blit char string
	int c;
	for(c=0;c<num_chars && txt[c];c++)
		BitBlt(hdc,x+c*6,y,6,8,hdc_gfx,txt[c]*6,yo,SRCCOPY);

	// after null, draw blank characters for remaining space
	for(;c<num_chars;c++)
		BitBlt(hdc,x+c*6,y,6,8,hdc_gfx,0,       yo,SRCCOPY);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_seq_pos_big(HDC const hdc,int const x,int const y,int const pos)
{
	// get bar, beat and ppq position
	int const bar=pos/(project.master_numerator*project.master_ppqn);
	int const bea=(pos/project.master_ppqn)%project.master_numerator;
	int const ppq=pos%project.master_ppqn;

	// get bar digits
	int const bar_dig_c=bar/100;
	int const bar_dig_d=(bar/10)%10;
	int const bar_dig_u=bar%10;

	// get peak digits
	int const ppq_dig_c=ppq/100;
	int const ppq_dig_d=(ppq/10)%10;
	int const ppq_dig_u=ppq%10;

	// blit digits
	BitBlt(hdc,x+00,y,12,16,hdc_gfx,64+bar_dig_c*12,80,SRCCOPY);
	BitBlt(hdc,x+13,y,12,16,hdc_gfx,64+bar_dig_d*12,80,SRCCOPY);
	BitBlt(hdc,x+26,y,12,16,hdc_gfx,64+bar_dig_u*12,80,SRCCOPY);
	BitBlt(hdc,x+38,y,12,16,hdc_gfx,184,80,SRCCOPY);
	BitBlt(hdc,x+50,y,12,16,hdc_gfx,64+bea*12,80,SRCCOPY);
	BitBlt(hdc,x+62,y,12,16,hdc_gfx,184,80,SRCCOPY);
	BitBlt(hdc,x+74,y,12,16,hdc_gfx,64+ppq_dig_c*12,80,SRCCOPY);
	BitBlt(hdc,x+87,y,12,16,hdc_gfx,64+ppq_dig_d*12,80,SRCCOPY);
	BitBlt(hdc,x+100,y,12,16,hdc_gfx,64+ppq_dig_u*12,80,SRCCOPY);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_event(HDC const hdc,int const x,int const y,int const w,int const h,COLORREF const text_color,COLORREF const back_color,char* text)
{
	// background
	arg_gdi_paint_solid_rect(hdc,x,y,w,h,back_color);

	// big event frame
	arg_gdi_paint_solid_rect(hdc,x,y,1,1,back_color+ARG_GDI_3D_CLR_OFFSET+ARG_GDI_3D_CLR_OFFSET);
	arg_gdi_paint_solid_rect(hdc,x+1,y,w-2,1,back_color+ARG_GDI_3D_CLR_OFFSET);
	arg_gdi_paint_solid_rect(hdc,x,y+1,1,h-2,back_color+ARG_GDI_3D_CLR_OFFSET);
	arg_gdi_paint_solid_rect(hdc,x+1,y+h-1,w-2,1,back_color-ARG_GDI_3D_CLR_OFFSET);
	arg_gdi_paint_solid_rect(hdc,x+w-1,y+1,1,h-2,back_color-ARG_GDI_3D_CLR_OFFSET);
	arg_gdi_paint_solid_rect(hdc,x+w-1,y+h-1,1,1,back_color-ARG_GDI_3D_CLR_OFFSET-ARG_GDI_3D_CLR_OFFSET);

	// paint event text
	SetTextColor(hdc,text_color);
	SetBkColor(hdc,back_color);

	// text out event
	TextOut(hdc,x+3,y+2,text,15);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_marker(HDC const hdc,COLORREF const color,int const type,char* label,int const y,int const w,int const label_offset,int const selected)
{
	// paint marker dotted line
	for(int x=0;x<w;x+=4)
		arg_gdi_paint_solid_rect(hdc,x,y,2,1,color);

	// paint marker
	BitBlt(hdc,0,y+label_offset,TRACK_WIDTH,16,hdc_gfx,320+selected*TRACK_WIDTH,368+type*16,SRCCOPY);

	// paint label
	paint_txt(hdc,38,y+label_offset+4,label,9,selected*4);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_wire(HDC const hdc,int const x1,int const y1,int const x2,int const y2,COLORREF const clr)
{	
	// get vector 
	double const d_vw=double(x2-x1);
	double const d_vh=double(y2-y1);

	// get modulus
	double const d_mo=sqrt(d_vw*d_vw+d_vh*d_vh);

	// paint line wire
	arg_gdi_paint_line(hdc,x1,y1,x2,y2,PS_SOLID,clr);

	// is modulus higher than 1.0
	if(d_mo>1.0)
	{
		// get normalized values
		double const d_xr=(d_vw/d_mo)*8.0;
		double const d_yr=(d_vh/d_mo)*8.0;

		// get center
		double const d_cx=double(x1)+d_vw*0.5;
		double const d_cy=double(y1)+d_vh*0.5;

		// polygon points
		POINT pnt[3];

		// get the three arrow coordinates
		pnt[0].x=(long)(d_cx+d_xr);
		pnt[0].y=(long)(d_cy+d_yr);
		pnt[1].x=(long)(d_cx+d_yr);
		pnt[1].y=(long)(d_cy-d_xr);
		pnt[2].x=(long)(d_cx-d_yr);
		pnt[2].y=(long)(d_cy+d_xr);

		// point polygon
		Polygon(hdc,pnt,3);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_pin_array(HDC const hdc,int const x,int const y,int const num_pins,ADX_PIN* ppin,int const master_o_x,int const master_o_y,int const rout_area_y)
{
	// paint pins
	for(int p=0;p<num_pins;p++)
	{
		// get pin pointer
		ADX_PIN* pp=&ppin[p];

		// scan wire
		for(int w=0;w<pp->num_wires;w++)
		{
			// get wire pointer
			ADX_WIRE* pw=&pp->pwire[w];

			// get wire colour
			int const clr_scl=int(pw->value*255.0f);

			// wire colour code
			COLORREF const clr_code=clr_scl*0x00010101;

			// create and select the brush that will be used to fill the polygon
			HBRUSH const hbrush=CreateSolidBrush(clr_code);
			
			// select brush
			HBRUSH const hbrush_old=(HBRUSH)SelectObject(hdc,hbrush);

			// wire connected to master output, paint wire
			if(pw->instance_index==MAX_INSTANCES)
				paint_wire(hdc,x+p*8+4,y,master_o_x+pw->pin_index*8+4,master_o_y+4,0);

			// wire connected to machine
			if(pw->instance_index<MAX_INSTANCES)
			{
				// get dest machine
				ADX_INSTANCE* pi=&instance[pw->instance_index];

				// check if effect is loaded
				if(pi->peffect!=NULL)
				{
					// get instance module screen coordinates
					int const i_x=0+pi->x-user_rout_offset_x;
					int const i_y=rout_area_y+pi->y-user_rout_offset_y;

					// paint wire
					paint_wire(hdc,x+p*8+4,y,i_x+4+pw->pin_index*8,i_y+4,0);
				}
			}

			// select old hbrush object
			SelectObject(hdc,hbrush_old);

			// delete brush
			DeleteObject(hbrush);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_track_midi_vumeters(HDC const hdc,int const x,int const y,int const visible_tracks)
{
	// draw track frames
	for(int t=0;t<visible_tracks;t++)
	{
		// get track index
		int const ti=user_trk_offset+t;

		// decide whenever track is visible
		if(ti<MAX_TRACKS)
		{
			// blit vumeter bar
			BitBlt(hdc,x+34+t*TRACK_WIDTH,y,4,12,hdc_gfx,256+(master_midi_vumeter[ti]/8)*4,80,SRCCOPY);

			// decay track vumeter
			master_midi_vumeter[ti]=arg_tool_clipped_assign(master_midi_vumeter[ti]-6,0,128);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint(HWND const hwnd,HDC const hdc,int const w,int const h)
{
	// temporal text buffer holder
	char buf_a[256];

	// instance current selected instance pointer
	ADX_INSTANCE* pi=&instance[user_instance];

	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// get user quantization
	int const i_quantize=edit_get_quantization();

	// get master pos
	int const i_master_play_pos=seq_sample_to_pos(master_transport_sampleframe);

	// set font, get default font
	HFONT const hfont_default=(HFONT)SelectObject(hdc,(HFONT)hfont_terminal);

	// set transparent text mode
	SetBkMode(hdc,TRANSPARENT);

	// master timer		
	gui_format_time(master_transport_sampleframe,buf_a);
	paint_txt(hdc,135,21,buf_a,9,2);

	// master cpu
	sprintf(buf_a,"%.2d.%.1d",int(dsp_cpu_cost),int(dsp_cpu_cost*10.0)%10);
	paint_txt(hdc,159,37,buf_a,4,2);

	// master transport cycle switch
	BitBlt(hdc,195,17,32,16,hdc_gfx,64,240+(user_pressed==1)*16+(master_time_info.flags & kVstTransportCycleActive)*8,SRCCOPY);

	// master config rec-live switch
	BitBlt(hdc,227,17,32,16,hdc_gfx,96,240+(user_pressed==2)*16+cfg.rec_live*32,SRCCOPY);

	// master config stop wrap
	BitBlt(hdc,195,33,64,16,hdc_gfx,128,240+(user_pressed==3)*16+cfg.stop_wrap*32,SRCCOPY);

	// master tempo
	sprintf(buf_a,"%.3d.%.1d",int(project.master_tempo),int(project.master_tempo*10.0)%10);
	paint_txt(hdc,71,53,buf_a,5,0);
	BitBlt(hdc,115,49,16,16,hdc_gfx,16+(user_pressed==4)*16,32,SRCCOPY);

	// master ppqn
	sprintf(buf_a,"%.3d",project.master_ppqn);
	paint_txt(hdc,199,53,buf_a,3,0);

	// master signature
	sprintf(buf_a,"%d:%d",project.master_numerator,project.master_denominator);
	paint_txt(hdc,71,69,buf_a,3,0);

	// master block size
	sprintf(buf_a,"%.6d",dsp_block_size);
	paint_txt(hdc,199,69,buf_a,6,0);

	// master position
	paint_seq_pos_big(hdc,10,25,i_master_play_pos);

	// master transport play
	BitBlt(hdc,3,85,64,28,hdc_gfx,64,96+(user_pressed==7)*28+(master_time_info.flags & kVstTransportPlaying)*28,SRCCOPY);

	// master transport stop
	BitBlt(hdc,67,85,64,28,hdc_gfx,128,96+(user_pressed==8)*28,SRCCOPY);

	// master transport record events
	BitBlt(hdc,131,85,64,28,hdc_gfx,192,96+(user_pressed==9)*28+user_record*56,SRCCOPY);

	// master transport record automation
	BitBlt(hdc,195,85,64,28,hdc_gfx,256,96+(user_pressed==10)*28+((master_time_info.flags & kVstAutomationWriting)>0)*56,SRCCOPY);

	// edit pos
	paint_seq_pos_big(hdc,10,141,pp->usr_pos);
	
	// edit timer
	gui_format_time(seq_pos_to_sample(pp->usr_pos),buf_a);
	paint_txt(hdc,135,137,buf_a,9,2);

	// edit seq num events
	sprintf(buf_a,"%.6d",seq_num_events);
	paint_txt(hdc,135,153,buf_a,6,2);

	// edit step switch
	BitBlt(hdc,195,133,32,16,hdc_gfx,192,240+(user_pressed==11)*16+user_edit_step*32,SRCCOPY);

	// edit overwrite switch
	BitBlt(hdc,227,133,32,16,hdc_gfx,224,240+(user_pressed==12)*16+user_edit_overwrite*32,SRCCOPY);

	// edit sequencer/routing page switch
	BitBlt(hdc,195,149,64,16,hdc_gfx,256,240+(user_pressed==13)*16+user_page*32,SRCCOPY);

	// edit quantization
	sprintf(buf_a,"%.4d",i_quantize);
	paint_txt(hdc,71,169,buf_a,4,0);

	// edit midi channel
	sprintf(buf_a,"%.1X (%.2d)",user_midi_ch,user_midi_ch+1);
	paint_txt(hdc,71,185,buf_a,6,0);

	// edit kbd note offset
	sprintf(buf_a,"%d (%.2d)",user_kbd_note_offset/12,user_kbd_note_offset);
	paint_txt(hdc,71,201,buf_a,6,0);

	// edit midi mask
	sprintf(buf_a,"%X",user_midi_mask);
	paint_txt(hdc,71,217,buf_a,2,0);

	// edit pattern sequencer zoom
	sprintf(buf_a,"1:%.3d",pp->usr_ppp);
	paint_txt(hdc,199,169,buf_a,5,0);
	BitBlt(hdc,243,165,16,16,hdc_gfx,16+(user_pressed==14)*16,32,SRCCOPY);

	// edit mode switch (notes or patterns)
	if(pp->usr_mod==0)
		paint_txt(hdc,199,185,"Event  ",7,0);
	else
		paint_txt(hdc,199,185,"Pattern",7,0);

	// edit mode switch button
	BitBlt(hdc,243,181,16,16,hdc_gfx,16+(user_pressed==15)*16,80,SRCCOPY);

	// edit note velocity
	sprintf(buf_a,"%.2X(%.3d)",user_kbd_velo,user_kbd_velo);
	paint_txt(hdc,199,201,buf_a,7,0);
	BitBlt(hdc,243,197,16,16,hdc_gfx,16+(user_pressed==16)*16,32,SRCCOPY);

	// edit note pre-release
	sprintf(buf_a,"%.4d",pp->usr_pre);
	paint_txt(hdc,199,217,buf_a,4,0);
	BitBlt(hdc,243,213,16,16,hdc_gfx,16+(user_pressed==17)*16,32,SRCCOPY);

	// edit display current instance index
	sprintf(buf_a,"%.2X",user_instance);
	paint_txt(hdc,333,201,buf_a,2,0);

	// edit display current instance alias
	paint_txt(hdc,353,201,pi->alias,19,0);

	// edit current instance spinners
	BitBlt(hdc,473,197,16,16,hdc_gfx,16+(user_pressed==18)*16,0,SRCCOPY);
	BitBlt(hdc,489,197,16,16,hdc_gfx,16+(user_pressed==19)*16,16,SRCCOPY);

	// edit current instance ghosted program spinners and program menu arrow
	BitBlt(hdc,473,213,16,16,hdc_gfx,48,0,SRCCOPY);
	BitBlt(hdc,489,213,16,16,hdc_gfx,48,16,SRCCOPY);
	BitBlt(hdc,505,213,16,16,hdc_gfx,48,48,SRCCOPY);

	// check if instance is loaded
	if(pi->peffect!=NULL)
	{
		// edit display current instance program
		sprintf(buf_a,"%.2X",pi->peffect->dispatcher(pi->peffect,effGetProgram,0,0,NULL,0.0f));
		paint_txt(hdc,333,217,buf_a,2,0);

		// get program name
		memset(buf_a,0,24);
		pi->peffect->dispatcher(pi->peffect,effGetProgramName,0,0,buf_a,0.0f);
		paint_txt(hdc,353,217,buf_a,19,0);

		// check number of programs
		if(pi->peffect->numPrograms>1)
		{
			// edit current instance program spinners and program menu arrow
			BitBlt(hdc,473,213,16,16,hdc_gfx,16+(user_pressed==20)*16,0,SRCCOPY);
			BitBlt(hdc,489,213,16,16,hdc_gfx,16+(user_pressed==21)*16,16,SRCCOPY);
			BitBlt(hdc,505,213,16,16,hdc_gfx,16,48,SRCCOPY);
		}
	}
	else
	{
		// no instance loaded (no program)
		paint_txt(hdc,333,217,"--",2,0);
		paint_txt(hdc,353,217,"---                ",19,0);
	}

	// blit instance list scrollbar
	BitBlt(hdc,265,17,16,176,hdc_gfx,48,96,SRCCOPY);
	BitBlt(hdc,265,17+(user_instance_list_offset*32)/49,16,16,hdc_gfx,16+(user_pressed==22)*16,96,SRCCOPY);
	
	// vst instance list entries
	for(int i=0;i<11;i++)
	{
		// get instance index
		int const ii=user_instance_list_offset+i;

		// instance selected flag
		int const isf=(ii==user_instance);

		// screen position
		int const iy=17+i*16;

		// get instance pointer
		pi=&instance[ii];

		// paint selected/unselected instance item list entry background
		BitBlt(hdc,281,iy,240,16,hdc_gfx,64,416-isf*16,SRCCOPY);

		// format instance index
		sprintf(buf_a,"%.2X",ii);
		paint_txt(hdc,284,iy+4,buf_a,2,isf);

		// paint alias
		paint_txt(hdc,304,iy+4,pi->alias,14,isf);

		// clear buf_a
		memset(buf_a,0,14);

		// get plugin program label (if instanced)
		if(pi->peffect!=NULL)
			pi->peffect->dispatcher(pi->peffect,effGetProgramName,0,0,buf_a,0.0f);
		else
			sprintf(buf_a,"---");

		// paint program label
		paint_txt(hdc,398,iy+4,buf_a,14,isf);

		// instance list mute
		if(pi->process_mute)
			BitBlt(hdc,489,iy,16,16,hdc_gfx,192,0,SRCCOPY);

		// instance list thru (bypass)
		if(pi->process_thru)
			BitBlt(hdc,505,iy,16,16,hdc_gfx,208,0,SRCCOPY);
	}

	// get current selected instance
	pi=&instance[user_instance];

	// check if instance is loaded
	if(pi->peffect!=NULL)
	{
		// blit parameter list scrollbar background
		BitBlt(hdc,527,17,16,176,hdc_gfx,48,96,SRCCOPY);

		// get parameter list slider screen position
		if(pi->peffect->numParams>11)
			BitBlt(hdc,527,17+(user_parameter_list_offset*160)/(pi->peffect->numParams-11),16,16,hdc_gfx,16+(user_pressed==23)*16,96,SRCCOPY);

		// display parameter(s) entries
		for(int pil=0;pil<11;pil++)
		{
			// get parameter index
			int const param_index=user_parameter_list_offset+pil;

			// get parameter y-screen coordinate
			int const iy=17+pil*16;		

			// get labels (if param is available)
			if(param_index<pi->peffect->numParams)
			{
				// parameter selected flag
				int const psf=(param_index==user_parameter);

				// blit parameter background
				BitBlt(hdc,543,iy,240,16,hdc_gfx,64,448-psf*16,SRCCOPY);

				// blit parameter index MSB
				sprintf(buf_a,"%.2X",param_index>>8);
				paint_txt(hdc,546,iy+4,buf_a,2,psf);

				// blit parameter index LSB
				sprintf(buf_a,"%.2X",param_index&0xFF);
				paint_txt(hdc,565,iy+4,buf_a,2,psf);

				// blit parameter name
				memset(buf_a,0,12);
				pi->peffect->dispatcher(pi->peffect,effGetParamName,param_index,0,buf_a,0.0f);
				paint_txt(hdc,584,iy+4,buf_a,11,psf);

				// blit parameter label
				memset(buf_a,0,12);
				pi->peffect->dispatcher(pi->peffect,effGetParamDisplay,param_index,0,buf_a,0.0f);
				paint_txt(hdc,660,iy+4,buf_a,11,psf);

				// blit parameter midi cc#
				if(pi->pmidi_cc[param_index]<255)
				{
					sprintf(buf_a,"#%.3d",pi->pmidi_cc[param_index]);
					paint_txt(hdc,739,iy+4,buf_a,4,psf);
				}

				// select knob frame
				float const f_frame=pi->peffect->getParameter(pi->peffect,param_index)*128.0f;
				int const i_frame=arg_tool_clipped_assign(int(f_frame),0,127);

				// draw parameter knob
				BitBlt(hdc,768,iy+1,14,14,hdc_knb,0,i_frame*14,SRCCOPY);
			}
			else
			{
				// out of param index
				BitBlt(hdc,543,iy,240,16,hdc_gfx,64,464,SRCCOPY);
			}
		}
	}
	else
	{
		// not instance loaded
		BitBlt(hdc,527,17,256,176,hdc_gfx,320,0,SRCCOPY);
	}

	// draw master output vumeter background
	BitBlt(hdc,789,17,232,92,hdc_gfx,64,480,SRCCOPY);

	// paint master vumeter
	paint_master_vumeter(hdc);

	// project information
	paint_txt(hdc,857,201,project.name,26,0);
	paint_txt(hdc,857,217,project.info,26,0);

	// midi-input information
	BitBlt(hdc,853,177,32,16,hdc_gfx,64+(p_hmidi_in!=NULL)*32,572,SRCCOPY);

	// last midi-in message monitor
	paint_txt(hdc,921,181,user_midi_in_monitor,8,0);

	// sequencer view
	paint_sequencer(hwnd,hdc,w,h);

	// routing page
	paint_routing(hwnd,hdc,w,h);

	// set default font
	SelectObject(hdc,(HFONT)hfont_default);
}
