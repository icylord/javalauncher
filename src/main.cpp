#include "jvmlauncher/JVMLauncher.h"
#include "singleinstance/SingleInstance.h"
#include "config/ConfigReader.h"
#include "config/ConfigDefaults.h"
#include "updater/Updater.h"
#include "utils/utils.h"
#include <sstream>
#include <Shlobj.h>
#include <windows.h>
#include <conio.h>

using namespace std;

vector<string> getCliArgs(int argc, char** argv, ConfigReader& config, Updater& updater) {
    vector<string> cliArgs = Utils::arrayToVector(argc, argv);
    cliArgs.erase(cliArgs.begin());
    cliArgs.push_back("-l");
    cliArgs.push_back("bob-1");
    cliArgs = Utils::mergeVectors(config.getVectorValue("application.args", vector<string>(0)),cliArgs);
    return cliArgs;
}

vector<string> getJvmArgs(ConfigReader config) {
    vector<string> jvmArgs;
    jvmArgs.push_back("-Dfile.encoding=utf-8");
    jvmArgs = Utils::mergeVectors(config.getVectorValue("jvm.args", vector<string>(0)), jvmArgs);
    return jvmArgs;
}

int main(int argc, char** argv) {
	PWSTR wChar;
	SHGetKnownFolderPath(FOLDERID_UserProgramFiles, 0, NULL, &wChar);
	std::wstring wpath(wChar);
	std::string path = Utils::ws2s(wpath);
	CoTaskMemFree(static_cast<LPVOID>(wChar));
    Utils::disableFolderVirtualisation();
    ConfigReader config;
    SingleInstance singleInstance(config);
    if (!singleInstance.getCanStart()) {
        cout << "Another instance already running." << endl;
        return EXIT_FAILURE;
    }
    Updater updater (config);
	try {
	    JVMLauncher* launcher = new JVMLauncher(
			config.getStringValue("application.path", APPLICATION_PATH),
			config.getStringValue("application.main", APPLICATION_MAIN),
	        getJvmArgs(config), getCliArgs(argc, argv, config, updater), config);
        launcher->LaunchJVM();
        launcher->callLauncherUtils();
        if (updater.doUpdate(JVMLauncher::getDirectory())) {
            launcher->destroyJVM();
            updater.relaunch();
        }
        launcher->callMainMethod();
        launcher->destroyJVM();
    } catch (JVMLauncherException& ex) {
        cout << "Launching the JVM failed" << endl;
        cout << ex.what() << endl;
        cout << "Press any key to exit" << endl;
        _getch();
    }
    singleInstance.stopped();
    return EXIT_SUCCESS;
}