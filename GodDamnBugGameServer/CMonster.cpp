#include "stdafx.h"


CMonster::CMonster(void)
	: mClientID(0)
	, mMonsterType(0)
	, mActivityArea(0)
	, mbDieFlag(FALSE)
	, mHP(0)
	, mDamage(0)
	, mPosX(0)
	, mPosY(0)
	, mTileX(0)
	, mTileY(0)
	, mRotation(0)
	, mCurSector{ 0, }
	, mOldSector{ 0, }
	, mPlayerAccountNo(-1)
	, mLastSufferTick(0)
	, mLastMoveTick(0)
	, mLastMoveForAttackTick(0)
	, mLastAttackTick(0)
	, mpGodDamnBug(nullptr)
{}

CMonster::~CMonster(void){}

void CMonster::SetMonsterDefaultValue(INT initializeHP, INT initializeDamage)
{
	mInitializeHP = initializeHP;

	mInitializeDamage = initializeDamage;

	return;
}

CMonster* CMonster::Alloc(UINT64 clientID, INT monsterType, INT activityArea, CGodDamnBug *pGodDamnBug)
{
	CMonster* pMonster = mMonsterFreeList.Alloc();

	pMonster->setMonsterInfo(clientID, monsterType, activityArea);
	
	pMonster->mpGodDamnBug = pGodDamnBug;

	return pMonster;
}


void CMonster::Free(void)
{
	if (mMonsterFreeList.Free(this) == FALSE)
	{
		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[Free] Monster Free is Failed");

		CCrashDump::Crash();
	}

	return;
}

void CMonster::setMonsterRespawnCoordinate(void)
{
	switch ((eMonsterActivityArea)mActivityArea)
	{
	case eMonsterActivityArea::MonsterArea1:

		mTileX = (X_MONSTER_RESPAWN_CENTER_1 - 16) + (rand() % 33);
		mTileY = (Y_MONSTER_RESPAWN_CENTER_1 - 16) + (rand() % 33);

		break;
	
	case eMonsterActivityArea::MonsterArea2:

		mTileX = (X_MONSTER_RESPAWN_CENTER_2 - 16) + (rand() % 33);
		mTileY = (Y_MONSTER_RESPAWN_CENTER_2 - 16) + (rand() % 33);

		break;
	
	case eMonsterActivityArea::MonsterArea3:

		mTileX = (X_MONSTER_RESPAWN_CENTER_3 - 16) + (rand() % 33);
		mTileY = (Y_MONSTER_RESPAWN_CENTER_3 - 16) + (rand() % 33);

		break;

	case eMonsterActivityArea::MonsterArea4:

		mTileX = (X_MONSTER_RESPAWN_CENTER_4 - 16) + (rand() % 33);
		mTileY = (Y_MONSTER_RESPAWN_CENTER_4 - 16) + (rand() % 33);

		break;
	
	case eMonsterActivityArea::MonsterArea5:

		mTileX = (X_MONSTER_RESPAWN_CENTER_5 - 16) + (rand() % 33);
		mTileY = (Y_MONSTER_RESPAWN_CENTER_5 - 16) + (rand() % 33);

		break;
	
	case eMonsterActivityArea::MonsterArea6:

		mTileX = (X_MONSTER_RESPAWN_CENTER_6 - 16) + (rand() % 33);
		mTileY = (Y_MONSTER_RESPAWN_CENTER_6 - 16) + (rand() % 33);

		break;
	
	case eMonsterActivityArea::MonsterArea7:

		mTileX = (X_MONSTER_RESPAWN_CENTER_7 - 16) + (rand() % 33);
		mTileY = (Y_MONSTER_RESPAWN_CENTER_7 - 16) + (rand() % 33);

		break;
	
	default:

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[setMonsterRespawnCoordinate] mActivityArea Value Error : %d", mActivityArea);

		CCrashDump::Crash();

		break;
	}

	mPosX = X_BLOCK(mTileX);
	mPosY = Y_BLOCK(mTileY);

	mCurSector.posX = X_SECTOR(mTileX);
	mCurSector.posY = Y_SECTOR(mTileY);

	return;
}


