#include <gtest/gtest.h>

#include "../src/DirectPlay8Address.hpp"

// #define INSTANTIATE_FROM_COM

class DirectPlay8AddressInitial: public ::testing::Test {
	private:
		DirectPlay8Address *addr;
		
	protected:
		IDirectPlay8Address *idp8;
		
		DirectPlay8AddressInitial()
		{
			#ifdef INSTANTIATE_FROM_COM
			CoInitialize(NULL);
			CoCreateInstance(CLSID_DirectPlay8Address, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay8Address, (void**)(&idp8));
			#else
			addr = new DirectPlay8Address(NULL);
			idp8 = addr;
			#endif
		}
		
		~DirectPlay8AddressInitial()
		{
			#ifdef INSTANTIATE_FROM_COM
			idp8->Release();
			CoUninitialize();
			#else
			delete addr;
			#endif
		}
};

TEST_F(DirectPlay8AddressInitial, HasNoComponents)
{
	DWORD num;
	ASSERT_EQ(idp8->GetNumComponents(&num), S_OK);
	EXPECT_EQ(num, (DWORD)(0));
}

class DirectPlay8AddressWithWStringComponent: public DirectPlay8AddressInitial
{
	protected:
		const wchar_t *REFKEY = L"key";
		const DWORD REFKSIZE = 4;
		
		const wchar_t *REFVAL = L"wide string value";
		const DWORD REFVSIZE = 18 * sizeof(wchar_t);
		
		wchar_t kbuf[256];
		unsigned char vbuf[256];
		
		virtual void SetUp() override
		{
			memset(kbuf, 0xFF, sizeof(kbuf));
			memset(vbuf, 0xFF, sizeof(vbuf));
			
			ASSERT_EQ(idp8->AddComponent(REFKEY, REFVAL, REFVSIZE, DPNA_DATATYPE_STRING), S_OK);
		}
};

TEST_F(DirectPlay8AddressWithWStringComponent, HasOneComponent)
{
	DWORD num;
	ASSERT_EQ(idp8->GetNumComponents(&num), S_OK);
	EXPECT_EQ(num, (DWORD)(1));
}

TEST_F(DirectPlay8AddressWithWStringComponent, ComponentByIndexNameSizeZero)
{
	DWORD ksize = 0;
	DWORD vsize = REFVSIZE;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByIndex(0, (wchar_t*)(kbuf), &ksize, (void*)(vbuf), &vsize, &type), DPNERR_BUFFERTOOSMALL);
	
	EXPECT_EQ(ksize, REFKSIZE);
	EXPECT_EQ(vsize, REFVSIZE);
}

TEST_F(DirectPlay8AddressWithWStringComponent, ComponentByIndexBufferSizeZero)
{
	DWORD ksize = REFKSIZE;
	DWORD vsize = 0;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByIndex(0, (wchar_t*)(kbuf), &ksize, (void*)(vbuf), &vsize, &type), DPNERR_BUFFERTOOSMALL);
	
	EXPECT_EQ(ksize, REFKSIZE);
	EXPECT_EQ(vsize, REFVSIZE);
}

TEST_F(DirectPlay8AddressWithWStringComponent, ComponentByIndexNameSizeSmall)
{
	DWORD ksize = REFKSIZE - 1;
	DWORD vsize = REFVSIZE;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByIndex(0, (wchar_t*)(kbuf), &ksize, (void*)(vbuf), &vsize, &type), DPNERR_BUFFERTOOSMALL);
	
	EXPECT_EQ(ksize, REFKSIZE);
	EXPECT_EQ(vsize, REFVSIZE);
}

TEST_F(DirectPlay8AddressWithWStringComponent, ComponentByIndexBufferSizeSmall)
{
	DWORD ksize = REFKSIZE;
	DWORD vsize = REFVSIZE - 1;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByIndex(0, (wchar_t*)(kbuf), &ksize, (void*)(vbuf), &vsize, &type), DPNERR_BUFFERTOOSMALL);
	
	EXPECT_EQ(ksize, REFKSIZE);
	EXPECT_EQ(vsize, REFVSIZE);
}

