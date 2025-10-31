#include "gamescene.h"
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QBrush>
#include <random>
#include <QStack>
#include <algorithm> // for std::max and std::min

GameScene::GameScene(qreal x, qreal y, qreal width, qreal height, QObject *parent)
    : QGraphicsScene(x, y, width, height, parent)
{
    // Set background
    setBackgroundBrush(QBrush(Qt::black));
    playerSprite = nullptr; // Initialize pointer

    // Timer
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &GameScene::moveEntities);
}

void GameScene::clearLevelItems()
{
    // Delete player
    if (playerSprite) {
        removeItem(playerSprite);
        delete playerSprite;
        playerSprite = nullptr;
    }

    // Delete ghosts
    for (Ghost &g : ghosts) {
        if (g.sprite) {
            removeItem(g.sprite);
            delete g.sprite;
        }
    }
    ghosts.clear();

    // Delete coins
    for (QGraphicsPixmapItem* coin : coinItems) {
        removeItem(coin);
        delete coin;
    }
    coinItems.clear();

    // Delete walls
    for (QGraphicsPixmapItem* wall : wallItems) {
        removeItem(wall);
        delete wall;
    }
    wallItems.clear();
}

void GameScene::loadLevel(int levelNumber)
{
    gameTimer->stop();
    clearLevelItems();

    ghostStartPositions.clear();
    maze.clear();
    ROWS = 0;
    COLS = 0;
    offsetX = 0;
    offsetY = 0;

    currentLevel = levelNumber;
    emit levelChanged(currentLevel);

    // Ghost speed scaling
    ghostMoveFrequency = std::max(1, 3 - (levelNumber - 1));
    gameTickCounter = 0;

    // Maze size formula
    int mazeRows = 17 + (levelNumber - 1) * 6;
    int mazeCols = 25 + (levelNumber - 1) * 8;

    // Fit gridStep to the view
    int stepX = sceneRect().width() / mazeCols;
    int stepY = sceneRect().height() / mazeRows;
    gridStep = std::min(stepX, stepY);
    blockSize = gridStep;

    generateMaze(mazeRows, mazeCols);

    // Recalculate offsets to center the maze
    if (ROWS > 0 && COLS > 0) {
        offsetX = (sceneRect().width() - (COLS * gridStep)) / 2;
        offsetY = (sceneRect().height() - (ROWS * gridStep)) / 2;
    }

    drawMaze(); // This will create player/ghost sprites
    initGame();
    gameTimer->start(tickMs);
}


void GameScene::generateMaze(int rows, int cols)
{
    ROWS = rows;
    COLS = cols;
    maze.resize(ROWS);
    for(int r = 0; r < ROWS; ++r) {
        maze[r].resize(COLS, '1'); // 1 = Wall
    }

    QVector<QVector<bool>> visited(ROWS, QVector<bool>(COLS, false));
    QStack<QPoint> stack;
    std::random_device rd;
    std::mt19937 gen(rd());

    // 1. Start DFS from (1, 1)
    playerStartPos = QPoint(1, 1);
    stack.push(playerStartPos);
    maze[1][1] = '0'; // 0 = Path
    visited[1][1] = true;

    QPoint dirs[4] = {{0, -2}, {0, 2}, {-2, 0}, {2, 0}};

    while (!stack.isEmpty()) {
        QPoint current = stack.top();
        QVector<QPoint> neighbors;

        std::shuffle(dirs, dirs + 4, gen);
        for (const QPoint& dir : dirs) {
            int nx = current.x() + dir.x();
            int ny = current.y() + dir.y();

            if (nx > 0 && nx < COLS - 1 && ny > 0 && ny < ROWS - 1 && !visited[ny][nx]) {
                neighbors.push_back(QPoint(nx, ny));
            }
        }

        if (neighbors.isEmpty()) {
            stack.pop(); // Backtrack
        } else {
            QPoint next = neighbors.first();

            int wallX = current.x() + (next.x() - current.x()) / 2;
            int wallY = current.y() + (next.y() - current.y()) / 2;
            maze[wallY][wallX] = '0';

            maze[next.y()][next.x()] = '0';
            visited[next.y()][next.x()] = true;
            stack.push(next);
        }
    }

    populateMaze(gen);
}

