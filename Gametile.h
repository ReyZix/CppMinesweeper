#ifndef GAMETILE_H
#define GAMETILE_H

#include <vector>
#include <SFML/Graphics.hpp>
using namespace std;

class GameTile {
private:
    sf::Sprite sprite;
    bool isMine = false;
    bool revealed = false;
    bool isFlagged = false;
    int adjacentMines = 0;

    vector<GameTile*> adjacentTiles;
    vector<sf::Sprite> numberSprites; // Store overlay sprites for numbers
    sf::Sprite mineSprite;            // Overlay sprite for mines

    // Pause state variables
    bool pauseWasRevealed = false;
    bool pauseWasFlagged = false;
    int pausedAdjacentMines = 0;

public:
    // Constructor to initialize with the hidden texture
    GameTile(sf::Texture &hiddenTexture) {
        sprite.setTexture(hiddenTexture);
    }

    // Add adjacent tiles to the tile
    void addAdjacentTile(GameTile* tile) {
        adjacentTiles.push_back(tile);
    }

    // Getters and setters for mine status
    void setMine(bool mineStatus) { isMine = mineStatus; }
    bool hasMine() const { return isMine; }

    // Reveal the tile
    void reveal(sf::Texture &revealedTexture, const sf::Texture *numberTexture = nullptr, const sf::Texture *mineTexture = nullptr) {
        if (isFlagged) return; // Prevent revealing flagged tiles
        revealed = true;
        sprite.setTexture(revealedTexture); // Set the base texture to revealed

        if (isMine && mineTexture) {
            mineSprite.setTexture(*mineTexture);
            mineSprite.setPosition(sprite.getPosition());
        } else if (numberTexture && adjacentMines > 0) {
            sf::Sprite numberSprite(*numberTexture);
            numberSprite.setPosition(sprite.getPosition());
            numberSprites.push_back(numberSprite);
        }
    }

    void flag(sf::Texture &flagTexture, sf::Texture &hiddenTexture) {
        if (revealed) return; // Don't allow flagging revealed tiles

        if (isFlagged) {
            // Unflag the tile
            isFlagged = false;
            sprite.setTexture(hiddenTexture);

            // Clear the flag sprite
            mineSprite = sf::Sprite(); // Reset the mine/flag sprite
        } else {
            // Place a flag
            isFlagged = true;
            mineSprite.setTexture(flagTexture);
            mineSprite.setPosition(sprite.getPosition());
        }
    }

    void reveal(const sf::Texture &revealedTexture, const sf::Texture *numberTexture = nullptr, const sf::Texture *mineTexture = nullptr);






    void revealMineAfterLoss(const sf::Texture &mineTexture) {
        if (isFlagged) {
            // Keep the flag sprite but overlay the mine sprite
            mineSprite.setTexture(mineTexture);
            mineSprite.setPosition(sprite.getPosition());
        } else {
            // Directly reveal the mine if it wasn't flagged
            sprite.setTexture(mineTexture);
        }
        revealed = true; // Mark the tile as revealed
    }











    // Getters and setters for adjacent mines count
    void setAdjacentMines(int count) { adjacentMines = count; }
    int getAdjacentMines() const { return adjacentMines; }

    // Check if the tile is revealed
    bool isRevealed() const { return revealed; }

    // Getter for flagged state
    bool getIsFlagged() const { return isFlagged; }

    // Save the current state for pause
    void savePauseState() {
        pauseWasRevealed = revealed;
        pauseWasFlagged = isFlagged;
        pausedAdjacentMines = adjacentMines;
    }



    void setPausedTexture(sf::Texture &revealedTexture) {
        sprite.setTexture(revealedTexture); // Overlay with tile_revealed
        if (isFlagged) {
            mineSprite.setTexture(revealedTexture); // Clear the flag during pause
        }
        numberSprites.clear(); // Temporarily hide numbers during pause
    }



    // Restore the state after unpause
    void restorePauseState() {
        revealed = pauseWasRevealed;
        isFlagged = pauseWasFlagged;
        adjacentMines = pausedAdjacentMines;
    }

    void restoreTexture(sf::Texture &hiddenTexture, sf::Texture &flagTexture, sf::Texture &revealedTexture, vector<sf::Texture> &numberTextures) {
        if (isFlagged) {
            sprite.setTexture(hiddenTexture);
            mineSprite.setTexture(flagTexture);
            mineSprite.setPosition(sprite.getPosition());
        } else if (revealed) {
            sprite.setTexture(revealedTexture);
            if (adjacentMines > 0) {
                sf::Sprite numberSprite(numberTextures[adjacentMines - 1]);
                numberSprite.setPosition(sprite.getPosition());
                numberSprites.push_back(numberSprite);
            }
        } else {
            sprite.setTexture(hiddenTexture);
            mineSprite.setTexture(sf::Texture()); // Clear flag if unflagged
        }
    }

   // int getAdjacentMines() const { return adjacentMines; }



    void draw(sf::RenderWindow &window) const {
        window.draw(sprite); // Always draw the base sprite first

        if (isFlagged) {
            window.draw(mineSprite); // Draw the flag if the tile is flagged
        }

        if (revealed) {
            if (isMine) {
                window.draw(mineSprite); // Draw the mine if the tile is revealed and is a mine
            }
            if (adjacentMines > 0) {
                for (const auto &numberSprite : numberSprites) {
                    window.draw(numberSprite);
                }
            }
        }
    }


    const vector<GameTile*>& getAdjacentTiles() const {
        return adjacentTiles;
    }


    // Set position for tile and overlays
    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
        mineSprite.setPosition(x, y);
        for (auto &numberSprite : numberSprites) {
            numberSprite.setPosition(x, y);
        }
    }

    // Get position of the tile
    sf::Vector2f getPosition() const { return sprite.getPosition(); }

    // Get bounds of the tile
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
};

#endif
