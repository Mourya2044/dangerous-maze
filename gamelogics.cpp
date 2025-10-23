#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsScene>
#include <QPen>
#include <QBrush>
#include <QKeyEvent>
#include <QDebug>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>

void MainWindow::initGame() {
    // in constructor after drawMaze() and block creation:
    currentDir = DirNone;
    desiredDir = DirNone;
    score = 0;
    lives = 3;
    ui->coinsCollected->setText("Coins Collected: " + QString::number(score)); // reuse label or separate

    // spawn ghosts
    spawnGhosts();

}

int MainWindow::dx(Direction d) {
    if (d == DirLeft) return -1;
    if (d == DirRight) return 1;
    return 0;
}
int MainWindow::dy(Direction d) {
    if (d == DirUp) return -1;
    if (d == DirDown) return 1;
    return 0;
}

QPoint MainWindow::nextCell(const QPoint &pos, Direction dir) {
    return QPoint(pos.x() + dx(dir), pos.y() + dy(dir));
}

bool MainWindow::isWall(int row, int col) {
    if (row < 0 || col < 0 || row >= ROWS || col >= COLS) return true;
    return maze[row][col] == 1;
}

void MainWindow::drawMaze()
{
    coinItems.clear();

    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            int x = j * gridStep + offsetX;
            int y = i * gridStep + offsetY;

            if (maze[i][j] == 1) {
                scene->addRect(x, y, gridStep, gridStep, QPen(Qt::gray), QBrush(Qt::darkGray));
            }
            else if (maze[i][j] == 2) {
                // coin
                QGraphicsRectItem *coin = scene->addRect(x + gridStep/4, y + gridStep/4, gridStep/2, gridStep/2,
                                                         QPen(Qt::yellow), QBrush(Qt::yellow));
                coinItems[{i, j}] = coin;
            }
            else {
                scene->addRect(x, y, gridStep, gridStep, QPen(Qt::lightGray));
            }
        }
    }

    // Start and end markers
    scene->addRect(offsetX + 1 * gridStep, offsetY + 1 * gridStep, gridStep, gridStep, QPen(Qt::green), QBrush(Qt::green));
    scene->addRect(offsetX + (COLS - 2) * gridStep, offsetY + (ROWS - 2) * gridStep, gridStep, gridStep, QPen(Qt::blue), QBrush(Qt::blue));
    scene->addRect(0, 0, SCENE_WIDTH, SCENE_HEIGHT, QPen(Qt::black));
}


void MainWindow::generateMaze()
{
    // Initialize all as walls
    maze = QVector<QVector<int>>(ROWS, QVector<int>(COLS, 1));

    // Carve fixed paths for corridors (2-cell wide paths)
    for (int r = 1; r < ROWS - 1; r++) {
        for (int c = 1; c < COLS - 1; c++) {
            // Make outer boundary walls
            if (r == 1 || r == ROWS - 2 || c == 1 || c == COLS - 2) {
                maze[r][c] = 0; // open boundary for entrances/exits
            } else if ((r % 2 == 0) && (c % 2 == 0)) {
                // Create regular corridors
                maze[r][c] = 2;

                // Randomly connect to one neighbor
                std::vector<QPoint> dirs = {{0, -2}, {0, 2}, {-2, 0}, {2, 0}};
                std::random_device rd;
                std::mt19937 gen(rd());
                std::shuffle(dirs.begin(), dirs.end(), gen);

                for (const QPoint& dir : dirs) {
                    int nr = r + dir.y();
                    int nc = c + dir.x();
                    if (nr > 0 && nr < ROWS - 1 && nc > 0 && nc < COLS - 1 && maze[nr][nc] == 1) {
                        maze[r + dir.y()/2][c + dir.x()/2] = 0;
                        break;
                    }
                }
            }
        }
    }

    // Ensure spawn zones and starting positions
    maze[1][1] = 0;                  // player start
    maze[ROWS - 2][COLS - 2] = 0;    // exit/pellet zone

    // Optional: make horizontal symmetry
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS / 2; c++) {
            maze[r][COLS - 1 - c] = maze[r][c];
        }
    }
}

void MainWindow::setPlayerPos() {
    block->setRect(offsetX + pos.x() * gridStep, offsetY + pos.y() * gridStep, blockSize, blockSize);
}

