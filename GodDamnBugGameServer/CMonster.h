#pragma once


class CMonster
{
public:

	template<class DATA>
	friend class CTLSLockFreeObjectFreeList;

	friend class CGodDamnBug;
	friend class CPlayer;

	static void SetMonsterDefaultValue(INT initializeHP, INT initializeDamage);

	static CMonster* Alloc(UINT64 clientID, INT monsterType, INT activityArea, CGodDamnBug *pGodDamnBug);

	void Free(void);

	void Update(void);

	void UpdateAttackDamage(UINT64 accountNo, INT damage);

private:

	CMonster(void);

	~CMonster(void);

	void setMonsterRespawnCoordinate(void);

	void setMonsterTypeStats(void);

	void setMonsterInfo(UINT64 clientID, INT monsterType, INT activityArea);

	void revivalMonster(void);
	void moveMonster(void);
	void attackToAttackedPlayer(void);

	BOOL checkAttackDistance(CPlayer *pPlayer);

	BOOL chaeckMonsterMoveRange(INT moveTileX, INT moveTileY);

	INT getGameRotation(INT rotation);
	void getMonsterMoveCoordinate(INT rotation, FLOAT* moveX, FLOAT* moveY);
	void getMonsterToAttackedCoordinate(CPlayer *pPlayer, INT rotation, FLOAT* pMoveX, FLOAT* pMoveY);

	BOOL updateMonsterSector(void);

	void sendAroundCreateMonster(void);
	void sendAroundUpdateMonsterSector(void);
	void sendRemoveAroundDeleteObject(CGodDamnBug::stSectorAround* pRemoveSectorAround);
	void sendAddAroundCreateObject(CGodDamnBug::stSectorAround* pAddSectorAround);
	void sendAroundMonsterMoveStart(void);
	void sendAroundMonsterDie(void);
	void sendAroundMonsterAttackAction(void);
	void sendAroundMonsterAttackDamage(CPlayer *pPlayer);

	BOOL checkAttackedPlayerSector(CPlayer* pPlayer);

	void getRotationForAttackedPlayer(CPlayer* pPlayer, INT *pRotation);


	UINT64 mClientID;

	INT mMonsterType;

	INT mActivityArea;

	BOOL mbDieFlag;

	INT mHP;

	INT mDamage;

	FLOAT mPosX;
	FLOAT mPosY;

	INT mTileX;
	INT mTileY;

	INT mRotation;


	CGodDamnBug::stSector mCurSector;
	CGodDamnBug::stSector mOldSector;

	INT64 mPlayerAccountNo;

	// 가장 마지막에 공격당했던 Tick, 특정 시간 이내에 유저에게 피해를 입었다면 근처 유저를 찾아간다.
	DWORD mLastSufferTick;
	
	// MONSTER_SPEED에 맞춰서 움직이도록 하기 위한 Tick
	DWORD mLastMoveTick;

	// 공격한 플레이어를 쫓아가서 때리기 위해 체크하는 Tick 
	// 몬스터가 움직이자 마자 공격하면 클라의 애니메이션 처리가 완료되기 전에 공격된다.
	DWORD mLastMoveForAttackTick;

	// 몬스터 공격 속도를 조정하기 위한 Tick
	DWORD mLastAttackTick;

	// 몬스터가 일정 주기로 되살아 나도록 한다.
	DWORD mLastDieTick;

	CGodDamnBug* mpGodDamnBug;


	inline static INT mInitializeHP;
	inline static INT mInitializeDamage;
	inline static CTLSLockFreeObjectFreeList<CMonster> mMonsterFreeList = { 0, FALSE };
};

