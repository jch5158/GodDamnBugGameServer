#pragma once

class CCristal;
class CMonster;

class CGodDamnBug : public CMMOServer
{
public:

	friend class CPlayer;
	friend class CMonster;

	CGodDamnBug(void);

	~CGodDamnBug(void);

	struct stTilePoint
	{
		INT tileX;
		INT tileY;
	};

	struct stCollisionBox
	{
		stTilePoint pointOne;
		stTilePoint pointTwo;
	};

	struct stSector
	{
		INT posX;
		INT posY;
	};

	struct stSectorAround
	{
		INT count;
		stSector around[9];
	};

	struct stSectorAround16
	{
		INT count;
		stSector around[16];
	};


	CRedisConnector* GetRedisConnector(void); 

	CTLSDBConnector* GetDBConnector(void);

	CLanLoginClient* GetLanLoginClient(void) const;

	LONG GetDBWriteThreadTPS(void) const;

	DWORD GetDBWriteThreadQueueSize(void);

	BOOL InsertPlayerMap(UINT64 accountNo, CPlayer* pPlayer);
	BOOL ErasePlayerMap(UINT64 accountNo);
	CPlayer* FindPlayerFromPlayerMap(UINT64 accountNo);
	
	BOOL InsertPlayerSectorMap(INT sectorPosX, INT sectorPosY, UINT64 clientID, CPlayer* pPlayer);
	BOOL ErasePlayerSectorMap(INT sectorPosX, INT sectorPosY, UINT64 clientID);

	BOOL InsertCristalSectorMap(INT sectorPosX, INT sectorPosY, UINT64 clientID, CCristal* pCristal);
	BOOL EraseCristalSectorMap(INT sectorPosX, INT sectorPosY, UINT64 clientID);

	void InsertMonsterArray(CMonster* pMonster);
	BOOL EraseMonsterArray(CMonster* pMonster);
	CMonster* FindMonsterFromMonsterArray(UINT64 clientID);

	BOOL InsertMonsterSectorMap(INT sectorPosX, INT sectorPosY, UINT64 clientID, CMonster* pMonster);
	BOOL EraseMonsterSectorMap(INT sectorPosX, INT sectorPosY, UINT64 clientID);


	BOOL InsertAuthReleasePlayerMap(UINT64 accountNo, CPlayer* pPlayer);
	BOOL EraseAuthReleasePlayerMap(UINT64 accountNo);
	CPlayer* FindPlayerFromAuthReleasePlayerMap(UINT64 accountNo);


	BOOL InsertReleasePlayerMap(UINT64 accountNo, CPlayer* pPlayer);
	BOOL EraseReleasePlayerMap(UINT64 accountNo);
	CPlayer* FindPlayerFromReleasePlayerMap(UINT64 accountNo);


	BOOL InsertSeatedPlayerMap(UINT64 clientID, CPlayer* pPlayer);
	BOOL EraseSeatedPlayerMap(UINT64 clientID);
	CPlayer* FindPlayerFromSeatedPlayerMap(UINT64 clientID);

	void SetLoginClient(CLanLoginClient* pLanLoginClient);

	UINT64 GetObjectID(void);

	void SendPacketOneSector(INT sectorPosX, INT sectorPosY, CPlayer* pExceptionPlayer, CMessage* pMessage);
	void SendPacketAroundSector(INT sectorPosX, INT sectorPosY, CPlayer* pExceptionPlayer, CMessage* pMessage);


	// tile ?????? ???????? ???? ?????? ???????? ????
	static void GetSector(INT tileX, INT tileY, CGodDamnBug::stSector* pSector);
	
	// sectorPosX, sectorPosY ?? ???????? ???? ???? ?????????? ???????? ????
	static void GetSectorAround(INT sectorPosX, INT sectorPosY, CGodDamnBug::stSectorAround* pSectorAround);

	static void GetSectorAround16(INT sectorPosX, INT sectorPosY, CGodDamnBug::stSectorAround16* pSectorAround16);
	
