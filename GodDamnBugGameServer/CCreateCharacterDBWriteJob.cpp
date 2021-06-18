#include "stdafx.h"

CCreateCharacterDBWriteJob::CCreateCharacterDBWriteJob(void)
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
{
}

CCreateCharacterDBWriteJob::~CCreateCharacterDBWriteJob(void)
{

	
}

void CCreateCharacterDBWriteJob::DBWrite(void)
{
REQUERY:

	if (mpDBConnector->Query((WCHAR*)L"begin;") == FALSE)
	{
		if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
		{
			mpDBConnector->Reconnect();

			goto REQUERY;
		}

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[CreateDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}


	if (mpDBConnector->Query((WCHAR*)L"INSERT INTO `character` (`accountno`,`charactertype`, `posx`, `posy`, `tilex`, `tiley`, `rotation`, `cristal`, `hp`, `exp`, `level`, `die`) VALUES ('%lld','%d', '%lf', '%lf', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d');",
		mAccountNo, mCharacterType, mPosX, mPosY, mTileX, mTileY, mRotation, mCristalCount, mHP, mEXP, mLevel, mbDieFlag) == FALSE)
	{
		if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
		{
			mpDBConnector->Reconnect();

			goto REQUERY;
		}

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[CreateDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}

	// 게임 로그
	if (mpDBConnector->Query((WCHAR*)L"INSERT INTO `logdb`.`gamelog` (`logtime`, `accountno`,`servername`, `type`, `code`, `param1`) VALUES(NOW(), '%lld', 'Game', '3', '32', '%d');",
		mAccountNo, mCharacterType) == FALSE)
	{
		if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
		{
			mpDBConnector->Reconnect();

			goto REQUERY;
		}

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[DieDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}

	if (mpDBConnector->Query((WCHAR*)L"commit;") == FALSE)
	{
		if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
		{
			mpDBConnector->Reconnect();

			goto REQUERY;
		}

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[CreateDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}

	
	return;
}

void CCreateCharacterDBWriteJob::SetCreateCharacterDBWriteJob(CTLSDBConnector* pDBConnector, CPlayer* pPlayer)
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

	return;
}


void* CCreateCharacterDBWriteJob::operator new(size_t size)
{
	return mObjectFreeList.Alloc();
}


void CCreateCharacterDBWriteJob::operator delete(void* ptr)
{
	if (mObjectFreeList.Free((CLoginDBWriteJob*)ptr) == FALSE)
	{
		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[operator delete] delete Failed");

		CCrashDump::Crash();
	}

	return;
}

