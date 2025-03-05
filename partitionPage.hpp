#include "mainWindow.hpp"
#include <QTableWidget>
#include <kpmcore/core/partition.h>

class PartitionPage : public QWidget
{
Q_OBJECT
private:
    PageContent* page;
    QFormLayout* formLayout;
    QComboBox* deviceCombobox;
    QTableWidget* partitionTable;
    const int partitionRole = Qt::UserRole;
    bool hasInitialized = false;
    void showEvent(QShowEvent* event) override
    {
        QWidget::showEvent(event);
        initialize();
    }
    Partition* selectedPartition;

private slots:
    void onDeviceChanged(int index);
    void onPartitionItemChanged(const QTableWidgetItem* currentItem, const QTableWidgetItem* previousItem);
    
public: 
    PageContent* getPage()
    {
        return page;
    };
    
    PartitionPage(QWidget* parent);

    void initialize();


};
