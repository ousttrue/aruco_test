#pragma once

inline void OutputDebugPrintfA(LPCSTR pszFormat, ...)
{
	va_list	argp;
	char pszBuf[256];
	va_start(argp, pszFormat);
	vsprintf_s(pszBuf, pszFormat, argp);
	va_end(argp);
	OutputDebugStringA(pszBuf);
}

