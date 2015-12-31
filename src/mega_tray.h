/*************************************************************************/
/* megatag - A simple library to tag files graphically                   */
/* Copyright (C) 2015                                                    */
/* Johannes Lorenz (jlsf2013 @ sourceforge)                              */
/*                                                                       */
/* This program is free software; you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation; either version 3 of the License, or (at */
/* your option) any later version.                                       */
/* This program is distributed in the hope that it will be useful, but   */
/* WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      */
/* General Public License for more details.                              */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program; if not, write to the Free Software           */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA  */
/*************************************************************************/

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
	QStringList word_list;
	QCompleter completer;
	QLineEdit tag_edit;

	QGridLayout grids[num_buttons];
	QPushButton buttons[num_buttons][8];

	QAction act_quit;

//	QAction act_about;

	void setup_ui();
	QStringList get_word_list();
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
