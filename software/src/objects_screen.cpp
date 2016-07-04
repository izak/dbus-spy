#include <velib/qt/ve_qitem.hpp>
#include "objects_screen.h"
#include "object_list_model.h"
#include "object_listview.h"

ObjectsScreen::ObjectsScreen(VeQItem *root, QObject *parent):
	QObject(parent),
	mTitleWindow(0),
	mListViewWindow(0),
	mListView(0),
	mEditWindow(0),
	mEditForm(0)
{
	mTitleWindow = newwin(1, getmaxx(stdscr), 0, 0);
	wmove(mTitleWindow, 0, 0);
	wattron(mTitleWindow, COLOR_PAIR(1));
	wprintw(mTitleWindow, root->id().toLatin1().data());
	wattroff(mTitleWindow, COLOR_PAIR(1));
	wrefresh(mTitleWindow);

	mListViewWindow = newwin(getmaxy(stdscr) - 2, getmaxx(stdscr), 1, 0);
	keypad(mListViewWindow, true);
	ObjectListModel *model = new ObjectListModel(root, true, this);
	mListView = new ObjectListView(model, mListViewWindow, this);
	refresh();
}

bool ObjectsScreen::handleInput(int c)
{
	if (mEditForm == 0)
	{
		switch (c)
		{
		case KEY_LEFT:
			emit goBack();
			return true;
		case KEY_ENTER:
		case '\n':
		{
			startEdit("Edit value: ", mListView->getValue(mListView->getSelection()));
			refresh();
			return true;
		}
		case 't':
			mListView->setShowText(!mListView->showText());
			refresh();
			return true;
		default:
			return mListView->handleInput(c);
		}
	}
	else
	{
		switch (c)
		{
		case KEY_LEFT:
			c = REQ_PREV_CHAR;
			break;
		case KEY_RIGHT:
			c = REQ_NEXT_CHAR;
			break;
		case KEY_HOME:
			c = REQ_BEG_LINE;
			break;
		case KEY_END:
			c = REQ_END_LINE;
			refresh();
			break;
		case KEY_BACKSPACE:
			c = REQ_DEL_PREV;
			break;
		case 0x014A: // delete key?
			c = REQ_DEL_CHAR;
			break;
		case KEY_ENTER:
		case '\n':
		{
			QVariant qv;
			QString v = getEditValue();
			bool ok = false;
			int i = v.toLong(&ok);
			if (ok) {
				qv = i;
			} else {
				double d = v.toDouble(&ok);
				if (ok) {
					qv = d;
				} else {
					qv = v;
				}
			}
			mListView->setValue(mListView->getSelection(), qv);
			endEdit();
			refresh();
			return true;
		}
		case 0x1B: // escape
			endEdit();
			refresh();
			return true;
		default:
			break;
		}
		if (c != 0) {
			form_driver(mEditForm, c);
			wrefresh(mEditWindow);
		}
		return true;
	}
}

QString ObjectsScreen::getEditValue() const
{
	if (mEditForm == 0)
		return QString();
	form_driver(mEditForm, REQ_VALIDATION);
	QString r = field_buffer(mEditFields[0], 0);
	return r.trimmed();
}

void ObjectsScreen::startEdit(const QString &description, const QString &text)
{
	if (mEditForm != 0)
		return;
	int y = getmaxy(stdscr) - 1;
	int x0 = description.size();
	mEditFields[0] = new_field(1, getmaxx(stdscr) - x0, 0, 0, 0, 0);
	mEditFields[1] = 0;
	set_field_buffer(mEditFields[0], 0, text.toLatin1().data());
	mEditForm = new_form(mEditFields);
	int rows = 0;
	int cols = 0;
	scale_form(mEditForm, &rows, &cols);
	mEditWindow = newwin(rows, cols, y, x0);
	keypad(mEditWindow, true);
	set_form_win(mEditForm, mEditWindow);
	set_form_sub(mEditForm, derwin(mEditWindow, rows, cols, 0, 0));
	post_form(mEditForm);
	wrefresh(mEditWindow);
	set_current_field(mEditForm, mEditFields[0]);
	curs_set(1);

	mvprintw(y, 0, description.toLatin1().data());
	wmove(mEditWindow, 0, 0);
}

void ObjectsScreen::endEdit()
{
	if (mEditForm == 0)
		return;
	curs_set(0);
	int y = getmaxy(stdscr) - 1;
	move(y, 0);
	clrtoeol();
	unpost_form(mEditForm);
	delwin(mEditWindow);
	free_form(mEditForm);
	free_field(mEditFields[0]);
	mEditForm = 0;
}