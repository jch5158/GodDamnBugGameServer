#pragma once
class CRestartDBWriteJob : public CDBWriteJob
{
public:

	CRestartDBWriteJob(void);

	~CRestartDBWriteJob(void);

	virtual void DBWrite(void) final;

	void SetRestartDBWriteJob(CTLSDBConnector* pDBConnector, CPlayer* pPlayer);

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

