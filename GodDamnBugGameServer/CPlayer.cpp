#include "stdafx.h"


CPlayer::CPlayer(void)
	: mAccountNo(0)
	, mClientID(0)
	, mCharacterType(0)
	, mPosX(-1)
	, mPosY(-1)
	, mTileX(-1)
	, mTileY(-1)
	, mRotation(0)
	, mCristalCount(0)
	, mHP(0)
	, mEXP(0)
	, mLevel(0)
	, mbDieFlag(FALSE)
	, mDamage(0)
	, mRecoveryHP(0)
	, mbSitFlag(FALSE)
	, mbLoginFlag(FALSE)
	, mVKey(0)
	, mHKey(0)
	, mCurSector{ 0, }
	, mOldSector{ 0, }
	, mOldHP(0)
	, mRecoveryHPStartTick(0)
	, mRecoveryHPLastTick(0)
	, mDBWriteCount(0)
	, mPlayerLock{ 0, }
	, mpGodDamnBug(nullptr)
	, mPlayerID{ 0, }
	, mPlayerNick{ 0, }
{
	InitializeCriticalSection(&mPlayerLock);
}

CPlayer::~CPlayer(void)
{
	DeleteCriticalSection(&mPlayerLock);
}


void CPlayer::OnAuthClientJoin(void)
{
	setAuthClientJoinState();

	return;
}

void CPlayer::OnAuthClientLeave(void)
{
	// mAccountNo ��ü�� ���õ��� �ʰ� �������� �ٷ� ���� �� �ֵ��� mbReleaseFlag�� TRUE �� ����
	if (mAccountNo == ULLONG_MAX)
	{
		SetReleaseFlag(TRUE);
	}
	else if (CheckLogoutInAuth() == TRUE)
	{
		// mbLoginFlag�� FALSE �� ��� AuthThread ���� �α��� ��Ŷ�� �������ʾҰų� setDuplicateLogin���� False�� ����Ǿ��� ����̴�.
		if (mbLoginFlag == TRUE)
		{
			mpGodDamnBug->InsertLogoutDBWriteJob(dfLOGIN_STATUS_NONE, this);
		}

		mpGodDamnBug->InsertAuthReleasePlayerMap(mAccountNo, this);
	}

	return;
}

void CPlayer::OnAuthMessage(CMessage* pMessage)
{
	WORD messageType;

	*pMessage >> messageType;

	if (authRecvProcedure(messageType, pMessage) == FALSE)
	{
		Disconnect();
	}

	return;
}

void CPlayer::OnGameClientJoin(void)
{
	// ���������� ���� ���Ϳ��� �����ϰ� ������ ���� �ֺ� Ŭ���̾�Ʈ���� �˷��ִ� ���ŷο� �۾��� �ؾ��ϴµ�, �׷��� ���� �ʰ�
	// ���Ϳ� ���ܵ����μ� ���������� ���� ������ ó���� �������� �ʴ´�.
	auto iter = mpGodDamnBug->mPlayerSectorMap[mCurSector.posY][mCurSector.posX].find(mClientID);
	if (iter != mpGodDamnBug->mPlayerSectorMap[mCurSector.posY][mCurSector.posX].end())
	{
		CPlayer* pPlayer = iter->second;
	
		// �÷��̾ ���� ���¶���� ������ϰ� �Ѵ�.
		if (mbDieFlag == TRUE)
		{
			playerRestart(TRUE);
		}
		else
		{
			// pPlayer �� OnGameClientLeave ���� ���� ȣ�� �� �� �ֱ� ������ mbSitFlag�� �˻��Ѵ�.
			if (pPlayer->mbSitFlag == TRUE)
			{
				// �������־���� 
				pPlayer->mbSitFlag = FALSE;

				// �ɾ��ִ� ���� ü��ȸ���� �Ͽ��� ������ HP ȸ������ ����ȭ�� �����.
				mHP = pPlayer->mHP;

				// DB �� �����ϱ� ���� �÷��̾��� mOldHP ���� ����
				mOldHP = pPlayer->mOldHP;

				// DB �� �����ϱ� ���� �÷��̾��� mRecoveryHPStartTick ���� ����
				mRecoveryHPStartTick = pPlayer->mRecoveryHPStartTick;

				// �ɾ��ִ� ���¿��� �ٽ� ���ִ� ���·� ����� ���� �ֺ��� �ѷ��ش�.
				pPlayer->sendAroundCharacterMoveStop();

				// ���� �÷��̾� ����
				mpGodDamnBug->EraseSeatedPlayerMap(mClientID);

				// �ɾ��־��ٸ� ���� �α׾ƿ��� �ɾƼ� ���� �������� ������ HPRecovory �����͸� �����Ѵ�.
				mpGodDamnBug->InsertHPRecoveryDBWriteJob(this);
			}
			
			mpGodDamnBug->ErasePlayerSectorMap(mCurSector.posX, mCurSector.posY, mClientID);

			mpGodDamnBug->InsertPlayerSectorMap(mCurSector.posX, mCurSector.posY, mClientID, this);

			// �� ĳ���� ������û ������
			sendCreateMyCharacter();

			// �ֺ� ���Ϳ� �ٸ� ������Ʈ
			sendCreateAroundOthrerObject();
		}

		// ���Ϳ� �����ߴ� �÷��̾ ���������� ���� �� �ֵ��� ���ش�.
		pPlayer->SetReleaseFlag(TRUE);
	}
	else
	{
		// �׾��ִ� ���¿��ٸ� Player �츮��
		if (mbDieFlag == TRUE)
		{
			playerRestart(FALSE);
		}
		else
		{
			// �ߺ� �α����� �ƴ϶�� 
			mpGodDamnBug->InsertPlayerSectorMap(mCurSector.posX, mCurSector.posY, mClientID, this);

			// �� ĳ���� ������û ������
			sendCreateMyCharacter();

			// �ֺ� ���Ϳ� �ִ� ĳ���� ������ ������û ������
			sendCreateAroundOthrerObject();

			// �ֺ� ���Ϳ� �ִ� �ٸ� �����鿡�� �� ĳ���� ���� ��û ������
			sendAroundRespawnCharacter();
		}
	}
	
	return;
}