void CMonster::setMonsterTypeStats(void)
{
	switch ((eMonsterType)mMonsterType)
	{
	case eMonsterType::Monster1:
		
		mHP = mInitializeHP;

		mDamage = mInitializeDamage;

		break;
	default:

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[setMonsterTypeStats] monsterType Error : %d", mMonsterType);

		CCrashDump::Crash();

		break;
	}

	return;
}


void CMonster::setMonsterInfo(UINT64 clientID, INT monsterType, INT activityArea)
{
	mClientID = clientID;

	mMonsterType = monsterType;

	mActivityArea = activityArea;

	// Ÿ�Կ� ���� ���� ü��, �������� �����Ѵ�.
	setMonsterTypeStats();

	// ������ ������ ��ǥ�� �����Ѵ�.
	setMonsterRespawnCoordinate();

	// ���� ���͸� �������.
	CGodDamnBug::GetSector(mTileX, mTileY, &mCurSector);

	mbDieFlag = FALSE;

	mRotation = rand() % 360;

	mPlayerAccountNo = -1;

	mLastMoveForAttackTick = 0;

	mLastSufferTick = 0;

	mLastMoveTick = 0;

	mLastAttackTick = 0;

	mLastDieTick = 0;

	return;
}



void CMonster::Update(void)
{
	revivalMonster();

	moveMonster();

	attackToAttackedPlayer();

	return;
}

void CMonster::UpdateAttackDamage(UINT64 accountNo, INT damage)
{
	mLastSufferTick = timeGetTime();

	// � Ŭ���̾�Ʈ�� �����ߴ��� Ȯ���Ѵ�.
	mPlayerAccountNo = accountNo;

	mHP -= damage;

	if (mHP <= 0)
	{
		// �׾����� ������ �÷��̾� �ʱ�ȭ
		mPlayerAccountNo = -1;

		mLastMoveForAttackTick = 0;

		mHP = 0;

		mbDieFlag = TRUE;

		mLastDieTick = timeGetTime();

		mpGodDamnBug->EraseMonsterSectorMap(mCurSector.posX, mCurSector.posY, mClientID);

		sendAroundMonsterDie();

		mpGodDamnBug->CreateCristal(mPosX, mPosY, mTileX, mTileY, mCurSector);
	}

	return;
}

void CMonster::revivalMonster(void)
{
	if (mbDieFlag == FALSE)
	{
		return;
	}

	// ������ �ð� �ڿ� ���� ��Ȱ �ڿ� ���� ��Ȱ
	if (timeGetTime() - mLastDieTick < MONSTER_REVIVAL)
	{
		return;
	}


	// Ÿ�Կ� ���� ���� ü��, �������� �����Ѵ�.
	setMonsterTypeStats();

	// ������ ������ ��ǥ�� �����Ѵ�.
	setMonsterRespawnCoordinate();

	// ���� ���͸� �������.
	CGodDamnBug::GetSector(mTileX, mTileY, &mCurSector);

	mbDieFlag = FALSE;

	mRotation = rand() % 360;

	mPlayerAccountNo = -1;

	mLastMoveForAttackTick = 0;

	mLastSufferTick = 0;

	mLastMoveTick = 0;

	mLastAttackTick = 0;

	mLastDieTick = 0;

	mpGodDamnBug->InsertMonsterSectorMap(mCurSector.posX, mCurSector.posY, mClientID, this);

	sendAroundCreateMonster();

	return;
}