	// ???????? ???? ???????? ???? ?????? ?????? ???????? ?????? ???????? ????????.
	static void GetUpdateSectorAround(stSector oldSector, stSector curSector, stSectorAround* pRemoveSectorAround, stSectorAround* pAddSectorAround);

	
	// pMessage ???? ????
	static void PackingLoginResponse(BYTE status, UINT64 accountNo, CMessage* pMessage);
	static void PackingCharacterSelectResponse(BYTE status, CMessage* pMessage);
	static void PackingCreateMyCharacter(UINT64 clientID, BYTE characterType, WCHAR* pNickName, DWORD cbNickName, FLOAT posX, FLOAT posY, WORD rotaion, INT cristalCount, INT HP, INT64 EXP, WORD level, CMessage* pMessage);
	static void PackingCreateOthreadCharacter(UINT64 clientID, BYTE characterType, WCHAR* pNickName, DWORD cbNickName, FLOAT posX, FLOAT posY, WORD rotation, WORD level, BYTE bRespawnFlag, BYTE bSitFlag, BYTE bDieFlag, CMessage* pMessage);
	static void PackingCreateMonster(UINT64 clientID, FLOAT posX, FLOAT posY, WORD rotation, BYTE bRespawnFlag, CMessage *pMessage);
	static void PackingRemoveObject(UINT64 clientID, CMessage* pMessage);
	static void PackingCharacterMoveStart(UINT64 clientID, FLOAT posX, FLOAT posY, WORD rotation, BYTE vKey, BYTE hKey, CMessage* pMessage);
	static void PackingCharacterMoveStop(UINT64 clientID, FLOAT posX, FLOAT posY, WORD rotation, CMessage* pMessage);
	static void PackingMonsterMoveStart(UINT64 clientID, FLOAT posX, FLOAT posY, WORD rotation, CMessage* pMessage);
	static void PackingPlayerAttack1Action(UINT64 clientID, CMessage* pMessage);
	static void PackingPlayerAttack2Action(UINT64 clientID, CMessage* pMessage);
	static void PackingMonsterAttackAction(UINT64 clientID, CMessage* pMessage);
	static void PackingCristalPickAction(UINT64 clientID, CMessage* pMessage);
	static void PackingCristalPick(UINT64 clientID, UINT64 cristalClientID, INT playerCristalAmount, CMessage* pMessage);
	static void PackingCharacterSit(UINT64 clientID, CMessage* pMessage);
	static void PackingAttackDamage(UINT64 attackClientID, UINT64 victimClientID, INT damage, CMessage* pMessage);
	static void PackingPlayerDie(UINT64 clientID, INT minusCristal, CMessage* pMessage);
	static void PackingMonsterDie(UINT64 clientID, CMessage* pMessage);
	static void PackingCreateCristal(UINT64 clientID, BYTE cristalType, FLOAT posX, FLOAT posY, CMessage* pMessage);
	static void PackingPlayerHP(INT HP, CMessage* pMessage);
	static void PackingPlayerRestart(CMessage* pMessage);

	static void GetAttackCollisionBox(INT rotation, INT tileX, INT tileY, INT collisionRange, stCollisionBox* pCollisionBox);
	static void GetAttackSectorAround16(INT rotation, stSector curSector, stSectorAround16* pAttackSectorAround16);


	BOOL GetIDWithNickFromAccountDB(UINT64 accountNo, WCHAR* pPlayerID, DWORD cbPlayerID, WCHAR* pPlayerNick, DWORD cbPlayerNick);


	void CreateCristal(FLOAT posX, FLOAT posY, INT tileX, INT tileY, CGodDamnBug::stSector curSector);
	void RemoveCristal(CCristal *pCristal);

	void InsertLoginDBWriteJob(BYTE status, CPlayer* pPlayer);
	void InsertLogoutDBWriteJob(BYTE status, CPlayer* pPlayer);
	void InsertDieDBWriteJob(CPlayer* pPlayer);
	void InsertCreateCharacterDBWriteJob(CPlayer* pPlayer);
	void InsertRestartDBWriteJob(CPlayer* pPlayer);
	void InsertCristalPickDBWriteJob(INT getCristalCount, CPlayer* pPlayer);
	void InsertHPRecoveryDBWriteJob(CPlayer* pPlayer);

	void InitializeDBWriteThreadTPS(void);

	BOOL CompareSessionToken(UINT64 accountNo, CHAR* pSessionToken, DWORD tokenSize);

private:

	virtual BOOL OnStart(void) final;

	virtual void OnStartAuthenticThread(void) final;

	virtual void OnStartUpdateThread(void) final;

	// accept ???? ???? ????
	virtual BOOL OnConnectionRequest(WCHAR* userIP, WORD userPort) final;

	virtual void OnAuthenticUpdate(void) final;

	// UpdateThread ???? ?????? ?????????? 1 Loop?? ???????? ????????.
	virtual void OnGameUpdate(void) final;