void CPlayer::OnGameClientLeave(void)
{
	BOOL bLogoutFlag = FALSE;

	{
		CCriticalSection criticalSection(&mPlayerLock);

		// mbLoginFlag�� FALSE�� ��� �ߺ��α������� ���� roginRequest �ʿ��� �̹� Logout�� ���� DB ó���� �Ͽ���
		// OnGameClientJoin ���� ���Ϳ� �ִ� Player �� player ���� �����ϱ� ������ ���͸ʿ��� ���ŵ��� �ʵ��� �Ѵ�.
		if (mbLoginFlag == TRUE)
		{
			mbLoginFlag = FALSE;

			bLogoutFlag = TRUE;
		}
	}

	if (bLogoutFlag == TRUE)
	{	
		if (mbSitFlag == TRUE)
		{
			mbSitFlag = FALSE;

			// �ɾ��ִ� ���¿��� �ٽ� ���ִ� ���·� ����� ���� �ֺ��� �ѷ��ش�.
			// sendAroundCharacterMoveStop();

			// ���� �÷��̾� ����
			mpGodDamnBug->EraseSeatedPlayerMap(mClientID);

			// �ɾ��־��ٸ� ���� �α׾ƿ��� �ɾƼ� ���� �������� ������ HPRecovory �����͸� �����Ѵ�.
			mpGodDamnBug->InsertHPRecoveryDBWriteJob(this);
		}

		// ���� �ʿ��� �����Ѵ�.
		mpGodDamnBug->ErasePlayerSectorMap(mCurSector.posX, mCurSector.posY, mClientID);

		// ���� �ʿ��� ������ ��ǥ�� mClientID ����
		sendAroundRemoveObject(mCurSector.posX, mCurSector.posY, mClientID);

		mpGodDamnBug->InsertLogoutDBWriteJob(dfLOGIN_STATUS_NONE, this);

		// mDBCount �� 0�� �� ������ �� �ֵ��� releasePlayerMap�� Insert �Ѵ�.
		mpGodDamnBug->InsertReleasePlayerMap(mAccountNo, this);
	}

	return;
}

void CPlayer::OnGameMessage(CMessage* pMessage)
{
	WORD messageType;

	*pMessage >> messageType;

	if (gameRecvProcedure(messageType, pMessage) == FALSE)
	{
		Disconnect();
	}

	return;
}


void CPlayer::SetGodDamnBugPtr(CGodDamnBug* pGodDamnBug)
{
	mpGodDamnBug = pGodDamnBug;

	return;
}

void CPlayer::SetPlayerDefaultValue(INT damage, INT HP, INT recoveryHP)
{
	mInitializeDamage = damage;

	mInitializeHP = HP;

	mInitializeRecoveryHP = recoveryHP;	

	return;
}


BOOL CPlayer::setDuplicateLoginPlayer(BYTE* pStatus)
{	
	BOOL bLoginFlag;

	EnterCriticalSection(&mpGodDamnBug->mPlayerMapLock);

	// ���� �������� �÷��̾ �ִ��� Ȯ���Ѵ�.
	CPlayer* pPlayer = mpGodDamnBug->FindPlayerFromPlayerMap(mAccountNo);
	if (pPlayer == nullptr)
	{
		LeaveCriticalSection(&mpGodDamnBug->mPlayerMapLock);

		return FALSE;
	}
		
	// mPlayerMap �� ���� �������� �ʴ� ��찡 �ֱ� ������ pPlayer���� �� ���¿��� ���� �������־�� �Ѵ�.
	EnterCriticalSection(&pPlayer->mPlayerLock);

	// ������ �������ִ� Disconnect();
	pPlayer->Disconnect();
	
	bLoginFlag = pPlayer->mbLoginFlag;

	// ���� �÷��̾��� LoginFlag�� FALSE �� �����Ͽ� 
	pPlayer->mbLoginFlag = FALSE;

	// �α����߾��� ���� logout DB ����� 
	if (bLoginFlag == TRUE)
	{
		// ���� �α����ߴ� �÷��̾� �α׾ƿ� DB ó��
		mpGodDamnBug->InsertLogoutDBWriteJob(dfLOGIN_STATUS_NONE, pPlayer);
	}

	// LoginFlag�� FALSE �� ��� Player �� ���� �������� �ʴ´�. 
	LeaveCriticalSection(&pPlayer->mPlayerLock);

	if (pPlayer->mCharacterType == 0)
	{
		mCharacterType = 0;

		mDBWriteCount = pPlayer->mDBWriteCount;

		*pStatus = dfGAME_LOGIN_NOCHARACTER;
	}
	else
	{
		SetPlayerInfo(pPlayer->mClientID, pPlayer->mCharacterType, pPlayer->mPosX, pPlayer->mPosY, pPlayer->mTileX, pPlayer->mTileY, pPlayer->mRotation, pPlayer->mCristalCount, pPlayer->mHP, pPlayer->mEXP, pPlayer->mLevel, pPlayer->mbDieFlag, pPlayer->mDamage, pPlayer->mRecoveryHP, pPlayer->mDBWriteCount);

		// LoginFlag �� FALSE ���ٸ� �̹� ���Ϳ� �������� �ʰ� releasePlayerMap�� Insert �Ǿ����� �ʱ� ������ SetReleaseFlag �� ���� mbReleaseFlag �� TRUE�� ����
		if (bLoginFlag == FALSE)
		{
			pPlayer->SetReleaseFlag(TRUE);
		}

		*pStatus = dfGAME_LOGIN_OK;

		SetAuthToGameFlag(TRUE);
	}

	// ������ �� �ֵ��� 0���� �������� ������ DBWriteCount�� �ߺ� �α����� �� Player �� ���� �Ϸ�
	pPlayer->mDBWriteCount = 0;

	// �ٸ� �����忡�� Map�� ���� Player �� �������� ���ϵ��� �Ѵ�.
	mpGodDamnBug->ErasePlayerMap(mAccountNo);

	// this data ������ Insert
	mpGodDamnBug->InsertPlayerMap(mAccountNo, this);

	LeaveCriticalSection(&mpGodDamnBug->mPlayerMapLock);

	wcscpy_s(mPlayerID, pPlayer->mPlayerID);

	wcscpy_s(mPlayerNick, pPlayer->mPlayerNick);

	return TRUE;
}


BOOL CPlayer::getPlayerInfoFromGameDB(void)
{
	CTLSDBConnector* pDBConnector = mpGodDamnBug->GetDBConnector();

REQUERY:
	
	if (pDBConnector->Query((WCHAR*)L"SELECT * FROM `character` where accountno = %lld;", mAccountNo) == FALSE)
	{
		if (pDBConnector->CheckReconnectErrorCode() == TRUE)
		{
			pDBConnector->Reconnect();

			goto REQUERY;
		}

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"CGodDamnBug", L"[SetDBPlayerInfo] MySQL Error Code : %d, Error Message : %s", pDBConnector->GetLastError(), pDBConnector->GetLastErrorMessage());

		CCrashDump::Crash();
	}

	pDBConnector->StoreResult();

	MYSQL_ROW mysqlRow = pDBConnector->FetchRow();
	if (mysqlRow == nullptr)
	{
		return FALSE;
	}


	SetPlayerInfo(mpGodDamnBug->GetObjectID(), atoi(mysqlRow[1]), (FLOAT)atof(mysqlRow[2]), (FLOAT)atof(mysqlRow[3]), atoi(mysqlRow[4]), atoi(mysqlRow[5]), atoi(mysqlRow[6]), atoi(mysqlRow[7]), atoi(mysqlRow[8]), atoll(mysqlRow[9]), atoi(mysqlRow[10]), atoi(mysqlRow[11]), mInitializeDamage, mInitializeRecoveryHP, 0);

	pDBConnector->FreeResult();

	return TRUE;
}




