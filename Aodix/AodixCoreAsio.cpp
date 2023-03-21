/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core ASIO Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ASIO_MAX_I_CHANNELS	32
#define ASIO_MAX_O_CHANNELS	32

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ASIO_DRIVER_INFO
{
	// ASIOInit()
	ASIODriverInfo driverInfo;

	// ASIOGetChannels()
	long inputChannels;
	long outputChannels;

	// ASIOGetBufferSize()
	long minSize;
	long maxSize;
	long preferredSize;
	long granularity;

	// ASIOGetSampleRate()
	ASIOSampleRate sampleRate;

	// ASIOOutputReady()
	bool postOutput;

	// ASIOGetLatencies()
	long inputLatency;
	long outputLatency;

	// ASIOCreateBuffers ()
	long inputBuffers;	// becomes number of actual created input buffers
	long outputBuffers;	// becomes number of actual created output buffers

	// the below two arrays share the same indexing, as the data in them are linked together

	// buffer info's
	ASIOBufferInfo bufferInfos[ASIO_MAX_I_CHANNELS+ASIO_MAX_O_CHANNELS];

	// channel info's
	ASIOChannelInfo channelInfos[ASIO_MAX_I_CHANNELS+ASIO_MAX_O_CHANNELS];

	// Information from ASIOGetSamplePosition()
	// data is converted to double floats for easier use, however 64 bit integer can be used, too
	double nanoSeconds;
	double samples;
	double tcSamples;	// time code samples

	// bufferSwitchTimeInfo()
	ASIOTime tInfo;				// time info state
	unsigned long sysRefTime;   // system reference time, when bufferSwitch() was called

	// Signal the end of processing in this example
	bool stopped;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern AsioDrivers*	asioDrivers;
extern CAodixCore*	gl_padx;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ASIO_DRIVER_INFO	asio_driver_info;
ASIOCallbacks		asio_callbacks;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CRITICAL_SECTION	asio_critical_section;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// conversion from 64 bit ASIOSample/ASIOTimeStamp to double float
#if NATIVE_INT64
#define ASIO64toDouble(a)  (a)
#else
const double twoRaisedTo32 = 4294967296.;
#define ASIO64toDouble(a)  ((a).lo + (a).hi * twoRaisedTo32)
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double asio_cb_double_clip(double d_input,double const range)
{
	// clip max
	if(d_input>range)
		d_input=range;

	// clip min
	if(d_input<-range)
		d_input=-range;

	// return clipped sample
	return d_input;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ASIOTime *asio_cb_buffer_switch_time_info(ASIOTime *timeInfo,long index,ASIOBool processNow)
{
	// get current timestamp cycle
	double const init_cycle=arg_sys_rdtsc();

	// enter critical section
	gl_padx->asio_enter_cs();

	// store the timeInfo for later use
	asio_driver_info.tInfo=*timeInfo;

	// get the time stamp of the buffer, not necessary if no synchronization to other media is required
	if(timeInfo->timeInfo.flags & kSystemTimeValid)
		asio_driver_info.nanoSeconds=ASIO64toDouble(timeInfo->timeInfo.systemTime);
	else
		asio_driver_info.nanoSeconds=0;

	if (timeInfo->timeInfo.flags & kSamplePositionValid)
		asio_driver_info.samples=ASIO64toDouble(timeInfo->timeInfo.samplePosition);
	else
		asio_driver_info.samples=0;

	if (timeInfo->timeCode.flags & kTcValid)
		asio_driver_info.tcSamples=ASIO64toDouble(timeInfo->timeCode.timeCodeSamples);
	else
		asio_driver_info.tcSamples=0;

	// get the system reference time
	asio_driver_info.sysRefTime=timeGetTime();

	// get dsp num samples
	int const dsp_num_samples=gl_padx->dsp_block_size;

	// clear dsp input buffers
	gl_padx->dsp_clear_input_buffers();

	// asio input processing, transform input wavedata
	for(int i=0;i<asio_driver_info.inputBuffers;i++)
	{
		// get input pin index
		int const asio_input_index=i;

		// get pin buffer info
		ASIOBufferInfo* pbi=&asio_driver_info.bufferInfos[asio_input_index];

		// get channel info type
		int const ch_type=asio_driver_info.channelInfos[asio_input_index].type;

		// from 16-bit signed integer LSB to 32-bit float
		if(ch_type==ASIOSTInt16LSB)
		{
			// get asio input buffer pointer
			short* psrc=(short*)pbi->buffers[index];

			// mix dsp inputs
			for(int pi=0;pi<NUM_DSP_INPUTS;pi++)
			{
				// check if dsp input pin is assigned to this asio input index
				if(gl_padx->cfg.asio_input_pin[pi]==i)
				{
					// get input dsp buffer
					float* pdst=gl_padx->dsp_input_buffer[pi];

					// get double inverse multiplier
					double const d_sc_inv=1.0/32768.0;

					// cast and mix transfer
					for(int s=0;s<dsp_num_samples;s++)
						pdst[s]+=double(psrc[s])*d_sc_inv;
				}
			}
		}

		// from 32-bit signed integer LSB to 32-bit float
		if(ch_type==ASIOSTInt32LSB)
		{
			// get asio input buffer pointer
			int* psrc=(int*)pbi->buffers[index];

			// mix dsp inputs
			for(int pi=0;pi<NUM_DSP_INPUTS;pi++)
			{
				// check if dsp input pin is assigned to this asio input index
				if(gl_padx->cfg.asio_input_pin[pi]==i)
				{
					// get input dsp buffer
					float* pdst=gl_padx->dsp_input_buffer[pi];

					// get double inverse multiplier
					double const d_sc_inv=1.0/2147483648.0;

					// cast and mix transfer
					for(int s=0;s<dsp_num_samples;s++)
						pdst[s]+=double(psrc[s])*d_sc_inv;
				}
			}
		}
	}

	// main asio host dsp process
	gl_padx->dsp_work();

	// asio output processing, transform output wavedata
	for(int o=0;o<asio_driver_info.outputBuffers;o++)
	{
		// get ouput pin index
		int const asio_output_index=asio_driver_info.inputBuffers+o;

		// get pin buffer info
		ASIOBufferInfo* pbi=&asio_driver_info.bufferInfos[asio_output_index];

		// get channel info type
		int const ch_type=asio_driver_info.channelInfos[asio_output_index].type;

		// from 32-bit float to 16-bit signed integer LSB
		if(ch_type==ASIOSTInt16LSB)
		{
			// get asio output buffer pointer
			short* pdst=(short*)pbi->buffers[index];

			// clear buffer
			for(int s=0;s<asio_driver_info.preferredSize;s++)
				pdst[s]=0;

			// mix dsp outputs
			for(int po=0;po<NUM_DSP_OUTPUTS;po++)
			{
				// check if dsp output pin is assigned to this asio output index
				if(gl_padx->cfg.asio_output_pin[po]==o)
				{
					// get output dsp buffer
					float* psrc=gl_padx->dsp_output_buffer[po];

					// cast and mix transfer
					for(s=0;s<dsp_num_samples;s++)
						pdst[s]=int(asio_cb_double_clip(double(pdst[s])+double(psrc[s])*32768.0,32767.0));
				}
			}
		}

		// from 32-bit float to 32-bit signed integer LSB
		if(ch_type==ASIOSTInt32LSB)
		{
			// get asio output buffer pointer
			int* pdst=(int*)pbi->buffers[index];

			// clear buffer
			for(int s=0;s<asio_driver_info.preferredSize;s++)
				pdst[s]=0;

			// mix dsp outputs
			for(int po=0;po<NUM_DSP_OUTPUTS;po++)
			{
				// check if dsp output pin is assigned to this asio output index
				if(gl_padx->cfg.asio_output_pin[po]==o)
				{
					// get output dsp buffer
					float* psrc=gl_padx->dsp_output_buffer[po];

					// cast and mix transfer
					for(s=0;s<dsp_num_samples;s++)
						pdst[s]=int(asio_cb_double_clip(double(pdst[s])+double(psrc[s])*2147483648.0,2147483647.0));
				}
			}
		}
	}

	// finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
	if(asio_driver_info.postOutput)
		ASIOOutputReady();

	// leave critical section
	gl_padx->asio_leave_cs();

	// performance count
	double const elapsed_cycles=arg_sys_rdtsc()-init_cycle;

	// system cycles per dsp block
	double const d_system_cycles_per_dsp_block_frame=(gl_padx->d_system_clocks_per_sec*double(dsp_num_samples))/gl_padx->cfg.asio_driver_sample_rate;

	// calculate cpu performance percent
	double const d_per_cent=(elapsed_cycles*100.0)/d_system_cycles_per_dsp_block_frame;

	// lowpass cpu meter
	gl_padx->dsp_cpu_cost+=(d_per_cent-gl_padx->dsp_cpu_cost)*0.01f;

	// return
	return 0L;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void asio_cb_buffer_switch(long index,ASIOBool processNow)
{
	// enter critical section
	gl_padx->asio_enter_cs();

	// as this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs to be created
	// though it will only set the timeInfo.samplePosition and timeInfo.systemTime fields and the according flags
	ASIOTime timeInfo;
	memset(&timeInfo,0,sizeof(timeInfo));

	// get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	if(ASIOGetSamplePosition(&timeInfo.timeInfo.samplePosition,&timeInfo.timeInfo.systemTime)==ASE_OK)
		timeInfo.timeInfo.flags=kSystemTimeValid | kSamplePositionValid;

	// call time and buffer switch info
	asio_cb_buffer_switch_time_info(&timeInfo,index,processNow);

	// leave critical section
	gl_padx->asio_leave_cs();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void asio_cb_sample_rate_changed(ASIOSampleRate sRate)
{
	// do whatever you need to do if the sample rate changed
	// usually this only happens during external sync.
	// Audio processing is not stopped by the driver, actual sample rate
	// might not have even changed, maybe only the sample rate status of an
	// AES/EBU or S/PDIF digital input at the audio device.
	// You might have to update time/sample related conversion routines, etc.
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long asio_cb_messages(long selector, long value, void* message, double* opt)
{
	// currently the parameters "value", "message" and "opt" are not used.
	long ret = 0;

	switch(selector)
	{
	case kAsioSelectorSupported:
		if(value == kAsioResetRequest
			|| value == kAsioEngineVersion
			|| value == kAsioResyncRequest
			|| value == kAsioLatenciesChanged
			// the following three were added for ASIO 2.0, you don't necessarily have to support them
			|| value == kAsioSupportsTimeInfo
			|| value == kAsioSupportsTimeCode
			|| value == kAsioSupportsInputMonitor)
			ret = 1L;
		break;

	case kAsioResetRequest:
		// defer the task and perform the reset of the driver during the next "safe" situation
		// You cannot reset the driver right now, as this code is called from the driver.
		// Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
		// Afterwards you initialize the driver again.
		ret = 1L;
		break;

	case kAsioResyncRequest:
		// This informs the application, that the driver encountered some non fatal data loss.
		// It is used for synchronization purposes of different media.
		// Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
		// Windows Multimedia system, which could loose data because the Mutex was hold too long
		// by another thread.
		// However a driver can issue it in other situations, too.
		ret = 1L;
		break;

	case kAsioLatenciesChanged:
		// This will inform the host application that the drivers were latencies changed.
		// Beware, it this does not mean that the buffer sizes have changed!
		// You might need to update internal delay data.
		ret = 1L;
		break;

	case kAsioEngineVersion:
		// return the supported ASIO version of the host application
		// If a host applications does not implement this selector, ASIO 1.0 is assumed
		// by the driver
		ret = 2L;
		break;

	case kAsioSupportsTimeInfo:
		// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
		// is supported.
		// For compatibility with ASIO 1.0 drivers the host application should always support
		// the "old" bufferSwitch method, too.
		ret = 1;
		break;

	case kAsioSupportsTimeCode:
		// informs the driver wether application is interested in time code info.
		// If an application does not need to know about time code, the driver has less work
		// to do.
		ret = 0;
		break;
	}

	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::asio_enter_cs(void)
{
	EnterCriticalSection(&asio_critical_section);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::asio_leave_cs(void)
{
	LeaveCriticalSection(&asio_critical_section);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::asio_init(HWND const hwnd)
{
	// init global critical section object
	InitializeCriticalSection(&asio_critical_section);

	// set default blocksize
	dsp_block_size=MAX_DSP_BLOCK_SIZE;

	// return inmediantly if no driver is selected
	if(cfg.asio_driver_id<0)
		return;

	// asio driver name store
	char asio_driver_name[32];

	// retrieve asio driver name
	asio_get_driver_name(cfg.asio_driver_id,asio_driver_name,32);

	// open asio driver
	if(!asioDrivers->loadDriver(asio_driver_name))
	{
		MessageBox(hwnd,"Error: Failed To Open Driver","Aodix - ASIO Initialization",MB_OK | MB_ICONERROR);
		return;
	}

	// init asio
	if(ASIOInit(&asio_driver_info.driverInfo)!=ASE_OK)
	{
		MessageBox(hwnd,"Error: ASIOInit() Failed","Aodix - ASIO Initialization",MB_OK | MB_ICONERROR);
		return;
	}

	// get the number of available channels
	if(ASIOGetChannels(&asio_driver_info.inputChannels,&asio_driver_info.outputChannels)!=ASE_OK)
	{
		MessageBox(hwnd,"Error: ASIOGetChannels() Failed","Aodix - ASIO Initialization",MB_OK | MB_ICONERROR);
		return;
	}

	// get the usable buffer sizes
	if(ASIOGetBufferSize(&asio_driver_info.minSize,&asio_driver_info.maxSize,&asio_driver_info.preferredSize,&asio_driver_info.granularity)!=ASE_OK)
	{
		MessageBox(hwnd,"Error: ASIOGetBufferSize() Failed","Aodix - ASIO Initialization",MB_OK | MB_ICONERROR);
		return;
	}

	// notify host preferred block size
	dsp_block_size=asio_driver_info.preferredSize;

	// set sample rate
	ASIOSetSampleRate(cfg.asio_driver_sample_rate);

	// get the currently selected sample rate
	ASIOGetSampleRate(&asio_driver_info.sampleRate);

	// re-asign sample rate
	cfg.asio_driver_sample_rate=asio_driver_info.sampleRate;

	// scan vst instances
	for(long i=0;i<MAX_INSTANCES;i++)
	{
		ADX_INSTANCE* pi=&instance[i];

		// update sample rate and block size
		if(pi->peffect!=NULL)
		{
			pi->peffect->dispatcher(pi->peffect,effSetSampleRate,0,0,NULL,float(cfg.asio_driver_sample_rate));
			pi->peffect->dispatcher(pi->peffect,effSetBlockSize,0,dsp_block_size,NULL,0.0f);
		}
	}

	// check wether the driver requires the ASIOOutputReady() optimization
	// (can be used by the driver to reduce output latency by one block)
	asio_driver_info.postOutput=(ASIOOutputReady()==ASE_OK);

	// set up the asioCallback structure and create the ASIO data buffer
	asio_callbacks.bufferSwitch=&asio_cb_buffer_switch;
	asio_callbacks.sampleRateDidChange=&asio_cb_sample_rate_changed;
	asio_callbacks.asioMessage=&asio_cb_messages;
	asio_callbacks.bufferSwitchTimeInfo=&asio_cb_buffer_switch_time_info;

	// create buffers for all inputs and outputs of the card with the 
	// preferredSize from ASIOGetBufferSize() as buffer size
	// filling the bufferInfos from the start without a gap
	ASIOBufferInfo *info=asio_driver_info.bufferInfos;

	// prepare inputs
	asio_num_inputs=asio_driver_info.inputBuffers=asio_driver_info.inputChannels;

	for(i=0;i<asio_driver_info.inputBuffers;i++,info++)
	{
		info->isInput=ASIOTrue;
		info->channelNum=i;
		info->buffers[0]=info->buffers[1]=0;
	}

	// prepare outputs
	asio_num_outputs=asio_driver_info.outputBuffers=asio_driver_info.outputChannels;

	for(i=0;i<asio_driver_info.outputBuffers;i++,info++)
	{
		info->isInput=ASIOFalse;
		info->channelNum=i;
		info->buffers[0]=info->buffers[1]=0;
	}

	// get number of total buffers (ins + outs)
	int const asio_num_buffers=asio_driver_info.inputBuffers+asio_driver_info.outputBuffers;

	// create and activate buffers
	if(ASIOCreateBuffers(asio_driver_info.bufferInfos,asio_num_buffers,asio_driver_info.preferredSize,&asio_callbacks)!=ASE_OK)
	{
		MessageBox(hwnd,"Error: ASIOCreateBuffers() Failed","Aodix - ASIO Initialization",MB_OK | MB_ICONERROR);
		return;
	}

	// now get all the buffer details, sample word length, name, word clock group and activation
	for(i=0;i<asio_num_buffers;i++)
	{
		asio_driver_info.channelInfos[i].channel=asio_driver_info.bufferInfos[i].channelNum;
		asio_driver_info.channelInfos[i].isInput=asio_driver_info.bufferInfos[i].isInput;

		if(ASIOGetChannelInfo(&asio_driver_info.channelInfos[i])!=ASE_OK)
		{
			MessageBox(hwnd,"Error: ASIOGetChannelInfo() Failed","Aodix - ASIO Initialization",MB_OK | MB_ICONERROR);
			return;
		}
	}

	// start asio
	if(ASIOStart()!=ASE_OK)
	{
		MessageBox(hwnd,"Error: ASIOStart() Failed","Aodix - ASIO Initialization",MB_OK | MB_ICONERROR);
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::asio_close(HWND const hwnd)
{
	// shutdown asio
	if(ASIOStop()!=ASE_OK)
	{
		MessageBox(hwnd,"Error: ASIOStop() Failed","Aodix - ASIO Shutdown",MB_OK | MB_ICONERROR);
		return;
	}

	// destroy asio buffers
	if(ASIODisposeBuffers()!=ASE_OK)
	{
		MessageBox(hwnd,"Error: ASIODisposeBuffers() Failed","Aodix - ASIO Shutdown",MB_OK | MB_ICONERROR);
		return;
	}

	// exit asio
	if(ASIOExit()!=ASE_OK)
	{
		MessageBox(hwnd,"Error: ASIOExit() Failed","Aodix - ASIO Shutdown",MB_OK | MB_ICONERROR);
		return;
	}

	// close asio driver
	asioDrivers->removeCurrentDriver();

	// destroy global critical section object
	DeleteCriticalSection(&asio_critical_section);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CAodixCore::asio_is_inited(void)
{
	return asioDrivers->getCurrentDriverIndex() != -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::asio_get_driver_name(int const id,char* buf,int const num_chars)
{
	// reset buffer if fails
	buf[0]=0;

	// retrieve asio driver name
	if(id>=0)
		asioDrivers->asioGetDriverName(id,buf,num_chars);
	else
		sprintf(buf,"Click To Select Driver");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long CAodixCore::asio_init_list(void)
{
	// create driver list if not done
	if(!asioDrivers)
		asioDrivers=new AsioDrivers();

	// return number of asio drivers
	return asioDrivers->asioGetNumDev();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::asio_fill_pin_menu(HMENU const hmenu,int const fill_inputs,int const init_id)
{
	// stack vars
	char buf[32];
	int opt_id=init_id;

	// head menu label
	if(fill_inputs)
		sprintf(buf,"Master Input Pin %d",user_input_pin);
	else
		sprintf(buf,"Master Output Pin %d",user_output_pin);

	// add head option
	arg_menu_add_item(hmenu,buf,opt_id++);

	// set default state
	MENUITEMINFO mii;
	memset((void*)(&mii),0,sizeof(mii));
	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_STATE;
	mii.fState=MFS_DEFAULT | MFS_HILITE;

	// change menu item info
	SetMenuItemInfo(hmenu,0,TRUE,&mii);

	// add separator
	arg_menu_add_item(hmenu,NULL,opt_id++);

	// get pin count
	int const pin_count=(asio_driver_info.inputChannels*fill_inputs)+(asio_driver_info.outputChannels*(!fill_inputs));

	// fill menu with pin channels
	for(long p=0;p<pin_count;p++)
	{
		// get channel info pointer
		ASIOChannelInfo* pci=&asio_driver_info.channelInfos[(asio_driver_info.inputChannels*(!fill_inputs))+p];

		// add option
		arg_menu_add_item(hmenu,pci->name,opt_id++);
	}

	// add separator
	arg_menu_add_item(hmenu,NULL,opt_id++);

	// get asio hardware assignment from selected pin
	int asio_pin_assignment=0;

	// no assigned pin
	if(fill_inputs)
	{
		// add no assigned input option 
		arg_menu_add_item(hmenu,"No Input",opt_id++);

		// get pin input selected assignment
		asio_pin_assignment=cfg.asio_input_pin[user_input_pin];
	}
	else
	{
		// add no assigned output option 
		arg_menu_add_item(hmenu,"No Output",opt_id++);

		// get pin output selected assignment
		asio_pin_assignment=cfg.asio_output_pin[user_output_pin];
	}

	// check assigned pin
	if(asio_pin_assignment<pin_count)
		CheckMenuRadioItem(hmenu,2,3+pin_count,2+asio_pin_assignment,MF_BYPOSITION);
	else
		CheckMenuRadioItem(hmenu,2,3+pin_count,3+pin_count,MF_BYPOSITION);
}