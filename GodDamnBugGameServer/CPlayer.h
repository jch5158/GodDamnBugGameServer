#pragma once

class CPlayer : public CMMOServer::CSession
{
public:

	friend class CGodDamnBug;
	friend class CCristal;
	friend class CMonster;
	friend class CLoginDBWriteJob;
	friend class CLogoutDBWriteJob;
	friend class CRestartDBWriteJob;
	friend class CHPRecoveryDBWriteJob;
	friend class CDieDBWriteJob;
	friend class CCristalPickDBWriteJob;
	friend class CCreateCharacterDBWriteJob;

	CPlayer(void);

	~CPlayer(void);

	virtual void OnAuthClientJoin(void) final;

	virtual void OnAuthClientLeave(void) final;

	virtual void OnAuthMessage(CMessage* pMessage) final;

	virtual void OnGameClientJoin(void) final;

	virtual void OnGameClientLeave(void) final;

	virtual void OnGameMessage(CMessage* pMessage) final;

	void SetGodDamnBugPtr(CGodDamnBug* pGodDamnBug);

	static void SetPlayerDefaultValue(INT damage, INT HP, INT recoveryHP);

	void SetPlayerInfo(UINT64 clientID, BYTE characterType, FLOAT posX, FLOAT posY, INT tileX, INT tileY, WORD rotation, INT cristalCount, INT HP, INT64 EXP, WORD level, BOOL bDieFlag, INT damage, INT recoveryHP, LONG DBWriteCount);

	void SetPlayerRespawnCoordinate(void);

	LONG GetDBWriteCount(void) const;

	UINT64 GetAccountNo(void) const;

	UINT64 GetClientID(void) const;

	void HPRecovery(void);

	void UpdateAttackDamage(INT damage);

private:

	BOOL setDuplicateLoginPlayer(BYTE *pStatus);
	
	BOOL getPlayerInfoFromGameDB(void);

	BOOL authRecvProcedure(WORD messageType, CMessage* pMessage);
	BOOL gameRecvProcedure(WORD messageType, CMessage* pMessage);

	BOOL recvLoginRequest(CMessage* pMessage);
	BOOL recvCharacterSelectRequest(CMessage* pMessage);
	BOOL recvMoveCharacter(CMessage* pMessage);
	BOOL recvMoveStopCharacter(CMessage* pMessage);
	BOOL recvCharacterAttack1(CMessage* pMessage);
	BOOL recvCharacterAttack2(CMessage* pMessage);
	BOOL recvCristalPick(CMessage* pMessage);
	BOOL recvCharacterSit(CMessage* pMessage);
	BOOL recvPlayerRestart(CMessage* pMessage);


	// ���ú� ��Ŷ ����
	BOOL validateRecvCharacterSelectRequest(BYTE characterType);
	BOOL validateRecvMoveCharacter(UINT64 clientID, FLOAT posX, FLOAT posY, WORD rotation, BYTE vKey, BYTE hKey);
	BOOL validateRecvMoveStopCharacter(UINT64 clientID, FLOAT posX, FLOAT posY, WORD rotation);
	BOOL validateRecvCharacterAttack1(UINT64 clientID);
	BOOL validateRecvCharacterAttack2(UINT64 clientID);
	BOOL validateRecvCristalPick(UINT64 clientID);
	BOOL validateRecvCharacterSit(UINT64 clientID);
	BOOL validateRecvPlayerRestart(void);


	// send
	void sendLoginResponse(BYTE status);
	void sendCharacterSelectResponse(BYTE status);

	// �� ĳ���� ������û ������
	void sendCreateMyCharacter(void);
	
	// �ֺ� ���Ϳ� �ִ� �����鿡�� ������ ĳ���� ���� ��û������
	void sendAroundRespawnCharacter(void);
	
	// �ֺ� ���Ϳ� �ִ� �ٸ� ������ ĳ���� ���� ��û ������
	void sendCreateAroundOthrerObject(void);


	// �÷��̾� ���Ͱ� ����Ǿ��� �� ȣ���ϴ� �Լ�
	void sendAroundUpdateCharacterSector(void);

	// �����Ǵ� ���Ϳ� ��ĳ���� ���� ��û ������
	void sendRemoveSectorDeleteMyCharacter(CGodDamnBug::stSectorAround* pRemoveSectorAround);

	// �����Ǵ� ���Ϳ� �ִ� ������Ʈ ������û ������
	void sendRemoveSectorDeleteObject(CGodDamnBug::stSectorAround* pRemoveSectorAround);

