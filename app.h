
#include <imgui.h>
#include <implot.h>

#include <string>
#include <vector>

using namespace std;
using namespace ImGui;

typedef double value_t;

struct Thing {
    ImVec2 pos;
    ImVec2 vel;
    float radius;
    float distance;
    ImColor color;
    float pop_counter;
};

class MyApp {
  public:
    MyApp();
    void setup();
    void update();
    void fire();

    struct GameState {
        int score;
        float difficulty = 0;
        float grace_factor = 1.2;
        int wave;
        vector<Thing> things;
        string formula = "x";
        float firing_counter;
        bool allow_dragging = false;
        value_t plot_min;
        value_t plot_max;
    } gameState;

  private:
    void reset_game();
    void calculate();
    void start_wave();
    void update_things();
    void main_menu();
    void game_grid(ImVec2 grid_size);
    void controls();
    void game_window();
};

