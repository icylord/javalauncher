#include "jvmlauncher/JVMLauncher.h"
#include "singleinstance/SingleInstance.h"
#include "config/ConfigReader.h"
#include "config/ConfigDefaults.h"
#include "updater/Updater.h"
#include "utils/utils.h"
#include "version.h"
#include <sstream>
#include <Shlobj.h>
#include <windows.h>
#include <conio.h>

using namespace std;

int main(int argc, char** argv) {
	if (argc >= 2 && argv[1] == std::string("--LAUNCHER_VERSION")) {
		cout << LAUNCHER_VERSION << endl;
		exit(0);
	}
	std::ifstream file((char*)(Utils::GetAppDataDirectory() + Utils::getExeName()).c_str());
	std::string version = "-1";
	if (file.good()) {
		file.close();
		version = Utils::launchAppReturnOutput(Utils::GetAppDataDirectory() + Utils::getExeName());
	}
	Utils::disableFolderVirtualisation();
	ConfigReader config;
	SingleInstance singleInstance(config);
	if (!singleInstance.getCanStart()) {
		cout << "Another instance already running." << endl;
		return EXIT_FAILURE;
	}
	Updater updater(config);
	try {
		
		JVMLauncher* launcher = new JVMLauncher(Utils::arrayToVector(argc, argv), config);
		launcher->LaunchJVM();
		std::string directory = launcher->callGetDirectory();
		int compareValue = launcher->callIsNewer(version, LAUNCHER_VERSION);
		if (compareValue > 0) {
			std::string commandLine = Utils::getExeName() + " ";
			ShellExecute(NULL, LPSTR("open"), LPSTR((Utils::GetAppDataDirectory() + Utils::getExeName()).c_str()), LPSTR(argv), NULL, SW_SHOWNORMAL);
			exit(0);
		}
		if (updater.doUpdate(directory)) {
			launcher->destroyJVM();
			updater.relaunch();
		}
		launcher->callMainMethod();
		launcher->destroyJVM();
	}
	catch (JVMLauncherException& ex) {
		cout << "Launching the JVM failed: ";
		cout << ex.what() << endl;
		cout << "Press any key to exit" << endl;
		_getch();
	}
	cout << "Tidying up and exiting." << endl;
	singleInstance.stopped();
	return EXIT_SUCCESS;
}