void CMonster::moveMonster(void)
{
	if (mbDieFlag == TRUE)
	{
		return;
	}
	
	if (timeGetTime() - mLastMoveTick < MONSTER_SPEED)
	{
		return;
	}

	FLOAT moveX;
	FLOAT moveY;

	INT moveTileX;
	INT moveTileY;

	INT rotation;
	
	if (mPlayerAccountNo != -1)
	{
		EnterCriticalSection(&mpGodDamnBug->mPlayerMapLock);

		// Player �� ���� �б⸸ �����ϱ� ������ ���� ���� �ʴ´�.
		CPlayer* pPlayer = mpGodDamnBug->FindPlayerFromPlayerMap(mPlayerAccountNo);

		LeaveCriticalSection(&mpGodDamnBug->mPlayerMapLock);
		
		// �÷��̾ �������� �ʴٸ�
		if (pPlayer == nullptr || pPlayer->mbDieFlag == TRUE)
		{
			mPlayerAccountNo = -1;

			mLastMoveForAttackTick = 0;

			// ���� ������ ����.
			rotation = rand() % 360;

			// ������ �� ��ǥ�� ����.
			getMonsterMoveCoordinate(rotation, &moveX, &moveY);
		}
		else
		{
			// �����ߴ� �÷��̾ ����� ���Ϳ� �ִ��� Ȯ���Ѵ�.
			if (checkAttackedPlayerSector(pPlayer) == FALSE)
			{
				return;
			}

			// ����� ���Ϳ� �ִ� �÷��̾�� �������� ������ ����.
			getRotationForAttackedPlayer(pPlayer, &rotation);

			// ������ �� ��ǥ�� ����.
			getMonsterToAttackedCoordinate(pPlayer, rotation, &moveX, &moveY);

			if (timeGetTime() - mLastSufferTick > MONSTER_RAGE_TIME)
			{
				// ���� �г� ����
				mPlayerAccountNo = -1;

				mLastMoveForAttackTick = 0;
			}
			else
			{
				mLastMoveForAttackTick = timeGetTime();
			}
		}
	}
	else
	{
		// ���� ������ ����.
		rotation = rand() % 360;

		// ������ �� ��ǥ�� ����.
		getMonsterMoveCoordinate(rotation, &moveX, &moveY);
	}



	moveTileX = (INT)X_TILE(moveX);
	moveTileY = (INT)Y_TILE(moveY);

	// ������ ����ٸ� ���� �����ӿ��� �����̼� ���� ���� �����δ�.
	if (chaeckMonsterMoveRange(moveTileX, moveTileY) == FALSE)
	{
		return;
	}

	mRotation = getGameRotation(rotation);

	mPosX = moveX;
	mPosY = moveY;

	mTileX = moveTileX;
	mTileY = moveTileY;

	// ���� �̵��� ���� �� 
	sendAroundMonsterMoveStart();

	if (updateMonsterSector() == TRUE)
	{
		// ���� ������ ��ȭ�� �־��ٸ� �ֺ� Ŭ���̾�Ʈ���� ��ȭ�� ������ �˷��ش�.
		sendAroundUpdateMonsterSector();
	}

	// ������ ó���� �Ϸ�Ǿ��ٸ� mLastMoveTick �� ���Ž����־� 500ms ���� ������ �� �ֵ��� ���ش�.
	mLastMoveTick = timeGetTime();

	return;
}



void CMonster::attackToAttackedPlayer(void)
{
	if (mbDieFlag == TRUE)
	{
		return;
	}

	if (mPlayerAccountNo == -1)
	{
		return;
	}

	EnterCriticalSection(&mpGodDamnBug->mPlayerMapLock);

	CPlayer* pPlayer = mpGodDamnBug->FindPlayerFromPlayerMap(mPlayerAccountNo);
	if (pPlayer == nullptr)
	{
		mPlayerAccountNo = -1;

		mLastMoveForAttackTick = 0;

		LeaveCriticalSection(&mpGodDamnBug->mPlayerMapLock);

		return;
	}

	LeaveCriticalSection(&mpGodDamnBug->mPlayerMapLock);

	do
	{
		if (pPlayer->mbDieFlag == TRUE)
		{
			mPlayerAccountNo = -1;

			mLastMoveForAttackTick = 0;

			break;
		}

		// �����̰� 1�ʵڿ� ������ �� �ִ�.
		if (timeGetTime() - mLastMoveForAttackTick < 1000 || mLastMoveForAttackTick == 0)
		{
			break;
		}

		if (timeGetTime() - mLastAttackTick < MONSTER_ATTACK_SPEED)
		{
			break;
		}

		if (checkAttackDistance(pPlayer) == FALSE)
		{
			break;
		}

		// ���� �׼� �Ѹ���
		sendAroundMonsterAttackAction();

		// ������ ó��
		sendAroundMonsterAttackDamage(pPlayer);

		pPlayer->UpdateAttackDamage(mDamage);

		mLastAttackTick = timeGetTime();

	} while (0);

	return;
}

