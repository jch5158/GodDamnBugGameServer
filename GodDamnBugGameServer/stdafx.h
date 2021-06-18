#pragma once

#define _USE_MATH_DEFINES

#include <WS2tcpip.h>
#include <time.h>
#include <math.h>
#include "RedisLibrary/RedisLibrary/CTLSRedisConnector.h"
#include "DBConnectorLibrary/DBConnectorLibrary/CTLSDBConnector.h"
#include "DumpLibrary/DumpLibrary/CCrashDump.h"
#include "SystemLogLibrary/SystemLogLibrary/CSystemLog.h"
#include "CriticalSectionLibrary/CriticalSectionLibrary/CCriticalSection.h"
#include "PerformanceProfiler/PerformanceProfiler/CTLSPerformanceProfiler.h"
#include "ParserLibrary/ParserLibrary/CParser.h"
#include "CPUProfiler/CPUProfiler/CCPUProfiler.h"
#include "HardwareProfilerLibrary/HardwareProfilerLibrary/CHardwareProfiler.h"

#include "MessageLibrary/MessageLibrary/CMessage.h"
#include "RingBufferLibrary/RingBufferLib/CRingBuffer.h"
#include "RingBufferLibrary/RingBufferLib/CTemplateRingBuffer.h"
#include "LockFreeObjectFreeList/ObjectFreeListLib/CLockFreeObjectFreeList.h"
#include "LockFreeObjectFreeList/ObjectFreeListLib/CTLSLockFreeObjectFreeList.h"
#include "LockFreeStack/LockFreeStackLib/CLockFreeStack.h"
#include "LockFreeQueue/LockFreeQueueLib/CLockFreeQueue.h"

#include "CommonProtocol.h"
#include "GodDamnBugDifine.h"
#include "ProfileDataType.h"
#include "MMOServer/MMOServer/CMMOServer.h"
#include "NetworkEngine/LanClientEngine/LanClientEngine/CLanClient.h"

#include "CDBWriteJob.h"
#include "CLoginDBWriteJob.h"
#include "CLogoutDBWriteJob.h"
#include "CDieDBWriteJob.h"
#include "CCreateCharacterDBWriteJob.h"
#include "CRestartDBWriteJob.h"
#include "CCristalPickDBWriteJob.h"
#include "CHPRecoveryDBWriteJob.h"

#include "CLanLoginClient.h"
#include "CGodDamnBug.h"
#include "CLanMonitoringClient.h"
#include "CPlayer.h"
#include "CCristal.h"
#include "CMonster.h"

#include <unordered_map>
#include <list>

#pragma comment(lib,"Winmm.lib")