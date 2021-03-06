/* DirectPlay Lite
 * Copyright (C) 2018 Daniel Collins <solemnwarning@solemnwarning.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <winsock2.h>
#include <algorithm>
#include <dplay8.h>
#include <mutex>
#include <objbase.h>
#include <psapi.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>

/* Durations specified in milliseconds. */
#define TEST_DURATION         (8 * 60 * 60 * 1000)
#define MEMORY_STATS_INTERVAL (30 * 1000)

static const GUID APP_GUID = { 0x8723c2c6, 0x0b89, 0x4ea0, { 0xad, 0xe8, 0xec, 0x53, 0x66, 0x51, 0x68, 0x9f } };

static int64_t pc_freq;
static int64_t now_ms();

static int64_t start_time;
static int64_t usage_time;
static IDirectPlay8Peer *instance;

static HRESULT CALLBACK callback(PVOID pvUserContext, DWORD dwMessageType, PVOID pMessage);
static void print_usage();
static void timed_printf(const char *fmt, ...);

int main(int argc, char **argv)
{
	{
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		pc_freq = li.QuadPart;
	}
	
	HRESULT res = CoInitialize(NULL);
	if(res != S_OK)
	{
		fprintf(stderr, "CoInitialize failed with HRESULT %08x\n", (unsigned)(res));
		return 1;
	}
	
	/* Interval between destroying and re-creating the DirectPlay8Peer
	 * object and closing and re-initialising it.
	 *
	 * Each of these will increase as the test runs.
	*/
	
	int64_t reconstruct_interval  = (86400 * 1000);
	int64_t reinitialise_interval = (86400 * 1000);
	
	start_time = now_ms();
	int64_t end_time = start_time + TEST_DURATION;
	
	print_usage();
	
	while(now_ms() < end_time)
	{
		timed_printf("Constructing DirectPlay8Peer instance...");
		
		res = CoCreateInstance(CLSID_DirectPlay8Peer, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay8Peer, (void**)(&instance));
		if(res != S_OK)
		{
			fprintf(stderr, "Failed to construct DirectPlay8Peer instance (HRESULT %08x)\n", (unsigned)(res));
			return 1;
		}
		
		print_usage();
		
		int64_t construct_time = now_ms();
		int64_t destruct_time  = construct_time + reconstruct_interval;
		reconstruct_interval *= 2;
		
		while(now_ms() < destruct_time && now_ms() < end_time)
		{
			timed_printf("Initialising DirectPlay8Peer instance...");
			
			int64_t initialise_time = now_ms();
			int64_t close_time = now_ms() + reinitialise_interval;
			reinitialise_interval *= 2;
			
			res = instance->Initialize(NULL, &callback, 0);
			if(res != S_OK)
			{
				fprintf(stderr, "IDirectPlay8Peer::Initialize failed with HRESULT %08x\n", (unsigned)(res));
				return 1;
			}
			
			print_usage();
			
			DPN_APPLICATION_DESC app_desc;
			memset(&app_desc, 0, sizeof(app_desc));
			
			app_desc.dwSize = sizeof(app_desc);
			app_desc.guidApplication = APP_GUID;
			app_desc.pwszSessionName = (wchar_t*)(L"IDirectPlay8Peer soak test");
			
			IDirectPlay8Address *address;
			res = CoCreateInstance(CLSID_DirectPlay8Address, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay8Address, (void**)(&address));
			if(res != S_OK)
			{
				fprintf(stderr, "Failed to construct DirectPlay8Address instance (HRESULT %08x)\n", (unsigned)(res));
				return 1;
			}
			
			res = address->SetSP(&CLSID_DP8SP_TCPIP);
			if(res != S_OK)
			{
				fprintf(stderr, "IDirectPlay8Address::SetSP failed with HRESULT %08x\n", (unsigned)(res));
				return 1;
			}
			
			IDirectPlay8Address *addresses[] = { address };
			
			res = instance->Host(&app_desc, addresses, 1, NULL, NULL, NULL, 0);
			if(res != S_OK)
			{
				fprintf(stderr, "IDirectPlay8Peer::Host failed with HRESULT %08x\n", (unsigned)(res));
				return 1;
			}
			
			address->Release();
			
			print_usage();
			
			while(now_ms() < close_time && now_ms() < destruct_time && now_ms() < end_time)
			{
				int64_t sleep_until = std::min({ usage_time, close_time, destruct_time, end_time });
				int64_t sleep_for   = sleep_until - now_ms();
				
				if(sleep_for > 0)
				{
					Sleep(sleep_for);
				}
				
				if(now_ms() >= usage_time)
				{
					print_usage();
				}
			}
			
			timed_printf("Closing DirectPlay8Peer instance...");
			
			/* Alternate between hard/soft closes. */
			
			static bool hard_close = false;
			res = instance->Close(hard_close ? DPNCLOSE_IMMEDIATE : 0);
			if(res != S_OK)
			{
				fprintf(stderr, "IDirectPlay8Peer::Close() failed with HRESULT %08x\n", (unsigned int)(res));
				return 1;
			}
			
			hard_close = !hard_close;
			
			print_usage();
		}
		
		timed_printf("Destroying DirectPlay8Peer instance...");
		
		instance->Release();
		instance = NULL;
		
		print_usage();
	}
	
	CoUninitialize();
	
	return 0;
}

