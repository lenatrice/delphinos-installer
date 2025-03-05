#include "mainWindow.hpp"

class PartitionPage : public QWidget
{
Q_OBJECT
private:
    PageContent* page;
    QFormLayout* formLayout;

public: 
    PageContent* getPage()
    {
        return page;
    };

    PartitionPage(QWidget* parent);
};
