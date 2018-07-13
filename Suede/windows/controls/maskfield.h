#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>

class MaskField : public QComboBox {
	Q_OBJECT

public:
	MaskField(QWidget* parent);

public:
	template <class T>
	void setEnums(T selected);
	void setItems(const QStringList& items, int mask);

signals:
	void selectionChanged(int mask);

private slots:
	void onSelectedChanged(int state);

private:
	void updateCheckBoxes();

	void setSelectedMask(int value);
	void addSelectedMask(int index);
	void removeSelectedMask(int index);

	int addItem(const QString& str);
	void updateEverythingMask(int size);
	
	void updateText();
	uint count1Bits(uint x, int& p);

	void clearAll();
	enum {
		MaxItems = sizeof(int) * 8,
	};

private:
	int selected_;
	int everything_;

	QListWidget* view_;
	QList<QCheckBox*> checkBoxes_;
};

template <class T>
void MaskField::setEnums(T selected) {
	QStringList list;

	// skip None & Everything.
	for (int i = 1; i < T::size() - 1; ++i) {
		list << T::value(i).to_string();
	}

	setItems(list, selected);
}
