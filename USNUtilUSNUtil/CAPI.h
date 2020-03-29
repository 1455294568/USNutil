#pragma once

#include "Cdef.h"
#include "CType.h"


KEVIN_API int KEVIN_CALLBACK GetAllDrives(RetStrArr* retA);

KEVIN_API int KEVIN_CALLBACK EnumAllNTFSFileName(FilePathCallBack& callback);
