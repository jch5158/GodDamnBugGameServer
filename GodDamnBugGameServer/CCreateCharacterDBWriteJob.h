#pragma once
class CCreateCharacterDBWriteJob : public CDBWriteJob
{
public:

	CCreateCharacterDBWriteJob(void);

	~CCreateCharacterDBWriteJob(void);

	virtual void DBWrite(void) final;

	void SetCreateCharacterDBWriteJob(CTLSDBConnector* pDBConnector, CPlayer *pPlayer);

	void* operator new(size_t size);

	void operator delete(void* ptr);

private:

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

};

