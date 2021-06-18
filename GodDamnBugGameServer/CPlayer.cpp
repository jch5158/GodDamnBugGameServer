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
	// mAccountNo 자체도 셋팅되지 않고 끊겼으면 바로 끊길 수 있도록 mbReleaseFlag를 TRUE 로 변경
	if (mAccountNo == ULLONG_MAX)
	{
		SetReleaseFlag(TRUE);
	}
	else if (CheckLogoutInAuth() == TRUE)
	{
		// mbLoginFlag가 FALSE 일 경우 AuthThread 에서 로그인 패킷을 보내지않았거나 setDuplicateLogin에서 False로 변경되었을 경우이다.
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
	// 재접속으로 인해 섹터에서 제거하고 제거한 것을 주변 클라이언트에게 알려주는 번거로운 작업을 해야하는데, 그렇게 하지 않고
	// 섹터에 남겨둠으로서 재접속으로 인한 별도의 처리를 진행하지 않는다.
	auto iter = mpGodDamnBug->mPlayerSectorMap[mCurSector.posY][mCurSector.posX].find(mClientID);
	if (iter != mpGodDamnBug->mPlayerSectorMap[mCurSector.posY][mCurSector.posX].end())
	{
		CPlayer* pPlayer = iter->second;
	
		// 플레이어가 죽은 상태라면은 재시작하게 한다.
		if (mbDieFlag == TRUE)
		{
			playerRestart(TRUE);
		}
		else
		{
			// pPlayer 의 OnGameClientLeave 보다 먼저 호출 될 수 있기 때문에 mbSitFlag를 검사한다.
			if (pPlayer->mbSitFlag == TRUE)
			{
				// 변경해주어야함 
				pPlayer->mbSitFlag = FALSE;

				// 앉아있는 동안 체력회복을 하였기 때문에 HP 회복으로 동기화를 맞춘다.
				mHP = pPlayer->mHP;

				// DB 에 저장하기 위해 플레이어의 mOldHP 값을 대입
				mOldHP = pPlayer->mOldHP;

				// DB 에 저장하기 위해 플레이어의 mRecoveryHPStartTick 값을 대입
				mRecoveryHPStartTick = pPlayer->mRecoveryHPStartTick;

				// 앉아있는 상태에서 다시 서있는 상태로 만들기 위해 주변에 뿌려준다.
				pPlayer->sendAroundCharacterMoveStop();

				// 쉬는 플레이어 제거
				mpGodDamnBug->EraseSeatedPlayerMap(mClientID);

				// 앉아있었다면 게임 로그아웃시 앉아서 쉬는 컨텐츠가 끝나기 HPRecovory 데이터를 저장한다.
				mpGodDamnBug->InsertHPRecoveryDBWriteJob(this);
			}
			
			mpGodDamnBug->ErasePlayerSectorMap(mCurSector.posX, mCurSector.posY, mClientID);

			mpGodDamnBug->InsertPlayerSectorMap(mCurSector.posX, mCurSector.posY, mClientID, this);

			// 내 캐릭터 생성요청 보내기
			sendCreateMyCharacter();

			// 주변 섹터에 다른 오브젝트
			sendCreateAroundOthrerObject();
		}

		// 섹터에 존재했던 플레이어가 정상적으로 끊길 수 있도록 해준다.
		pPlayer->SetReleaseFlag(TRUE);
	}
	else
	{
		// 죽어있는 상태였다면 Player 살리기
		if (mbDieFlag == TRUE)
		{
			playerRestart(FALSE);
		}
		else
		{
			// 중복 로그인이 아니라면 
			mpGodDamnBug->InsertPlayerSectorMap(mCurSector.posX, mCurSector.posY, mClientID, this);

			// 내 캐릭터 생성요청 보내기
			sendCreateMyCharacter();

			// 주변 섹터에 있는 캐릭터 나에게 생성요청 보내기
			sendCreateAroundOthrerObject();

			// 주변 섹터에 있는 다른 유저들에게 내 캐릭터 생성 요청 보내기
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

		// mbLoginFlag가 FALSE일 경우 중복로그인으로 인해 roginRequest 쪽에서 이미 Logout에 대한 DB 처리를 하였음
		// OnGameClientJoin 에서 섹터에 있는 Player 로 player 값을 셋팅하기 때문에 섹터맵에서 제거되지 않도록 한다.
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

			// 앉아있는 상태에서 다시 서있는 상태로 만들기 위해 주변에 뿌려준다.
			// sendAroundCharacterMoveStop();

			// 쉬는 플레이어 제거
			mpGodDamnBug->EraseSeatedPlayerMap(mClientID);

			// 앉아있었다면 게임 로그아웃시 앉아서 쉬는 컨텐츠가 끝나기 HPRecovory 데이터를 저장한다.
			mpGodDamnBug->InsertHPRecoveryDBWriteJob(this);
		}

		// 섹터 맵에서 제거한다.
		mpGodDamnBug->ErasePlayerSectorMap(mCurSector.posX, mCurSector.posY, mClientID);

		// 섹터 맵에서 제거한 좌표로 mClientID 삭제
		sendAroundRemoveObject(mCurSector.posX, mCurSector.posY, mClientID);

		mpGodDamnBug->InsertLogoutDBWriteJob(dfLOGIN_STATUS_NONE, this);

		// mDBCount 가 0일 때 해제될 수 있도록 releasePlayerMap에 Insert 한다.
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

	// 현재 접속중인 플레이어가 있는지 확인한다.
	CPlayer* pPlayer = mpGodDamnBug->FindPlayerFromPlayerMap(mAccountNo);
	if (pPlayer == nullptr)
	{
		LeaveCriticalSection(&mpGodDamnBug->mPlayerMapLock);

		return FALSE;
	}
		
	// mPlayerMap 을 통해 접근하지 않는 경우가 있기 때문에 pPlayer락을 건 상태에서 값을 변경해주어야 한다.
	EnterCriticalSection(&pPlayer->mPlayerLock);

	// 기존에 접속해있던 Disconnect();
	pPlayer->Disconnect();
	
	bLoginFlag = pPlayer->mbLoginFlag;

	// 현재 플레이어의 LoginFlag를 FALSE 로 변경하여 
	pPlayer->mbLoginFlag = FALSE;

	// 로그인했었을 때만 logout DB 남기기 
	if (bLoginFlag == TRUE)
	{
		// 먼저 로그인했던 플레이어 로그아웃 DB 처리
		mpGodDamnBug->InsertLogoutDBWriteJob(dfLOGIN_STATUS_NONE, pPlayer);
	}

	// LoginFlag가 FALSE 일 경우 Player 의 값은 수정되지 않는다. 
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

		// LoginFlag 가 FALSE 였다면 이미 섹터에 존재하지 않고 releasePlayerMap에 Insert 되어있지 않기 때문에 SetReleaseFlag 를 통해 mbReleaseFlag 를 TRUE로 변경
		if (bLoginFlag == FALSE)
		{
			pPlayer->SetReleaseFlag(TRUE);
		}

		*pStatus = dfGAME_LOGIN_OK;

		SetAuthToGameFlag(TRUE);
	}

	// 해제될 수 있도록 0으로 변경해줌 어차피 DBWriteCount는 중복 로그인을 한 Player 에 복사 완료
	pPlayer->mDBWriteCount = 0;

	// 다른 스레드에서 Map을 통해 Player 에 접근하지 못하도록 한다.
	mpGodDamnBug->ErasePlayerMap(mAccountNo);

	// this data 셋팅후 Insert
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

		// 중복 로그인을 확인한다. FALSE 일 경우 현재 접속중이 아니니 다음 로직을 진행한다.
		if (setDuplicateLoginPlayer(&status) == TRUE)
		{		
			break;
		}

		// Account ID 및 Nick 조회
		if (mpGodDamnBug->GetIDWithNickFromAccountDB(mAccountNo, mPlayerID, player::PLAYER_STRING, mPlayerNick, player::PLAYER_STRING) == FALSE)
		{
			status = dfGAME_LOGIN_FAIL;

			break;
		}

		// DB 확인 후 캐릭터가 있을 경우 플레이어 셋팅을 하고 없을 경우 캐릭터 생성 status로 요청한다.
		if (getPlayerInfoFromGameDB() == TRUE)
		{
			status = dfGAME_LOGIN_OK;

			// AuthToGameFlag를 셋팅한다.
			SetAuthToGameFlag(TRUE);
		}
		else
		{
			status = dfGAME_LOGIN_NOCHARACTER;

			// 아직 캐릭터 선택을 하지 않았음기 때문에 0으로 만들어준다.
			mCharacterType = 0;
		}

		{
			CCriticalSection criticalSection(&mpGodDamnBug->mPlayerMapLock);

			// PlayerMap 에 Insert 한다.
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

	// 플레이어 기본 리스폰 좌표 셋팅
	SetPlayerRespawnCoordinate();

	{
		// DBWriteThread에서 DBCount를 차감하기 때문에 동기화가 필요
		CCriticalSection criticalSection(&mPlayerLock);
	
		// 플레이어 정보를 셋팅합니다.
		SetPlayerInfo(mpGodDamnBug->GetObjectID(), characterType, mPosX, mPosY, mTileX, mTileY, rand() % 360, 0, mInitializeHP, 0, 1, 0, mInitializeDamage, mInitializeRecoveryHP, mDBWriteCount);
	}

	// 캐릭터 설정의 성공하였음을 송신
	sendCharacterSelectResponse(TRUE);

	// 캐릭터 선택하였음을 DB에 저장
	mpGodDamnBug->InsertCreateCharacterDBWriteJob(this);

	// AuthToGameFlag를 셋팅한다.
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

		// mbLoginFlag 가 FALSE인 상태에서 좌표에 대한 메시지를 처리해버리면 좌표가 바뀔 수 있기 때문에
		// 섹터에 대한 동기화가 틀어진다. 
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
	// 몬스터에 의해서 공격을 받기 때문에 이미 보내놓은 메시지가 늦게 처리되어 mbDieFlag가 TRUE가 될 수 있음
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

	// 공격은 중복 로그인으로 인한 동기화를 맞출 필요가 없음
	if (validateRecvCharacterAttack1(clientID) == FALSE)
	{
		return FALSE;
	}

	// 어택 모션 뿌리기
	sendAroundCharacterAttack1();

	CMonster* pVictimMonster;

	if (getAttack1VictimMonster(&pVictimMonster) == FALSE)
	{
		return TRUE;
	}

	// 데미지 뿌리기
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

	// 공격 모션 뿌리기
	sendAroundCharacterAttack2();

	CMonster* pVictimMonster;

	if (getAttack2VictimMonster(&pVictimMonster) == FALSE)
	{
		return TRUE;
	}	

	// 데미지 뿌리기
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
//	// 공격 모션 뿌리기
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
//	// 데미지 이모션 뿌리기
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
		// mbLoginFlag 확인 및 CristalCount 증가로 인해 PlayerLock 사용해서 동기화를 맞춘다.
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

	// 앉기 플래그로 변경
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


	// 정해진 캐릭터 타입의 범위를 벗어났다면 return FALSE
	// 특정 패킷을 보내고 끊는다는 건 의미가 없음 그냥 끊어버리자.
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

	// 삭제되는 섹터에 clientID 오브젝트 삭제 요청 보내기
	sendRemoveSectorDeleteMyCharacter(&removeSectorAround);
	
	// 삭제되는 섹터에 있는 오브젝트 삭제요청 보내기
	sendRemoveSectorDeleteObject(&removeSectorAround);

	// 추가되는 섹터에 pPlayer 캐릭터 생성 요청 보내기
	sendAddSectorCreateMyCharacter(&addSectorAround);

	// 추가되는 섹터에 있는 오브젝트 pPlayer한테 생성요청 보내기
	sendAddSectorCreateObject(&addSectorAround);

	return;
}

void CPlayer::sendRemoveSectorDeleteMyCharacter(CGodDamnBug::stSectorAround* pRemoveSectorAround)
{
	CMessage* pMessage = CMessage::Alloc();

	// 오브젝트 삭제 패킷 만들기
	CGodDamnBug::PackingRemoveObject(mClientID, pMessage);

	for (INT count = 0; count < pRemoveSectorAround->count; ++count)
	{
		// 삭제되는 섹터에 전부 뿌리기
		mpGodDamnBug->SendPacketOneSector(pRemoveSectorAround->around[count].posX, pRemoveSectorAround->around[count].posY, nullptr, pMessage);
	}

	pMessage->Free();

	return;
}


void CPlayer::sendRemoveSectorDeleteObject(CGodDamnBug::stSectorAround* pRemoveSectorAround)
{
	for (INT count = 0; count < pRemoveSectorAround->count; ++count)
	{	
		// 삭제되는 영역의 플레이어 오브젝트 삭제
		for (auto& iter : mpGodDamnBug->mPlayerSectorMap[pRemoveSectorAround->around[count].posY][pRemoveSectorAround->around[count].posX])
		{
			CMessage* pMessage = CMessage::Alloc();

			CGodDamnBug::PackingRemoveObject(iter.second->mClientID, pMessage);

			SendPacket(pMessage);

			pMessage->Free();
		}


		// 삭제되는 영역의 몬스터 오브젝트 삭제
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

	// pPlayer 캐릭터 생성 패킷 만들기
	CGodDamnBug::PackingCreateOthreadCharacter(mClientID, mCharacterType, mPlayerNick, player::PLAYER_STRING * sizeof(WCHAR), mPosX, mPosY, mRotation, mLevel, FALSE, mbSitFlag, mbDieFlag, pMessage);

	for (INT count = 0; count < pAddSectorAround->count; ++count)
	{
		// 추가되는 섹터에 뿌리기
		mpGodDamnBug->SendPacketOneSector(pAddSectorAround->around[count].posX, pAddSectorAround->around[count].posY, nullptr, pMessage);
	}

	pMessage->Free();

	return;
}

// 주변 섹터에 해당 오브젝트 삭제요청 보내기
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
		// 추가되는 섹터의 플레이어 생성요청
		for (auto& iter : mpGodDamnBug->mPlayerSectorMap[pAddSectorAround->around[count].posY][pAddSectorAround->around[count].posX])
		{
			CPlayer* pAddPlayer = iter.second;

			CMessage* pMessage = CMessage::Alloc();

			CGodDamnBug::PackingCreateOthreadCharacter(pAddPlayer->mClientID, pAddPlayer->mCharacterType, pAddPlayer->mPlayerNick, player::PLAYER_STRING * sizeof(WCHAR), pAddPlayer->mPosX, pAddPlayer->mPosY, pAddPlayer->mRotation, pAddPlayer->mLevel, FALSE, pAddPlayer->mbSitFlag, pAddPlayer->mbDieFlag, pMessage);

			SendPacket(pMessage);

			pMessage->Free();
		}

		// 추가되는 섹터의 몬스터 생성요청
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

	// pVictimPlayer 까지 포함해서 패킷을 뿌린다.
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

	// pVictimPlayer 까지 포함해서 패킷을 뿌린다.
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

	// 플레이어 섹터 셋팅
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

		// 플레이어 죽음을 DB에 저장한다.
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
	// 클라이언트 오브젝트를 지워야 할 때
	if (bObjectRemoveFlag == TRUE)
	{
		sendAroundRemoveObject(mCurSector.posX, mCurSector.posY, mClientID);

		mpGodDamnBug->ErasePlayerSectorMap(mCurSector.posX, mCurSector.posY, mClientID);
	}

	{
		CCriticalSection criticalSection(&mPlayerLock);

		// 재시작을 위한 HP 및 좌표를 새로 할당 받는다.
		setRestartPlayerInfo();
	}

	// 새로 셋팅한 좌표를 기준으로 섹터에 다시 넣어준다.
	mpGodDamnBug->InsertPlayerSectorMap(mCurSector.posX, mCurSector.posY, mClientID, this);

	// 플레이어 재시작 메시지 송신
	sendPlayerRestart();

	// 내 캐릭터 생성 메시지 송신
	sendCreateMyCharacter();

	// 내 캐릭터 생성하라고 주변에 뿌리기
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
