#include "stdafx.h"


CHPRecoveryDBWriteJob::CHPRecoveryDBWriteJob(void)
	: mCharacterType(0)
	, mPosX(0.0f)
	, mPosY(0.0f)
	, mTileX(0)
	, mTileY(0)
	, mRotation(0)
	, mCristalCount(0)
	, mHP(0)
	, mEXP(0)
	, mLevel(0)
	, mbDieFlag(FALSE)
	, mOldHP(0)
	, mSitTimeSec(0)
{
}

CHPRecoveryDBWriteJob::~CHPRecoveryDBWriteJob(void)
{
}

void CHPRecoveryDBWriteJob::DBWrite(void)
{
REQUERY:

	if (mpDBConnector->Query((WCHAR*)L"begin;") == FALSE)
	{
		if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
		{
			mpDBConnector->Reconnect();

			goto REQUERY;
		}

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[HPRecoveryDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}

	// 캐릭터 DB 저장
	if (mpDBConnector->Query((WCHAR*)L"UPDATE `character` SET characterType = '%d', posX = '%lf', posY = '%lf', tileX = '%d', tileY = '%d', rotation = '%d', cristal = '%d', hp = '%d', exp = '%lld', level = '%d', die = '%d' WHERE accountno = '%lld'",
		mCharacterType, mPosX, mPosY, mTileX, mTileY, mRotation, mCristalCount, mHP, mEXP, mLevel, mbDieFlag, mAccountNo) == FALSE)
	{
		if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
		{
			mpDBConnector->Reconnect();

			goto REQUERY;
		}

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[LoginDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}


	// 게임 로그
	if (mpDBConnector->Query((WCHAR*)L"INSERT INTO `logdb`.`gamelog` (`logtime`, `accountno`,`servername`, `type`, `code`, `param1`, `param2`,`param3`) VALUES(NOW(), '%lld', 'Game', '5', '51', '%d', '%d','%d');",
		mAccountNo, mOldHP, mHP, mSitTimeSec) == FALSE)
	{
		if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
		{
			mpDBConnector->Reconnect();

			goto REQUERY;
		}

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[LogoutDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}


	if (mpDBConnector->Query((WCHAR*)L"commit;") == FALSE)
	{
		if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
		{
			mpDBConnector->Reconnect();

			goto REQUERY;
		}

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[HPRecoveryDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}

	return;
}



void CHPRecoveryDBWriteJob::SetHPRecoveryDBWriteJob(CTLSDBConnector* pDBConnector, CPlayer* pPlayer)
{
	mAccountNo = pPlayer->mAccountNo;
	mpDBConnector = pDBConnector;
	
	mCharacterType = pPlayer->mCharacterType;
	mPosX = pPlayer->mPosX;
	mPosY = pPlayer->mPosY;
	mTileX = pPlayer->mTileX;
	mTileY = pPlayer->mTileY;
	mRotation = pPlayer->mRotation;
	mCristalCount = pPlayer->mCristalCount;
	mHP = pPlayer->mHP;
	mEXP = pPlayer->mEXP;
	mLevel = pPlayer->mLevel;
	mbDieFlag = pPlayer->mbDieFlag;

	mOldHP = pPlayer->mOldHP;
	mSitTimeSec = timeGetTime() - pPlayer->mRecoveryHPStartTick;

	return;
}


void* CHPRecoveryDBWriteJob::operator new(size_t size)
{
	return mObjectFreeList.Alloc();
}


void CHPRecoveryDBWriteJob::operator delete(void* ptr)
{
	if (mObjectFreeList.Free((CLoginDBWriteJob*)ptr) == FALSE)
	{
		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[operator delete] delete Failed");

		CCrashDump::Crash();
	}

	return;
}

