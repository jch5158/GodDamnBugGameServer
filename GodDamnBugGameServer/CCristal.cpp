#include "stdafx.h"


CCristal::CCristal(void)
	: mClientID(0)
	, mCristalType(0)
	, mPosX(0)
	, mPosY(0)
	, mTileX(0)
	, mTileY(0)
	, mAmount(0)
	, mCurSector{ 0, }
{}

CCristal::~CCristal(void){}


void CCristal::SetCristalTypeAmount(INT typeAmountArray[])
{	
	for (INT count = (INT)eCristalType::Cristal1; count <= (INT)eCristalType::Cristal3; ++count)
	{
		mCristalAmountMap.insert(std::make_pair(count, typeAmountArray[count - 1]));
	}

	return;
}


CCristal* CCristal::Alloc(UINT64 clientID, FLOAT posX, FLOAT posY, INT tileX, INT tileY, CGodDamnBug::stSector curSector)
{
	CCristal* pCristal = mCristalFreeList.Alloc();

	pCristal->mClientID = clientID;

	pCristal->mCristalType = gatchaCristalType();

	pCristal->setCristalAmount();

	pCristal->mPosX = posX;
	pCristal->mPosY = posY;

	pCristal->mTileX = tileX;
	pCristal->mTileY = tileY;

	pCristal->mCurSector.posX = curSector.posX;
	pCristal->mCurSector.posY = curSector.posY;
		
	return pCristal;
}


void CCristal::Free(void)
{
	if (mCristalFreeList.Free(this) == FALSE)
	{
		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[Free] Cristal Free is Failed");

		CCrashDump::Crash();
	}

	return;
}

INT CCristal::GetCristalAmount(void) const
{	
	return mAmount;
}

BYTE CCristal::gatchaCristalType(void)
{
	DWORD randValue = rand() % 10;

	if (randValue < 6)
	{
		return (BYTE)eCristalType::Cristal1;
	}
	else if (randValue >= 6 && randValue < 9)
	{
		return (BYTE)eCristalType::Cristal2;
	}
	else
	{
		return (BYTE)eCristalType::Cristal3;
	}
}


void CCristal::setCristalAmount(void)
{
	auto iter = mCristalAmountMap.find(mCristalType);
	if (iter == mCristalAmountMap.end())
	{
		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[setCristalAmount] mCristalType : %d", mCristalType);

		CCrashDump::Crash();
	}

	mAmount = iter->second;

	return;
}