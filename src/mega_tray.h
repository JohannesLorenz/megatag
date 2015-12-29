#ifndef MEGA_TRAY_H
#define MEGA_TRAY_H

#include <QMenu>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QWidgetAction>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCompleter>
#include "megatag.h"

class mega_tray : public QSystemTrayIcon, megatag
{
	Q_OBJECT
	QMenu tray_menu;
	QMenu tag_menu;
	QAction act_quit;
	QTimer next_tag;
	QWidgetAction widget_action;
	QWidget widget;
	QVBoxLayout main_layout;
	QLabel information;
	std::map<std::string, std::size_t> id_of;
	QStringList word_list;
	QCompleter completer;
	QLineEdit tag_edit;

	char path[PATH_MAX];
	const char* _basename;

//	QAction act_about;

	void setup_ui();
	QStringList get_word_list();

private slots:
	void submitTag();
	void tag();
	void close_app();
	void triggerTray(QSystemTrayIcon::ActivationReason reason);

public:
	mega_tray();
};

#endif // MEGA_TRAY_H