BOOL CPlayer::authRecvProcedure(WORD messageType, CMessage* pMessage)
{
	switch (messageType)
	{
	case en_PACKET_CS_GAME_REQ_LOGIN:

		return recvLoginRequest(pMessage);

	case en_PACKET_CS_GAME_REQ_CHARACTER_SELECT:

		return recvCharacterSelectRequest(pMessage);

	case en_PACKET_CS_GAME_REQ_HEARTBEAT:

		break;

	default:

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"GodDamnBug", L"[authRecvProcedure] Message Type  : %d", messageType);

		return FALSE;
	}

	return TRUE;
}



BOOL CPlayer::gameRecvProcedure(WORD messageType, CMessage* pMessage)
{
	switch (messageType)
	{
	case en_PACKET_CS_GAME_REQ_MOVE_CHARACTER:

		return recvMoveCharacter(pMessage);

	case en_PACKET_CS_GAME_REQ_STOP_CHARACTER:

		return recvMoveStopCharacter(pMessage);

	case en_PACKET_CS_GAME_REQ_ATTACK1:

		return recvCharacterAttack1(pMessage);

	case en_PACKET_CS_GAME_REQ_ATTACK2:

		return recvCharacterAttack2(pMessage);

	case en_PACKET_CS_GAME_REQ_PICK:

		return recvCristalPick(pMessage);

	case en_PACKET_CS_GAME_REQ_SIT:

		return recvCharacterSit(pMessage);

	case en_PACKET_CS_GAME_REQ_PLAYER_RESTART:

		return recvPlayerRestart(pMessage);

	case en_PACKET_CS_GAME_REQ_HEARTBEAT:

		break;

	default:

		CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"GodDamnBug", L"[gameRecvProcedure] Message Type  : %d", messageType);


		return FALSE;
	}

	return TRUE;
}


BOOL CPlayer::recvLoginRequest(CMessage* pMessage)
{
	BYTE status;

	*pMessage >> mAccountNo;

	do
	{
		if (mpGodDamnBug->CompareSessionToken(mAccountNo, pMessage->GetPayloadPtr(), TOKEN_SIZE) == FALSE)
		{
			status = dfGAME_LOGIN_FAIL;

			break;
		}
		
		mbLoginFlag = TRUE;

		// �ߺ� �α����� Ȯ���Ѵ�. FALSE �� ��� ���� �������� �ƴϴ� ���� ������ �����Ѵ�.
		if (setDuplicateLoginPlayer(&status) == TRUE)
		{		
			break;
		}

		// Account ID �� Nick ��ȸ
		if (mpGodDamnBug->GetIDWithNickFromAccountDB(mAccountNo, mPlayerID, player::PLAYER_STRING, mPlayerNick, player::PLAYER_STRING) == FALSE)
		{
			status = dfGAME_LOGIN_FAIL;

			break;
		}

		// DB Ȯ�� �� ĳ���Ͱ� ���� ��� �÷��̾� ������ �ϰ� ���� ��� ĳ���� ���� status�� ��û�Ѵ�.
		if (getPlayerInfoFromGameDB() == TRUE)
		{
			status = dfGAME_LOGIN_OK;

			// AuthToGameFlag�� �����Ѵ�.
			SetAuthToGameFlag(TRUE);
		}
		else
		{
			status = dfGAME_LOGIN_NOCHARACTER;

			// ���� ĳ���� ������ ���� �ʾ����� ������ 0���� ������ش�.
			mCharacterType = 0;
		}

		{
			CCriticalSection criticalSection(&mpGodDamnBug->mPlayerMapLock);

			// PlayerMap �� Insert �Ѵ�.
			mpGodDamnBug->InsertPlayerMap(mAccountNo, this);
		}

	} while (0);
	
	sendLoginResponse(status);

	mpGodDamnBug->InsertLoginDBWriteJob(status, this);

	mpGodDamnBug->GetLanLoginClient()->NotificationClientLoginSuccess(mAccountNo);

	return TRUE;
}


BOOL CPlayer::recvCharacterSelectRequest(CMessage* pMessage)
{
	BYTE characterType;

	*pMessage >> characterType;
	
	if (validateRecvCharacterSelectRequest(characterType) == FALSE)
	{
		return FALSE;
	}

	// �÷��̾� �⺻ ������ ��ǥ ����
	SetPlayerRespawnCoordinate();

	{
		// DBWriteThread���� DBCount�� �����ϱ� ������ ����ȭ�� �ʿ�
		CCriticalSection criticalSection(&mPlayerLock);
	
		// �÷��̾� ������ �����մϴ�.
		SetPlayerInfo(mpGodDamnBug->GetObjectID(), characterType, mPosX, mPosY, mTileX, mTileY, rand() % 360, 0, mInitializeHP, 0, 1, 0, mInitializeDamage, mInitializeRecoveryHP, mDBWriteCount);
	}

	// ĳ���� ������ �����Ͽ����� �۽�
	sendCharacterSelectResponse(TRUE);

	// ĳ���� �����Ͽ����� DB�� ����
	mpGodDamnBug->InsertCreateCharacterDBWriteJob(this);

	// AuthToGameFlag�� �����Ѵ�.
	SetAuthToGameFlag(TRUE);

	return TRUE;
}


BOOL CPlayer::recvMoveCharacter(CMessage* pMessage)
{
	if (mbDieFlag == TRUE)
	{
		return TRUE;
	}

	UINT64 clientID;

	FLOAT posX;
	FLOAT posY;

	WORD rotation;

	BYTE vKey;
	BYTE hKey;

	*pMessage >> clientID >> posX >> posY >> rotation >> vKey >> hKey;

	{
		CCriticalSection criticalSection(&mPlayerLock);

		// mbLoginFlag �� FALSE�� ���¿��� ��ǥ�� ���� �޽����� ó���ع����� ��ǥ�� �ٲ� �� �ֱ� ������
		// ���Ϳ� ���� ����ȭ�� Ʋ������. 
		if (validateRecvMoveCharacter(clientID, posX, posY, rotation, vKey, hKey) == FALSE)
		{
			return FALSE;
		}

		mPosX = posX;
		mPosY = posY;

		mTileX = (INT)X_TILE(posX);
		mTileY = (INT)Y_TILE(posY);

		mRotation = rotation;
	}

	mVKey = vKey;
	mHKey = hKey;
	
	sendAroundCharacterMoveStart();

	if (updatePlayerSector() == TRUE)
	{
		sendAroundUpdateCharacterSector();
	}
	
	return TRUE;
}

