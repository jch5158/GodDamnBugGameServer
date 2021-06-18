#pragma once

#include "stdafx.h"
#include <conio.h>


class CServerController
{

public:

	CServerController(void)
		: mbControlModeFlag(FALSE)
		, mbShutdownFlag(FALSE)
		, mpMonitoringClient(nullptr)
	    , mpLoginClient(nullptr)
		, mpGodDamnBug(nullptr)
	{
	}

	~CServerController(void)
	{
	}


	BOOL GetShutdownFlag(void) const
	{
		return mbShutdownFlag;
	}

	void SetGameServer(CGodDamnBug* pGodDamnBug)
	{
		mpGodDamnBug = pGodDamnBug;

		return;
	}

	void SetMonitoringClient(CLanMonitoringClient* pMonitoringClient)
	{
		mpMonitoringClient = pMonitoringClient;

		return;
	}

	void SetLoginClient(CLanLoginClient* pLoginClient)
	{
		mpLoginClient = pLoginClient;

		return;
	}

	void ServerControling(void)
	{

		if (_kbhit() == TRUE)
		{
			WCHAR controlKey = _getwch();

			if (controlKey == L'u' || controlKey == L'U')
			{
				mbControlModeFlag = TRUE;
			}

			if ((controlKey == L'd' || controlKey == L'D') && mbControlModeFlag)
			{
				CCrashDump::Crash();
			}

			if ((controlKey == L'q' || controlKey == L'Q') && mbControlModeFlag)
			{
				mbShutdownFlag = TRUE;
			}

			if (controlKey == L'l' || controlKey == L'L')
			{
				mbControlModeFlag = FALSE;
			}
		}


		wprintf_s(L"\n\n\n\n"
			L"                                                              [ Game Server ]\n"
			L" 旨收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收旬\n\n"
			L"    Game Server Bind IP : %s | Game Server Bind Port : %d | Game Server Accept Total : %lld |  Current Client : %4d / %4d \n\n"
			L"    Game Current Client : %4d | Auth Current Client : %4d | Running Thread : %d | Worker Thread : %d | Nagle : %d \n\n"
			L"    Accept TPS : %d | Send TPS : %d | Recv TPS : %d | Authentic TPS : %d | Update TPS : %d\n\n"
			L"  收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收 \n\n"
			L"    Monitoring Connect IP : %s | Monitoring Connect Port : %d | Connect State : %d\n\n"
			L"    Running Thread : %d | Worker Thread : %d | Nagle : %d    \n\n"
			L"  收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收 \n\n"
			L"    Login Connect IP : %s | Login Connect Port : %d | Connect State : %d\n\n"
			L"    Running Thread : %d | Worker Thread : %d | Nagle : %d\n\n"
			L"  收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收 \n\n"
			L"    Control Mode : %d  | [ L ] : Control Lock | [ U ] : Control Unlock | [ D ] : Crash | [ Q ] : Exit | LogLevel : %d \n\n"
			L"  收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收 \n\n"
			L"    Message Chunk Alloc Count : %d | Monster Chunk Alloc Count : %d | Cristal Chunk Alloc Count : %d \n\n"
			L"    DBWriteJob Chunk Alloc Count : %d \n\n"
			L" 曲收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收收旭\n\n"
			, mpGodDamnBug->GetServerBindIP(), mpGodDamnBug->GetServerBindPort(), mpGodDamnBug->GetAcceptTotal(), mpGodDamnBug->GetCurrentClientCount(), mpGodDamnBug->GetMaxClientCount()
			, mpGodDamnBug->GetGameClientCount(), mpGodDamnBug->GetAuthClientCount(), mpGodDamnBug->GetRunningThreadCount(), mpGodDamnBug->GetWorkerThreadCount(), mpGodDamnBug->GetNagleFlag()
			, mpGodDamnBug->GetAcceptTPS(),mpGodDamnBug->GetSendTPS(), mpGodDamnBug->GetRecvTPS() , mpGodDamnBug->GetAuthenticTPS(), mpGodDamnBug->GetUpdateTPS()
			, mpMonitoringClient->GetConnectIP(), mpMonitoringClient->GetConnectPort(), mpMonitoringClient->GetConnectStateFlag()
			, mpMonitoringClient->GetRunningThreadCount(), mpMonitoringClient->GetWorkerThreadCount(), mpMonitoringClient->GetNagleFlag()
			, mpLoginClient->GetConnectIP(), mpLoginClient->GetConnectPort(), mpLoginClient->GetConnectStateFlag()
			, mpLoginClient->GetRunningThreadCount(), mpLoginClient->GetWorkerThreadCount(), mpLoginClient->GetNagleFlag()
			, mbControlModeFlag, CSystemLog::GetInstance()->GetLogLevel()
			, CLockFreeObjectFreeList<CTLSLockFreeObjectFreeList<CMessage>::CChunk>::GetAllocNodeCount(), CLockFreeObjectFreeList<CTLSLockFreeObjectFreeList<CMonster>::CChunk>::GetAllocNodeCount()
			, CLockFreeObjectFreeList<CTLSLockFreeObjectFreeList<CCristal>::CChunk>::GetAllocNodeCount(), CLockFreeObjectFreeList<CTLSLockFreeObjectFreeList<CLoginDBWriteJob>::CChunk>::GetAllocNodeCount()
		);

	}

private:

	BOOL mbControlModeFlag;

	BOOL mbShutdownFlag;

	CLanMonitoringClient* mpMonitoringClient;

	CLanLoginClient* mpLoginClient;

	CGodDamnBug* mpGodDamnBug;
};

