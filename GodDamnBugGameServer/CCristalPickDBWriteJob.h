#pragma once
class CCristalPickDBWriteJob : public CDBWriteJob
{
public:

	CCristalPickDBWriteJob(void);

	~CCristalPickDBWriteJob(void);

	virtual void DBWrite(void) final;

	void SetCristalPickDBWriteJob(CTLSDBConnector* pDBConnector, CPlayer *pPlayer, INT getCristalCount);

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

	INT mGetCristalCount;

};