BOOL CPlayer::recvMoveStopCharacter(CMessage* pMessage)
{
	// ���Ϳ� ���ؼ� ������ �ޱ� ������ �̹� �������� �޽����� �ʰ� ó���Ǿ� mbDieFlag�� TRUE�� �� �� ����
	if (mbDieFlag == TRUE)
	{
		return TRUE;
	}

	UINT64 clientID;

	FLOAT posX;
	FLOAT posY;

	WORD rotation;


	*pMessage >> clientID >> posX >> posY >> rotation;

	{
		CCriticalSection criticalSection(&mPlayerLock);

		if (validateRecvMoveStopCharacter(clientID, posX, posY, rotation) == FALSE)
		{
			return FALSE;
		}

		mPosX = posX;
		mPosY = posY;

		mTileX = (INT)X_TILE(posX);
		mTileY = (INT)Y_TILE(posY);

		mRotation = rotation;
	}
		
	mVKey = 0;
	mHKey = 0;
	
	if (mbSitFlag == TRUE)
	{
		mbSitFlag = FALSE;

		mpGodDamnBug->EraseSeatedPlayerMap(clientID);

		sendPlayerHP();

		mpGodDamnBug->InsertHPRecoveryDBWriteJob(this);
	}

	if (updatePlayerSector() == TRUE)
	{
		sendAroundUpdateCharacterSector();
	}

	sendAroundCharacterMoveStop();

	return TRUE;
}



BOOL CPlayer::recvCharacterAttack1(CMessage* pMessage)
{
	if (mbDieFlag == TRUE)
	{
		return TRUE;
	}

	UINT64 clientID;

	*pMessage >> clientID;

	// ������ �ߺ� �α������� ���� ����ȭ�� ���� �ʿ䰡 ����
	if (validateRecvCharacterAttack1(clientID) == FALSE)
	{
		return FALSE;
	}

	// ���� ��� �Ѹ���
	sendAroundCharacterAttack1();

	CMonster* pVictimMonster;

	if (getAttack1VictimMonster(&pVictimMonster) == FALSE)
	{
		return TRUE;
	}

	// ������ �Ѹ���
	sendAroundAttack1Damage(pVictimMonster, mDamage);

	pVictimMonster->UpdateAttackDamage(mAccountNo, mDamage);

	return TRUE;
}

BOOL CPlayer::recvCharacterAttack2(CMessage* pMessage)
{
	if (mbDieFlag == TRUE)
	{
		return TRUE;
	}

	UINT64 clientID;

	*pMessage >> clientID;

	if (validateRecvCharacterAttack2(clientID) == FALSE)
	{
		return FALSE;
	}

	// ���� ��� �Ѹ���
	sendAroundCharacterAttack2();

	CMonster* pVictimMonster;

	if (getAttack2VictimMonster(&pVictimMonster) == FALSE)
	{
		return TRUE;
	}	

	// ������ �Ѹ���
	sendAroundAttack2Damage(pVictimMonster, mDamage);

	pVictimMonster->UpdateAttackDamage(mAccountNo, mDamage);

	return TRUE;
}


//
//BOOL CPlayer::recvCharacterAttack2(CMessage* pMessage)
//{
//	if (mbDieFlag == TRUE)
//	{
//		return TRUE;
//	}
//
//	UINT64 clientID;
//
//	*pMessage >> clientID;
//
//	if (validateRecvCharacterAttack2(clientID) == FALSE)
//	{
//		return FALSE;
//	}
//
//	// ���� ��� �Ѹ���
//	sendAroundCharacterAttack2();
//
//	DWORD findVictimMonsterCount;
//
//	CMonster* pVictimMonsterArray[ATTACK2_VICTIM_COUNT];
//
//	CGodDamnBug::stSectorAround16 pSectorAround16;
//
//	if (getAttack2VictimMonsters(pVictimMonsterArray, _countof(pVictimMonsterArray), &findVictimMonsterCount, &pSectorAround16) == FALSE)
//	{
//		return TRUE;
//	}
//
//	// ������ �̸�� �Ѹ���
//	//sendAroundAttack2Damage(pVictimMonsterArray, findVictimMonsterCount, mDamage / 2, &pSectorAround16);
//
//	for (DWORD count = 0; count < findVictimMonsterCount; ++count)
//	{
//		pVictimMonsterArray[count]->UpdateAttackDamage(mAccountNo, mDamage / 2);
//	}
//
//	return TRUE;
//}



BOOL CPlayer::recvCristalPick(CMessage* pMessage)
{
	if (mbDieFlag == TRUE)
	{
		return TRUE;
	}

	UINT64 clientID;

	CCristal* pCristal;

	*pMessage >> clientID;

	{
		// mbLoginFlag Ȯ�� �� CristalCount ������ ���� PlayerLock ����ؼ� ����ȭ�� �����.
		CCriticalSection criticalSection(&mPlayerLock);

		if (validateRecvCristalPick(clientID) == FALSE)
		{
			return FALSE;
		}

		sendAroundCristalPickAction();

		if (pickCristal(&pCristal) == FALSE)
		{
			return TRUE;
		}
			
		mCristalCount += pCristal->mAmount;
	}


	sendAroundCristalPick(pCristal->mClientID);

	mpGodDamnBug->RemoveCristal(pCristal);

	mpGodDamnBug->InsertCristalPickDBWriteJob(pCristal->mAmount, this);

	return TRUE;
}

BOOL CPlayer::recvCharacterSit(CMessage* pMessage)
{
	if (mbDieFlag == TRUE)
	{
		return TRUE;
	}

	UINT64 clientID;

	*pMessage >> clientID;

	if (validateRecvCharacterSit(clientID) == FALSE)
	{
		return FALSE;
	}

	mOldHP = mHP;

	// �ɱ� �÷��׷� ����
	mbSitFlag = TRUE;

	mRecoveryHPStartTick = timeGetTime();
	
	mpGodDamnBug->InsertSeatedPlayerMap(clientID, this);

	sendAroundCharacterSit();

	return TRUE;
}

BOOL CPlayer::recvPlayerRestart(CMessage* pMessage)
{
	//if (mbDieFlag == FALSE)
	//{
	//	return TRUE;
	//}

	{
		CCriticalSection criticalSection(&mPlayerLock);

		if (validateRecvPlayerRestart() == FALSE)
		{
			return FALSE;
		}

		playerRestart(TRUE);
	}	

	return TRUE;
}


BOOL CPlayer::validateRecvCharacterSelectRequest(BYTE characterType)
{
	if (mbLoginFlag == FALSE)
	{
		return FALSE;
	}


	// ������ ĳ���� Ÿ���� ������ ����ٸ� return FALSE
	// Ư�� ��Ŷ�� ������ ���´ٴ� �� �ǹ̰� ���� �׳� ���������.
	if (characterType >= (BYTE)eCharacterType::LastType)
	{
		return FALSE;
	}

	return TRUE;
}



