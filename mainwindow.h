#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QTimer timer;       // temporizador

    double niveltank;   // nivel tanque principal
    double nivelA;      // nivel tanque auxiliar A
    double nivelB;      // nivel tanque auxiliar B

public slots:    // categoria de las funciones de la clase que sirven para ser asociadas a un botón
    void act_entrada_tank(int val);
    void act_salida_tank(int val);
    void act_caudalA(int val);
    void act_caudalB(int val);
    void act_niveles();

};

#endif // MAINWINDOW_H

