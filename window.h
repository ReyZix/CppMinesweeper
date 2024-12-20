//
// Created by BTK on 11/24/2024.
//

#ifndef WINDOW_H
#define WINDOW_H

#include <SFML/Graphics.hpp>
#include <string>
using namespace std;
class Window {
protected:
    sf::RenderWindow window;
    sf::Font font;
    int width;
    int height;
    string title;


    public:
    Window(int width, int height,const string &title): width(width), height(height), title(title) {
        window.create(sf::VideoMode(width, height), title, sf::Style::Close);

        if(!font.loadFromFile("files/font.ttf")) {
            throw std::runtime_error("Could not load font");
        }

    }


    virtual ~Window() {} // window destructor

    virtual void run() = 0;



    protected: //this one is to set text

    void setText(sf::Text &text,const string& content, float x, float y, unsigned int size, sf::Color color) {
        text.setFont(font);
        text.setString(content);
        text.setCharacterSize(size);
        text.setFillColor(color);
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.width/2.0f, textRect.height/2.0f);
        text.setPosition(x,y);



    }
};





#endif //WINDOW_H
