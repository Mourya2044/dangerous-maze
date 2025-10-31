#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QVector>
#include <QMap>
#include <QPointF>
#include <random> // Added for maze generation

class GameScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit GameScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);
    void loadLevel(int levelNumber);

signals:
    // --- UPDATED SIGNAL ---
    void scoreChanged(int levelScore, int totalScore);
    void livesChanged(int lives);
    void gameOver();
    // --- NEW SIGNAL ---
    void levelChanged(int currentLevel);


protected:
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void moveEntities();

private:
    // Game entities
    QGraphicsPixmapItem *playerSprite;
    QMap<QPair<int, int>, QGraphicsPixmapItem*> coinItems;
    QVector<QVector<char>> maze; // Using char

    // To track walls for deletion
    QVector<QGraphicsPixmapItem*> wallItems;

    // Map dimensions
    int COLS = 0;
    int ROWS = 0;
    int gridStep = 40;
    int blockSize = 40;
    int offsetX = 0;
    int offsetY = 0;

    // Player state
    QPointF pos; // Player's grid position
    enum Direction { DirNone = -1, DirLeft = 0, DirRight = 1, DirUp = 2, DirDown = 3 };
    Direction currentDir;
    Direction desiredDir;
    QPoint playerStartPos;

    // Game state
    QTimer *gameTimer;
    int tickMs = 140;
    int score = 0; // This will now be "level score"
    int lives = 3;
    int currentLevel = 1;

    // --- NEW VARIABLES ---
    int totalScore = 0;
    int gameTickCounter = 0;
    int ghostMoveFrequency = 3;
    // --- END NEW ---

    // Ghosts
    struct Ghost {
        QPoint pos;
        Direction dir;
        QGraphicsPixmapItem *sprite;
    };
    QVector<Ghost> ghosts;
    QVector<QPoint> ghostStartPositions;

    // Helper functions
    void initGame();

    // Maze Generation
    void generateMaze(int rows, int cols);
    void populateMaze(std::mt19937& gen);

    void drawMaze();
    void spawnGhosts();

    void clearLevelItems(); // Replaces clear()

    void movePlayerTick();
    void moveGhostsTick();

    void setPlayerPos();
    bool tryChangeDirection(QPoint gridPos, Direction from, Direction to);
    bool isWall(int row, int col);
    QPoint nextCell(const QPoint &pos, Direction dir);
    int dx(Direction d);
    int dy(Direction d);
};

#endif // GAMESCENE_H
