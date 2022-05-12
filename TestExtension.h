// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the TESTEXTENSION_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// TESTEXTENSION_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef TESTEXTENSION_EXPORTS
#define TESTEXTENSION_API __declspec(dllexport)
#else
#define TESTEXTENSION_API __declspec(dllimport)
#endif

// This class is exported from the dll
class TESTEXTENSION_API CTestExtension {
public:
	CTestExtension(void);
	// TODO: add your methods here.
};

extern TESTEXTENSION_API int nTestExtension;

TESTEXTENSION_API int fnTestExtension(void);