BOOL CPlayer::validateRecvMoveCharacter(UINT64 clientID, FLOAT posX, FLOAT posY, WORD rotation, BYTE vKey, BYTE hKey)
{
	if (mbLoginFlag == FALSE)
	{
		return FALSE;
	}


	if (mClientID != clientID)
	{
		return FALSE;
	}


	if (mbSitFlag == TRUE)
	{
		return FALSE;
	}

	if (posX >= X_RANGE || posX < 0 || posY >= Y_RANGE || posY < 0)
	{
		return FALSE;
	}

	if (rotation > 360)
	{
		return FALSE;
	}

	if (vKey > 2 || hKey > 2)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CPlayer::validateRecvMoveStopCharacter(UINT64 clientID, FLOAT posX, FLOAT posY, WORD rotation)
{
	if (mbLoginFlag == FALSE)
	{
		return FALSE;
	}

	if (mClientID != clientID)
	{
		return FALSE;
	}

	if (posX >= X_RANGE || posX < 0 || posY >= Y_RANGE || posY < 0)
	{
		return FALSE;
	}

	if (rotation > 360)
	{
		return FALSE;
	}
	
	return TRUE;
}


BOOL CPlayer::validateRecvCharacterAttack1(UINT64 clientID)
{
	if (mbLoginFlag == FALSE)
	{
		return FALSE;
	}

	if (mClientID != clientID)
	{
		return FALSE;
	}

	if (mbSitFlag == TRUE)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CPlayer::validateRecvCharacterAttack2(UINT64 clientID)
{
	if (mbLoginFlag == FALSE)
	{
		return FALSE;
	}

	if (mClientID != clientID)
	{
		return FALSE;
	}


	if (mbSitFlag == TRUE)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CPlayer::validateRecvCristalPick(UINT64 clientID)
{
	if (mbLoginFlag == FALSE)
	{
		return FALSE;
	}


	if (mClientID != clientID)
	{
		return FALSE;
	}
	

	if (mbSitFlag == TRUE)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CPlayer::validateRecvCharacterSit(UINT64 clientID)
{

	if (mbLoginFlag == FALSE)
	{
		return FALSE;
	}

	if (mClientID != clientID)
	{
		return FALSE;
	}

	if (mbSitFlag == TRUE)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CPlayer::validateRecvPlayerRestart(void)
{
	if (mbLoginFlag == FALSE)
	{
		return FALSE;
	}

	if (mbSitFlag == TRUE)
	{
		return FALSE;
	}

	if (mbDieFlag == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}



void CPlayer::sendLoginResponse(BYTE status)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingLoginResponse(status, mAccountNo, pMessage);

	SendPacket(pMessage);

	pMessage->Free();

	return;
}

void CPlayer::sendCharacterSelectResponse(BYTE status)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingCharacterSelectResponse(status, pMessage);

	SendPacket(pMessage);

	pMessage->Free();

	return;
}


void CPlayer::sendCreateMyCharacter(void)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingCreateMyCharacter(mClientID, mCharacterType, mPlayerNick, sizeof(WCHAR) * player::PLAYER_STRING, mPosX, mPosY, mRotation, mCristalCount, mHP, mEXP, mLevel, pMessage);

	SendPacket(pMessage);

	pMessage->Free();

	return;
}

void CPlayer::sendAroundRespawnCharacter(void)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingCreateOthreadCharacter(mClientID, mCharacterType, mPlayerNick, player::PLAYER_STRING * sizeof(WCHAR), mPosX, mPosY, mRotation, mLevel, TRUE, mbSitFlag, mbDieFlag, pMessage);

	mpGodDamnBug->SendPacketAroundSector(mCurSector.posX, mCurSector.posY, this, pMessage);

	pMessage->Free();

	return;
}

void CPlayer::sendCreateAroundOthrerObject(void)
{	
	CGodDamnBug::stSectorAround sectorAround;

	CGodDamnBug::GetSectorAround(mCurSector.posX, mCurSector.posY, &sectorAround);

	for (INT count = 0; count < sectorAround.count; ++count)
	{
		for (auto &iter : mpGodDamnBug->mPlayerSectorMap[sectorAround.around[count].posY][sectorAround.around[count].posX])
		{
			if (this != iter.second)
			{
				CPlayer* pOtherPlayer = iter.second;

				CMessage* pMessage = CMessage::Alloc();
	
				CGodDamnBug::PackingCreateOthreadCharacter(pOtherPlayer->mClientID, pOtherPlayer->mCharacterType, pOtherPlayer->mPlayerNick, player::PLAYER_STRING * sizeof(WCHAR), pOtherPlayer->mPosX, pOtherPlayer->mPosY, pOtherPlayer->mRotation, pOtherPlayer->mLevel, FALSE, pOtherPlayer->mbSitFlag, pOtherPlayer->mbDieFlag, pMessage);

				SendPacket(pMessage);

				pMessage->Free();
			}
		}

		for (auto& iter : mpGodDamnBug->mMonsterSectorMap[sectorAround.around[count].posY][sectorAround.around[count].posX])
		{
			CMonster* pMonster = iter.second;

			CMessage* pMessage = CMessage::Alloc();

			CGodDamnBug::PackingCreateMonster(pMonster->mClientID, pMonster->mPosX, pMonster->mPosY, pMonster->mRotation, FALSE, pMessage);

			SendPacket(pMessage);

			pMessage->Free();
		}


		for (auto& iter : mpGodDamnBug->mCristalSectorMap[sectorAround.around[count].posY][sectorAround.around[count].posX])
		{
			CCristal* pCristal = iter.second;

			CMessage* pMessage = CMessage::Alloc();

			CGodDamnBug::PackingCreateCristal(pCristal->mClientID, pCristal->mCristalType, pCristal->mPosX, pCristal->mPosY, pMessage);

			SendPacket(pMessage);

			pMessage->Free();
		}
	}

	return;
}

void CPlayer::sendAroundUpdateCharacterSector(void)
{
	CGodDamnBug::stSectorAround removeSectorAround;
	CGodDamnBug::stSectorAround addSectorAround;

	CGodDamnBug::GetUpdateSectorAround(mOldSector, mCurSector, &removeSectorAround, &addSectorAround);

	// �����Ǵ� ���Ϳ� clientID ������Ʈ ���� ��û ������
	sendRemoveSectorDeleteMyCharacter(&removeSectorAround);
	
	// �����Ǵ� ���Ϳ� �ִ� ������Ʈ ������û ������
	sendRemoveSectorDeleteObject(&removeSectorAround);

	// �߰��Ǵ� ���Ϳ� pPlayer ĳ���� ���� ��û ������
	sendAddSectorCreateMyCharacter(&addSectorAround);

	// �߰��Ǵ� ���Ϳ� �ִ� ������Ʈ pPlayer���� ������û ������
	sendAddSectorCreateObject(&addSectorAround);

	return;
}

void CPlayer::sendRemoveSectorDeleteMyCharacter(CGodDamnBug::stSectorAround* pRemoveSectorAround)
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


void CPlayer::sendRemoveSectorDeleteObject(CGodDamnBug::stSectorAround* pRemoveSectorAround)
{
	for (INT count = 0; count < pRemoveSectorAround->count; ++count)
	{	
		// �����Ǵ� ������ �÷��̾� ������Ʈ ����
		for (auto& iter : mpGodDamnBug->mPlayerSectorMap[pRemoveSectorAround->around[count].posY][pRemoveSectorAround->around[count].posX])
		{
			CMessage* pMessage = CMessage::Alloc();

			CGodDamnBug::PackingRemoveObject(iter.second->mClientID, pMessage);

			SendPacket(pMessage);

			pMessage->Free();
		}


		// �����Ǵ� ������ ���� ������Ʈ ����
		for (auto& iter : mpGodDamnBug->mMonsterSectorMap[pRemoveSectorAround->around[count].posY][pRemoveSectorAround->around[count].posX])
		{
			CMessage* pMessage = CMessage::Alloc();

			CGodDamnBug::PackingRemoveObject(iter.second->mClientID, pMessage);

			SendPacket(pMessage);

			pMessage->Free();
		}


		for (auto& iter : mpGodDamnBug->mCristalSectorMap[pRemoveSectorAround->around[count].posY][pRemoveSectorAround->around[count].posX])
		{
			CMessage* pMessage = CMessage::Alloc();

			CGodDamnBug::PackingRemoveObject(iter.second->mClientID, pMessage);

			SendPacket(pMessage);

			pMessage->Free();
		}
	}

	return;
}





void CPlayer::sendAddSectorCreateMyCharacter(CGodDamnBug::stSectorAround* pAddSectorAround)
{
	CMessage* pMessage = CMessage::Alloc();

	// pPlayer ĳ���� ���� ��Ŷ �����
	CGodDamnBug::PackingCreateOthreadCharacter(mClientID, mCharacterType, mPlayerNick, player::PLAYER_STRING * sizeof(WCHAR), mPosX, mPosY, mRotation, mLevel, FALSE, mbSitFlag, mbDieFlag, pMessage);

	for (INT count = 0; count < pAddSectorAround->count; ++count)
	{
		// �߰��Ǵ� ���Ϳ� �Ѹ���
		mpGodDamnBug->SendPacketOneSector(pAddSectorAround->around[count].posX, pAddSectorAround->around[count].posY, nullptr, pMessage);
	}

	pMessage->Free();

	return;
}

// �ֺ� ���Ϳ� �ش� ������Ʈ ������û ������
void CPlayer::sendAroundRemoveObject(INT sectorPosX, INT sectorPosY, UINT64 clientID)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingRemoveObject(clientID, pMessage);

	mpGodDamnBug->SendPacketAroundSector(sectorPosX, sectorPosY, this, pMessage);

	pMessage->Free();

	return;
}


void CPlayer::sendAddSectorCreateObject(CGodDamnBug::stSectorAround* pAddSectorAround)
{
	for (INT count = 0; count < pAddSectorAround->count; ++count)
	{
		// �߰��Ǵ� ������ �÷��̾� ������û
		for (auto& iter : mpGodDamnBug->mPlayerSectorMap[pAddSectorAround->around[count].posY][pAddSectorAround->around[count].posX])
		{
			CPlayer* pAddPlayer = iter.second;

			CMessage* pMessage = CMessage::Alloc();

			CGodDamnBug::PackingCreateOthreadCharacter(pAddPlayer->mClientID, pAddPlayer->mCharacterType, pAddPlayer->mPlayerNick, player::PLAYER_STRING * sizeof(WCHAR), pAddPlayer->mPosX, pAddPlayer->mPosY, pAddPlayer->mRotation, pAddPlayer->mLevel, FALSE, pAddPlayer->mbSitFlag, pAddPlayer->mbDieFlag, pMessage);

			SendPacket(pMessage);

			pMessage->Free();
		}

		// �߰��Ǵ� ������ ���� ������û
		for (auto& iter : mpGodDamnBug->mMonsterSectorMap[pAddSectorAround->around[count].posY][pAddSectorAround->around[count].posX])
		{
			CMonster* pMonster = iter.second;

			CMessage* pMessage = CMessage::Alloc();

			CGodDamnBug::PackingCreateMonster(pMonster->mClientID, pMonster->mPosX, pMonster->mPosY, pMonster->mRotation, FALSE, pMessage);

			SendPacket(pMessage);

			pMessage->Free();
		}


		for (auto& iter : mpGodDamnBug->mCristalSectorMap[pAddSectorAround->around[count].posY][pAddSectorAround->around[count].posX])
		{
			CCristal* pCristal = iter.second;

			CMessage* pMessage = CMessage::Alloc();

			CGodDamnBug::PackingCreateCristal(pCristal->mClientID, pCristal->mCristalType, pCristal->mPosX, pCristal->mPosY, pMessage);

			SendPacket(pMessage);

			pMessage->Free();
		}
	}

	return;
}


void CPlayer::sendAroundCharacterMoveStart(void)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingCharacterMoveStart(mClientID, mPosX, mPosY, mRotation, mVKey, mHKey, pMessage);

	mpGodDamnBug->SendPacketAroundSector(mCurSector.posX, mCurSector.posY, this, pMessage);

	pMessage->Free();

	return;
}

