#include <iostream>
#include <string>
#include <SFML/Graphics.hpp>
#include "window.h"
#include <cctype>
#include <vector>
#include <fstream>
#include <random>
#include <queue>
#include "Gametile.h"
#include "leaderboardWindow.h"

using namespace std;

class WelcomeWindow : public Window {
private:

    sf::Text title, prompt, userInput, cursor;
    std::string playerName;
    bool shouldLaunchGame = false;




    void handleInput(sf::Event &event) {
        if (event.type == sf::Event::Closed) {
            shouldLaunchGame = false;
            window.close();
        } else if (event.type == sf::Event::TextEntered) {
            if (isalpha(event.text.unicode) && playerName.size() < 10) {
                playerName += static_cast<char>(event.text.unicode);
                playerName[0] = toupper(playerName[0]);
                for (size_t i = 1; i < playerName.size(); ++i)
                    playerName[i] = tolower(playerName[i]);
            } else if (event.text.unicode == '\b' && !playerName.empty()) {
                playerName.pop_back();
            }
            userInput.setString(playerName);


            sf::FloatRect textBounds = userInput.getLocalBounds();
            userInput.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
            userInput.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f - 45);
        } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
            if (!playerName.empty()) {
                shouldLaunchGame = true;
                window.close();
            }
        }
    }

public:

    string getPlayerName() const {
        return playerName;
    }

    WelcomeWindow(int width, int height) : Window(width, height, "Welcome to Minesweeper") {
        setText(title, "WELCOME TO MINESWEEPER!", width / 2, height / 2 - 150, 24, sf::Color::White);
        setText(prompt, "Enter your name:", width / 2, height / 2 - 75, 20, sf::Color::White);
        setText(userInput, "", width / 2, height / 2 - 45, 18, sf::Color::Yellow);
        setText(cursor, "|", width / 2, height / 2 - 45, 18, sf::Color::Yellow);
    }

    void run() override {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                handleInput(event);
            }

            // Update cursor pos
            cursor.setPosition(
                userInput.getPosition().x + userInput.getLocalBounds().width / 2.0f + 5,
                userInput.getPosition().y
            );

            // Render
            window.clear(sf::Color::Blue);
            window.draw(title);
            window.draw(prompt);
            window.draw(userInput);
            window.draw(cursor);
            window.display();
        }
    }

    bool shouldLaunch() const { return shouldLaunchGame; }
};

class GameWindow : public Window {
private:
    vector<vector<GameTile>> tiles;
    sf::Texture hiddenTexture, revealedTexture, flagTexture, mineTexture;
    vector<sf::Texture> numberTextures;
    int cols, rows, mines;

    // Button Textures and Sprites
    sf::Texture happyFaceTexture, debugTexture, playTexture, leaderboardTexture, winFaceTexture, loseFaceTexture;
    sf::Sprite happyFaceButton, debugButton, playButton, leaderboardButton;

    bool debugMode = false;
    bool gameOver = false;

    sf::Texture digitsTexture;
    sf::Sprite counterSprites[3];
    int remainingMines;

    sf::Sprite timerMinutesSprites[2]; // Two sprites for the minutes (e.g., "01")
    sf::Sprite timerSecondsSprites[2]; // Two sprites for the seconds (e.g., "23")
    sf::Clock gameClock;              // SFML clock to track elapsed time
    int elapsedTime = 0;              // Total elapsed time in seconds
    int currentMinutes = 0;           // Minutes part of the timer
    int currentSeconds = 0;
    sf::Texture pauseTexture;
    sf::Time pausedTime;
    sf::Time elapsedBeforePause;

    string playerName;




    bool paused = false; // Tracks if the game is paused
    std::vector<std::vector<bool>> tileRevealedStates; // Stores revealed states of tiles
    std::vector<std::vector<bool>> tileFlaggedStates;  // Stores flagged states of tiles


