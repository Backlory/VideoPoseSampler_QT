#ifndef SELECTRESOLUTION_H
#define SELECTRESOLUTION_H

#include <QDialog>

namespace Ui {
class selectResolution;
}

class selectResolution : public QDialog
{
    Q_OBJECT

public:
    explicit selectResolution(QWidget *parent = nullptr);
    ~selectResolution();
    int getResolution(int &width, int &height);
    
private:
    Ui::selectResolution *ui;
};

#endif // SELECTRESOLUTION_H