TEST_F(DirectPlay8AddressWithWStringComponent, ComponentByIndexSizeExact)
{
	DWORD ksize = REFKSIZE;
	DWORD vsize = REFVSIZE;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByIndex(0, (wchar_t*)(kbuf), &ksize, (void*)(vbuf), &vsize, &type), S_OK);
	
	EXPECT_EQ(ksize, REFKSIZE);
	EXPECT_EQ(vsize, REFVSIZE);
	EXPECT_EQ(type, (DWORD)(DPNA_DATATYPE_STRING));
	
	EXPECT_EQ(std::wstring((const wchar_t*)(kbuf), ksize), std::wstring(REFKEY, REFKSIZE));
	
	EXPECT_EQ(std::wstring((const wchar_t*)(vbuf), (vsize / sizeof(wchar_t))),
		std::wstring(REFVAL, (REFVSIZE / sizeof(wchar_t))));
}

TEST_F(DirectPlay8AddressWithWStringComponent, ComponentByIndexSizeBig)
{
	DWORD ksize = REFKSIZE * 2;
	DWORD vsize = REFVSIZE * 2;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByIndex(0, (wchar_t*)(kbuf), &ksize, (void*)(vbuf), &vsize, &type), S_OK);
	
	EXPECT_EQ(ksize, REFKSIZE);
	EXPECT_EQ(vsize, REFVSIZE);
	EXPECT_EQ(type, (DWORD)(DPNA_DATATYPE_STRING));
	
	EXPECT_EQ(std::wstring((const wchar_t*)(kbuf), ksize), std::wstring(REFKEY, REFKSIZE));
	
	EXPECT_EQ(std::wstring((const wchar_t*)(vbuf), (vsize / sizeof(wchar_t))),
		std::wstring(REFVAL, (REFVSIZE / sizeof(wchar_t))));
}

TEST_F(DirectPlay8AddressWithWStringComponent, ComponentByIndexWrongIndex)
{
	DWORD ksize = sizeof(kbuf);
	DWORD vsize = sizeof(vbuf);
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByIndex(1, (wchar_t*)(kbuf), &ksize, (void*)(vbuf), &vsize, &type), DPNERR_DOESNOTEXIST);
}

TEST_F(DirectPlay8AddressWithWStringComponent, ComponentByNameBufferSizeZero)
{
	DWORD vsize = 0;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, NULL, &vsize, &type), DPNERR_BUFFERTOOSMALL);
	
	EXPECT_EQ(vsize, REFVSIZE);
}

TEST_F(DirectPlay8AddressWithWStringComponent, ComponentByNameBufferSizeSmall)
{
	DWORD vsize = REFVSIZE - 1;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, (void*)(vbuf), &vsize, &type), DPNERR_BUFFERTOOSMALL);
	
	EXPECT_EQ(vsize, REFVSIZE);
}

TEST_F(DirectPlay8AddressWithWStringComponent, ComponentByNameBufferSizeExact)
{
	DWORD vsize = REFVSIZE;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, (void*)(vbuf), &vsize, &type), S_OK);
	
	EXPECT_EQ(vsize, REFVSIZE);
	EXPECT_EQ(type, (DWORD)(DPNA_DATATYPE_STRING));
	
	EXPECT_EQ(std::wstring((const wchar_t*)(vbuf), (vsize / sizeof(wchar_t))),
		std::wstring(REFVAL, (REFVSIZE / sizeof(wchar_t))));
}

TEST_F(DirectPlay8AddressWithWStringComponent, ComponentByNameBufferSizeBig)
{
	DWORD vsize = REFVSIZE * 2;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, (void*)(vbuf), &vsize, &type), S_OK);
	
	EXPECT_EQ(vsize, REFVSIZE);
	EXPECT_EQ(type, (DWORD)(DPNA_DATATYPE_STRING));
	
	EXPECT_EQ(std::wstring((const wchar_t*)(vbuf), (vsize / sizeof(wchar_t))),
		std::wstring(REFVAL, (REFVSIZE / sizeof(wchar_t))));
}

TEST_F(DirectPlay8AddressWithWStringComponent, ComponentByNameWrongName)
{
	DWORD vsize = sizeof(vbuf);
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(L"wrongkey", (void*)(vbuf), &vsize, &type), DPNERR_DOESNOTEXIST);
}

