#ifndef OBJECTSSCREEN_H
#define OBJECTSSCREEN_H

#include <QObject>
#include <QStringList>
#include <form.h>
#include <ncurses.h>

class ObjectListView;
class VeQItem;

class ObjectsScreen : public QObject
{
	Q_OBJECT
public:
	ObjectsScreen(VeQItem *root, QObject *parent = 0);

	virtual bool handleInput(int c);

signals:
	void goBack();

private:
	QString getEditValue() const;

	void startEdit(const QString &description, const QString &text);

	void endEdit();

	WINDOW *mTitleWindow;
	WINDOW *mListViewWindow;
	ObjectListView *mListView;
	WINDOW *mEditWindow;
	FIELD *mEditFields[2];
	FORM *mEditForm;
};

#endif // OBJECTSSCREEN_H