void GameScene::populateMaze(std::mt19937& gen)
{
    // 1. Add Loops (Alternative Paths)
    std::uniform_int_distribution<> dist(0, 99);
    int loopDensity = 15;

    for (int r = 1; r < ROWS - 1; ++r) {
        for (int c = 1; c < COLS - 1; ++c) {
            if (maze[r][c] == '1') {
                bool horizontal = (c > 0 && c < COLS - 1 && maze[r][c-1] == '0' && maze[r][c+1] == '0');
                bool vertical = (r > 0 && r < ROWS - 1 && maze[r-1][c] == '0' && maze[r+1][c] == '0');

                if ((horizontal || vertical) && dist(gen) < loopDensity) {
                    maze[r][c] = '0';
                }
            }
        }
    }

    // 2. Populate Coins
    std::uniform_int_distribution<> coinDist(0, 6); // 1 in 7 chance
    for (int r = 1; r < ROWS - 1; ++r) {
        for (int c = 1; c < COLS - 1; ++c) {
            if (maze[r][c] == '0' && QPoint(c, r) != playerStartPos) {
                if (coinDist(gen) == 0) {
                    maze[r][c] = '2';
                }
            }
        }
    }

    // 3. Place Exit
    maze[ROWS - 2][COLS - 1] = 'E'; // 'E' = Exit
    maze[ROWS - 2][COLS - 3] = '0';
    maze[ROWS - 3][COLS - 2] = '0';

    // 4. Place Ghosts
    int ghostCount = std::min(currentLevel + 1, 10);
    std::uniform_int_distribution<> rowDist(1, ROWS - 2);
    std::uniform_int_distribution<> colDist(1, COLS - 2);
    int minPlayerDist = (ROWS + COLS) / 4;

    for (int i = 0; i < ghostCount; ++i) {
        while (true) {
            int r = rowDist(gen);
            int c = colDist(gen);
            int distToPlayer = abs(r - playerStartPos.y()) + abs(c - playerStartPos.x());

            if (maze[r][c] != '1' && distToPlayer > minPlayerDist) {
                ghostStartPositions.append(QPoint(c, r));
                break;
            }
        }
    }

    // 5. Place Player
    maze[playerStartPos.y()][playerStartPos.x()] = 'P';
}


void GameScene::drawMaze()
{
    QPixmap wallPixmap(":/sprites/wall.png");
    QPixmap coinPixmap(":/sprites/coin.png");
    QPixmap playerPixmap(":/sprites/player.png");

    playerSprite = addPixmap(playerPixmap.scaled(blockSize, blockSize));
    playerSprite->setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFocusItem(playerSprite);

    // --- CHANGE 1: SET ROTATION ORIGIN ---
    // Tell the sprite to rotate around its center, not its top-left corner
    playerSprite->setTransformOriginPoint(blockSize / 2, blockSize / 2);
    // --- END CHANGE ---

    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            int x = j * gridStep + offsetX;
            int y = i * gridStep + offsetY;

            if (maze[i][j] == '1') {
                QGraphicsPixmapItem *wall = addPixmap(wallPixmap.scaled(gridStep, gridStep));
                wall->setPos(x, y);
                wallItems.push_back(wall);
            }
            else if (maze[i][j] == '2') {
                QGraphicsPixmapItem *coin = addPixmap(coinPixmap.scaled(gridStep, gridStep));
                coin->setPos(x, y);
                coinItems[{i, j}] = coin;
            }
        }
    }
    spawnGhosts();
}

void GameScene::initGame() {
    currentDir = DirNone;
    desiredDir = DirNone;
    score = 0;
    lives = 3;

    emit scoreChanged(score, totalScore);
    emit livesChanged(lives);

    pos = playerStartPos;
    setPlayerPos();
}

void GameScene::spawnGhosts() {
    QPixmap ghostPixmap(":/sprites/ghost.png");

    for (const QPoint& gPos : ghostStartPositions) {
        Ghost g;
        g.pos = gPos;
        g.dir = DirLeft;
        g.sprite = addPixmap(ghostPixmap.scaled(gridStep, gridStep));
        g.sprite->setPos(offsetX + g.pos.x()*gridStep, offsetY + g.pos.y()*gridStep);
        ghosts.push_back(g);
    }
}

void GameScene::keyReleaseEvent(QKeyEvent *event) {
    Direction d = DirNone;
    switch (event->key()) {
    case Qt::Key_Left: case Qt::Key_A: d = DirLeft; break;
    case Qt::Key_Right: case Qt::Key_D: d = DirRight; break;
    case Qt::Key_Up: case Qt::Key_W: d = DirUp; break;
    case Qt::Key_Down: case Qt::Key_S: d = DirDown; break;
    default: QGraphicsScene::keyReleaseEvent(event); return;
    }
    desiredDir = d;
}

int GameScene::dx(Direction d) {
    if (d == DirLeft) return -1;
    if (d == DirRight) return 1;
    return 0;
}
int GameScene::dy(Direction d) {
    if (d == DirUp) return -1;
    if (d == DirDown) return 1;
    return 0;
}

QPoint GameScene::nextCell(const QPoint &pos, Direction dir) {
    return QPoint(pos.x() + dx(dir), pos.y() + dy(dir));
}

