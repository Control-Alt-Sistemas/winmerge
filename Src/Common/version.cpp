/**
 *  @file version.cpp
 *
 *  @brief Implementation of CVersionInfo class
 */ 


#include "stdafx.h"
#include "version.h"

CVersionInfo::CVersionInfo(LPCTSTR szFileToVersion /* = NULL*/, 
						   LPCTSTR szLanguage /* = NULL*/,
						   LPCTSTR szCodepage /* = NULL*/)
{
	Init();
	m_strFileName = szFileToVersion;
	m_strLanguage = szLanguage;
	m_strCodepage = szCodepage;
}

CVersionInfo::CVersionInfo(HINSTANCE hModule)
{
	Init();
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(hModule, szFileName, MAX_PATH);
	m_strFileName = szFileName;
}

void CVersionInfo::Init()
{
	m_lpstrVffInfo = NULL;
	m_strFileName = _T("");
	m_strLanguage = _T("");
	m_strCodepage = _T("");
	m_strCompanyName = _T("");
	m_strFileDescription  = _T("");
	m_strFileVersion = _T(""); 
	m_strInternalName = _T(""); 
	m_strLegalCopyright = _T(""); 
	m_strOriginalFilename = _T(""); 
	m_strProductName = _T(""); 
	m_strProductVersion = _T(""); 
	m_strComments = _T("");
	m_strSpecialBuild = _T("");
	m_strPrivateBuild = _T("");
	m_bQueryDone=FALSE;
	memset(&m_FixedFileInfo, 0, sizeof(m_FixedFileInfo));
}

CString CVersionInfo::GetFileVersion()
{
	if (!m_bQueryDone)
		GetVersionInfo();
	return m_strFileVersion;
}

CString CVersionInfo::GetPrivateBuild()
{
	if (!m_bQueryDone)
		GetVersionInfo();
	return m_strPrivateBuild;
}

CString CVersionInfo::GetSpecialBuild()
{
	if (!m_bQueryDone)
		GetVersionInfo();
	return m_strSpecialBuild;
}

CString CVersionInfo::GetCompanyName()
{
	if (!m_bQueryDone)
		GetVersionInfo();
	return m_strCompanyName;
}

CString CVersionInfo::GetFileDescription()
{
	if (!m_bQueryDone)
		GetVersionInfo();
	return m_strFileDescription;
}

CString CVersionInfo::GetInternalName()
{
	if (!m_bQueryDone)
		GetVersionInfo();
	return m_strInternalName;
}

CString CVersionInfo::GetLegalCopyright()
{
	if (!m_bQueryDone)
		GetVersionInfo();
	return m_strLegalCopyright;
}

CString CVersionInfo::GetOriginalFilename()
{
	if (!m_bQueryDone)
		GetVersionInfo();
	return m_strOriginalFilename;
}

CString CVersionInfo::GetProductVersion()
{
	if (!m_bQueryDone)
		GetVersionInfo();
	return m_strProductVersion;
}

static CString MakeVersionString(DWORD hi, DWORD lo)
{
	CString sver;
	sver.Format(_T("%d.%d.%d.%d"), HIWORD(hi), LOWORD(hi), HIWORD(lo), LOWORD(lo));
	return sver;
}

CString CVersionInfo::GetFixedProductVersion()
{
	if (!m_bQueryDone)
		GetVersionInfo();
	if (!m_dwVerInfoSize)
		return _T("");
	return MakeVersionString(m_FixedFileInfo.dwProductVersionMS
		, m_FixedFileInfo.dwProductVersionLS);
}

CString CVersionInfo::GetFixedFileVersion()
{
	if (!m_bQueryDone)
		GetVersionInfo();
	if (!m_dwVerInfoSize)
		return _T("");
	return MakeVersionString(m_FixedFileInfo.dwFileVersionMS
		, m_FixedFileInfo.dwFileVersionLS);
}

CString CVersionInfo::GetComments()
{
	if (!m_bQueryDone)
		GetVersionInfo();
	return m_strComments;
}

void CVersionInfo::GetVersionInfo()
{
	DWORD   dwVerHnd=0;			// An 'ignored' parameter, always '0'
	TCHAR szFileName[MAX_PATH];

	m_bQueryDone=TRUE;
	if (m_strFileName.IsEmpty())
		GetModuleFileName(NULL, szFileName, MAX_PATH);
	else
		_tcscpy(szFileName, m_strFileName);
	
	m_dwVerInfoSize = GetFileVersionInfoSize(szFileName, &dwVerHnd);
	if (m_dwVerInfoSize) {
		HANDLE  hMem;
		hMem = GlobalAlloc(GMEM_MOVEABLE, m_dwVerInfoSize);
		m_lpstrVffInfo  = (LPTSTR)GlobalLock(hMem);
		if (GetFileVersionInfo(szFileName, dwVerHnd, m_dwVerInfoSize, m_lpstrVffInfo))
		{
			GetFixedVersionInfo();
			if (m_strLanguage.IsEmpty()
				|| m_strCodepage.IsEmpty())
			{
				LPWORD langInfo;
				DWORD langLen;
				if (VerQueryValue((LPVOID)m_lpstrVffInfo,
					_T("\\VarFileInfo\\Translation"),
					(LPVOID *)&langInfo, (UINT *)&langLen))
				{
					m_strLanguage.Format(_T("%4.4X"), langInfo[0]);	
					m_strCodepage.Format(_T("%4.4X"), langInfo[1]);	
				}
			}
			QueryValue(_T("CompanyName"), m_strCompanyName);
			QueryValue(_T("FileDescription"), m_strFileDescription);
			QueryValue(_T("FileVersion"), m_strFileVersion); 
			QueryValue(_T("InternalName"), m_strInternalName); 
			QueryValue(_T("LegalCopyright"), m_strLegalCopyright); 
			QueryValue(_T("OriginalFilename"), m_strOriginalFilename); 
			QueryValue(_T("ProductName"), m_strProductName); 
			QueryValue(_T("ProductVersion"), m_strProductVersion); 
			QueryValue(_T("Comments"), m_strComments);
			QueryValue(_T("SpecialBuild"), m_strSpecialBuild);
			QueryValue(_T("PrivateBuild"), m_strPrivateBuild);
		}
		GlobalUnlock(hMem);
		GlobalFree(hMem);
	}
}

void CVersionInfo::QueryValue(LPCTSTR szId, CString& s)
{
	ASSERT(m_lpstrVffInfo != NULL);
	LPSTR   lpVersion;			// String pointer to 'version' text
	UINT    uVersionLen;
	BOOL    bRetCode;

	TCHAR szSelector[256];	
	_stprintf(szSelector,_T("\\StringFileInfo\\%s%s\\%s"),
			m_strLanguage, m_strCodepage, szId);
	bRetCode = VerQueryValue((LPVOID)m_lpstrVffInfo,
		szSelector,
		(LPVOID *)&lpVersion,
		&uVersionLen);
	
	if(bRetCode)
	{
		s = lpVersion;
		s.TrimLeft();
		s.TrimRight();
	}
	else
		s = _T("");
}

void CVersionInfo::GetFixedVersionInfo()
{
	VS_FIXEDFILEINFO * pffi;
	UINT len = sizeof(*pffi);
	BOOL bRetCode = VerQueryValue(
		(LPVOID)m_lpstrVffInfo, _T("\\"), (LPVOID *)&pffi, &len);
	memcpy(&m_FixedFileInfo, pffi, sizeof(m_FixedFileInfo));
}


/*******************************************************************************
       END OF FILE
*******************************************************************************/