static int64_t now_ms()
{
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
	
	return ticks.QuadPart / (pc_freq / 1000);
}

static HRESULT CALLBACK callback(PVOID pvUserContext, DWORD dwMessageType, PVOID pMessage)
{
	switch(dwMessageType)
	{
		case DPN_MSGID_CREATE_PLAYER:
		{
			DPNMSG_CREATE_PLAYER *cp = (DPNMSG_CREATE_PLAYER*)(pMessage);
			timed_printf("New player ID: %u", (unsigned)(cp->dpnidPlayer));
			
			break;
		}
		
		case DPN_MSGID_DESTROY_PLAYER:
		{
			DPNMSG_DESTROY_PLAYER *dp = (DPNMSG_DESTROY_PLAYER*)(pMessage);
			timed_printf("Destroyed player ID: %u", (unsigned)(dp->dpnidPlayer));
			
			break;
		}
		
		case DPN_MSGID_RECEIVE:
		{
			DPNMSG_RECEIVE *r = (DPNMSG_RECEIVE*)(pMessage);
			
			DPN_BUFFER_DESC bd = { r->dwReceiveDataSize, r->pReceiveData };
			
			DPNHANDLE s_handle;
			HRESULT res = instance->SendTo(r->dpnidSender, &bd, 1, 0, NULL, &s_handle, DPNSEND_GUARANTEED);
			if(res != DPNSUCCESS_PENDING)
			{
				fprintf(stderr, "IDirectPlay8Peer::SendTo() failed with HRESULT %08x\n", (unsigned int)(res));
			}
			
			break;
		}
		
		default:
		{
			break;
		}
	}
	
	return S_OK;
}

static void print_usage()
{
	static SIZE_T peak_usage = 0;
	
	PROCESS_MEMORY_COUNTERS_EX mc;
	mc.cb = sizeof(mc);
	
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)(&mc), sizeof(mc));
	
	if(peak_usage < mc.PrivateUsage)
	{
		peak_usage = mc.PrivateUsage;
	}
	
	timed_printf("Current memory usage: %u bytes, peak usage: %u bytes",
		(unsigned)(mc.PrivateUsage), (unsigned)(peak_usage));
	
	usage_time = now_ms() + MEMORY_STATS_INTERVAL;
}

static void timed_printf(const char *fmt, ...)
{
	static std::mutex lock;
	std::unique_lock<std::mutex> l(lock);
	
	int64_t now = now_ms();
	printf("[T+%06u.%03us] ",
		(unsigned)((now - start_time) / 1000),
		(unsigned)((now - start_time) % 1000));
	
	va_list argv;
	va_start(argv, fmt);
	vprintf(fmt, argv);
	va_end(argv);
	
	printf("\n");
}
