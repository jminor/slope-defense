// my_app.cpp
#include <Mahi/Gui.hpp>
#include <Mahi/Util.hpp>

#include <exprtk.hpp>

using namespace mahi::gui;
using namespace mahi::util;
using namespace std;

typedef double value_t;
typedef exprtk::symbol_table<value_t> symbol_table_t;
typedef exprtk::expression<value_t>   expression_t;
typedef exprtk::parser<value_t>       parser_t;

float random(float low, float high) {
    return drand48() * (high - low) + low;
}

struct Thing {
    ImVec2 pos;
    ImVec2 vel;
    float radius;
    ImColor color;
};

// Inherit from Application
class MyApp : public Application {
public:
    struct GameState {
        int score;
        int wave;
        vector<Thing> things;
        string formula = "x";
    } gameState;


    MyApp(const Config& config) : Application(config) {
        reset_game();
    }

    void reset_game() {
        gameState.score = 0;
        gameState.wave = 1;
        gameState.things.clear();
        start_wave();
    }

//    const int num_pts = 1000;
#define num_pts 1000
    ImPlotPoint pts[num_pts];
    bool is_valid = false;

    bool show_demo = false;
    bool show_implot_demo = false;

    void calculate() {
        value_t val_x;
        value_t val_t;
        symbol_table_t symbol_table;
        expression_t expression;
        parser_t parser;

//        symbol_table.clear();
        symbol_table.add_variable("x", val_x);
        symbol_table.add_variable("t", val_t);
        symbol_table.add_constants();
        expression.register_symbol_table(symbol_table);

        is_valid = parser.compile(gameState.formula, expression);

        if (is_valid) {
            val_t = time().as_seconds();
            for (int i=0; i<num_pts; i++) {
                val_x = i/(num_pts-1) * 200 - 100;
                pts[i].x = val_x;
                pts[i].y = expression.value();
            }
        }
    }

    void start_wave() {
        for (int i=0; i<gameState.wave; i++) {
            Thing t;
            t.pos = ImVec2(random(-100, 100), random(-100, 100));
            t.vel = ImVec2(random(-1, 1), random(-1, 1));
            t.radius = 5;
            t.color = ImColor(random(0,1), random(0,1), random(0,1), 1.0);
            gameState.things.push_back(t);
        }
        calculate();
    }

    void update_things() {
        float dt = delta_time().as_seconds();
        for (auto& t : gameState.things) {
            t.pos.x += t.vel.x * dt;
            t.pos.y += t.vel.y * dt;
        }

    }

    void game_window() {
        ImGui::Begin("Slope Defense");

        char buf[1000];
        snprintf(buf, sizeof(buf), "%s", gameState.formula.c_str());
        if (ImGui::InputText("Formula", buf, sizeof(buf))) {
            gameState.formula = string(buf);
        }
        calculate();

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255,0,0,255));
        if (ImGui::Button("Fire!"))
          print("Hello, World!");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::SetNextItemWidth(100);
        if (ImGui::InputInt("Wave", &gameState.wave)) {
            gameState.wave = max(1, gameState.wave);
            start_wave();
        }

        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 grid_size(avail.x, avail.y);

        ImPlot::SetNextPlotLimits(-100, 100, -100, 100);
        ImPlot::BeginPlot("Game Grid", "x", "y", grid_size);

        ImPlot::PlotLine("sin(x)", &pts[0].x, &pts[0].y, num_pts, 0, sizeof(ImPlotPoint));
        ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
//        ImPlot::PlotLine("x^2", xs2, ys2, 11);

        for (int i=0; i<gameState.things.size(); i++) {
            auto& thing = gameState.things[i];
            char name[100];
            snprintf(name, sizeof(name), "P%d", i);
            double x = thing.pos.x;
            double y = thing.pos.y;
            if (ImPlot::DragPoint(name, &x, &y, true, thing.color, thing.radius)) {
                thing.pos.x = x;
                thing.pos.y = y;
            }
        }

        ImPlot::EndPlot();

        ImGui::End();
    }

    void setup() {
        ImGui::StyleColorsMahiDark1();
        ImGuiStyle &style = ImGui::GetStyle();
        // Main
        style.WindowPadding    = ImVec2(8, 8);
        style.FramePadding     = ImVec2(3, 2);
        style.ItemSpacing      = ImVec2(4, 4);
        style.ItemInnerSpacing = ImVec2(4, 4);
        style.IndentSpacing    = 20.0f;
        style.ScrollbarSize    = 15.0f;
        style.GrabMinSize      = 5.0f;
        // Rounding
        style.WindowRounding    = 2.0f;
        style.ChildRounding     = 2.0f;
        style.FrameRounding     = 2.0f;
        style.PopupRounding     = 2.0f;
        style.ScrollbarRounding = 10.0f;
        style.GrabRounding      = 2.0f;
        style.TabRounding       = 2.0f;
        // Alignment
        style.WindowMenuButtonPosition = ImGuiDir_Right;
        // Setup Dear ImGui style
        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look
        // identical to regular ones.
        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding              = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // DPI scaling method 1 (this worked for my previous project, but not this...):
        io.FontGlobalScale *= 1.5f;
    }

    void update() override {
        update_things();

        game_window();

        if (show_demo) {
            ImGui::ShowDemoWindow(&show_demo);
        }

        if (show_implot_demo) {
            ImPlot::ShowDemoWindow(&show_implot_demo);
        }
    }
};

int main() {
    Application::Config config;
    config.transparent = true;
    config.decorated = false;
    config.resizable = false;
    MyApp app(config);
    app.setup();
    app.run();
    return 0;
}