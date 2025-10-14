#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsScene>
#include <QPen>
#include <QBrush>
#include <QKeyEvent>
#include <QDebug>
#include <cstdlib>
#include <ctime>
#include <stack>
#include <algorithm>
#include <random>

void MainWindow::drawMaze()
{
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            int x = j * gridStep + offsetX;
            int y = i * gridStep + offsetY;

            if (maze[i][j] == 1)
                scene->addRect(x, y, gridStep, gridStep, QPen(Qt::gray), QBrush(Qt::darkGray));
            else
                scene->addRect(x, y, gridStep, gridStep, QPen(Qt::lightGray));
        }
    }
    // Draw start and end markers
    scene->addRect(offsetX + 1 * gridStep,offsetY + 1 * gridStep, gridStep, gridStep, QPen(Qt::green), QBrush(Qt::green)); // Start
    scene->addRect(offsetX + (COLS - 2) * gridStep,offsetY + (ROWS - 2) * gridStep, gridStep, gridStep, QPen(Qt::blue), QBrush(Qt::blue)); // Goal

    scene->addRect(0, 0, SCENE_WIDTH, SCENE_HEIGHT, QPen(Qt::black));
}

void MainWindow::generateMaze()
{
    // Initialize all as walls
    maze = QVector<QVector<int>>(ROWS, QVector<int>(COLS, 1));

    // Utility lambda for bounds check
    auto inBounds = [&](int r, int c) {
        return (r > 0 && c > 0 && r < ROWS - 1 && c < COLS - 1);
    };

    // Movement directions (Up, Down, Left, Right)
    QVector<QPoint> directions = {
        {0, -2}, {0, 2}, {-2, 0}, {2, 0}
    };

    // Stack for backtracking
    std::stack<QPoint> st;

    // Start point (must be odd indices)
    int startRow = 1;
    int startCol = 1;
    maze[startRow][startCol] = 0;
    st.push(QPoint(startCol, startRow));

    std::random_device rd;
    std::mt19937 gen(rd());

    while (!st.empty()) {
        QPoint current = st.top();
        int cx = current.x();
        int cy = current.y();

        // Shuffle directions for randomness
        std::shuffle(directions.begin(), directions.end(), gen);

        bool moved = false;
        for (const QPoint &dir : directions) {
            int nx = cx + dir.x();
            int ny = cy + dir.y();

            if (inBounds(ny, nx) && maze[ny][nx] == 1) {
                // Carve path
                maze[ny][nx] = 0;
                maze[cy + dir.y() / 2][cx + dir.x() / 2] = 0;
                st.push(QPoint(nx, ny));
                moved = true;
                break;
            }
        }

        if (!moved)
            st.pop(); // Backtrack
    }

    // Ensure start and end are clear
    maze[1][1] = 0;
    maze[ROWS - 2][COLS - 2] = 0;
}


void MainWindow::setPlayerPos() {
    block->setRect(offsetX + pos.x() * gridStep, offsetY + pos.y() * gridStep, blockSize, blockSize);
}

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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    int newRow = pos.y();
    int newCol = pos.x();

    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_A:
        if (newCol > 0 && maze[newRow][newCol - 1] == 0)
            pos.setX(pos.x() - 1);
        break;

    case Qt::Key_Right:
    case Qt::Key_D:
        if (newCol < COLS - 1 && maze[newRow][newCol + 1] == 0)
            pos.setX(pos.x() + 1);
        break;

    case Qt::Key_Up:
    case Qt::Key_W:
        if (newRow > 0 && maze[newRow - 1][newCol] == 0)
            pos.setY(pos.y() - 1);
        break;

    case Qt::Key_Down:
    case Qt::Key_S:
        if (newRow < ROWS - 1 && maze[newRow + 1][newCol] == 0)
            pos.setY(pos.y() + 1);
        break;

    default:
        QMainWindow::keyReleaseEvent(event);
        return;
    }

    // Update block visual position
    setPlayerPos();
    if(pos.x() == COLS-2 && pos.y() == ROWS-2) {
        qDebug()<<"Reached End";
        on_regenMaze_clicked();
    }
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
    pos = QPointF(1, 1);
    scene->clear();
    block = scene->addRect(offsetX + pos.x()*gridStep, offsetY + pos.y()*gridStep, blockSize, blockSize, QPen(Qt::NoPen), QBrush(Qt::red));
    // setPlayerPos();
    block->setFlag(QGraphicsItem::ItemIsFocusable, true);
    block->setFocus();
    setPlayerPos();
    generateMaze();
    drawMaze();
}
