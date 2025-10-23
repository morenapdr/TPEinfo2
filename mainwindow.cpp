#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //conecto los diales con las funciones de cambio de caudal
    connect(ui->dial1,SIGNAL(valueChanged(int)),this, SLOT(act_entrada_tank(int)));
    connect(ui->dial2,SIGNAL(valueChanged(int)),this, SLOT(act_salida_tank(int)));
    connect(ui->dialA,SIGNAL(valueChanged(int)),this, SLOT(act_caudalA(int)));
    connect(ui->dialB,SIGNAL(valueChanged(int)),this, SLOT(act_caudalB(int)));

    //conecto el temporizador a la funcion de actualizar niveles en los tanques
    connect(&timer, SIGNAL(timeout()), this, SLOT(act_niveles()));
    timer.start(1000);

    //inicializo los niveles en 0
    niveltank=0;
    nivelA=0;
    nivelB=0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::act_entrada_tank(int val)
{
    double caudalMax=ui->caudal1->text().toDouble();
    double caudalActual=(val / 100.0) * caudalMax;
    ui->number1->display(caudalActual);
}

void MainWindow::act_salida_tank(int val)
{
    double caudalMax=ui->caudal2->text().toDouble();
    double caudalActual=(val / 100.0) * caudalMax;
    ui->number2->display(caudalActual);
}

void MainWindow::act_caudalA(int val)
{
    double caudalMax=ui->caudalA->text().toDouble();
    double caudalActual=(val / 100.0) * caudalMax;
    ui->numberA->display(caudalActual);
}

void MainWindow::act_caudalB(int val)
{
    double caudalMax=ui->caudalB->text().toDouble();
    double caudalActual=(val / 100.0) * caudalMax;
    ui->numberB->display(caudalActual);
}

void MainWindow::act_niveles()
{
    // leo capacidades maximas
    double capacidad_tank=ui->cap1->text().toDouble();
    double capacidadA=ui->capA->text().toDouble();
    double capacidadB=ui->capB->text().toDouble();

    // leo caudales max
    double caudalMaxEntrada=ui->caudal1->text().toDouble();
    double caudalMaxSalida=ui->caudal2->text().toDouble();
    double caudalMaxA=ui->caudalA->text().toDouble();
    double caudalMaxB=ui->caudalB->text().toDouble();

    // Leer porcentajes de diales
    double porcEntrada=ui->dial1->value();
    double porcSalida=ui->dial2->value();
    double porcA=ui->dialA->value();
    double porcB=ui->dialB->value();

    // inicializo en 0
    double qin=0, qout=0, qA_out=0, qB_out=0;

    // estados de los enables
    bool habilitarA=ui->enableA->isChecked();
    bool habilitarB=ui->enableB->isChecked();

    // niveles
    bool tanqueA_lleno=nivelA>=capacidadA;
    bool tanqueB_lleno=nivelB>=capacidadB;
    bool tanqueA_vacio=nivelA<0.1*capacidadA;
    bool tanqueB_vacio=nivelB<0.1*capacidadB;
    bool tank1_lleno=niveltank>=capacidad_tank;
    bool tank1_vacio=niveltank<0.1 *capacidad_tank;

    //calculo el caudal de entrada
    if(!tank1_lleno)
        qin=(porcEntrada /100.0)* caudalMaxEntrada;
    else{
        qin=0;
        ui->dial1->setValue(0);
        ui->number1->display(0);
        ui->dial1->setEnabled(false);
    }

    //calculo caudal de salida
    bool salidaPermitida=!tank1_vacio&&
                           (habilitarA || habilitarB) &&
                           (!tanqueA_lleno || !tanqueB_lleno);

    if(salidaPermitida)
        qout=(porcSalida/100.0)*caudalMaxSalida;
    else{
        qout=0;
        ui->dial2->setValue(0);
        ui->number2->display(0);
        ui->dial2->setEnabled(false);
    }

    //calculo salidas de a y b
    if(!tanqueA_vacio)
        qA_out=(porcA/100.0)* caudalMaxA;
    else{
        qA_out=0;
        ui->dialA->setValue(0);
        ui->numberA->display(0);
        ui->dialA->setEnabled(false);
    }

    if(!tanqueB_vacio)
        qB_out=(porcB/100.0)* caudalMaxB;
    else{
        qB_out=0;
        ui->dialB->setValue(0);
        ui->numberB->display(0);
        ui->dialB->setEnabled(false);
    }

    // distribuyo el caudal principal

    double qA_in=0, qB_in=0;

    double espacioA=capacidadA - nivelA;
    double espacioB=capacidadB - nivelB;

    if(habilitarA && !tanqueA_lleno && habilitarB && !tanqueB_lleno) {
        double mitad = qout / 2.0;
        qA_in=std::min(mitad, espacioA * 3600.0); //std::min devuelve el menor
        qB_in=std::min(mitad, espacioB * 3600.0);
    }else if(habilitarA && !tanqueA_lleno) {
        qA_in=std::min(qout, espacioA * 3600.0);
    }else if(habilitarB && !tanqueB_lleno) {
        qB_in=std::min(qout, espacioB * 3600.0);
    }

    //actualizo los niveles
    niveltank+=(qin - qout) /3600.0;
    nivelA+= (qA_in - qA_out) /3600.0;
    nivelB+= (qB_in - qB_out) /3600.0;

    // limito los niveles
    niveltank= std::clamp(niveltank,0.0, capacidad_tank); //std:: clamp limita niveltank entre 0 y capacidad del tanque
    nivelA= std::clamp(nivelA,0.0, capacidadA);
    nivelB= std::clamp(nivelB,0.0, capacidadB);

    // actualizo interfaz
    ui->tank1->setValue((niveltank / capacidad_tank) * 100.0);
    ui->tankA->setValue((nivelA / capacidadA) * 100.0);
    ui->tankB->setValue((nivelB / capacidadB) * 100.0);

    ui->number1->display(qin);
    ui->number2->display(qout);
    ui->numberA->display(qA_out);
    ui->numberB->display(qB_out);

    //habilito los diales si las condiciones se cumplen
    if(!tank1_lleno && caudalMaxEntrada > 0)
        ui->dial1->setEnabled(true);

    if(salidaPermitida && caudalMaxSalida > 0)
        ui->dial2->setEnabled(true);

    if(!tanqueA_vacio && caudalMaxA > 0)
        ui->dialA->setEnabled(true);

    if(!tanqueB_vacio && caudalMaxB > 0)
        ui->dialB->setEnabled(true);

 //para chequear resultados
    qDebug() << "Nivel cisterna:" << niveltank
             << " Nivel A:" << nivelA
             << " Nivel B:" << nivelB;
}





















