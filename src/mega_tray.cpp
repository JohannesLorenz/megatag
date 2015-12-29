#include <QApplication>
#include <QDebug>
#include <climits>
#include <iostream>
#include <libgen.h>
#include <sys/stat.h>
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
	tray_menu.addAction(&act_quit);
	setContextMenu(&tray_menu);

	main_layout.addWidget(&information);

	tag_edit.setCompleter(&completer);
	main_layout.addWidget(&tag_edit);

	widget.setLayout(&main_layout);
	widget_action.setDefaultWidget(&widget);
	tag_menu.addAction(&widget_action);
	tag_menu.addSeparator();
	tag_menu.addAction(&act_quit);
	tag_menu.setWindowIcon(QIcon("../../icon.png"));

	QObject::connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		this, SLOT(triggerTray(QSystemTrayIcon::ActivationReason)));
//	QObject::connect(&act_about, SIGNAL(triggered()), this, SLOT(showInfo()));
	QObject::connect(&act_quit, SIGNAL(triggered()), this, SLOT(close_app()));
	//	QObject::connect(&act_settings, SIGNAL(triggered()), this, SLOT(options()));
	QObject::connect(&next_tag, SIGNAL(timeout()), this, SLOT(tag()));
	QObject::connect(&tag_edit, SIGNAL(editingFinished()), this, SLOT(submitTag()));
}

QStringList mega_tray::get_word_list()
{
	QStringList _word_list;

	const auto append_to_string_list =
		[&](int, char **argv, char **) {
			_word_list.append(argv[1]);
			qDebug() << argv[1] << ", " << atoi(argv[0]);
			id_of.emplace(argv[1],atoi(argv[0]));
			qDebug() << argv[1] << ", " << atoi(argv[0]);
			return true;
		};

	db.func("SELECT id,name FROM ids ORDER BY name;", append_to_string_list);

	return _word_list;
}

void mega_tray::submitTag()
{
	const QString new_text = tag_edit.text();
	if(new_text.length())
	{
		std::size_t tags_id;
		const auto itr = id_of.find(new_text.toStdString());

		if(itr == id_of.end())
		{
			functor_print f;

			db.exec("INSERT INTO ids (id, name) "
				"VALUES (NULL, '" + new_text.toStdString() + "');", f);

			// re-read the list (TODO? https://www.sqlite.org/autoinc.html)
			word_list = get_word_list();

			const auto get_new_id =
				[&](int, char **argv, char **)
				{
					return tags_id = atoi(argv[0]), true;
				};

			db.func("SELECT id FROM ids WHERE name='" + new_text.toStdString() + "';",
				get_new_id);
		}
		else
		{
			tags_id = itr->second;
		}

		bool found = false; // TODO: genereal template class... argv[0]-find?
		const auto find_file =
			[&](int, char **argv, char **)
			{
				found = found || (!strcmp(argv[0], path));
				return true;
			};
		db.func("SELECT path FROM files ORDER BY path;", find_file);

		if(found)
		{
			std::string tags;
			const auto get_csv =
				[&](int, char **argv, char **)
				{
					return tags = argv[0], true;
				};

			db.func("SELECT tags FROM files WHERE path='" + std::string(path) + "';",
				get_csv);

			std::string::iterator itr = tags.begin();
			const auto inc_itr = [&]() {
				for(; isdigit(*itr); ++itr) ;
				for(; *itr == ' ' || *itr == ','; ++itr) ;
			};

			for(; itr != tags.end(); inc_itr())
			{
				qDebug() << &*itr << ", id" << tags_id;
				if(itr == tags.end() || (std::size_t)atoi(&*itr) >= tags_id)
				 break;
			}

			if(itr == tags.end() || (std::size_t)atoi(&*itr) != tags_id)
			{
				tags.insert(std::distance(tags.begin(), itr), std::to_string(tags_id) + ", ");

				db.exec("UPDATE files"
					" SET tags='" + tags +
					"' WHERE path='" + std::string(path) + "';");
			}
		}
		else
		{
			struct stat attrib;
			stat(path, &attrib);
			time_t the_time = attrib.st_mtime;

			functor_print p;
			const char* sep = "', '";
			db.exec("INSERT INTO files (id, path, tags, last_changed, filetype, quality, md5sum) "
				"VALUES (NULL, '" + std::string(path) + sep
					+ std::to_string(tags_id) + ", " + sep
					+ std::to_string(the_time) + sep
					+ "TODO', '0', 'TODO');"
					, p);
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

		get_input(("readlink /proc/" + std::to_string(pid) +
			"/fd/* | grep -v '\\(/dev/\\|pipe:\\|socket:\\)'").c_str());
		//get_input("readlink /proc/$(pidof mplayer)/fd/* | grep -v '\\(/dev/\\|pipe:\\|socket:\\)'");
		std::cin >> path;
		_basename = basename(path);
		if(!_basename)
		 throw std::runtime_error(std::string("Could not find basename of played file") + path);
		++_basename;

		qDebug() << "playing: " << _basename;

		information.setText("Select tags for " + QString(_basename));

		setContextMenu(&tag_menu);
		contextMenu()->setWindowFlags(Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint);
		QPoint pos = geometry().bottomLeft();
		qDebug() << "POS: " << pos;
		contextMenu()->move(pos);
		contextMenu()->show();
	} else {
		qDebug() << "retrying...";
	}
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
	act_quit("Quit", this),
	widget_action(&tray_menu),
	word_list(get_word_list()),
	completer(word_list, &widget /* TODO: really widget? */)
{
	setup_ui();
}

