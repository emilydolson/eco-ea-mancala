//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "web/web.h"
#include "../mancala.h"
#include "geometry/Circle2D.h"

namespace UI = emp::web;



class Interface : public UI::Animate {
private:
    Mancala game;
    UI::Document doc;
    UI::Canvas canvas;
    emp::array<emp::Circle, 14> holes;
    int player = 0;
    double width;
    double height;
    bool over = false;

public:
    Interface(double width, double height) : doc("emp_base"),
                  canvas(doc.AddCanvas(width, height, "map")),
                  width(width), height(height) {

        holes[0] = emp::Circle(8*width/9, height/2, (width/18)*.8);
        holes[7] = emp::Circle(width/9, height/2, (width/18)*.8);

        for (int i = 1; i < 7; i++) {
            holes[i] = emp::Circle((7-i+1)*(width/9), height/4, (width/18)*.8);
        }

        for (int i = 8; i < 14; i++) {
            holes[i] = emp::Circle((i-6)*(width/9), 3*height/4, (width/18)*.8);
        }

        canvas.On("click", [this](UI::MouseEvent event){ MouseClick(event);});

        Start();
    }

    void DoFrame() {
        canvas.Clear("brown");
        for (int i = 0; i < 14; i++) {
            canvas.Draw(holes[i], "white");
            canvas.CenterText(holes[i].GetCenterX(), holes[i].GetCenterY(), emp::to_string(game[i]), "black");
        }

        if (over) {
            canvas.CenterText(width/2, 10, "Game over! Player " + emp::to_string(game.GetWinner()) + " wins. Click to restart.", "white");
        } else {
            canvas.CenterText(width/2, 10, "Player " + emp::to_string(player) + "'s turn", "white");
        }
    }

    void MouseClick(UI::MouseEvent & event) {

        if (over) {
            game.Reset();
            over = false;
            return;
        }

        int x = event.clientX - canvas.GetXPos();
        int y = event.clientY - canvas.GetYPos();

        for (int i = 0; i < (int)holes.size(); i++) {
            if(holes[i].Contains(x, y)) {
                if ((player && i > 7) || (!player && i > 0 && i < 7)) {
                    if (!game.ChooseCell(i)) {
                        player = !player;
                    }
                }
            }
      }

      if (game.IsOver()) {
          over = true;
      }
  }

};

Interface inter(800, 400);

int main()
{
}