TEST_F(DirectPlay8AddressWithWStringComponent, Clear)
{
	ASSERT_EQ(idp8->Clear(), S_OK);
	
	{
		DWORD num;
		ASSERT_EQ(idp8->GetNumComponents(&num), S_OK);
		ASSERT_EQ(num, (DWORD)(0));
	}
	
	{
		DWORD vsize = sizeof(vbuf);
		DWORD type;
		
		EXPECT_EQ(idp8->GetComponentByName(REFKEY, (void*)(vbuf), &vsize, &type), DPNERR_DOESNOTEXIST);
	}
	
	{
		DWORD ksize = sizeof(kbuf);
		DWORD vsize = sizeof(vbuf);
		DWORD type;
		
		ASSERT_EQ(idp8->GetComponentByIndex(0, (wchar_t*)(kbuf), &ksize, (void*)(vbuf), &vsize, &type), DPNERR_DOESNOTEXIST);
	}
}

class DirectPlay8AddressWithAStringComponent: public DirectPlay8AddressInitial
{
	protected:
		const wchar_t *REFKEY = L"key";
		
		const char *REFVAL = "ASCII string value";
		const DWORD REFVSIZE = 19;
		
		unsigned char vbuf[256];
		
		virtual void SetUp() override
		{
			memset(vbuf, 0xFF, sizeof(vbuf));
			
			ASSERT_EQ(idp8->AddComponent(REFKEY, REFVAL, REFVSIZE, DPNA_DATATYPE_STRING_ANSI), S_OK);
		}
};

TEST_F(DirectPlay8AddressWithAStringComponent, HasOneComponent)
{
	DWORD num;
	ASSERT_EQ(idp8->GetNumComponents(&num), S_OK);
	EXPECT_EQ(num, (DWORD)(1));
}

TEST_F(DirectPlay8AddressWithAStringComponent, ComponentByNameBufferSizeZero)
{
	DWORD vsize = 0;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, NULL, &vsize, &type), DPNERR_BUFFERTOOSMALL);
	
	EXPECT_EQ(vsize, REFVSIZE);
}

TEST_F(DirectPlay8AddressWithAStringComponent, ComponentByNameBufferSizeSmall)
{
	DWORD vsize = REFVSIZE - 1;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, (void*)(vbuf), &vsize, &type), DPNERR_BUFFERTOOSMALL);
	
	EXPECT_EQ(vsize, REFVSIZE);
}

TEST_F(DirectPlay8AddressWithAStringComponent, ComponentByNameBufferSizeExact)
{
	DWORD vsize = REFVSIZE;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, (void*)(vbuf), &vsize, &type), S_OK);
	
	EXPECT_EQ(vsize, REFVSIZE);
	EXPECT_EQ(type, (DWORD)(DPNA_DATATYPE_STRING_ANSI));
	
	EXPECT_EQ(std::string((const char*)(vbuf), vsize), std::string(REFVAL, REFVSIZE));
}

TEST_F(DirectPlay8AddressWithAStringComponent, ComponentByNameBufferSizeBig)
{
	DWORD vsize = REFVSIZE * 2;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, (void*)(vbuf), &vsize, &type), S_OK);
	
	EXPECT_EQ(vsize, REFVSIZE);
	EXPECT_EQ(type, (DWORD)(DPNA_DATATYPE_STRING_ANSI));
	
	EXPECT_EQ(std::string((const char*)(vbuf), vsize), std::string(REFVAL, REFVSIZE));
}

class DirectPlay8AddressWithDWORDComponent: public DirectPlay8AddressInitial
{
	protected:
		const wchar_t *REFKEY = L"key";
		
		const DWORD REFVAL = 0x0EA7BEEF;
		const DWORD REFVSIZE = sizeof(REFVAL);
		
		unsigned char vbuf[256];
		
		virtual void SetUp() override
		{
			memset(vbuf, 0xFF, sizeof(vbuf));
			
			ASSERT_EQ(idp8->AddComponent(REFKEY, &REFVAL, REFVSIZE, DPNA_DATATYPE_DWORD), S_OK);
		}
};

TEST_F(DirectPlay8AddressWithDWORDComponent, HasOneComponent)
{
	DWORD num;
	ASSERT_EQ(idp8->GetNumComponents(&num), S_OK);
	EXPECT_EQ(num, (DWORD)(1));
}