void CPlayer::sendAroundCharacterMoveStop(void)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingCharacterMoveStop(mClientID, mPosX, mPosY, mRotation, pMessage);

	mpGodDamnBug->SendPacketAroundSector(mCurSector.posX, mCurSector.posY, this, pMessage);

	pMessage->Free();

	return;
}

void CPlayer::sendAroundCharacterAttack1(void)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingPlayerAttack1Action(mClientID, pMessage);

	mpGodDamnBug->SendPacketAroundSector(mCurSector.posX, mCurSector.posY, this, pMessage);

	pMessage->Free();

	return;
}


void CPlayer::sendAroundCharacterAttack2(void)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingPlayerAttack2Action(mClientID, pMessage);

	mpGodDamnBug->SendPacketAroundSector(mCurSector.posX, mCurSector.posY, this, pMessage);

	pMessage->Free();

	return;
}


void CPlayer::sendAroundCristalPickAction(void)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingCristalPickAction(mClientID, pMessage);

	mpGodDamnBug->SendPacketAroundSector(mCurSector.posX, mCurSector.posY, this, pMessage);

	pMessage->Free();

	return;
}


void CPlayer::sendAroundCristalPick(UINT64 cristalClientID)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingCristalPick(mClientID, cristalClientID, mCristalCount, pMessage);

	mpGodDamnBug->SendPacketAroundSector(mCurSector.posX, mCurSector.posY, nullptr, pMessage);

	pMessage->Free();

	return;
}


