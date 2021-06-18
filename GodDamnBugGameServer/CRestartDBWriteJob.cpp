#include "stdafx.h"


CRestartDBWriteJob::CRestartDBWriteJob(void)
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

CRestartDBWriteJob::~CRestartDBWriteJob(void)
{

}

void CRestartDBWriteJob::DBWrite(void)
{
REQUERY:

	if (mpDBConnector->Query((WCHAR*)L"begin;") == FALSE)
	{
		if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
		{
			mpDBConnector->Reconnect();

			goto REQUERY;
		}

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[RestartDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}

	// ĳ���� ������ DB �� �����Ѵ�.
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


	// ���� �α�
	if (mpDBConnector->Query((WCHAR*)L"INSERT INTO `logdb`.`gamelog` (`logtime`, `accountno`,`servername`, `type`, `code`, `param1`, `param2`) VALUES(NOW(), '%lld', 'Game', '3', '33', '%d', '%d');",
		mAccountNo, mTileX, mTileY) == FALSE)
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

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[RestartDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}

	return;
}

void CRestartDBWriteJob::SetRestartDBWriteJob(CTLSDBConnector* pDBConnector, CPlayer* pPlayer)
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


void* CRestartDBWriteJob::operator new(size_t size)
{
	return mObjectFreeList.Alloc();
}


void CRestartDBWriteJob::operator delete(void* ptr)
{
	if (mObjectFreeList.Free((CLoginDBWriteJob*)ptr) == FALSE)
	{
		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[operator delete] delete Failed");

		CCrashDump::Crash();
	}

	return;
}

