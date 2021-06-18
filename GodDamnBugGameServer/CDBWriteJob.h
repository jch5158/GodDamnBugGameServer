#pragma once


class CPlayer;

class CGodDamnBug;

class CLoginDBWriteJob;

class CDBWriteJob
{
public:

	friend class CGodDamnBug;

	CDBWriteJob(void);

	virtual ~CDBWriteJob(void);

	virtual void DBWrite(void) = 0;

protected:

	UINT64 mAccountNo;

	CTLSDBConnector* mpDBConnector;
	
	inline static CTLSLockFreeObjectFreeList<CLoginDBWriteJob> mObjectFreeList = { 0,FALSE };
};

