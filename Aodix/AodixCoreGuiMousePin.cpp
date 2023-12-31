/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Gui (Scan Pin) Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::gui_mouse_scan_pin(HWND const hwnd,int const xm,int const ym,ADX_PIN* ppins,int const num_pins,int const pin_x,int const pin_y,int const is_double_click,int const is_midi_pin)
{
	// get dsp routing area
	int const rout_area_y=232;

	// get master output module screen coordinates
	int const m_o_x=master_o_x-user_rout_offset_x;
	int const m_o_y=rout_area_y+master_o_y-user_rout_offset_y;

	// scan pins
	for(int p=(num_pins-1);p>=0;p--)
	{
		// get audio output pin pointer
		ADX_PIN* pp=&ppins[p];

		// scan wires
		for(int w=(pp->num_wires-1);w>=0;w--)
		{
			// check flag
			bool check=false;

			// get audio output wire pointer
			ADX_WIRE* pw=&pp->pwire[w];

			// stack coordinates
			int const x1=pin_x+p*8+4;
			int const y1=pin_y+4;
			int x2=0;
			int y2=0;

			// wire connected to master output, paint wire
			if(is_midi_pin==0 && pw->instance_index==MAX_INSTANCES)
			{
				// set wire coordinates
				x2=m_o_x+pw->pin_index*8+4;
				y2=m_o_y+4;

				// set check flag
				check=true;
			}

			// wire connected to machine
			if(pw->instance_index<MAX_INSTANCES)
			{
				// get dest machine
				ADX_INSTANCE* pi=&instance[pw->instance_index];

				// check if effect is loaded
				if(pi->peffect!=NULL)
				{
					// get destination instance module screen coordinates
					int const d_i_x=0+pi->x-user_rout_offset_x;
					int const d_i_y=rout_area_y+pi->y-user_rout_offset_y;

					// set wire coordinates (audio pin)
					if(is_midi_pin==0)
					{
						x2=d_i_x+4+pw->pin_index*8;
						y2=d_i_y+4;
					}

					// set wire coordinates (midi pin)
					if(is_midi_pin==1)
					{
						x2=d_i_x+4+pi->peffect->numInputs*8;
						y2=d_i_y+4;
					}

					// set check flag
					check=true;
				}
			}

			// check coordinates
			if(check)
			{
				// get vector
				float const d_vw=float(x2-x1);
				float const d_vh=float(y2-y1);

				// get modulus
				float const d_mo=sqrtf(d_vw*d_vw+d_vh*d_vh);

				// is modulus higher than 1.0
				if(d_mo>1.0f)
				{
					// get normalized values
					float const d_xr=(d_vw/d_mo)*8.0f;
					float const d_yr=(d_vh/d_mo)*8.0f;

					// get center
					float const d_cx=float(x1)+d_vw*0.5f;
					float const d_cy=float(y1)+d_vh*0.5f;

					// get integer center
					int const i_cx=int(d_cx);
					int const i_cy=int(d_cy);

					// polygon points
					float xp[3];
					float yp[3];

					// get the three polygon coordinates
					xp[0]=d_cx+d_xr;
					yp[0]=d_cy+d_yr;
					xp[1]=d_cx+d_yr;
					yp[1]=d_cy-d_xr;
					xp[2]=d_cx-d_yr;
					yp[2]=d_cy+d_xr;

					// check
					if(arg_tool_check_point_in_2d_poly(3,xp,yp,float(xm),float(ym)))
					{
						// check double click state
						if(is_double_click)
						{
							// get wire target index
							int const clicked_wire_instance_index=pw->instance_index;

							// check control key
							if(GetKeyState(VK_CONTROL)<0)
							{
								// enter critical section
								asio_enter_cs();

								// delete all wires going to the same destination in instance
								for(int ps=0;ps<num_pins;ps++)
								{
									// get scan pins pointer
									ADX_PIN* psp=&ppins[ps];

									// scan wires
									for(int ws=0;ws<psp->num_wires;ws++)
									{
										// check wire destination
										if(psp->pwire[ws].instance_index==clicked_wire_instance_index)
											edit_del_wire(psp,ws--);
									}
								}

								// leave critical section
								asio_leave_cs();
							}
							else
							{
								// delete single wire
								edit_del_wire(pp,w);
							}

							// notify and return
							user_pressed=PRESS_WIRE_DELETE;
							return;
						}
						else
						{
							// set 'wire-gain tweak' user mode
							user_pressed_wire=pw;
							user_pressed_wire_x=i_cx;
							user_pressed_wire_y=i_cy;
							user_pressed=PRESS_WIRE_GAIN;
							return;
						}
					}
				}
			}
		}
	}
}
