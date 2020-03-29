#pragma once
#include "define.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <future>
#include "CType.h"

namespace Kevin
{
	namespace Util {

		class USNutil
		{
		public:
			USNutil();
			~USNutil();
			std::vector<std::string> GetAllDrive();
			bool CheckIfNTFS(std::string diskName);
			bool GetDriveHandle(std::string driveName, HANDLE& handle);
			bool InitUsn(HANDLE& handle);
			bool GetUsnInfo(HANDLE& handle, USN_JOURNAL_DATA& usnInfo);
			void GetPathFromRefNum(HANDLE& handle, DWORDLONG refNum, std::string diskName, FilePathCallBack& callback);
			std::vector<DWORDLONG> GetUsnData(HANDLE& handle, USN_JOURNAL_DATA usnInfo);
		private:
		};

	}
}