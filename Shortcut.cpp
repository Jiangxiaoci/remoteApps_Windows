#include "Shortcut.h"

bool createWindowsShortcut(const QString &filePath, const QString &shortcutPath)
{
    HRESULT hres;
    IShellLink* psl;

    // Initialize the COM library
    hres = CoInitialize(NULL);
    if (!SUCCEEDED(hres)) {
        return false;
    }

    // Create an IShellLink object and get a pointer to the IShellLink interface
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres)) {
        IPersistFile* ppf;

        // Set the path to the shortcut target
        psl->SetPath(filePath.toStdWString().c_str());
        // Set the description of the shortcut
        psl->SetDescription(L"Shortcut created by Qt");

        // Query IShellLink for the IPersistFile interface
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
        if (SUCCEEDED(hres)) {
            // Save the link by calling IPersistFile::Save
            hres = ppf->Save(shortcutPath.toStdWString().c_str(), TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    CoUninitialize();
    return SUCCEEDED(hres);
}
