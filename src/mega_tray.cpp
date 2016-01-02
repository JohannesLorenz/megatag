/*************************************************************************/
/* megatag - A simple library to tag files graphically                   */
/* Copyright (C) 2015-2016                                               */
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

#include <QApplication>
#include <QDebug>
#include <climits>
#include <iostream>
#include <libgen.h>
#include <QMessageBox>
#include "mega_tray.h"

//! this is used for Qt's timer
constexpr long double operator"" _ms ( unsigned long long duration ) {
	return duration;
}

void mega_tray::setup_ui()
{
	setToolTip("MegaTag - click to tag a file");

//	tray_menu.addAction(&act_about);
//	tray_menu.addSeparator();
//	tray_menu.addAction(&act_settings);
//	tray_menu.addSeparator();
	setContextMenu(&tag_menu);

	tag_edit.setCompleter(&completer);

	for(std::size_t i = 0; i < num_buttons; ++i) // TODO: enumerator?
	for(std::size_t j = 0; j < 8; ++j)
	{
		QPushButton& btn = buttons[i][j];
		btn.setFixedWidth(120);
		QObject::connect(&btn, SIGNAL(released()), this, SLOT(submit_tag_by_button()));
		grids[i].addWidget(&btn, j/4, j%4);
	}

	information[suggested_widget].setText("Recommended tags");
	information[recent_widget].setText("Recent tags");
	information[popular_widget].setText("Popular tags");

	for(std::size_t i = 0; i < num_widgets; ++i)
	{
		main_layouts[i].addWidget(information + i);
		widgets[i].setLayout(main_layouts + i);
		QWidgetAction* new_action = new QWidgetAction(&tag_menu);
		new_action->setDefaultWidget(widgets + i);
		tag_menu.addAction(new_action);
		tag_menu.addSeparator();
		widget_actions[i] = new_action;
		widget_actions[i]->setVisible(false);
	}

	main_layouts[information_widget].addWidget(&tag_edit);
	main_layouts[suggested_widget].addLayout(grids + suggested_buttons);
	main_layouts[recent_widget].addLayout(grids + recent_buttons);
	main_layouts[popular_widget].addLayout(grids + popular_buttons);

	tag_menu.addAction(&act_quit);
	tag_menu.setWindowIcon(QIcon("../../icon.png"));

	QObject::connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		this, SLOT(triggerTray(QSystemTrayIcon::ActivationReason)));
//	QObject::connect(&act_about, SIGNAL(triggered()), this, SLOT(showInfo()));
	QObject::connect(&act_quit, SIGNAL(triggered()), this, SLOT(close_app()));
//	QObject::connect(&act_settings, SIGNAL(triggered()), this, SLOT(options()));
	QObject::connect(&next_tag, SIGNAL(timeout()), this, SLOT(tag()));
	QObject::connect(&tag_edit, SIGNAL(editingFinished()), this, SLOT(submit_tag_by_text_edit()));
}

QStringList mega_tray::get_word_list()
{
	QStringList _word_list;

	const auto append_to_string_list =
		[&](char **argv) {
			_word_list.append(argv[1]);
			qDebug() << argv[1] << ", " << atoi(argv[0]);
			id_of.emplace(argv[1],atoi(argv[0]));
			qDebug() << argv[1] << ", " << atoi(argv[0]);
		};

	db.func0("SELECT id,name FROM ids ORDER BY name;", append_to_string_list);

	return _word_list;
}

void mega_tray::submit_tag_by_text_edit()
{
	QString new_text = tag_edit.text();
	if(new_text.length())
	{
		bool error = false;
		for(QChar c : new_text)
		{
			error = error || (!c.isLetterOrNumber() && c != '-' && c != '_' && c != ' ');
		}

		if(error)
		 QMessageBox::information(nullptr, "Invalid input", "Only allowed: alpha-numerics, or one of \" -_\"");
		else
		{
			new_text.replace('-', ' ');
			new_text.replace('_', ' ');
			new_text = new_text.trimmed();
			new_text.replace(' ', '-');

			QChar recent = new_text[1];
			QString new_text_2;
			for(QChar& c : new_text)
			{
				if(recent != c)
				 new_text_2.append(recent = c);
			}

			submit_tag(new_text_2);
		}
	}
}

