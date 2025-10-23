#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsScene>
#include <QPen>
#include <QBrush>
#include <QKeyEvent>
#include <QDebug>
#include <cstdlib>
#include <ctime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- Create scene ---
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 760, 460);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

    // --- Draw grid ---

    QPen gridPen(Qt::lightGray);
    gridPen.setWidth(0);
    gridStep = 20;
    SCENE_WIDTH = scene->width();
    SCENE_HEIGHT = scene->height();
    offsetX = SCENE_WIDTH % gridStep / 2;
    offsetY = SCENE_HEIGHT % gridStep / 2;

    COLS = SCENE_WIDTH / gridStep;
    ROWS = SCENE_HEIGHT / gridStep;
    generateMaze();
    drawMaze();

    // --- Border ---
    scene->addRect(0, 0, SCENE_WIDTH, SCENE_HEIGHT, QPen(Qt::black));

    // --- Movable block ---
    blockSize = gridStep;
    pos = QPointF(1, 1);

    block = scene->addRect(offsetX + pos.x()*gridStep, offsetY + pos.y()*gridStep, blockSize, blockSize, QPen(Qt::NoPen), QBrush(Qt::red));
    // setPlayerPos();
    block->setFlag(QGraphicsItem::ItemIsFocusable, true);
    block->setFocus();
    initGame();

    // Timer
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::moveEntities);
    gameTimer->start(tickMs);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    Direction d = DirNone;
    switch (event->key()) {
    case Qt::Key_Left: case Qt::Key_A: d = DirLeft; break;
    case Qt::Key_Right: case Qt::Key_D: d = DirRight; break;
    case Qt::Key_Up: case Qt::Key_W: d = DirUp; break;
    case Qt::Key_Down: case Qt::Key_S: d = DirDown; break;
    default: QMainWindow::keyPressEvent(event); return;
    }
    desiredDir = d;
}

void MainWindow::regenMage() {
    pos = QPointF(1, 1);
    scene->clear();
    block = scene->addRect(offsetX + pos.x()*gridStep, offsetY + pos.y()*gridStep, blockSize, blockSize, QPen(Qt::NoPen), QBrush(Qt::red));
    // setPlayerPos();
    block->setFlag(QGraphicsItem::ItemIsFocusable, true);
    block->setFocus();
    setPlayerPos();
    generateMaze();
    drawMaze();
    initGame();
}


// --- Page navigation ---
void MainWindow::on_nextButton_clicked() {
    ui->stackedWidget->setCurrentIndex(1);
    pos = QPointF(1, 1);
    setPlayerPos();
}

void MainWindow::on_backButton_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_regenMaze_clicked() {
    regenMage();
}


