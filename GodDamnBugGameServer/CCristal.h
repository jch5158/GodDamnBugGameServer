#pragma once


class CCristal
{
public:

	template <class DATA>
	friend class CTLSLockFreeObjectFreeList;

	friend class CGodDamnBug;
	friend class CPlayer;

	static void SetCristalTypeAmount(INT typeAmountArray[]);

	static CCristal* Alloc(UINT64 clientID, FLOAT posX, FLOAT posY, INT tileX, INT tileY, CGodDamnBug::stSector curSector);

	void Free(void);

	INT GetCristalAmount(void) const;

private:

	CCristal(void);

	~CCristal(void);

	static BYTE gatchaCristalType(void);
	void setCristalAmount(void);
	
	// key : type, value : amount
	inline static std::unordered_map<INT, INT> mCristalAmountMap;
	inline static CTLSLockFreeObjectFreeList<CCristal> mCristalFreeList = { 0, FALSE };
	

	UINT64 mClientID;

	BYTE mCristalType;

	FLOAT mPosX;
	FLOAT mPosY;

	INT mTileX;
	INT mTileY;

	INT mAmount;

	CGodDamnBug::stSector mCurSector;
};

