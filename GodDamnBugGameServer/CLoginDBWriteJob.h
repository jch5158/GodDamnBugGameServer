#pragma once



class CLoginDBWriteJob : public CDBWriteJob
{
public:

	CLoginDBWriteJob(void);

	~CLoginDBWriteJob(void);

	virtual void DBWrite(void) final;
	
	//BYTE status, BYTE characterType, FLOAT posX, FLOAT posY, INT tileX, INT tileY, INT rotation, INT cristalCount, INT HP, UINT64 EXP, INT level, BOOL bDieFlag, WCHAR* pClientIP, INT clientPort
	void SetLoginDBWriteJob(CTLSDBConnector* pDBConnector, CPlayer* pPlayer, BYTE status);

	void* operator new(size_t size);

	void operator delete(void* ptr);

private:

	// �α��� ���� ���
	BYTE mStatus;

	BYTE mCharacterType;

	FLOAT mPosX;

	FLOAT mPosY;

	INT mTileX;

	INT mTileY;

	INT mRotation;

	INT mCristalCount;

	INT mHP;

	UINT64 mEXP;

	INT mLevel;

	BOOL mbDieFlag;

	WORD mClientPort;

	WCHAR mClientIP[IP_LENGTH];

};