BOOL CMonster::checkAttackDistance(CPlayer* pPlayer)
{
	CGodDamnBug::stCollisionBox collisionBox;
	CGodDamnBug::GetAttackCollisionBox(mRotation, mTileX, mTileY, MONSTER_ATTACK_RANGE, &collisionBox);

	if (pPlayer->mTileX >= collisionBox.pointOne.tileX && pPlayer->mTileY >= collisionBox.pointOne.tileY
		&& pPlayer->mTileX <= collisionBox.pointTwo.tileX && pPlayer->mTileY <= collisionBox.pointTwo.tileY)
	{
		return TRUE;
	}

	return FALSE;
}




BOOL CMonster::chaeckMonsterMoveRange(INT moveTileX, INT moveTileY)
{

	switch ((eMonsterActivityArea)mActivityArea)
	{
	case eMonsterActivityArea::MonsterArea1:

		if (moveTileX > X_MONSTER_RESPAWN_CENTER_1 + MONSTER_RESPAWN_RANGE_1 || moveTileX < X_MONSTER_RESPAWN_CENTER_1 - MONSTER_RESPAWN_RANGE_1
			|| moveTileY > Y_MONSTER_RESPAWN_CENTER_1 + MONSTER_RESPAWN_RANGE_1 || moveTileY < Y_MONSTER_RESPAWN_CENTER_1 - MONSTER_RESPAWN_RANGE_1)		
		{
			return FALSE;
		}	

		break;
	case eMonsterActivityArea::MonsterArea2:

		if (moveTileX > X_MONSTER_RESPAWN_CENTER_2 + MONSTER_RESPAWN_RANGE_2 || moveTileX < X_MONSTER_RESPAWN_CENTER_2 - MONSTER_RESPAWN_RANGE_2
			|| moveTileY > Y_MONSTER_RESPAWN_CENTER_2 + MONSTER_RESPAWN_RANGE_2 || moveTileY < Y_MONSTER_RESPAWN_CENTER_2 - MONSTER_RESPAWN_RANGE_2)
		{
			return FALSE;
		}

		break;
	case eMonsterActivityArea::MonsterArea3:

		if (moveTileX > X_MONSTER_RESPAWN_CENTER_3 + MONSTER_RESPAWN_RANGE_3 || moveTileX < X_MONSTER_RESPAWN_CENTER_3 - MONSTER_RESPAWN_RANGE_3
			|| moveTileY > Y_MONSTER_RESPAWN_CENTER_3 + MONSTER_RESPAWN_RANGE_3 ||  moveTileY < Y_MONSTER_RESPAWN_CENTER_3 - MONSTER_RESPAWN_RANGE_3)
		{
			return FALSE;
		}

		break;
	case eMonsterActivityArea::MonsterArea4:

		if (moveTileX > X_MONSTER_RESPAWN_CENTER_4 + MONSTER_RESPAWN_RANGE_4 || moveTileX < X_MONSTER_RESPAWN_CENTER_4 - MONSTER_RESPAWN_RANGE_4
			|| moveTileY > Y_MONSTER_RESPAWN_CENTER_4 + MONSTER_RESPAWN_RANGE_4 || moveTileY < Y_MONSTER_RESPAWN_CENTER_4 - MONSTER_RESPAWN_RANGE_4)
		{
			return FALSE;
		}


		break;
	case eMonsterActivityArea::MonsterArea5:

		if (moveTileX > X_MONSTER_RESPAWN_CENTER_5 + MONSTER_RESPAWN_RANGE_5 || moveTileX < X_MONSTER_RESPAWN_CENTER_5 - MONSTER_RESPAWN_RANGE_5
			|| moveTileY > Y_MONSTER_RESPAWN_CENTER_5 + MONSTER_RESPAWN_RANGE_5 || moveTileY < Y_MONSTER_RESPAWN_CENTER_5 - MONSTER_RESPAWN_RANGE_5)
		{
			return FALSE;
		}

		break;
	case eMonsterActivityArea::MonsterArea6:

		if (moveTileX > X_MONSTER_RESPAWN_CENTER_6 + MONSTER_RESPAWN_RANGE_6 || moveTileX < X_MONSTER_RESPAWN_CENTER_6 - MONSTER_RESPAWN_RANGE_6
			|| moveTileY > Y_MONSTER_RESPAWN_CENTER_6 + MONSTER_RESPAWN_RANGE_6 || moveTileY < Y_MONSTER_RESPAWN_CENTER_6 - MONSTER_RESPAWN_RANGE_6)
		{
			return FALSE;
		}

		break;
	case eMonsterActivityArea::MonsterArea7:

		if (moveTileX > X_MONSTER_RESPAWN_CENTER_7 + MONSTER_RESPAWN_RANGE_7 || moveTileX < X_MONSTER_RESPAWN_CENTER_7 - MONSTER_RESPAWN_RANGE_7
			|| moveTileY > Y_MONSTER_RESPAWN_CENTER_7 + MONSTER_RESPAWN_RANGE_7 || moveTileY < Y_MONSTER_RESPAWN_CENTER_7 - MONSTER_RESPAWN_RANGE_7)
		{
			return FALSE;
		}

		break;
	default:

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[chaeckMonsterMoveRange] mActivityArea Value Error : %d", mActivityArea);

		CCrashDump::Crash();

		break;
	}


	return TRUE;
}