    void loadConfig(const string &configPath) {
        ifstream configFile(configPath);
        if (!configFile) throw runtime_error("Unable to open config file");

        configFile >> cols >> rows >> mines;
    }

    void loadTextures() {
        if (!hiddenTexture.loadFromFile("files/images/tile_hidden.png") ||
            !revealedTexture.loadFromFile("files/images/tile_revealed.png") ||
            !flagTexture.loadFromFile("files/images/flag.png") ||
            !mineTexture.loadFromFile("files/images/mine.png") ||
            !digitsTexture.loadFromFile("files/images/digits.png")) {
            throw runtime_error("Unable to load textures");
            }

        numberTextures.resize(8);
        for (int i = 1; i <= 8; ++i) {
            if (!numberTextures[i - 1].loadFromFile("files/images/number_" + to_string(i) + ".png")) {
                throw runtime_error("Unable to load number textures");
            }
        }
    }


    void loadButtonTextures() {
        if (!happyFaceTexture.loadFromFile("files/images/face_happy.png") ||
            !debugTexture.loadFromFile("files/images/debug.png") ||
            !playTexture.loadFromFile("files/images/play.png") || // Load play.png
            !pauseTexture.loadFromFile("files/images/pause.png") || // Load pause.png
            !leaderboardTexture.loadFromFile("files/images/leaderboard.png") ||
            !winFaceTexture.loadFromFile("files/images/face_win.png") ||
            !loseFaceTexture.loadFromFile("files/images/face_lose.png")) {
            throw runtime_error("Unable to load button textures");
            }

        happyFaceButton.setTexture(happyFaceTexture);
        debugButton.setTexture(debugTexture);
        playButton.setTexture(pauseTexture); // Initially set to pause
        leaderboardButton.setTexture(leaderboardTexture);
    }


    void flagTile(GameTile &tile) {
        if (tile.isRevealed()) return; // Ignore revealed tiles

        bool wasFlagged = tile.getIsFlagged(); // Check current flag state

        // Toggle the flag state of the tile
        tile.flag(flagTexture, hiddenTexture);

        // Update the remaining mines count
        if (wasFlagged) {
            remainingMines++; // Unflagging increases the count
        } else {
            remainingMines--; // Flagging decreases the count
        }

        // Update the counter display to reflect the new mine count
        updateCounter();
    }







    LeaderboardWindow* leaderboardWindow = nullptr;
    bool isLeaderboardOpen = false;
    // ... (other members remain the same)

    // New Method to Open Leaderboard
    void openLeaderboard(const std::string &currentPlayerName, int currentTime) {
        if (!leaderboardWindow) {
            int leaderboardWidth = cols * 32;
            int leaderboardHeight = rows * 16 + 50;
            leaderboardWindow = new LeaderboardWindow(leaderboardWidth, leaderboardHeight);
        }

        // Open the leaderboard and pass the current player's details
        leaderboardWindow->open(currentPlayerName, currentTime);
        isLeaderboardOpen = true;
    }




    // New Method to Close Leaderboard
    void closeLeaderboard() {
        if (leaderboardWindow) {
            delete leaderboardWindow;
            leaderboardWindow = nullptr;
        }
        isLeaderboardOpen = false;
    }






    void resetGame() {
        paused = false; // Reset paused state
        gameOver = false;
        debugMode = false;

        remainingMines = mines; // Reset the mine counter
        updateCounter();

        // Restart the clock and reset the paused time
        gameClock.restart();
        elapsedBeforePause = sf::Time::Zero; // Clear any previously accumulated paused time

        elapsedTime = 0; // Reset elapsed time
        currentMinutes = 0;
        currentSeconds = 0;
        updateTimer();

        happyFaceButton.setTexture(happyFaceTexture); // Reset happy face texture
        initializeTiles(); // Reinitialize the board
        placeMines(); // Randomly place mines
    }









