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
    QPointF pos;
    int gridStep;
    int blockSize;
    int SCENE_WIDTH;
    int SCENE_HEIGHT;
    int COLS;
    int ROWS;
    int offsetX = 0;
    int offsetY = 0;
};

#endif // MAINWINDOW_H
