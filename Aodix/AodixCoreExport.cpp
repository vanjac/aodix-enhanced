/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Export Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::export_adx_file_dlg(HWND const hwnd)
{
	// filename holder
	char filename[_MAX_PATH];
	sprintf(filename,project.name);

	// open file dialog
	if(arg_file_dialog_save(hinstance_app,hwnd,"Save Aodix Project File",filename,"Aodix Project File (*.adx)\0*.adx\0\0","adx",""))
		export_adx_file(hwnd,filename);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::export_adx_pin(ADX_PIN* pin_array,int num_pins,FILE* pfile)
{
	// write pin data
	for(int p=0;p<num_pins;p++)
	{
		// get pin pointer
		ADX_PIN* pp=&pin_array[p];

		// write num wires
		fwrite(&pp->num_wires,sizeof(int),1,pfile);

		// write wire data
		fwrite(pp->pwire,sizeof(ADX_WIRE),pp->num_wires,pfile);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::export_adx_file(HWND const hwnd,char* filename)
{
	// set wait cursor
	SetCursor(hcursor_wait);

	// check if registered
#ifdef FULL_VERSION

	// open file
	FILE* pfile=fopen(filename,"wb");

	if(pfile!=NULL)
	{
		// enter critical section
		asio_enter_cs();

		// update last file saved
		sprintf(user_last_file,filename);

		// update recent files
		recent_files_push(filename);

		// write header id
		long const header_id='ADX4';
		fwrite(&header_id,sizeof(long),1,pfile);

		// write project version id (host version)
		fwrite(&aodix_version,sizeof(int),1,pfile);

		// write project track structure
		fwrite(&project,sizeof(ADX_PROJECT),1,pfile);

		// write num events
		fwrite(&seq_num_events,sizeof(int),1,pfile);

		// write events
		fwrite(seq_event,sizeof(ADX_EVENT),seq_num_events,pfile);

		// write instance data
		for(int i=0;i<MAX_INSTANCES;i++)
		{
			// get instance pointer
			ADX_INSTANCE* pi=&instance[i];

			// write instance data if plugin is instanced
			if(pi->peffect!=NULL)
			{
				// flag on
				fputc(1,pfile);

				// write instance path
				fwrite(pi->dll_path,sizeof(char),_MAX_PATH,pfile);

				// write effect id
				fwrite(&pi->peffect->uniqueID,sizeof(long),1,pfile);

				// write instance coordinates in dsp routing
				fwrite(&pi->x,sizeof(int),1,pfile);
				fwrite(&pi->y,sizeof(int),1,pfile);

				// write instance process flags
				fwrite(&pi->process_mute,sizeof(unsigned char),1,pfile);
				fwrite(&pi->process_thru,sizeof(unsigned char),1,pfile);

				// write instance label alias
				fwrite(pi->alias,sizeof(char),32,pfile);

				// write instance output pin data
				export_adx_pin(pi->pout_pin,pi->peffect->numOutputs,pfile);

				// write instance midi out pin data
				export_adx_pin(&pi->mout_pin,1,pfile);

				// get current instance program index
				int const curr_prg_index=pi->peffect->dispatcher(pi->peffect,effGetProgram,0,0,NULL,0.0f);

				// write current instance program index
				fwrite(&curr_prg_index,sizeof(int),1,pfile);

				// write instance chunk
				if(pi->peffect->flags & effFlagsProgramChunks)
				{
					// bank chunk data pointer
					void* pchkdata=NULL;

					// dispatch
					int const chk_byte_size=pi->peffect->dispatcher(pi->peffect,effGetChunk,0,0,&pchkdata,0.0f);

					// write chunk bytesize
					fwrite(&chk_byte_size,sizeof(int),1,pfile);

					// write chunk data
					fwrite(pchkdata,chk_byte_size,1,pfile);
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

						// write program name
						char buf_prg[32];
						buf_prg[0]=0;

						// get program name
						pi->peffect->dispatcher(pi->peffect,effGetProgramName,0,0,buf_prg,0.0f);

						// write program name
						fwrite(buf_prg,sizeof(char),32,pfile);

						// write all parameters
						for(int pa=0;pa<pi->peffect->numParams;pa++)
						{
							float const pa_value=pi->peffect->getParameter(pi->peffect,pa);
							fwrite(&pa_value,sizeof(float),1,pfile);
						}
					}

					// set current program
					pi->peffect->dispatcher(pi->peffect,effSetProgram,0,curr_prg_index,NULL,0.0f);
				}

				// write midi cc array
				if(pi->peffect->numParams)
					fwrite(pi->pmidi_cc,sizeof(unsigned char),pi->peffect->numParams,pfile);
			}
			else
			{
				// flag off, no effect instanced
				fputc(0,pfile);
			}
		}

		// write master input routing pin(s)
		export_adx_pin(master_input_pin,NUM_DSP_INPUTS,pfile);

		// write master i/o module screen coordinates
		fwrite(&master_i_x,sizeof(int),1,pfile);
		fwrite(&master_i_y,sizeof(int),1,pfile);
		fwrite(&master_o_x,sizeof(int),1,pfile);
		fwrite(&master_o_y,sizeof(int),1,pfile);

		// write user routing view offset
		fwrite(&user_rout_offset_x,sizeof(int),1,pfile);
		fwrite(&user_rout_offset_y,sizeof(int),1,pfile);

		// close file
		fclose(pfile);

		// leave critical section
		asio_leave_cs();
	}

	// end of directive
#endif

	// set arrow cursor
	SetCursor(hcursor_arro);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::export_cub_file_dlg(HWND const hwnd,int const is_fxp)
{
	// filename holder
	char filename[_MAX_PATH];
	filename[0]=0;

	int result=0;

	// save file dialog
	if(is_fxp)
		result=arg_file_dialog_save(hinstance_app,hwnd,"Export Cubase Program File",filename,"FXP Program Files (*.fxp)\0*.fxp\0\0","","");
	else	
		result=arg_file_dialog_save(hinstance_app,hwnd,"Export Cubase Bank File",filename,"FXB Program Files (*.fxb)\0*.fxb\0\0","","");

	// dialog success
	if(result)
	{
		// set wait cursor
		SetCursor(hcursor_wait);

		// enter critical section
		asio_enter_cs();

		// export cubase fxp file
		export_cub_file(hwnd,filename,is_fxp);

		// leave critical section
		asio_leave_cs();

		// post refresh
		gui_is_dirty=1;

		// set arrow cursor
		SetCursor(hcursor_arro);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::export_cub_file(HWND const hwnd,char* filename,int const is_fxp)
{
	ADX_INSTANCE* pi=&instance[user_instance];

	// effect instanced
	if(pi->peffect!=NULL)
	{
		// open file
		FILE* pfile=fopen(filename,"wb");

		// check file opening
		if(pfile!=NULL)
		{
			// write chunk magic 'CcnK'
			long const chunk_magic=import_reverse_long('CcnK');
			fwrite(&chunk_magic,sizeof(long),1,pfile);

			// write bytesize of this chunk, excl. magic + byteSize
			long const byte_size=import_reverse_long(0);
			fwrite(&byte_size,sizeof(long),1,pfile);

			// fx magic
			long fx_magic=0;

			// set fx magic for programs for banks
			if(is_fxp)
			{
				// programs with chunk or chunkless
				if(pi->peffect->flags & effFlagsProgramChunks)
					fx_magic=import_reverse_long('FPCh');
				else
					fx_magic=import_reverse_long('FxCk');
			}
			else
			{
				// banks with chunk or chunkless
				if(pi->peffect->flags & effFlagsProgramChunks)
					fx_magic=import_reverse_long('FBCh');
				else
					fx_magic=import_reverse_long('FxBk');
			}

			// write magic
			fwrite(&fx_magic,sizeof(long),1,pfile);

			// write version
			long const version=import_reverse_long(1);
			fwrite(&version,sizeof(long),1,pfile);

			// write fx unique id
			long const fx_id=import_reverse_long(pi->peffect->uniqueID);
			fwrite(&fx_id,sizeof(long),1,pfile);

			// write fx version
			long const fx_version=import_reverse_long(pi->peffect->version);
			fwrite(&fx_version,sizeof(long),1,pfile);

			// write for preset (program) (.fxp) without chunk (magic = 'FxCk')
			if(fx_magic=='kCxF')
			{
				// write num params
				long const num_params=import_reverse_long(pi->peffect->numParams);
				fwrite(&num_params,sizeof(long),1,pfile);

				// write program name
				char prg_name[28];
				prg_name[0]=0;
				pi->peffect->dispatcher(pi->peffect,effGetProgramName,0,0,prg_name,0.0f);
				fwrite(prg_name,28,1,pfile);

				// write params
				for(int pa=0;pa<pi->peffect->numParams;pa++)
				{
					float const f_param_value=pi->peffect->getParameter(pi->peffect,pa);
					long* ppv=(long*)(&f_param_value);
					long const i_param_value=import_reverse_long(*ppv);
					fwrite(&i_param_value,sizeof(long),1,pfile);
				}

				// close file
				fclose(pfile);
				return;
			}

			// write for preset (program) (.fxp) with chunk (magic = 'FPCh')
			if(fx_magic=='hCPF')
			{
				// write num programs
				long const num_programs=import_reverse_long(pi->peffect->numPrograms);
				fwrite(&num_programs,sizeof(long),1,pfile);

				// write program name
				char prg_name[28];
				prg_name[0]=0;
				pi->peffect->dispatcher(pi->peffect,effGetProgramName,0,0,prg_name,0.0f);
				fwrite(prg_name,28,1,pfile);

				// request chunk (is_preset)
				void* pchkdata=NULL;
				long const chunk_size=pi->peffect->dispatcher(pi->peffect,effGetChunk,1,0,&pchkdata,0.0f);

				// write chunk size
				long const r_chunk_size=import_reverse_long(chunk_size);
				fwrite(&r_chunk_size,sizeof(long),1,pfile);

				// write chunk data
				fwrite(pchkdata,1,chunk_size,pfile);

				// close file
				fclose(pfile);
				return;
			}

			// write for bank (.fxb) without chunk (magic = 'FxBk')
			if(fx_magic=='kBxF')
			{
				// write num programs
				long const num_programs=import_reverse_long(pi->peffect->numPrograms);
				fwrite(&num_programs,sizeof(long),1,pfile);

				// write future
				char future[128];
				memset(future,0,128);
				fwrite(future,128,1,pfile);

				// retrieve current program
				int const curr_prg_index=pi->peffect->dispatcher(pi->peffect,effGetProgram,0,0,NULL,0.0f);

				// write programs
				for(int p=0;p<pi->peffect->numPrograms;p++)
				{
					// set program
					pi->peffect->dispatcher(pi->peffect,effSetProgram,0,p,NULL,0.0f);

					// write chunk magic 'CcnK'
					fwrite(&chunk_magic,sizeof(long),1,pfile);

					// write bytesize of this chunk, excl. magic + byteSize
					long const prg_byte_size=import_reverse_long(0);
					fwrite(&prg_byte_size,sizeof(long),1,pfile);

					// write prg fx magic
					long const prg_fx_magic=import_reverse_long('FxCk');
					fwrite(&prg_fx_magic,sizeof(long),1,pfile);

					// write version
					fwrite(&version,sizeof(long),1,pfile);

					// write fx unique id
					fwrite(&fx_id,sizeof(long),1,pfile);

					// write fx version
					fwrite(&fx_version,sizeof(long),1,pfile);

					// write num params
					long const num_params=import_reverse_long(pi->peffect->numParams);
					fwrite(&num_params,sizeof(long),1,pfile);

					// write program name
					char prg_name[28];
					prg_name[0]=0;
					pi->peffect->dispatcher(pi->peffect,effGetProgramName,0,0,prg_name,0.0f);
					fwrite(prg_name,28,1,pfile);

					// write params
					for(int pa=0;pa<pi->peffect->numParams;pa++)
					{
						float const f_param_value=pi->peffect->getParameter(pi->peffect,pa);
						long* ppv=(long*)(&f_param_value);
						long const i_param_value=import_reverse_long(*ppv);
						fwrite(&i_param_value,sizeof(long),1,pfile);
					}
				}

				// set current program
				pi->peffect->dispatcher(pi->peffect,effSetProgram,0,curr_prg_index,NULL,0.0f);

				// close file
				fclose(pfile);
				return;
			}

			// write for bank (.fxb) with chunk (magic = 'FBCh')
			if(fx_magic=='hCBF')
			{
				// write num programs
				long const num_programs=import_reverse_long(pi->peffect->numPrograms);
				fwrite(&num_programs,sizeof(long),1,pfile);

				// write future
				char future[128];
				memset(future,0,128);
				fwrite(future,128,1,pfile);

				// request chunk (is_bank)
				void* pchkdata=NULL;
				long const chunk_size=pi->peffect->dispatcher(pi->peffect,effGetChunk,0,0,&pchkdata,0.0f);

				// write chunk size
				long const r_chunk_size=import_reverse_long(chunk_size);
				fwrite(&r_chunk_size,sizeof(long),1,pfile);

				// write chunk data
				fwrite(pchkdata,1,chunk_size,pfile);

				// close file
				fclose(pfile);
				return;
			}

			// close file
			fclose(pfile);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::export_mid_file_dlg(HWND const hwnd)
{
	// filename holder
	char filename[_MAX_PATH];
	sprintf(filename,project.name);

	// open file dialog
	if(arg_file_dialog_save(hinstance_app,hwnd,"Export MIDI File",filename,"MIDI File (*.mid)\0*.mid\0\0","mid",""))
		export_mid_file(hwnd,filename);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::export_mid_file(HWND const hwnd,char* filename)
{
}