    bool checkWin() {
        for (const auto &row : tiles) {
            for (const auto &tile : row) {
                if (!tile.isRevealed() && !tile.hasMine()) {
                    return false; // If any hidden, non-mine tile exists, the game isn't won yet
                }
            }
        }

        // Flag all remaining mines
        for (auto &row : tiles) {
            for (auto &tile : row) {
                if (tile.hasMine() && !tile.getIsFlagged()) {
                    tile.flag(flagTexture, hiddenTexture); // Flag the mine if not already flagged
                }
            }
        }

        // Set remaining mines to 0 and update the counter
        remainingMines = 0;
        updateCounter();

        // Player wins: Capture winning time
        sf::Time totalElapsedTime = elapsedBeforePause + gameClock.getElapsedTime();
        int totalSeconds = static_cast<int>(totalElapsedTime.asSeconds());

        // Ensure leaderboardWindow is initialized
        if (!leaderboardWindow) {
            leaderboardWindow = new LeaderboardWindow(cols * 32, rows * 16 + 50);
        }

        // Update the leaderboard
        leaderboardWindow->addPlayerScore(playerName, totalSeconds);

        // Save the leaderboard immediately
        leaderboardWindow->saveLeaderboard();

        // Automatically open the leaderboard after a win
        openLeaderboard(playerName, totalSeconds);

        // Update UI and game state
        happyFaceButton.setTexture(winFaceTexture);
        gameOver = true; // Stop the game and timer
        cout << "You Win!" << endl;

        return true;
    }
















    void positionButtons() {
        int buttonRowY = 32 * (rows + 0.5); // Y position of the buttons

        // Position buttons based on the given formulas
        happyFaceButton.setPosition((cols * 32) / 2 - 32, buttonRowY);
        debugButton.setPosition((cols * 32) - 304, buttonRowY);
        playButton.setPosition((cols * 32) - 240, buttonRowY);
        leaderboardButton.setPosition((cols * 32) - 176, buttonRowY);
    }


    void positionTimer() {
        int rowsYOffset = 32 * (rows + 0.5) + 16;
        int minutesStartX = (cols * 32) - 97;
        int secondsStartX = (cols * 32) - 54;

        std::cout << "Minutes Timer Position: (" << minutesStartX << ", " << rowsYOffset << ")\n";
        std::cout << "Seconds Timer Position: (" << secondsStartX << ", " << rowsYOffset << ")\n";

        for (int i = 0; i < 2; ++i) {
            timerMinutesSprites[i].setTexture(digitsTexture);
            timerMinutesSprites[i].setPosition(minutesStartX + (i * 21), rowsYOffset);
            timerMinutesSprites[i].setTextureRect(sf::IntRect(0, 0, 21, 32));
        }
        for (int i = 0; i < 2; ++i) {
            timerSecondsSprites[i].setTexture(digitsTexture);
            timerSecondsSprites[i].setPosition(secondsStartX + (i * 21), rowsYOffset);
            timerSecondsSprites[i].setTextureRect(sf::IntRect(0, 0, 21, 32));
        }
    }



    void updateTimer() {
        if (gameOver || paused) return; // Do not update timer if game is over or paused

        sf::Time totalElapsedTime = elapsedBeforePause + gameClock.getElapsedTime();
        int totalSeconds = static_cast<int>(totalElapsedTime.asSeconds());
        currentMinutes = totalSeconds / 60;
        currentSeconds = totalSeconds % 60;

        for (int i = 1; i >= 0; --i) {
            int digit = currentMinutes % 10;
            currentMinutes /= 10;
            timerMinutesSprites[i].setTextureRect(sf::IntRect(digit * 21, 0, 21, 32));
        }

        for (int i = 1; i >= 0; --i) {
            int digit = currentSeconds % 10;
            currentSeconds /= 10;
            timerSecondsSprites[i].setTextureRect(sf::IntRect(digit * 21, 0, 21, 32));
        }
    }