	// Player ?? Session ?????? ???? ?????????? ????
	virtual void OnAssociateSessionWithPlayer(void) final;

	virtual void OnCloseAuthenticThread(void) final;

	virtual void OnCloseUpdateThread(void) final;

	virtual void OnStop(void) final;


	// gamddb?? ?????????? ????
	void connectDB(void);

	// gamedb ???? ???? ????
	void disconnectDB(void);

	// ?????? ???? ?????? ????
	// ???? ??????, ???? ????, ???? ???? ?????? 
	void setPlayerStats(void);
	void getPlayerStatsFromGameDB(INT* pDamage, INT* pHP, INT* pRecoveryHP);

	void setMonsterStats(void);	
	void getMonsterStatsFromGameDB(INT monsterType, INT *pHP, INT* pDamage);


	// DB???? ???????? ?????? ?????? ???? ?? ???????? ?????? ???? ????
	void setCristalAmount(void);
	void getCristalAmountFromGameDB(INT typeAmountArray[]);
		

	// DB ?????? ???? ????
	BOOL setupDBWriteThread(void);

	// DB ?????? ????
	void closeDBWriteThread(void);

	// DB ??????
	static DWORD WINAPI ExecuteDBWriteThread(void *pParam);

	// DB ?????? ????
	void DBWriteThread(void);


	// ???? ???? ?????? ???????? ???????? ????????.
	void setMonsters(void);
	void setArea1Monsters(void);
	void setArea2Monsters(void);
	void setArea3Monsters(void);
	void setArea4Monsters(void);
	void setArea5Monsters(void);
	void setArea6Monsters(void);
	void setArea7Monsters(void);


	void authReleasePlayers(void);


	void monstersUpdate(void);
	void gameReleasePlayers(void);
	void recoverySeatedPlayer(void);

	void sendAroundCreateCristal(CCristal* pCristal);



	// ???????????? ???? ?????? ???????? ???? ????
	UINT64 mObjectID;

	LONG mDBWriteThreadTPS;

	BOOL mbDBWriteThreadFlag;

	DWORD mDBWriteThreadID;

	CLanLoginClient* mpLanLoginClient;

	HANDLE mDBWriteThreadHandle;

	HANDLE mDBWriteJobEvent;

	// ???????? ?? ??
	// ???? GameThread?????? ???????? ???????? ?????? GameThread???? FindPlayerFromPlayerMap?? ?????? ???? ???? ?????? ?????? ????.
	CRITICAL_SECTION mPlayerMapLock;

	// ?????? ??????
	CRedisConnector mRedisConnector;

	// gamedb?? ???? ??????
	CTLSDBConnector mDBConnector;

	CLockFreeQueue<CDBWriteJob*> mDBWriteJobQueue;


	// ?????? ???? ???? Player ?????? ???????? ????????
	// key : clientID, value : CPlayer* 
	std::unordered_map<UINT64, CPlayer*> mSeatedPlayerMap;

	// AuthenticThread???? GameThread?? ???????? ?????? ???? ?????????? ???????? ???? Map
	// key : accountNo, value : CPlayer* 
	std::unordered_map<UINT64, CPlayer*> mAuthReleasePlayerMap;

	// Release?? ???????? Player ?????? ???????? ????????
	// key : accountNo, value : CPlayer*
	std::unordered_map<UINT64, CPlayer*> mReleasePlayerMap;
	
	// ???? ???????? ?????? ???????? Player Map 
	// key : accountNo, value : CPlayer*
	std::unordered_map<UINT64, CPlayer*> mPlayerMap;

	std::vector<CMonster*> mMonsterArray;

	// ???? ???????? ?????? ???? CPlayer* ?????? ???????? ????????
	// key : clientID, value : CPlayer*
	std::unordered_map<UINT64, CPlayer*> mPlayerSectorMap[Y_SECTOR_RANGE][X_SECTOR_RANGE];	

	
	// ???? ???????? ?????? ???? CMonster* ?????? ???????? ????????
	// key : clientID, value : CPlayer*
	std::unordered_map<UINT64, CMonster*> mMonsterSectorMap[Y_SECTOR_RANGE][X_SECTOR_RANGE];


	// ???? ???????? ?????? ???? CCristal* ?????? ???????? ????????
	// key : clientID, value : CCristal*
	std::unordered_map<UINT64, CCristal*> mCristalSectorMap[Y_SECTOR_RANGE][X_SECTOR_RANGE];

};