bool GameScene::isWall(int row, int col) {
    if (maze.isEmpty() || ROWS == 0 || COLS == 0) {
        return true;
    }
    if (row < 0 || col < 0 || row >= ROWS || col >= COLS) {
        return true;
    }
    return maze[row][col] == '1';
}

void GameScene::setPlayerPos() {
    if (playerSprite) {
        playerSprite->setPos(offsetX + pos.x() * gridStep, offsetY + pos.y() * gridStep);
    }
}

void GameScene::movePlayerTick() {
    QPoint gridPos = QPoint(pos.x(), pos.y());

    if (desiredDir != DirNone && desiredDir != currentDir) {
        if (tryChangeDirection(gridPos, currentDir, desiredDir)) {
            currentDir = desiredDir;
        }
    }

    if (currentDir != DirNone) {
        QPoint nxt = nextCell(gridPos, currentDir);
        if (!isWall(nxt.y(), nxt.x())) {
            pos = QPointF(nxt);
            setPlayerPos();

            // --- CHANGE 2: APPLY ROTATION ---
            // (Assuming your 'player.png' sprite faces right by default)
            switch (currentDir) {
            case DirRight:
                playerSprite->setRotation(0);
                break;
            case DirLeft:
                playerSprite->setRotation(180);
                break;
            case DirUp:
                playerSprite->setRotation(270); // or -90
                break;
            case DirDown:
                playerSprite->setRotation(90);
                break;
            default:
                break; // No change if DirNone
            }
            // --- END CHANGE ---

            int cellY = pos.y();
            int cellX = pos.x();

            if (maze[cellY][cellX] == '2') {
                maze[cellY][cellX] = '0';
                QPair<int,int> key = {cellY, cellX};
                if (coinItems.contains(key)) {
                    removeItem(coinItems[key]);
                    delete coinItems[key];
                    coinItems.remove(key);
                }

                score++;
                totalScore++;
                emit scoreChanged(score, totalScore);
            }
            else if (maze[cellY][cellX] == 'E') {
                gameTimer->stop();
                loadLevel(currentLevel + 1);
                return;
            }

        } else {
            currentDir = DirNone;
        }
    } else {
        if (desiredDir != DirNone) {
            QPoint nxt = nextCell(gridPos, desiredDir);
            if (!isWall(nxt.y(), nxt.x())) {
                currentDir = desiredDir;
            }
        }
    }
}

void GameScene::moveEntities() {
    gameTickCounter++;
    movePlayerTick();
    moveGhostsTick();

    for (const Ghost &g : ghosts) {
        if (g.pos == QPoint(pos.x(), pos.y())) {
            lives--;
            emit livesChanged(lives);
            if (lives <= 0) {
                gameTimer->stop();
                totalScore = 0;
                emit gameOver();
            } else {
                pos = playerStartPos;
                setPlayerPos();
                currentDir = DirNone;
                desiredDir = DirNone;
            }
            break;
        }
    }
}

bool GameScene::tryChangeDirection(QPoint gridPos, Direction from, Direction to) {
    QPoint nxt = nextCell(gridPos, to);
    return !isWall(nxt.y(), nxt.x());
}

void GameScene::moveGhostsTick() {
    // Speed Control Check
    if (gameTickCounter % ghostMoveFrequency != 0) {
        return; // Skip ghost movement for this tick
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    for (Ghost &g : ghosts) {
        QVector<Direction> options;
        Direction oppositeDir = DirNone;
        if (g.dir == DirLeft) oppositeDir = DirRight;
        else if (g.dir == DirRight) oppositeDir = DirLeft;
        else if (g.dir == DirUp) oppositeDir = DirDown;
        else if (g.dir == DirDown) oppositeDir = DirUp;

        for (Direction d : {DirLeft, DirRight, DirUp, DirDown}) {
            QPoint nxt = nextCell(g.pos, d);
            if (!isWall(nxt.y(), nxt.x())) {
                if (d != oppositeDir) {
                    options.push_back(d);
                }
            }
        }

        if (options.empty()) {
            QPoint nxt = nextCell(g.pos, oppositeDir);
            if (oppositeDir != DirNone && !isWall(nxt.y(), nxt.x())) {
                options.push_back(oppositeDir);
            }
        }

        if (!options.empty()) {
            bool found = false;
            for(Direction d : options) {
                if(d == g.dir) {
                    found = true;
                    break;
                }
            }

            if (found && (rand() % 100 < 80)) {
                // 80% chance to keep going straight
            } else {
                std::uniform_int_distribution<> dist(0, options.size()-1);
                g.dir = options[dist(gen)];
            }

            g.pos = nextCell(g.pos, g.dir);
            g.sprite->setPos(offsetX + g.pos.x()*gridStep, offsetY + g.pos.y()*gridStep);
        }
    }
}