void CPlayer::sendAroundCharacterSit(void)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingCharacterSit(mClientID, pMessage);

	mpGodDamnBug->SendPacketAroundSector(mCurSector.posX, mCurSector.posY, this, pMessage);

	pMessage->Free();

	return;
}

void CPlayer::sendAroundAttack1Damage(CMonster* pVictimMonster, INT damage)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingAttackDamage(mClientID, pVictimMonster->mClientID, damage, pMessage);

	// pVictimPlayer ���� �����ؼ� ��Ŷ�� �Ѹ���.
	mpGodDamnBug->SendPacketAroundSector(pVictimMonster->mCurSector.posX, pVictimMonster->mCurSector.posY, nullptr, pMessage);

	pMessage->Free();

	return;
}

//void CPlayer::sendAroundAttack2Damage(CMonster** pVicTimMonsterArray, DWORD victimMonsterCount, INT damage, CGodDamnBug::stSectorAround16* pSectorAround16)
//{
//	for (INT sectorCount = 0; sectorCount < pSectorAround16->count; ++sectorCount)
//	{
//		for (DWORD count = 0; count < victimMonsterCount; ++count)
//		{		
//			CMessage* pMessage = CMessage::Alloc();
//
//			CGodDamnBug::PackingAttackDamage(mClientID, pVicTimMonsterArray[count]->mClientID, damage, pMessage);
//
//			mpGodDamnBug->SendPacketOneSector(pSectorAround16->around[sectorCount].posX, pSectorAround16->around[sectorCount].posY, nullptr, pMessage);
//		
//			pMessage->Free();
//		}
//	}
//
//	return;
//}

void CPlayer::sendAroundAttack2Damage(CMonster* pVictimMonster, INT damage)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingAttackDamage(mClientID, pVictimMonster->mClientID, damage, pMessage);

	// pVictimPlayer ���� �����ؼ� ��Ŷ�� �Ѹ���.
	mpGodDamnBug->SendPacketAroundSector(pVictimMonster->mCurSector.posX, pVictimMonster->mCurSector.posY, nullptr, pMessage);

	pMessage->Free();

}




void CPlayer::sendAroundPlayerDie(INT minusCristal)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingPlayerDie(mClientID, minusCristal, pMessage);

	mpGodDamnBug->SendPacketAroundSector(mCurSector.posX, mCurSector.posY, nullptr, pMessage);

	pMessage->Free();

	return;
}

void CPlayer::sendPlayerHP(void)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingPlayerHP(mHP, pMessage);

	SendPacket(pMessage);

	pMessage->Free();

	return;
}

void CPlayer::sendPlayerRestart(void)
{
	CMessage* pMessage = CMessage::Alloc();

	CGodDamnBug::PackingPlayerRestart(pMessage);

	SendPacket(pMessage);

	pMessage->Free();

	return;
}


void CPlayer::SetPlayerInfo(UINT64 clientID, BYTE characterType, FLOAT posX, FLOAT posY, INT tileX, INT tileY, WORD rotation, INT cristalCount, INT HP, INT64 EXP, WORD level, BOOL bDieFlag, INT damage, INT recoveryHP, LONG DBWriteCount)
{		
	mClientID = clientID;

	mCharacterType = characterType;

	mPosX = posX;

	mPosY = posY;

	mTileX = tileX;

	mTileY = tileY;

	mRotation = rotation;

	mCristalCount = cristalCount;

	mHP = HP;

	mEXP = EXP;

	mLevel = level;

	mbDieFlag = bDieFlag;

	mDamage = damage;

	mRecoveryHP = recoveryHP;

	mbSitFlag = FALSE;

	mVKey = 0;

	mHKey = 0;

	mbDieFlag = bDieFlag;

	// �÷��̾� ���� ����
	CGodDamnBug::GetSector(tileX, tileY, &mCurSector);

	//pPlayer->mOldSector = mCurSector;

	mDBWriteCount = DBWriteCount;

	return;
}


void CPlayer::SetPlayerRespawnCoordinate(void)
{
	mTileX = 85 + (rand() % 21);
	mTileY = 93 + (rand() % 31);

	mPosX = X_BLOCK(mTileX);
	mPosY = Y_BLOCK(mTileY);

	return;
}


LONG CPlayer::GetDBWriteCount(void) const
{
	return mDBWriteCount;
}

UINT64 CPlayer::GetAccountNo(void) const
{
	return mAccountNo;
}

UINT64 CPlayer::GetClientID(void) const
{
	return mClientID;
}

void CPlayer::HPRecovery(void)
{
	DWORD nowTime = timeGetTime();

	if (nowTime - mRecoveryHPLastTick >= 1000)
	{
		CCriticalSection criticalSection(&mPlayerLock);

		if (mHP + mRecoveryHP < mInitializeHP)
		{
			mHP += mRecoveryHP;
		}
		else
		{
			mHP = mInitializeHP;
		}

		mRecoveryHPLastTick = nowTime;
	}

	return;
}


void CPlayer::UpdateAttackDamage(INT damage)
{
	if (mbSitFlag == TRUE)
	{
		mbSitFlag = FALSE;

		mpGodDamnBug->EraseSeatedPlayerMap(mClientID);

		sendPlayerHP();

		mpGodDamnBug->InsertHPRecoveryDBWriteJob(this);
	}

	CCriticalSection criticalSection(&mPlayerLock);

	mHP -= damage;

	if (mHP <= 0)
	{
		mHP = 0;

		mbDieFlag = TRUE;

		INT minusCristal;

		if (mCristalCount < 1000)
		{
			minusCristal = mCristalCount;

			mCristalCount = 0;
		}
		else
		{
			minusCristal = 1000;

			mCristalCount -= 1000;
		}

		sendAroundPlayerDie(minusCristal);

		mpGodDamnBug->CreateCristal(mPosX, mPosY, mTileX, mTileY, mCurSector);

		// �÷��̾� ������ DB�� �����Ѵ�.
		mpGodDamnBug->InsertDieDBWriteJob(this);
	}

	return;
}


BOOL CPlayer::updatePlayerSector(void)
{
	CGodDamnBug::stSector sector;

	CGodDamnBug::GetSector(mTileX, mTileY, &sector);

	if (mCurSector.posX == sector.posX && mCurSector.posY == sector.posY)
	{
		return FALSE;
	}

	mOldSector = mCurSector;

	mpGodDamnBug->ErasePlayerSectorMap(mOldSector.posX, mOldSector.posY, mClientID);

	mCurSector = sector;

	mpGodDamnBug->InsertPlayerSectorMap(sector.posX, sector.posY, mClientID, this);

	return TRUE;
}

