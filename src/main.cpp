#include <QApplication>
#include <QSystemTrayIcon>

#include <cstdlib>
#include <iostream>

#include "mega_tray.h"

class MyApplication : public QApplication
{
	using QApplication::QApplication;
	bool notify(QObject *receiver_, QEvent *event_)
	{
		try
		{
			return QApplication::notify(receiver_, event_);
		} catch(std::exception const& e) {
			std::cerr << e.what() << std::endl;
			return EXIT_FAILURE;
		} catch(...) {
			std::cerr << "caught unknown exception!" << std::endl;
			return EXIT_FAILURE;
		}
		return false;
	}

};

int run(MyApplication& app, int , char**)
{
	// TODO: move to ctor?
	MyApplication::setQuitOnLastWindowClosed(false);

	/*QSystemTrayIcon icon(QIcon("../../icon.png"));
	icon.setToolTip("MegaTag - click to tag a file");
	icon.showMessage("Test", "Message", QSystemTrayIcon::Information, 10000);
	icon.show();*/
	mega_tray icon;
	icon.show();

	return app.exec();
}

int main(int argc, char** argv)
{
	MyApplication app(argc, argv);
	/*app.setOrganizationName("megatag");
	app.setOrganizationDomain("github.com/JohannesLorenz/megatag");
	app.setApplicationName("megatag");*/

	try
	{
		run(app, argc, argv);
	} catch(std::exception const& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	} catch(...) {
		std::cerr << "caught unknown exception!" << std::endl;
		return EXIT_FAILURE;
	}
	std::cerr << "Success!" << std::endl;
	return EXIT_SUCCESS;
}
