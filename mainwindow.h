#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void drawMaze();
    void generateMaze();
    void setPlayerPos();

private slots:
    void on_backButton_clicked();
    void on_nextButton_clicked();
    void on_regenMaze_clicked();

private:
    Ui::MainWindow *ui;
    QGraphicsRectItem *block;      // The movable block
    int blockStep = 5;             // Pixels per timer tick
    QGraphicsScene *scene;
    QVector<QVector<int>> maze;
    QMap<QPair<int, int>, QGraphicsRectItem*> coinItems;
    int coinsCollected = 0;
    QPointF pos;
    int gridStep;
    int blockSize;
    int SCENE_WIDTH;
    int SCENE_HEIGHT;
    int COLS;
    int ROWS;
    int offsetX = 0;
    int offsetY = 0;

    // game state
    enum Direction { DirNone = -1, DirLeft = 0, DirRight = 1, DirUp = 2, DirDown = 3 };

    QTimer *gameTimer;
    int tickMs = 140;             // movement tick (adjust for speed)
    Direction currentDir;         // direction pacman currently moving
    Direction desiredDir;         // buffered direction from input

    int score = 0;
    int lives = 3;

    // ghosts
    struct Ghost {
        QPoint pos;                      // grid coords
        Direction dir;
        QGraphicsPixmapItem *sprite;     // or QGraphicsRectItem for prototype
        QColor color;                    // for prototype
    };
    QVector<Ghost> ghosts;

    // helper functions
    void initGame();
    void regenMage();
    void spawnGhosts();
    void moveEntities();                 // tick handler: move player + ghosts
    void movePlayerTick();
    void moveGhostsTick();
    bool tryChangeDirection(QPoint gridPos, Direction from, Direction to);
    bool isWall(int row, int col);
    QPoint nextCell(const QPoint &pos, Direction dir);
    int dx(Direction d);
    int dy(Direction d);

};

#endif // MAINWINDOW_H
