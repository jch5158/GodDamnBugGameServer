#include "stdafx.h"


CLoginDBWriteJob::CLoginDBWriteJob(void)
	: mStatus(0)
	, mCharacterType(0)
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
	, mClientPort(0)
	, mClientIP{ 0, }
{
}

CLoginDBWriteJob::~CLoginDBWriteJob(void)
{
}

void CLoginDBWriteJob::DBWrite(void)
{
REQUERY:

	if (mpDBConnector->Query((WCHAR*)L"begin;") == FALSE)
	{
		if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
		{
			mpDBConnector->Reconnect();

			goto REQUERY;
		}

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[LoginDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}

	if (mpDBConnector->Query((WCHAR*)L"UPDATE `accountdb`.`status` SET `status` = '%d' WHERE `accountno` = '%lld'", mStatus, mAccountNo) == FALSE)
	{
		if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
		{
			mpDBConnector->Reconnect();

			goto REQUERY;
		}

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[LoginDBWrite] MySQL Error Code : %d, Error Message : %s",mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}



	if (mCharacterType != 0)
	{
		// ĳ���� ������ DB �� �����Ѵ�.
		//if (mpDBConnector->Query((WCHAR*)L"UPDATE `character` SET characterType = '%d', posX = '%lf', posY = '%lf', tileX = '%d', tileY = '%d', rotation = '%d', cristal = '%d', hp = '%d', exp = '%lld', level = '%d', die = '%d' WHERE accountno = '%lld'",
		//	mCharacterType, mPosX, mPosY, mTileX, mTileY, mRotation, mCristalCount, mHP, mEXP, mLevel, mbDieFlag, mAccountNo) == FALSE)
		//{
		//	if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
		//	{
		//		mpDBConnector->Reconnect();

		//		goto REQUERY;
		//	}

		//	CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[LoginDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		//	CCrashDump::Crash();
		//}

		if (mpDBConnector->Query((WCHAR*)L"INSERT INTO `logdb`.`gamelog` (`logtime`, `accountno`,`servername`, `type`, `code`, `param1`, `param2`, `param3`, `param4`, `message`) VALUES(NOW(), '%lld', 'Game', '1', '11', '%d', '%d', '%d', '%d', '%s:%d');",
			mAccountNo, mTileX, mTileY, mCristalCount, mHP, mClientIP, mClientPort) == FALSE)
		{
			if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
			{
				mpDBConnector->Reconnect();

				goto REQUERY;
			}

			CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[LoginDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

			CCrashDump::Crash();
		}
	}
	else
	{
		if (mpDBConnector->Query((WCHAR*)L"INSERT INTO `logdb`.`gamelog` (`logtime`, `accountno`,`servername`, `type`, `code`,`message`) VALUES(NOW(), '%lld', 'Game', '1', '11', '%s:%d');",
			mAccountNo, mClientIP, mClientPort) == FALSE)
		{
			if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
			{
				mpDBConnector->Reconnect();

				goto REQUERY;
			}

			CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[LoginDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

			CCrashDump::Crash();
		}
	}

	if (mpDBConnector->Query((WCHAR*)L"commit;") == FALSE)
	{
		if (mpDBConnector->CheckReconnectErrorCode() == TRUE)
		{
			mpDBConnector->Reconnect();

			goto REQUERY;
		}

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[LoginDBWrite] MySQL Error Code : %d, Error Message : %s", mpDBConnector->GetLastError(), mpDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}

	return;
}


void CLoginDBWriteJob::SetLoginDBWriteJob(CTLSDBConnector* pDBConnector, CPlayer* pPlayer, BYTE status)
{	
	mAccountNo = pPlayer->mAccountNo;	
	mpDBConnector = pDBConnector;
	
	mCharacterType = pPlayer->mCharacterType;

	// mCharacterType�� 0�̶�� ĳ���� ������ ���� ���� ���Ͽ����� �ǹ��Ѵ�.
	if (mCharacterType != 0)
	{
		mStatus = status;
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
	}

	mClientPort = pPlayer->GetClientPort();
	pPlayer->GetClientIP(mClientIP, IP_LENGTH);

	return;
}

void* CLoginDBWriteJob::operator new(size_t size)
{
	return mObjectFreeList.Alloc();
}


void CLoginDBWriteJob::operator delete(void* ptr)
{
	if (mObjectFreeList.Free((CLoginDBWriteJob*)ptr) == FALSE)
	{
		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[operator delete] delete Failed");

		CCrashDump::Crash();
	}

	return;
}