BOOL CPlayer::getAttack1VictimMonster(CMonster** pVictimMonster)
{
	CGodDamnBug::stCollisionBox collisionBox;
	CGodDamnBug::GetAttackCollisionBox(mRotation, mTileX, mTileY, PLAYER_ATTACK1_RANGE, &collisionBox);

	CGodDamnBug::stSectorAround sectorAround;
	CGodDamnBug::GetSectorAround(mCurSector.posX, mCurSector.posY, &sectorAround);

	for (INT count = 0; count < sectorAround.count; ++count)
	{
		for (auto& iter : mpGodDamnBug->mMonsterSectorMap[sectorAround.around[count].posY][sectorAround.around[count].posX])
		{
			if (iter.second->mTileX >= collisionBox.pointOne.tileX && iter.second->mTileY >= collisionBox.pointOne.tileY
				&& iter.second->mTileX <= collisionBox.pointTwo.tileX && iter.second->mTileY <= collisionBox.pointTwo.tileY)
			{
				*pVictimMonster = iter.second;

				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CPlayer::getAttack2VictimMonster(CMonster** pVictimMonster)
{
	CGodDamnBug::stCollisionBox collisionBox;
	CGodDamnBug::GetAttackCollisionBox(mRotation, mTileX, mTileY, PLAYER_ATTACK2_RANGE, &collisionBox);

	CGodDamnBug::stSectorAround sectorAround;
	CGodDamnBug::GetSectorAround(mCurSector.posX, mCurSector.posY, &sectorAround);

	for (INT count = 0; count < sectorAround.count; ++count)
	{
		for (auto& iter : mpGodDamnBug->mMonsterSectorMap[sectorAround.around[count].posY][sectorAround.around[count].posX])
		{
			if (iter.second->mTileX >= collisionBox.pointOne.tileX && iter.second->mTileY >= collisionBox.pointOne.tileY
				&& iter.second->mTileX <= collisionBox.pointTwo.tileX && iter.second->mTileY <= collisionBox.pointTwo.tileY)
			{
				*pVictimMonster = iter.second;

				return TRUE;
			}
		}
	}

	return FALSE;
}


//BOOL CPlayer::getAttack2VictimMonsters(CMonster** pVictimMonsterArray, DWORD arrayLength, DWORD* pFindVictimMonsterCount, CGodDamnBug::stSectorAround16* pSectorAround16)
//{
//	BOOL bFindFlag = FALSE;
//
//	CGodDamnBug::stCollisionBox collisionBox;
//	CGodDamnBug::GetAttackCollisionBox(mRotation, mTileX, mTileY, PLAYER_ATTACK2_RANGE, &collisionBox);
//
//	CGodDamnBug::GetAttackSectorAround16(mRotation, mCurSector, pSectorAround16);
//
//	INT findPlayerCount = 0;
//
//	for (INT count = 0; count < pSectorAround16->count; ++count)
//	{
//		for (auto& iter : mpGodDamnBug->mMonsterSectorMap[pSectorAround16->around[count].posY][pSectorAround16->around[count].posX])
//		{
//			if (iter.second->mTileX >= collisionBox.pointOne.tileX && iter.second->mTileY >= collisionBox.pointOne.tileY
//				&& iter.second->mTileX <= collisionBox.pointTwo.tileX && iter.second->mTileY <= collisionBox.pointTwo.tileY)
//			{
//				pVictimMonsterArray[findPlayerCount++] = iter.second;
//
//				bFindFlag = TRUE;
//
//				if (findPlayerCount == arrayLength)
//				{
//					count = pSectorAround16->count;
//					break;
//				}
//			}
//		}
//	}
//
//	*pFindVictimMonsterCount = findPlayerCount;
//
//	return bFindFlag;
//}


void CPlayer::playerRestart(BOOL bObjectRemoveFlag)
{
	// Ŭ���̾�Ʈ ������Ʈ�� ������ �� ��
	if (bObjectRemoveFlag == TRUE)
	{
		sendAroundRemoveObject(mCurSector.posX, mCurSector.posY, mClientID);

		mpGodDamnBug->ErasePlayerSectorMap(mCurSector.posX, mCurSector.posY, mClientID);
	}

	{
		CCriticalSection criticalSection(&mPlayerLock);

		// ������� ���� HP �� ��ǥ�� ���� �Ҵ� �޴´�.
		setRestartPlayerInfo();
	}

	// ���� ������ ��ǥ�� �������� ���Ϳ� �ٽ� �־��ش�.
	mpGodDamnBug->InsertPlayerSectorMap(mCurSector.posX, mCurSector.posY, mClientID, this);

	// �÷��̾� ����� �޽��� �۽�
	sendPlayerRestart();

	// �� ĳ���� ���� �޽��� �۽�
	sendCreateMyCharacter();

	// �� ĳ���� �����϶�� �ֺ��� �Ѹ���
	sendAroundRespawnCharacter();

	sendCreateAroundOthrerObject();

	mpGodDamnBug->InsertRestartDBWriteJob(this);

	return;
}

void CPlayer::getCristalPickCollisionBox(CGodDamnBug::stCollisionBox* pCollisionBox)
{
	pCollisionBox->pointOne.tileX = mTileX - CRISTAL_PICK_RANGE / 2;
	pCollisionBox->pointOne.tileY = mTileY - CRISTAL_PICK_RANGE / 2;

	pCollisionBox->pointTwo.tileX = mTileX + CRISTAL_PICK_RANGE / 2;
	pCollisionBox->pointTwo.tileY = mTileY + CRISTAL_PICK_RANGE / 2;

	return;
}


BOOL CPlayer::pickCristal(CCristal** pCristal)
{	
	CGodDamnBug::stCollisionBox collisionBox;
	getCristalPickCollisionBox(&collisionBox);

	CGodDamnBug::stSectorAround sectorAround;
	CGodDamnBug::GetSectorAround(mCurSector.posX, mCurSector.posY, &sectorAround);

	for (INT count = 0; count < sectorAround.count; ++count)
	{
		for (auto& iter : mpGodDamnBug->mCristalSectorMap[sectorAround.around[count].posY][sectorAround.around[count].posX])
		{
			if (iter.second->mTileX >= collisionBox.pointOne.tileX && iter.second->mTileY >= collisionBox.pointOne.tileY
				&& iter.second->mTileX <= collisionBox.pointTwo.tileX && iter.second->mTileY <= collisionBox.pointTwo.tileY)
			{
				*pCristal = iter.second;

				return TRUE;
			}
		}
	}

	return FALSE;
}

void CPlayer::setRestartPlayerInfo(void)
{
	mbDieFlag = FALSE;

	mHP = mInitializeHP;

	SetPlayerRespawnCoordinate();

	CGodDamnBug::GetSector(mTileX, mTileY, &mCurSector);

	return;
}

void CPlayer::setAuthClientJoinState(void)
{
	mAccountNo = ULLONG_MAX;

	mCharacterType = 0;

	mbLoginFlag = FALSE;

	return;
}