TEST_F(DirectPlay8AddressWithDWORDComponent, ComponentByNameBufferSizeZero)
{
	DWORD vsize = 0;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, NULL, &vsize, &type), DPNERR_BUFFERTOOSMALL);
	
	EXPECT_EQ(vsize, REFVSIZE);
}

TEST_F(DirectPlay8AddressWithDWORDComponent, ComponentByNameBufferSizeSmall)
{
	DWORD vsize = REFVSIZE - 1;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, (void*)(vbuf), &vsize, &type), DPNERR_BUFFERTOOSMALL);
	
	EXPECT_EQ(vsize, REFVSIZE);
}

TEST_F(DirectPlay8AddressWithDWORDComponent, ComponentByNameBufferSizeExact)
{
	DWORD vsize = REFVSIZE;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, (void*)(vbuf), &vsize, &type), S_OK);
	
	EXPECT_EQ(vsize, REFVSIZE);
	EXPECT_EQ(type, (DWORD)(DPNA_DATATYPE_DWORD));
	
	EXPECT_EQ(*(DWORD*)(vbuf), REFVAL);
}

TEST_F(DirectPlay8AddressWithDWORDComponent, ComponentByNameBufferSizeBig)
{
	DWORD vsize = REFVSIZE * 2;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, (void*)(vbuf), &vsize, &type), S_OK);
	
	EXPECT_EQ(vsize, REFVSIZE);
	EXPECT_EQ(type, (DWORD)(DPNA_DATATYPE_DWORD));
	
	EXPECT_EQ(*(DWORD*)(vbuf), REFVAL);
}

class DirectPlay8AddressWithGUIDComponent: public DirectPlay8AddressInitial
{
	protected:
		const wchar_t *REFKEY = L"key";
		
		const GUID REFVAL = CLSID_DirectPlay8Address;
		const DWORD REFVSIZE = sizeof(REFVAL);
		
		unsigned char vbuf[256];
		
		virtual void SetUp() override
		{
			memset(vbuf, 0xFF, sizeof(vbuf));
			
			ASSERT_EQ(idp8->AddComponent(REFKEY, &REFVAL, REFVSIZE, DPNA_DATATYPE_GUID), S_OK);
		}
};

TEST_F(DirectPlay8AddressWithGUIDComponent, HasOneComponent)
{
	DWORD num;
	ASSERT_EQ(idp8->GetNumComponents(&num), S_OK);
	EXPECT_EQ(num, (DWORD)(1));
}

TEST_F(DirectPlay8AddressWithGUIDComponent, ComponentByNameBufferSizeZero)
{
	DWORD vsize = 0;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, NULL, &vsize, &type), DPNERR_BUFFERTOOSMALL);
	
	EXPECT_EQ(vsize, REFVSIZE);
}

TEST_F(DirectPlay8AddressWithGUIDComponent, ComponentByNameBufferSizeSmall)
{
	DWORD vsize = REFVSIZE - 1;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, (void*)(vbuf), &vsize, &type), DPNERR_BUFFERTOOSMALL);
	
	EXPECT_EQ(vsize, REFVSIZE);
}

TEST_F(DirectPlay8AddressWithGUIDComponent, ComponentByNameBufferSizeExact)
{
	DWORD vsize = REFVSIZE;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, (void*)(vbuf), &vsize, &type), S_OK);
	
	EXPECT_EQ(vsize, REFVSIZE);
	EXPECT_EQ(type, (DWORD)(DPNA_DATATYPE_GUID));
	
	EXPECT_EQ(*(GUID*)(vbuf), REFVAL);
}

TEST_F(DirectPlay8AddressWithGUIDComponent, ComponentByNameBufferSizeBig)
{
	DWORD vsize = REFVSIZE * 2;
	DWORD type;
	
	ASSERT_EQ(idp8->GetComponentByName(REFKEY, (void*)(vbuf), &vsize, &type), S_OK);
	
	EXPECT_EQ(vsize, REFVSIZE);
	EXPECT_EQ(type, (DWORD)(DPNA_DATATYPE_GUID));
	
	EXPECT_EQ(*(GUID*)(vbuf), REFVAL);
}
