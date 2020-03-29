#include "CAPI.h"
#include "USNutil.h"

using namespace Kevin::Util;

USNutil util;

int KEVIN_CALLBACK GetAllDrives(RetStrArr* retA)
{
	RetStrArr* ret = (RetStrArr*)malloc(sizeof(RetStrArr) * MAXDRIVE);
	auto driveList = util.GetAllDrive();
	for (int i = 0; i < driveList.size(); i++)
	{
		int len = driveList[i].length();
		const char* driveName = driveList[i].c_str();
		ret[i].result = driveName;
		ret[i].resultLen = len;
	}
	return 0;
}

int KEVIN_CALLBACK EnumAllNTFSFileName(FilePathCallBack& callback)
{
	HANDLE handle = NULL;
	remove("fileinfo.txt");
	USNutil util;
	auto driveList = util.GetAllDrive();
	for (auto a : driveList)
	{
		if (util.CheckIfNTFS(a))
		{
			if (util.GetDriveHandle(a, handle))
			{
				if (util.InitUsn(handle))
				{
					USN_JOURNAL_DATA usnInfo;
					if (util.GetUsnInfo(handle, usnInfo))
					{
						auto refsNum = util.GetUsnData(handle, usnInfo);
						for (auto num : refsNum)
						{
							util.GetPathFromRefNum(handle, num, a, callback);
						}
					}
				}
				CloseHandle(handle);
			}
		}
	}
	return 0;
}
