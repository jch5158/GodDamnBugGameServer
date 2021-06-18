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

	// ���� �������� ���ݴ��ߴ� Tick, Ư�� �ð� �̳��� �������� ���ظ� �Ծ��ٸ� ��ó ������ ã�ư���.
	DWORD mLastSufferTick;
	
	// MONSTER_SPEED�� ���缭 �����̵��� �ϱ� ���� Tick
	DWORD mLastMoveTick;

	// ������ �÷��̾ �Ѿư��� ������ ���� üũ�ϴ� Tick 
	// ���Ͱ� �������� ���� �����ϸ� Ŭ���� �ִϸ��̼� ó���� �Ϸ�Ǳ� ���� ���ݵȴ�.
	DWORD mLastMoveForAttackTick;

	// ���� ���� �ӵ��� �����ϱ� ���� Tick
	DWORD mLastAttackTick;

	// ���Ͱ� ���� �ֱ�� �ǻ�� ������ �Ѵ�.
	DWORD mLastDieTick;

	CGodDamnBug* mpGodDamnBug;


	inline static INT mInitializeHP;
	inline static INT mInitializeDamage;
	inline static CTLSLockFreeObjectFreeList<CMonster> mMonsterFreeList = { 0, FALSE };
};