    void positionCounter() {
        int startX = 33; // Starting x-coordinate
        int startY = 32 * (rows + 0.5) + 16; // Starting y-coordinate

        for (int i = 0; i < 3; ++i) {
            counterSprites[i].setTexture(digitsTexture);
            counterSprites[i].setPosition(startX + (i * 21), startY); // Offset by 21px for each digit
        }
    }


    void updateCounter() {
        int count = abs(remainingMines); // Use absolute value for digit extraction
        for (int i = 2; i >= 0; --i) {
            int digit = count % 10; // Extract the last digit
            count /= 10;

            counterSprites[i].setTextureRect(sf::IntRect(digit * 21, 0, 21, 32)); // Update texture rect
        }

        // Handle negative numbers (if needed)
        if (remainingMines < 0) {
            counterSprites[0].setTextureRect(sf::IntRect(10 * 21, 0, 21, 32)); // '-' is at index 10
        }
    }



    void initializeTiles() {
        tiles.clear();
        tileRevealedStates.resize(rows, std::vector<bool>(cols, false));
        tileFlaggedStates.resize(rows, std::vector<bool>(cols, false));

        for (int i = 0; i < rows; ++i) {
            vector<GameTile> row;
            for (int j = 0; j < cols; ++j) {
                GameTile tile(hiddenTexture);
                tile.setPosition(j * 32, i * 32);
                row.push_back(tile);
            }
            tiles.push_back(row);
        }

        // Assign adjacent tiles for recursive revealing
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        if (dr == 0 && dc == 0) continue;
                        int nr = r + dr, nc = c + dc;
                        if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                            tiles[r][c].addAdjacentTile(&tiles[nr][nc]);
                        }
                    }
                }
            }
        }
    }


    void placeMines() {
        int minesToPlace = mines;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> rowDist(0, rows - 1);
        uniform_int_distribution<> colDist(0, cols - 1);

        while (minesToPlace > 0) {
            int row = rowDist(gen);
            int col = colDist(gen);
            if (!tiles[row][col].hasMine()) {
                tiles[row][col].setMine(true);
                minesToPlace--;
            }
        }

        calculateAdjacentMines();
    }

    void calculateAdjacentMines() {
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                if (tiles[r][c].hasMine()) continue;

                int mineCount = 0;
                for (auto neighbor : tiles[r][c].getAdjacentTiles()) {
                    if (neighbor->hasMine()) {
                        mineCount++;
                    }
                }
                tiles[r][c].setAdjacentMines(mineCount);
            }
        }
    }

    void handleInput(sf::Event &event) {
    if (event.type == sf::Event::Closed) {
        window.close();
        if (isLeaderboardOpen) closeLeaderboard();
    } else if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        // Handle leaderboard button interaction (always allow opening leaderboard)
        if (leaderboardButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            if (!isLeaderboardOpen) {
                openLeaderboard(playerName, elapsedTime); // Open leaderboard with current player's name and time
            } else {
                closeLeaderboard(); // Close leaderboard if already open
            }
            return;
        }

        // Handle happy face button interaction (always allow reset)
        if (happyFaceButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            resetGame();
            return;
        }

        // Handle pause/resume button
        if (!gameOver && playButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            togglePause();
            return;
        }

        // Handle debug button interaction (disable interaction if paused)
        if (!paused && debugButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            debugMode = !debugMode;
            return;
        }

        // Handle tile interactions (disable interaction if game is over or paused)
        if (!gameOver && !paused) {
            for (auto &row : tiles) {
                for (auto &tile : row) {
                    if (tile.getBounds().contains(mousePos.x, mousePos.y)) {
                        if (event.mouseButton.button == sf::Mouse::Left) {
                            revealTile(tile);
                        } else if (event.mouseButton.button == sf::Mouse::Right) {
                            flagTile(tile);
                        }
                        return;
                    }
                }
            }
        }
    }
}






    void checkWinAndUpdateLeaderboard() {
    // Player wins: Capture winning time
    sf::Time totalElapsedTime = elapsedBeforePause + gameClock.getElapsedTime();
    int totalSeconds = static_cast<int>(totalElapsedTime.asSeconds());

    // Set remaining mines to 0 and update the counter display
    remainingMines = 0;
    updateCounter();

    // Automatically flag all remaining mines when the player wins
    for (auto &row : tiles) {
        for (auto &tile : row) {
            if (!tile.isRevealed() && tile.hasMine() && !tile.getIsFlagged()) {
                tile.flag(flagTexture, hiddenTexture); // Flag the mine if not already flagged
            }
        }
    }

    // Load the leaderboard entries from the file
    vector<LeaderboardEntry> currentEntries;
    ifstream leaderboardFile("files/leaderboard.txt");
    if (leaderboardFile.is_open()) {
        string line;
        while (getline(leaderboardFile, line)) {
            stringstream ss(line);
            string timeStr, name;
            getline(ss, timeStr, ',');
            getline(ss, name);

            // Convert time format (MM:SS) to total seconds
            int minutes = stoi(timeStr.substr(0, 2));
            int seconds = stoi(timeStr.substr(3, 2));
            int totalTime = minutes * 60 + seconds;

            currentEntries.push_back({totalTime, timeStr, name});
        }
        leaderboardFile.close();
    }


    string formattedTime = LeaderboardWindow::formatTime(totalSeconds);
    currentEntries.push_back({totalSeconds, formattedTime, playerName});


    sort(currentEntries.begin(), currentEntries.end(), [](const LeaderboardEntry &a, const LeaderboardEntry &b) {
        return a.time < b.time;
    });
    if (currentEntries.size() > 5) {
        currentEntries.resize(5);
    }

    // Check if the player's score is in the top 5
    bool isInTop5 = false;
    for (const auto &entry : currentEntries) {
        if (entry.name == playerName && entry.time == totalSeconds) {
            isInTop5 = true;
            break;
        }
    }

    // Write the updated leaderboard back to the file
    if (isInTop5) {
        ofstream leaderboardOutFile("files/leaderboard.txt", ios::trunc);
        if (leaderboardOutFile.is_open()) {
            for (const auto &entry : currentEntries) {
                leaderboardOutFile << entry.formattedTime << "," << entry.name << endl;
            }
            leaderboardOutFile.close();
        }
    }


    if (isInTop5 && !isLeaderboardOpen) {
        openLeaderboard(playerName, totalSeconds);
    }


    happyFaceButton.setTexture(winFaceTexture);
    gameOver = true; // Stop the game and timer
    cout << "You Win!" << endl;
}








    void revealAllMinesAfterLoss() {
        for (auto &row : tiles) {
            for (auto &tile : row) {
                if (tile.hasMine()) {
                    // Use the specialized reveal method to handle flags and mines correctly
                    tile.revealMineAfterLoss(mineTexture);
                }
            }
        }
    }




    void loseGame() {
        // Set game state to over
        gameOver = true;
        happyFaceButton.setTexture(loseFaceTexture);

        // Reveal all mines, including those with flags
        revealAllMinesAfterLoss();
    }







    void togglePause() {
        if (gameOver) return; // Prevent pause/unpause if the game is over

        paused = !paused; // Toggle pause state

        if (paused) {
            playButton.setTexture(playTexture); // Show the play icon
            elapsedBeforePause += gameClock.getElapsedTime(); // Save the elapsed time
            gameClock.restart(); // Restart the clock to track pause duration

            // Save states and override all tiles with revealed texture for visual feedback
            for (auto &row : tiles) {
                for (auto &tile : row) {
                    tile.savePauseState();
                    tile.setPausedTexture(revealedTexture); // Force tile to appear revealed
                }
            }
        } else {
            playButton.setTexture(pauseTexture); // Show the pause icon
            gameClock.restart(); // Restart the clock for post-pause timing

            // Restore states for all tiles
            for (auto &row : tiles) {
                for (auto &tile : row) {
                    tile.restorePauseState();
                    tile.restoreTexture(hiddenTexture, flagTexture, revealedTexture, numberTextures);
                }
            }
        }
    }

























    void revealTile(GameTile &tile) {

        if (tile.getIsFlagged() || tile.isRevealed()) return;

        if (tile.hasMine()) {
            
            for (auto &row : tiles) {
                for (auto &t : row) {
                    if (t.hasMine()) {
                        t.reveal(revealedTexture, nullptr, &mineTexture);
                    }
                }
            }
            happyFaceButton.setTexture(loseFaceTexture);
            gameOver = true;
            return;
        }

        if (tile.getAdjacentMines() > 0) {
            tile.reveal(revealedTexture, &numberTextures[tile.getAdjacentMines() - 1]);
        } else {
            queue<GameTile *> toReveal;
            toReveal.push(&tile);
            tile.reveal(revealedTexture);  // Mark the initial tile as revealed

            while (!toReveal.empty()) {
                GameTile *current = toReveal.front();
                toReveal.pop();

                for (auto neighbor : current->getAdjacentTiles()) {
                    if (!neighbor->isRevealed() && !neighbor->hasMine() && !neighbor->getIsFlagged()) {
                        neighbor->reveal(revealedTexture);

                        if (neighbor->getAdjacentMines() == 0) {
                            toReveal.push(neighbor);
                        } else {
                            neighbor->reveal(revealedTexture, &numberTextures[neighbor->getAdjacentMines() - 1]);
                        }
                    }
                }
            }
        }

        // Check if the player has won
        if (checkWin()) {
            happyFaceButton.setTexture(winFaceTexture);
            gameOver = true;
        }
    }