	// �߰��Ǵ� ���Ϳ� pPlayer ������û ������
	void sendAddSectorCreateMyCharacter(CGodDamnBug::stSectorAround* pAddSectorAround);

	// �߰��Ǵ� ���Ϳ� �ִ� ������Ʈ ������û ������
	void sendAddSectorCreateObject(CGodDamnBug::stSectorAround* pAddSectorAround);

	// �ֺ� ���Ϳ� �ش� ������Ʈ ������û ������
	void sendAroundRemoveObject(INT sectorPosX, INT sectorPosY, UINT64 clientID);


	void sendAroundCharacterMoveStart(void);
	void sendAroundCharacterMoveStop(void);
	void sendAroundCharacterAttack1(void);
	void sendAroundCharacterAttack2(void);
	void sendAroundCristalPickAction(void);
	void sendAroundCristalPick(UINT64 cristalClientID);
	void sendAroundCharacterSit(void);
	void sendAroundAttack1Damage(CMonster* pVictimMonster, INT damage);
	
	// �������� ��
	//void sendAroundAttack2Damage(CMonster** pVicTimMonsterArray, DWORD victimMonsterCount, INT damage, CGodDamnBug::stSectorAround16 *pSectorAround16);
	
	void sendAroundAttack2Damage(CMonster* pVictimMonster, INT damage);


	void sendAroundPlayerDie(INT minusCristal);	
	void sendPlayerHP(void);
	void sendPlayerRestart(void);


	// �÷��̾��� ������ ������ �Ǿ����� Ȯ���Ѵ�.
	BOOL updatePlayerSector(void);

	BOOL getAttack1VictimMonster(CMonster** pVictimMonster);
	
	// ���� ���ݿ�
	//BOOL getAttack2VictimMonsters(CMonster** pVictimMonsterArray, DWORD arrayLength, DWORD* pFindVictimMonsterCount, CGodDamnBug::stSectorAround16* pSectorAround16);

	BOOL getAttack2VictimMonster(CMonster** pVictimMonster);



	void playerRestart(BOOL bObjectRemoveFlag);

	void getCristalPickCollisionBox(CGodDamnBug::stCollisionBox *pCollisionBox);
	BOOL pickCristal(CCristal** pCristal);

	// �÷��̾ Restart �� �ϱ����� ������ �����Ѵ�.
	void setRestartPlayerInfo(void);

	// mbLoginFlag�� FALSE�� �������ش�.
	void setAuthClientJoinState(void);


	// ĳ���� ���� �� �⺻ ���ݷ�
	inline static INT mInitializeDamage = 0;

	// ĳ���� ���� �� �⺻ HP
	inline static INT mInitializeHP = 0;

	// ĳ���� ���� �� �⺻ ȸ�� HP
	inline static INT mInitializeRecoveryHP = 0;

	UINT64 mAccountNo;

	UINT64 mClientID;

	BYTE mCharacterType;

	FLOAT mPosX;

	FLOAT mPosY;

	INT mTileX;

	INT mTileY;	

	WORD mRotation;

	INT mCristalCount;

	INT mHP;

	INT64 mEXP;

	WORD mLevel;

	BOOL mbDieFlag;

	INT mDamage;

	INT mRecoveryHP;

	BOOL mbSitFlag;

	BOOL mbLoginFlag;

	//����Ű ��������. 0 (�ȴ���) - 1 (�Ʒ�) - 2 (��)
	BYTE mVKey;

	//�¿�Ű ��������. 0 (�ȴ���) - 1 (����) - 2 (����)
	BYTE mHKey;

	// ���� ����
	CGodDamnBug::stSector mCurSector;

	// ���� ����
	CGodDamnBug::stSector mOldSector;

	INT mOldHP;

	// ó�� ���� �ð� DB �����
	DWORD mRecoveryHPStartTick;

	// �ð��� ���� ȸ����
	DWORD mRecoveryHPLastTick;

	// DB �����忡�� ó������ ��û���� ����, mDBWriteCount �� 0�� �Ǳ� ���� player�� �������� �ʴ´�.
	// ó�����̶���� ���� ���������� ���ο� ������ �����Ͽ��� �� �ش� player �� �̾��ش�.
	LONG mDBWriteCount;

	
	// DBWriteThread ���� DBCount�� ���ҽ�Ű��, AuthThread���� �ߺ� �α������� ���� Player ���� �����ϴµ� ����ϴ� ����ȭ ��ü
	CRITICAL_SECTION mPlayerLock;

	CGodDamnBug* mpGodDamnBug;
	
	WCHAR mPlayerID[player::PLAYER_STRING];

	WCHAR mPlayerNick[player::PLAYER_STRING];
};

