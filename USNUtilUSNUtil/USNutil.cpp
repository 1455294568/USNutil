#include "USNutil.h"

using namespace std;

Kevin::Util::USNutil::USNutil()
{
}

Kevin::Util::USNutil::~USNutil()
{
}

vector<string> Kevin::Util::USNutil::GetAllDrive()
{
	vector<string> driveList;
	char rootPath[10] = { 0 };
	for (char i = 'A'; i <= 'Z'; i++)
	{
		sprintf(rootPath, "%c:\\", i);
		auto type = GetDriveTypeA(rootPath);
		if (type != DRIVE_NO_ROOT_DIR)
		{
			stringstream ss;
			ss << i << ":\\";
			driveList.push_back(ss.str());
		}
	}
	return driveList;
}

bool Kevin::Util::USNutil::CheckIfNTFS(string diskName)
{
	char fileSystem[BUFFERLEN];
	GetVolumeInformationA(diskName.c_str(), NULL, 0, NULL, NULL, NULL, fileSystem, BUFFERLEN);
	if (0 == strcmp(fileSystem, "NTFS"))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Kevin::Util::USNutil::GetDriveHandle(std::string driveName, HANDLE& handle)
{
	stringstream ss;
	ss << "\\\\.\\" << driveName;
	string aa = ss.str();
	aa.erase(aa.length() - 1);
	handle = CreateFileA(aa.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
	if (handle != INVALID_HANDLE_VALUE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Kevin::Util::USNutil::InitUsn(HANDLE& handle)
{
	DWORD br;
	CREATE_USN_JOURNAL_DATA cujd;
	cujd.MaximumSize = 0;
	cujd.AllocationDelta = 0;
	return DeviceIoControl(handle, FSCTL_CREATE_USN_JOURNAL, &cujd, sizeof(cujd), NULL, 0, &br, NULL);
}

bool Kevin::Util::USNutil::GetUsnInfo(HANDLE& handle, USN_JOURNAL_DATA& usnInfo)
{
	DWORD br;
	return DeviceIoControl(handle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &usnInfo, sizeof(usnInfo), &br, NULL);
}

void Kevin::Util::USNutil::GetPathFromRefNum(HANDLE& handle, DWORDLONG refNum, string diskName, FilePathCallBack& callback)
{
	FILE_NAME_INFO* fileNameInfomation = (FILE_NAME_INFO*)malloc(sizeof(FILE_NAME_INFO) * BUFFERLEN);
	FILE_ID_DESCRIPTOR fid = { sizeof(fid), FileIdType };
	fid.FileId.QuadPart = refNum;

	HANDLE fileHandle = OpenFileById(handle, &fid, 0, FILE_SHARE_READ, 0, 0);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		cout << "failed to get file handle: error code: " << GetLastError() << endl;
	}

	if (!GetFileInformationByHandleEx(fileHandle, FileNameInfo, fileNameInfomation, BUFFERLEN))
	{
		auto err = GetLastError();
		//cout << "Get FileInfo failed, error code: " << err << endl;
	}
	else
	{
		diskName.erase(diskName.length() - 1);
		char fileName[BUFFERLEN];
		sprintf(fileName, "%s%.*S", diskName.c_str(), fileNameInfomation->FileNameLength / 2,
			fileNameInfomation->FileName);
		async(launch::async, [callback, fileName] {
			callback(fileName);
		});
	}
	free(fileNameInfomation);
	CloseHandle(fileHandle);
}

vector<DWORDLONG> Kevin::Util::USNutil::GetUsnData(HANDLE& handle, USN_JOURNAL_DATA usnInfo)
{
	vector<DWORDLONG> refNums;
	MFT_ENUM_DATA med;
	med.StartFileReferenceNumber = 0;
	med.LowUsn = usnInfo.FirstUsn;
	med.HighUsn = usnInfo.NextUsn;
	med.MaxMajorVersion = usnInfo.MaxSupportedMajorVersion;
	med.MinMajorVersion = usnInfo.MinSupportedMajorVersion;
	char buffer[MAXBUFFLEN];
	DWORD usnDataSize;
	PUSN_RECORD usnRecord;
	BOOL ret = DeviceIoControl(handle, FSCTL_ENUM_USN_DATA, &med, sizeof(med), buffer, MAXBUFFLEN, &usnDataSize, NULL);
	while (ret != 0)
	{
		//cout << "¿ªÊ¼..." << endl;
		DWORD retBytes = usnDataSize - sizeof(USN);
		usnRecord = (PUSN_RECORD)(((char*)buffer) + sizeof(USN));
		//cout << "********************" << endl;

		while (retBytes > 0)
		{
			const int strLen = usnRecord->FileNameLength;
			char fileName[BUFFERLEN] = { 0 };

			refNums.push_back(usnRecord->FileReferenceNumber);

			// Next record
			const DWORD recordLen = usnRecord->RecordLength;
			retBytes -= recordLen;
			usnRecord = (PUSN_RECORD)(((char*)usnRecord) + recordLen);
		}
		// next page 
		med.StartFileReferenceNumber = *(USN*)&buffer;
		ret = DeviceIoControl(handle, FSCTL_ENUM_USN_DATA, &med, sizeof(med), buffer, MAXBUFFLEN, &usnDataSize, NULL);
	}
	return refNums;
}
