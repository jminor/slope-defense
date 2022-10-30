// my_app.cpp
#include <Mahi/Gui.hpp>
#include <Mahi/Util.hpp>

#include <imgui_internal.h>

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
    float pop_counter;
};

// Inherit from Application
class MyApp : public Application {
public:
    struct GameState {
        int score;
        float difficulty = 0;
        int wave;
        vector<Thing> things;
        string formula = "x";
        float firing_counter;
    } gameState;


    MyApp(const Config& config) : Application(config) {
        reset_game();
    }

    void reset_game() {
        gameState.score = 0;
        gameState.wave = 1;
        gameState.things.clear();
        gameState.firing_counter = 0;
        start_wave();
    }

//    const int num_pts = 1000;
#define num_pts 1000
    ImPlotPoint pts[num_pts];
    bool is_valid = false;

    bool show_demo = false;
    bool show_implot_demo = false;
    bool show_style_editor = false;

    bool set_focus = true;

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
                val_x = i/(value_t)(num_pts-1) * 200.0 - 100.0;
                pts[i].x = val_x;
                pts[i].y = expression.value();
            }

            // check for collisions
            if (gameState.firing_counter > 0) {
                for (auto& t : gameState.things) {
                    // already popping?
                    if (t.pop_counter > 0) continue;

                    val_x = t.pos.x;
                    value_t val_y = expression.value();

                    if (abs(val_y - t.pos.y) < t.radius / 2) {
                        t.pop_counter = 0.3;
                    }
                }
            }
        }
    }

    void start_wave() {
        for (int i=0; i<gameState.wave * 2 + 5; i++) {
            Thing t;
            t.pos = ImVec2(random(-100, 100), random(-100, 100));
            t.vel = ImVec2(random(-1, 1), random(-1, 1));
            t.radius = 5;
            t.color = ImColor(random(0,1), random(0,1), random(0,1), 1.0);
            t.pop_counter = 0;
            gameState.things.push_back(t);
        }
    }

    void update_things() {
        float dt = delta_time().as_seconds();

        auto it = gameState.things.begin();
        while (it != gameState.things.end()) {

            if (it->pop_counter > 0) {
                it->pop_counter -= dt;
                if (it->pop_counter <= 0) {
                    it = gameState.things.erase(it);
                    continue;
                }

                it->radius += 30.0 * dt;
                it->color = ImColor(1.0f, 1.0f, 1.0f, 10*ImSaturate(it->pop_counter));
            }

            it->pos.x += it->vel.x * dt * gameState.difficulty;
            it->pos.y += it->vel.y * dt * gameState.difficulty;

            // wrap
            if (it->pos.x < -100) it->pos.x += 200;
            if (it->pos.y < -100) it->pos.y += 200;
            if (it->pos.x > 100) it->pos.x -= 200;
            if (it->pos.y > 100) it->pos.y -= 200;

            ++it;
        }

        // Update the graph based on the current formula
        // Note: We do this every frame beacause it might vary based on time 't'
        // and also because collisions are handled in there
        calculate();

        if (gameState.firing_counter > 0) {
            gameState.firing_counter = fmaxf(0, gameState.firing_counter - dt);
        };

        if (gameState.things.size() == 0) {
            gameState.wave++;
            start_wave();
        }
    }

    void main_menu() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Window")) {
                ImGui::MenuItem("Style Editor", NULL, &show_style_editor);
                ImGui::MenuItem("ImGui Demo", NULL, &show_demo);
                ImGui::MenuItem("ImPlot Demo", NULL, &show_implot_demo);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    static int MyCallback(ImGuiInputTextCallbackData* data)
    {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
        {
            MyApp* app = (MyApp*)data->UserData;
            app->fire();
        }
    }

    void fire() {
        gameState.firing_counter = 3;
        set_focus = true;
    }

    void game_window() {
        auto viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkPos());
        ImGui::SetNextWindowSize(viewport->GetWorkSize());
        ImGui::Begin("Slope Defense", NULL,
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoDocking
                     );

        main_menu();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth()-500);
        ImGui::BeginGroup();
        {

            ImGui::Text("Formula");
            ImGui::Text("y = ");
            ImGui::SameLine();
            char buf[1000];
            snprintf(buf, sizeof(buf), "%s", gameState.formula.c_str());
            if (set_focus) {
                ImGui::SetKeyboardFocusHere();
                set_focus = false;
            }
            if (ImGui::InputText("##Formula", buf, sizeof(buf), ImGuiInputTextFlags_CallbackCompletion, MyCallback, this)) {
                gameState.formula = string(buf);
            }
            ImGui::SetItemDefaultFocus();
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                fire();
            }
        }
        ImGui::EndGroup();

        ImGui::SameLine();

        ImGui::BeginGroup();
        {
            ImGui::Text(" ");
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255,150,0,255));
            if (ImGui::Button("Activate!")) {
                fire();
            }
            ImGui::PopStyleColor();
        }
        ImGui::EndGroup();

        ImGui::SameLine();
        ImGui::Dummy(ImVec2(50,0));
        ImGui::SameLine();

        ImGui::BeginGroup();
        {
            ImGui::Text("Wave");
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth()/2.0);
            if (ImGui::InputInt("##Wave", &gameState.wave)) {
                gameState.wave = max(1, gameState.wave);
                start_wave();
            }
        }
        ImGui::EndGroup();

        ImGui::SameLine();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
        ImGui::BeginGroup();
        {
            ImGui::Text("Difficulty");
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
            if (ImGui::DragFloat("##Difficulty", &gameState.difficulty, 0.01, 0, 10)) {
//                gameState.wave = max(1, gameState.wave);
//                start_wave();
            }
        }
        ImGui::EndGroup();

        ImVec2 avail = ImGui::GetContentRegionAvail();
        float size = fminf(avail.x, avail.y);
        ImVec2 grid_size(size, size);

        ImPlot::SetNextPlotLimits(-100, 100, -100, 100);
        ImPlot::BeginPlot("Game Grid", "x", "y", grid_size,
                          0,
                          ImPlotAxisFlags_Lock,
                          ImPlotAxisFlags_Lock);

        const ImVec4 line_normal_color = (ImVec4)ImColor(0.3f, 0.3f, 0.8f, 1.0f);
        const ImVec4 line_firing_color = (ImVec4)ImColor(1.0f, 0.7f, 0.1f, 1.0f);
        ImVec4 line_color = ImLerp(line_normal_color,
                                   line_firing_color,
                                   ImSaturate(gameState.firing_counter));
        float line_weight = ImLerp(1.0f, 5.0f, ImSaturate(gameState.firing_counter));

        ImPlot::SetNextLineStyle(line_color, line_weight);
        ImPlot::PlotLine("", &pts[0].x, &pts[0].y, num_pts, 0, sizeof(ImPlotPoint));
        ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);

        for (int i=0; i<gameState.things.size(); i++) {
            auto& thing = gameState.things[i];
            char name[100];
            snprintf(name, sizeof(name), "P%d", i);
            double x = thing.pos.x;
            double y = thing.pos.y;
            if (ImPlot::DragPoint(name, &x, &y, true, thing.color, thing.radius)) {
//                thing.pos.x = x;
//                thing.pos.y = y;
            }
        }

        ImPlot::EndPlot();

        ImGui::End();
    }

    void setup() {
//        ImGui::StyleColorsMahiDark1();
        ImGui::StyleColorsClassic();
        ImPlot::StyleColorsClassic();
        ImGuiStyle &style = ImGui::GetStyle();
        // Main
        style.WindowBorderSize = 0;
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

        io.FontGlobalScale *= 1.75f;
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

        if (show_style_editor) {
            ImGui::Begin("Style Editor", &show_style_editor);
            ImGui::ShowStyleEditor();
            ImGui::End();
        }
    }
};

int main() {
    Application::Config config;
    config.title = "Slope Defense";
    config.width = 1024 / 2;
    config.height = 768 / 2;
//    config.transparent = true;
//    config.decorated = false;
//    config.resizable = false;
    MyApp app(config);
    app.setup();
    app.run();
    return 0;
}