void mega_tray::submit_tag_by_button()
{
	QPushButton* button = dynamic_cast<QPushButton*>(sender());
	const QString new_text = button->text();
	if(new_text.length())
	 submit_tag(new_text);
}

void mega_tray::submit_tag(const QString& new_text)
{
	int tags_id = -1;

	if(is_reachable_from_current(tags_id))
	{
		QMessageBox::information(nullptr, "Tag not added", "This tag is already implicated by other tags, not adding it");
		qDebug() << "...";
	}
	else
	{
		bool already_tagged = false, reachable = false;
		tags_id = get_tag_id(new_text.toStdString());
		if(tags_id == -1)
		{
			db.exec("INSERT INTO ids (id, name, timestamp, used_count) "
				"VALUES (NULL, '" + new_text.toStdString() + "', '"
				+ std::to_string(get_ftime()) + "', '1');");

			// re-read the list
			word_list = get_word_list();
			tags_id = get_tag_id(new_text.toStdString());
			// can not be already tagged since id was new
		}
		else
		{
			already_tagged = db.contains("SELECT * FROM tags WHERE file_id='" + std::to_string(file_id) +
				"' AND tag_id='" + std::to_string(tags_id) + "';");
			qDebug() << "already tagged?" << already_tagged;
			if(!already_tagged)
			{
				if(is_reachable_from_current(tags_id))
				 reachable = true;

				if(!reachable)
				{
					db.exec("UPDATE ids SET used_count = used_count + 1 WHERE id = '"
						+ std::to_string(tags_id) + "';");
					db.exec("UPDATE ids SET timestamp='" + std::to_string(get_ftime()) + "' WHERE id = '"
						+ std::to_string(tags_id) + "';");
				}
			}
		}

		if(!already_tagged && !reachable)
		{
			// FEATURE: genereal template class... argv[0]-find?
			if(file_id == -1)
			{
				const char* sep = "', '";
				db.exec("INSERT INTO files (id, path, last_changed, filetype, quality, md5sum) "
					"VALUES (NULL, '" + std::string(path) + sep
						+ std::to_string(get_ftime()) + sep
						+ "TODO', '0', 'TODO');");
				get_file_id();
			}

			db.exec("INSERT INTO tags (file_id, tag_id) "
				 "VALUES(" + std::to_string(file_id) + ", " + std::to_string(tags_id) + ");");

			std::set<std::size_t> s = are_reachable_from(tags_id);
			for(const std::size_t implicated : s)
			{
				// TODO: erase? (ERASE OR IGNORE?)
				db.exec("DELETE FROM tags WHERE file_id='" + std::to_string(file_id)
					+ "' AND tag_id = '" + std::to_string(implicated) + "';");
			}

			on_update_keywords_for_file();
		}

		tag_edit.clear();
	}
}

void mega_tray::tag()
{
	pid_t pid = get_xprop_pid();
	if(pid)
	{
		next_tag.stop(); // no need to re-run timer
		qDebug() << "PID: " << pid;

		std::string command;
		std::ifstream comm_f("/proc/" + std::to_string(pid) + "/comm");
		comm_f >> command;
		if(!command.compare(0, 7, "mplayer"))
		{
			using namespace std::literals::string_literals;

			get_input(("readlink /proc/" + std::to_string(pid) +
				"/fd/* | grep -v '\\(/dev/\\|pipe:\\|socket:\\)'").c_str());
			std::cin >> path;
			_basename = basename(path);
			if(!_basename)
			 throw std::runtime_error("Could not find basename of played file"s + path);
			++_basename;

			qDebug() << "playing: " << _basename;

			on_update_keywords_for_file();

			for(std::size_t i = 0; i<num_widgets; ++i)
			 widget_actions[i]->setVisible(true);

			/*
				try to pop up the window...
			*/
			tag_menu.setWindowFlags(Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint);
			QPoint pos = geometry().bottomLeft();
			qDebug() << "POS: " << pos;
			contextMenu()->move(pos);
			contextMenu()->show();
			/*QPoint pos = geometry().bottomLeft();
			tag_menu.popup(pos);
			tag_menu.show();*/
		}
	} else {
		qDebug() << "retrying...";
	}
}

