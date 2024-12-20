#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include "window.h"

using namespace std;

struct LeaderboardEntry {
    int time;             // Total seconds
    string formattedTime; // "MM:SS" format
    string name;
};

class LeaderboardWindow : public Window {
private:
    vector<LeaderboardEntry> entries;
    sf::Font font;
    sf::Text title;
    vector<sf::Text> playerTexts;

    string leaderboardFilePath = "files/leaderboard.txt";

    void loadLeaderboard() {
        entries.clear(); // Clear any existing entries to avoid duplicates
        ifstream file(leaderboardFilePath);
        if (!file.is_open()) {
            cerr << "Error: Could not open leaderboard file.\n";
            return;
        }

        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string timeStr, name;
            getline(ss, timeStr, ',');
            getline(ss, name);

            // Convert MM:SS format to total seconds
            int minutes = stoi(timeStr.substr(0, 2));
            int seconds = stoi(timeStr.substr(3, 2));
            int totalSeconds = minutes * 60 + seconds;

            entries.push_back({totalSeconds, timeStr, name});
        }
        file.close();

        // Sort entries by time (ascending order)
        sort(entries.begin(), entries.end(), [](const LeaderboardEntry &a, const LeaderboardEntry &b) {
            return a.time < b.time;
        });
    }



    void displayLeaderboard(const string &currentPlayerName = "", int currentTime = -1) {
        // Clear existing texts
        playerTexts.clear();

        // Create the title
        title.setString("LEADERBOARD");
        title.setFont(font);
        title.setCharacterSize(20);
        title.setFillColor(sf::Color::White);
        title.setStyle(sf::Text::Bold | sf::Text::Underlined);
        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setPosition(window.getSize().x / 2 - titleBounds.width / 2, window.getSize().y / 2 - 120);

        // Display entries
        int yOffset = window.getSize().y / 2 + 20;
        for (size_t i = 0; i < entries.size(); ++i) {
            string rank = to_string(i + 1) + ". ";
            string playerEntry = rank + entries[i].formattedTime + " " + entries[i].name;

            // Highlight current player with an asterisk
            if (entries[i].name == currentPlayerName && entries[i].time == currentTime) {
                playerEntry += " *";
            }

            sf::Text text;
            text.setFont(font);
            text.setCharacterSize(18);
            text.setFillColor(sf::Color::White);
            text.setStyle(sf::Text::Bold);
            text.setString(playerEntry);

            // Center align each line
            sf::FloatRect textBounds = text.getLocalBounds();
            text.setPosition(window.getSize().x / 2 - textBounds.width / 2, yOffset);

            playerTexts.push_back(text);
            yOffset += 30; // Adjust spacing between lines
        }
    }

public:
    LeaderboardWindow(int width, int height) : Window(width, height, "Leaderboard") {
        if (!font.loadFromFile("files/font.ttf")) {
            throw runtime_error("Error: Unable to load font from files/font.ttf");
        }
    }

    void open(const std::string &currentPlayerName = "", int currentTime = -1) {
        // Load leaderboard entries from file
        loadLeaderboard();

        // Only display; do not add currentPlayerName unless they won the game
        displayLeaderboard(currentPlayerName, currentTime);

        // Show leaderboard window
        run();
    }

    void run() override {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }

            window.clear(sf::Color::Blue);
            window.draw(title);
            for (const auto &text : playerTexts) {
                window.draw(text);
            }
            window.display();
        }
    }

    void saveLeaderboard() {
        ofstream file(leaderboardFilePath, ios::trunc); // Overwrite file contents
        if (!file.is_open()) {
            cerr << "Error: Could not open leaderboard file for writing.\n";
            return;
        }

        for (const auto &entry : entries) {
            file << entry.formattedTime << "," << entry.name << endl;
        }

        file.close();
    }



    void addPlayerScore(const std::string &playerName, int totalSeconds) {
        // Format the time (e.g., "MM:SS")
        std::string formattedTime = formatTime(totalSeconds);

        // Load existing leaderboard entries to ensure no data is lost
        loadLeaderboard();

        // Check if the player's score already exists
        auto it = std::find_if(entries.begin(), entries.end(), [&](const LeaderboardEntry &entry) {
            return entry.name == playerName;
        });

        if (it != entries.end()) {
            // Update the score if the new time is better
            if (totalSeconds < it->time) {
                it->time = totalSeconds;
                it->formattedTime = formattedTime;
            }
        } else {
            // Add the new player's score
            entries.push_back({totalSeconds, formattedTime, playerName});
        }

        // Sort entries by time (ascending order)
        std::sort(entries.begin(), entries.end(), [](const LeaderboardEntry &a, const LeaderboardEntry &b) {
            return a.time < b.time;
        });

        // Keep only the top 5 scores
        if (entries.size() > 5) {
            entries.resize(5);
        }

        // Save the updated leaderboard to the file
        saveLeaderboard();
    }




    static string formatTime(int totalSeconds) {
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        stringstream ss;
        ss << (minutes < 10 ? "0" : "") << minutes << ":" << (seconds < 10 ? "0" : "") << seconds;
        return ss.str();
    }


    const vector<LeaderboardEntry>& getEntries() const {
        return entries;
    }



};
