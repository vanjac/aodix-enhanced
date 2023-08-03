/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Paint Routing
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_routing(HWND const hwnd,HDC const hdc,int const w,int const h)
{
    if(user_page==1 && h>232)
	{
        // temporal text buffer holder
        char buf_a[256];

        // instance current selected instance pointer
        ADX_INSTANCE* pi=&instance[user_instance];

        // back color
        COLORREF const bck_color=GetPixel(hdc_gfx,0,0);

        // alphablend structure
        BLENDFUNCTION bf;
        bf.AlphaFormat=0;
        bf.BlendFlags=0;
        bf.BlendOp=0;

		// routing view client area
		int const rout_area_y=232;
		int const rout_area_h=h-rout_area_y;

		// rout clip region
		arg_gdi_set_clip_rgn(hdc,0,rout_area_y,w,rout_area_h);

		// paint back
		arg_gdi_paint_solid_rect(hdc,0,rout_area_y,w,rout_area_h,bck_color);

		// paint rout horizontal grid
		for(int xg=0;xg<w;xg+=64)
			arg_gdi_paint_solid_rect(hdc,xg-(user_rout_offset_x%64),rout_area_y,1,rout_area_h,bck_color+0x00080808);

		// paint rout vertical grid
		for(int yg=0;yg<rout_area_h;yg+=64)
			arg_gdi_paint_solid_rect(hdc,0,rout_area_y+yg-(user_rout_offset_y%64),w,1,bck_color+0x00080808);

		// paint rout routing center offset-axis
		arg_gdi_paint_solid_rect(hdc,-user_rout_offset_x,rout_area_y,1,rout_area_h,bck_color+0x00202020);
		arg_gdi_paint_solid_rect(hdc,0,rout_area_y-user_rout_offset_y,w,1,bck_color+0x00202020);
		arg_gdi_paint_solid_rect(hdc,-user_rout_offset_x,rout_area_y-user_rout_offset_y,1,1,bck_color+0x00808080);

		// get master input module screen coordinates
		int const m_i_x=master_i_x-user_rout_offset_x;
		int const m_i_y=rout_area_y+master_i_y-user_rout_offset_y;

		// get master output module screen coordinates
		int const m_o_x=master_o_x-user_rout_offset_x;
		int const m_o_y=rout_area_y+master_o_y-user_rout_offset_y;

		// paint master input box
		BitBlt(hdc,m_i_x,m_i_y,256,16,hdc_gfx,64,16+(user_pressed==37)*16,SRCCOPY);

		// paint master input connection panel
		for(int i=0;i<NUM_DSP_INPUTS;i++)
			BitBlt(hdc,m_i_x+i*8,m_i_y+16,8,16,hdc_gfx,112+(cfg.asio_input_pin[i]>=asio_num_inputs)*16,0,SRCCOPY);

		// paint master input selected pin
		BitBlt(hdc,m_i_x+user_input_pin*8,m_i_y+16,8,8,hdc_gfx,104,0,SRCCOPY);

		// paint master input pin array
		paint_pin_array(hdc,m_i_x,m_i_y+28,NUM_DSP_INPUTS,master_input_pin,m_o_x,m_o_y,rout_area_y);

		// paint master output box
		BitBlt(hdc,m_o_x,m_o_y+8,256,16,hdc_gfx,64,48+(user_pressed==38)*16,SRCCOPY);

		// paint master output connection panel
		for(int o=0;o<NUM_DSP_OUTPUTS;o++)
			BitBlt(hdc,m_o_x+o*8,m_o_y,8,8,hdc_gfx,120+(cfg.asio_output_pin[o]>=asio_num_outputs)*8,0,SRCCOPY);

		// paint master output selected pin
		BitBlt(hdc,m_o_x+user_output_pin*8,m_o_y,8,8,hdc_gfx,104,0,SRCCOPY);

		// text blck color
		SetTextColor(hdc,0);

		// paint all instance boxes
		for(int i=0;i<MAX_INSTANCES;i++)
		{
			// get instance pointer
			ADX_INSTANCE* pi=&instance[i];

			// check if instance is loaded
			if(pi->peffect!=NULL)
			{
				// get num in and out pins (audio + midi)
				int const i_ni=pi->peffect->numInputs+1;
				int const i_no=pi->peffect->numOutputs+1;

				// get instance module screen coordinates and box width
				int const i_x=pi->x-user_rout_offset_x;
				int const i_y=rout_area_y+pi->y-user_rout_offset_y;
				int const i_w=arg_tool_clipped_assign(max(i_ni,i_no)*8,128,MAX_SIGNED_INT);

				// blit instance box
				BitBlt(hdc,i_x,i_y,i_w-2,48,hdc_gfx,0,672,SRCCOPY);
				BitBlt(hdc,i_x+i_w-2,i_y,2,48,hdc_gfx,262,672,SRCCOPY);

				// get instance current program label
				buf_a[0]=0;
				pi->peffect->dispatcher(pi->peffect,effGetProgramName,0,0,buf_a,0.0f);

				// paint instance labels
				TextOut(hdc,i_x+36,i_y+12,pi->alias,min(strlen(pi->alias),(i_w-40)/6));
				TextOut(hdc,i_x+4,i_y+28,buf_a,min(strlen(buf_a),(i_w-8)/6));

				// paint machine audio inputs
				for(int in=0;in<pi->peffect->numInputs;in++)
					BitBlt(hdc,i_x+in*8,i_y,8,8,hdc_gfx,64,0,SRCCOPY);

				// paint machine midi input pin
				BitBlt(hdc,i_x+pi->peffect->numInputs*8,i_y,8,8,hdc_gfx,80,0,SRCCOPY);

				// paint machine audio outputs
				for(int ou=0;ou<pi->peffect->numOutputs;ou++)
					BitBlt(hdc,i_x+ou*8,i_y+40,8,8,hdc_gfx,72,0,SRCCOPY);

				// paint machine midi output pin
				BitBlt(hdc,i_x+pi->peffect->numOutputs*8,i_y+40,8,8,hdc_gfx,80,0,SRCCOPY);

				// instance list mute
				if(pi->process_mute)
					BitBlt(hdc,i_x,i_y+8,16,16,hdc_gfx,192,0,SRCCOPY);

				// instance list thru (bypass)
				if(pi->process_thru)
					BitBlt(hdc,i_x+16,i_y+8,16,16,hdc_gfx,208,0,SRCCOPY);

				// is current instance
				if(user_instance==i)
				{
					// alphablend selected machine
					bf.SourceConstantAlpha=64;
					AlphaBlend(hdc,i_x,i_y,i_w,48,hdc_gfx,240,0,16,16,bf);
				}

				// paint instance audio output pin(s) array
				paint_pin_array(hdc,i_x,i_y+44,pi->peffect->numOutputs,pi->pout_pin,m_o_x,m_o_y,rout_area_y);

				// paint midi-out link wires
				for(int w=0;w<pi->mout_pin.num_wires;w++)
				{
					// midi-out link source-destination screen coordinates
					int const mol_x1=i_x+pi->peffect->numOutputs*8;
					int const mol_y1=i_y+40;

					// get midi-out wire
					ADX_WIRE* pw=&pi->mout_pin.pwire[w];

					// get midi-out link destination instance
					ADX_INSTANCE* pi_mol=&instance[pw->instance_index];

					// check that destination instance is loaded
					if(pi_mol->peffect!=NULL)
					{
						int const mol_x2=pi_mol->x-user_rout_offset_x+4+pi_mol->peffect->numInputs*8;
						int const mol_y2=rout_area_y+pi_mol->y-user_rout_offset_y+4;

						// paint midi-out link wire
						paint_wire(hdc,mol_x1+4,mol_y1+4,mol_x2,mol_y2,0x0080FFFF);
					}
				}
			}
		}

		// paint dragging midi-out link wire
		if(user_pressed==25)
		{
			// get instance wire source
			ADX_INSTANCE* pi=&instance[user_dragging_rout_instance_index];

			// get instance module screen coordinates
			int const i_x=pi->x-user_rout_offset_x;
			int const i_y=rout_area_y+pi->y-user_rout_offset_y;

			// paint wire
			paint_wire(hdc,i_x+4+pi->peffect->numOutputs*8,i_y+44,user_lxm,user_lym,0x00FFFFFF);
		}

		// paint dragging instance audio wire
		if(user_pressed==36)
		{
			// instance wire
			if(user_dragging_rout_instance_index<MAX_INSTANCES)
			{
				// get instance wire source
				ADX_INSTANCE* pi=&instance[user_dragging_rout_instance_index];

				// get instance module screen coordinates
				int const i_x=pi->x-user_rout_offset_x;
				int const i_y=rout_area_y+pi->y-user_rout_offset_y;

				// paint wire
				paint_wire(hdc,i_x+4+user_dragging_rout_pin_index*8,i_y+44,user_lxm,user_lym,0x00FFFFFF);
			}

			// paint dragging master-input wire
			if(user_dragging_rout_instance_index==INPUT_INSTANCE)
				paint_wire(hdc,m_i_x+4+user_dragging_rout_pin_index*8,m_i_y+28,user_lxm,user_lym,0x00FFFFFF);
		}

		// paint wire gain bar
		if(user_pressed==44)
		{
			// fill ab struct
			bf.SourceConstantAlpha=128;
			AlphaBlend(hdc,user_pressed_wire_x-18,user_pressed_wire_y-66,37,133,hdc_gfx,576,0,37,133,bf);
			
			// get inversed gain
			float const inv_gain=1.0-user_pressed_wire->value;

			// integer gain offset
			int const i_gain=int(inv_gain*112.0f);

			// slider position
			int const s_xp=user_pressed_wire_x-15;
			int const s_yp=(user_pressed_wire_y-63)+i_gain;

			// blit gain slider
			BitBlt(hdc,s_xp,s_yp,32,16,hdc_gfx,613,0,SRCCOPY);
		
			// format gain text
			sprintf(buf_a,"%.3d%%",int(user_pressed_wire->value*100.0f));
			paint_txt(hdc,s_xp+3,s_yp+4,buf_a,4,3);
		}

		// end clip
		arg_gdi_end_clip_rgn(hdc);
	}
}