void mega_tray::update_information_string()
{
	std::string info_str = "Select tags for ";
	info_str += _basename;

	file_id = -1;
	get_file_id();
	qDebug() << "file id:" << file_id;

	if(file_id != -1)
	{
		info_str += "<br/>Current tags:<b>";

		db.func0("SELECT ids.name"
			" FROM tags JOIN ids"
				" ON tags.tag_id = ids.id"
			" WHERE tags.file_id = " + std::to_string(file_id) +
			" ORDER BY name;",
			[&](char** arg) { (info_str += ' ') += arg[0]; } );

		info_str += "</b>";
	}

	information[information_widget].setText(QString::fromStdString(info_str));
}

void mega_tray::on_update_keywords_for_file()
{
	// word list is not necessarily updated
	update_information_string();

#define SELECT_UNUSED_ \
		"SELECT ids.name,ids.id,tags.tag_id" \
		" FROM ids LEFT OUTER JOIN tags ON (tags.tag_id = ids.id)" \
		" WHERE tags.tag_id is null OR tags.tag_id=''"

	std::size_t next_button = 0;
	db.func0(SELECT_UNUSED_
		" ORDER BY ids.timestamp DESC"
		" LIMIT 32;",
		[&](char** arg){
			if(next_button < 8 && !is_reachable_from_current(arg[1]))
			buttons[recent_buttons][next_button++].setText(arg[0]); });
	for(; next_button < 8; ++next_button)
	 buttons[recent_buttons][next_button].setText("");

	next_button=0;
	db.func0(SELECT_UNUSED_
		" ORDER BY ids.used_count DESC"
		" LIMIT 32;",
		[&](char** arg){
			if(next_button < 8 && !is_reachable_from_current(arg[1]))
			buttons[popular_buttons][next_button++].setText(arg[0]); });
	for(; next_button < 8; ++next_button)
	 buttons[popular_buttons][next_button].setText("");

	next_button=0;
	db.func0("SELECT ids.name,ids.id,tags.tag_id"
		" FROM ids"
		" LEFT OUTER JOIN tags ON (tags.tag_id = ids.id) "
		" JOIN keywords ON (ids.id = keywords.id) "
		" WHERE tags.tag_id is null OR tags.tag_id=''"
		// keyword is part of path:
		" AND '" + std::string(path) + "' like ('%' || keywords.keyword || '%') "
		" ORDER BY ids.name"
		" LIMIT 32;",
			[&](char** arg){
			if(next_button < 8 && !is_reachable_from_current(arg[1]))
			 buttons[suggested_buttons][next_button++].setText(arg[0]); }
			);
	for(; next_button < 8; ++next_button)
	 buttons[suggested_buttons][next_button].setText("");
#undef SELECT_UNUSED_
}

void mega_tray::close_app()
{
	QApplication::quit();
}

void mega_tray::triggerTray(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
		case QSystemTrayIcon::Trigger:
		case QSystemTrayIcon::DoubleClick:
		{
			// we need to un-grab the mouse, so that xprop can
			// access it -> pass this task to the timer
			next_tag.start(100_ms);
			break;
		}
		default:
			//tray_menu.exec(QCursor::pos());
			contextMenu()->exec(QCursor::pos());
			break;
	}
}

mega_tray::mega_tray() :
	QSystemTrayIcon(QIcon("../../icon.png")),
	word_list(get_word_list()),
	completer(word_list, &widgets[information_widget] /* TODO: really widget? */),
	act_quit("Quit", this)
{
	setup_ui();
}