void MainWindow::movePlayerTick() {
    QPoint gridPos = QPoint(pos.x(), pos.y());

    // try to turn (prefer desiredDir)
    if (desiredDir != DirNone && desiredDir != currentDir) {
        if (tryChangeDirection(gridPos, currentDir, desiredDir)) {
            currentDir = desiredDir;
        }
    }

    // move forward if possible
    if (currentDir != DirNone) {
        QPoint nxt = nextCell(gridPos, currentDir);
        if (!isWall(nxt.y(), nxt.x())) {
            pos = QPointF(nxt); // update grid coords
            setPlayerPos();
            // collect pellet
            if (maze[pos.y()][pos.x()] == 2) {
                maze[pos.y()][pos.x()] = 0;
                QPair<int,int> key = {pos.y(), pos.x()};
                if (coinItems.contains(key)) {
                    scene->removeItem(coinItems[key]);
                    delete coinItems[key];
                    coinItems.remove(key);
                }
                score++;
                ui->coinsCollected->setText("Coins Collected: " + QString::number(score));
            }
        } else {
            // blocked, stop moving
            currentDir = DirNone;
        }
    } else {
        // not moving; if desiredDir available, start moving next tick
        if (desiredDir != DirNone) {
            QPoint nxt = nextCell(gridPos, desiredDir);
            if (!isWall(nxt.y(), nxt.x())) {
                currentDir = desiredDir;
                pos = QPointF(nxt);
                setPlayerPos();
                // collect if coin...
            }
        }
    }
}

void MainWindow::moveEntities() {
    movePlayerTick();
    moveGhostsTick();
    // collisions
    for (const Ghost &g : ghosts) {
        if (g.pos == QPoint(pos.x(), pos.y())) {
            // collision: handle life lost
            lives--;
            if (lives <= 0) {
                // game over
                qDebug() << "Game Over";
                gameTimer->stop();
            } else {
                // respawn player and ghosts to start
                pos = QPoint(1,1);
                setPlayerPos();
                // respawn ghosts to initial positions (implement)
            }
            break;
        }
    }

    if(pos.x() == COLS-2 && pos.y() == ROWS-2) {
        regenMage();
    }
}

bool MainWindow::tryChangeDirection(QPoint gridPos, Direction from, Direction to) {
    QPoint nxt = nextCell(gridPos, to);
    return !isWall(nxt.y(), nxt.x());
}

// --------------- GHOST -------------------
void MainWindow::spawnGhosts() {
    ghosts.clear();
    // example two ghosts at near corners
    Ghost g1; g1.pos = QPoint(COLS-3, 1); g1.dir = DirLeft;
    QPixmap ghostPixmap(gridStep, gridStep);
    ghostPixmap.fill(Qt::magenta);
    g1.sprite = scene->addPixmap(ghostPixmap);
    g1.sprite->setPos(offsetX + g1.pos.x()*gridStep, offsetY + g1.pos.y()*gridStep);
    ghosts.push_back(g1);

    Ghost g2; g2.pos = QPoint(1, ROWS-3); g2.dir = DirRight;
    // QPixmap ghostPixmap(gridStep, gridStep);
    ghostPixmap.fill(Qt::magenta);
    g2.sprite = scene->addPixmap(ghostPixmap);
    g2.sprite->setPos(offsetX + g1.pos.x()*gridStep, offsetY + g1.pos.y()*gridStep);

    ghosts.push_back(g2);
}

void MainWindow::moveGhostsTick() {
    std::random_device rd;
    std::mt19937 gen(rd());

    for (Ghost &g : ghosts) {
        // find possible directions (no wall)
        QVector<Direction> options;
        for (Direction d : {DirLeft, DirRight, DirUp, DirDown}) {
            QPoint nxt = nextCell(g.pos, d);
            if (!isWall(nxt.y(), nxt.x()))
                options.push_back(d);
        }
        // prefer continuing same direction if possible
        if (!options.empty()) {
            if (std::find(options.begin(), options.end(), g.dir) != options.end() && (rand()%100 < 70)) {
                // 70% keep dir
            } else {
                std::uniform_int_distribution<> dist(0, options.size()-1);
                g.dir = options[dist(gen)];
            }
            // move one step
            g.pos = nextCell(g.pos, g.dir);
            g.sprite->setPos(offsetX + g.pos.x()*gridStep, offsetY + g.pos.y()*gridStep);
        }
    }
}

