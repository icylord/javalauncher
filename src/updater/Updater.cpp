#include "Updater.h"

using namespace std;

Updater::Updater(ConfigReader& config) {
    this->config = config;
    this->newVersion = "";
}

void Updater::createUpdateMutex() {
    updateMutex = CreateMutex(NULL, true, TEXT("DMDirc-Update"));
}

void Updater::waitForUpdaterMutex() {
    WaitForSingleObject(updateMutex, INFINITE);
}

void Updater::releaseUpdateMutex() {
    ReleaseMutex(updateMutex);
}

bool Updater::doUpdate(std::string directory) {
    Updater::createUpdateMutex();
    Updater::waitForUpdaterMutex();
    Updater::deleteOldLauncher();
    std::ifstream file ((char*) (directory + "/" + Utils::getExeName()).c_str());
    if (file.good()) {
        file.close();
        Updater::backupExistingLauncher();
        Updater::moveNewLauncher(directory + "/" + Utils::getExeName(), Utils::getExePathAndName());
        return true;
    }
    return false;
}

void Updater::moveNewLauncher(std::string oldName, std::string newName) {
    if (MoveFileEx((char*) oldName.c_str(), (char*) newName.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING) == 0) {
        perror("Unable to move new launcher");
    }
}

void Updater::relaunch() {
    Updater::createUpdateMutex();
    STARTUPINFO         sInfo;
    PROCESS_INFORMATION pInfo;
    ZeroMemory(&sInfo, sizeof(sInfo));
    sInfo.cb = sizeof(sInfo);
    ZeroMemory(&pInfo, sizeof(pInfo));
    CreateProcess((char*) Utils::getExePathAndName().c_str(),
        NULL, NULL, NULL, false, CREATE_NO_WINDOW, NULL, NULL, &sInfo, &pInfo);
    Updater::releaseUpdateMutex();
    ExitProcess(0);
}

void Updater::deleteOldLauncher() {
    std::string exeNameOld = Utils::getExePathAndName()+ ".old";
    std::ifstream file (exeNameOld.c_str());
    if (file.good()) {
        file.close();
        if (remove(exeNameOld.c_str()) != 0) {
            perror("Failed to delete the old launcher");
        }
    }
}

void Updater::backupExistingLauncher() {
    TCHAR buffer[MAX_PATH] = {0};
    DWORD bufSize = sizeof (buffer) / sizeof (*buffer);
    GetModuleFileName(NULL, buffer, bufSize);
    MoveFile(Utils::getExePathAndName().c_str(), (Utils::getExePathAndName() + ".old").c_str());
}

bool Updater::isUpdateWaiting() {
    return !newVersion.empty();
}

std::string Updater::getNewVersion() {
    return newVersion;
}
