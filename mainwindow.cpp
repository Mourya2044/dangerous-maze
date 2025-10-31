#include "mainwindow.h"
#include <QVBoxLayout>
#include <QGraphicsProxyWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 1. Create the main view
    view = new QGraphicsView(this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFixedSize(SCENE_WIDTH + 2, SCENE_HEIGHT + 2); // +2 for border

    // 2. Create the scenes
    mainMenuScene = new MainMenuScene(0, 0, SCENE_WIDTH, SCENE_HEIGHT, this);
    gameScene = new GameScene(0, 0, SCENE_WIDTH, SCENE_HEIGHT, this);

    // 3. Create UI elements
    levelLabel = new QLabel("Level: 1");
    levelLabel->setAttribute(Qt::WA_TranslucentBackground);
    levelLabel->setStyleSheet("color: white; font-weight: bold;");

    scoreLabel = new QLabel("Coins: 0");
    scoreLabel->setAttribute(Qt::WA_TranslucentBackground);
    scoreLabel->setStyleSheet("color: white; font-weight: bold;");

    livesLabel = new QLabel("Lives: 3");
    livesLabel->setAttribute(Qt::WA_TranslucentBackground);
    livesLabel->setStyleSheet("color: white; font-weight: bold;");

    totalScoreLabel = new QLabel("Total: 0");
    totalScoreLabel->setAttribute(Qt::WA_TranslucentBackground);
    totalScoreLabel->setStyleSheet("color: white; font-weight: bold;");

    // 4. Add UI to the GameScene
    QGraphicsProxyWidget *levelProxy = gameScene->addWidget(levelLabel);
    QGraphicsProxyWidget *scoreProxy = gameScene->addWidget(scoreLabel);
    QGraphicsProxyWidget *livesProxy = gameScene->addWidget(livesLabel);
    QGraphicsProxyWidget *totalScoreProxy = gameScene->addWidget(totalScoreLabel);

    // Arrange UI
    levelProxy->setPos(10, 5);
    scoreProxy->setPos(10, 25);
    livesProxy->setPos(SCENE_WIDTH - 80, 5);
    totalScoreProxy->setPos(SCENE_WIDTH - 80, 25);

    // --- THIS IS THE BUG FIX ---
    // Set a high Z-value to ensure UI is drawn on top of the maze
    levelProxy->setZValue(100);
    scoreProxy->setZValue(100);
    livesProxy->setZValue(100);
    totalScoreProxy->setZValue(100);
    // --- END FIX ---

    // 5. Set up the central widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->addWidget(view);
    setCentralWidget(centralWidget);

    // 6. Connect signals
    connect(mainMenuScene, &MainMenuScene::startGameClicked, this, &MainWindow::startGame);
    connect(gameScene, &GameScene::scoreChanged, this, &MainWindow::updateScore);
    connect(gameScene, &GameScene::livesChanged, this, &MainWindow::updateLives);
    connect(gameScene, &GameScene::gameOver, this, &MainWindow::showMainMenu);
    // --- NEW CONNECTION ---
    connect(gameScene, &GameScene::levelChanged, this, &MainWindow::updateLevel);

    // 7. Start with the main menu
    showMainMenu();

    // Set fixed window size
    setFixedSize(centralWidget->sizeHint());
    setWindowTitle("Mage Game");
}

MainWindow::~MainWindow()
{
}

void MainWindow::showMainMenu()
{
    view->setScene(mainMenuScene);
    mainMenuScene->setFocus();
}

void MainWindow::startGame()
{
    gameScene->loadLevel(1); // Start level 1
    view->setScene(gameScene);
    gameScene->setFocus();
}

// --- UPDATED SLOT ---
void MainWindow::updateScore(int levelScore, int totalScore)
{
    scoreLabel->setText("Coins: " + QString::number(levelScore));
    totalScoreLabel->setText("Total: " + QString::number(totalScore));
}

void MainWindow::updateLives(int lives)
{
    livesLabel->setText("Lives: " + QString::number(lives));
}

// --- NEW SLOT ---
void MainWindow::updateLevel(int level)
{
    levelLabel->setText("Level: " + QString::number(level));
}
