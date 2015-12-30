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
#include <QSplitter>
#include <QGridLayout>
#include <QPushButton>
#include "megatag.h"

class mega_tray : public QSystemTrayIcon, megatag
{
	Q_OBJECT

	enum widget_types
	{
		information_widget,
		suggested_widget,
		recent_widget,
		popular_widget,
		num_widgets
	};

	enum button_types
	{
		suggested_buttons,
		recent_buttons,
		popular_buttons,
		num_buttons
	};

	QMenu tag_menu;
	QTimer next_tag;
	QWidgetAction* widget_actions[num_widgets];
	QWidget widgets[num_widgets];
	QVBoxLayout main_layout;
	QLabel information;
	std::map<std::string, std::size_t> id_of;
	QStringList word_list;
	QCompleter completer;
	QLineEdit tag_edit;

	QGridLayout grids[num_buttons];
	QPushButton buttons[num_buttons][8];

	QAction act_quit;

//	QAction act_about;

	void setup_ui();
	QStringList get_word_list();
	std::string get_current_tags();
	static void inc_tag_itr(std::string::const_iterator& itr);
	void get_file_id();
	void update_information_string();
	void on_update_keywords_for_file();
	void submit_tag(const QString& new_text);

private slots:
	void submit_tag_by_text_edit();
	void submit_tag_by_button();
	void tag();
	void close_app();
	void triggerTray(QSystemTrayIcon::ActivationReason reason);

public:
	mega_tray();
};

#endif // MEGA_TRAY_H