INT CMonster::getGameRotation(INT rotation)
{
	INT mathRotation = 90 - rotation;

	if (mathRotation < 0)
	{
		mathRotation += 360;
	}

	return mathRotation;
}

void CMonster::getMonsterMoveCoordinate(INT rotation, FLOAT* moveX, FLOAT* moveY)
{
	FLOAT radian = (FLOAT)DEGREE_TO_RADIAN(rotation);

	// INT�� Ÿ���� Ÿ���� �ƴ� ���� �������� �̵��Ÿ��� ����Ѵ�.
	FLOAT blockMoveRange = (FLOAT)MONSTER_TILE_MOVE_RANGE / 2.0f;

	// ������ ���� ���̸� ����.
	FLOAT vectorLength = sqrtf((blockMoveRange * blockMoveRange) * 2);

	// �̵� ���� �� X ��ǥ�� ��´�.
	*moveX = mPosX + cosf(radian) * vectorLength;

	// �̵� ���� �� Y ��ǥ�� ��´�.
	*moveY = mPosY + sinf(radian) * vectorLength;

	return;
}

void CMonster::getMonsterToAttackedCoordinate(CPlayer* pPlayer, INT rotation, FLOAT* pMoveX, FLOAT* pMoveY)
{
	FLOAT radian = (FLOAT)DEGREE_TO_RADIAN(rotation);

	// INT�� Ÿ���� Ÿ���� �ƴ� ���� �������� �̵��Ÿ��� ����Ѵ�.
	FLOAT blockMoveRange = (FLOAT)MONSTER_TILE_MOVE_RANGE / 2.0f;

	// ������ ���� ���̸� ����.
	FLOAT vectorLength = sqrtf((blockMoveRange * blockMoveRange) * 2);

	FLOAT distanceX = cosf(radian) * vectorLength;
	
	FLOAT monsterX = mPosX + distanceX;
	if (distanceX <= 0.0f)
	{
		// �Ѿ�� �������� �ʴ´�.
		if (pPlayer->mPosX > monsterX)
		{
			*pMoveX = mPosX;
		}
		else
		{
			*pMoveX = monsterX;
		}
	}
	else
	{
		if (pPlayer->mPosX < monsterX)
		{
			*pMoveX = mPosX;
		}
		else
		{
			*pMoveX = monsterX;
		}
	}

	FLOAT distanceY = sinf(radian) * vectorLength;

	// �̵� ���� �� Y ��ǥ�� ��´�.
	FLOAT monsterY = mPosY + distanceY;
	if (distanceY <= 0.0f)
	{
		if (pPlayer->mPosY > monsterY)
		{
			*pMoveY = mPosY;
		}
		else
		{
			*pMoveY = monsterY;
		}
	}
	else
	{
		if (pPlayer->mPosY < monsterY)
		{
			*pMoveY = mPosY;
		}
		else
		{
			*pMoveY = monsterY;
		}
	}

	return;
}




BOOL CMonster::updateMonsterSector(void)
{
	CGodDamnBug::stSector sector;

	CGodDamnBug::GetSector(mTileX, mTileY, &sector);

	if (mCurSector.posX == sector.posX && mCurSector.posY == sector.posY)
	{
		return FALSE;
	}

	mOldSector = mCurSector;

	mpGodDamnBug->EraseMonsterSectorMap(mOldSector.posX, mOldSector.posY, mClientID);

	mCurSector = sector;

	mpGodDamnBug->InsertMonsterSectorMap(sector.posX, sector.posY, mClientID, this);

	return TRUE;
}