public:
    GameWindow(const string &configPath, int width, int height, const string &playerName)
    : Window(width, height, "Minesweeper Game"), playerName(playerName) {
        loadConfig(configPath);
        loadTextures();
        loadButtonTextures();
        initializeTiles();
        positionButtons();
        positionCounter();
        positionTimer();
        placeMines();
        remainingMines = mines;
        updateCounter();
    }





    void run() override {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                handleInput(event);
            }

            window.clear(sf::Color::White);

            // Update the timer only if not paused and game is ongoing
            if (!paused && !gameOver) {
                updateTimer();
            }

            // Draw the game tiles
            for (const auto &row : tiles) {
                for (const auto &tile : row) {
                    tile.draw(window);

                    // Draw debug information only if not paused and debug mode is active
                    if (!paused && debugMode && tile.hasMine()) {
                        sf::Sprite mineSprite;
                        mineSprite.setTexture(mineTexture);
                        mineSprite.setPosition(tile.getPosition());
                        window.draw(mineSprite);
                    }
                }
            }

            // Draw the timer
            for (const auto &sprite : timerMinutesSprites) {
                window.draw(sprite);
            }
            for (const auto &sprite : timerSecondsSprites) {
                window.draw(sprite);
            }

            // Draw the counter and buttons
            for (const auto &sprite : counterSprites) {
                window.draw(sprite);
            }
            window.draw(happyFaceButton);
            window.draw(debugButton);
            window.draw(playButton);
            window.draw(leaderboardButton);

            window.display();
        }
    }






};




int main() {
    const std::string configPath = "files/config.cfg";

    int cols, rows;
    ifstream configFile(configPath);
    if (!configFile) {
        cerr << "Error: Could not open config file.\n";
        return -1;
    }
    configFile >> cols >> rows;
    configFile.close();

    int width = cols * 32;
    int height = rows * 32 + 100;

    WelcomeWindow welcomeWindow(width, height);
    welcomeWindow.run();

    if (welcomeWindow.shouldLaunch()) {
        std::string playerName = welcomeWindow.getPlayerName(); // Retrieve the player's name
        GameWindow gameWindow(configPath, width, height, playerName); // Pass the name to GameWindow
        gameWindow.run();
    }

    return 0;
}




