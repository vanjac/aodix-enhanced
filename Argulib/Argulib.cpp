/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Argulib Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "argulib.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable: 4091)
#include "shlobj.h"
#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// System Functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double arg_sys_get_cpu_speed(void)
{
	LARGE_INTEGER ulFreq,ulTicks,ulValue,ulStartCounter,ulEAX_EDX;

	// query for high-resolution counter frequency (this is not the CPU frequency)
	if (QueryPerformanceFrequency(&ulFreq))
	{
		// Query current value:
		QueryPerformanceCounter(&ulTicks);

		// Calculate end value (one second interval); this is (current + frequency)
		ulValue.QuadPart = ulTicks.QuadPart + ulFreq.QuadPart;

		// Read CPU time-stamp counter:
		__asm rdtsc;

		// And save in ulEAX_EDX:
		__asm mov ulEAX_EDX.LowPart,eax;
		__asm mov ulEAX_EDX.HighPart,edx;

		// Store starting counter value:
		ulStartCounter.QuadPart = ulEAX_EDX.QuadPart;

		// Loop for one second (measured with the high-resolution counter):
		do
		{
			QueryPerformanceCounter(&ulTicks);
		}
		while(ulTicks.QuadPart<=ulValue.QuadPart);

		__asm rdtsc;						// mow again read CPU time-stamp counter:
		__asm mov ulEAX_EDX.LowPart,eax;	// save low
		__asm mov ulEAX_EDX.HighPart,edx;	// save high

		// calculate number of cycles done in interval; 1000000 Hz = 1 MHz
		return double(ulEAX_EDX.QuadPart-ulStartCounter.QuadPart);
	}
	else
	{
		// no high-resolution counter present:
		return 1000.0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double arg_sys_rdtsc(void)
{
	// cycle counter register
	LARGE_INTEGER ulEAX_EDX;

	// cpu performance measure
	__asm
	{
		rdtsc;						// now read again CPU time-stamp counter:
		mov ulEAX_EDX.LowPart,eax;	// save counter
		mov ulEAX_EDX.HighPart,edx;
	}

	// calculate number of cycles done in interval; 1000000 hz = 1 mhz
	return double(ulEAX_EDX.QuadPart);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tool functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int arg_tool_clipped_assign(int const value,int const min,int const max)
{
	// minimun return
	if(value<min)
		return min;

	// maximun return
	if(value>max)
		return max;

	// return value
	return value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int arg_tool_check_plane_xy(int const x,int const y,int const zx,int const zy,int const zw,int const zh)
{
	// check zone
	return (x>=zx && y>=zy && x<(zx+zw) && y<(zy+zh));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int arg_tool_check_point_in_2d_poly(int const npol,float *xp,float *yp,float const x,float const y)
{
	int i,j,c=0;

	for(i=0,j=npol-1;i<npol;j=i++)
	{
		if((((yp[i] <= y) && (y < yp[j])) || ((yp[j] <= y) && (y < yp[i]))) && (x < (xp[j] - xp[i]) * (y - yp[i]) / (yp[j] - yp[i]) + xp[i]))
			c=!c;
	}

	return c;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tool Functions (Files And Directories)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_tool_get_filetitle(char* pfilename,char* pfiletitle)
{
	// temporal intermediate buffer
	char buf[_MAX_PATH];

	// first copy full filename to temporal string
	strcpy(buf,pfilename);

	// get buffer string length
	int const buf_str_len=strlen(buf);

	// remove right extension dot
	for(int i=buf_str_len-1;i>=0;i--)
	{
		// check dot
		if(buf[i]=='.')
		{
			// set end and exit loop
			buf[i]=0;
			i=-1;
		}
	}

	// find right slashdots
	for(int i=strlen(buf)-1;i>=0;i--)
	{
		// check dot
		if(buf[i]=='\\' || buf[i]=='/')
		{
			// clamp to 26 characters
			buf[i+26]=0;

			// copy filetitle to buffer and return
			strcpy(pfiletitle,buf+i+1);
			return;
		}
	}

	// copy filetitle to buffer
	strcpy(pfiletitle,buf);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Win32 GDI functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_gdi_set_clip_rgn(HDC const hdc,int const x,int const y,int const w,int const h)
{
	HRGN const hrgn=CreateRectRgn(x,y,x+w,y+h);
	SelectClipRgn(hdc,hrgn);
	DeleteObject(hrgn);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_gdi_end_clip_rgn(HDC const hdc)
{
	SelectClipRgn(hdc,NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_gdi_paint_invert_rect(HDC const hdc,int const x,int const y,int const w,int const h)
{
	// rect
	RECT r;
	r.left=x;
	r.top=y;
	r.right=x+w;
	r.bottom=y+h;

	// invert rectangle
	InvertRect(hdc,&r);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_gdi_paint_line(HDC const hdc,int const x,int const y,int const x2,int const y2,int const pen_style,COLORREF const color)
{
	// create pen
	HPEN const hpen_cur=CreatePen(pen_style,1,color);

	// select pen
	HPEN const hpen_old=(HPEN)SelectObject(hdc,hpen_cur);

	// draw line
	MoveToEx(hdc,x,y,NULL);
	LineTo(hdc,x2,y2);

	// restore default pen
	SelectObject(hdc,hpen_old);

	// delete pen object
	DeleteObject(hpen_cur);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_gdi_paint_solid_rect(HDC const hdc,int const x,int const y,int const w,int const h,COLORREF const color)
{
	// rect
	RECT r;
	r.left=x;
	r.top=y;
	r.right=x+w;
	r.bottom=y+h;

	// solid rectangle
	SetBkColor(hdc,color);
	ExtTextOut(hdc,0,0,ETO_OPAQUE,&r,NULL,0,NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Riff WAV functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_menu_add_item(HMENU const hmenu,char* txt,int const id,HMENU const hsubmenu)
{
	// temp item
	MENUITEMINFO item;

	// fill up the menu struct
	item.cbSize=sizeof(MENUITEMINFO);
	item.fMask=MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
	item.hSubMenu=hsubmenu;
	item.wID=id;

	// check txt param
	if(txt!=NULL)
	{
		// text option
		item.fType=MFT_STRING;
		item.dwTypeData=txt;
		item.cch=strlen(txt);
	}
	else
	{
		// separator
		item.fType=MFT_SEPARATOR;
		item.dwTypeData=NULL;
		item.cch=0;
	}

	// insert into menu
	InsertMenuItem(hmenu,0,FALSE,&item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_menu_track(HMENU const hmenu,HWND const hwnd,int const x,int const y)
{
	POINT pnt;
	pnt.x=x;
	pnt.y=y;
	ClientToScreen(hwnd,&pnt);
	TrackPopupMenu(hmenu,TPM_LEFTALIGN,pnt.x,pnt.y,0,hwnd,NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Browse Folder Dialog functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_browse_folder(HWND const hwnd,char* buf)
{
	// init browse info struct
	BROWSEINFO BrowseInfo;
	memset(&BrowseInfo,0,sizeof(BrowseInfo));
	BrowseInfo.hwndOwner=hwnd;
	BrowseInfo.pszDisplayName=buf;
	BrowseInfo.lpszTitle="Select Directory";
	BrowseInfo.ulFlags=BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS;

	// throw display dialog
	LPITEMIDLIST pList=SHBrowseForFolder(&BrowseInfo);

	if(pList!=NULL)
	{
		// convert from MIDLISt to real string path
		SHGetPathFromIDList(pList,buf);

		// global pointer to the shell's IMalloc interface.  
		LPMALLOC pMalloc; 

		// Get the shell's allocator and free the PIDL returned by SHBrowseForFolder
		if (SUCCEEDED(SHGetMalloc(&pMalloc))) 
			pMalloc->Free(pList);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// File Dialog functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int arg_file_dialog_open(HINSTANCE const hinstance,HWND const hwnd,char* title,char* filename,char* filter,char* default_extension,char* initial_dir,int const filter_index,int const allow_multi)
{
	// filename struct holder
	OPENFILENAME OFN;
	memset(&OFN,0,sizeof(OPENFILENAME));

	// setup structure
	OFN.lpstrTitle=title;
	OFN.lStructSize=sizeof(OPENFILENAME);
	OFN.hwndOwner=hwnd;
	OFN.lpstrFilter=filter;
	OFN.lpstrFile=filename;
	OFN.nFilterIndex=0;
	OFN.nMaxFile=_MAX_PATH;
	OFN.lpstrInitialDir=initial_dir;
	OFN.nFilterIndex=filter_index;
	OFN.hInstance=hinstance;
	OFN.Flags=OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLESIZING;

	// multiselection flag
	if(allow_multi)
	{
		OFN.Flags|=OFN_ALLOWMULTISELECT;
		OFN.nMaxFile=_MAX_PATH*ARG_FD_MAX_MS_FILES;
	}

	// open file accepted
	if(GetOpenFileName(&OFN)!=0)
		return 1;

	// canceled
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int arg_file_dialog_save(HINSTANCE const hinstance,HWND const hwnd,char* title,char* filename,char* filter,char* default_extension,char* initial_dir)
{
	// filename struct holder
	OPENFILENAME OFN;
	memset(&OFN,0,sizeof(OPENFILENAME));

	// fill structure
	OFN.lpstrTitle=title;
	OFN.lStructSize=sizeof(OPENFILENAME);
	OFN.hwndOwner=hwnd;
	OFN.lpstrFilter=filter;
	OFN.lpstrFile=filename;
	OFN.nMaxFile=_MAX_PATH;
	OFN.Flags=OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT;
	OFN.lpstrInitialDir=initial_dir;
	OFN.lpstrDefExt=default_extension;
	OFN.hInstance=hinstance;

	// save program accepted
	if(GetSaveFileName(&OFN)!=0)
		return 1;

	// canceled
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DSP functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_dsp_zero(float* psrc,int const num_samples)
{
	for(int s=0;s<num_samples;s++)
		psrc[s]=0.0f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_dsp_copy(float* psrc,float* pdst,int const num_samples)
{
	memcpy(pdst,psrc,sizeof(float)*num_samples);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_dsp_bmix(float* psrc,float* pdst,int const num_samples)
{
	for(int s=0;s<num_samples;s++)
		pdst[s]+=psrc[s];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_dsp_gmix(float* psrc,float* pdst,int const num_samples,float const gain)
{
	for(int s=0;s<num_samples;s++)
		pdst[s]+=psrc[s]*gain;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float arg_dsp_conv(float* psamples,float* pfir,int const num_taps)
{
	// stack vars
	float f1=0.0f;
	float f2=0.0f;

	// interleaved convolution (pipeline optimization)
	for(int i=0;i<num_taps;i+=2)
	{
		f1+=psamples[i]*pfir[i];
		f2+=psamples[i+1]*pfir[i+1];
	}

	// sum and result
	return f1+f2;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_dsp_limiter(float& sl,float &sr,float &l_env,float const l_att,float const l_rel)
{
	// limiter get peak
	float const peak=max(fabs(sl),fabs(sr));

	// limiter envelope (level detection)
	if(peak>l_env)
		l_env=l_att*(l_env-peak)+peak;
	else
		l_env=l_rel*(l_env-peak)+peak;

	// gain processor
	if(l_env>1.0f)
	{
		// compute gain
		float const l_gain=powf(l_env,-1.0f);

		// apply gain
		sl*=l_gain;
		sr*=l_gain;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_rgba_shade(unsigned char* pp,int const r,int const g,int const b,int const a)
{
	// shade pixel
	pp[0]=((a*b)+(255-a)*pp[0])/256;
	pp[1]=((a*g)+(255-a)*pp[1])/256;
	pp[2]=((a*r)+(255-a)*pp[2])/256;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void arg_dsp_set_filter_coeffs(ARG_DSP_FILTER_COEFFS* pfc,int const type,float const f,float const q,float const sample_rate)
{
	// thru
	if(type==0)
	{
		pfc->b0a0=1.0f;
		pfc->b1a0=0.0f;
		pfc->b2a0=0.0f;
		pfc->a1a0=0.0f;
		pfc->a2a0=0.0f;
		return;
	}

	// biquad constants
	float const w=f*arg_dsp_k2pi/sample_rate;
	float const tsin=sinf(w);
	float const tcos=cosf(w);
	float const alph=tsin/(2.0f*q);
	float const iap1=1.0f/(1.0f+alph);

	// lowpass
	if(type==1)
	{
		pfc->b0a0=(1.0f-tcos)*0.5f*iap1;
		pfc->b1a0=(1.0f-tcos)*iap1;
		pfc->b2a0=(1.0f-tcos)*0.5f*iap1;
		pfc->a1a0=-2.0f*tcos*iap1;
		pfc->a2a0=(1.0f-alph)*iap1;
	}

	// hipass
	if(type==2)
	{
		pfc->b0a0=(1.0f+tcos)*0.5f*iap1;
		pfc->b1a0=-(1.0f+tcos)*iap1;
		pfc->b2a0=(1.0f+tcos)*0.5f*iap1;
		pfc->a1a0=-2.0f*tcos*iap1;
		pfc->a2a0=(1.0f-alph)*iap1;
	}

	// bandpass constant skirt gain, peak gain = Q
	if(type==3)
	{
		pfc->b0a0=tsin*0.5f*iap1;
		pfc->b1a0=0.0f;
		pfc->b2a0=-tsin*0.5f*iap1;
		pfc->a1a0=-2.0f*tcos*iap1;
		pfc->a2a0=(1.0f-alph)*iap1;
	}

	// notch
	if(type==4)
	{
		pfc->b0a0=iap1;
		pfc->b1a0=-2.0f*tcos*iap1;
		pfc->b2a0=iap1;
		pfc->a1a0=-2.0f*tcos*iap1;
		pfc->a2a0=(1.0f-alph)*iap1;
	}

	// allpass filter
	if(type==5)
	{
		pfc->b0a0=(1.0f-alph)*iap1;
		pfc->b1a0=-2.0f*tcos*iap1;
		pfc->b2a0=(1.0f+alph)*iap1;
		pfc->a1a0=-2.0f*tcos*iap1;
		pfc->a2a0=(1.0f-alph)*iap1;
	}
};
