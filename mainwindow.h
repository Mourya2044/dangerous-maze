#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QLabel>
#include "gamescene.h"
#include "mainmenuscene.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showMainMenu();
    void startGame();
    // --- UPDATED SLOT ---
    void updateScore(int levelScore, int totalScore);
    void updateLives(int lives);
    // --- NEW SLOT ---
    void updateLevel(int level);


private:
    QGraphicsView *view;
    GameScene *gameScene;
    MainMenuScene *mainMenuScene;

    QLabel *scoreLabel; // "Coins"
    QLabel *livesLabel;

    // --- NEW LABELS ---
    QLabel *totalScoreLabel;
    QLabel *levelLabel;

    // Define game area size
    const int SCENE_WIDTH = 800;
    const int SCENE_HEIGHT = 544;
};
#endif // MAINWINDOW_H