void CMonster::sendAroundCreateMonster(void)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingCreateMonster(mClientID, mPosX, mPosY, mRotation, TRUE, pMessage);

	mpGodDamnBug->SendPacketAroundSector(mCurSector.posX, mCurSector.posY, nullptr, pMessage);

	pMessage->Free();

	return;
}

void CMonster::sendAroundUpdateMonsterSector(void)
{
	CGodDamnBug::stSectorAround removeSectorAround;
	CGodDamnBug::stSectorAround addSectorAround;

	CGodDamnBug::GetUpdateSectorAround(mOldSector, mCurSector, &removeSectorAround, &addSectorAround);

	sendRemoveAroundDeleteObject(&removeSectorAround);

	sendAddAroundCreateObject(&addSectorAround);

	return;
}


void CMonster::sendRemoveAroundDeleteObject(CGodDamnBug::stSectorAround* pRemoveSectorAround)
{
	CMessage* pMessage = CMessage::Alloc();

	// ������Ʈ ���� ��Ŷ �����
	CGodDamnBug::PackingRemoveObject(mClientID, pMessage);

	for (INT count = 0; count < pRemoveSectorAround->count; ++count)
	{
		// �����Ǵ� ���Ϳ� ���� �Ѹ���
		mpGodDamnBug->SendPacketOneSector(pRemoveSectorAround->around[count].posX, pRemoveSectorAround->around[count].posY, nullptr, pMessage);
	}

	pMessage->Free();

	return;
}

void CMonster::sendAddAroundCreateObject(CGodDamnBug::stSectorAround* pAddSectorAround)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingCreateMonster(mClientID, mPosX, mPosY, mRotation, FALSE, pMessage);

	for (INT count = 0; count < pAddSectorAround->count; ++count)
	{
		mpGodDamnBug->SendPacketOneSector(pAddSectorAround->around[count].posX, pAddSectorAround->around[count].posY, nullptr, pMessage);
	}

	pMessage->Free();

	return;
}

void CMonster::sendAroundMonsterMoveStart(void)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingMonsterMoveStart(mClientID, mPosX, mPosY, mRotation, pMessage);

	mpGodDamnBug->SendPacketAroundSector(mCurSector.posX, mCurSector.posY, nullptr, pMessage);

	pMessage->Free();

	return;
}

void CMonster::sendAroundMonsterDie(void)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingMonsterDie(mClientID, pMessage);

	mpGodDamnBug->SendPacketAroundSector(mCurSector.posX, mCurSector.posY, nullptr, pMessage);

	pMessage->Free();

	return;
}

void CMonster::sendAroundMonsterAttackAction(void)
{
	CMessage* pMessage = CMessage::Alloc();
	
	CGodDamnBug::PackingMonsterAttackAction(mClientID, pMessage);

	mpGodDamnBug->SendPacketAroundSector(mCurSector.posX, mCurSector.posY, nullptr, pMessage);
	
	pMessage->Free();

	return;
}

void CMonster::sendAroundMonsterAttackDamage(CPlayer* pPlayer)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingAttackDamage(mClientID, pPlayer->mClientID, mDamage, pMessage);

	mpGodDamnBug->SendPacketAroundSector(pPlayer->mCurSector.posX, pPlayer->mCurSector.posY, nullptr, pMessage);

	pMessage->Free();

	return;
}


BOOL CMonster::checkAttackedPlayerSector(CPlayer* pPlayer)
{
	if (abs(pPlayer->mCurSector.posX - mCurSector.posX) < 2 && abs(pPlayer->mCurSector.posY - mCurSector.posY) < 2)
	{
		return TRUE;
	}

	return FALSE;
}


void CMonster::getRotationForAttackedPlayer(CPlayer* pPlayer, INT* pRotation)
{
	FLOAT distanceX = pPlayer->mPosX - mPosX;
	FLOAT distanceY = pPlayer->mPosY - mPosY;

	FLOAT radian = atan2(distanceY, distanceX);

	FLOAT rotation = RADIAN_TO_DEGREE(radian);
	if (rotation < 0)
	{
		rotation = 360 + rotation;
	}

	*pRotation = (INT)rotation;

	return;
}