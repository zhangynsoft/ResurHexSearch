// FilesHexSearch.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "DebugMsg.h"

char GetHighHelf(char c)
{
    if (c>='A'&&c<='Z')
    {
        return (c-'A'+0x0a)<<4;
    }
    if (c>='a'&&c<='z'+0x0a)
    {
        return (c-'a'+0x0a)<<4;
    }

    return (c-'0')<<4;
}

char GetLowHelf(char c)
{
    if (c>='A'&&c<='Z')
    {
        return (c-'A'+0x0a)&0x0f;
    }
    if (c>='a'&&c<='z')
    {
        return (c-'a'+0x0a)&0x0f;
    }

    return (c-'0')&0x0f;
}

char pHex[128] = {0};
size_t HexLen = 0;

VOID Str2Hex(CString strHex)
{
    char* pStrHex = strHex.GetBuffer();
    int i=0;
    for(;i<strHex.GetLength()/2;++i)
    {
        pHex[i] = GetHighHelf(pStrHex[i*2])|GetLowHelf(pStrHex[i*2+1]);
    }

    pHex[i] = '\0';
}

BOOL SearchHex(CString strFileName)
{
    BOOL bRet = FALSE;

    HANDLE fileHandle = INVALID_HANDLE_VALUE;

    char *pReadBuf = NULL;
    do 
    {
        try
        {
            fileHandle = ::CreateFile( strFileName,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL );     

            if(INVALID_HANDLE_VALUE==fileHandle)
            {
                break;
            }

        }
        catch (...)
        {

        }

        DWORD fileSize = ::GetFileSize(fileHandle, NULL);

        if (fileSize==0)
        {
            break;
        }

        ///预留4个字节用于保存头部，后面每一个Block都使用前一个Block尾部的4个字节
        pReadBuf = new char[fileSize];

        DWORD dwRead = 0;
        BOOL bRet = ::ReadFile( fileHandle,
            pReadBuf,
            fileSize,
            &dwRead,
            NULL);

        if (!bRet)
        {
            break;
        }
        
        //WriteDbgMsg(strFileName + CString(" searching....\n"));
        char* pSearch = pReadBuf;
        
        while(pSearch<pReadBuf+dwRead)
        {
            if(0==memcmp(pSearch, pHex, HexLen))
            {
                CString strLine;
                strLine.Format("File [%s] found. offset:0x%x\n", strFileName, pSearch-pReadBuf);
                WriteDbgMsg( strLine );
                break;
            }
            pSearch++;
        }

    }while(0);

    ::CloseHandle(fileHandle);
    delete[] pReadBuf;

    return bRet;
}

void find(CString strPath)
{
    CString strFind;
    WIN32_FIND_DATA FindFileData;

    strFind  = strPath + CString("\\*.*");

    HANDLE hFind=::FindFirstFile(strFind,&FindFileData);
    if(INVALID_HANDLE_VALUE == hFind)    return;

    while(TRUE)
    {
        if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if(FindFileData.cFileName[0]!='.')
            {
                CString strChildPath = strPath + CString("\\") + CString(FindFileData.cFileName);
                find(strChildPath);
            }
        }
        else
        {
            SearchHex( strPath + CString("\\") + CString(FindFileData.cFileName) );
        }
        if(!::FindNextFile(hFind,&FindFileData))    break;
    }

    ::FindClose(hFind);
}

int _tmain(int argc, _TCHAR* argv[])
{
    if (argc==3)
    {
        CString strPath = argv[1];
        CString strHex = argv[2];

        if (strHex.GetLength()%2!=0)
        {
            WriteDbgMsg("Please input hex numer correctly.\n");
            return -1;
        }

        WriteDbgMsg("==========================FilesHexSearch==========================\n");
        WriteDbgMsg(CString(" The parent path is:")+strPath);
        HexLen = strHex.GetLength()/2;
        Str2Hex(strHex);

        find(strPath);
    }
	return 0;
}

