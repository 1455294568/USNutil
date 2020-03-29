#include "define.h"
#include <iostream>
#include "CAPI.h"

using namespace std;

void PrintFile(const char* fileName)
{
	cout << fileName << endl;
}

FilePathCallBack callback(PrintFile);

int main()
{
	while (true)
	{
		EnumAllNTFSFileName(callback);
		getchar();
	}
	return 0;
}